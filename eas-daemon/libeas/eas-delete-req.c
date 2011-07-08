/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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

#include "eas-delete-req.h"
#include "eas-sync-msg.h"
#include "../../libeasaccount/src/eas-account-list.h"
#include <string.h>

struct _EasDeleteReqPrivate
{
    EasSyncMsg* syncMsg;
    gchar* accountID;
    gchar* syncKey;
    gchar* folder_id;
    GSList *server_ids_array;
    EasItemType itemType;
};

#define EAS_DELETE_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_DELETE_REQ, EasDeleteReqPrivate))

G_DEFINE_TYPE (EasDeleteReq, eas_delete_req, EAS_TYPE_REQUEST_BASE);

static void
eas_delete_req_init (EasDeleteReq *object)
{
    EasDeleteReqPrivate *priv;

    object->priv = priv = EAS_DELETE_REQ_PRIVATE (object);

    g_debug ("eas_delete_req_init++");
    priv->accountID = NULL;
    priv->syncKey = NULL;
    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_DELETE_ITEM);

    g_debug ("eas_delete_req_init--");
}

static void
eas_delete_req_dispose (GObject *object)
{
    EasDeleteReq *req = (EasDeleteReq *) object;
    EasDeleteReqPrivate *priv = req->priv;
	GSList *item = NULL;

    g_debug ("eas_delete_req_dispose++");

	for (item = priv->server_ids_array; item; item = item->next)
	{
		if (item->data)
		{
			g_free (item->data);
			item->data = NULL;
		}
	}

    g_debug ("eas_delete_req_dispose--");
    G_OBJECT_CLASS (eas_delete_req_parent_class)->dispose (object);
}

static void
eas_delete_req_finalize (GObject *object)
{
    EasDeleteReq *req = (EasDeleteReq *) object;
    EasDeleteReqPrivate *priv = req->priv;

    g_debug ("eas_delete_req_finalize++");

    g_free (priv->syncKey);
    g_slist_free (priv->server_ids_array);
    g_free (priv->accountID);
    g_debug ("eas_delete_req_finalize--");
    G_OBJECT_CLASS (eas_delete_req_parent_class)->finalize (object);
}

static void
eas_delete_req_class_init (EasDeleteReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

    g_debug ("eas_delete_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasDeleteReqPrivate));

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp)eas_delete_req_MessageComplete;

    object_class->finalize = eas_delete_req_finalize;
    object_class->dispose = eas_delete_req_dispose;
    g_debug ("eas_delete_req_class_init--");

}

gboolean
eas_delete_req_Activate (EasDeleteReq *self, GError** error)
{
    gboolean ret;
    EasDeleteReqPrivate *priv = self->priv;
    xmlDoc *doc;
    gboolean getChanges = FALSE;

    g_debug ("eas_delete_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if(priv->folder_id == NULL|| strlen(priv->folder_id)<=0)
	{
		EasAccount* acc = NULL;
		EasAccountList *account_list = NULL;
		GConfClient* client = NULL;

		client = gconf_client_get_default();
		g_assert(client != NULL);
		/* Get list of accounts from gconf repository */
		account_list = eas_account_list_new (client);
		g_assert(account_list != NULL);

		acc = eas_account_list_find(account_list, EAS_ACCOUNT_FIND_ACCOUNT_UID, priv->accountID);
		switch (priv->itemType)
		{
			case EAS_ITEM_CALENDAR:
				priv->folder_id = eas_account_get_calendar_folder (acc);
				break;
			case EAS_ITEM_CONTACT:
				priv->folder_id = eas_account_get_contact_folder (acc);
				break;
			default:
				g_warning("trying to get default folder for unspecified item type");
		}

	}

    //create sync  msg type
    priv->syncMsg = eas_sync_msg_new (priv->syncKey, priv->accountID, priv->folder_id, priv->itemType);

    g_debug ("eas_delete_req_Activate- syncKey = %s", priv->syncKey);

    //build request msg
    doc = eas_sync_msg_build_message (priv->syncMsg, getChanges, NULL, NULL, priv->server_ids_array);
	if(!doc)
	{
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
		ret = FALSE;
		goto finish;
	}
	
    ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                       "Sync",
                                       doc, // full transfer
                                       (struct _EasRequestBase *) self,
                                       error);

finish:	
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }

    g_debug ("eas_delete_req_Activate--");

    return ret;
}

EasDeleteReq *eas_delete_req_new (const gchar* accountId, const gchar *syncKey, const gchar *folderId, const GSList *server_ids_array, EasItemType itemType, DBusGMethodInvocation *context)
{
    EasDeleteReq* self = g_object_new (EAS_TYPE_DELETE_REQ, NULL);
    EasDeleteReqPrivate *priv = self->priv;
    guint listCount;
    guint listLen = g_slist_length ( (GSList*) server_ids_array);
    gchar *server_id = NULL;

    g_debug ("eas_delete_req_new++");
	g_return_val_if_fail(syncKey != NULL, NULL);

    priv->syncKey = g_strdup (syncKey);
    priv->folder_id = g_strdup (folderId);

	priv->itemType = itemType;

    for (listCount = 0; listCount < listLen; listCount++)
    {
        server_id = g_slist_nth_data ( (GSList*) server_ids_array, listCount);
        priv->server_ids_array = g_slist_append (priv->server_ids_array, g_strdup (server_id));
    }

    priv->accountID = g_strdup (accountId);
    eas_request_base_SetContext (&self->parent_instance, context);

    g_debug ("eas_delete_req_new--");
    return self;
}


gboolean eas_delete_req_MessageComplete (EasDeleteReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret = TRUE;
    EasDeleteReqPrivate *priv = self->priv;
    GError *error = NULL;
	gchar *ret_sync_key = NULL;
	
    g_debug ("eas_delete_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
		ret = FALSE;
        error = error_in;
        goto finish;
    }

    ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
    if (!ret)
    {
        g_assert (error != NULL);
		goto finish;
    }

	ret_sync_key = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));

finish:
    if(!ret)
	{
		dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
	}
    else
    {
        //TODO: make sure this stuff is ok to go over dbus.

        dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance),
                              ret_sync_key);
    }
    xmlFreeDoc (doc);
	g_free(ret_sync_key);

    g_debug ("eas_delete_req_MessageComplete--");
	return TRUE;
}

