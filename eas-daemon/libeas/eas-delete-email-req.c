/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-delete-email-req.h"



G_DEFINE_TYPE (EasDeleteEmailReq, eas_delete_email_req, EAS_TYPE_REQUEST_BASE);

static void
eas_delete_email_req_init (EasDeleteEmailReq *object)
{
	/* TODO: Add initialization code here */
}

static void
eas_delete_email_req_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_delete_email_req_parent_class)->finalize (object);
}

static void
eas_delete_email_req_class_init (EasDeleteEmailReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	object_class->finalize = eas_delete_email_req_finalize;
}


void
eas_delete_email_req_Activate (EasDeleteEmailReq *self, const gchar *syncKey, const gchar *serverId, EFlag *flag)
{
	/* TODO: Add public function implementation here */
}
