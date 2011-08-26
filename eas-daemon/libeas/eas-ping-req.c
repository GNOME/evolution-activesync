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
#include "eas-ping-msg.h"
#include "eas-ping-req.h"
#include "eas-connection-errors.h"

G_DEFINE_TYPE (EasPingReq, eas_ping_req, EAS_TYPE_REQUEST_BASE);

#define EAS_PING_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PING_REQ, EasPingReqPrivate))

struct _EasPingReqPrivate {
	EasPingMsg* ping_msg;
	gchar* account_id;
	gchar* heartbeat;
	GSList* folder_array;
	EasPingReqState state;
};

static void
eas_ping_req_init (EasPingReq *object)
{
	/* initialization code */
	EasPingReqPrivate *priv;

	g_debug ("eas_ping_req_init++");

	object->priv = priv = EAS_PING_REQ_PRIVATE (object);

	priv->ping_msg = NULL;
	priv->account_id = NULL;
	priv->heartbeat = NULL;
	priv->folder_array = NULL;

	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_PING);

	g_debug ("eas_ping_req_init++");

	return;

}

static void
eas_ping_req_dispose (GObject *object)
{
	EasPingReq *req = EAS_PING_REQ (object);
	EasPingReqPrivate *priv = req->priv;
	if (priv->ping_msg) {
		g_object_unref (priv->ping_msg);
		priv->ping_msg = NULL;
	}
	G_OBJECT_CLASS (eas_ping_req_parent_class)->dispose (object);
}

static void
eas_ping_req_finalize (GObject *object)
{
	/* deinitalization code */
	EasPingReq *req = EAS_PING_REQ (object);
	EasPingReqPrivate *priv = req->priv;

	g_debug ("eas_ping_req_finalize++");
	g_free (priv->account_id);
	g_free (priv->heartbeat);

	g_slist_free (priv->folder_array);

	G_OBJECT_CLASS (eas_ping_req_parent_class)->finalize (object);

	g_debug ("eas_ping_req_finalize--");
}

static void
eas_ping_req_class_init (EasPingReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	g_debug ("eas_ping_req_class_init++");

	g_type_class_add_private (klass, sizeof (EasPingReqPrivate));
	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_ping_req_MessageComplete;

	object_class->finalize = eas_ping_req_finalize;
	object_class->dispose = eas_ping_req_dispose;

	g_debug ("eas_ping_req_class_init--");
}

EasPingReq *eas_ping_req_new (const gchar* account_id, const gchar *heartbeat, const GSList* folder_list, DBusGMethodInvocation *context)
{
	EasPingReq* self = g_object_new (EAS_TYPE_PING_REQ, NULL);
	EasPingReqPrivate *priv = self->priv;
	guint listCount;
	guint listLen = g_slist_length ( (GSList*) folder_list);
	gchar *server_id = NULL;

	g_debug ("eas_ping_req_new++");
	g_assert (heartbeat);
	g_assert (folder_list);

	priv->heartbeat = g_strdup (heartbeat);

	for (listCount = 0; listCount < listLen; listCount++) {
		server_id = g_slist_nth_data ( (GSList*) folder_list, listCount);
		priv->folder_array = g_slist_append (priv->folder_array, g_strdup (server_id));
	}

	priv->account_id = g_strdup (account_id);

	eas_request_base_SetContext (&self->parent_instance, context);

	g_debug ("eas_ping_req_new--");
	return self;
}

gboolean
eas_ping_req_Activate (EasPingReq *self, GError** error)
{
	gboolean ret;
	EasPingReqPrivate *priv = self->priv;
	xmlDoc *doc;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_ping_req_Activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	//create sync msg object
	priv->ping_msg = eas_ping_msg_new ();

	g_debug ("build messsage");
	//build request msg
	doc = eas_ping_msg_build_message (priv->ping_msg, priv->account_id, priv->heartbeat, priv->folder_array);
	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		ret = FALSE;
		goto finish;
	}

	g_debug ("send message");
	ret = eas_request_base_SendRequest (parent,
					    "Ping",
					    doc, // full transfer
	                    FALSE,
					    error);

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_ping_req_Activate--");
	return ret;
}


gboolean
eas_ping_req_MessageComplete (EasPingReq *self, xmlDoc* doc, GError* error_in)
{
	gboolean ret = TRUE;
	GError *error = NULL;
	EasPingReqPrivate *priv = self->priv;
	gboolean finished = FALSE;
	gchar ** ret_changed_folders_array = NULL;
	GSList *folder_array = NULL;
	EasPingReqState state;
	guint list_length = 0;
	int loop = 0;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_ping_req_MessageComplete++");

	// if an error occurred, store it and signal daemon
	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_ping_msg_parse_response (priv->ping_msg, doc, &error);
	if (!ret) {
		g_assert (error != NULL);
	}
	//TODO: get status message - if status = 1 - re-issue message, if status =
	//      2 signal folder IDs to client.

	state = eas_ping_msg_get_state (priv->ping_msg);
	switch (state) {
	case EasPingReqSendHeartbeat: {
		g_debug ("send message but no body");
		eas_ping_req_Activate (self, &error);
		finished = FALSE;
		ret = TRUE;
	}
	break;
	case EasPingReqNotifyClient: {
		folder_array = eas_ping_msg_get_changed_folders (priv->ping_msg);
		list_length = g_slist_length (folder_array);
		ret_changed_folders_array = g_malloc0 ( (list_length + 1) * sizeof (gchar*));
		if (!ret_changed_folders_array) {
			g_set_error (&error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			finished = TRUE;
			ret = FALSE;
			goto finish;
		}

		for (; loop < list_length; ++loop) {
			ret_changed_folders_array[loop] = g_slist_nth_data (folder_array, loop); // No transfer
		}
		dbus_g_method_return (eas_request_base_GetContext (parent),
				      ret_changed_folders_array);

		g_free (ret_changed_folders_array);
		ret_changed_folders_array = NULL;

		finished = TRUE;
		ret = TRUE;
	}
	break;
	default: {
		ret = FALSE;
		finished = TRUE;
	}
	}

finish:
	xmlFreeDoc (doc);
	if (!ret) {
		g_debug ("eas_ping_req_Return Error");
		dbus_g_method_return_error (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	}

	g_debug ("eas_ping_req_MessageComplete--");
	return finished;
}



