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
#include "eas-get-item-estimate-msg.h"
#include "eas-get-item-estimate-req.h"
#include "serialise_utils.h"

G_DEFINE_TYPE (EasGetItemEstimateReq, eas_get_item_estimate_req, EAS_TYPE_REQUEST_BASE);

#define EAS_GET_ITEM_ESTIMATE_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_ITEM_ESTIMATE_REQ, EasGetItemEstimateReqPrivate))

struct _EasGetItemEstimateReqPrivate {
	EasGetItemEstimateMsg* get_item_estimate_msg;
	gchar* sync_key;
	gchar* folder_id;
};

static void
eas_get_item_estimate_req_init (EasGetItemEstimateReq *object)
{
	/* initialization code */
	EasGetItemEstimateReqPrivate *priv;

	g_debug ("eas_get_item_estimate_req_init++");

	object->priv = priv = EAS_GET_ITEM_ESTIMATE_REQ_PRIVATE (object);

	priv->get_item_estimate_msg = NULL;
	priv->sync_key = NULL;
	priv->folder_id = NULL;

	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_GET_ITEM_ESTIMATE);

	g_debug ("eas_get_item_estimate_req_init++");

	return;

}

static void
eas_get_item_estimate_req_dispose (GObject *object)
{
	EasGetItemEstimateReq *req = EAS_GET_ITEM_ESTIMATE_REQ (object);
	EasGetItemEstimateReqPrivate *priv = req->priv;

	g_debug ("eas_get_item_estimate_req_dispose++");

	if (priv->get_item_estimate_msg) {
		g_object_unref (priv->get_item_estimate_msg);
		priv->get_item_estimate_msg = NULL;
	}

	G_OBJECT_CLASS (eas_get_item_estimate_req_parent_class)->dispose (object);

	g_debug ("eas_get_item_estimate_req_dispose--");
}

static void
eas_get_item_estimate_req_finalize (GObject *object)
{
	EasGetItemEstimateReq *req = EAS_GET_ITEM_ESTIMATE_REQ (object);
	EasGetItemEstimateReqPrivate *priv = req->priv;

	g_debug ("eas_get_item_estimate_req_finalize++");
	g_free (priv->sync_key);
	g_free (priv->folder_id);

	G_OBJECT_CLASS (eas_get_item_estimate_req_parent_class)->finalize (object);

	g_debug ("eas_get_item_estimate_req_finalize--");
}

static void
eas_get_item_estimate_req_class_init (EasGetItemEstimateReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_get_item_estimate_req_MessageComplete;

	g_type_class_add_private (klass, sizeof (EasGetItemEstimateReqPrivate));

	object_class->finalize = eas_get_item_estimate_req_finalize;
	object_class->dispose = eas_get_item_estimate_req_dispose;

	g_debug ("eas_get_item_estimate_req_class_init--");
}


EasGetItemEstimateReq *eas_get_item_estimate_req_new (const gchar *sync_key,
						      const gchar *folder_id,
						      DBusGMethodInvocation *context)
{
	EasGetItemEstimateReq* self = g_object_new (EAS_TYPE_GET_ITEM_ESTIMATE_REQ, NULL);
	EasGetItemEstimateReqPrivate *priv = self->priv;

	g_debug ("eas_get_item_estimate_req_new++");

	g_assert (sync_key);
	g_assert (folder_id);

	priv->sync_key = g_strdup (sync_key);
	priv->folder_id = g_strdup (folder_id);

	eas_request_base_SetContext (&self->parent_instance, context);

	g_debug ("eas_get_item_estimate_req_new--");
	return self;
}

gboolean
eas_get_item_estimate_req_Activate (EasGetItemEstimateReq *self, GError** error)
{
	gboolean ret;
	EasGetItemEstimateReqPrivate *priv = self->priv;
	xmlDoc *doc;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	EasConnection *conn = eas_request_base_GetConnection (EAS_REQUEST_BASE (self));
	g_debug ("eas_get_item_estimate_req_Activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	//create msg object
	priv->get_item_estimate_msg = eas_get_item_estimate_msg_new (conn, priv->sync_key, priv->folder_id);

	g_debug ("build messsage");
	//build request msg
	doc = eas_get_item_estimate_msg_build_message (priv->get_item_estimate_msg);

	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		ret = FALSE;
		g_object_unref (priv->get_item_estimate_msg);
		goto finish;
	}

	g_debug ("send message");
	ret = eas_request_base_SendRequest (parent,
					    "GetItemEstimate",
					    doc, // full transfer
	                                    FALSE,
					    error);
finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_get_item_estimate_req_Activate--");
	return ret;
}


gboolean
eas_get_item_estimate_req_MessageComplete (EasGetItemEstimateReq *self, xmlDoc* doc, GError* error_in)
{
	gboolean ret = TRUE;
	GError *error = NULL;
	EasGetItemEstimateReqPrivate *priv = self->priv;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	guint ret_estimate = 0;

	g_debug ("eas_get_item_estimate_req_MessageComplete++");

	// if an error occurred, store it and signal daemon
	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_get_item_estimate_msg_parse_response (priv->get_item_estimate_msg, doc, &error);
	if (!ret) {
		g_assert (error != NULL);
	}

	// get the estimate
	ret_estimate = eas_get_item_estimate_get_estimate (priv->get_item_estimate_msg);

finish:
	xmlFreeDoc (doc);
	if (!ret) {
		dbus_g_method_return_error (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	} else {
		dbus_g_method_return (eas_request_base_GetContext (parent), ret_estimate);
	}

	g_debug ("eas_get_item_estimate_req_MessageComplete--");
	return TRUE;
}



