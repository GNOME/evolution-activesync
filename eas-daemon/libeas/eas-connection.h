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

#ifndef _EAS_CONNECTION_H_
#define _EAS_CONNECTION_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
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

struct _EasConnectionClass
{
	GObjectClass parent_class;
};

struct _EasConnection
{
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
void eas_connection_autodiscover (EasAutoDiscoverCallback cb,
                                  gpointer cb_data, 
                                  const gchar* email, 
                                  const gchar* username);

/**
 * Searches for an existing open connection for the GConf account details 
 * identified by the accountId, or creates a new connection if the account 
 * details are valid but no existing connection exists.
 *
 * @param[in] accountId
 *	  Unique account identifier that maps on to a set of GConf account details.
 *
 * @return NULL or EasConnection GObject corresponding to the accountId.
 */
EasConnection* eas_connection_find (const gchar* accountId);

/**
 * Create a new connection using the details supplied in the EasAccount object.
 * Note: The users details must be present in GConf.
 * 
 * @param[in] account
 *	  GObject containing the account details of a GConf account.
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
void eas_connection_set_policy_key(EasConnection* self, 
                                   const gchar* policyKey);

void eas_connection_resume_request(EasConnection* self);
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
gboolean eas_connection_send_request(EasConnection* self, 
                                     const gchar* cmd, 
                                     xmlDoc* doc, 
                                     struct _EasRequestBase *request, 
                                     GError** error);



G_END_DECLS

#endif /* _EAS_CONNECTION_H_ */
