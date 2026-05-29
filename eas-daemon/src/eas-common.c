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
#include "eas-gdbus-common.h"
#include "../libeas/eas-connection-errors.h"
#include "activesyncd-common-defs.h"
#include "eas-connection.h"
#include "eas-2way-sync-req.h"
#include "eas-sync-folder-hierarchy-req.h"
#include "eas-provision-req.h"

struct _EasCommonPrivate {
	EasGDBusCommon *skeleton;
};

G_DEFINE_TYPE_WITH_PRIVATE (EasCommon, eas_common, EAS_TYPE_INTERFACE_BASE);

static void
eas_common_emit_progress (EasInterfaceBase *base, guint request_id, guint percent)
{
	EasCommon *self = EAS_COMMON (base);
	eas_gdbus_common_emit_mail_operation_progress (EAS_GDBUS_COMMON (self->priv->skeleton),
						       request_id, percent);
}

/* handle-* signal callbacks - forward to implementation methods */

static gboolean
on_handle_start_sync (EasGDBusCommon *obj, GDBusMethodInvocation *invocation,
		      gint new_value, EasCommon *self)
{
	GError *error = NULL;
	eas_common_start_sync (self, new_value, &error);
	if (error) {
		g_dbus_method_invocation_return_gerror (invocation, error);
		g_error_free (error);
	} else {
		eas_gdbus_common_complete_start_sync (obj, invocation);
	}
	return TRUE;
}

static gboolean
on_handle_get_protocol_version (EasGDBusCommon *obj, GDBusMethodInvocation *invocation,
				const gchar *account_uid, EasCommon *self)
{
	GError *error = NULL;
	gchar *version = NULL;
	eas_common_get_protocol_version (self, account_uid, &version, &error);
	if (error) {
		g_dbus_method_invocation_return_gerror (invocation, error);
		g_error_free (error);
	} else {
		eas_gdbus_common_complete_get_protocol_version (obj, invocation, version);
		g_free (version);
	}
	return TRUE;
}

static gboolean
on_handle_sync_folder_items (EasGDBusCommon *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
			     const gchar *account_uid, guint item_type,
			     const gchar *sync_key_in, const gchar *folder_id, guint filter_type,
			     const gchar *const *add_items, const gchar *const *delete_items,
			     const gchar *const *change_items, guint request_id, EasCommon *self)
{
	return eas_common_sync_folder_items (self, account_uid, item_type, sync_key_in, folder_id,
					     filter_type, add_items, delete_items, change_items,
					     request_id, invocation);
}

static gboolean
on_handle_get_folders (EasGDBusCommon *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
		       const gchar *account_uid, gboolean update, EasCommon *self)
{
	return eas_common_get_folders (self, account_uid, update, invocation);
}

static gboolean
on_handle_cancel_request (EasGDBusCommon *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
			  const gchar *account_uid, guint request_id, EasCommon *self)
{
	return eas_common_cancel_request (self, account_uid, request_id, invocation);
}

static gboolean
on_handle_get_provision_list (EasGDBusCommon *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
			      const gchar *account_uid, EasCommon *self)
{
	return eas_common_get_provision_list (self, account_uid, invocation);
}

static gboolean
on_handle_accept_provision_list (EasGDBusCommon *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
				 const gchar *account_uid, const gchar *tid,
				 const gchar *tid_status, EasCommon *self)
{
	return eas_common_accept_provision_list (self, account_uid, tid, tid_status, invocation);
}

static gboolean
on_handle_autodiscover (EasGDBusCommon *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
			const gchar *email, const gchar *username, EasCommon *self)
{
	return eas_common_autodiscover (self, email, username, invocation);
}

static void
eas_common_init (EasCommon *object)
{
	EasCommonPrivate *priv;
	object->priv = priv = eas_common_get_instance_private(object);

	priv->skeleton = eas_gdbus_common_skeleton_new ();

	g_signal_connect (priv->skeleton, "handle-start-sync",
			  G_CALLBACK (on_handle_start_sync), object);
	g_signal_connect (priv->skeleton, "handle-get-protocol-version",
			  G_CALLBACK (on_handle_get_protocol_version), object);
	g_signal_connect (priv->skeleton, "handle-sync-folder-items",
			  G_CALLBACK (on_handle_sync_folder_items), object);
	g_signal_connect (priv->skeleton, "handle-get-folders",
			  G_CALLBACK (on_handle_get_folders), object);
	g_signal_connect (priv->skeleton, "handle-cancel-request",
			  G_CALLBACK (on_handle_cancel_request), object);
	g_signal_connect (priv->skeleton, "handle-get-provision-list",
			  G_CALLBACK (on_handle_get_provision_list), object);
	g_signal_connect (priv->skeleton, "handle-accept-provision-list",
			  G_CALLBACK (on_handle_accept_provision_list), object);
	g_signal_connect (priv->skeleton, "handle-autodiscover",
			  G_CALLBACK (on_handle_autodiscover), object);
}

static void
eas_common_dispose (GObject *object)
{
	EasCommon *self = EAS_COMMON (object);
	g_clear_object (&self->priv->skeleton);
	G_OBJECT_CLASS (eas_common_parent_class)->dispose (object);
}

static void
eas_common_finalize (GObject *object)
{
	G_OBJECT_CLASS (eas_common_parent_class)->finalize (object);
}

static void
eas_common_class_init (EasCommonClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasInterfaceBaseClass *base_class = EAS_INTERFACE_BASE_CLASS (klass);

	object_class->dispose = eas_common_dispose;
	object_class->finalize = eas_common_finalize;
	base_class->emit_progress = eas_common_emit_progress;
}

GDBusInterfaceSkeleton *
eas_common_get_skeleton (EasCommon *self)
{
	return G_DBUS_INTERFACE_SKELETON (self->priv->skeleton);
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
			      const gchar * const * add_items_array,	// array of serialised items to add
			      const gchar * const * delete_items_array,// array of serialised items/ids to delete
			      const gchar * const * change_items_array,// array of serialised items to change
			      guint request_id,
			      GDBusMethodInvocation* context)
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
		g_dbus_method_invocation_return_gerror(context, error);
		g_error_free (error);
	}

	g_debug ("eas_common_sync_folder_items--");

	return ret;
}

gboolean
eas_common_cancel_request (EasCommon* self,
			   const gchar* account_uid,
			   guint request_id,
			   GDBusMethodInvocation* context)
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
		g_dbus_method_invocation_return_gerror(context, error);
		g_error_free (error);
	}
	else
	{
		g_dbus_method_invocation_return_value (context, NULL);
	}
		
finish:	
	g_debug("eas_common_cancel_request--");
	
	return ret;	
}

struct folder_state {
	GDBusMethodInvocation *context;
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
		g_dbus_method_invocation_return_gerror(state->context, error);
	else {
		gchar **folders = eas_connection_get_folders (state->cnc);

		g_dbus_method_invocation_return_value (state->context,
						       g_variant_new ("(^as)", folders));
		g_strfreev (folders);
	}
	g_free (state);
}

gboolean
eas_common_get_folders (EasCommon* self,
			const gchar* account_uid,
			gboolean refresh,
			GDBusMethodInvocation* context)
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
		g_dbus_method_invocation_return_gerror(context, error);
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

		g_dbus_method_invocation_return_value (context,
						       g_variant_new ("(^as)", folders));
		g_strfreev (folders);
	}

	g_free (sync_key);
	g_debug ("eas_common_get_folders--");

	return TRUE;
}

gboolean
eas_common_get_provision_list (EasCommon* self,
			const gchar* account_uid,
			GDBusMethodInvocation* context)
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

		g_dbus_method_invocation_return_gerror(context, error);
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
			GDBusMethodInvocation* context)
{
	eas_connection_autodiscover (email, username, context);
	return TRUE;
}

gboolean
eas_common_accept_provision_list (EasCommon* self,
			const gchar* account_uid,
			const gchar* tid,
			const gchar* tid_status,
			GDBusMethodInvocation* context)
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

		g_dbus_method_invocation_return_gerror(context, error);
		g_error_free (error);
		return FALSE;
	}

	req = eas_provision_req_new (FALSE, tid_status, tid, context);

	eas_request_base_SetConnection (&req->parent_instance, connection);

	eas_provision_req_Activate (req, &error);

	g_debug ("%s--", __func__);
	return TRUE;
}
