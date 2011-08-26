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

#include <string.h>
#include "eas-common.h"
#include "eas-common-stub.h"
#include "../libeas/eas-connection-errors.h"
#include "activesyncd-common-defs.h"
#include "eas-connection.h"
#include "eas-2way-sync-req.h"
#include "eas-sync-folder-hierarchy-req.h"
#include "eas-marshal.h"
#include "eas-provision-req.h"

G_DEFINE_TYPE (EasCommon, eas_common, EAS_TYPE_INTERFACE_BASE);


static void
eas_common_init (EasCommon *object)
{
	/* TODO: Add initialization code here */

}

static void
eas_common_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_common_parent_class)->finalize (object);
}

static void
eas_common_class_init (EasCommonClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasInterfaceBaseClass *base_class = EAS_INTERFACE_BASE_CLASS (klass);

	object_class->finalize = eas_common_finalize;

	// create the progress signal we emit
	base_class->signal_id = g_signal_new (EAS_MAIL_SIGNAL_PROGRESS,				// name of the signal
					      G_OBJECT_CLASS_TYPE (klass),  										// type this signal pertains to
					      G_SIGNAL_RUN_LAST,														// flags used to specify a signal's behaviour
					      0,																		// class offset
					      NULL,																	// accumulator
					      NULL,																	// user data for accumulator
					      eas_marshal_VOID__UINT_UINT,	// Function to marshal the signal data into the parameters of the signal call
					      G_TYPE_NONE,															// handler return type
					      2,																		// Number of parameter GTypes to follow
					      // GTypes of the parameters
					      G_TYPE_UINT,
					      G_TYPE_UINT);

	/* Binding to GLib/D-Bus" */
	dbus_g_object_type_install_info (EAS_TYPE_COMMON,
					 &dbus_glib_eas_common_object_info);
	dbus_g_error_domain_register (EAS_CONNECTION_ERROR,
				      "org.meego.activesyncd",
				      EAS_TYPE_CONNECTION_ERROR);
}

gboolean
eas_common_start_sync (EasCommon* obj, gint valueIn, GError** error)
{
	return TRUE;
}

gboolean
eas_common_get_protocol_version (EasCommon *obj,
				 const gchar *account_uid,
				 gchar **ret, GError **error)
{
	EasConnection *connection = eas_connection_find (account_uid);
	gint proto_ver;

	if (!connection) {
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
		return FALSE;
	}
	proto_ver = eas_connection_get_protocol_version (connection);
	*ret = g_strdup_printf ("%d.%d", proto_ver / 10, proto_ver % 10);

	return TRUE;
}


gboolean
eas_common_sync_folder_items (EasCommon* self,
			      const gchar* account_uid,
			      EasItemType item_type,
			      const gchar* sync_key,
			      const gchar* folder_id,
			      guint filter_type,
			      const gchar** add_items_array,	// array of serialised items to add
			      const gchar** delete_items_array,// array of serialised items/ids to delete
			      const gchar** change_items_array,// array of serialised items to change
			      guint request_id,
			      DBusGMethodInvocation* context)
{
	gboolean ret = TRUE;
	EasConnection *connection;
	GError *error = NULL;
	Eas2WaySyncReq *req = NULL;
	GSList *add_items_list = NULL;
	GSList *delete_items_list = NULL;
	GSList *change_items_list = NULL;
	int index = 0;
	const gchar* item = NULL;

	g_debug ("eas_common_sync_folder_items++");

	g_debug ("filter_type = 0x%x", filter_type);
	g_debug ("folder_id = %s", folder_id);
	g_debug ("item_type = %d", item_type);
	//g_debug("first change_items_array string = %s", *change_items_array);
	//g_debug("first add_items_array string = %s", *add_items_array);

	if (*add_items_array != NULL) {
		g_warning ("2-way sync doesn't support adding (untested)");
	}

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

	// Convert add_items_array into list (of strings)
	if (*add_items_array != NULL) {
		g_debug ("add_items_array");
		while ( (item = add_items_array[index++])) { // null terminated list of flattened items (strings)
			add_items_list = g_slist_prepend (add_items_list, g_strdup (item));
		}
	}

	// Convert delete_items_array into GSList
	if (*delete_items_array != NULL) {
		g_debug ("delete_items_array");
		while ( (item = delete_items_array[index++])) { // null terminated list of ids
			delete_items_list = g_slist_prepend (delete_items_list, g_strdup (item));
		}
	}

	// Convert change_items_array into list (of strings)
	if (*change_items_array != NULL) {
		g_debug ("change_items_array");
		while ( (item = change_items_array[index++])) { // null terminated list of flattened items (strings)
			change_items_list = g_slist_prepend (change_items_list, g_strdup (item));
		}
	}

	g_debug ("create request");

	req = eas_2way_sync_req_new (sync_key,
				     account_uid,
				     folder_id,
				     filter_type,
				     item_type,
				     add_items_list,		// list of serialised items
				     delete_items_list,
				     change_items_list,
				     context);
// not freeing lists as transferred to request
	eas_request_base_SetConnection (&req->parent_instance, connection);
	eas_request_base_SetInterfaceObject (&req->parent_instance, EAS_INTERFACE_BASE (self));
	eas_request_base_SetRequestId (&req->parent_instance, request_id);
	eas_request_base_SetRequestProgressDirection (&req->parent_instance, FALSE);//incoming progress updates

	g_debug ("activate request");
	// Activate Request
	ret = eas_2way_sync_req_Activate (req,
					  &error);

finish:

	if (!ret) {
		g_debug ("returning error %s", error->message);
		g_assert (error != NULL);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}

	g_debug ("eas_common_sync_folder_items--");

	return ret;
}

gboolean
eas_common_cancel_request (EasCommon* self,
			   const gchar* account_uid,
			   guint request_id,
			   DBusGMethodInvocation* context)
{
	gboolean ret = TRUE;
	EasConnection *connection;
	GError *error = NULL;
	g_debug("eas_common_cancel_request++");	
	
    connection = eas_connection_find (account_uid);
    if (!connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

	// call connection to cancel the request with the supplied id
	ret = eas_connection_cancel_request(connection, request_id, &error);
	if(!ret)
	{
		g_debug("eas_common_cancel_request returning error %s", error->message);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}
	else
	{
		dbus_g_method_return(context);
	}
		
finish:	
	g_debug("eas_common_cancel_request--");
	
	return ret;	
}

struct folder_state {
	DBusGMethodInvocation *context;
	EasConnection *cnc;
};

static void
eas_common_update_folders (void *self, const gchar *ret_sync_key,
			   GSList *added_folders, GSList *updated_folders,
			   GSList *deleted_folders, GError *error)
{
	struct folder_state *state = self;

	eas_connection_update_folders (state->cnc, ret_sync_key, added_folders,
				       updated_folders, deleted_folders, error);
	if (error)
		dbus_g_method_return_error (state->context, error);
	else {
		gchar **folders = eas_connection_get_folders (state->cnc);

		dbus_g_method_return (state->context, folders);
		g_strfreev (folders);
	}
	g_free (state);
}

gboolean
eas_common_get_folders (EasCommon* self,
			const gchar* account_uid,
			gboolean refresh,
			DBusGMethodInvocation* context)
{
	EasConnection *connection;
	GError *error = NULL;
	EasSyncFolderHierarchyReq *req = NULL;
	gchar *sync_key = NULL;

	g_debug ("eas_common_get_folders++ : account_uid[%s]",
		 (account_uid ? account_uid : "NULL"));

	connection = eas_connection_find (account_uid);
	if (!connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);
	err:
		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	sync_key = eas_connection_get_folder_sync_key (connection);
	if (!sync_key) {
		sync_key = g_strdup ("0");
		refresh = TRUE;
	}

	if (refresh) {
		struct folder_state *state = g_malloc0 (sizeof (*state));

		state->context = context;
		state->cnc = connection;

		req = eas_sync_folder_hierarchy_req_new (sync_key, account_uid, context);

		eas_sync_folder_hierarchy_req_set_results_fn (req, eas_common_update_folders,
							      state);

		eas_request_base_SetConnection (&req->parent_instance,
						connection);

		if (!eas_sync_folder_hierarchy_req_Activate (req, &error)) {
			g_free (state);
			goto err;
		}
	} else {
		gchar **folders = eas_connection_get_folders (connection);

		dbus_g_method_return (context, folders);
		g_strfreev (folders);
	}

	g_free (sync_key);
	g_debug ("eas_common_get_folders--");

	return TRUE;
}

gboolean
eas_common_get_provision_list (EasCommon* self,
			const gchar* account_uid,
			DBusGMethodInvocation* context)
{
	EasConnection *connection;
	GError *error = NULL;
	EasProvisionReq *req = NULL;

	g_debug ("%s++ : account_uid[%s]", __func__, (account_uid ?: "NULL"));

	connection = eas_connection_find (account_uid);
	if (!connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);

		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	req = eas_provision_req_new (FALSE, NULL, NULL, context);

	eas_request_base_SetConnection (&req->parent_instance, connection);

	eas_provision_req_Activate (req, &error);

	g_debug ("%s--", __func__);
	return TRUE;
}

gboolean
eas_common_autodiscover (EasCommon* self,
			const gchar* email,
            const gchar* username,
			DBusGMethodInvocation* context)
{
	eas_connection_autodiscover (email, username, context);
	return TRUE;
}

gboolean
eas_common_accept_provision_list (EasCommon* self,
			const gchar* account_uid,
			const gchar* tid,
			const gchar* tid_status,
			DBusGMethodInvocation* context)
{
	EasConnection *connection;
	GError *error = NULL;
	EasProvisionReq *req = NULL;

	g_debug ("%s++ : account_uid[%s]", __func__, (account_uid ?: "NULL"));

	connection = eas_connection_find (account_uid);
	if (!connection) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
			     "Failed to find account [%s]",
			     account_uid);

		dbus_g_method_return_error (context, error);
		g_error_free (error);
		return FALSE;
	}

	req = eas_provision_req_new (FALSE, tid_status, tid, context);

	eas_request_base_SetConnection (&req->parent_instance, connection);

	eas_provision_req_Activate (req, &error);

	g_debug ("%s--", __func__);
	return TRUE;
}
