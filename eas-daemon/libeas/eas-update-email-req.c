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


#include "eas-utils.h"
#include "eas-sync-msg.h"
#include "eas-update-email-req.h"
#include "serialise_utils.h"

G_DEFINE_TYPE (EasUpdateEmailReq, eas_update_email_req, EAS_TYPE_REQUEST_BASE);

#define EAS_UPDATE_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReqPrivate))

struct _EasUpdateEmailReqPrivate {
	EasSyncMsg* sync_msg;
	gchar* account_id;
	gchar* sync_key;
	gchar* folder_id;
	gchar** serialised_email_array;
};

static void
eas_update_email_req_init (EasUpdateEmailReq *object)
{
	/* initialization code */
	EasUpdateEmailReqPrivate *priv;

	g_debug ("eas_update_email_req_init++");

	object->priv = priv = EAS_UPDATE_EMAIL_REQ_PRIVATE (object);

	priv->sync_msg = NULL;
	priv->account_id = NULL;
	priv->sync_key = NULL;
	priv->folder_id = NULL;
	priv->serialised_email_array = NULL;

	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_UPDATE_MAIL);

	g_debug ("eas_update_email_req_init++");

	return;

}

static void
eas_update_email_req_dispose (GObject *object)
{
	EasUpdateEmailReq *req = EAS_UPDATE_EMAIL_REQ (object);
	EasUpdateEmailReqPrivate *priv = req->priv;

	g_debug ("eas_update_email_req_dispose++");

	if (priv->sync_msg) {
		g_object_unref (priv->sync_msg);
		priv->sync_msg = NULL;
	}

	G_OBJECT_CLASS (eas_update_email_req_parent_class)->dispose (object);

	g_debug ("eas_update_email_req_dispose--");
}

static void
eas_update_email_req_finalize (GObject *object)
{
	EasUpdateEmailReq *req = EAS_UPDATE_EMAIL_REQ (object);
	EasUpdateEmailReqPrivate *priv = req->priv;

	g_debug ("eas_update_email_req_finalize++");
	g_free (priv->account_id);
	g_free (priv->sync_key);
	g_free (priv->folder_id);
	g_strfreev (priv->serialised_email_array);

	G_OBJECT_CLASS (eas_update_email_req_parent_class)->finalize (object);

	g_debug ("eas_update_email_req_finalize--");
}

static void
eas_update_email_req_class_init (EasUpdateEmailReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_update_email_req_MessageComplete;

	g_type_class_add_private (klass, sizeof (EasUpdateEmailReqPrivate));

	object_class->finalize = eas_update_email_req_finalize;
	object_class->dispose = eas_update_email_req_dispose;

	g_debug ("eas_update_email_req_class_init--");
}

// TODO - update this to take a GSList of serialised emails? rem to copy the list
EasUpdateEmailReq *eas_update_email_req_new (const gchar* account_id,
					     const gchar *sync_key,
					     const gchar *folder_id,
					     const gchar **serialised_email_array,
					     DBusGMethodInvocation *context)
{
	EasUpdateEmailReq* self = g_object_new (EAS_TYPE_UPDATE_EMAIL_REQ, NULL);
	EasUpdateEmailReqPrivate *priv = self->priv;
	guint i;
	guint num_serialised_emails = 0;

	g_debug ("eas_update_email_req_new++");

	g_assert (sync_key);
	g_assert (folder_id);
	g_assert (serialised_email_array);

	num_serialised_emails = array_length (serialised_email_array);
	priv->sync_key = g_strdup (sync_key);
	priv->folder_id = g_strdup (folder_id);

	priv->serialised_email_array = g_malloc0 ( (num_serialised_emails + 1) * sizeof (gchar*)); // allow for null terminate
	if (!priv->serialised_email_array) {
		g_warning ("Failed to allocate memory!");
		goto cleanup;
	}

	for (i = 0; i < num_serialised_emails; i++) {
		priv->serialised_email_array[i] = g_strdup (serialised_email_array[i]);
	}

	priv->serialised_email_array[i] = NULL;

	priv->account_id = g_strdup (account_id);

	eas_request_base_SetContext (&self->parent_instance, context);

cleanup:
	if (!priv->serialised_email_array) {
		g_free (priv->sync_key);
		g_free (priv->folder_id);
		if (self) {
			g_object_unref (self);
			self = NULL;
		}
	}

	g_debug ("eas_update_email_req_new--");
	return self;
}

gboolean
eas_update_email_req_Activate (EasUpdateEmailReq *self, GError** error)
{
	gboolean ret;
	EasUpdateEmailReqPrivate *priv = self->priv;
	xmlDoc *doc;
	GSList *update_emails = NULL;   // sync msg expects a list, we have an array
	guint i = 0;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	EasConnection *conn = eas_request_base_GetConnection (parent);

	g_debug ("eas_update_email_req_Activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	while (priv->serialised_email_array[i]) {
		g_debug ("append email to list");
		update_emails = g_slist_append (update_emails, priv->serialised_email_array[i]);
		i++;
	}

	//create sync msg object
	priv->sync_msg = eas_sync_msg_new (priv->sync_key, conn, priv->folder_id, EAS_ITEM_MAIL);

	g_debug ("build messsage");
	//build request msg
	doc = eas_sync_msg_build_message (priv->sync_msg, 0, FALSE, NULL, update_emails, NULL);
	g_slist_free (update_emails);
	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		ret = FALSE;
		g_object_unref (priv->sync_msg);
		goto finish;
	}

	g_debug ("send message");
	ret = eas_request_base_SendRequest (parent,
					    "Sync",
					    doc, // full transfer
	                                    FALSE,
					    error);

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_update_email_req_Activate--");
	return ret;
}


gboolean
eas_update_email_req_MessageComplete (EasUpdateEmailReq *self, xmlDoc* doc, GError* error_in)
{
	gboolean ret = TRUE;
	GError *error = NULL;
	EasUpdateEmailReqPrivate *priv = self->priv;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	GSList* failed_updates = NULL;
	gchar **ret_failed_updates_array = NULL;

	g_debug ("eas_update_email_req_MessageComplete++");

	// if an error occurred, store it and signal daemon
	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_sync_msg_parse_response (priv->sync_msg, doc, &error);
	if (!ret) {
		g_assert (error != NULL);
	}

	// get list of flattened emails with status codes
	failed_updates = eas_sync_msg_get_update_responses (priv->sync_msg);

	if (failed_updates) {
		// create array of flattened emails to pass over dbus
		ret = build_serialised_email_info_array (&ret_failed_updates_array, failed_updates, &error);
	}

finish:
	xmlFreeDoc (doc);
	if (!ret) {
		dbus_g_method_return_error (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	} else {
		dbus_g_method_return (eas_request_base_GetContext (parent),
				      eas_sync_msg_get_syncKey (priv->sync_msg),
				      ret_failed_updates_array);
	}

	g_strfreev (ret_failed_updates_array);
	ret_failed_updates_array = NULL;

	g_debug ("eas_update_email_req_MessageComplete--");
	return TRUE;
}



