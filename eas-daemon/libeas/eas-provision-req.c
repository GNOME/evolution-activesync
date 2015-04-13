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

#include "eas-provision-req.h"
#include "eas-provision-msg.h"
#include "eas-connection-errors.h"
#include "eas-provision-list.h"

typedef enum {
	EasProvisionStep1 = 0,
	EasProvisionStep2
} EasProvisionState;

struct _EasProvisionReqPrivate {
	EasProvisionMsg* msg;
	EasProvisionState state;
	gboolean internal;

};

#define EAS_PROVISION_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PROVISION_REQ, EasProvisionReqPrivate))



G_DEFINE_TYPE (EasProvisionReq, eas_provision_req, EAS_TYPE_REQUEST_BASE);


static void
eas_provision_req_init (EasProvisionReq *object)
{
	EasProvisionReqPrivate *priv;

	object->priv = priv = EAS_PROVISION_REQ_PRIVATE (object);

	priv->msg = NULL;
	priv->state = EasProvisionStep1;
	
	eas_request_base_SetRequestType (&object->parent_instance, EAS_REQ_PROVISION);
}

static void
eas_provision_req_dispose (GObject *object)
{
	EasProvisionReq *req = EAS_PROVISION_REQ (object);
	EasProvisionReqPrivate *priv = req->priv;

	if (priv->msg) {
		g_object_unref (priv->msg);
		priv->msg = NULL;
	}

	G_OBJECT_CLASS (eas_provision_req_parent_class)->dispose (object);
}


static void
eas_provision_req_finalize (GObject *object)
{
	G_OBJECT_CLASS (eas_provision_req_parent_class)->finalize (object);
}

static void
eas_provision_req_class_init (EasProvisionReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_provision_req_MessageComplete;

	g_type_class_add_private (klass, sizeof (EasProvisionReqPrivate));

	object_class->finalize = eas_provision_req_finalize;
	object_class->dispose = eas_provision_req_dispose;
}


EasProvisionReq*
eas_provision_req_new (gboolean internal,
                       const gchar* policy_status, 
                       const gchar* policy_key, 
                       DBusGMethodInvocation *context)
{
	EasProvisionReq* self = g_object_new (EAS_TYPE_PROVISION_REQ, NULL);
	EasProvisionReqPrivate *priv = EAS_PROVISION_REQ_PRIVATE (self);

	if (self) 
	{
		// Build the message
		priv->msg = eas_provision_msg_new ();
		if (priv->msg) 
		{
			priv->internal = internal;
			eas_provision_msg_set_policy_status (priv->msg, policy_status);
			eas_provision_msg_set_policy_key (priv->msg, policy_key);

			if (policy_status && policy_key)
			{
				priv->state = EasProvisionStep2;
			
			}
			if(!internal)
				eas_request_base_SetContext (&self->parent_instance, context);
		}
		else
		{
			g_object_unref(self);
			self = NULL;
		}
	}

	return self;
}

gboolean
eas_provision_req_Activate (EasProvisionReq* self, GError** error)
{
	gboolean ret = FALSE;
	EasProvisionReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	doc = eas_provision_msg_build_message (priv->msg);
	if (!doc) {
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		goto finish;
	}

	ret = eas_request_base_SendRequest (parent,
					    "Provision",
					    doc, // full transfer
	                                    priv->internal, //if internal then high priority
					    error);
finish:
	return ret;
}

gboolean
eas_provision_req_MessageComplete (EasProvisionReq* self, xmlDoc *doc, GError* error_in)
{
	GError* error = NULL;
	gboolean ret = FALSE;
	gboolean cleanup = FALSE;
	EasProvisionReqPrivate *priv = self->priv;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_provision_req_MessageComplete++");

	// if an error occurred, store it and signal daemon
	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_provision_msg_parse_response (priv->msg, doc, &error);
	if (!ret) {
		g_warning ("Failed to parse provisioning response");
		goto finish;
	}

	switch (priv->state) {
	default: {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADREQUESTSTATE,
			     "Unknown Provision Request state [%d]",
			     priv->state);
		ret = FALSE;
	}
	break;

	// We are receiving the temporary policy key and need to now make a
	// second provision msg and send it using the new data.
	case EasProvisionStep1: {
		if(priv->internal){
			EasProvisionMsg *msg = NULL;

			g_debug ("eas_provision_req_MessageComplete - EasProvisionStep1");

			msg = eas_provision_msg_new ();
			if (!msg) {
				g_set_error (&error,
						EAS_CONNECTION_ERROR,
						EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
						"Failed to create message for provision request");
				ret = FALSE;
				goto finish;
			}
			
			eas_provision_msg_set_policy_status (msg, eas_provision_msg_get_policy_status (priv->msg));
			eas_provision_msg_set_policy_key (msg, eas_provision_msg_get_policy_key (priv->msg));

			eas_connection_set_policy_key (eas_request_base_GetConnection (&self->parent_instance),
					      eas_provision_msg_get_policy_key (priv->msg));

			g_object_unref (priv->msg);
			priv->msg = msg;

			priv->state = EasProvisionStep2;

			ret = eas_provision_req_Activate (self, &error);
			if (!ret) {
				g_warning ("Failed to activate provision request");
				goto finish;
			}
		}
		else{
			gchar* serialised_provision_list = NULL;
			EasProvisionList* list = NULL;
			g_debug("eas_provision_req_MessageComplete - EasProvisionStep1");

			list = eas_provision_msg_get_provision_list(priv->msg);
			if(!eas_provision_list_serialise (list, &serialised_provision_list)){
				g_set_error (&error,
						EAS_CONNECTION_ERROR,
						EAS_CONNECTION_PROVISION_ERROR_GENERALSERVERERROR,
						"Unable to serialize the provision list responded from server");
				ret = FALSE;
				g_free(serialised_provision_list);
				goto finish;
			}

			dbus_g_method_return (eas_request_base_GetContext (parent),
				eas_provision_msg_get_policy_key (priv->msg),
				eas_provision_msg_get_policy_status (priv->msg),
				serialised_provision_list);

			g_free(serialised_provision_list);
		}
	}
	break;

	// We now have the final provisioning policy key Set the policy key in the connection
	case EasProvisionStep2: {
		g_debug ("eas_provision_req_MessageComplete - EasProvisionStep2");

		eas_connection_set_policy_key (eas_request_base_GetConnection (parent),
					       eas_provision_msg_get_policy_key (priv->msg));

		if(!priv->internal){
			dbus_g_method_return (eas_request_base_GetContext (parent));
			cleanup = TRUE;
		}
		else{
			//tell connection that we are back to dealing with standard requests
			eas_connection_set_reprovisioning (eas_request_base_GetConnection (parent),
			                                   FALSE);
			//and update policy key on all queued messages
			eas_connection_replace_policy_key(eas_request_base_GetConnection (parent));
		}
	}
	break;
	}

finish:
	xmlFreeDoc (doc);
	if (!ret && !priv->internal) {
		g_assert (error != NULL);
		dbus_g_method_return_error (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	}

	g_debug ("eas_provision_req_MessageComplete--");
	return cleanup;
}

const gchar*
eas_provision_req_GetTid (EasProvisionReq *self)
{
	return eas_provision_msg_get_policy_key (self->priv->msg);
}