/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 */
 
#include "eas-get-email-attachment-msg.h"
#include "eas-get-email-attachment-req.h"


struct _EasGetEmailAttachmentReqPrivate
{
	EasGetEmailAttachmentMsg* emailAttachmentMsg;
    guint64 accountUid;
    gchar *fileReference; 
    gchar *mimeDirectory; 
};

#define EAS_GET_EMAIL_ATTACHMENT_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReqPrivate))



G_DEFINE_TYPE (EasGetEmailAttachmentReq, eas_get_email_attachment_req, EAS_TYPE_REQUEST_BASE);

static void
eas_get_email_attachment_req_init (EasGetEmailAttachmentReq *object)
{
	EasGetEmailAttachmentReqPrivate* priv;
	g_debug("eas_get_email_attachment_req_init++");
	object->priv = priv = EAS_GET_EMAIL_ATTACHMENT_REQ_PRIVATE(object);
	
	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_GET_EMAIL_ATTACHMENT);

	priv->emailAttachmentMsg = NULL;
    priv->accountUid = 0;
    priv->fileReference = NULL; 
    priv->mimeDirectory = NULL; 	
	g_debug("eas_get_email_attachment_req_init--");
}

static void
eas_get_email_attachment_req_finalize (GObject *object)
{
	EasGetEmailAttachmentReq* self = (EasGetEmailAttachmentReq *)object;
	EasGetEmailAttachmentReqPrivate *priv = self->priv;

	g_debug("eas_get_email_attachment_req_finalize++");
	
	if (priv->emailAttachmentMsg)
	{
		g_object_unref(priv->emailAttachmentMsg);
	}

	g_free(priv->fileReference);
	g_free(priv->mimeDirectory);

	G_OBJECT_CLASS (eas_get_email_attachment_req_parent_class)->finalize (object);
	g_debug("eas_get_email_attachment_req_finalize--");
}

static void
eas_get_email_attachment_req_class_init (EasGetEmailAttachmentReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);
	
	g_debug("eas_get_email_attachment_req_class_init++");
	g_type_class_add_private (klass, sizeof (EasGetEmailAttachmentReqPrivate));

	object_class->finalize = eas_get_email_attachment_req_finalize;

	g_debug("eas_get_email_attachment_req_class_init--");	
}

EasGetEmailAttachmentReq* 
eas_get_email_attachment_req_new (const guint64 account_uid, 
                            const gchar *file_reference,
                            const gchar *mime_directory,
                            EFlag *flag)
{
	EasGetEmailAttachmentReq* req = NULL;
	EasGetEmailAttachmentReqPrivate *priv = NULL;
	
	g_debug("eas_get_email_attachment_req_new++");

	req = g_object_new(EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, NULL);
	priv = req->priv;
	
	priv->accountUid = account_uid;
	priv->fileReference = g_strdup(file_reference); 
	priv->mimeDirectory = g_strdup(mime_directory);
	eas_request_base_SetFlag(&req->parent_instance, flag);

	g_debug("eas_get_email_attachment_req_new--");
	return req;
}
                   
void eas_get_email_attachment_req_Activate (EasGetEmailAttachmentReq* self, GError** error)
{
	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	
	g_debug("eas_get_email_attachment_req_Activate++");
	
	priv->emailAttachmentMsg = eas_get_email_attachment_msg_new ( priv->fileReference, priv->mimeDirectory);
	doc = eas_get_email_attachment_msg_build_message (priv->emailAttachmentMsg);

	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
	                            "ItemOperations", 
	                            doc, 
	                            (struct _EasRequestBase *)self, 
	                            error);
	
	g_debug("eas_get_email_attachment_req_Activate--");
}

void eas_get_email_attachment_req_MessageComplete (EasGetEmailAttachmentReq* self, xmlDoc *doc, GError** error)
{

	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	
	g_debug("eas_get_email_attachment_req_MessageComplete++");

	eas_get_email_attachment_msg_parse_response (priv->emailAttachmentMsg, doc, error);
	
	xmlFree(doc);

	g_debug("eas_get_email_attachment_req_MessageComplete--");
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
}

void eas_get_email_attachment_req_ActivateFinish (EasGetEmailAttachmentReq* self, GError **error)
{
	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	g_debug("eas_get_email_attachment_req_ActivateFinish++");
	/* TODO: Add public function implementation here */
	g_debug("eas_get_email_attachment_req_ActivateFinish--");
}

