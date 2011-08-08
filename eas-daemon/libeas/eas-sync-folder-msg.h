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

#ifndef _EAS_SYNC_FOLDER_MSG_H_
#define _EAS_SYNC_FOLDER_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_FOLDER_MSG             (eas_sync_folder_msg_get_type ())
#define EAS_SYNC_FOLDER_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsg))
#define EAS_SYNC_FOLDER_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgClass))
#define EAS_IS_SYNC_FOLDER_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_FOLDER_MSG))
#define EAS_IS_SYNC_FOLDER_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_FOLDER_MSG))
#define EAS_SYNC_FOLDER_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgClass))

typedef struct _EasSyncFolderMsgClass EasSyncFolderMsgClass;
typedef struct _EasSyncFolderMsg EasSyncFolderMsg;
typedef struct _EasSyncFolderMsgPrivate EasSyncFolderMsgPrivate;

struct _EasSyncFolderMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasSyncFolderMsg {
	EasMsgBase parent_instance;

	EasSyncFolderMsgPrivate *priv;
};

GType eas_sync_folder_msg_get_type (void) G_GNUC_CONST;

/**
 * Create a sync folder message.
 *
 * @param[in] accountId
 *	  Unique identifier for a user account.
 * @param[in] syncKey
 *	  The client's current syncKey.
 *
 * @return NULL or a newly created EasSyncFolderMsg GObject.
 */
EasSyncFolderMsg* eas_sync_folder_msg_new (const gchar* syncKey, const gchar* accountId);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* eas_sync_folder_msg_build_message (EasSyncFolderMsg* self);

/**
 * Parses the response from the server, storing the email attachment according
 * to the parameters set when the EasSyncFolderMsg GObject instance was
 * created.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 * @param[in] doc
 *	  libxml DOM tree structure containing the XML to be parsed. [no transfer]
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_sync_folder_msg_parse_response (EasSyncFolderMsg* self,
					     const xmlDoc *doc,
					     GError** error);

/**
 * Retrieves the response added folders.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or List of EasFolder GObjects that have been added.
 */
GSList* eas_sync_folder_msg_get_added_folders (EasSyncFolderMsg* self);

/**
 * Retrieves the response updated folders.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or List of EasFolder GObjects that have been updated.
 */
GSList* eas_sync_folder_msg_get_updated_folders (EasSyncFolderMsg* self);

/**
 * Retrieves the response deleted folders.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or List of EasFolder GObjects that have been deleted.
 */
GSList* eas_sync_folder_msg_get_deleted_folders (EasSyncFolderMsg* self);

/**
 * Retrieves the sync key in the response from the server.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or The updated sync key supplied by the server response. [no transfer]
 */
gchar* eas_sync_folder_msg_get_syncKey (EasSyncFolderMsg* self);

/**
 * Retrieves the default contact folder id in the response from the server.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or The updated default contact folder id supplied by the server response. [no transfer]
 */
gchar* eas_sync_folder_msg_get_def_con_folder (EasSyncFolderMsg* self);

/**
 * Retrieves the default calendar folder id in the response from the server.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or The updated default calendar folder id supplied by the server response. [no transfer]
 */
gchar* eas_sync_folder_msg_get_def_cal_folder (EasSyncFolderMsg* self);

G_END_DECLS

#endif /* _EAS_SYNC_FOLDER_MSG_H_ */
