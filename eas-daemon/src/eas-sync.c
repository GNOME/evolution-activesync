/*
 * ActiveSync DBus dæmon
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

#include "eas-sync.h"
#include "eas-sync-stub.h"
#include "eas-sync-req.h"
#include "eas-delete-req.h"
#include "eas-update-item-req.h"
#include "eas-add-item-req.h"
#include "eas-sync-folder-hierarchy-req.h"
#include <eas-item-info.h>
#include "eas-get-email-body-req.h"

#include "../libeas/eas-connection.h"
#include "eas-mail.h"

G_DEFINE_TYPE (EasSync, eas_sync, G_TYPE_OBJECT);

#define EAS_SYNC_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC, EasSyncPrivate))


struct _EasSyncPrivate {
	EasConnection* connection;
};

static void
eas_sync_init (EasSync *object)
{
	EasSyncPrivate *priv = NULL;
	g_debug ("++ eas_sync_init()");
	object->priv = priv = EAS_SYNC_PRIVATE (object);
	priv->connection = NULL;
	g_debug ("-- eas_sync_init()");
}

static void
eas_sync_finalize (GObject *object)
{
	g_debug ("eas_sync_finalize++");
	G_OBJECT_CLASS (eas_sync_parent_class)->finalize (object);
	g_debug ("eas_sync_finalize--");
}

static void
eas_sync_class_init (EasSyncClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_sync_finalize;

	g_type_class_add_private (klass, sizeof (EasSyncPrivate));

	/* Binding to GLib/D-Bus" */
	dbus_g_object_type_install_info (EAS_TYPE_SYNC,
					 &dbus_glib_eas_sync_object_info);
}

EasSync* eas_sync_new (void)
{
	EasSync* easCal = NULL;
	easCal = g_object_new (EAS_TYPE_SYNC, NULL);
	return easCal;
}


EasConnection*
eas_sync_get_eas_connection (EasSync* self)
{
	EasSyncPrivate* priv = self->priv;
	g_debug ("eas_sync_get_eas_connection++");
	return priv->connection;
	g_debug ("eas_sync_get_leas_connection--");
}

/*
 Takes a NULL terminated array of serialised calendar or contacts items and
 creates a list of EasItemInfo objects
*/
static gboolean
build_calendar_list (const gchar **serialised_cal_array, GSList **cal_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;

	g_assert (cal_list);

	g_assert (g_slist_length (*cal_list) == 0);

	while (serialised_cal_array[i]) {
		EasItemInfo *calInfo = eas_item_info_new();
		if (calInfo) {
			*cal_list = g_slist_append (*cal_list, calInfo); // add it to the list first to aid cleanup
			if (!cal_list) {
				g_object_unref (calInfo);
				ret = FALSE;
				goto cleanup;
			}
			if (!eas_item_info_deserialise (calInfo, serialised_cal_array[i])) {
				ret = FALSE;
				goto cleanup;
			}
		} else {
			ret = FALSE;
			goto cleanup;
		}
		i++;
	}

cleanup:
	if (!ret) {
		g_set_error (error,
		             EAS_CONNECTION_ERROR,
		             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
		             ("out of memory"));

		if (*cal_list != NULL){
			g_slist_foreach (*cal_list, (GFunc) g_object_unref, NULL);
			g_slist_free (*cal_list);
			*cal_list =NULL;
		}
	}

	g_debug ("list has %d items", g_slist_length (*cal_list));
	return ret;
}


void
eas_sync_get_latest_items (EasSync* self,
			   const gchar* account_uid,
			   guint64 type,
			   const gchar* folder_id,
			   const gchar* sync_key,
			   DBusGMethodInvocation* context)
{
	GError *error = NULL;
	// Create the request
	EasSyncReq *syncReqObj = NULL;

	g_debug ("eas_sync_get_latest_items++");

	self->priv->connection = eas_connection_find (account_uid);
	if (!self->priv->connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return;
	}


	g_debug ("eas_sync_get_latest_calendar_items++");
	syncReqObj = eas_sync_req_new (sync_key, account_uid, folder_id, type, context);

	eas_request_base_SetConnection (&syncReqObj->parent_instance,
					self->priv->connection);

	g_debug ("eas_sync_get_latest_items - new req");

	eas_sync_req_Activate (syncReqObj, &error);

	g_debug ("eas_sync_get_latest_items  - activate req");


	// Return the error or the requested data to the calendar client
	if (error) {
		g_debug (">> Daemon : Error ");
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}

	g_debug ("eas_sync_get_latest_items--");
}

gboolean
eas_sync_delete_items (EasSync* self,
		       const gchar* account_uid,
		       const guint64 type,
		       const gchar* folder_id,
		       const gchar* sync_key,
		       const gchar** deleted_items_array,
		       DBusGMethodInvocation* context)
{
	GError *error = NULL;
	EasDeleteReq *req = NULL;
	GSList *server_ids_list = NULL;
	const gchar* id = NULL;
	int index = 0;

	g_debug ("eas_sync_delete_items++");

	self->priv->connection = eas_connection_find (account_uid);
	if (!self->priv->connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	// Convert server_ids_array into GSList
	while ( (id = deleted_items_array[index++])) {
		server_ids_list = g_slist_prepend (server_ids_list, g_strdup (id));
	}

	req = eas_delete_req_new (account_uid, sync_key, folder_id, server_ids_list, type, context);

	eas_request_base_SetConnection (&req->parent_instance,
					self->priv->connection);

	// Start the request
	eas_delete_req_Activate (req, &error);


	if (error) {
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}
	g_debug ("eas_sync_delete_items--");
	return TRUE;
}

gboolean
eas_sync_update_items (EasSync* self,
		       const gchar* account_uid,
		       guint64 type,
		       const gchar* folder_id,
		       const gchar* sync_key,
		       const gchar **calendar_items,
		       DBusGMethodInvocation* context)
{
	GError* error = NULL;
	GSList *items = NULL;
	EasUpdateItemReq *req = NULL;

	g_debug ("eas_sync_update_calendar_items++");

	self->priv->connection = eas_connection_find (account_uid);
	if (!self->priv->connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	switch (type) {
	case EAS_ITEM_CALENDAR:
	case EAS_ITEM_CONTACT: {
		build_calendar_list (calendar_items, &items, &error);
		if (error) {
			dbus_g_method_return_error (context, error);
			g_error_free (error);
			return FALSE;
		}
	}
	break;
	default: {
			g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTSUPPORTED,
			     "Unknown type");
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
		}
	}
	// Create the request
	req = eas_update_item_req_new (account_uid, sync_key, type, folder_id, items, context);

	eas_request_base_SetConnection (&req->parent_instance,
					eas_sync_get_eas_connection (self));

	// Start the request
	eas_update_item_req_Activate (req, &error);

	if (error) {
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	g_debug ("eas_sync_update_items--");
	return TRUE;
}

gboolean
eas_sync_add_items (EasSync* self,
		    const gchar* account_uid,
		    guint64 type,
		    const gchar* folder_id,
		    const gchar* sync_key,
		    const gchar **calendar_items,
		    DBusGMethodInvocation* context)
{
	GError* error = NULL;
	GSList *items = NULL;
	EasAddItemReq *req = NULL;

	g_debug ("eas_sync_add_items++");

	self->priv->connection = eas_connection_find (account_uid);
	if (!self->priv->connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	switch (type) {
	case EAS_ITEM_CALENDAR:
	case EAS_ITEM_CONTACT: {
		build_calendar_list (calendar_items, &items, &error);
		if (error) {
			dbus_g_method_return_error (context, error);
			g_error_free (error);
			return FALSE;
		}
	}
	break;
	default: {
			g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTSUPPORTED,
			     "Unknown type");
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
		}
	}

	// Create the request
	req = eas_add_item_req_new (account_uid, sync_key, folder_id, type, items, context);

	eas_request_base_SetConnection (&req->parent_instance,
					eas_sync_get_eas_connection (self));

	// Start the request
	eas_add_item_req_Activate (req, &error);

	// Check for error
	if (error) {
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}

	g_debug ("eas_sync_add_items--");
	return TRUE;
}

gboolean
eas_sync_fetch_item (EasSync* self,
		     const gchar* account_uid,
		     const gchar* collection_id,
		     const gchar *server_id,
		     const guint64 type,
		     DBusGMethodInvocation* context)
{
	EasConnection *connection;
	gboolean ret;
	GError *error = NULL;
	EasGetEmailBodyReq *req = NULL;

	g_debug ("eas_mail_fetch_item++");

	connection = eas_connection_find (account_uid);
	if (!connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
		ret = FALSE;
		goto finish;
	}

	// Create Request
	req = eas_get_email_body_req_new (account_uid,
					  collection_id,
					  server_id,
					  NULL,
					  type,
					  context);

	eas_request_base_SetConnection (&req->parent_instance, connection);

	ret = eas_get_email_body_req_Activate (req, &error);


finish:
	if (!ret) {
		g_assert (error != NULL);
		g_warning ("eas_mail_fetch_email_body - failed to get data from message");
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}

	g_debug ("eas_mail_fetch_email_body--");
	return TRUE;
}
