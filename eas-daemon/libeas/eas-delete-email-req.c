/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-delete-email-req.h"
#include "eas-sync-msg.h"

struct _EasDeleteEmailReqPrivate
{
	EasSyncMsg* syncMsg;
	guint64 accountID;
	gchar* syncKey;
	gchar* folder_id;
	GSList *server_ids_array;
	EasItemType ItemType;
};

#define EAS_DELETE_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReqPrivate))

G_DEFINE_TYPE (EasDeleteEmailReq, eas_delete_email_req, EAS_TYPE_REQUEST_BASE);

static void
eas_delete_email_req_init (EasDeleteEmailReq *object)
{
	EasDeleteEmailReqPrivate *priv;
	
	object->priv = priv = EAS_DELETE_EMAIL_REQ_PRIVATE(object);

	g_debug("eas_delete_email_req_init++");
	priv->accountID = -1;
	priv->syncKey = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_DELETE_MAIL);

	g_debug("eas_delete_email_req_init--");
}

static void
eas_delete_email_req_finalize (GObject *object)
{
	EasDeleteEmailReq *req = (EasDeleteEmailReq *) object;
	EasDeleteEmailReqPrivate *priv = req->priv;

	g_debug("eas_delete_email_req_finalize++");

	g_free(priv->syncKey);
	g_slist_foreach(priv->server_ids_array, (GFunc)g_object_unref, NULL);
	g_slist_free(priv->server_ids_array);
	
	g_debug("eas_delete_email_req_finalize--");
	G_OBJECT_CLASS (eas_delete_email_req_parent_class)->finalize (object);
}

static void
eas_delete_email_req_class_init (EasDeleteEmailReqClass *klass)
{
	g_debug("eas_delete_email_req_class_init++");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasDeleteEmailReqPrivate));

	object_class->finalize = eas_delete_email_req_finalize;
	g_debug("eas_delete_email_req_class_init--");

}

void
eas_delete_email_req_Activate (EasDeleteEmailReq *self, GError** error)
{
	EasDeleteEmailReqPrivate *priv = self->priv;
	xmlDoc *doc;
	gboolean getChanges = FALSE;

	g_debug("eas_delete_email_req_Activate++");
	//create sync  msg type
	priv->syncMsg = eas_sync_msg_new (priv->syncKey, priv->accountID, priv->folder_id, priv->ItemType);

    g_debug("eas_delete_email_req_Activate- syncKey = %s", priv->syncKey);

	g_debug("eas_delete_email_req_Activate - build messsage");
	//build request msg
	doc = eas_sync_msg_build_message (priv->syncMsg, getChanges, NULL, NULL, priv->server_ids_array);

	g_debug("eas_delete_email_req_Activate - send message");
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
	                            "Sync", 
	                            doc, 
	                            (struct _EasRequestBase *)self, 
	                            error);
	g_debug("eas_delete_email_req_Activate--");	
}

EasDeleteEmailReq *eas_delete_email_req_new (guint64 accountId, const gchar *syncKey, const gchar *folderId, const GSList *server_ids_array, EFlag *flag)
{
	g_debug("eas_delete_email_req_new++");

	EasDeleteEmailReq* self = g_object_new (EAS_TYPE_DELETE_EMAIL_REQ, NULL);
	EasDeleteEmailReqPrivate *priv = self->priv;
	
	g_assert(syncKey);
	
	priv->syncKey = g_strdup(syncKey);
	priv->folder_id = g_strdup(folderId);
	guint listCount;
	guint listLen = g_slist_length((GSList*)server_ids_array);
	gchar *server_id = NULL;
	for(listCount = 0;listCount < listLen;listCount++)
	{
		server_id = g_slist_nth_data((GSList*)server_ids_array, listCount);
		priv->server_ids_array = g_slist_append(priv->server_ids_array, g_strdup(server_id));
	}
	
	priv->accountID = accountId;
	eas_request_base_SetFlag(&self->parent_instance, flag);

	g_debug("eas_delete_email_req_new--");
	return self;
}

void eas_delete_email_req_ActivateFinish (EasDeleteEmailReq* self, gchar** ret_sync_key, GError** error)
{
	EasDeleteEmailReqPrivate *priv = self->priv;
	
	g_debug("eas_delete_email_req_ActivateFinish++");

	*ret_sync_key = g_strdup(eas_sync_msg_get_syncKey(priv->syncMsg));
	
	g_debug("eas_delete_email_req_ActivateFinish--");
}

void eas_delete_email_req_MessageComplete (EasDeleteEmailReq *self, xmlDoc* doc, GError** error)
{
	EasDeleteEmailReqPrivate *priv = self->priv;
	
	g_debug("eas_delete_email_req_MessageComplete++");

	eas_sync_msg_parse_reponse (priv->syncMsg, doc, error);

	xmlFree(doc);
	
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));

	g_debug("eas_delete_email_req_MessageComplete--");
}

