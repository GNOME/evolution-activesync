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

#ifndef _EAS_CONNECTION_H_
#define _EAS_CONNECTION_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
#include <dbus/dbus-glib.h>
#include "eas-connection-errors.h"
#include "../../libeasaccount/src/eas-account.h"

G_BEGIN_DECLS

#define EAS_TYPE_CONNECTION             (eas_connection_get_type ())
#define EAS_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CONNECTION, EasConnection))
#define EAS_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CONNECTION, EasConnectionClass))
#define EAS_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CONNECTION))
#define EAS_IS_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CONNECTION))
#define EAS_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CONNECTION, EasConnectionClass))

typedef struct _EasConnectionClass EasConnectionClass;
typedef struct _EasConnectionPrivate EasConnectionPrivate;
typedef struct _EasConnection EasConnection;

struct _EasRequestBase;

struct _EasConnectionClass {
	GObjectClass parent_class;
};

struct _EasConnection {
	GObject parent_instance;

	EasConnectionPrivate* priv;
};

GType eas_connection_get_type (void) G_GNUC_CONST;

/**
 * Callback for passed to the eas_connection_autodiscover.
 *
 * @param[in] serverUri
 *	  NULL or Uri found by the autodiscover process, if set needs to be
 *    freed with g_free(). [full transfer]
 * @param[in] data
 *	  User data passed into eas_connection_autodiscover as cb_data. [full transfer]
 * @param[in] error
 *	  Propagated GError if not NULL, must be freed with g_error_free(). [full transfer]
 */
typedef void (*EasAutoDiscoverCallback) (char* serverUri, gpointer data, GError *error);

/**
 * Asynchronous function which attempts to discover the activesync url for an
 * exchange server given a user's email and username.
 *
 * @param[in] cb
 *      Asychronous callback function.
 * @param[in] cb_data
 *      Arbitary user data.
 * @param[in] email
 *      User's email that forms the basis for the autodiscover attempt.
 * @param[in] username
 *      Optional User's Exchange Server username, if the user's email is not of
 *		the form username@server.com. May also be used to specify a username
 *		including a windows domain if required. e.g 'DOMAIN\\username'.
 *		If not required can be set to NULL.
 */
void
eas_connection_autodiscover (const gchar* email,
			     const gchar* username,
                             DBusGMethodInvocation* context);

/**
 * Searches for an existing open connection for the GSettings account details
 * identified by the accountId, or creates a new connection if the account
 * details are valid but no existing connection exists.
 *
 * @param[in] accountId
 *	  Unique account identifier that maps on to a set of GSettings account
 *  details.
 *
 * @return NULL or EasConnection GObject corresponding to the accountId.
 */
EasConnection* eas_connection_find (const gchar* accountId);

/**
 * Create a new connection using the details supplied in the EasAccount object.
 * Note: The users details must be present in GSettings.
 *
 * @param[in] account
 *	  GObject containing the account details of a GSettings account.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns NULL. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @returns NULL or EasConnection GObject correspoing to the details in the account.
 */
EasConnection* eas_connection_new (EasAccount* account,
				   GError** error);

// Provisioning APIs
/**
 * Setter for the policy key for this connection instance.
 *
 * @param[in] self
 *	  GObject instance of EasConnection.
 * @param[in] policyKey
 *	  Policy key to be set for this connection instance.
 */
void eas_connection_set_policy_key (EasConnection* self,
				    const gchar* policyKey);

void eas_connection_resume_request (EasConnection* self, gboolean provisionSuccessful);
/**
 * Send the fully formed request over the connection to the server.
 *
 * @param[in] self
 *	  GObject instance of EasConnection.
 * @param[in] cmd
 *	  ActiveSync command string. e.g. FolderSync, Provision, Sync [no transfer]
 * @param[in] doc
 *	  Xml document tree to be transformed into the body of the request. [full transfer]
 * @param[in] request
 *	  EasXXXReq GObject corresponding to the request.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_connection_send_request (EasConnection* self,
				      const gchar* cmd,
				      xmlDoc* doc,
				      struct _EasRequestBase *request,
                                      gboolean highpriority,
				      GError** error);


/**
 * Substitutes xml body responses from the server with prefabricated responses
 * that are located in a directory named 'eas-test-responses' in the user's
 * HOME directory.
 *
 * Purely a debug API intended to allow the insertion of various 'status' level
 * server responses.
 *
 * @param[in] reponse_file_list [no transfer]
 *	  A NULL terminated array of NULL terminated strings of filenames.
 *
 * @param[in] mock_status_codes [no transfer]
 *	  An array of guints.
 *
 */
void eas_connection_add_mock_responses (const gchar** response_file_list, const GArray *mock_status_codes);


/**
 * Returns the EasAccount associated with a given connection
 *
 * @param[in] self
 *	  GObject instance of EasConnection.
 *
 * @return EasAccount object
 *
 */
EasAccount *eas_connection_get_account (EasConnection *self);

/**
 * Returns the ActiveSync protocol version used for a given connection
 *
 * @param[in] self
 *	  GObject instance of EasConnection.
 *
 * @return int ActiveSync protocol version multiplied by 10 (e.g. 121 for 12.1)
 *
 */
int eas_connection_get_protocol_version (EasConnection *self);


gchar*
eas_connection_get_multipartdata (EasConnection* self, guint partID);

void
eas_connection_update_folders (void *self, const gchar *ret_sync_key,
			       GSList *added_folders, GSList *updated_folders,
			       GSList *deleted_folders, GError *error);
gchar *
eas_connection_get_folder_sync_key (EasConnection *cnc);

gchar **eas_connection_get_folders (EasConnection *cnc);

/**
 * Cancel the request
 *
 * @param[in] self
 *	  GObject instance of EasConnection.
 * @param[in] request_id id of the request to be cancelled
 *	  
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller 
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_connection_cancel_request(EasConnection* self, 
                                     guint request_id, 
                                     GError** error); 

/**
 * Get a list of activesync protocols supported by the exchange server
 * and store them in GSettings 
 *
 * @param[in]   cnc
 *		instance of EasConnection
* @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller 
 *	  should free the memory with g_error_free() if it has been set. [full transfer] 
 *
 * @return TRUE if successful, otherwise FALSE. 
 */
gboolean eas_connection_fetch_server_protocols (EasConnection *cnc, GError **error);

void eas_connection_set_reprovisioning(EasConnection *cnc, gboolean reprovisioning);

void eas_connection_replace_policy_key(EasConnection *cnc);

void update_policy_key(gpointer job, gpointer policy_key);

G_END_DECLS

#endif /* _EAS_CONNECTION_H_ */
