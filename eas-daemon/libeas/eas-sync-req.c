/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-req.h"
#include "eas-sync-msg.h"

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


void
eas_sync_req_Activate (EasSyncReq *self, const gchar* syncKey, guint64 accountID, EFlag *flag, const gchar* folderId, EasItemType type)
{
	EasSyncReqPrivate* priv = self->priv;
	xmlDoc *doc;
	gboolean getChanges = FALSE;

	g_debug("eas_sync_req_activate++");
	eas_request_base_SetFlag(&self->parent_instance, flag);
	
	priv->accountID = accountID;
	
	priv->ItemType = type;
	
	priv->folderID = g_strdup(folderId);
	
	g_debug("eas_sync_req_activate - new Sync  mesg");
	//create sync  msg type
	priv->syncMsg = eas_sync_msg_new (syncKey, accountID, folderId, type);

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
	
	

	g_debug("eas_sync_req_activate - send message");
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self);
	g_debug("eas_sync_req_activate--");
}

void
eas_sync_req_MessageComplete (EasSyncReq *self, xmlDoc* doc)
{
	EasSyncReqPrivate *priv = self->priv;
	
	g_debug("eas_sync_req_MessageComplete++");

	eas_sync_msg_parse_reponse (priv->syncMsg, doc);

	xmlFree(doc);
	
		switch (priv->state) {
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
			
			eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self);

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
	g_debug("eas_sync_req_MessageComplete--");
}

void
eas_sync_req_ActivateFinish (EasSyncReq* self,
								gchar** ret_sync_key,
								GSList** added_items,
								GSList** updated_items,
								GSList** deleted_items)
{
	EasSyncReqPrivate *priv = self->priv;
	
	g_debug("eas_sync_req_Activate_Finish++");

	*ret_sync_key    = g_strdup(eas_sync_msg_get_syncKey(priv->syncMsg));
	*added_items   = eas_sync_msg_get_added_items (priv->syncMsg);
	*updated_items = eas_sync_msg_get_updated_items (priv->syncMsg);
	*deleted_items = eas_sync_msg_get_deleted_items (priv->syncMsg);
	
	g_debug("eas_sync_req_Activate_Finish--");
}
