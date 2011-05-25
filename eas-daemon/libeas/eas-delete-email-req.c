/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-delete-email-req.h"


struct _EasDeleteEmailReqPrivate
{
//	EasSyncFolderMsg* syncFolderMsg;
	guint64 accountID;
	gchar* syncKey;
	gchar* server_id;
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

	g_debug("eas_sync_folder_hierarchy_req_finalize++");

	g_free(priv->syncKey);
	g_free(priv->server_id);
	
	g_debug("eas_sync_folder_hierarchy_req_finalize--");
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
eas_delete_email_req_Activate (EasDeleteEmailReq *self)
{
	/* TODO: Add public function implementation here */
}

EasDeleteEmailReq *eas_delete_email_req_new (guint64 accountId, const gchar *syncKey, const gchar *serverId, EFlag *flag)
{
	g_debug("eas_delete_email_req_new++");

	EasDeleteEmailReq* self = g_object_new (EAS_TYPE_DELETE_EMAIL_REQ, NULL);
	EasDeleteEmailReqPrivate *priv = self->priv;
	
	g_assert(syncKey);
	
	priv->syncKey = g_strdup(syncKey);
	priv->server_id = g_strdup(serverId);
	priv->accountID = accountId;
//	eas_request_base_SetFlag(&self->parent_instance, flag);

	g_debug("eas_delete_email_req_new--");
	return self;
}
