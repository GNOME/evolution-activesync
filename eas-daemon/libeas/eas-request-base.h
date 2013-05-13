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

#ifndef _EAS_REQUEST_BASE_H_
#define _EAS_REQUEST_BASE_H_

#include <glib-object.h>
#include <libsoup/soup.h>
#include <libxml/xmlreader.h> // xmlDoc
#include <libedataserver/eds-version.h>
#if EDS_CHECK_VERSION(3,6,0)
# include <libedataserver/libedataserver.h>
#else
# include <libedataserver/e-flag.h>
#endif
#include "eas-connection.h"
#include <dbus/dbus-glib.h>
#include <string.h>
#include "../src/eas-interface-base.h"
#include "../src/eas-mail.h"
#include <libeassync.h>

G_BEGIN_DECLS

#define EAS_TYPE_REQUEST_BASE             (eas_request_base_get_type ())
#define EAS_REQUEST_BASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_REQUEST_BASE, EasRequestBase))
#define EAS_REQUEST_BASE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_REQUEST_BASE, EasRequestBaseClass))
#define EAS_IS_REQUEST_BASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_REQUEST_BASE))
#define EAS_IS_REQUEST_BASE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_REQUEST_BASE))
#define EAS_REQUEST_BASE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_REQUEST_BASE, EasRequestBaseClass))

typedef struct _EasRequestBaseClass EasRequestBaseClass;
typedef struct _EasRequestBase EasRequestBase;
typedef struct _EasRequestBasePrivate EasRequestBasePrivate;


typedef gboolean (*EasRequestBaseMessageCompleteFp) (EasRequestBase *self, xmlDoc* doc, GError* error_in);
typedef void (*EasRequestBaseGotChunkFp) (EasRequestBase *self, SoupMessage *msg, SoupBuffer *chunk);

struct _EasRequestBaseClass {
	GObjectClass parent_class;

	EasRequestBaseMessageCompleteFp do_MessageComplete;
	EasRequestBaseGotChunkFp do_GotChunk;
};

struct _EasRequestBase {
	GObject parent_instance;

	EasRequestBasePrivate* priv;
};

typedef enum {
	EAS_REQ_BASE = 0,
	EAS_REQ_PROVISION,
	EAS_REQ_SYNC_FOLDER_HIERARCHY,
	EAS_REQ_SYNC,
	EAS_REQ_2WAY_SYNC,	// used by the common api
	EAS_REQ_SEND_EMAIL,
	EAS_REQ_DELETE_ITEM,
	EAS_REQ_GET_EMAIL_BODY,
	EAS_REQ_UPDATE_MAIL,
	EAS_REQ_GET_EMAIL_ATTACHMENT,
	EAS_REQ_UPDATE_ITEM,
	EAS_REQ_ADD_ITEM,
	EAS_REQ_MOVE_EMAIL,
	EAS_REQ_PING,
	EAS_REQ_GET_ITEM_ESTIMATE,
	// Add other requests here

	EAS_REQ_LAST
} EasRequestType;

GType eas_request_base_get_type (void) G_GNUC_CONST;

gboolean eas_request_base_MessageComplete (EasRequestBase *self, xmlDoc* doc,  GError* error_in);

gboolean eas_request_base_SendRequest (EasRequestBase* self, const gchar* cmd, xmlDoc *doc, gboolean highpriority, GError **error);


/**
 * Getter for request type.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The request type currently set for this instance.
 */
EasRequestType eas_request_base_GetRequestType (EasRequestBase* self);

/**
 * Setter for request type.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] type
 *      The request type to be set.
 */
void eas_request_base_SetRequestType (EasRequestBase* self, EasRequestType type);

/**
 * Getter for connection.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The connection currently set for this instance.
 */
struct _EasConnection* eas_request_base_GetConnection (EasRequestBase* self);

/**
 * Setter for request type.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] connection
 *      The connection to be set.
 */
void eas_request_base_SetConnection (EasRequestBase* self, struct _EasConnection* connection);

/**
 * Getter for dbus interface.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The interface object currently set for this instance.
 */
EasInterfaceBase* eas_request_base_GetInterfaceObject (EasRequestBase* self);

/**
 * Setter for dbus interface.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] dbus_interface
 *      The interface object to be set.
 */
void eas_request_base_SetInterfaceObject (EasRequestBase* self, EasInterfaceBase *dbus_interface);

/**
 * Getter for request id.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The request_id currently set for this instance.
 */
guint eas_request_base_GetRequestId (EasRequestBase* self);

/**
 * Setter for request id.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] request_id
 *      The request_id to be set.
 */
void eas_request_base_SetRequestId (EasRequestBase* self, guint request_id);

/**
 * Getter for request owner.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The request_owner currently set for this instance.
 */
const gchar *eas_request_base_GetRequestOwner (EasRequestBase* self);

/**
 * Setter for request owner.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] request_owner
 *      The request_owner to be set.
 */
void eas_request_base_SetRequestOwner (EasRequestBase* self, gchar *request_owner);

/**
 * Getter for cancelled.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return whether the current instance has been cancelled.
 */
gboolean eas_request_base_IsCancelled (EasRequestBase* self);

/**
 * Setter for cancelled.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * sets the instance to cancelled
 */
void eas_request_base_Cancelled (EasRequestBase* self);


/**
 * Getter for outgoing.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The direction currently set for this instance (TRUE == outgoing, FALSE == incoming).
 */
gboolean eas_request_base_GetRequestProgressDirection (EasRequestBase* self);

/**
 * Setter for outgoing.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] outgoing_progress
 *      The outgoing_progress to be set (TRUE == outgoing, FALSE == incoming)
 */
void eas_request_base_SetRequestProgressDirection (EasRequestBase* self, gboolean outgoing_progress);

/**
 * Getter for data_size.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The data_size currently set for this instance.
 */
guint eas_request_base_GetDataSize (EasRequestBase* self);

/**
 * Setter for data_size.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] request_id
 *      The data_size to be set.
 */
void eas_request_base_SetDataSize (EasRequestBase* self, guint size);

/**
 * Getter for data_length_so_far.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The data_length_so_far currently set for this instance.
 */
guint eas_request_base_GetDataLengthSoFar (EasRequestBase* self);

/**
 * Updater for data_length_so_far.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] length
 *      The length to increase data_length_so_far by.
 */
void eas_request_base_UpdateDataLengthSoFar (EasRequestBase* self, guint length);

/**
 * Setter for data_length_so_far.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] length
 *      The length to be set.
 */
void eas_request_base_SetDataLengthSoFar (EasRequestBase* self, guint length);

/**
 * Getter for soup message.
 * @param[in] self
 *      GObject Instance.
 *
 * @return The soup message currently set for this instance.
 */
SoupMessage *eas_request_base_GetSoupMessage (EasRequestBase* self);

/**
 * Setter for soup message.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] soup_message
 *      The soup message to be set.
 */
void eas_request_base_SetSoupMessage (EasRequestBase* self, SoupMessage *soup_message);

/**
 * Getter for DBus context token.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return The DBus context token currently set for this instance.
 */
DBusGMethodInvocation *eas_request_base_GetContext (EasRequestBase* self);

/**
 * Setter for DBus context token.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] context
 *      The DBus context token to be set.
 */
void eas_request_base_SetContext (EasRequestBase* self, DBusGMethodInvocation* context);

/**
 * Function hook invoked during the "got_chunk" signal from libsoup.
 *
 * @param[in] self
 *      GObject Instance.
 * @param[in] msg
 *      The current SoupMessage we're receiving buffer chunks from.
 * @param[in] chunk
 *      The chunk of data from the server.
 */
void eas_request_base_GotChunk (EasRequestBase *self, SoupMessage *msg, SoupBuffer *chunk);

/**
 * Function hook invoked during the "got_chunk" signal from libsoup.
 *
 * @param[in] self
 *      GObject Instance.
 *
 * @return NULL or Pointer to buffer containing the WBXML extracted during chunking. [no transfer]
 */
guchar* eas_request_base_GetWbxmlFromChunking (EasRequestBase *self);
gsize eas_request_base_GetWbxmlFromChunkingSize (EasRequestBase *self);
void eas_request_base_SetWbxmlFromChunking (EasRequestBase *self, guchar* wbxml, gsize wbxml_length);

gboolean eas_request_base_UseMultipart (EasRequestBase* self);

void eas_request_base_Set_UseMultipart (EasRequestBase* self, gboolean use_multipart);

// @@Deprecated, to be removed once move email is updated to not use this.
EFlag *eas_request_base_GetFlag (EasRequestBase* self);
void eas_request_base_SetFlag (EasRequestBase* self, EFlag* flag);

G_END_DECLS

#endif /* _EAS_REQUEST_BASE_H_ */
