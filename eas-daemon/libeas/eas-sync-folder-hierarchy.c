/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * 
 */

#include "eas-sync-folder-hierarchy.h"
#include "eas-sync-folder-msg.h"



G_DEFINE_TYPE (EasSyncFolderHierarchy, eas_sync_folder_hierarchy, EAS_TYPE_REQUEST_BASE);

#define EAS_SYNC_FOLDER_HIERARCHY_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_FOLDER_HIERARCHY, EasSyncFolderHierarchyPrivate))

typedef enum {
	EasSyncFolderHierarchyStep1 = 0,
	EasSyncFolderHierarchyStep2
} EasSyncFolderHierarchyState;


struct _EasSyncFolderHierarchyPrivate {
	EasSyncFolderMsg* syncFolderMsg;
	EasSyncFolderHierarchyState state;
	guint64 accountID;
};

static void
eas_sync_folder_hierarchy_init (EasSyncFolderHierarchy *object)
{
	EasSyncFolderHierarchyPrivate *priv;

	object->priv = priv = EAS_SYNC_FOLDER_HIERARCHY_PRIVATE(object);

	g_debug("eas_sync_folder_hierarchy_init++");
	priv->syncFolderMsg = NULL;
	priv->state = EasSyncFolderHierarchyStep1;
	priv->accountID= -1;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_SYNC_FOLDER_HIERARCHY);

	g_debug("eas_sync_folder_hierarchy_init--");
}

static void
eas_sync_folder_hierarchy_finalize (GObject *object)
{
	EasSyncFolderHierarchy *req = (EasSyncFolderHierarchy *) object;
	EasSyncFolderHierarchyPrivate *priv = req->priv;
	/* TODO: Add deinitalization code here */

	g_debug("eas_sync_folder_hierarchy_finalize++");
	
	if (priv->syncFolderMsg) {
		g_object_unref(priv->syncFolderMsg);
	}

	G_OBJECT_CLASS (eas_sync_folder_hierarchy_parent_class)->finalize (object);
	g_debug("eas_sync_folder_hierarchy_finalize--");
}

static void
eas_sync_folder_hierarchy_class_init (EasSyncFolderHierarchyClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_debug("eas_sync_folder_hierarchy_class_init++");
	g_type_class_add_private (klass, sizeof (EasSyncFolderHierarchyPrivate));
	object_class->finalize = eas_sync_folder_hierarchy_finalize;
	g_debug("eas_sync_folder_hierarchy_class_init--");
}


void
eas_sync_folder_hierarchy_Activate (EasSyncFolderHierarchy* self, const gchar* syncKey, guint64 accountId, EFlag *flag)
{
	EasSyncFolderHierarchyPrivate* priv = self->priv;
	xmlDoc *doc;

	g_debug("eas_sync_folder_hierarchy_activate++");
	eas_request_base_SetFlag(&self->parent_instance, flag);
	
	priv->accountID = accountId;

	g_debug("eas_sync_folder_hierarchy_activate - new Sync Folder mesg");
	//create syn folder msg type
	priv->syncFolderMsg = eas_sync_folder_msg_new (syncKey, accountId);

	g_debug("eas_sync_folder_hierarchy_activate - build messsage");
	//build request msg
	doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);
	
	//if syncKey is not 0, then we are not doing a first time sync and only need to send one message
	// so we  move state machine forward.
	if (!g_strcmp0(syncKey,"0"))
	{
		priv->state = EasSyncFolderHierarchyStep2;
	}

	g_debug("eas_sync_folder_hierarchy_activate - send message");
	// TODO
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "FolderSync", doc, self);
	g_debug("eas_sync_folder_hierarchy_activate--");
}

/**
 * @param doc The protocol xml to be parsed. MUST be freed with xmlFree()
 */
void 
eas_sync_folder_hierarchy_MessageComplete (EasSyncFolderHierarchy* self, xmlDoc* doc)
{
	EasSyncFolderHierarchyPrivate *priv = self->priv;
	
	g_debug("eas_sync_folder_hierarchy_MessageComplete++");

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
	g_debug("eas_sync_folder_hierarchy_MessageComplete--");
}

void eas_sync_folder_hierarchy_Activate_Finish (EasSyncFolderHierarchy* self,
                                                gchar** ret_sync_key,
												GSList** added_folders,
												GSList** updated_folders,
												GSList** deleted_folders)
{
	EasSyncFolderHierarchyPrivate *priv = self->priv;
	
	g_debug("eas_sync_folder_hierarchy_Activate_Finish++");

	*ret_sync_key    = g_strdup(eas_sync_folder_msg_get_syncKey(priv->syncFolderMsg));
	*added_folders   = eas_sync_folder_msg_get_added_folders (priv->syncFolderMsg);
	*updated_folders = eas_sync_folder_msg_get_updated_folders (priv->syncFolderMsg);
	*deleted_folders = eas_sync_folder_msg_get_deleted_folders (priv->syncFolderMsg);
	
	g_debug("eas_sync_folder_hierarchy_Activate_Finish--");
}
