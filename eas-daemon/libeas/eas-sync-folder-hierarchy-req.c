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

#include "eas-sync-folder-hierarchy-req.h"
#include "eas-sync-folder-msg.h"
#include "eas-connection-errors.h"
#include "eas-folder.h"
#include "serialise_utils.h"


struct _EasSyncFolderHierarchyReqPrivate {
	EasSyncFolderMsg* syncFolderMsg;
	gchar* accountID;
	gchar* syncKey;
	sync_folders_results_fn results_fn;
	void *results_fn_data;
};

#define EAS_SYNC_FOLDER_HIERARCHY_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReqPrivate))



G_DEFINE_TYPE (EasSyncFolderHierarchyReq, eas_sync_folder_hierarchy_req, EAS_TYPE_REQUEST_BASE);


static void
eas_sync_folder_hierarchy_req_return (void *self, const gchar *ret_sync_key,
				      GSList *added_folders, GSList *updated_folders,
				      GSList *deleted_folders, GError *error);

static void
eas_sync_folder_hierarchy_req_init (EasSyncFolderHierarchyReq *object)
{
	EasSyncFolderHierarchyReqPrivate *priv;

	object->priv = priv = EAS_SYNC_FOLDER_HIERARCHY_REQ_PRIVATE (object);

	g_debug ("eas_sync_folder_hierarchy_req_init++");
	priv->syncFolderMsg = NULL;
	priv->accountID = NULL;
	priv->syncKey = NULL;

	eas_request_base_SetRequestType (&object->parent_instance,
					 EAS_REQ_SYNC_FOLDER_HIERARCHY);

	g_debug ("eas_sync_folder_hierarchy_req_init--");
}

// finalize should free anything we own. called once
static void
eas_sync_folder_hierarchy_req_finalize (GObject *object)
{
	EasSyncFolderHierarchyReq *req = (EasSyncFolderHierarchyReq *) object;
	EasSyncFolderHierarchyReqPrivate *priv = req->priv;

	g_debug ("eas_sync_folder_hierarchy_req_finalize++");

	g_free (priv->syncKey);
	g_free (priv->accountID);

	G_OBJECT_CLASS (eas_sync_folder_hierarchy_req_parent_class)->finalize (object);
	g_debug ("eas_sync_folder_hierarchy_req_finalize--");
}

// dispose should unref all members on which you own a reference.
// might be called multiple times, so guard against calling g_object_unref() on an invalid GObject
static void
eas_sync_folder_hierarchy_req_dispose (GObject *object)
{
	EasSyncFolderHierarchyReq *req = (EasSyncFolderHierarchyReq *) object;
	EasSyncFolderHierarchyReqPrivate *priv = req->priv;

	g_debug ("eas_sync_folder_hierarchy_req_dispose++");

	if (priv->syncFolderMsg) {
		g_object_unref (priv->syncFolderMsg);
		priv->syncFolderMsg = NULL;
	}

	G_OBJECT_CLASS (eas_sync_folder_hierarchy_req_parent_class)->dispose (object);
	g_debug ("eas_sync_folder_hierarchy_req_dispose--");
}


static void
eas_sync_folder_hierarchy_req_class_init (EasSyncFolderHierarchyReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);

	g_debug ("eas_sync_folder_hierarchy_req_class_init++");

	g_type_class_add_private (klass, sizeof (EasSyncFolderHierarchyReqPrivate));

	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_sync_folder_hierarchy_req_MessageComplete;

	object_class->finalize = eas_sync_folder_hierarchy_req_finalize;
	object_class->dispose = eas_sync_folder_hierarchy_req_dispose;
	g_debug ("eas_sync_folder_hierarchy_req_class_init--");

}

EasSyncFolderHierarchyReq*
eas_sync_folder_hierarchy_req_new (const gchar* syncKey, const gchar* accountId, DBusGMethodInvocation* context)
{
	EasSyncFolderHierarchyReq* self = g_object_new (EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, NULL);
	EasSyncFolderHierarchyReqPrivate *priv = self->priv;

	g_debug ("eas_sync_folder_hierarchy_req_new++");

	g_return_val_if_fail (syncKey, NULL);

	priv->syncKey = g_strdup (syncKey);
	priv->accountID = g_strdup (accountId);
	priv->results_fn = eas_sync_folder_hierarchy_req_return;
	priv->results_fn_data = context;
	eas_request_base_SetContext (&self->parent_instance, context);

	g_debug ("eas_sync_folder_hierarchy_req_new--");
	return self;
}

void eas_sync_folder_hierarchy_req_set_results_fn (EasSyncFolderHierarchyReq *req,
						   sync_folders_results_fn fn,
						   void *fn_data)
{
	EasSyncFolderHierarchyReqPrivate *priv = req->priv;

	priv->results_fn = fn;
	priv->results_fn_data = fn_data;
}


gboolean
eas_sync_folder_hierarchy_req_Activate (EasSyncFolderHierarchyReq* self, GError** error)
{
	gboolean ret = FALSE;
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	xmlDoc *doc = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_debug ("eas_sync_folder_hierarchy_req_Activate++");
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	// Create sync folder msg type
	priv->syncFolderMsg = eas_sync_folder_msg_new (priv->syncKey, priv->accountID);
	if (!priv->syncFolderMsg) {
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		goto finish;
	}
	doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);
	if (!doc) {
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		g_free (priv->syncFolderMsg);
		priv->syncFolderMsg = NULL;
		goto finish;
	}

	ret = eas_request_base_SendRequest (parent, "FolderSync", doc, FALSE, error);

finish:
	g_debug ("eas_sync_folder_hierarchy_req_Activate--");
	return ret;
}

static void
eas_sync_folder_hierarchy_req_return (void *self, const gchar *ret_sync_key,
				      GSList *added_folders, GSList *updated_folders,
				      GSList *deleted_folders, GError *error)
{
	DBusGMethodInvocation *context = self;
	gchar** ret_created_folders_array = NULL;
	gchar** ret_updated_folders_array = NULL;
	gchar** ret_deleted_folders_array = NULL;

	// Serialise the response data from GSList* to char** for transmission over Dbus

	if (!error &&
	    build_serialised_folder_array (&ret_created_folders_array, added_folders, &error) &&
	    build_serialised_folder_array (&ret_updated_folders_array, updated_folders, &error) &&
	    build_serialised_folder_array (&ret_deleted_folders_array, deleted_folders, &error)) {
		dbus_g_method_return (context,
				      ret_sync_key,
				      ret_created_folders_array,
				      ret_deleted_folders_array,
				      ret_updated_folders_array);
	} else {
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	}

	g_strfreev (ret_created_folders_array);
	g_strfreev (ret_updated_folders_array);
	g_strfreev (ret_deleted_folders_array);
}

/*
 * @param error any error that occured (or NULL) [full transfer]
 */
gboolean
eas_sync_folder_hierarchy_req_MessageComplete (EasSyncFolderHierarchyReq* self, xmlDoc *doc, GError* error)
{
	EasSyncFolderHierarchyReqPrivate* priv = self->priv;
	GSList* added_folders = NULL;
	GSList* updated_folders  = NULL;
	GSList* deleted_folders  = NULL;
	const gchar *ret_sync_key = NULL;

	// if an error occurred, store it and signal daemon
	if (error)
		goto finish;

	g_debug ("eas_sync_folder_hierarchy_req_MessageComplete++");

	// if an error occurs when parsing, store it
	if (!eas_sync_folder_msg_parse_response (priv->syncFolderMsg, doc, &error)) {
		g_assert (error != NULL);
		goto finish;
	}

	ret_sync_key = eas_sync_folder_msg_get_syncKey (priv->syncFolderMsg); // no transfer

	added_folders   = eas_sync_folder_msg_get_added_folders (priv->syncFolderMsg);
	updated_folders = eas_sync_folder_msg_get_updated_folders (priv->syncFolderMsg);
	deleted_folders = eas_sync_folder_msg_get_deleted_folders (priv->syncFolderMsg);

	g_debug ("Folder Hierarchy: A:%d, U:%d, D:%d",
		 g_slist_length (added_folders),
		 g_slist_length (updated_folders),
		 g_slist_length (deleted_folders));

finish:
	priv->results_fn (priv->results_fn_data, ret_sync_key, added_folders,
			  updated_folders, deleted_folders, error);

	xmlFreeDoc (doc);
	g_debug ("eas_sync_folder_hierarchy_req_MessageComplete--");
	return TRUE;
}
