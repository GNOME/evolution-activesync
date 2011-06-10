/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-req.h"
#include "eas-sync-msg.h"
#include "eas-connection-errors.h"

G_DEFINE_TYPE (EasSyncReq, eas_sync_req, EAS_TYPE_REQUEST_BASE);

#define EAS_SYNC_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_REQ, EasSyncReqPrivate))

typedef enum {
	EasSyncReqStep1 = 0,
	EasSyncReqStep2
} EasSyncReqState;


struct _EasSyncReqPrivate
{
	EasSyncMsg* syncMsg;
	EasSyncReqState state;
	guint accountID;
	gchar* folderID;
	EasItemType ItemType;
	GError *error;
};



static void
eas_sync_req_init (EasSyncReq *object)
{
	EasSyncReqPrivate *priv;

	object->priv = priv = EAS_SYNC_REQ_PRIVATE(object);

	g_debug("eas_sync_req_init++");
	priv->syncMsg = NULL;
	priv->state = EasSyncReqStep1;
	priv->accountID= -1;
	priv->folderID = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_SYNC);

	g_debug("eas_sync_req_init--");
}

static void
eas_sync_req_finalize (GObject *object)
{
	g_debug("eas_sync_req_finalize++");

    EasSyncReq *req = (EasSyncReq*)object;
	EasSyncReqPrivate *priv = req->priv;
	
	if(priv->syncMsg){
		g_object_unref(priv->syncMsg);
	}
	
	g_free(priv->folderID);

	G_OBJECT_CLASS (eas_sync_req_parent_class)->finalize (object);
	
	g_debug("eas_sync_req_finalize--");

}

static void
eas_sync_req_class_init (EasSyncReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasSyncReqPrivate));

	object_class->finalize = eas_sync_req_finalize;
}


gboolean
eas_sync_req_Activate (EasSyncReq *self, const gchar* syncKey, guint64 accountID, EFlag *flag, const gchar* folderId, EasItemType type, GError** error)
{
	gboolean ret = FALSE;
	g_debug("eas_sync_req_activate++");
	EasSyncReqPrivate* priv = self->priv;
	xmlDoc *doc;
	gboolean getChanges = FALSE;

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
	eas_request_base_SetFlag(&self->parent_instance, flag);
	
	priv->accountID = accountID;
	
	priv->ItemType = type;
	
	priv->folderID = g_strdup(folderId);
	
	g_debug("eas_sync_req_activate - new Sync  mesg");
	//create sync  msg type
	priv->syncMsg = eas_sync_msg_new (syncKey, accountID, folderId, type);
	if(!priv->syncMsg)
	{
		ret = FALSE;
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));		
		goto finish;
	}
	
    g_debug("eas_sync_req_activate- syncKey = %s", syncKey);

    //if syncKey is not 0, then we are not doing a first time sync and only need to send one message
	// so we  move state machine forward	
	if (g_strcmp0(syncKey,"0"))
	{
	    g_debug("switching state");
		priv->state = EasSyncReqStep2;
		getChanges = TRUE;
	}
	
	g_debug("eas_sync_req_activate - build messsage");
	//build request msg
	doc = eas_sync_msg_build_message (priv->syncMsg, getChanges, NULL, NULL, NULL);
	if(!doc)
	{
		ret = FALSE;
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));	
		g_free(priv->syncMsg);
		priv->syncMsg = NULL;		
		goto finish;
	}
	
	g_debug("eas_sync_req_activate - send message");
	ret = eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self, error);

finish:
	if(!ret)
	{
		g_assert(error == NULL || *error != NULL);
	}
	g_debug("eas_sync_req_activate--");

	return ret;
}


void
eas_sync_req_MessageComplete (EasSyncReq *self, xmlDoc* doc, GError* error_in)
{
	g_debug("eas_sync_req_MessageComplete++");
	
	GError *error = NULL;
	EasSyncReqPrivate* priv = self->priv;
	
	// if an error occurred, store it and signal daemon
	if(error_in)
	{
		priv->error = error_in;
		e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		goto finish;
	}
	
	gboolean ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
	xmlFree(doc);
	if(!ret)
	{
		self->priv->error = error; 
		e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		goto finish;
	}	
	
	switch (priv->state) 
	{
		default:
		{
			g_assert(0);
		}
		break;

		//We have started a first time sync, and need to get the sync Key from the result, and then do the proper sync
		case EasSyncReqStep1:
		{
			g_debug("eas_sync_req_MessageComplete step 1");
		    //get syncKey
		    gchar* syncKey = g_strdup(eas_sync_msg_get_syncKey (priv->syncMsg));
			
			g_debug("eas_sync_req synckey = %s", syncKey);
			
			//clean up old message
			if (priv->syncMsg) {
				g_object_unref(priv->syncMsg);
			}
			
			//create new message with new syncKey
			priv->syncMsg = eas_sync_msg_new (syncKey, priv->accountID, priv->folderID, priv->ItemType);

			//build request msg
			doc = eas_sync_msg_build_message (priv->syncMsg, TRUE, NULL, NULL, NULL);
			
			//move to new state
			priv->state = EasSyncReqStep2;	
			
			ret = eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self, &error);
			if(!ret)
			{
				self->priv->error = error; 
				e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
				goto finish;
			}			

		}
		break;

		//we did a proper sync, so we need to inform the daemon that we have finished, so that it can continue and get the data
		case EasSyncReqStep2:
		{
		    g_debug("eas_sync_req_MessageComplete step 2");
			e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
		}
		break;
	}

finish:	
	g_debug("eas_sync_req_MessageComplete--");
}

gboolean
eas_sync_req_ActivateFinish (EasSyncReq* self,
								gchar** ret_sync_key,
								GSList** added_items,
								GSList** updated_items,
								GSList** deleted_items, 
								GError** error)
{
	g_debug("eas_sync_req_Activate_Finish++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);	
	
	EasSyncReqPrivate *priv = self->priv;

	if(priv->error != NULL)// propogate any preceding error
	{
		/* store priv->error in error, if error != NULL,
		* otherwise call g_error_free() on priv->error
		*/
		g_propagate_error (error, priv->error);	
		priv->error = NULL;

		return FALSE;
	}	

	*ret_sync_key    = g_strdup(eas_sync_msg_get_syncKey(priv->syncMsg));
	*added_items   = eas_sync_msg_get_added_items (priv->syncMsg);
	*updated_items = eas_sync_msg_get_updated_items (priv->syncMsg);
	*deleted_items = eas_sync_msg_get_deleted_items (priv->syncMsg);
	
	g_debug("eas_sync_req_Activate_Finish--");

	return TRUE;
}
