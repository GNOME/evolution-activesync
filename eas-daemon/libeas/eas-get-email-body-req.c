/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */
 
#include "eas-get-email-body-msg.h"
#include "eas-get-email-body-req.h"


struct _EasGetEmailBodyReqPrivate
{
	EasGetEmailBodyMsg* emailBodyMsg;
	guint64 accountUid;
	gchar* serverId;
	gchar* collectionId;
	gchar* mimeDirectory;
};

#define EAS_GET_EMAIL_BODY_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_BODY_REQ, EasGetEmailBodyReqPrivate))



G_DEFINE_TYPE (EasGetEmailBodyReq, eas_get_email_body_req, EAS_TYPE_REQUEST_BASE);

static void
eas_get_email_body_req_init (EasGetEmailBodyReq *object)
{
	EasGetEmailBodyReqPrivate* priv;
	g_debug("eas_get_email_body_req_init++");
	object->priv = priv = EAS_GET_EMAIL_BODY_REQ_PRIVATE(object);
	
	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_GET_EMAIL_BODY);

	priv->emailBodyMsg = NULL;
	g_debug("eas_get_email_body_req_init--");
}

static void
eas_get_email_body_req_finalize (GObject *object)
{
	EasGetEmailBodyReq* self = (EasGetEmailBodyReq *)object;
	EasGetEmailBodyReqPrivate *priv = self->priv;

	g_debug("eas_get_email_body_req_finalize++");
	
	if (priv->emailBodyMsg)
	{
		g_object_unref(priv->emailBodyMsg);
	}

	g_free(priv->serverId);
	g_free(priv->collectionId);
	g_free(priv->mimeDirectory);

	G_OBJECT_CLASS (eas_get_email_body_req_parent_class)->finalize (object);
	g_debug("eas_get_email_body_req_finalize--");
}

static void
eas_get_email_body_req_class_init (EasGetEmailBodyReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);
	
	g_debug("eas_get_email_body_req_class_init++");

	g_type_class_add_private (klass, sizeof (EasGetEmailBodyReqPrivate));

	object_class->finalize = eas_get_email_body_req_finalize;
	g_debug("eas_get_email_body_req_class_init--");
}


EasGetEmailBodyReq*
eas_get_email_body_req_new (const guint64 account_uid, 
                            const gchar *collection_id, 
                            const gchar *server_id, 
                            const gchar *mime_directory,
                            EFlag* flag)
{
	EasGetEmailBodyReq* req = NULL;
	EasGetEmailBodyReqPrivate *priv = NULL;
	
	g_debug("eas_get_email_body_req_new++");

	req = g_object_new(EAS_TYPE_GET_EMAIL_BODY_REQ, NULL);
	priv = req->priv;

	priv->accountUid = account_uid;
	priv->collectionId = g_strdup(collection_id);
	priv->serverId = g_strdup(server_id);
	priv->mimeDirectory = g_strdup(mime_directory);
	eas_request_base_SetFlag(&req->parent_instance, flag);

	g_debug("eas_get_email_body_req_new--");
	return req;
}

void
eas_get_email_body_req_Activate (EasGetEmailBodyReq* self, GError** error)
{
	EasGetEmailBodyReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	
	g_debug("eas_get_email_body_req_Activate++");
	
	priv->emailBodyMsg = eas_get_email_body_msg_new (priv->serverId, priv->collectionId, priv->mimeDirectory);
	doc = eas_get_email_body_msg_build_message (priv->emailBodyMsg);

	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
	                            "ItemOperations", 
	                            doc, 
	                            (struct _EasRequestBase *)self, 
	                            error);
	
	g_debug("eas_get_email_body_req_Activate--");
}

void
eas_get_email_body_req_MessageComplete (EasGetEmailBodyReq* self, xmlDoc *doc, GError** error)
{
	EasGetEmailBodyReqPrivate *priv = self->priv;
	
	g_debug("eas_get_email_body_req_MessageComplete++");

	eas_get_email_body_msg_parse_response (priv->emailBodyMsg, doc, error);
	
	xmlFree(doc);

	g_debug("eas_get_email_body_req_MessageComplete--");
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
}

void
eas_get_email_body_req_ActivateFinish (EasGetEmailBodyReq* self, GError** error)
{
	EasGetEmailBodyReqPrivate *priv = self->priv;
	g_debug("eas_get_email_body_req_ActivateFinish++");
	/* TODO: Add public function implementation here */
	g_debug("eas_get_email_body_req_ActivateFinish--");
}
