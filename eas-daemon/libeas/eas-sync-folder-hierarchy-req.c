/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-folder-hierarchy-req.h"
#include "eas-sync-folder-msg.h"
#include "eas-connection-errors.h"


typedef enum {
	EasSyncFolderHierarchyStep1 = 0,
	EasSyncFolderHierarchyStep2
} EasSyncFolderHierarchyReqState;

struct _EasSyncFolderHierarchyReqPrivate
{
	GError *error;  // error passed into MessageComplete 
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
	priv->error = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_SYNC_FOLDER_HIERARCHY);

	g_debug("eas_sync_folder_hierarchy_req_init--");
}

static void
eas_sync_folder_hierarchy_req_finalize (GObject *object)
{
	EasSyncFolderHierarchyReq *req = (EasSyncFolderHierarchyReq *) object;
	EasSyncFolderHierarchyReqPrivate *priv = req->priv;

	g_debug("eas_sync_folder_hierarchy_req_finalize++");

	g_free(priv->syncKey);
	
	if (priv->syncFolderMsg) {
		g_object_unref(priv->syncFolderMsg);
	}
	if(priv->error)
	{
		g_clear_error(&priv->error);
	}

	G_OBJECT_CLASS (eas_sync_folder_hierarchy_req_parent_class)->finalize (object);
	g_debug("eas_sync_folder_hierarchy_req_finalize--");
}

static void
eas_sync_folder_hierarchy_req_class_init (EasSyncFolderHierarchyReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);
	void *tmp = parent_class;
	tmp = object_class;

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


gboolean
eas_sync_folder_hierarchy_req_Activate (EasSyncFolderHierarchyReq* self, GError** error)
{
	gboolean ret = FALSE;
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	xmlDoc *doc = NULL;
	
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_debug("eas_sync_folder_hierarchy_req_Activate++");

	// Create sync folder msg type
	priv->syncFolderMsg = eas_sync_folder_msg_new (priv->syncKey, priv->accountID);
	if(!priv->syncFolderMsg)
	{
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));		
		goto finish;
	}
	doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);
	if(!doc)
	{
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));	
		g_free(priv->syncFolderMsg);
		priv->syncFolderMsg = NULL;		
		goto finish;
	}
	ret = eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
	                                  "FolderSync", 
	                                  doc, 
	                                  (struct _EasRequestBase *)self, 
	                                  error);

finish:
	g_debug("eas_sync_folder_hierarchy_req_Activate--");
	return ret;
}

/*
 * @param error any error that occured (or NULL) [full transfer]
 */
void
eas_sync_folder_hierarchy_req_MessageComplete (EasSyncFolderHierarchyReq* self, xmlDoc *doc, GError* error_in)
{
	GError *error = NULL;
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	gboolean ret;
	
	// if an error occurred, store it and signal daemon
	if(error_in)
	{
		priv->error = error_in;
		e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		goto finish;
	}
	
	g_debug("eas_sync_folder_hierarchy_req_MessageComplete++");

	// if an error occurs when parsing, store it
	ret = eas_sync_folder_msg_parse_response (priv->syncFolderMsg, doc, &error);
	xmlFree(doc);
	if(!ret)
	{
		self->priv->error = error; 
		e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		goto finish;
	}
	
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
			
			ret = eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
			                                  "FolderSync", 
			                                  doc, 
			                                  (struct _EasRequestBase *)self, 
			                                  &error);
			if(!ret)
			{
				self->priv->error = error; 
				e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
				goto finish;
			}
		}
		break;

		//we did a proper sync, so we need to inform the daemon that we have finished, so that it can continue and get the data
		case EasSyncFolderHierarchyStep2:
		{
			e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		}
		break;
	}

finish:	
	g_debug("eas_sync_folder_hierarchy_req_MessageComplete--");
}

gboolean
eas_sync_folder_hierarchy_req_ActivateFinish (EasSyncFolderHierarchyReq* self, 
                                              gchar** ret_sync_key, 
                                              GSList** added_folders, 
                                              GSList** updated_folders, 
                                              GSList** deleted_folders, 
                                              GError** error)
{
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_debug("eas_sync_folder_hierarchy_req_ActivateFinish++");

	if(priv->error != NULL)// propogate any preceding error
	{
		/* store priv->error in error, if error != NULL,
		* otherwise call g_error_free() on priv->error
		*/
		g_propagate_error (error, priv->error);	
		priv->error = NULL;

		return FALSE;
	}
	
	*ret_sync_key    = g_strdup(eas_sync_folder_msg_get_syncKey(priv->syncFolderMsg));
	*added_folders   = eas_sync_folder_msg_get_added_folders (priv->syncFolderMsg);
	*updated_folders = eas_sync_folder_msg_get_updated_folders (priv->syncFolderMsg);
	*deleted_folders = eas_sync_folder_msg_get_deleted_folders (priv->syncFolderMsg);
	
	g_debug("eas_sync_folder_hierarchy_req_ActivateFinish--");

	return TRUE;
}
