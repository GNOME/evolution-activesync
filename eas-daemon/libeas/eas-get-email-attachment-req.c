/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 */

#include "eas-connection-errors.h"
#include "eas-get-email-attachment-msg.h"
#include "eas-get-email-attachment-req.h"


struct _EasGetEmailAttachmentReqPrivate
{
	EasGetEmailAttachmentMsg* emailAttachmentMsg;
    guint64 accountUid;
    gchar *fileReference; 
    gchar *mimeDirectory; 
	GError *error;
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

	priv->error = NULL;
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

	if(priv->error)
	{
		g_clear_error(&priv->error);
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
	void *tmp = parent_class;
	tmp = object_class;
	
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

gboolean
eas_get_email_attachment_req_Activate (EasGetEmailAttachmentReq* self, GError** error)
{
	gboolean ret;
	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	
	g_debug("eas_get_email_attachment_req_Activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
	priv->emailAttachmentMsg = eas_get_email_attachment_msg_new ( priv->fileReference, priv->mimeDirectory);
	doc = eas_get_email_attachment_msg_build_message (priv->emailAttachmentMsg);
	if(!doc)
	{
		ret = FALSE;
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));		
		goto finish;
	}	

	ret = eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
	                            "ItemOperations", 
	                            doc, 
	                            (struct _EasRequestBase *)self, 
	                            error);
finish:
	if(!ret)
	{
		g_assert(error == NULL || *error != NULL);
	}
	g_debug("eas_get_email_attachment_req_Activate--");	
	return ret;
}


void eas_get_email_attachment_req_MessageComplete (EasGetEmailAttachmentReq* self, xmlDoc *doc, GError* error_in)
{
	gboolean ret;
	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	GError *error = NULL;
	
	g_debug("eas_get_email_attachment_req_MessageComplete++");
	
	// if an error occurred, store it and signal daemon
	if(error_in)
	{
		priv->error = error_in;
		goto finish;
	}	

	ret = eas_get_email_attachment_msg_parse_response (priv->emailAttachmentMsg, doc, &error);
	xmlFree(doc);
	if(!ret)
	{
		self->priv->error = error; 
		goto finish;		
	}

finish:	
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));

	g_debug("eas_get_email_attachment_req_MessageComplete--");	
}

gboolean
eas_get_email_attachment_req_ActivateFinish (EasGetEmailAttachmentReq* self, GError **error)
{
	EasGetEmailAttachmentReqPrivate *priv = self->priv;

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
	g_debug("eas_get_email_attachment_req_ActivateFinish++");
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);	

	if(priv->error != NULL)// propogate any preceding error
	{
		/* store priv->error in error, if error != NULL,
		* otherwise call g_error_free() on priv->error
		*/
		g_propagate_error (error, priv->error);	
		priv->error = NULL;

		return FALSE;
	}	
	
	g_debug("eas_get_email_attachment_req_ActivateFinish--");

	return TRUE;
}

