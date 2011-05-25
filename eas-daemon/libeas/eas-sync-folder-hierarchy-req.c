/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-folder-hierarchy-req.h"
#include "eas-sync-folder-msg.h"


typedef enum {
	EasSyncFolderHierarchyStep1 = 0,
	EasSyncFolderHierarchyStep2
} EasSyncFolderHierarchyReqState;

struct _EasSyncFolderHierarchyReqPrivate
{
	EasSyncFolderMsg* syncFolderMsg;
	EasSyncFolderHierarchyReqState state;
	guint64 accountID;
	gchar* syncKey;
};

#define EAS_SYNC_FOLDER_HIERARCHY_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReqPrivate))



G_DEFINE_TYPE (EasSyncFolderHierarchyReq, eas_sync_folder_hierarchy_req, EAS_TYPE_REQUEST_BASE);

static void
eas_sync_folder_hierarchy_req_init (EasSyncFolderHierarchyReq *object)
{
	EasSyncFolderHierarchyReqPrivate *priv;
	
	object->priv = priv = EAS_SYNC_FOLDER_HIERARCHY_REQ_PRIVATE(object);

	g_debug("eas_sync_folder_hierarchy_req_init++");
	priv->syncFolderMsg = NULL;
	priv->state = EasSyncFolderHierarchyStep1;
	priv->accountID = -1;
	priv->syncKey = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_SYNC_FOLDER_HIERARCHY);

	g_debug("eas_sync_folder_hierarchy_req_init--");
}

static void
eas_sync_folder_hierarchy_req_finalize (GObject *object)
{
	EasSyncFolderHierarchyReq *req = (EasSyncFolderHierarchyReq *) object;
	EasSyncFolderHierarchyReqPrivate *priv = req->priv;
	/* TODO: Add deinitalization code here */

	g_debug("eas_sync_folder_hierarchy_req_finalize++");

	g_free(priv->syncKey);
	
	if (priv->syncFolderMsg) {
		g_object_unref(priv->syncFolderMsg);
	}

	G_OBJECT_CLASS (eas_sync_folder_hierarchy_req_parent_class)->finalize (object);
	g_debug("eas_sync_folder_hierarchy_req_finalize--");
}

static void
eas_sync_folder_hierarchy_req_class_init (EasSyncFolderHierarchyReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_debug("eas_sync_folder_hierarchy_req_class_init++");

	g_type_class_add_private (klass, sizeof (EasSyncFolderHierarchyReqPrivate));

	object_class->finalize = eas_sync_folder_hierarchy_req_finalize;
	g_debug("eas_sync_folder_hierarchy_req_class_init--");

}

EasSyncFolderHierarchyReq*
eas_sync_folder_hierarchy_req_new (const gchar* syncKey, guint64 accountId, EFlag *flag)
{
	EasSyncFolderHierarchyReq* self = g_object_new (EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, NULL);
	EasSyncFolderHierarchyReqPrivate *priv = self->priv;

	g_debug("eas_sync_folder_hierarchy_req_new++");
	
	g_assert(syncKey);
	
	priv->syncKey = g_strdup(syncKey);
	priv->accountID = accountId;
	eas_request_base_SetFlag(&self->parent_instance, flag);

	if (syncKey && !g_strcmp0(syncKey,"0"))
	{
		priv->state = EasSyncFolderHierarchyStep2;
	}

	g_debug("eas_sync_folder_hierarchy_req_new--");
	return self;
}

void
eas_sync_folder_hierarchy_req_Activate (EasSyncFolderHierarchyReq* self)
{
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	xmlDoc *doc = NULL;
	
	g_debug("eas_sync_folder_hierarchy_req_Activate++");

	// Create syn folder msg type
	priv->syncFolderMsg = eas_sync_folder_msg_new (priv->syncKey, priv->accountID);

	doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);

	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "FolderSync", doc, self);
	g_debug("eas_sync_folder_hierarchy_req_Activate--");
}

void
eas_sync_folder_hierarchy_req_MessageComplete (EasSyncFolderHierarchyReq* self, xmlDoc *doc)
{
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	
	g_debug("eas_sync_folder_hierarchy_req_MessageComplete++");

	eas_sync_folder_msg_parse_reponse (priv->syncFolderMsg, doc);

	xmlFree(doc);
	
		switch (priv->state) {
		default:
		{
			g_assert(0);
		}
		break;

		//We have started a first time sync, and need to get the sync Key from the result, and then do the proper sync
		case EasSyncFolderHierarchyStep1:
		{
		    //get syncKey
		    gchar* syncKey = g_strdup(eas_sync_folder_msg_get_syncKey (priv->syncFolderMsg));
			
			//clean up old message
			if (priv->syncFolderMsg) {
				g_object_unref(priv->syncFolderMsg);
			}
			
			//create new message with new syncKey
			priv->syncFolderMsg = eas_sync_folder_msg_new (syncKey, priv->accountID);

			//build request msg
			doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);
			
			//move to new state
			priv->state = EasSyncFolderHierarchyStep2;	
			
			eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "FolderSync", doc, self);

		}
		break;

		//we did a proper sync, so we need to inform the daemon that we have finished, so that it can continue and get the data
		case EasSyncFolderHierarchyStep2:
		{
			e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		}
		break;
	}
	g_debug("eas_sync_folder_hierarchy_req_MessageComplete--");
}

void
eas_sync_folder_hierarchy_req_ActivateFinish (EasSyncFolderHierarchyReq* self, 
                                              gchar** ret_sync_key, 
                                              GSList** added_folders, 
                                              GSList** updated_folders, 
                                              GSList** deleted_folders)
{
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	
	g_debug("eas_sync_folder_hierarchy_req_ActivateFinish++");

	*ret_sync_key    = g_strdup(eas_sync_folder_msg_get_syncKey(priv->syncFolderMsg));
	*added_folders   = eas_sync_folder_msg_get_added_folders (priv->syncFolderMsg);
	*updated_folders = eas_sync_folder_msg_get_updated_folders (priv->syncFolderMsg);
	*deleted_folders = eas_sync_folder_msg_get_deleted_folders (priv->syncFolderMsg);
	
	g_debug("eas_sync_folder_hierarchy_req_ActivateFinish--");
}
