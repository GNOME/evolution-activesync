/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-get-email-body-req.h"
#include "eas-get-email-body-msg.h"

struct _EasGetEmailBodyReqPrivate
{
	EasGetEmailBodyMsg* emailBodyMsg;
};

#define EAS_GET_EMAIL_BODY_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_BODY_REQ, EasGetEmailBodyReqPrivate))



G_DEFINE_TYPE (EasGetEmailBodyReq, eas_get_email_body_req, EAS_TYPE_REQUEST_BASE);

static void
eas_get_email_body_req_init (EasGetEmailBodyReq *object)
{
	EasGetEmailBodyReqPrivate* priv;
	object->priv = priv = EAS_GET_EMAIL_BODY_REQ_PRIVATE(object);
	
	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_GET_EMAIL_BODY);

	priv->emailBodyMsg = NULL;
}

static void
eas_get_email_body_req_finalize (GObject *object)
{
	EasGetEmailBodyReq* self = (EasGetEmailBodyReq *)object;
	EasGetEmailBodyReqPrivate *priv = self->priv;

	if (priv->emailBodyMsg)
	{
		g_object_unref(priv->emailBodyMsg);
	}

	G_OBJECT_CLASS (eas_get_email_body_req_parent_class)->finalize (object);
}

static void
eas_get_email_body_req_class_init (EasGetEmailBodyReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasGetEmailBodyReqPrivate));

	object_class->finalize = eas_get_email_body_req_finalize;
}


EasGetEmailBodyReq*
eas_get_email_body_req_new (void)
{
	EasGetEmailBodyReq* req = NULL;

	req = g_object_new(EAS_TYPE_GET_EMAIL_BODY_REQ, NULL);
	
	return req;
}

void
eas_get_email_body_req_Activate (EasGetEmailBodyReq* self)
{
	EasGetEmailBodyReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	
	/* TODO: Add public function implementation here */

	priv->emailBodyMsg = eas_get_email_body_msg_new ("syncKey", "email_id");
	doc = eas_get_email_body_msg_build_message (priv->emailBodyMsg);

	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self);
}

void
eas_get_email_body_req_MessageComplete (EasGetEmailBodyReq* self, xmlDoc *doc)
{
	EasGetEmailBodyReqPrivate *priv = self->priv;

	eas_get_email_body_msg_parse_response (priv->emailBodyMsg, doc);
	 
	xmlFree(doc);

	/* TODO: Add public function implementation here */
}

void
eas_get_email_body_req_ActivateFinish (EasGetEmailBodyReq* self)
{
	EasGetEmailBodyReqPrivate *priv = self->priv;
	/* TODO: Add public function implementation here */
}
