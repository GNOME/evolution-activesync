/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eas-send-email-req.h"



G_DEFINE_TYPE (EasSendEmailReq, eas_send_email_req, G_TYPE_OBJECT);

#define EAS_SEND_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqPrivate))

struct _EasSendEmailReqPrivate
{
	// TODO
};

static void
eas_send_email_req_init (EasSendEmailReq *object)
{
	/* TODO: Add initialization code here */
	EasSendEmailReqPrivate *priv;

	object->priv = priv = EAS_SEND_EMAIL_REQ_PRIVATE(object);

	g_debug("eas_send_email_req_init++");

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_SEND_EMAIL);

	g_debug("eas_send_email_req_init--");	
}

static void
eas_send_email_req_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	EasSendEmailReq *req = (EasSendEmailReq *)object;
	
	EasSendEmailReqPrivate *priv = req->priv;	
	g_free (priv);
	req->priv = NULL;

	G_OBJECT_CLASS (eas_send_email_req_parent_class)->finalize (object);
}

static void
eas_send_email_req_class_init (EasSendEmailReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_send_email_req_finalize;
}

