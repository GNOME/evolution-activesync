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

#ifndef _EAS_SYNC_MSG_H_
#define _EAS_SYNC_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"
#include "eas-request-base.h"
#include "eas-connection.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_MSG             (eas_sync_msg_get_type ())
#define EAS_SYNC_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_MSG, EasSyncMsg))
#define EAS_SYNC_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_MSG, EasSyncMsgClass))
#define EAS_IS_SYNC_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_MSG))
#define EAS_IS_SYNC_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_MSG))
#define EAS_SYNC_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_MSG, EasSyncMsgClass))

typedef struct _EasSyncMsgClass EasSyncMsgClass;
typedef struct _EasSyncMsg EasSyncMsg;
typedef struct _EasSyncMsgPrivate EasSyncMsgPrivate;

struct _EasSyncMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasSyncMsg {
	EasMsgBase parent_instance;

	EasSyncMsgPrivate *priv;
};

GType eas_sync_msg_get_type (void) G_GNUC_CONST;

/**
 * Create a new sync message.
 *
 * @param[in] syncKey
 *	  The client's current syncKey.
 * @param[in] connection
 *	  EasConnection object used for this message.
 * @param[in] FolderID
 *	  Identifier for the folder on the exchange server.
 * @param[in] type
 *	  The type of item to be synced - Email, Calendar, Contact etc
 *
 * @return NULL or a newly created EasSyncMsg GObject.
 */
EasSyncMsg*
eas_sync_msg_new (const gchar* syncKey,
		  EasConnection *connection,
		  const gchar *FolderID,
		  const EasItemType type);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 * @param[in] getChanges
 *	  TRUE if pulling changes from the exchange server.
 *	  FALSE if sending changes to the exchange server.
 * @param[in] added
 *	  Valid only where the instance has an EasItemType of EAS_ITEM_CALENDAR or EAS_ITEM_CONTACT.
 *	  Caller provides a list of EasItemInfo GObjects, or NULL.
 * @param[in] updated
 *	  Where the instance has an EasItemType of EAS_ITEM_MAIL:
 *	  Caller provides a list of EasEmailInfo GObjects, or NULL.
 *	  Where the instance has an EasItemType of EAS_ITEM_CALENDAR or EAS_ITEM_CONTACT.
 *	  Caller provides a list of EasItemInfo GObjects, or NULL.
 * @param[in] deleted
 *	  Caller provides a list of ServerId's for the items to be deleted, or NULL.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc*
eas_sync_msg_build_message (EasSyncMsg* self,
			    guint filter_type,
			    gboolean getChanges,
			    GSList *added,
			    GSList *updated,
			    GSList *deleted);

/**
 * Parses the response from the server, storing the email attachment according
 * to the parameters set when the EasSyncMsg GObject instance was
 * created.
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 * @param[in] doc
 *	  libxml DOM tree structure containing the XML to be parsed. [no transfer]
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean
eas_sync_msg_parse_response (EasSyncMsg* self,
			     xmlDoc *doc,
			     GError** error);
/**
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 *
 * @return NULL or List of *serialized* GObjects determined by the instance's EasItemType. [no transfer]
 *	  - EAS_ITEM_MAIL     EasEmailInfo
 *	  - EAS_ITEM_CALENDAR EasItemInfo
 *	  - EAS_ITEM_CONTACT  EasItemInfo
 */
GSList* eas_sync_msg_get_added_items (EasSyncMsg* self);

/**
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 *
 * @return NULL or List of *serialized* GObjects determined by the instance's EasItemType. [no transfer]
 *	  - EAS_ITEM_MAIL     EasEmailInfo
 *	  - EAS_ITEM_CALENDAR EasItemInfo
 *	  - EAS_ITEM_CONTACT  EasItemInfo
 */
GSList* eas_sync_msg_get_updated_items (EasSyncMsg* self);

/**
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 *
 * @return NULL or List of *serialized* GObjects determined by the instance's EasItemType. [no transfer]
 *	  - EAS_ITEM_MAIL     EasEmailInfo
 *	  - EAS_ITEM_CALENDAR EasItemInfo
 *	  - EAS_ITEM_CONTACT  EasItemInfo
 */
GSList* eas_sync_msg_get_update_responses (EasSyncMsg* self);

/**
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 *
 * @return NULL or List of *serialized* GObjects determined by the instance's EasItemType. [no transfer]
 *	  - EAS_ITEM_MAIL     EasEmailInfo
 *	  - EAS_ITEM_CALENDAR EasItemInfo
 *	  - EAS_ITEM_CONTACT  EasItemInfo
 */
GSList* eas_sync_msg_get_deleted_items (EasSyncMsg* self);

/**
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 *
 * @return NULL or The updated sync key supplied by the server response. [no transfer]
 */
gchar* eas_sync_msg_get_syncKey (EasSyncMsg* self);

/**
 *
 * @param[in] self
 *	  The EasSyncMsg GObject instance.
 *
 * @return TRUE if more available, otherwise FALSE.
 */
gboolean eas_sync_msg_get_more_available (EasSyncMsg* self);

G_END_DECLS

#endif /* _EAS_SYNC_MSG_H_ */
