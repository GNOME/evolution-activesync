/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */
/*
 * ActiveSync core protocol library
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "eas-connection-errors.h"
#include "eas-get-email-attachment-msg.h"
#include "eas-get-email-attachment-req.h"


struct _EasGetEmailAttachmentReqPrivate {
	EasGetEmailAttachmentMsg* emailAttachmentMsg;
	gchar* accountUid;
	gchar *fileReference;
	gchar *mimeDirectory;
};

#define EAS_GET_EMAIL_ATTACHMENT_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReqPrivate))



G_DEFINE_TYPE (EasGetEmailAttachmentReq, eas_get_email_attachment_req, EAS_TYPE_REQUEST_BASE);

static void
eas_get_email_attachment_req_init (EasGetEmailAttachmentReq *object)
{
	EasGetEmailAttachmentReqPrivate* priv;
	g_debug ("eas_get_email_attachment_req_init++");
	object->priv = priv = EAS_GET_EMAIL_ATTACHMENT_REQ_PRIVATE (object);

	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_GET_EMAIL_ATTACHMENT);

	priv->emailAttachmentMsg = NULL;
	priv->accountUid = NULL;
	priv->fileReference = NULL;
	priv->mimeDirectory = NULL;
	g_debug ("eas_get_email_attachment_req_init--");
}

static void
eas_get_email_attachment_req_dispose (GObject *object)
{
	EasGetEmailAttachmentReq* self = EAS_GET_EMAIL_ATTACHMENT_REQ (object);
	EasGetEmailAttachmentReqPrivate *priv = self->priv;

	g_debug ("eas_get_email_attachment_req_dispose++");

	if (priv->emailAttachmentMsg) {
		g_object_unref (priv->emailAttachmentMsg);
		priv->emailAttachmentMsg = NULL;
	}

	G_OBJECT_CLASS (eas_get_email_attachment_req_parent_class)->dispose (object);
	g_debug ("eas_get_email_attachment_req_dispose--");
}

static void
eas_get_email_attachment_req_finalize (GObject *object)
{
	EasGetEmailAttachmentReq* self = EAS_GET_EMAIL_ATTACHMENT_REQ (object);
	EasGetEmailAttachmentReqPrivate *priv = self->priv;

	g_debug ("eas_get_email_attachment_req_finalize++");

	g_free (priv->fileReference);
	g_free (priv->mimeDirectory);
	g_free (priv->accountUid);

	G_OBJECT_CLASS (eas_get_email_attachment_req_parent_class)->finalize (object);
	g_debug ("eas_get_email_attachment_req_finalize--");
}

static void
eas_get_email_attachment_req_class_init (EasGetEmailAttachmentReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	g_debug ("eas_get_email_attachment_req_class_init++");
	g_type_class_add_private (klass, sizeof (EasGetEmailAttachmentReqPrivate));

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_get_email_attachment_req_MessageComplete;

	object_class->finalize = eas_get_email_attachment_req_finalize;
	object_class->dispose = eas_get_email_attachment_req_dispose;

	g_debug ("eas_get_email_attachment_req_class_init--");
}

EasGetEmailAttachmentReq*
eas_get_email_attachment_req_new (const gchar* account_uid,
				  const gchar *file_reference,
				  const gchar *mime_directory,
				  DBusGMethodInvocation *context)
{
	EasGetEmailAttachmentReq* req = NULL;
	EasGetEmailAttachmentReqPrivate *priv = NULL;

	g_debug ("eas_get_email_attachment_req_new++");

	req = g_object_new (EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, NULL);
	priv = req->priv;

	priv->accountUid = g_strdup (account_uid);
	priv->fileReference = g_strdup (file_reference);
	priv->mimeDirectory = g_strdup (mime_directory);
	eas_request_base_SetContext (&req->parent_instance, context);

	g_debug ("eas_get_email_attachment_req_new--");
	return req;
}

gboolean
eas_get_email_attachment_req_Activate (EasGetEmailAttachmentReq* self, GError** error)
{
	gboolean ret;
	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_get_email_attachment_req_Activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv->emailAttachmentMsg = eas_get_email_attachment_msg_new (priv->fileReference, priv->mimeDirectory);
	doc = eas_get_email_attachment_msg_build_message (priv->emailAttachmentMsg);
	if (!doc) {
		ret = FALSE;
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		goto finish;
	}

	ret = eas_request_base_SendRequest (parent,
					    "ItemOperations",
					    doc,
	                                    FALSE,
					    error);
finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_get_email_attachment_req_Activate--");
	return ret;
}


gboolean eas_get_email_attachment_req_MessageComplete (EasGetEmailAttachmentReq* self, xmlDoc *doc, GError* error_in)
{
	gboolean ret = TRUE;
	EasGetEmailAttachmentReqPrivate *priv = self->priv;
	GError *error = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_get_email_attachment_req_MessageComplete++");

	// if an error occurred, store it and signal daemon
	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_get_email_attachment_msg_parse_response (priv->emailAttachmentMsg, doc, &error);
	if (!ret) {
		g_assert (error != NULL);
		goto finish;
	}

	if (eas_request_base_UseMultipart (parent)) {
		gchar * data = NULL;
		data = eas_connection_get_multipartdata (eas_request_base_GetConnection (parent), 0);
		if (!eas_get_email_attachment_msg_write_file (priv->emailAttachmentMsg, data, &error)) {
			ret = FALSE;
		}
	}

finish:
	xmlFreeDoc (doc);
	if (!ret) {
		g_assert (error != NULL);
		dbus_g_method_return_error (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	} else {
		dbus_g_method_return (eas_request_base_GetContext (parent));
	}
	g_debug ("eas_get_email_attachment_req_MessageComplete--");
	return TRUE;
}


