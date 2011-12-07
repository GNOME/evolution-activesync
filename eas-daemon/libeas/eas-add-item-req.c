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

#include "eas-sync-msg.h"
#include "eas-add-item-req.h"
#include "serialise_utils.h"
#include "../../libeasaccount/src/eas-account-list.h"
#include <string.h>

G_DEFINE_TYPE (EasAddItemReq, eas_add_item_req, EAS_TYPE_REQUEST_BASE);

#define EAS_ADD_ITEM_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_ADD_ITEM_REQ, EasAddItemReqPrivate))

struct _EasAddItemReqPrivate {
	EasSyncMsg* sync_msg;
	gchar* account_id;
	gchar* sync_key;
	EasItemType item_type;
	gchar* folder_id;
	GSList *serialised_calendar;
};

static void
eas_add_item_req_init (EasAddItemReq *object)
{
	/* initialization code */
	EasAddItemReqPrivate *priv;
	g_debug ("eas_add_item_req_init++");

	object->priv = priv = EAS_ADD_ITEM_REQ_PRIVATE (object);

	priv->sync_msg = NULL;
	priv->account_id = NULL;
	priv->sync_key = NULL;
	priv->item_type = EAS_ITEM_LAST;
	priv->folder_id = NULL;
	priv->serialised_calendar = NULL;

	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_ADD_ITEM);

	g_debug ("eas_add_item_req_init++");
}

static void
eas_add_item_req_dispose (GObject *object)
{
	EasAddItemReq *req = EAS_ADD_ITEM_REQ (object);
	EasAddItemReqPrivate *priv = req->priv;

	g_debug ("eas_add_item_req_dispose++");

	if (priv->sync_msg) {
		g_object_unref (priv->sync_msg);
		priv->sync_msg = NULL;
	}

	G_OBJECT_CLASS (eas_add_item_req_parent_class)->dispose (object);

	g_debug ("eas_add_item_req_dispose--");
}

static void
eas_add_item_req_finalize (GObject *object)
{
	/* deinitalization code */
	EasAddItemReq *req = EAS_ADD_ITEM_REQ (object);
	EasAddItemReqPrivate *priv = req->priv;

	g_debug ("eas_add_item_req_finalize++");

	g_free (priv->sync_key);
	g_free (priv->account_id);
	g_free (priv->folder_id);

	g_slist_foreach (priv->serialised_calendar, (GFunc) g_object_unref, NULL);
	g_slist_free (priv->serialised_calendar);

	G_OBJECT_CLASS (eas_add_item_req_parent_class)->finalize (object);

	g_debug ("eas_add_item_req_finalize--");
}

static void
eas_add_item_req_class_init (EasAddItemReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasAddItemReqPrivate));

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_add_item_req_MessageComplete;

	object_class->finalize = eas_add_item_req_finalize;
	object_class->dispose = eas_add_item_req_dispose;

	g_debug ("eas_add_item_req_class_init--");
}


EasAddItemReq *
eas_add_item_req_new (const gchar* account_id,
		      const gchar *sync_key,
		      const gchar *folder_id,
		      const EasItemType item_type,
		      GSList* serialised_calendar,
		      DBusGMethodInvocation *context)
{
	EasAddItemReq* self = g_object_new (EAS_TYPE_ADD_ITEM_REQ, NULL);
	EasAddItemReqPrivate *priv = self->priv;

	g_debug ("eas_add_item_req_new++");

	priv->sync_key = g_strdup (sync_key);
	priv->folder_id = g_strdup (folder_id);
	priv->serialised_calendar = serialised_calendar; // Take ownership
	priv->account_id = g_strdup (account_id);
	priv->item_type = item_type;

	eas_request_base_SetContext (&self->parent_instance, context);

	g_debug ("eas_add_item_req_new--");
	return self;
}

gboolean
eas_add_item_req_Activate (EasAddItemReq *self, GError **error)
{
	EasAddItemReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	gboolean success = FALSE;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	EasConnection *conn = eas_request_base_GetConnection (EAS_REQUEST_BASE (self));
	g_debug ("eas_add_item_req_Activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (priv->folder_id == NULL || strlen (priv->folder_id) <= 0) {
		EasAccount *acc;

		acc = eas_connection_get_account (conn);
		switch (priv->item_type) {
		case EAS_ITEM_CALENDAR:
			g_free (priv->folder_id);
			priv->folder_id = g_strdup (eas_account_get_calendar_folder (acc));
			break;
		case EAS_ITEM_CONTACT:
			g_free (priv->folder_id);
			priv->folder_id = g_strdup (eas_account_get_contact_folder (acc));
			break;
		default:
			g_warning ("trying to get default folder for unspecified item type");
		}
		g_object_unref (acc);
	}
	//create sync msg object
	priv->sync_msg = eas_sync_msg_new (priv->sync_key, conn, priv->folder_id, priv->item_type);

	//build request msg
	doc = eas_sync_msg_build_message (priv->sync_msg, 0, FALSE, priv->serialised_calendar, NULL, NULL);

	success = eas_request_base_SendRequest (parent,
						"Sync",
						doc, // full transfer
	                                        FALSE,
						error);
	if (!success) {
		g_assert (error == NULL || (*error != NULL));
	}

	g_debug ("eas_add_item_req_Activate--");
	return success;
}


gboolean
eas_add_item_req_MessageComplete (EasAddItemReq *self,
				  xmlDoc* doc,
				  GError* error)
{
	GError *local_error = NULL;
	EasAddItemReqPrivate *priv = self->priv;
	GSList* added_items = NULL;
	gchar** ret_added_items_array = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_add_item_req_MessageComplete++");

	// If we have an error send it back to client
	if (error) {
		local_error = error;
		goto finish;
	}

	if (FALSE == eas_sync_msg_parse_response (priv->sync_msg, doc, &local_error)) {
		goto finish;
	}

	added_items = eas_sync_msg_get_added_items (priv->sync_msg);
	build_serialised_calendar_info_array (&ret_added_items_array, added_items, &error);

finish:

	if (local_error) {
		dbus_g_method_return_error (eas_request_base_GetContext (parent), local_error);
		g_error_free (local_error);
	} else {
		dbus_g_method_return (eas_request_base_GetContext (parent),
				      eas_sync_msg_get_syncKey (priv->sync_msg),
				      ret_added_items_array);
		if (ret_added_items_array) {
			gint index = 0;

			while (NULL != ret_added_items_array[index]) {
				g_free (ret_added_items_array[index]);
				ret_added_items_array[index] = NULL;
				++ index;
			}

			g_free (ret_added_items_array);
			ret_added_items_array = NULL;
		}
	}
	// We always need to free 'doc' and release the semaphore.
	xmlFreeDoc (doc);

	g_debug ("eas_add_item_req_MessageComplete--");
	return TRUE;
}

