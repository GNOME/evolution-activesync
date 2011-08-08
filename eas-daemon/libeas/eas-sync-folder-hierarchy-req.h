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

#ifndef _EAS_SYNC_FOLDER_HIERARCHY_REQ_H_
#define _EAS_SYNC_FOLDER_HIERARCHY_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ             (eas_sync_folder_hierarchy_req_get_type ())
#define EAS_SYNC_FOLDER_HIERARCHY_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReq))
#define EAS_SYNC_FOLDER_HIERARCHY_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReqClass))
#define EAS_IS_SYNC_FOLDER_HIERARCHY_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ))
#define EAS_IS_SYNC_FOLDER_HIERARCHY_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ))
#define EAS_SYNC_FOLDER_HIERARCHY_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReqClass))

typedef struct _EasSyncFolderHierarchyReqClass EasSyncFolderHierarchyReqClass;
typedef struct _EasSyncFolderHierarchyReq EasSyncFolderHierarchyReq;
typedef struct _EasSyncFolderHierarchyReqPrivate EasSyncFolderHierarchyReqPrivate;

struct _EasSyncFolderHierarchyReqClass {
	EasRequestBaseClass parent_class;
};

struct _EasSyncFolderHierarchyReq {
	EasRequestBase parent_instance;

	EasSyncFolderHierarchyReqPrivate* priv;
};

GType eas_sync_folder_hierarchy_req_get_type (void) G_GNUC_CONST;

/* Callback function with results */

typedef void (*sync_folders_results_fn) (void *priv, const gchar *ret_sync_key,
					 GSList *added_folders, GSList *updated_folders,
					 GSList *deleted_folders, GError *error);

void eas_sync_folder_hierarchy_req_set_results_fn (EasSyncFolderHierarchyReq *req,
						   sync_folders_results_fn fn,
						   void *fn_data);

/**
 * Create a new sync folder hierarchy request GObject
 *
 * @param[in] syncKey
 *	  Current sync key.
 * @param[in] accountId
 *	  Unique identifier for a user account.
 * @param[in] context
 *	  DBus context token.
 *
 * @return An allocated EasSyncFolderHierarchyReq GObject or NULL
 */
EasSyncFolderHierarchyReq* eas_sync_folder_hierarchy_req_new (const gchar* syncKey,
							      const gchar* accountId,
							      DBusGMethodInvocation* context);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasSyncFolderHierarchyReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean
eas_sync_folder_hierarchy_req_Activate (EasSyncFolderHierarchyReq* self,
					GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then returning the results across the dbus to the client
 *
 * @param[in] self
 *	  The EasSyncFolderHierarchyReq GObject instance whose messages are complete.
 * @param[in] doc
 *    Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 *
 * @return TRUE if finished and needs unreffing, FALSE otherwise.
 */
gboolean
eas_sync_folder_hierarchy_req_MessageComplete (EasSyncFolderHierarchyReq* self,
					       xmlDoc *doc,
					       GError* error_in);


G_END_DECLS

#endif /* _EAS_SYNC_FOLDER_HIERARCHY_REQ_H_ */
