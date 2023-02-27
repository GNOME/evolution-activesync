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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eas-connection.h"
#include <glib.h>
#include <libsoup/soup.h>

#include <wbxml/wbxml.h>
#include <libedataserver/eds-version.h>
#if EDS_CHECK_VERSION(3,6,0)
# include <libedataserver/libedataserver.h>
#else
# include <libedataserver/e-flag.h>
#endif
#include <libxml/xmlreader.h> // xmlDoc
#include <time.h>
#include <unistd.h>
#include <libsecret/secret.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>

#include <sys/stat.h>

//#include "eas-accounts.h"
#include "../../libeasaccount/src/eas-account-list.h"
#include "eas-connection-errors.h"

// List of includes for each request type
#include "eas-provision-req.h"

#include "../src/activesyncd-common-defs.h"
#include "../src/eas-mail.h"
#include <eas-folder.h>
#include <errno.h>

//#define ACTIVESYNC_14

#ifdef ACTIVESYNC_14
#define AS_DEFAULT_PROTOCOL 140
#else
#define AS_DEFAULT_PROTOCOL 121
#endif

/* For the number of connections */
#define EAS_CONNECTION_MAX_REQUESTS 1

#define QUEUE_LOCK(x) (g_rec_mutex_lock(&(x)->priv->queue_lock))
#define QUEUE_UNLOCK(x) (g_rec_mutex_unlock(&(x)->priv->queue_lock))

struct _EasConnectionPrivate {
	SoupSession* soup_session;
	GThread* soup_thread;
	GMainLoop* soup_loop;
	GMainContext* soup_context;

	gchar* accountUid;
	EasAccount *account;

	gboolean retrying_asked;

	gchar *folders_keyfile;
	GKeyFile *folders;


	GSList* multipart_strings_list;

	int protocol_version;
	gchar *proto_str;

	GSList *jobs;
	GSList *provisioning_jobs;
	GSList *active_job_queue;
	GRecMutex queue_lock;
	gboolean reprovisioning;
};

#define EAS_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_CONNECTION, EasConnectionPrivate))

#define KEYRING_ITEM_ATTRIBUTE_USER	"eas-user"
#define KEYRING_ITEM_ATTRIBUTE_SERVER	"eas-server"

static SecretSchema password_schema = {
	"org.gnome.Evolution.ActiveSync",
	SECRET_SCHEMA_DONT_MATCH_NAME,
	{
		{ KEYRING_ITEM_ATTRIBUTE_USER, SECRET_SCHEMA_ATTRIBUTE_STRING },
		{ KEYRING_ITEM_ATTRIBUTE_SERVER, SECRET_SCHEMA_ATTRIBUTE_STRING },
		{ NULL, 0 }
	}
};

typedef struct _EasLibsecretResponse {
	gboolean success;
	guint error_code; /* from SECRET_ERROR domain */
	gchar* password;
	EFlag *semaphore;
	const gchar* username;
	const gchar* serverUri;
} EasLibsecretResponse;

typedef struct _EasNode EasNode;

struct _EasNode {
	EasRequestBase *request;
	EasConnection *cnc;
};

static GMutex connection_list;
static GHashTable *g_open_connections = NULL;
static GSettings *g_gsetting = NULL;
static EasAccountList* g_account_list = NULL;
static GSList* g_mock_response_list = NULL;
static GArray *g_mock_status_codes = NULL;
static gboolean g_mock_test = FALSE;

static void connection_authenticate (SoupSession *sess, SoupMessage *msg,
				     SoupAuth *auth, gboolean retrying,
				     gpointer data);
static gpointer eas_soup_thread (gpointer user_data);
static void handle_server_response (SoupSession *session, SoupMessage *msg, gpointer data);
static gboolean wbxml2xml (const WB_UTINY *wbxml, const WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len, GError **error);
static void parse_for_status (xmlNode *node, gboolean *isErrorStatus);
static gchar *device_type, *device_id;

static void write_response_to_file (const WB_UTINY* xml, WB_ULONG xml_len);
static gboolean mainloop_password_fetch (gpointer data);
static gboolean mainloop_password_store (gpointer data);


G_DEFINE_TYPE (EasConnection, eas_connection, G_TYPE_OBJECT);

#define HTTP_STATUS_OK (200)
#define HTTP_STATUS_PROVISION (449)

static void
eas_connection_accounts_init()
{
	g_debug ("eas_connection_accounts_init++");
	if (!g_gsetting) {
		g_gsetting = g_settings_new("org.meego.activesyncd");
		if (g_gsetting == NULL) {
			g_critical ("Error Failed to create GSettings");
			return;
		}
		g_debug ("-->created gsetting");

		g_account_list = eas_account_list_new (g_gsetting);
		if (g_account_list == NULL) {
			g_critical ("Error Failed to create account list");
			return;
		}
		g_debug ("-->created account_list");

		device_type = g_settings_get_string (g_gsetting, "device-type");
		if (!device_type || !device_type[0]) {
			device_type = strdup ("MeeGo");
			g_settings_set_string (g_gsetting, "device-type", device_type);
		}

		device_id = g_settings_get_string (g_gsetting, "device-id");
		if (!device_id || !device_id[0]) {
			device_id = g_strdup_printf ("%08x%08x%08x%08x",
						     g_random_int(), g_random_int(),
						     g_random_int(), g_random_int());
			g_settings_set_string (g_gsetting, "device-id", device_id);
		}

		g_debug("Sync now\n");

		g_settings_sync ();

		g_debug ("device type %s, device id %s", device_type, device_id);
	}
	g_debug ("eas_connection_accounts_init--");
}

static void
eas_soup_logger_printer (SoupLogger *logger,
			 SoupLoggerLogLevel level,
			 char direction,
			 const char *data,
			 gpointer user_data)
{
	g_debug ("%c %s", direction, data);
}

static void
eas_connection_init (EasConnection *self)
{
	EasConnectionPrivate *priv;
	self->priv = priv = EAS_CONNECTION_PRIVATE (self);

	g_debug ("eas_connection_init++");

	g_rec_mutex_init(&priv->queue_lock);
	priv->soup_context = g_main_context_new ();
	priv->soup_loop = g_main_loop_new (priv->soup_context, FALSE);

	priv->soup_thread = g_thread_new ("eas_soup_thread", eas_soup_thread, priv);

	/* create the SoupSession for this connection */
	priv->soup_session =
		soup_session_async_new_with_options (SOUP_SESSION_USE_NTLM,
						     TRUE,
						     SOUP_SESSION_ASYNC_CONTEXT,
						     priv->soup_context,
						     SOUP_SESSION_TIMEOUT,
						     120,
						     NULL);

	g_signal_connect (priv->soup_session,
			  "authenticate",
			  G_CALLBACK (connection_authenticate),
			  self);

	if (getenv ("EAS_SOUP_LOGGER") && (atoi (g_getenv ("EAS_SOUP_LOGGER")) >= 1)) {
		SoupLogger *logger;
		logger = soup_logger_new (SOUP_LOGGER_LOG_HEADERS, -1);
		soup_logger_set_printer (logger, eas_soup_logger_printer, NULL, NULL);
		soup_session_add_feature (priv->soup_session, SOUP_SESSION_FEATURE (logger));
	}

	priv->accountUid = NULL;
	priv->account = NULL; // Just a reference
	priv->protocol_version = AS_DEFAULT_PROTOCOL;
	priv->proto_str = NULL;
	priv->reprovisioning = FALSE;

	priv->retrying_asked = FALSE;

	g_debug ("eas_connection_init--");
}

static void
eas_connection_dispose (GObject *object)
{
	EasConnection *cnc = (EasConnection *) object;
	EasConnectionPrivate *priv = NULL;
	gchar* hashkey = NULL;

	g_debug ("eas_connection_dispose++");
	g_return_if_fail (EAS_IS_CONNECTION (cnc));

	priv = cnc->priv;

	if (g_open_connections) {
		hashkey = g_strdup_printf ("%s@%s",
					   eas_account_get_username (priv->account),
					   eas_account_get_uri (priv->account));
		g_hash_table_remove (g_open_connections, hashkey);
		g_free (hashkey);
		if (g_hash_table_size (g_open_connections) == 0) {
			g_hash_table_destroy (g_open_connections);
			g_open_connections = NULL;
		}
	}

	g_signal_handlers_disconnect_by_func (priv->soup_session,
					      connection_authenticate,
					      cnc);

	if (priv->soup_session) {
		g_object_unref (priv->soup_session);
		priv->soup_session = NULL;

		if (g_main_loop_is_running (priv->soup_loop)) {
			g_main_loop_quit (priv->soup_loop);
		} else {
			g_main_loop_unref (priv->soup_loop);
			priv->soup_loop = NULL;
		}
		g_thread_join (priv->soup_thread);
		priv->soup_thread = NULL;

		if (priv->soup_loop) {
			g_main_loop_unref (priv->soup_loop);
			priv->soup_loop = NULL;
		}
		g_main_context_unref (priv->soup_context);
		priv->soup_context = NULL;
	}


	if (priv->jobs) {
		g_slist_free (priv->jobs);
		priv->jobs = NULL;
	}
	if (priv->provisioning_jobs) {
		g_slist_free (priv->provisioning_jobs);
		priv->provisioning_jobs = NULL;
	}

	if (priv->active_job_queue) {
		g_slist_free (priv->active_job_queue);
		priv->active_job_queue = NULL;
	}
	G_OBJECT_CLASS (eas_connection_parent_class)->dispose (object);

	g_debug ("eas_connection_dispose--");
}

static void
eas_connection_finalize (GObject *object)
{
	EasConnection *cnc = (EasConnection *) object;
	EasConnectionPrivate *priv = cnc->priv;

	g_debug ("eas_connection_finalize++");

	g_free (priv->accountUid);

	g_free (priv->proto_str);

	g_free (priv->folders_keyfile);
	g_key_file_free (priv->folders);

	G_OBJECT_CLASS (eas_connection_parent_class)->finalize (object);
	g_debug ("eas_connection_finalize--");
}

static void
eas_connection_class_init (EasConnectionClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	g_debug ("eas_connection_class_init++");

	g_type_class_add_private (klass, sizeof (EasConnectionPrivate));

	object_class->dispose = eas_connection_dispose;
	object_class->finalize = eas_connection_finalize;

	eas_connection_accounts_init();

	g_debug ("eas_connection_class_init--");
}

static void
storePasswordCallback (GObject *source_object,
		       GAsyncResult *result,
		       gpointer user_data)
{
	EasLibsecretResponse *response = user_data;
	GError *local_error = NULL;

	g_assert (response);

	g_debug ("storePasswordCallback++");

	g_assert (response->semaphore);

	response->success = secret_password_store_finish (result, &local_error);
	response->error_code = (local_error && local_error->domain == SECRET_ERROR) ? local_error->code : ~0;
	e_flag_set (response->semaphore);
	g_debug ("storePasswordCallback--%s%s", local_error ? " Failed: " : "", local_error ? local_error->message : "");

	g_clear_error (&local_error);
}

static gboolean
mainloop_password_store (gpointer data)
{
	EasLibsecretResponse *response = data;
	gchar * description = g_strdup_printf ("ActiveSync Server Password for %s@%s",
					       response->username,
					       response->serverUri);

	g_debug ("mainloop_password_store++");

	g_assert (response->password);

	secret_password_store (&password_schema,
			       SECRET_COLLECTION_DEFAULT,
			       description,
			       response->password,
			       NULL,
			       storePasswordCallback,
			       response,
			       KEYRING_ITEM_ATTRIBUTE_USER, response->username,
			       KEYRING_ITEM_ATTRIBUTE_SERVER, response->serverUri,
			       NULL);
	g_free (description);

	g_debug ("mainloop_password_store--");
	return FALSE;
}

// nb: can't be called from the main thread or will hang
static gboolean
writePasswordToKeyring (const gchar *password, const gchar* username, const gchar* serverUri)
{
	gboolean success = FALSE;
	EasLibsecretResponse *response = g_malloc0 (sizeof (EasLibsecretResponse));
	GSource *source = NULL;

	g_debug ("writePasswordToKeyring++");
	response->semaphore = e_flag_new ();
	response->username = username;
	response->serverUri = serverUri;
	response->password = (gchar*) password;
	g_assert (response->semaphore);
	g_assert (response->password);

	/* We can't use g_idle_add, as the default context here is the soup thread */
	source = g_idle_source_new ();
	g_source_set_callback (source, mainloop_password_store, response, NULL);
	g_source_attach (source, NULL); //default context

	e_flag_wait (response->semaphore);
	e_flag_free (response->semaphore);

	success = response->success;
	g_free (response);

	g_debug ("writePasswordToKeyring--");
	return success;
}

static void
getPasswordCallback (GObject *source_object,
		     GAsyncResult *result,
		     gpointer user_data)
{
	EasLibsecretResponse *response = user_data;
	GError *local_error = NULL;

	g_debug ("getPasswordCallback++");

	g_assert (response);
	g_assert (response->semaphore);

	response->password = secret_password_lookup_finish (result, &local_error);
	response->success = local_error == NULL && response->password != NULL;
	response->error_code = (local_error && local_error->domain == SECRET_ERROR) ? local_error->code : SECRET_ERROR_NO_SUCH_OBJECT;
	e_flag_set (response->semaphore);

	g_debug ("getPasswordCallback--%s%s", local_error ? " Failed: " : "", local_error ? local_error->message : "");

	g_clear_error (&local_error);
}

static gboolean
fetch_and_store_password (const gchar *account_id, const gchar *username, const gchar *serverUri)
{
	gchar *argv[3];
	gchar * password = NULL;
	GError * error = NULL;
	gboolean success;
	gint exit_status = 0;

	g_debug ("fetch_and_store_password++");

	argv[0] = (gchar *) ASKPASS;
	argv[1] = g_strdup_printf (_("Please enter your ActiveSync password for %s"),
				   account_id);
	argv[2] = NULL;

	if (FALSE == g_spawn_sync (NULL, argv, NULL,
				   G_SPAWN_SEARCH_PATH,
				   NULL, NULL,
				   &password, NULL,
				   &exit_status,
				   &error)) {
		g_warning ("Failed to spawn : [%d][%s]", error->code, error->message);
		g_free (argv[1]);
		g_error_free (error);
		return FALSE;
	}

	g_strchomp (password);
	success = writePasswordToKeyring (password, username, serverUri);

	memset (password, 0, strlen (password));
	g_free (password);
	password = NULL;

	g_free (argv[1]);

	g_debug ("fetch_and_store_password--");
	return success;
}

static gboolean
mainloop_password_fetch (gpointer data)
{
	EasLibsecretResponse *response = data;

	g_debug ("mainloop_password_fetch++");


	secret_password_lookup (&password_schema,
				NULL,
				getPasswordCallback,
				response,
				KEYRING_ITEM_ATTRIBUTE_USER, response->username,
				KEYRING_ITEM_ATTRIBUTE_SERVER, response->serverUri,
				NULL);

	g_debug ("mainloop_password_fetch--");

	return FALSE;
}

/* error_code is from SECRET_ERROR domain */
static gboolean
getPasswordFromKeyring (const gchar* username, const gchar* serverUri, char** password, guint *out_error_code)
{
	gboolean success;
	EFlag *semaphore = NULL;
	EasLibsecretResponse *response = g_malloc0 (sizeof (EasLibsecretResponse));
	GSource *source = NULL;

	g_debug ("getPasswordFromKeyring++");

	g_assert (password && *password == NULL);

	response->semaphore = semaphore = e_flag_new ();
	response->username = username;
	response->serverUri = serverUri;

	/* We can't use g_idle_add, as the default context here is the soup thread */
	source = g_idle_source_new ();
	g_source_set_callback (source, mainloop_password_fetch, response, NULL);
	g_source_attach (source, NULL);

	e_flag_wait (response->semaphore);
	e_flag_free (response->semaphore);
	success = response->success;
	*out_error_code = response->error_code;
	*password = response->password;
	g_free (response);

	g_debug ("getPasswordFromKeyring--");
	return success;
}

static void
connection_authenticate (SoupSession *sess,
			 SoupMessage *msg,
			 SoupAuth *auth,
			 gboolean retrying,
			 gpointer data)
{
	EasConnection* cnc = (EasConnection *) data;
	const gchar * username = eas_account_get_username (cnc->priv->account);
	const gchar * serverUri = eas_account_get_uri (cnc->priv->account);
	const gchar * account_id = eas_account_get_uid (cnc->priv->account);
	guint error_code = ~0;
	gchar* password = NULL;

	g_debug ("  eas_connection - connection_authenticate++");

	// @@FIX ME - Temporary grab of password from GSettings 

	password = eas_account_get_password (cnc->priv->account);

	g_debug("Password = \'%s\'", password);

	if (password && password[0]) {
		g_warning ("Found password in GSettings, writing it to libsecret");

		if (!writePasswordToKeyring (password, username, serverUri)) {
			g_warning ("Failed to store GSettings password in libsecret");
		}
	}
	
	password = NULL;

	if (!getPasswordFromKeyring (username, serverUri, &password, &error_code)) {
		if (error_code == SECRET_ERROR_NO_SUCH_OBJECT) {
			g_warning ("Failed to find password in libsecret");

			if (fetch_and_store_password (account_id, username, serverUri)) {
				if (getPasswordFromKeyring (username, serverUri, &password, &error_code)) {
					g_debug ("First authentication attempt with newly set password");
					cnc->priv->retrying_asked = FALSE;
					soup_auth_authenticate (auth,
								username,
								password);
				} else {
					g_critical ("Failed to fetch and store password to libsecret [%d]", error_code);
				}

				if (password) {
					memset (password, 0 , strlen (password));
					g_free (password);
					password = NULL;
				}
			}
		} else {
			g_warning ("libsecret failed to find password [%d]", error_code);
		}
	} else {
		g_debug ("Found password in libsecret");
		if (!retrying) {
			g_debug ("First authentication attempt");

			cnc->priv->retrying_asked = FALSE;

			soup_auth_authenticate (auth,
						username,
						password);
			if (password) {
				memset (password, 0 , strlen (password));
				g_free (password);
				password = NULL;
			}
		} else if (!cnc->priv->retrying_asked) {
			g_debug ("Second authentication attempt - original password was incorrect");
			cnc->priv->retrying_asked = TRUE;

			if (password) {
				memset (password, 0 , strlen (password));
				g_free (password);
				password = NULL;
			}

			if (fetch_and_store_password (account_id, username, serverUri)) {
				if (getPasswordFromKeyring (username, serverUri, &password, &error_code)) {
					g_debug ("Second authentication with newly set password");
					soup_auth_authenticate (auth,
								username,
								password);
				} else {
					g_critical ("Failed to store password to libsecret [%d]", error_code);
				}

				if (password) {
					memset (password, 0 , strlen (password));
					g_free (password);
					password = NULL;
				}
			}
		} else {
			g_debug ("Failed too many times, authentication aborting");
		}
	}

	g_debug ("  eas_connection - connection_authenticate--");
}

static gpointer
eas_soup_thread (gpointer user_data)
{
	EasConnectionPrivate *priv = user_data;

	g_debug ("  eas_connection - eas_soup_thread++");

	if (priv->soup_context && priv->soup_loop) {
		g_main_context_push_thread_default (priv->soup_context);
		g_main_loop_run (priv->soup_loop);
		g_main_context_pop_thread_default (priv->soup_context);
	}
	g_debug ("  eas_connection - eas_soup_thread--");
	return NULL;
}

void eas_connection_set_policy_key (EasConnection* self, const gchar* policyKey)
{
	EasConnectionPrivate *priv = self->priv;

	g_debug ("eas_connection_set_policy_key++");

	eas_account_set_policy_key (priv->account, policyKey);

	eas_account_list_save_item (g_account_list,
				    priv->account,
				    EAS_ACCOUNT_POLICY_KEY);

	g_debug ("eas_connection_set_policy_key--");
}

EasAccount *
eas_connection_get_account (EasConnection *self)
{
	EasConnectionPrivate *priv = self->priv;

	return g_object_ref (priv->account);
}

int
eas_connection_get_protocol_version (EasConnection *self)
{
	EasConnectionPrivate *priv = self->priv;

	return priv->protocol_version;
}


static EasNode *
eas_node_new ()
{
	EasNode *node;
	g_debug ("eas_node_new++");
	node = g_new0 (EasNode, 1);
	g_debug ("eas_node_new--");
	return node;
}

static gboolean
call_handle_server_response (gpointer data)
{
	EasNode* node = (EasNode*)data;
	EasRequestBase *req = node->request;
	SoupMessage *msg = eas_request_base_GetSoupMessage (req);

	g_debug ("call_handle_server_response++");
	handle_server_response (NULL, msg, node);

	g_debug ("call_handle_server_response--");
	return FALSE;   // don't put back on main loop
}

static gboolean
call_soup_session_cancel_message (gpointer data)
{
	EasRequestBase *req = EAS_REQUEST_BASE (data);
	SoupMessage *msg = eas_request_base_GetSoupMessage (req);	
	EasConnection *cnc = eas_request_base_GetConnection(req);

	g_debug("cancelling soup request for request with id %d", eas_request_base_GetRequestId(req));
	soup_session_cancel_message(cnc->priv->soup_session, msg, SOUP_STATUS_CANCELLED);

	return FALSE;   // don't put back on main loop
}

#if 0
static gboolean
eas_queue_soup_message (gpointer _request)
{
	EasRequestBase *request = EAS_REQUEST_BASE (_request);
	EasConnection *self = eas_request_base_GetConnection (request);
	SoupMessage *msg = eas_request_base_GetSoupMessage (request);
	EasConnectionPrivate *priv = self->priv;

	soup_session_queue_message (priv->soup_session,
				    msg,
				    handle_server_response,
				    request);

	return FALSE;
}
#endif

/*
emits a signal (if the dbus interface object has been set)

*/
static void emit_signal (SoupBuffer *chunk, EasRequestBase *request)
{
	guint request_id = eas_request_base_GetRequestId (request);
	EasInterfaceBase* dbus_interface;
	EasInterfaceBaseClass* dbus_interface_klass;
	guint percent, total, so_far;

	dbus_interface = eas_request_base_GetInterfaceObject (request);
	if (dbus_interface) {
		dbus_interface_klass = EAS_INTERFACE_BASE_GET_CLASS (dbus_interface);

		total = eas_request_base_GetDataSize (request);

		eas_request_base_UpdateDataLengthSoFar (request, chunk->length);
		so_far = eas_request_base_GetDataLengthSoFar (request);

		// TODO what should we send if percentage not possible?
		if (total) {
			percent = so_far * 100 / total;

			g_debug ("emit signal with percent: %d * 100 / %d = %d", so_far, total, percent);

			//emit signal
			g_signal_emit (dbus_interface,
				       dbus_interface_klass->signal_id,
				       0,
				       request_id,
				       percent);
		}
	}
}

static void soap_got_chunk (SoupMessage *msg, SoupBuffer *chunk, gpointer data)
{
	EasRequestBase *request	 = (EasRequestBase *) data;
	gboolean outgoing_progress = eas_request_base_GetRequestProgressDirection (request);

	g_debug ("soap_got_chunk");

	if (msg->status_code != 200)
		return;

	if (!outgoing_progress) { // want incoming progress updates
		emit_signal (chunk, request);
	}

	eas_request_base_GotChunk (request, msg, chunk);

	g_debug ("Received %zd bytes for request %p", chunk->length, request);
}

static void soap_got_headers (SoupMessage *msg, gpointer data)
{
	struct _EasRequestBase *request = data;
	const gchar *size_hdr;
	gboolean outgoing_progress = eas_request_base_GetRequestProgressDirection (request);

	g_debug ("soap_got_headers");
	size_hdr = soup_message_headers_get_one (msg->response_headers,
						 "Content-Length");
	if (size_hdr) {
		gsize size = strtoull (size_hdr, NULL, 10);
		// store the response size in the request base
		if (!outgoing_progress) { // want incoming progress, so store size
			eas_request_base_SetDataSize (request, size);
		}

		g_debug ("Response size of request %p is %zu bytes", request, size);
		// We can stash this away and use it to provide progress updates
		// as a percentage. If we don't have a Content-Length: header,
		// for example if the server uses chunked encoding, then we cannot
		// do percentages.
	} else {
		if (!outgoing_progress) {
			eas_request_base_SetDataSize (request, 0);
		}
		g_debug ("Response size of request %p is unknown", request);
		// Note that our got_headers signal handler may be called more than
		// once for a given request, if it fails once with 'authentication
		// needed' then succeeds on being resubmitted. So if the *first*
		// response has a Content-Length but the second one doesn't, we have
		// to do the right thing and not keep using the first length.
	}
}


static void
soap_wrote_body_data (SoupMessage *msg, SoupBuffer *chunk, gpointer data)
{
	struct _EasRequestBase *request = data;
	gboolean outgoing_progress = eas_request_base_GetRequestProgressDirection (request);

	g_debug ("soap_wrote_body_data %zu", chunk->length);

	if (outgoing_progress) {
		emit_signal (chunk, request);
	}

	g_debug ("Wrote %zd bytes for request %p", chunk->length, request);
}

static void soap_wrote_headers (SoupMessage *msg, gpointer data)
{
	struct _EasRequestBase *request = data;
	const gchar *size_hdr;
	gboolean outgoing_progress = eas_request_base_GetRequestProgressDirection (request);

	g_debug ("soap_wrote_headers");

	if (outgoing_progress) {
		size_hdr = soup_message_headers_get_one (msg->request_headers,
							 "Content-Length");
		if (size_hdr) {
			gsize size = strtoull (size_hdr, NULL, 10);

			// store the request size in the request base
			if (outgoing_progress) {
				eas_request_base_SetDataSize (request, size);
				eas_request_base_SetDataLengthSoFar (request, 0);	// reset
			}

			g_debug ("Request size of request %p is %zu bytes", request, size);

		} else {
			eas_request_base_SetDataSize (request, 0);
			g_debug ("Request size of request %p is unknown", request);
		}
	}
}



static gboolean
eas_next_request (gpointer _cnc)
{
	EasConnection *cnc = _cnc;
	GSList *l;
	EasNode *node;
	gboolean using_provisioning_queue=FALSE;
	g_debug ("eas_next_request++");

	QUEUE_LOCK (cnc);

	l = cnc->priv->provisioning_jobs;

	if(l){
		using_provisioning_queue=TRUE;
	}else{
		l = cnc->priv->jobs;
	}
	
	if (!l || g_slist_length (cnc->priv->active_job_queue) >= EAS_CONNECTION_MAX_REQUESTS) {
		QUEUE_UNLOCK (cnc);
		return FALSE;
	}

	node = (EasNode *) l->data;

#if 0
	if (g_getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 1)) {
		soup_buffer_free (soup_message_body_flatten (SOUP_MESSAGE (eas_request_base_GetSoupMessage (node->request))->request_body));
		/* print request's body */
		printf ("\n The request headers");
		fputc ('\n', stdout);
		fputs (SOUP_MESSAGE (eas_request_base_GetSoupMessage (node->request))->request_body->data, stdout);
		fputc ('\n', stdout);
	}
#endif

	/* Remove the node from the job queue */
	if(using_provisioning_queue){
		cnc->priv->provisioning_jobs = g_slist_remove (cnc->priv->provisioning_jobs, (gconstpointer *) node);
		g_debug ("eas_next_request : provisioning job-- queuelength=%d", g_slist_length (cnc->priv->provisioning_jobs));
	}else{
		cnc->priv->jobs = g_slist_remove (cnc->priv->jobs, (gconstpointer *) node);
		g_debug ("eas_next_request : job-- queuelength=%d", g_slist_length (cnc->priv->jobs));
	}
	/* Add to active job list */
	cnc->priv->active_job_queue = g_slist_append (cnc->priv->active_job_queue, node);
	g_debug ("eas_next_request: active_job++  listlength=%d", g_slist_length (cnc->priv->active_job_queue));

	soup_session_queue_message (cnc->priv->soup_session,
				    SOUP_MESSAGE (eas_request_base_GetSoupMessage (node->request)),
				    handle_server_response,
				    node);
	QUEUE_UNLOCK (cnc);
	g_debug ("eas_next_request--");
	return FALSE;
}

static void
eas_trigger_next_request (EasConnection *cnc)
{
	GSource *source;
	g_debug ("eas_trigger_next_request++");
	source = g_idle_source_new ();
	g_source_set_priority (source, G_PRIORITY_DEFAULT);
	g_source_set_callback (source, eas_next_request, cnc, NULL);
	g_source_attach (source, cnc->priv->soup_context);
	g_debug ("eas_trigger_next_request--");
}

/**
	Free the active job and pull a new one into the queue 
	called when we're finished processing the response
 */
static void
eas_active_job_done (EasConnection *cnc, EasNode *eas_node)
{
	g_debug ("eas_active_job_done++");
	g_free (eas_node);
	eas_trigger_next_request(cnc);
	g_debug ("eas_active_job_done--");
}

/**
	Remove the job from the active queue, 
	called when we get a response from soup
 */
static void
eas_active_job_dequeue (EasConnection *cnc, EasNode *eas_node)
{
	g_debug ("eas_active_job_dequeue++");
	QUEUE_LOCK (cnc);

	cnc->priv->active_job_queue = g_slist_remove (cnc->priv->active_job_queue, eas_node);
	
	g_debug ("eas_active_job_dequeue:  queuelength=%d",g_slist_length(cnc->priv->active_job_queue));
	
	QUEUE_UNLOCK (cnc);
	g_debug ("eas_active_job_dequeue--");
}


/**
 * WBXML encode the message and send to activesync server via libsoup.
 * May also be required to temporarily hold the request message whilst
 * provisioning with the server occurs.
 *
 * @param self
 *	  The EasConnection GObject instance.
 * @param cmd
 *	  ActiveSync command string [no transfer]
 * @param doc
 *	  The message xml body [full transfer]
 * @param request
 *	  The request GObject
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean
eas_connection_send_request (EasConnection* self,
			     const gchar* cmd,
			     xmlDoc* doc,
			     EasRequestBase *request,
                             gboolean highpriority,
			     GError** error)
{
	gboolean ret = TRUE;
	EasConnectionPrivate *priv = EAS_CONNECTION_PRIVATE (self);
	SoupMessage *msg = NULL;
	gchar* uri = NULL;
	WB_UTINY *wbxml = NULL;
	WB_ULONG wbxml_len = 0;
	WBXMLError wbxml_ret = WBXML_OK;
	WBXMLConvXML2WBXML *conv = NULL;
	xmlChar* dataptr = NULL;
	int data_len = 0;
	const gchar *policy_key;
	gchar *fake_device_id = NULL;
	EasNode *node = NULL;

	g_debug ("eas_connection_send_request++");

	wbxml_ret = wbxml_conv_xml2wbxml_create (&conv);
	if (wbxml_ret != WBXML_OK) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     _("error %d returned from wbxml_conv_xml2wbxml_create"), wbxml_ret);
		ret = FALSE;
		goto finish;
	}

	fake_device_id = eas_account_get_device_id (priv->account);
	if (fake_device_id && fake_device_id[0]) {
		g_debug ("using fake_device_id");
		uri = g_strconcat (eas_account_get_uri (priv->account),
				   "?Cmd=", cmd,
				   "&User=", eas_account_get_username (priv->account),
				   "&DeviceId=", fake_device_id,
				   "&DeviceType=", device_type,
				   NULL);

	} else {

		uri = g_strconcat (eas_account_get_uri (priv->account),
				   "?Cmd=", cmd,
				   "&User=", eas_account_get_username (priv->account),
				   "&DeviceId=", device_id,
				   "&DeviceType=", device_type,
				   NULL);
	}

//TODO: Remove this g_debug
	g_debug ("The uri of soup_message_new is %s.", uri);
	msg = soup_message_new ("POST", uri);
	g_free (uri);
	if (!msg) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_SOUPERROR,
			     _("soup_message_new returned NULL"));
		ret = FALSE;
		goto finish;
	}

	soup_message_headers_append (msg->request_headers,
				     "MS-ASProtocolVersion",
				     priv->proto_str);

	if (eas_request_base_UseMultipart (request)) {
		soup_message_headers_append (msg->request_headers,
					     "MS-ASAcceptMultipart",
					     "T");
		// Instruct libsoup not to accumulate response data per 'got_chunk' 
		// into a buffer in the body.
		soup_message_body_set_accumulate (msg->response_body, FALSE);
	}

	soup_message_headers_append (msg->request_headers,
				     "User-Agent",
				     "libeas");

	if (0 == g_strcmp0 (cmd, "Provision")) 
	{
		EasProvisionReq *prov_req = EAS_PROVISION_REQ (request);
		policy_key = eas_provision_req_GetTid (prov_req);
		if (!policy_key)
		{
			soup_message_headers_append (msg->request_headers,
						     "X-MS-PolicyKey", 
						     "0");
		}
		else
		{
			soup_message_headers_append (msg->request_headers,
						     "X-MS-PolicyKey", 
						     policy_key);
		}
	}
	else
	{
		/* If policy key is set from provisioning add it, otherwise skip */
		if ( (policy_key = eas_account_get_policy_key (priv->account)) )
		{
			soup_message_headers_append (msg->request_headers,
						     "X-MS-PolicyKey", 
						     policy_key);
		}
		else
		{
			soup_message_headers_append (msg->request_headers,
						     "X-MS-PolicyKey", 
						     "0");
		}
	}
	
//in activesync 12.1, SendMail uses mime, not wbxml in the body
	if (priv->protocol_version >= 140 || g_strcmp0 (cmd, "SendMail")) {
		// Convert doc into a flat xml string
		xmlDocDumpFormatMemoryEnc (doc, &dataptr, &data_len, (gchar*) "utf-8", 1);
		wbxml_conv_xml2wbxml_disable_public_id (conv);
		wbxml_conv_xml2wbxml_disable_string_table (conv);
		wbxml_ret = wbxml_conv_xml2wbxml_run (conv, dataptr, data_len, &wbxml, &wbxml_len);

		if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5)) {
			gchar* tmp = g_strndup ( (gchar*) dataptr, data_len);
			g_debug ("\n=== XML Input ===\n%s=== XML Input ===", tmp);
			g_free (tmp);

			g_debug ("wbxml_conv_xml2wbxml_run [Ret:%s],  wbxml_len = [%d]", wbxml_errors_string (wbxml_ret), wbxml_len);
		}

		if (wbxml_ret != WBXML_OK) {
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_WBXMLERROR,
				     _("error %d returned from wbxml_conv_xml2wbxml_run"), wbxml_ret);
			ret = FALSE;
			goto finish;
		}

		soup_message_headers_set_content_length (msg->request_headers, wbxml_len);

		soup_message_set_request (msg,
					  "application/vnd.ms-sync.wbxml",
					  SOUP_MEMORY_COPY,
					  (gchar*) wbxml,
					  wbxml_len);
	} else {
		//Activesync 12.1 implementation for SendMail Command
		soup_message_headers_set_content_length (msg->request_headers, strlen ( (gchar*) doc));

		soup_message_set_request (msg,
					  "message/rfc822",
					  SOUP_MEMORY_COPY,
					  (gchar*) doc,
					  strlen ( (gchar*) doc));

	}

	if (g_mock_response_list) {
		// TODO fake complete response (wrong content type etc), not just status codes
		const gchar *response_body = "mock response body";

		soup_message_set_response (msg, "application/vnd.ms-sync.wbxml",
					   SOUP_MEMORY_COPY,
					   response_body,
					   strlen (response_body));
		// fake the status code in the soupmessage response
		if (g_mock_status_codes) {
			guint status_code = g_array_index (g_mock_status_codes, guint, 0);
			g_array_remove_index (g_mock_status_codes, 0);
			soup_message_set_status (msg, status_code);
		} else {
			soup_message_set_status (msg, SOUP_STATUS_OK);
		}
	}
	eas_request_base_SetSoupMessage (request, msg);

	if (g_mock_response_list) {	// call handle_server_response directly from soup thread
		g_debug ("put call_handle_server_response on soup loop");
		node = eas_node_new ();
		node->request = request;
		node->cnc = self;
		g_idle_add (call_handle_server_response, node);
		
	} else {	// send request via libsoup
		g_signal_connect (msg, "got-chunk", G_CALLBACK (soap_got_chunk), request);
		g_signal_connect (msg, "got-headers", G_CALLBACK (soap_got_headers), request);
		g_signal_connect (msg, "wrote-body-data", G_CALLBACK (soap_wrote_body_data), request);
		g_signal_connect (msg, "wrote-headers", G_CALLBACK (soap_wrote_headers), request);
#if 0
		// We have to call soup_session_queue_message() from the soup thread,
		// or libsoup screws up (https://bugzilla.gnome.org/642573)
		source = g_idle_source_new ();
		g_source_set_callback (source, eas_queue_soup_message, request, NULL);
		g_source_attach (source, priv->soup_context);
#endif

		/*Adding request to the queue*/
		node = eas_node_new ();
		node->request = request;
		node->cnc = self;

		QUEUE_LOCK (node->cnc);
		/* Add to a job queue */
		//if we are high priority, we go on the front of the provisioning queue
		if(highpriority){
			self->priv->provisioning_jobs = g_slist_prepend (self->priv->provisioning_jobs, (gpointer *) node);
			g_debug ("eas_connection_send_request : provisioning job++ queuelength=%d", g_slist_length (self->priv->provisioning_jobs));
		}else{
			//if we are currently reprovisioning, then add to the provisioning queue to ensure that 
			//policy gets updated when provisioning is finished
			if(self->priv->reprovisioning){
				self->priv->provisioning_jobs = g_slist_append (self->priv->provisioning_jobs, (gpointer *) node);
				g_debug ("eas_connection_send_request : provisioning job++ queuelength=%d", g_slist_length (self->priv->provisioning_jobs));
			}else{
				self->priv->jobs = g_slist_append (self->priv->jobs, (gpointer *) node);
				g_debug ("eas_connection_send_request : job++ queuelength=%d", g_slist_length (self->priv->jobs));
			}
		}
		QUEUE_UNLOCK (node->cnc);

		eas_trigger_next_request (node->cnc);
	}
finish:
	// @@WARNING - doc must always be freed before exiting this function.

	if (priv->protocol_version < 140 && !g_strcmp0 (cmd, "SendMail")) {
		g_free ( (gchar*) doc);
	} else {
		xmlFreeDoc (doc);
	}

	if (wbxml) free (wbxml);
	if (conv) wbxml_conv_xml2wbxml_destroy (conv);
	if (dataptr) xmlFree (dataptr);

	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_connection_send_request--");
	return ret;
}

typedef enum {
	INVALID = 0,
	VALID_NON_EMPTY,
	VALID_EMPTY,
	VALID_12_1_REPROVISION,
} RequestValidity;


static RequestValidity
isResponseValid (SoupMessage *msg, gboolean multipart, GError **error)
{
	const gchar *content_type = NULL;

	g_debug ("eas_connection - isResponseValid++");


	if (HTTP_STATUS_PROVISION == msg->status_code) {
		g_warning ("Server instructed 12.1 style re-provision");
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_REPROVISION,
			     _("HTTP request failed: %d — %s"),
			     msg->status_code, msg->reason_phrase);
		return VALID_12_1_REPROVISION;
	}


	if (HTTP_STATUS_OK != msg->status_code) {
		g_critical ("Failed with status [%d] : %s", msg->status_code, (msg->reason_phrase ? msg->reason_phrase : "-"));
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("HTTP request failed: %d — %s"),
			     msg->status_code, msg->reason_phrase);
		return INVALID;
	}


	if (FALSE == soup_message_body_get_accumulate (msg->response_body))
	{
		g_debug ("Body collected in GotChunk");
		return VALID_NON_EMPTY;
	}
	
	if (!msg->response_body->length) {
		g_debug ("Empty Content");
		return VALID_EMPTY;
	}

	content_type = soup_message_headers_get_one (msg->response_headers, "Content-Type");

	if (!multipart && (0 != g_strcmp0 ("application/vnd.ms-sync.wbxml", content_type))) {
		g_warning ("  Failed: Content-Type did not match WBXML");
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("HTTP response type was not WBXML"));
		return INVALID;
	}
	if (multipart && (0 != g_strcmp0 ("application/vnd.ms-sync.multipart", content_type))) {
		g_warning ("  Failed: Content-Type did not match mutipart");
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("HTTP response type was not MULTIPART"));
		return INVALID;
	}

	g_debug ("eas_connection - isResponseValid--");

	return VALID_NON_EMPTY;
}


/**
 * Converts from Microsoft ActiveSync Server protocol WBXML to XML
 * @param wbxml input buffer
 * @param wbxml_len input buffer length
 * @param xml output buffer [full transfer]
 * @param xml_len length of the output buffer in bytes
 */
static gboolean
wbxml2xml (const WB_UTINY *wbxml, const WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len, GError **error)
{
	gboolean ret = TRUE;
	WBXMLConvWBXML2XML *conv = NULL;
	WBXMLError wbxml_ret = WBXML_OK;

	g_debug ("eas_connection - wbxml2xml++");

	*xml = NULL;
	*xml_len = 0;

	if (NULL == wbxml) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     _("wbxml is NULL!"));
		g_warning ("  wbxml is NULL!");
		ret = FALSE;
		goto finish;
	}

	if (0 == wbxml_len) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     _("wbxml_len is 0!"));
		g_warning ("  wbxml_len is 0!");
		ret = FALSE;
		goto finish;
	}

	wbxml_ret = wbxml_conv_wbxml2xml_create (&conv);

	if (wbxml_ret != WBXML_OK) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     _("Failed to create WBXML to XML converter: %s"),
			     wbxml_errors_string (wbxml_ret));
		g_warning ("  Failed to create conv! %s", wbxml_errors_string (wbxml_ret));
		ret = FALSE;
		goto finish;
	}

	wbxml_conv_wbxml2xml_set_language (conv, WBXML_LANG_ACTIVESYNC);
	wbxml_conv_wbxml2xml_set_indent (conv, 2);

	wbxml_ret = wbxml_conv_wbxml2xml_run (conv,
					      (WB_UTINY *) wbxml,
					      wbxml_len,
					      xml,
					      xml_len);

	if (WBXML_OK != wbxml_ret) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     _("Error converting WBXML to XML: %s"),
			     wbxml_errors_string (wbxml_ret));
		g_warning ("  wbxml2xml Ret = %s", wbxml_errors_string (wbxml_ret));
		ret = FALSE;
	}

finish:
	if (conv) wbxml_conv_wbxml2xml_destroy (conv);

	return ret;
}

/**
 * @param wbxml binary XML to be dumped
 * @param wbxml_len length of the binary
 */
static void
dump_wbxml_response (const WB_UTINY *wbxml, const WB_LONG wbxml_len)
{
	WB_UTINY *xml = NULL;
	WB_ULONG xml_len = 0;
	gchar* tmp = NULL;

	if (0 != wbxml_len) {
		wbxml2xml (wbxml, wbxml_len, &xml, &xml_len, NULL);
		tmp = g_strndup ( (gchar*) xml, xml_len);
		g_debug ("\n=== dump start: xml_len [%d] ===\n%s=== dump end ===", xml_len, tmp);
		if (tmp) g_free (tmp);
		if (xml) free (xml);
	} else {
		g_warning ("No WBXML to decode");
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Autodiscover
//
////////////////////////////////////////////////////////////////////////////////

static xmlDoc *
autodiscover_as_xml (const gchar *email)
{
	xmlDoc *doc;
	xmlNode *node;
	xmlNs *ns;

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	node = xmlNewDocNode (doc, NULL, (xmlChar *) "Autodiscover", NULL);
	xmlDocSetRootElement (doc, node);
	ns = xmlNewNs (node, (xmlChar *) "http://schemas.microsoft.com/exchange/autodiscover/mobilesync/requestschema/2006", NULL);
	node = xmlNewChild (node, ns, (xmlChar *) "Request", NULL);
	xmlNewChild (node, ns, (xmlChar *) "EMailAddress", (xmlChar *) email);
	xmlNewChild (node, ns, (xmlChar *) "AcceptableResponseSchema",
			     (xmlChar *) "http://schemas.microsoft.com/exchange/autodiscover/mobilesync/responseschema/2006");

	return doc;
}

/**
 * @return NULL or serverUri as a NULL terminated string, caller must free
 *		   with xmlFree(). [full transfer]
 *
 */
static gchar*
autodiscover_parse_protocol (xmlNode *node)
{
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE &&
		    !g_strcmp0 ( (char *) node->name, "Url")) {
			char *asurl = (char *) xmlNodeGetContent (node);
			if (asurl)
				return asurl;
		}
	}
	return NULL;
}

typedef struct {
	EasConnection *cnc;
	GSimpleAsyncResult *simple;
	SoupMessage *msgs[2];
	DBusGMethodInvocation* context;
	EasAccount* account;
} EasAutoDiscoverData;

static void
autodiscover_soup_cb (SoupSession *session, SoupMessage *msg, gpointer data)
{
	GError *error = NULL;
	EasAutoDiscoverData *adData = data;
	guint status = msg->status_code;
	xmlDoc *doc = NULL;
	xmlNode *node = NULL;
	gchar *serverUrl = NULL;
	gint idx = 0;

	g_debug ("autodiscover_soup_cb++");

	for (; idx < 2; ++idx) {
		if (adData->msgs[idx] == msg)
			break;
	}

	if (idx == 2) {
		g_debug ("Request got cancelled and removed");
		return;
	}

	adData->msgs[idx] = NULL;

	if (status != HTTP_STATUS_OK) {
		g_warning ("Autodiscover HTTP response was not OK");
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Status code: %d — Response from server"),
			     status);
		goto failed;
	}

	doc = xmlReadMemory (msg->response_body->data,
			     msg->response_body->length,
			     "autodiscover.xml",
			     NULL,
			     XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

	if (!doc) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Failed to parse autodiscover response XML"));
		goto failed;
	}

	node = xmlDocGetRootElement (doc);
	if (g_strcmp0 ( (gchar*) node->name, "Autodiscover")) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Failed to find <Autodiscover> element"));
		goto failed;
	}
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) node->name, "Response"))
			break;
	}
	if (!node) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Failed to find <Response> element"));
		goto failed;
	}
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) node->name, "Action"))
			break;
	}
	if (!node) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Failed to find <Action> element"));
		goto failed;
	}
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) node->name, "Settings"))
			break;
	}
	if (!node) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Failed to find <Settings> element"));
		goto failed;
	}
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE &&
		    !g_strcmp0 ( (gchar*) node->name, "Server") &&
		    (serverUrl = autodiscover_parse_protocol (node)))
			break;
	}

	for (idx = 0; idx < 2; ++idx) {
		if (adData->msgs[idx]) {
			SoupMessage *m = adData->msgs[idx];
			adData->msgs[idx] = NULL;
			soup_session_cancel_message (adData->cnc->priv->soup_session,
						     m,
						     SOUP_STATUS_CANCELLED);
			g_debug ("Autodiscover success - Cancelling outstanding msg[%d]", idx);
		}
	}
	// Copy the pointer here so we can free using g_free() rather than xmlFree()
	g_simple_async_result_set_op_res_gpointer (adData->simple, g_strdup (serverUrl), NULL);
	if (serverUrl) xmlFree (serverUrl);
	g_simple_async_result_complete_in_idle (adData->simple);

	g_debug ("autodiscover_soup_cb (Success)--");
	return;

failed:
	for (idx = 0; idx < 2; ++idx) {
		if (adData->msgs[idx]) {
			/* Clear this error and hope the second one succeeds */
			g_warning ("First autodiscover attempt failed");
			g_clear_error (&error);
			return;
		}
	}

	/* This is returning the last error, it may be preferable for the
	 * first error to be returned.
	 */
	g_simple_async_result_set_from_error (adData->simple, error);
	g_simple_async_result_complete_in_idle (adData->simple);

	g_debug ("autodiscover_soup_cb (Failed)--");
}

static void
autodiscover_simple_cb (GObject *cnc, GAsyncResult *res, gpointer data)
{
	EasAutoDiscoverData *adData = data;
	GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (res);
	GError *error = NULL;
	gchar *url = NULL;

	g_debug ("autodiscover_simple_cb++");

	if (!g_simple_async_result_propagate_error (simple, &error)) {
		url = g_simple_async_result_get_op_res_gpointer (simple);
	}

	// Url ownership transferred to dbus
	dbus_g_method_return( adData->context,
			url);
	g_object_unref (G_OBJECT (adData->cnc));
	g_object_unref (G_OBJECT (adData->account));
	g_free (adData);

	g_debug ("autodiscover_simple_cb--");
}


static SoupMessage *
autodiscover_as_soup_msg (gchar *url, xmlOutputBuffer *buf)
{
	SoupMessage *msg = NULL;

	g_debug ("autodiscover_as_soup_msg++");

	msg = soup_message_new ("POST", url);

	soup_message_headers_append (msg->request_headers,
				     "User-Agent", "libeas");

	soup_message_set_request (msg,
				  "text/xml",
				  SOUP_MEMORY_COPY,
#ifdef LIBXML2_NEW_BUFFER
				  (gchar *) xmlOutputBufferGetContent(buf),
				  xmlOutputBufferGetSize(buf));
#else
				  (gchar*) buf->buffer->content,
				  buf->buffer->use);
#endif

	g_debug ("autodiscover_as_soup_msg--");
	return msg;
}

////////////////////////////////////////////////////////////////////////////////
//
// Public functions
//
////////////////////////////////////////////////////////////////////////////////

/**
 * @param[in] cb
 *	  Autodiscover response callback
 * @param[in] cb_data
 *	  Data to be passed into the callback
 * @param[in] email
 *	  User's exchange email address
 * @param[in] username
 *	  Exchange Server username in the format DOMAIN\username
 */
void
eas_connection_autodiscover (const gchar* email,
			     const gchar* username,
                             DBusGMethodInvocation* context)
{
	GError *error = NULL;
	gchar* domain = NULL;
	EasConnection *cnc = NULL;
	xmlDoc *doc = NULL;
	xmlOutputBuffer* txBuf = NULL;
	gchar* url = NULL;
	EasAutoDiscoverData *autoDiscData = NULL;
	EasAccount *account = NULL;

	g_debug ("eas_connection_autodiscover++");

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
	dbus_g_thread_init();

	if (!email) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Email is mandatory and must be provided"));
		dbus_g_method_return_error (context, error);
		return;
	}

	domain = strchr (email, '@');
	if (! (domain && *domain)) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("Failed to extract domain from email address"));
		dbus_g_method_return_error (context, error);
		return;
	}
	++domain; // Advance past the '@'

	account = eas_account_new();
	if (!account) {
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     _("Failed create temp account for autodiscover"));
		dbus_g_method_return_error (context, error);
		return;
	}
#if 0
	account->uid = g_strdup (email);
	account->serverUri = g_strdup ("autodiscover");
#endif

	eas_account_set_uid (account, email);
	eas_account_set_uri (account, "autodiscover");

	if (!username) { // Use the front of the email as the username
		gchar **split = g_strsplit (email, "@", 2);
		eas_account_set_username (account, split[0]);
		g_strfreev (split);
	} else { // Use the supplied username
		eas_account_set_username (account, username);
	}

	cnc = eas_connection_new (account, &error);

	if (!cnc) {
		dbus_g_method_return_error (context, error);
		g_object_unref (account);
		return;
	}

	doc = autodiscover_as_xml (email);
	txBuf = xmlAllocOutputBuffer (NULL);
	xmlNodeDumpOutput (txBuf, doc, xmlDocGetRootElement (doc), 0, 1, NULL);
	xmlOutputBufferFlush (txBuf);

	autoDiscData = g_new0 (EasAutoDiscoverData, 1);
	autoDiscData->context = context;
	autoDiscData->cnc = cnc;
	autoDiscData->account = account;
	autoDiscData->simple = g_simple_async_result_new (G_OBJECT (cnc),
							  autodiscover_simple_cb,
							  autoDiscData,
							  eas_connection_autodiscover);

	// URL formats to be tried
	g_debug ("Building message one");
	// 1 - https://<domain>/autodiscover/autodiscover.xml
	url = g_strdup_printf ("https://%s/autodiscover/autodiscover.xml", domain);
	autoDiscData->msgs[0] = autodiscover_as_soup_msg (url, txBuf);
	g_free (url);

	g_debug ("Building message two");
	// 2 - https://autodiscover.<domain>/autodiscover/autodiscover.xml
	url = g_strdup_printf ("https://autodiscover.%s/autodiscover/autodiscover.xml", domain);
	autoDiscData->msgs[1] = autodiscover_as_soup_msg (url, txBuf);
	g_free (url);

	soup_session_queue_message (cnc->priv->soup_session,
				    autoDiscData->msgs[0],
				    autodiscover_soup_cb,
				    autoDiscData);

	soup_session_queue_message (cnc->priv->soup_session,
				    autoDiscData->msgs[1],
				    autodiscover_soup_cb,
				    autoDiscData);

	g_object_unref (cnc); // GSimpleAsyncResult holds this now
	xmlOutputBufferClose (txBuf);
	xmlFreeDoc (doc);

	g_debug ("eas_connection_autodiscover--");
}

EasConnection*
eas_connection_find (const gchar* accountId)
{
	EasConnection *cnc = NULL;
	GError *error = NULL;
	EIterator *iter = NULL;
	gboolean account_found = FALSE;
	EasAccount* account = NULL;

	g_debug ("eas_connection_find++ : account_uid[%s]",
		 (accountId ? accountId : "NULL"));

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
	dbus_g_thread_init();

	if (!accountId) return NULL;

	eas_connection_accounts_init();

	iter = e_list_get_iterator (E_LIST (g_account_list));
	for (; e_iterator_is_valid (iter);  e_iterator_next (iter)) {
		account = EAS_ACCOUNT (e_iterator_get (iter));
		g_debug ("account->uid=%s\n", eas_account_get_uid (account));
		if (strcmp (eas_account_get_uid (account), accountId) == 0) {
			account_found = TRUE;
			break;
		}
	}
	g_object_unref (iter);

	if (!account_found) {
		g_warning ("No account details found for accountId [%s]", accountId);
		return NULL;
	}

	g_mutex_lock (&connection_list);
	if (g_open_connections) {
		gchar *hashkey = g_strdup_printf ("%s@%s@%s",
						  eas_account_get_username (account),
						  eas_account_get_uri (account),
						  eas_account_get_device_id (account));

		cnc = g_hash_table_lookup (g_open_connections, hashkey);
		g_free (hashkey);

		if (EAS_IS_CONNECTION (cnc)) {
			g_object_ref (cnc);
			g_mutex_unlock (&connection_list);
			g_debug ("eas_connection_find (Found) --");
			return cnc;
		}
	}
	g_mutex_unlock (&connection_list);

	cnc = eas_connection_new (account, &error);
	if (cnc) {
		g_debug ("eas_connection_find (Created) --");
	} else {
		if (error) {
			g_warning ("[%s][%d][%s]",
				   g_quark_to_string (error->domain),
				   error->code,
				   error->message);
			g_error_free (error);
		}
		g_warning ("eas_connection_find (Failed to create connection) --");
	}

	return cnc;
}


EasConnection*
eas_connection_new (EasAccount* account, GError** error)
{
	EasConnection *cnc = NULL;
	EasConnectionPrivate *priv = NULL;
	gchar *hashkey = NULL;
	gchar *cachedir;

	g_debug ("eas_connection_new++");

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
	dbus_g_thread_init();

	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	if (!account) {
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADARG,
			     _("An account must be provided."));
		return NULL;
	}

	g_debug ("Checking for open connection");
	g_mutex_lock (&connection_list);
	if (g_open_connections) {
		hashkey = g_strdup_printf ("%s@%s@%s", eas_account_get_username (account), eas_account_get_uri (account), eas_account_get_device_id (account));
		cnc = g_hash_table_lookup (g_open_connections, hashkey);
		g_free (hashkey);

		if (EAS_IS_CONNECTION (cnc)) {
			g_object_ref (cnc);
			g_mutex_unlock (&connection_list);
			return cnc;
		}
	}

	g_debug ("No existing connection, create new one");
	cnc = g_object_new (EAS_TYPE_CONNECTION, NULL);
	priv = cnc->priv;

	if (!cnc) {
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     _("A server url and username must be provided."));
		g_mutex_unlock (&connection_list);
		return NULL;
	}

	cachedir = g_build_filename (g_get_user_cache_dir (),
				     "activesync",
				     eas_account_get_uid (account),
				     NULL);
	if (g_mkdir_with_parents (cachedir, 0700)) {
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FILEERROR,
			     _("Failed to create folder cache directory %s: %s"),
			     cachedir, strerror (errno));
		g_free (cachedir);
		g_object_unref (cnc);
		g_mutex_unlock (&connection_list);
		return NULL;
	}

	priv->folders_keyfile = g_build_filename (cachedir, "folders", NULL);
	g_free (cachedir);
	priv->folders = g_key_file_new ();
	g_key_file_load_from_file (priv->folders, priv->folders_keyfile,
				   G_KEY_FILE_NONE, NULL);

	priv->protocol_version = eas_account_get_protocol_version (account);
	if (!priv->protocol_version)
		priv->protocol_version = AS_DEFAULT_PROTOCOL;

	priv->proto_str = g_strdup_printf ("%d.%d", priv->protocol_version / 10,
					   priv->protocol_version % 10);
	g_debug ("protocol version = %s", priv->proto_str);

	// Just a reference to the global account list
	priv->account = account;

	hashkey = g_strdup_printf ("%s@%s@%s", eas_account_get_username (account), eas_account_get_uri (account), eas_account_get_device_id (account));

	if (!g_open_connections) {
		g_debug ("Creating hashtable");
		g_open_connections = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	}

	g_debug ("Adding to hashtable");
	g_hash_table_insert (g_open_connections, hashkey, cnc);

	g_mutex_unlock (&connection_list);
	g_debug ("eas_connection_new--");
	return cnc;
}

static void
parse_for_status (xmlNode *node, gboolean *isErrorStatus)
{
	xmlNode *child = NULL;
	xmlNode *sibling = NULL;

	*isErrorStatus = FALSE;

	if (!node) return;
	if ( (child = node->children)) {
		parse_for_status (child, isErrorStatus);
	}

	if (*isErrorStatus == FALSE && node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
		gchar* status = (gchar*) xmlNodeGetContent (node);
		if (!g_strcmp0 (status, "1") || (!g_strcmp0 (status, "3") && !g_strcmp0 ( (gchar *) node->parent->name, "Response"))) {
			g_message ("parent_name[%s] status = [%s]", (char*) node->parent->name , status);
		} else {
			g_critical ("parent_name[%s] status = [%s]", (char*) node->parent->name , status);
			*isErrorStatus = TRUE;
		}
		xmlFree (status);
	}

	if ( (sibling = node->next) && *isErrorStatus == FALSE) {
		parse_for_status (sibling, isErrorStatus);
	}
}

static WB_UTINY *sanitize_utf8(WB_UTINY *in)
{
	int i;
	WB_UTINY *out = in;

	/* Validate and convert any invalid bytes to the replacement
	   character U+FFFD �. */
	for (i=0; out[i]; i++) {
		int nr_more;
		int j;

		if (!(out[i] & 0x80))
			continue;
		if ((out[i] & 0xc0) == 0x80)
			goto invalid;
		if ((out[i] & 0xe0) == 0xc0) {
			if (out[i] < 0xc2) /* non-canonical < 0x80 */
				goto invalid;
			nr_more = 1;
		} else if ((out[i] & 0xf0) == 0xe0) {
			if (out[i] == 0xe0 && out[i+1] < 0xa0) /* non-canonical < 0x800 */
				goto invalid;
			nr_more = 2;
		} else if ((out[i] & 0xf8) == 0xf0) {
			if (out[i] == 0xf0 && out[i+1] < 0x90) /* non-caninical < 0x10000 */
				goto invalid;
			nr_more = 3;
		} else
			goto invalid;

		for (j = 0; j < nr_more; j++)
			if ((out[i+j+1] & 0xc0) != 0x80)
				goto invalid;
		i += nr_more;
		continue;

	invalid:
		g_debug("byte %x is invalid", out[i]);
		out = realloc((char *)out, strlen((char *)out) + 3);
		g_debug("move out[%d]=%02x to out[%d]=%02x",
			i, out[i], i+3, out[i+3]);
		memmove (out + i + 3, out + i + 1, strlen((char *)out + i));
		out[i++] = 0xef;
		out[i++] = 0xbf;
		out[i] = 0xbd;
	}

	return (WB_UTINY *)out;
}

void
handle_server_response (SoupSession *session, SoupMessage *msg, gpointer data)
{
	EasNode *node = (EasNode *)data;
	EasRequestBase *req = EAS_REQUEST_BASE (node->request);
	EasConnection *self = EAS_CONNECTION (node->cnc);
	EasConnectionPrivate *priv = EAS_CONNECTION_PRIVATE (self);
	xmlDoc *doc = NULL;
	WB_UTINY *xml = NULL;
	WB_ULONG xml_len = 0;
	GError *error = NULL;
	RequestValidity validity = FALSE;
	gboolean cleanupRequest = FALSE;
	gboolean isProvisioningRequired = FALSE;

	eas_active_job_dequeue(node->cnc, node);

    g_debug ("eas_connection - handle_server_response++ node [%p], req [%p], self [%p], priv[%p]", node, req, self, priv);
	// if request has been cancelled, complete with appropriate error
	if(eas_request_base_IsCancelled(req))
	{
		g_debug("request was cancelled by caller");
		g_set_error (&error, 
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_CANCELLED,
			     _("request was cancelled by user"));

	    goto complete_request;
	}
	
	validity = isResponseValid (msg, eas_request_base_UseMultipart (req),  &error);

	//if we get a reprovision error set flag - and move request to provisioning queue
	if (VALID_12_1_REPROVISION == validity) {
		//add reference to msg, so that it doesn't get nuked after handle_server_response completes
		g_object_ref(msg);
               isProvisioningRequired = TRUE;
		self->priv->reprovisioning = TRUE;
		self->priv->provisioning_jobs = g_slist_append(self->priv->provisioning_jobs, (gpointer *)node);
        }

	if (INVALID == validity || VALID_12_1_REPROVISION == validity ) 
	{
		g_assert (error != NULL);
		if (INVALID == validity &&
		    getenv ("EAS_CAPTURE_RESPONSE") && (atoi (g_getenv ("EAS_CAPTURE_RESPONSE")) >= 1))

		{
			write_response_to_file ( (WB_UTINY*) msg->response_body->data, msg->response_body->length);
		}
		goto complete_request;
	}

	if (!g_mock_response_list && getenv ("EAS_DEBUG") && atoi (g_getenv ("EAS_DEBUG")) >= 5) 
	{
		dump_wbxml_response ( (WB_UTINY*) msg->response_body->data, msg->response_body->length);
	}

	if (VALID_NON_EMPTY == validity) {
		gboolean isStatusError = FALSE;

		// @@TRICKY - If we have entries in the mocked body list,
		//            feed that response back instead of the real server
		//            response.
		if (g_mock_response_list) {
			gchar *filename = g_slist_nth_data (g_mock_response_list, 0);
			gchar curPath[FILENAME_MAX];
			gchar *fullPath = NULL;
			g_mock_test = TRUE;

			memset (curPath, '\0', sizeof (curPath));

			if (!getcwd (curPath, sizeof (curPath)))
				return;

			curPath[FILENAME_MAX - 1] = '\0';

			fullPath = g_strconcat (curPath,
						"/check_tests/TestData/Mocked_Negative_Tests/",
						filename,
						NULL);

			g_debug ("Queued mock responses [%u]", g_slist_length (g_mock_response_list));

			g_mock_response_list = g_slist_remove (g_mock_response_list, filename);
			g_free (filename);

			g_debug ("Loading mock:[%s]", fullPath);

			doc = xmlReadFile (fullPath,
					   NULL,
					   0);
			if (!doc) {
				g_critical ("Failed to read mock response!");
			}
			g_free (fullPath);

			g_object_unref (msg);
			g_free(node);
		} else {
			WB_UTINY* wbxml = NULL;
			WB_ULONG wbxml_length = 0;
			
			if ( (wbxml = (WB_UTINY *)eas_request_base_GetWbxmlFromChunking (req)) )
			{
				wbxml_length = eas_request_base_GetWbxmlFromChunkingSize (req);
				dump_wbxml_response (wbxml, wbxml_length);
			}
			else
			{
				wbxml = (WB_UTINY *) msg->response_body->data;
				wbxml_length = msg->response_body->length;
			}
			
			if (!wbxml2xml ( wbxml ,
					 wbxml_length,
					 &xml,
					 &xml_len, &error))
				goto complete_request;

			/* Exchange has a habit of returning invalid data, and libxml does
			   strange things if we set XML_PARSE_RECOVER. So process it in
			   advance and turn any invalid bytes to � characters. This doesn't
			   break fetches of email which is *correctly* represented in legacy
			   8-bit charsets, because those are handled as multi-part fetches.
			   Anything we get here is going to be passed out to dbus in a string
			   type, and dbus would end up silently calling exit() if we pass it
			   non-UTF8 anyway */
			if (xml) {
				xml = sanitize_utf8 (xml);
				xml_len = strlen((char *)xml);
			}
			if (getenv ("EAS_CAPTURE_RESPONSE") && (atoi (g_getenv ("EAS_CAPTURE_RESPONSE")) >= 1)) {
				write_response_to_file (xml, xml_len);
			}

			g_debug ("handle_server_response - performing xmlReadMemory");

			// Otherwise proccess the server response
			doc = xmlReadMemory ( (const char*) xml,
					      xml_len,
					      "sync.xml",
					      NULL,
					      XML_PARSE_RECOVER|XML_PARSE_NOERROR);
		}

		if (doc) {
			xmlNode* node = xmlDocGetRootElement (doc);
			g_debug ("handle_server_response - doc created - parse for status");
			parse_for_status (node, &isStatusError); // TODO Also catch provisioning for 14.0
		} else {
			xmlError *xmlerr = xmlGetLastError ();
			g_set_error (&error,
				     EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_XMLTOOLARGETODOM,
				     _("Failed to parse XML: %s"),
				     xmlerr?xmlerr->message:"<unknown error>");
		}


		if (TRUE == isStatusError || !doc) {
			if (!getenv ("EAS_CAPTURE_RESPONSE") || (getenv ("EAS_CAPTURE_RESPONSE") && (atoi (g_getenv ("EAS_CAPTURE_RESPONSE")) == 0))) {
				write_response_to_file (xml, xml_len);
			}
		}

		if (xml) free (xml);
	}


complete_request:
	if(isProvisioningRequired){
		//createa a new internal provision req
		EasProvisionReq *prov_req = eas_provision_req_new (TRUE, NULL, NULL, NULL);
                g_debug ("  handle_server_response - parsed provisioning required");
		eas_request_base_SetConnection (&prov_req->parent_instance, self);
                //activate provisioning request.
                eas_provision_req_Activate (prov_req, NULL);   // TODO check return
		cleanupRequest = FALSE;
	}else{
		g_debug ("  handle_server_response - no parsed provisioning required");
		g_debug ("  handle_server_response - Handling request [%d]", eas_request_base_GetRequestType (req));

	
		cleanupRequest = eas_request_base_MessageComplete (req, doc, error);
	}

	//if cleanupRequest is set - we are done with this request, and should clean it up
	if (cleanupRequest) {
		g_object_unref (req);
	}
	//also need to clean up the multipart data
	g_slist_foreach (priv->multipart_strings_list, (GFunc) g_free, NULL);
	g_slist_free (priv->multipart_strings_list);
	priv->multipart_strings_list = NULL;

	g_debug ("Queued mock responses [%u]", g_slist_length (g_mock_response_list));

	if(g_mock_test){
		g_debug ("eas_connection - (mock test)handle_server_response--");
		return;
		}
	if(!self->priv->reprovisioning){
		eas_active_job_done (node->cnc, node);
	}
	g_debug ("eas_connection - handle_server_response--");
}

static void
write_response_to_file (const WB_UTINY* xml, WB_ULONG xml_len)
{
	static gulong sequence_number = 0;
	FILE *file = NULL;
	gchar *path = g_strdup_printf ("%s/eas-debug/", g_getenv ("HOME"));
	gchar *filename = NULL;
	gchar *fullPath = NULL;

	if (!sequence_number) {
		mkdir (path, S_IFDIR | S_IRWXU | S_IXGRP | S_IXOTH);
	}
	sequence_number++;

	filename = g_strdup_printf ("%010lu.%lu.xml", (gulong) sequence_number, (gulong) getpid ());
	fullPath = g_strconcat (path, filename, NULL);

	if ( (file = fopen (fullPath, "w"))) {
		if (xml)
			fwrite (xml, 1, xml_len, file);
		fclose (file);
	}

	g_free (path);
	g_free (filename);
	g_free (fullPath);
}

void
eas_connection_add_mock_responses (const gchar** response_file_list, const GArray *mock_soup_status_codes)
{
	const gchar* filename = NULL;
	guint index = 0;
	guint status_code;

	g_debug ("eas_connection_add_mock_responses++");

	if (!response_file_list) return;

	while ( (filename = response_file_list[index++])) {
		g_debug ("Adding [%s] to the mock list", filename);
		g_mock_response_list = g_slist_append (g_mock_response_list, g_strdup (filename));
	}

	if (!mock_soup_status_codes) return;

	if (NULL == g_mock_status_codes) {
		g_mock_status_codes = g_array_new (FALSE, FALSE, sizeof (guint));
	}

	for (index = 0; index < mock_soup_status_codes->len; index ++) {
		status_code = g_array_index (mock_soup_status_codes, guint, index);
		g_debug ("adding mock status code %d", status_code);
		g_array_append_val (g_mock_status_codes, status_code);
	}

	g_debug ("eas_connection_add_mock_responses--");
}

gchar*
eas_connection_get_multipartdata (EasConnection* self, guint partID)
{
	return g_slist_nth_data (self->priv->multipart_strings_list, partID);
}

void
eas_connection_update_folders (void *self, const gchar *ret_sync_key,
			       GSList *added_folders, GSList *updated_folders,
			       GSList *deleted_folders, GError *error)
{
	EasConnection *cnc = self;
	EasFolder *folder;
	GSList *l;
	GFile *file;
	gsize size;
	gchar *contents;

	if (error)
		return;

	for (l = deleted_folders; l; l = l->next) {
		folder = l->data;
		g_key_file_remove_group (cnc->priv->folders, folder->folder_id, NULL);
	}

	for (l = added_folders; l; l = l->next) {
		folder = l->data;
		g_key_file_set_string (cnc->priv->folders, folder->folder_id, "parent_id", folder->parent_id);
		g_key_file_set_string (cnc->priv->folders, folder->folder_id, "display_name", folder->display_name);
		g_key_file_set_integer (cnc->priv->folders, folder->folder_id, "type", folder->type);
	}

	for (l = updated_folders; l; l = l->next) {
		folder = l->data;
		g_key_file_set_string (cnc->priv->folders, folder->folder_id, "parent_id", folder->parent_id);
		g_key_file_set_string (cnc->priv->folders, folder->folder_id, "display_name", folder->display_name);
		g_key_file_set_integer (cnc->priv->folders, folder->folder_id, "type", folder->type);
	}

	g_key_file_set_string (cnc->priv->folders, "##storedata", "synckey", ret_sync_key);

	contents = g_key_file_to_data (cnc->priv->folders, &size, NULL);

	file = g_file_new_for_path (cnc->priv->folders_keyfile);
	g_file_replace_contents (file, contents, size, NULL, FALSE,
				 G_FILE_CREATE_PRIVATE, NULL, NULL, NULL);
	g_free (contents);
	g_object_unref (file);
}

gchar *
eas_connection_get_folder_sync_key (EasConnection *cnc)
{
	return g_key_file_get_string (cnc->priv->folders, "##storedata", "synckey", NULL);
}

gchar **eas_connection_get_folders (EasConnection *cnc)
{
	gchar **folders = g_key_file_get_groups (cnc->priv->folders, NULL);
	int i, j = 0;

	/* We re-use the original array, dropping the ##storedata element */
	for (i = 0; folders[i]; i++) {
		gchar *res = NULL;

		if (strcmp (folders[i], "##storedata")) {
			gchar *parent_id, *display_name;
			int type;

			parent_id = g_key_file_get_string (cnc->priv->folders,
						   folders[i], "parent_id", NULL);
			display_name = g_key_file_get_string (cnc->priv->folders,
							      folders[i], "display_name", NULL);
			type = g_key_file_get_integer (cnc->priv->folders,
						       folders[i], "type", NULL);

			res = g_strdup_printf ("%s\n%s\n%s\n%d", parent_id,
					       folders[i], display_name, type);
			g_free (parent_id);
			g_free (display_name);
		}

		g_free (folders[i]);
		folders[i] = NULL;

		/* Reuse the existing array */
		if (res)
			folders[j++] = res;
	}
	return folders;
}

gboolean 
eas_connection_cancel_request(EasConnection* cnc, 
                                     guint request_id, 
                                     GError** error)
{
	GSList *l;
	EasNode *node;
	gboolean found = FALSE;
	EasConnectionPrivate *priv = cnc->priv;
	EasRequestBase *request;
	gboolean ret = TRUE;
	
	QUEUE_LOCK (cnc);

	// is the request in the jobs queue?
	for(l = priv->jobs; l != NULL; l = l->next)
	{
		node = (EasNode *) l->data;
		request = node->request;
		
		if(eas_request_base_GetRequestId(request) == request_id)
		{
			GSource *source = NULL;
			EasNode* node = eas_node_new ();
			node->request = request;
			node->cnc = cnc;			
			
			g_debug("found the request in the jobs queue");
			found = TRUE;

			// set to cancelled
			eas_request_base_Cancelled(request);
			
			// remove it from the queue, 
			priv->jobs = g_slist_remove (priv->jobs, (gconstpointer *) node);
			
			// call handle_server_response from soup thread 
			source = g_idle_source_new ();
			g_source_set_callback (source, call_handle_server_response, node, NULL);
			g_source_attach (source, priv->soup_context);			

			break; // out of for loop
		}
	}
	if(!found)// not in the jobs queue
	{
		g_debug("look for the request in the active queue");
		// is the request in the active jobs queue?
		for(l = priv->active_job_queue; l != NULL; l = l->next)
		{
			node = (EasNode *) l->data;
			request = node->request;
			
			if(eas_request_base_GetRequestId(request) == request_id)
			{
				GSource *source;
				g_debug("found the request in the active queue");
				found = TRUE;

				// set to cancelled
				eas_request_base_Cancelled(request);

				// tell soup to cancel (nb: we need to do this from the soup thread)
				// For messages queued with soup_session_queue_message() and cancelled from the same thread, 
				// the callback will be invoked before soup_session_cancel_message() returns
				// call_soup_session_cancel_message(request);
				source = g_idle_source_new ();
				g_source_set_priority(source, G_PRIORITY_HIGH);
				g_source_set_callback (source, call_soup_session_cancel_message, request, NULL);
				g_source_attach (source, priv->soup_context);	
				break;  // out of for loop
			}
		}
	}

	if(!found)
	{
		ret = FALSE;
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADARG,
			     _("Request with id %d not in queue; can’t cancel"),
			     request_id);
	}
	QUEUE_UNLOCK (cnc);

	return ret;
}

/*
void
handle_options_response (SoupSession *session, SoupMessage *msg, gpointer data)
{
	EasConnection *cnc = (EasConnection*)data;
	EasConnectionPrivate *priv = cnc->priv;
	EasAccount *acc = priv->account;
	const gchar *protocol_versions = NULL;
	
	g_debug("handle_options_response++");
	
	// parse response store the list in GSettings (via EasAccount)
	if (HTTP_STATUS_OK != msg->status_code) {
		g_critical ("Failed with status [%d] : %s", msg->status_code, (msg->reason_phrase ? msg->reason_phrase : "-"));
	}	

	// get supported protocols
	protocol_versions = soup_message_headers_get_one(msg->response_headers, "MS-ASProtocolVersions");

	g_debug("server supports protocols %s", protocol_versions);

	// write the list to GSettings using new EasAccount API
	eas_account_set_server_protocols(acc, protocol_versions);

	eas_account_list_save_item (g_account_list,
				    priv->account,
				    EAS_ACCOUNT_SERVER_PROTOCOLS);

	g_debug("handle_options_response++");	
}
*/

// TODO This is a temporary workaround until we're going via the client library/dbus
static void
options_connection_authenticate (SoupSession *sess,
			 SoupMessage *msg,
			 SoupAuth *auth,
			 gboolean retrying,
			 gpointer data)
{
	EasConnection* cnc = (EasConnection *) data;
	const gchar * username = eas_account_get_username (cnc->priv->account);

	gchar* password = NULL;

	g_debug ("options_connection_authenticate++");

	// @@FIX ME - Temporary grab of password from GSettings
	password = eas_account_get_password (cnc->priv->account);

	soup_auth_authenticate (auth,
				username,
				password);
	g_debug ("  eas_connection - connection_authenticate--");
}				

/* 
 TODO - this isn't the right way to do this:
 currently uses a temporary authenticate callback 
 to avoid using the asynchronous libsecret functions
 It will be necessary to add an equivalent function to the client libraries 
	 rather than providing this 'shortcut' straight to easconnection. 
 
 If we connect to the authenticate signal, then connection_authenticate will
 hang since soup_session_send_message is being called from (and blocking) 
 the same thread that the libsecret stuff uses the idle loop of.
 
 use the HTTP OPTIONS command to ask server for a list of protocols
 store results in GSettings 
 */
gboolean 
eas_connection_fetch_server_protocols (EasConnection *cnc, GError **error)
{
	gboolean ret = TRUE;
	SoupMessage *msg = NULL;
	EasConnectionPrivate *priv = cnc->priv;
	EasAccount *acc = priv->account;
	const gchar *protocol_versions = NULL;
	SoupSession* soup_session;
	GSList *proto_vers = NULL;     // list of protocol versions supported by server (eg 120, 121, 140)
	gchar **strv = NULL;    // array of protocol versions supported by server (eg "12.0", "12.1")
	guint len, i;
	gdouble proto_ver;      // eg 12.1
	gint proto_ver_int;     // eg 120
	gchar *endptr;
	
	// need our own synchronous session so we can use the sync soup call
	soup_session = soup_session_sync_new_with_options (SOUP_SESSION_USE_NTLM,
						     TRUE,
						     SOUP_SESSION_TIMEOUT,
						     120,
						     NULL);	

	// since we've created a new soup session, we'll need to authenticate
	g_signal_connect (soup_session,
			  "authenticate",
			  G_CALLBACK (options_connection_authenticate),
			  cnc);	
	
	msg = soup_message_new("OPTIONS", eas_account_get_uri(acc));

	soup_message_headers_append (msg->request_headers,
				     "MS-ASProtocolVersion",
				     priv->proto_str);
	soup_message_headers_append (msg->request_headers,
				     "User-Agent",
				     "libeas");
	
	// use the sync version of soup request and parse msg on completion
	// rather than go via handle_server_response which requires setting up
	// eas request and message objects
	g_debug("send options message");
	soup_session_send_message(soup_session, msg);

	// parse response store the list in GSettings (via EasAccount)
	if (HTTP_STATUS_OK != msg->status_code) {
		g_critical ("Failed with status [%d] : %s", msg->status_code, (msg->reason_phrase ? msg->reason_phrase : "-"));
		g_set_error (error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FAILED,
			     _("HTTP request failed: %d — %s"),
			     msg->status_code, msg->reason_phrase);
		ret = FALSE;
		goto cleanup;
	}	

	// get supported protocols
	protocol_versions = soup_message_headers_get_one(msg->response_headers, "MS-ASProtocolVersions");

	g_debug("server supports protocols %s", protocol_versions);

	// convert to list of ints
	strv = g_strsplit (protocol_versions, ",", 0);
	len = g_strv_length (strv);
	for(i = 0; i < len; i++)
	{       // nb: cast to float necessary here (why!?) to avoid losing decimal part:       
		proto_ver = (gfloat)(g_strtod(strv[i], &endptr)) * 10.0;
		//g_debug("proto_ver = %f", proto_ver);
		proto_ver_int = proto_ver;
		//g_debug("appending %d", proto_ver_int);
		proto_vers = g_slist_append(proto_vers, GINT_TO_POINTER (proto_ver_int));
	}	

	// write the list to GSettings using new EasAccount API
	eas_account_set_server_protocols(acc, proto_vers);

	g_slist_free(proto_vers);
	
	eas_account_list_save_item (g_account_list,
				    priv->account,
				    EAS_ACCOUNT_SERVER_PROTOCOLS);	
	/*
	soup_session_queue_message (priv->soup_session,
				    msg,
				    handle_options_response,
				    cnc);	

	// TODO wait for signal that protocol list has been updated?				    
	*/
	
	
cleanup:
	
	g_object_unref (msg);
	
	return ret;
}

void eas_connection_set_reprovisioning(EasConnection *cnc, gboolean reprovisioning)
{
	cnc->priv->reprovisioning = reprovisioning;
}

void update_policy_key(gpointer job, gpointer policy_key)
{
	EasNode *node = (EasNode*) job;
	SoupMessage* msg = eas_request_base_GetSoupMessage (node->request);
	SoupMessageHeaders *headers = msg->request_headers;

	soup_message_headers_remove(headers, "X-MS-PolicyKey");

	soup_message_headers_append (headers,
				     "X-MS-PolicyKey", 
				     policy_key);
}

void eas_connection_replace_policy_key(EasConnection *cnc)
{
	g_slist_foreach(cnc->priv->provisioning_jobs,
	                update_policy_key,
	                (gpointer*)eas_account_get_policy_key (cnc->priv->account)) ;
}

