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

#include "eas-2way-sync-req.h"
#include "eas-sync-msg.h"
#include "eas-sync-folder-msg.h"
#include "eas-connection-errors.h"
#include "serialise_utils.h"
#include <string.h>
#include "../../libeasaccount/src/eas-account-list.h"

G_DEFINE_TYPE (Eas2WaySyncReq, eas_2way_sync_req, EAS_TYPE_REQUEST_BASE);

#define EAS_2WAY_SYNC_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_2WAY_SYNC_REQ, Eas2WaySyncReqPrivate))

typedef enum {
	Eas2WaySyncReqStep1 = 0,
	Eas2WaySyncReqStep2
} Eas2WaySyncReqState;


struct _Eas2WaySyncReqPrivate {
	EasSyncMsg* syncMsg;
	EasSyncFolderMsg* syncFolderMsg;
	gchar* sync_key;
	Eas2WaySyncReqState state;
	gchar* accountID;
	gchar* folderID;
	EasItemType ItemType;
	guint filter_type;
	GSList *serialised_add_items;   // list of serialised items to add
	GSList *delete_ids;				// list of ids to delete
	GSList *serialised_change_items;// list of serialised items to change
};



static void
eas_2way_sync_req_init (Eas2WaySyncReq *object)
{
	Eas2WaySyncReqPrivate *priv;

	object->priv = priv = EAS_2WAY_SYNC_REQ_PRIVATE (object);

	g_debug ("eas_2way_sync_req_init++");
	priv->syncMsg = NULL;
	priv->syncFolderMsg = NULL;
	priv->state = Eas2WaySyncReqStep1;
	priv->accountID = NULL;
	priv->folderID = NULL;
	priv->ItemType = EAS_ITEM_NOT_SPECIFIED;
	priv->filter_type = 0;
	priv->serialised_add_items = NULL;
	priv->delete_ids = NULL;
	priv->serialised_change_items = NULL;
	priv->sync_key = NULL;
	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_2WAY_SYNC);

	g_debug ("eas_2way_sync_req_init--");
}

static void
eas_2way_sync_req_dispose (GObject *object)
{
	Eas2WaySyncReq *req = EAS_2WAY_SYNC_REQ (object);
	Eas2WaySyncReqPrivate *priv = req->priv;

	g_debug ("eas_2way_sync_req_dispose++");

	if (priv->syncMsg) {
		g_object_unref (priv->syncMsg);
		priv->syncMsg = NULL;
	}
	if (priv->syncFolderMsg) {
		g_object_unref (priv->syncFolderMsg);
		priv->syncFolderMsg = NULL;
	}

	G_OBJECT_CLASS (eas_2way_sync_req_parent_class)->dispose (object);

	g_debug ("eas_2way_sync_req_dispose--");

}

static void
eas_2way_sync_req_finalize (GObject *object)
{
	Eas2WaySyncReq *req = EAS_2WAY_SYNC_REQ (object);
	Eas2WaySyncReqPrivate *priv = req->priv;

	g_debug ("eas_2way_sync_req_finalize++");

	g_free (priv->accountID);
	g_free (priv->folderID);
	g_free (priv->sync_key);
	// free lists
	if (priv->serialised_add_items) {
		g_slist_foreach (priv->serialised_add_items, (GFunc) g_free, NULL);
	}
	g_slist_free (priv->serialised_add_items);
	if (priv->delete_ids) {
		g_slist_foreach (priv->delete_ids, (GFunc) g_free, NULL);
	}
	g_slist_free (priv->delete_ids);
	if (priv->serialised_change_items) {
		g_slist_foreach (priv->serialised_change_items, (GFunc) g_free, NULL);
	}
	g_slist_free (priv->serialised_change_items);


	G_OBJECT_CLASS (eas_2way_sync_req_parent_class)->finalize (object);

	g_debug ("eas_2way_sync_req_finalize--");

}

static void
eas_2way_sync_req_class_init (Eas2WaySyncReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_2way_sync_req_MessageComplete;

	g_type_class_add_private (klass, sizeof (Eas2WaySyncReqPrivate));

	object_class->dispose = eas_2way_sync_req_dispose;
	object_class->finalize = eas_2way_sync_req_finalize;
}

Eas2WaySyncReq *eas_2way_sync_req_new (const gchar* syncKey,
				       const gchar* accountID,
				       const gchar* folderId,
				       guint filter_type,
				       EasItemType type,
				       GSList *serialised_add_items,		// serialised list of items. // full transfer
				       GSList *delete_ids,				// list of ids. full transfer
				       GSList *serialised_change_items,   // serialised list of items. // full transfer
				       DBusGMethodInvocation *context)
{
	Eas2WaySyncReq* self = g_object_new (EAS_TYPE_2WAY_SYNC_REQ, NULL);
	Eas2WaySyncReqPrivate *priv = self->priv;

	g_debug ("eas_2way_sync_req_new++");

	g_assert (syncKey);
	g_assert (accountID);

	priv->filter_type = filter_type;
	priv->sync_key = g_strdup (syncKey);
	priv->accountID = g_strdup (accountID);
	priv->serialised_add_items = serialised_add_items;
	priv->delete_ids = delete_ids;
	priv->serialised_change_items = serialised_change_items;

	// If we have a folderId which isn't NULL or an empty string
	if (folderId && g_strcmp0 ("", folderId)) {
		priv->folderID = g_strdup (folderId);
	}
	priv->ItemType = type;

	eas_request_base_SetContext (&self->parent_instance, context);

	g_debug ("eas_2way_sync_req_new--");
	return self;
}

gboolean
eas_2way_sync_req_Activate (Eas2WaySyncReq *self,
			    GError** error)
{
	gboolean ret = FALSE;
	Eas2WaySyncReqPrivate* priv;
	xmlDoc *doc;
	gboolean getChanges = FALSE;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_2way_sync_req_activate++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = self->priv;
	g_debug ("about to check folderID");

	// if Folder Id is not set, then for contacts, we get a default,
	// if the default has not been set yet, then call sync folder hierarchy,
	// which will write them to GSettings 
	if (priv->folderID == NULL) {
		EasAccount *acc = eas_connection_get_account (eas_request_base_GetConnection (EAS_REQUEST_BASE (self)));

		g_debug ("folder id missing - look for default");
		priv->state = Eas2WaySyncReqStep1;
		switch (priv->ItemType) {
		case EAS_ITEM_CALENDAR: {
			g_debug ("get calendar folder");
			priv->folderID = g_strdup (eas_account_get_calendar_folder (acc));
			if (priv->folderID != NULL) {
				g_debug ("default folder id for calendar = [%s]", priv->folderID);
				priv->state = Eas2WaySyncReqStep2;
			}
		}
		break;
		case EAS_ITEM_CONTACT: {
			g_debug ("default folder id for contacts = [%s]", priv->folderID);
			priv->folderID = g_strdup (eas_account_get_contact_folder (acc));
			if (priv->folderID != NULL) {
				priv->state = Eas2WaySyncReqStep2;
			}
		}
		break;
		default:
			g_warning ("Unexpected state");
			break;
		}
		g_object_unref (acc);
	} else {
		priv->state = Eas2WaySyncReqStep2;
	}

	g_debug ("default missing");

	if (priv->state == Eas2WaySyncReqStep1) {
		g_debug ("default folder id missing - do folder sync");
		//we don't have a default folder ID so we must do a folder sync first
		priv->syncFolderMsg = eas_sync_folder_msg_new ("0", priv->accountID);
		if (!priv->syncFolderMsg) {
			ret = FALSE;
			// set the error
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			goto finish;
		}
		//build request msg
		doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);
		if (!doc) {
			ret = FALSE;

			// set the error
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			goto finish;
		}

		g_debug ("eas_2way_sync_req_activate - send message");
		ret = eas_request_base_SendRequest (parent,
						    "FolderSync",
						    doc, // full transfer
		                                    FALSE,
						    error);

	} else {
		EasConnection *conn = eas_request_base_GetConnection (EAS_REQUEST_BASE (self));
		g_debug ("eas_2way_sync_req_activate - new Sync msg");

		//create sync msg type
		priv->syncMsg = eas_sync_msg_new (priv->sync_key, conn, priv->folderID, priv->ItemType);
		if (!priv->syncMsg) {
			ret = FALSE;
			// set the error
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			goto finish;
		}

		g_debug ("eas_2way_sync_req_activate- syncKey = %s", priv->sync_key);

		g_debug ("eas_2way_sync_req_activate - build messsage");
		//build request msg
		if (g_strcmp0 (priv->sync_key, "0")) {
			getChanges = TRUE;
		}
		doc = eas_sync_msg_build_message (priv->syncMsg, priv->filter_type, getChanges, priv->serialised_add_items, priv->serialised_change_items, priv->delete_ids);
		if (!doc) {
			ret = FALSE;

			// set the error
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			goto finish;
		}

		g_debug ("eas_2way_sync_req_activate - send message");
		ret = eas_request_base_SendRequest (parent,
						    "Sync",
						    doc, // full transfer
		                                    FALSE,
						    error);
	}
finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);

		if (priv->syncMsg) {
			g_object_unref (priv->syncMsg);
			priv->syncMsg = NULL;
		}
	}

	g_debug ("eas_2way_sync_req_activate--");

	return ret;
}

gboolean
eas_2way_sync_req_MessageComplete (Eas2WaySyncReq *self, xmlDoc* doc, GError* error_in)
{
	gboolean cleanup = FALSE;
	gboolean ret = TRUE;
	GError *error = NULL;
	Eas2WaySyncReqPrivate* priv = self->priv;
	gchar *ret_sync_key = NULL;
	gboolean ret_more_available = FALSE;
	gchar** ret_added_item_array = NULL;
	gchar** ret_deleted_item_array = NULL;
	gchar** ret_changed_item_array = NULL;
	GSList* added_items = NULL;
	GSList* updated_items = NULL;
	GSList* deleted_items = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	EasConnection *conn = eas_request_base_GetConnection (parent);

	g_debug ("eas_2way_sync_req_MessageComplete++");

	// if an error occurred, store it and signal daemon
	if (error_in) {
		xmlFreeDoc (doc);
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	switch (priv->state) {
	default: {
		ret = FALSE;
		g_set_error (&error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_SYNC_ERROR_INVALIDSTATE,
			     ("Invalid state"));
	}
	break;
	case Eas2WaySyncReqStep1: {
		ret = eas_sync_folder_msg_parse_response (priv->syncFolderMsg, doc, &error);

		xmlFreeDoc (doc);
		if (!ret) {
			g_assert (error != NULL);
			goto finish;
		}
		priv->state = Eas2WaySyncReqStep2;
		if (!priv->folderID) {
			if (priv->ItemType == EAS_ITEM_CALENDAR) {
				// cannot get from GSettings - as the update takes too long - get from sync msg response
				priv->folderID = g_strdup (eas_sync_folder_msg_get_def_cal_folder (priv->syncFolderMsg));
			} else {
				// cannot get from GSettings - as the update takes too long - get from sync msg response
				priv->folderID = g_strdup (eas_sync_folder_msg_get_def_con_folder (priv->syncFolderMsg));
			}
		}
		//clean up old message
		if (priv->syncFolderMsg) {
			g_object_unref (priv->syncFolderMsg);
			priv->syncFolderMsg = NULL;
		}

		g_assert (NULL == priv->syncMsg);
		//create sync  msg type
		priv->syncMsg = eas_sync_msg_new (priv->sync_key, conn, priv->folderID, priv->ItemType);
		if (!priv->syncMsg) {
			ret = FALSE;
			// set the error
			g_set_error (&error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			goto finish;
		}

		g_debug ("eas_2way_sync_req_activate- syncKey = %s", priv->sync_key);

		g_debug ("eas_2way_sync_req_activate - build messsage");
		//build request msg
		doc = eas_sync_msg_build_message (priv->syncMsg, 0, FALSE, NULL, NULL, NULL);
		if (!doc) {
			ret = FALSE;

			// set the error
			g_set_error (&error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			goto finish;
		}

		g_debug ("eas_2way_sync_req_MessageCompete - send  sync message");
		ret = eas_request_base_SendRequest (parent,
						    "Sync",
						    doc,
		                                    FALSE,
						    &error);

	}
	break;

	//we did a sync, so we need to inform the daemon that we have finished, so that it can continue and get the data
	case Eas2WaySyncReqStep2: {
		ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
		xmlFreeDoc (doc);
		if (!ret) {
			g_assert (error != NULL);

			if (error->domain == EAS_CONNECTION_ERROR &&
			    error->code == EAS_CONNECTION_SYNC_ERROR_FOLDERHIERARCHYCHANGED) {
				/* Need to update folder hierarchy before we can Sync.
				   Go back to Step1. */
				gchar *syncKey = eas_connection_get_folder_sync_key (conn);
				if (!syncKey)
					syncKey = g_strdup ("0");

				//clean up old message
				if (priv->syncMsg) {
					g_object_unref (priv->syncMsg);
					priv->syncMsg = NULL;
				}

				/* If errors happen here, let the first error show */
				priv->syncFolderMsg = eas_sync_folder_msg_new (syncKey, priv->accountID);
				g_free (syncKey);
				syncKey = NULL;
				if (!priv->syncFolderMsg)
					goto finish;

				//build request msg
				doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);
				if (!doc)
					goto finish;

				g_clear_error (&error);
				g_debug ("eas_sync_req_activate - send message");
				ret = eas_request_base_SendRequest (parent,
								    "FolderSync",
								    doc, // full transfer
				                                    FALSE,
								    &error);
				if (ret)
					priv->state = Eas2WaySyncReqStep1;
			}

			goto finish;
		}
		//finished state machine - req needs to be cleanup up by connection object
		cleanup = TRUE;
		g_debug ("eas_2way_sync_req_MessageComplete step 2");
		ret_more_available = eas_sync_msg_get_more_available (priv->syncMsg);
		ret_sync_key  = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));
		added_items   = eas_sync_msg_get_added_items (priv->syncMsg);
		updated_items = eas_sync_msg_get_updated_items (priv->syncMsg);
		deleted_items = eas_sync_msg_get_deleted_items (priv->syncMsg);

		if (!g_strcmp0 (priv->sync_key, "0")) {
			g_debug ("sync key was zero, faking more available so client loops");
			ret_more_available = TRUE;
		}

		switch (priv->ItemType) {
		case EAS_ITEM_MAIL: {
			ret = build_serialised_email_info_array (&ret_added_item_array, added_items, &error);
			if (ret) {
				ret = build_serialised_email_info_array (&ret_changed_item_array, updated_items, &error);
				if (ret) {
					ret = build_serialised_email_info_array (&ret_deleted_item_array, deleted_items, &error);
				}
			}
			if (!ret) {
				g_set_error (&error, EAS_CONNECTION_ERROR,
					     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
					     ("out of memory"));
				goto finish;
			}
		}
		break;
		case EAS_ITEM_CALENDAR:
		case EAS_ITEM_CONTACT: {
			if (build_serialised_calendar_info_array (&ret_added_item_array, added_items, &error)) {
				if (build_serialised_calendar_info_array (&ret_changed_item_array, updated_items, &error)) {
					build_serialised_calendar_info_array (&ret_deleted_item_array, deleted_items, &error);
				}
			}
		}
		break;
		default: {
			ret = FALSE;
			g_set_error (&error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_SYNC_ERROR_INVALIDTYPE,
				     ("Invalid type"));
			goto finish;
		}
		}
		// TODO add support for responses to add/delete/change
		dbus_g_method_return (eas_request_base_GetContext (parent),
				      ret_sync_key,
				      ret_more_available,
				      ret_added_item_array,
				      ret_deleted_item_array,
				      ret_changed_item_array,
				      NULL,						// add response (not supported yet)
				      NULL,						// delete response (not supported yet)
				      NULL);				   // TODO update response (not supported yet)


	}
	break;
	}

finish:
	if (!ret) {
		g_debug ("returning error %s", error->message);
		g_assert (error != NULL);
		dbus_g_method_return_error (eas_request_base_GetContext (parent), error);
		g_error_free (error);
		error = NULL;

		cleanup = TRUE;
	}
	g_free (ret_sync_key);
	g_strfreev (ret_added_item_array);
	g_strfreev (ret_deleted_item_array);
	g_strfreev (ret_changed_item_array);
	g_debug ("eas_2way_sync_req_MessageComplete--");

	return cleanup;
}
