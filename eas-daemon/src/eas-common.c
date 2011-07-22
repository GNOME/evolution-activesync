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

#include "eas-common.h"
#include "eas-common-stub.h"
#include "../libeas/eas-connection-errors.h"
#include "eas-connection.h"

G_DEFINE_TYPE (EasCommon, eas_common, G_TYPE_OBJECT);

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

    object_class->finalize = eas_common_finalize;

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
                               guint item_type,
                               const gchar* folder_id,
                               guint filter_type,
                               const gchar* sync_key,
                               const gchar** add_items,
                               const gchar** delete_items,                                       
                               const gchar** change_items,
                               DBusGMethodInvocation* context)
{
	
	GError *error = NULL;

	g_debug("eas_common_sync_folder_items++");
	
	g_set_error (&error,
                 EAS_CONNECTION_ERROR,
                 EAS_CONNECTION_ERROR_NOTSUPPORTED,
                 "sync folder items not yet supported");
	
	// TODO add support for this method
	
	dbus_g_method_return_error (context, error);		
	
	g_error_free (error);
	
	g_debug("eas_common_sync_folder_items--");
	
	return FALSE;
}
