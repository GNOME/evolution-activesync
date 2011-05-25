/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-get-email-body-msg.h"

struct _EasGetEmailBodyMsgPrivate
{
	gint some_data;
};

#define EAS_GET_EMAIL_BODY_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgPrivate))



G_DEFINE_TYPE (EasGetEmailBodyMsg, eas_get_email_body_msg, EAS_TYPE_MSG_BASE);

static void
eas_get_email_body_msg_init (EasGetEmailBodyMsg *object)
{
	EasGetEmailBodyMsgPrivate* priv;

	object->priv = priv = EAS_GET_EMAIL_BODY_MSG_PRIVATE(object);
	
	/* TODO: Add initialization code here */
}

static void
eas_get_email_body_msg_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_get_email_body_msg_parent_class)->finalize (object);
}

static void
eas_get_email_body_msg_class_init (EasGetEmailBodyMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasGetEmailBodyMsgPrivate));

	object_class->finalize = eas_get_email_body_msg_finalize;
}


EasGetEmailBodyMsg*
eas_get_email_body_msg_new (void)
{
	EasGetEmailBodyMsg* self = NULL;

	self = g_object_new(EAS_TYPE_GET_EMAIL_BODY_MSG, NULL);

	/* TODO: Add public function implementation here */
	return self;
}

xmlDoc*
eas_get_email_body_msg_build_message (EasGetEmailBodyMsg* self)
{
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	xmlDoc* doc = NULL;
	/* TODO: Add public function implementation here */
	return doc;
}

void
eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, xmlDoc *doc)
{
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	/* TODO: Add public function implementation here */
}
