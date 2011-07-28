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
#include "eas-marshal.h"

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
	base_class->signal_id = g_signal_new ( EAS_MAIL_SIGNAL_PROGRESS,				// name of the signal
	G_OBJECT_CLASS_TYPE ( klass ),  										// type this signal pertains to
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
	
	g_debug("eas_common_sync_folder_items++");
	
	g_debug("filter_type = 0x%x", filter_type);
	g_debug("folder_id = %s", folder_id);
	g_debug("item_type = %d", item_type);
	//g_debug("first change_items_array string = %s", *change_items_array);	
	//g_debug("first add_items_array string = %s", *add_items_array);		

	if(*add_items_array != NULL)
	{
		g_warning("2-way sync doesn't support adding (untested)");
	}
	
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

	// Convert add_items_array into list (of strings)
	if(*add_items_array != NULL)
	{
		g_debug("add_items_array");
		while ( (item = add_items_array[index++]))// null terminated list of flattened items (strings)
		{
		    add_items_list = g_slist_prepend (add_items_list, g_strdup (item));
		}
	}
	
    // Convert delete_items_array into GSList
	if(*delete_items_array != NULL)
	{
		g_debug("delete_items_array");
		while ( (item = delete_items_array[index++]))// null terminated list of ids
		{
		    delete_items_list = g_slist_prepend (delete_items_list, g_strdup (item));
		}
	}

	// Convert change_items_array into list (of strings)
	if(*change_items_array != NULL)
	{
		g_debug("change_items_array");
		while ( (item = change_items_array[index++]))// null terminated list of flattened items (strings)
		{
		    change_items_list = g_slist_prepend (change_items_list, g_strdup (item));	
		}
	}

	g_debug("create request");
	
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
	eas_request_base_SetInterfaceObject (&req->parent_instance, EAS_INTERFACE_BASE(self));	
	eas_request_base_SetRequestId (&req->parent_instance, request_id);
	eas_request_base_SetRequestProgressDirection (&req->parent_instance, FALSE);//incoming progress updates	

	g_debug("activate request");
    // Activate Request
    ret = eas_2way_sync_req_Activate (req,                                 
                                 &error);
	
finish:

    if (!ret)
    {
        g_debug ("returning error %s", error->message);
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
	
	g_debug("eas_common_sync_folder_items--");
	
    return ret;
}
