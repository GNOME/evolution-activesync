/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include "eas-connection.h"
#include <glib.h>
#include <libsoup/soup.h>

#include <wbxml/wbxml.h>
#include <libedataserver/e-flag.h>
#include <libxml/xmlreader.h> // xmlDoc
#include <time.h>
#include <unistd.h>
#include <gnome-keyring.h>

//#include "eas-accounts.h"
#include "eas-account-list.h"
#include "eas-connection-errors.h"

// List of includes for each request type
#include "eas-sync-folder-hierarchy-req.h"
#include "eas-provision-req.h"
#include "eas-sync-req.h"
#include "eas-get-email-body-req.h"
#include "eas-get-email-attachment-req.h"
#include "eas-send-email-req.h"
#include "eas-delete-email-req.h"
#include "eas-update-calendar-req.h"
#include "eas-update-email-req.h"
#include "eas-add-calendar-req.h"
#include "eas-move-email-req.h"

#ifdef ACTIVESYNC_14
    #define AS_VERSION "14.0"
#else
    #define AS_VERSION "12.1"
#endif

struct _EasConnectionPrivate
{
    SoupSession* soup_session;
    GThread* soup_thread;
    GMainLoop* soup_loop;
    GMainContext* soup_context;

    gchar* accountUid;
    EasAccount *account;

    gchar* policy_key;

	gboolean retrying_asked;

    const gchar* request_cmd;
    xmlDoc* request_doc;
    struct _EasRequestBase* request;
    GError **request_error;
};

#define EAS_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_CONNECTION, EasConnectionPrivate))

static GStaticMutex connection_list = G_STATIC_MUTEX_INIT;
static GHashTable *g_open_connections = NULL;
static GConfClient* g_gconf_client = NULL;
static EasAccountList* g_account_list = NULL;

static void connection_authenticate (SoupSession *sess, SoupMessage *msg,
                                     SoupAuth *auth, gboolean retrying,
                                     gpointer data);
static gpointer eas_soup_thread (gpointer user_data);
static void handle_server_response (SoupSession *session, SoupMessage *msg, gpointer data);
static gboolean wbxml2xml (const WB_UTINY *wbxml, const WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len);
static void parse_for_status (xmlNode *node);

G_DEFINE_TYPE (EasConnection, eas_connection, G_TYPE_OBJECT);

#define HTTP_STATUS_OK (200)

static void
eas_connection_accounts_init()
{
	g_debug("eas_connection_accounts_init++");
	if (!g_gconf_client)
	{
		// At this point we don't have an account Id so just load the list of accounts
		g_gconf_client = gconf_client_get_default();
		if (g_gconf_client == NULL) 
		{
			g_critical("Error Failed to create GConfClient");
			return;
		}
		g_debug("-->created gconf_client");
		
		g_account_list = eas_account_list_new (g_gconf_client);
		if (g_account_list == NULL) 
		{
			g_critical("Error Failed to create account list ");
			return;
		}
		g_debug("-->created account_list");
	}
	g_debug("eas_connection_accounts_init--");
}

static void
eas_connection_init (EasConnection *self)
{
    EasConnectionPrivate *priv;
    self->priv = priv = EAS_CONNECTION_PRIVATE (self);

    g_debug ("eas_connection_init++");

	dbus_g_thread_init();

    priv->soup_context = g_main_context_new ();
    priv->soup_loop = g_main_loop_new (priv->soup_context, FALSE);

    priv->soup_thread = g_thread_create (eas_soup_thread, priv, TRUE, NULL);

    /* create the SoupSession for this connection */
    priv->soup_session =
        soup_session_async_new_with_options (SOUP_SESSION_USE_NTLM,
                                             TRUE,
                                             SOUP_SESSION_ASYNC_CONTEXT,
                                             priv->soup_context,
                                             NULL);

    g_signal_connect (priv->soup_session,
                      "authenticate",
                      G_CALLBACK (connection_authenticate),
                      self);

    if (getenv ("EAS_SOUP_LOGGER") && (atoi (g_getenv ("EAS_SOUP_LOGGER")) >= 1))
    {
        SoupLogger *logger;
        logger = soup_logger_new (SOUP_LOGGER_LOG_BODY, -1);
        soup_session_add_feature (priv->soup_session, SOUP_SESSION_FEATURE (logger));
    }

    priv->accountUid = NULL;
	priv->account = NULL; // Just a reference

    priv->policy_key = NULL;

    priv->request_cmd = NULL;
    priv->request_doc = NULL;
    priv->request = NULL;
    priv->request_error = NULL;

	priv->retrying_asked = FALSE;

    g_debug ("eas_connection_init--");
}

static void
eas_connection_dispose (GObject *object)
{
    EasConnection *cnc = (EasConnection *) object;
    EasConnectionPrivate *priv = NULL;
    gchar* hashkey = NULL;

	g_debug("eas_connection_dispose++");
    g_return_if_fail (EAS_IS_CONNECTION (cnc));

    priv = cnc->priv;

    if (g_open_connections)
    {
        hashkey = g_strdup_printf ("%s@%s", 
                                   eas_account_get_username(priv->account), 
                                   eas_account_get_uri(priv->account));
        g_hash_table_remove (g_open_connections, hashkey);
        g_free (hashkey);
        if (g_hash_table_size (g_open_connections) == 0)
        {
            g_hash_table_destroy (g_open_connections);
            g_open_connections = NULL;
        }
    }

    g_signal_handlers_disconnect_by_func (priv->soup_session,
                                          connection_authenticate,
                                          cnc);

    if (priv->soup_session)
    {
        g_object_unref (priv->soup_session);
        priv->soup_session = NULL;

        g_main_loop_quit (priv->soup_loop);
        g_thread_join (priv->soup_thread);
        priv->soup_thread = NULL;

        g_main_loop_unref (priv->soup_loop);
        priv->soup_loop = NULL;
        g_main_context_unref (priv->soup_context);
        priv->soup_context = NULL;
    }

    if (priv->request)
    {
        // TODO - @@WARNING Check this is a valid thing to do.
        // It might only call the base gobject class finalize method not the
        // correct method. Not sure if gobjects are properly polymorphic.
        g_object_unref (priv->request);
		priv->request = NULL;
    }

    G_OBJECT_CLASS (eas_connection_parent_class)->dispose (object);

	g_debug("eas_connection_dispose--");
}

static void
eas_connection_finalize (GObject *object)
{
	EasConnection *cnc = (EasConnection *) object;
    EasConnectionPrivate *priv = cnc->priv;

    g_debug ("eas_connection_finalize++");

    g_free (priv->accountUid);
    g_free (priv->policy_key);


    if (priv->request_doc)
    {
#ifndef ACTIVESYNC_14
		if (!g_strcmp0 ("SendMail", priv->request_cmd))
		{
			g_free ((gchar*)priv->request_doc);
		}
		else
		{
			xmlFreeDoc (priv->request_doc);
		}
#else
        xmlFreeDoc (priv->request_doc);
#endif
		priv->request_doc = NULL;
    }
	
    priv->request_cmd = NULL;

    if (priv->request_error && *priv->request_error)
    {
        g_error_free (*priv->request_error);
    }

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
connection_authenticate (SoupSession *sess, 
                         SoupMessage *msg, 
                         SoupAuth *auth, 
                         gboolean retrying, 
                         gpointer data)
{
    EasConnection* cnc = (EasConnection *) data;
	gchar *argv[] = {(gchar*)ASKPASS, (gchar*)"Please enter your Exchange Password", NULL};
	gchar* password = NULL;
	gint exit_status = 0;
	GnomeKeyringResult result = GNOME_KEYRING_RESULT_OK;
	gchar* description = NULL;
	GError *error = NULL;

    g_debug ("  eas_connection - connection_authenticate++");

	result = gnome_keyring_find_password_sync (GNOME_KEYRING_NETWORK_PASSWORD,
	                                           &password,
	                                           "user", eas_account_get_username(cnc->priv->account) ,
                                               "server", eas_account_get_uri(cnc->priv->account),
	                                           NULL);

	if (GNOME_KEYRING_RESULT_NO_MATCH == result)
	{
		g_warning("Failed to find password in Gnome Keyring");

		if (FALSE == g_spawn_sync (NULL, argv, NULL,
		                           G_SPAWN_SEARCH_PATH,
		                           NULL, NULL,
		                           &password, NULL, 
		                           &exit_status,
		                           &error))
		{
			g_warning ("Failed to spawn : [%d][%s]",
				error->code, error->message);
			g_error_free (error);
			return;
		}

		g_strchomp (password);

		description = g_strdup_printf ("Exchange Server Password for %s@%s", 
		                               eas_account_get_username(cnc->priv->account),
		                               eas_account_get_uri(cnc->priv->account));
		
		result = gnome_keyring_store_password_sync (GNOME_KEYRING_NETWORK_PASSWORD,
		                                            NULL,
		                                            description,
		                                            password,
		                                            "user", eas_account_get_username(cnc->priv->account),
		                                            "server", eas_account_get_uri(cnc->priv->account),
		                                            NULL);

		g_free (description);

		if (GNOME_KEYRING_RESULT_OK == result)
		{
			soup_auth_authenticate (auth, 
				                    eas_account_get_username(cnc->priv->account),
				                    password);
		}
		else 
		{
			g_critical ("Failed to store password to gnome keyring [%d]", result);
		}

		memset (password, 0, strlen(password));
		g_free(password);
		password = NULL;
	}
	else if (result != GNOME_KEYRING_RESULT_OK)
	{
		g_warning("GnomeKeyring failed to find password [%d]", result);
	}
	else
	{
		if (!retrying)
		{
			cnc->priv->retrying_asked = FALSE;

			soup_auth_authenticate (auth, 
				                    eas_account_get_username(cnc->priv->account),
				                    password);
			if (password)
			{
				gnome_keyring_free_password(password);
			}
		}
		else if (!cnc->priv->retrying_asked)
		{
			gchar* description = NULL;

			cnc->priv->retrying_asked = TRUE;
			
			if (password)
			{
				gnome_keyring_free_password(password);
				password = NULL;
			}
			
			if (FALSE == g_spawn_sync (NULL, argv, NULL,
				                       G_SPAWN_SEARCH_PATH,
				                       NULL, NULL,
				                       &password, NULL, 
				                       &exit_status,
				                       &error))
			{
				g_warning ("Failed to spawn : [%d][%s]",
					error->code, error->message);
				g_error_free (error);
				return;
			}

			g_strchomp (password);

			description = g_strdup_printf ("Exchange Server Password for %s@%s", 
		                                   eas_account_get_username(cnc->priv->account),
		                                   eas_account_get_uri(cnc->priv->account));
		
			result = gnome_keyring_store_password_sync (GNOME_KEYRING_NETWORK_PASSWORD,
				                                        NULL,
		    		                                    description,
		        		                                password,
		            		                            "user", eas_account_get_username(cnc->priv->account),
		                		                        "server", eas_account_get_uri(cnc->priv->account),
		                    		                    NULL);

			g_free (description);

			if (GNOME_KEYRING_RESULT_OK == result)
			{
				soup_auth_authenticate (auth, 
			                            eas_account_get_username(cnc->priv->account),
			                            password);
			}
			else 
			{
				g_critical ("Failed to store password to gnome keyring [%d]", result);
			}

			memset (password, 0, strlen (password));
			g_free (password);
			password = NULL;
		}
	}

    g_debug ("  eas_connection - connection_authenticate--");
}

static gpointer
eas_soup_thread (gpointer user_data)
{
    EasConnectionPrivate *priv = user_data;

    g_debug ("  eas_connection - eas_soup_thread++");

    g_main_context_push_thread_default (priv->soup_context);
    g_main_loop_run (priv->soup_loop);
    g_main_context_pop_thread_default (priv->soup_context);

    g_debug ("  eas_connection - eas_soup_thread--");
    return NULL;
}

void eas_connection_set_policy_key (EasConnection* self, const gchar* policyKey)
{
    EasConnectionPrivate *priv = self->priv;

    g_debug ("eas_connection_set_policy_key++");

    g_free (priv->policy_key);
    priv->policy_key = g_strdup (policyKey);

    g_debug ("eas_connection_set_policy_key--");
}

void eas_connection_resume_request (EasConnection* self)
{
    EasConnectionPrivate *priv = self->priv;
    const gchar *_cmd;
    struct _EasRequestBase *_request;
    xmlDoc *_doc;
    GError **_error;

    g_debug ("eas_connection_resume_request++");

    // If these aren't set it's all gone horribly wrong
	g_return_if_fail (priv->request_cmd && priv->request && priv->request_doc);

    _cmd     = priv->request_cmd;
    _request = priv->request;
    _doc     = priv->request_doc;
    _error   = priv->request_error;

    priv->request_cmd   = NULL;
    priv->request       = NULL;
    priv->request_doc   = NULL;
    priv->request_error = NULL;

    eas_connection_send_request (self, _cmd, _doc, _request, _error);

    g_debug ("eas_connection_resume_request--");
}

static gboolean
eas_queue_soup_message (gpointer _request)
{
	struct _EasRequestBase *request = _request;
	EasConnection *self = eas_request_base_GetConnection (request);
	SoupMessage *msg = eas_request_base_GetSoupMessage (request);
	EasConnectionPrivate *priv = self->priv;

    soup_session_queue_message(priv->soup_session, 
                               msg, 
                               handle_server_response, 
                               request);

	return FALSE;
}


static void soap_got_chunk (SoupMessage *msg, SoupBuffer *chunk, gpointer data)
{
	struct _EasRequestBase *request = data;

	if (msg->status_code != 200)
		return;

	g_debug ("Received %zd bytes for request %p", chunk->length, request);
}

static void soap_got_headers (SoupMessage *msg, gpointer data)
{
	struct _EasRequestBase *request = data;
	const gchar *size_hdr;

	size_hdr = soup_message_headers_get_one (msg->response_headers,
											 "Content-Length");
	if (size_hdr) {
		gsize size = strtoull (size_hdr, NULL, 10);
		g_debug ("Response size of request %p is %zu bytes", request, size);
		// We can stash this away and use it to provide progress updates
		// as a percentage. If we don't have a Content-Length: header,
		// for example if the server uses chunked encoding, then we cannot
		// do percentages.
	} else {
		g_debug ("Response size of request %p is unknown", request);
		// Note that our got_headers signal handler may be called more than
		// once for a given request, if it fails once with 'authentication
		// needed' then succeeds on being resubmitted. So if the *first*
		// response has a Content-Length but the second one doesn't, we have
		// to do the right thing and not keep using the first length.
	}
}

/**
 * WBXML encode the message and send to exchange server via libsoup.
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
                             struct _EasRequestBase *request, 
                             GError** error)
{
    gboolean ret = TRUE;
    EasConnectionPrivate *priv = self->priv;
    SoupMessage *msg = NULL;
    gchar* uri = NULL;
    WB_UTINY *wbxml = NULL;
    WB_ULONG wbxml_len = 0;
    WBXMLError wbxml_ret = WBXML_OK;
    WBXMLConvXML2WBXML *conv = NULL;
    xmlChar* dataptr = NULL;
    int data_len = 0;
	GSource *source = NULL;

    g_debug ("eas_connection_send_request++");
    // If not the provision request, store the request
    if (g_strcmp0 (cmd, "Provision"))
    {
		gint recursive = 1;
		g_debug("store the request");
        priv->request_cmd = g_strdup (cmd);

#ifndef ACTIVESYNC_14
		if(!g_strcmp0(cmd, "SendMail"))
		{
			priv->request_doc = (gpointer)g_strdup((gchar*) doc);
		}
		else
		{
			priv->request_doc = xmlCopyDoc (doc, recursive);
		}
#else
        priv->request_doc = xmlCopyDoc (doc, recursive);
#endif

        priv->request = request;
        priv->request_error = error;
    }

    // If we need to provision, and not the provisioning msg
    if (!priv->policy_key && g_strcmp0 (cmd, "Provision"))
    {
        EasProvisionReq *req = eas_provision_req_new (NULL, NULL);
        g_debug ("  eas_connection_send_request - Provisioning required");

        eas_request_base_SetConnection (&req->parent_instance, self);
        ret = eas_provision_req_Activate (req, error);
        if (!ret)
        {
            g_assert (error == NULL || *error != NULL);
        }
        goto finish;
    }

    wbxml_ret = wbxml_conv_xml2wbxml_create (&conv);
    if (wbxml_ret != WBXML_OK)
    {
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_WBXMLERROR,
                     ("error %d returned from wbxml_conv_xml2wbxml_create"), wbxml_ret);
        ret = FALSE;
        goto finish;
    }

    uri = g_strconcat (eas_account_get_uri(priv->account),
                       "?Cmd=", cmd,
                       "&User=", eas_account_get_username(priv->account),
                       "&DeviceID=", "1234567890",   // TODO Need to get device type and id from the correct place.
                       "&DeviceType=", "FakeDevice", // TODO Need to get device type and id from the correct place.
                       NULL);

    msg = soup_message_new ("POST", uri);
    g_free (uri);
    if (!msg)
    {
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_SOUPERROR,
                     ("soup_message_new returned NULL"));
        ret = FALSE;
        goto finish;
    }

    soup_message_headers_append (msg->request_headers,
                                 "MS-ASProtocolVersion",
                                AS_VERSION);

    soup_message_headers_append (msg->request_headers,
                                 "User-Agent",
                                 "libeas");

    soup_message_headers_append (msg->request_headers,
                                 "X-MS-PolicyKey",
                                 (priv->policy_key ? priv->policy_key : "0"));
#ifndef ACTIVESYNC_14
//in activesync 12.1, SendMail uses mime, not wbxml in the body
if(g_strcmp0(cmd, "SendMail"))
{
#endif
	// Convert doc into a flat xml string
    xmlDocDumpFormatMemoryEnc (doc, &dataptr, &data_len, (gchar*) "utf-8", 1);
    wbxml_conv_xml2wbxml_disable_public_id (conv);
    wbxml_conv_xml2wbxml_disable_string_table (conv);
    wbxml_ret = wbxml_conv_xml2wbxml_run (conv, dataptr, data_len, &wbxml, &wbxml_len);

    if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5))
    {
        gchar* tmp = g_strndup ( (gchar*) dataptr, data_len);
        g_debug ("\n=== XML Input ===\n%s=== XML Input ===", tmp);
        g_free (tmp);

        g_debug ("wbxml_conv_xml2wbxml_run [Ret:%s],  wbxml_len = [%d]", wbxml_errors_string (wbxml_ret), wbxml_len);
    }

    wbxml_ret = wbxml_conv_xml2wbxml_run (conv, dataptr, data_len, &wbxml, &wbxml_len);
    if (wbxml_ret != WBXML_OK)
    {
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_WBXMLERROR,
                     ("error %d returned from wbxml_conv_xml2wbxml_run"), wbxml_ret);
        ret = FALSE;
        goto finish;
    }


    soup_message_headers_set_content_length (msg->request_headers, wbxml_len);

    soup_message_set_request (msg,
                              "application/vnd.ms-sync.wbxml",
                              SOUP_MEMORY_COPY,
                              (gchar*) wbxml,
                              wbxml_len);
#ifndef ACTIVESYNC_14
}
else
{
    //Activesync 12.1 implementation for SendMail Command
    soup_message_headers_set_content_length (msg->request_headers, strlen((gchar*)doc));

    soup_message_set_request (msg,
                              "message/rfc822",
                              SOUP_MEMORY_COPY,
                              (gchar*) doc,
                              strlen((gchar*)doc));

	//Avoid double-free in handle_server_response()
	priv->request_doc = NULL;
}
#endif
    eas_request_base_SetSoupMessage (request, msg);

    g_signal_connect (msg, "got-chunk", G_CALLBACK (soap_got_chunk), request);
    g_signal_connect (msg, "got-headers", G_CALLBACK (soap_got_headers), request);

    // We have to call soup_session_queue_message() from the soup thread,
    // or libsoup screws up (https://bugzilla.gnome.org/642573)
	source = g_idle_source_new ();
	g_source_set_callback (source, eas_queue_soup_message, request, NULL);
	g_source_attach (source, priv->soup_context);

finish:
	// @@WARNING - doc must always be freed before exiting this function.

#ifndef ACTIVESYNC_14
if(!g_strcmp0(cmd, "SendMail"))
	{
		g_free ((gchar*)doc);
	}
	else
	{
		xmlFreeDoc(doc);
	}
#else
	xmlFreeDoc (doc);
#endif

    if (wbxml) free (wbxml);
    if (conv) wbxml_conv_xml2wbxml_destroy (conv);
    if (dataptr) xmlFree (dataptr);

    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_connection_send_request--");
    return ret;
}

typedef enum
{
    INVALID = 0,
    VALID_NON_EMPTY,
    VALID_EMPTY,
} RequestValidity;


static RequestValidity
isResponseValid (SoupMessage *msg)
{
    const gchar *content_type = NULL;
    goffset header_content_length = 0;

    g_debug ("eas_connection - isResponseValid++");

    if (HTTP_STATUS_OK != msg->status_code)
    {
        g_critical ("Failed with status [%d] : %s", msg->status_code, (msg->reason_phrase ? msg->reason_phrase : "-"));
        return INVALID;
    }

    if (SOUP_ENCODING_CONTENT_LENGTH != soup_message_headers_get_encoding (msg->response_headers))
    {
        g_warning ("  Failed: Content-Length was not found");
        return INVALID;
    }

    header_content_length = soup_message_headers_get_content_length (msg->response_headers);
    if (header_content_length != msg->response_body->length)
    {
        g_warning ("  Failed: Header[%ld] and Body[%ld] Content-Length do not match",
                   (long) header_content_length, (long) msg->response_body->length);
        return INVALID;
    }

    if (!header_content_length)
    {
        g_debug ("Empty Content");
        return VALID_EMPTY;
    }

    content_type = soup_message_headers_get_one (msg->response_headers, "Content-Type");

    if (0 != g_strcmp0 ("application/vnd.ms-sync.wbxml", content_type))
    {
        g_warning ("  Failed: Content-Type did not match WBXML");
        return INVALID;
    }

    g_debug ("eas_connection - isResponseValid--");

    return VALID_NON_EMPTY;
}


/**
 * Converts from Microsoft Exchange Server protocol WBXML to XML
 * @param wbxml input buffer
 * @param wbxml_len input buffer length
 * @param xml output buffer [full transfer]
 * @param xml_len length of the output buffer in bytes
 */
static gboolean
wbxml2xml (const WB_UTINY *wbxml, const WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len)
{
    gboolean ret = TRUE;
    WBXMLConvWBXML2XML *conv = NULL;
    WBXMLError wbxml_ret = WBXML_OK;

    g_debug ("eas_connection - wbxml2xml++");

    *xml = NULL;
    *xml_len = 0;

    if (NULL == wbxml)
    {
        g_warning ("  wbxml is NULL!");
        ret = FALSE;
        goto finish;
    }

    if (0 == wbxml_len)
    {
        g_warning ("  wbxml_len is 0!");
        ret = FALSE;
        goto finish;
    }

    wbxml_ret = wbxml_conv_wbxml2xml_create (&conv);

    if (wbxml_ret != WBXML_OK)
    {
        g_warning ("  Failed to create conv! %s", wbxml_errors_string (ret));
        ret = FALSE;
        goto finish;
    }

    wbxml_conv_wbxml2xml_set_language (conv, WBXML_LANG_ACTIVESYNC);
    wbxml_conv_wbxml2xml_set_indent (conv, 1);

    wbxml_ret = wbxml_conv_wbxml2xml_run (conv,
                                          (WB_UTINY *) wbxml,
                                          wbxml_len,
                                          xml,
                                          xml_len);

    if (WBXML_OK != wbxml_ret)
    {
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

    wbxml2xml (wbxml, wbxml_len, &xml, &xml_len);

    tmp = g_strndup ( (gchar*) xml, xml_len);
    g_debug ("\n=== dump start: xml_len [%d] ===\n%s=== dump end ===", xml_len, tmp);
    if (tmp) g_free (tmp);
    if (xml) free (xml);
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
    xmlNode *node, *child;
    xmlNs *ns;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode (doc, NULL, (xmlChar *) "Autodiscover", NULL);
    xmlDocSetRootElement (doc, node);
    ns = xmlNewNs (node, (xmlChar *) "http://schemas.microsoft.com/exchange/autodiscover/mobilesync/requestschema/2006", NULL);
    node = xmlNewChild (node, ns, (xmlChar *) "Request", NULL);
    child = xmlNewChild (node, ns, (xmlChar *) "EMailAddress", (xmlChar *) email);
    child = xmlNewChild (node, ns, (xmlChar *) "AcceptableResponseSchema",
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
    for (node = node->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE &&
                !g_strcmp0 ( (char *) node->name, "Url"))
        {
            char *asurl = (char *) xmlNodeGetContent (node);
            if (asurl)
                return asurl;
        }
    }
    return NULL;
}

typedef struct
{
    EasConnection *cnc;
    GSimpleAsyncResult *simple;
    SoupMessage *msgs[2];
    EasAutoDiscoverCallback cb;
    gpointer cbdata;
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

    for (; idx < 2; ++idx)
    {
        if (adData->msgs[idx] == msg)
            break;
    }

    if (idx == 2)
    {
        g_debug ("Request got cancelled and removed");
        return;
    }

    adData->msgs[idx] = NULL;
	
    if (status != HTTP_STATUS_OK)
    {
        g_warning ("Autodiscover HTTP response was not OK");
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Status code: %d - Response from server",
                     status);
        goto failed;
    }

    doc = xmlReadMemory (msg->response_body->data,
                         msg->response_body->length,
                         "autodiscover.xml",
                         NULL,
                         XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

    if (!doc)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Failed to parse autodiscover response XML");
        goto failed;
    }

    node = xmlDocGetRootElement (doc);
    if (g_strcmp0 ( (gchar*) node->name, "Autodiscover"))
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Failed to find <Autodiscover> element");
        goto failed;
    }
    for (node = node->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) node->name, "Response"))
            break;
    }
    if (!node)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Failed to find <Response> element");
        goto failed;
    }
    for (node = node->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) node->name, "Action"))
            break;
    }
    if (!node)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Failed to find <Action> element");
        goto failed;
    }
    for (node = node->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) node->name, "Settings"))
            break;
    }
    if (!node)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Failed to find <Settings> element");
        goto failed;
    }
    for (node = node->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE &&
                !g_strcmp0 ( (gchar*) node->name, "Server") &&
                (serverUrl = autodiscover_parse_protocol (node)))
            break;
    }

    for (idx = 0; idx < 2; ++idx)
    {
        if (adData->msgs[idx])
        {
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
    for (idx = 0; idx < 2; ++idx)
    {
        if (adData->msgs[idx])
        {
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

    if (!g_simple_async_result_propagate_error (simple, &error))
    {
        url = g_simple_async_result_get_op_res_gpointer (simple);
    }

	// Url ownership transferred to cb
    adData->cb (url, adData->cbdata, error);
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
                              (gchar*) buf->buffer->content,
                              buf->buffer->use);

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
eas_connection_autodiscover (EasAutoDiscoverCallback cb,
                             gpointer cb_data,
                             const gchar* email,
                             const gchar* username)
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

    if (!email)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Email is mandatory and must be provided");
        cb (NULL, cb_data, error);
        return;
    }

    domain = strchr (email, '@');
    if (! (domain && *domain))
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FAILED,
                     "Failed to extract domain from email address");
        cb (NULL, cb_data, error);
        return;
    }
    ++domain; // Advance past the '@'

	account = eas_account_new();
	if (!account)
	{
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     "Failed create temp account for autodiscover");
        cb (NULL, cb_data, error);
        return;
	}
#if 0
	account->uid = g_strdup (email);
	account->serverUri = g_strdup("autodiscover");
#endif

	eas_account_set_uid(account, email);	
	eas_account_set_uri(account, "autodiscover");

    if (!username) // Use the front of the email as the username
    {
        gchar **split = g_strsplit (email, "@", 2);
		eas_account_set_username(account, split[0]);
        g_strfreev (split);
    }
    else // Use the supplied username
    {
        eas_account_set_username(account, username);
    }

    cnc = eas_connection_new (account, &error);

    if (!cnc)
    {
        cb (NULL, cb_data, error);
		g_object_unref (account);
        return;
    }

    doc = autodiscover_as_xml (email);
    txBuf = xmlAllocOutputBuffer (NULL);
    xmlNodeDumpOutput (txBuf, doc, xmlDocGetRootElement (doc), 0, 1, NULL);
    xmlOutputBufferFlush (txBuf);

    autoDiscData = g_new0 (EasAutoDiscoverData, 1);
    autoDiscData->cb = cb;
    autoDiscData->cbdata = cb_data;
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

    if (!accountId) return NULL;

    eas_connection_accounts_init();

	iter = e_list_get_iterator (E_LIST ( g_account_list));
	for (; e_iterator_is_valid (iter);  e_iterator_next (iter))
	{
		account = EAS_ACCOUNT (e_iterator_get (iter));
		g_print("account->uid=%s\n", eas_account_get_uid(account));
		if (strcmp (eas_account_get_uid(account), accountId) == 0) {
			account_found = TRUE;
			break;
		}
	}

	if(!account_found)
	{
		g_warning("No account details found for accountId [%s]", accountId);
		return NULL;
	}
	
    g_static_mutex_lock (&connection_list);
    if (g_open_connections)
    {
		gchar *hashkey = g_strdup_printf("%s@%s", 
                                         eas_account_get_username(account), 
                                         eas_account_get_uri(account));

        cnc = g_hash_table_lookup (g_open_connections, hashkey);
        g_free (hashkey);

        if (EAS_IS_CONNECTION (cnc))
        {
            g_object_ref (cnc);
            g_static_mutex_unlock (&connection_list);
            g_debug ("eas_connection_find (Found) --");
            return cnc;
        }
    }
    g_static_mutex_unlock (&connection_list);

	cnc = eas_connection_new (account, &error);
    if (cnc)
    {
        g_debug ("eas_connection_find (Created) --");
    }
    else
    {
        if (error)
        {
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

    g_debug ("eas_connection_new++");

    g_type_init ();

	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (!account)
    {
        g_set_error (error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_BADARG,
                     "An account must be provided.");
        return NULL;
    }

    g_debug ("Checking for open connection");
    g_static_mutex_lock (&connection_list);
    if (g_open_connections)
    {
        hashkey = g_strdup_printf ("%s@%s", eas_account_get_username(account), eas_account_get_uri(account));
        cnc = g_hash_table_lookup (g_open_connections, hashkey);
        g_free (hashkey);

        if (EAS_IS_CONNECTION (cnc))
        {
            g_object_ref (cnc);
            g_static_mutex_unlock (&connection_list);
            return cnc;
        }
    }

    g_debug ("No existing connection, create new one");
    cnc = g_object_new (EAS_TYPE_CONNECTION, NULL);
    priv = cnc->priv;

    if (!cnc)
    {
        g_set_error (error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     "A server url and username must be provided.");
        g_static_mutex_unlock (&connection_list);
        return NULL;
    }

	// Just a reference to the global account list
	priv->account = account;

    hashkey = g_strdup_printf ("%s@%s", eas_account_get_username(account), eas_account_get_uri(account));

    if (!g_open_connections)
    {
        g_debug ("Creating hashtable");
        g_open_connections = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    }

    g_debug ("Adding to hashtable");
    g_hash_table_insert (g_open_connections, hashkey, cnc);

    g_static_mutex_unlock (&connection_list);
    g_debug ("eas_connection_new--");
    return cnc;
}

static void
parse_for_status (xmlNode *node)
{
    xmlNode *child = NULL;
    xmlNode *sibling = NULL;

    if (!node) return;
    if ( (child = node->children))
    {
        parse_for_status (child);
    }

    if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status"))
    {
        gchar* status = (gchar*) xmlNodeGetContent (node);
        if (!g_strcmp0 (status, "1"))
        {
            g_message ("parent_name[%s] status = [%s]", (char*) node->parent->name , status);
        }
        else
        {
            g_critical ("parent_name[%s] status = [%s]", (char*) node->parent->name , status);
        }
        xmlFree (status);
    }

    if ( (sibling = node->next))
    {
        parse_for_status (sibling);
    }
}

void
handle_server_response (SoupSession *session, SoupMessage *msg, gpointer data)
{
    EasRequestBase *req = (EasRequestBase *) data;
    EasConnection *self = (EasConnection *) eas_request_base_GetConnection (req);
    EasConnectionPrivate *priv = self->priv;
    xmlDoc *doc = NULL;
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;
    gboolean isProvisioningRequired = FALSE;
    GError *error = NULL;
    RequestValidity validity = isResponseValid (msg);
	gboolean cleanupRequest = FALSE;

    g_debug ("eas_connection - handle_server_response++");
	g_debug("  eas_connection - handle_server_response self[%lx]", (unsigned long)self);
	g_debug("  eas_connection - handle_server_response priv[%lx]", (unsigned long)self->priv);

    if (INVALID == validity)
    {
        g_set_error (&error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_SOUPERROR,
                     ("Invalid soup message received"));
        goto complete_request;
    }

    if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5))
    {
        dump_wbxml_response ( (WB_UTINY*) msg->response_body->data, msg->response_body->length);
    }

    if (VALID_NON_EMPTY == validity)
    {
        if (!wbxml2xml ( (WB_UTINY*) msg->response_body->data,
                         msg->response_body->length,
                         &xml,
                         &xml_len))
        {
            g_set_error (&error, EAS_CONNECTION_ERROR,
                         EAS_CONNECTION_ERROR_WBXMLERROR,
                         ("Converting wbxml failed"));
            goto complete_request;
        }

        g_debug ("  handle_server_response - pre-xmlReadMemory");

        doc = xmlReadMemory ( (const char*) xml,
                              xml_len,
                              "sync.xml",
                              NULL,
                              0);

        if (xml) free (xml);

        if (doc)
        {
            xmlNode* node = xmlDocGetRootElement (doc);
            parse_for_status (node);
        }
    }

    // TODO Pre-process response status to see if provisioning is required
    // isProvisioningRequired = ????

complete_request:
    if (!isProvisioningRequired)
    {
        EasRequestType request_type = eas_request_base_GetRequestType (req);

        g_debug ("  handle_server_response - no parsed provisioning required");
        g_debug ("  handle_server_response - Handling request [%d]", request_type);

        if (request_type != EAS_REQ_PROVISION)
        {
            // Clean up request data
#ifndef ACTIVESYNC_14
			if (!g_strcmp0("SendMail", priv->request_cmd))
			{
				g_free((gchar*)priv->request_doc);
			}
			else
			{
        		xmlFreeDoc (priv->request_doc);
			}
#else
			xmlFreeDoc (priv->request_doc);
#endif
	

            priv->request_cmd = NULL;
            priv->request_doc = NULL;
            priv->request = NULL;
            priv->request_error = NULL;
        }

        switch (request_type)
        {
            default:
            {
                g_debug ("  Unknown RequestType [%d]", request_type);
            }
            break;

            case EAS_REQ_PROVISION:
            {
                cleanupRequest = eas_provision_req_MessageComplete ( (EasProvisionReq *) req, doc, error);
            }
            break;

            case EAS_REQ_SYNC_FOLDER_HIERARCHY:
            {
                 cleanupRequest = eas_sync_folder_hierarchy_req_MessageComplete ( (EasSyncFolderHierarchyReq *) req, doc, error);
            }
            break;

            case EAS_REQ_SYNC:
            {
                cleanupRequest = eas_sync_req_MessageComplete ( (EasSyncReq *) req, doc, error);
            }
            break;
            case EAS_REQ_GET_EMAIL_BODY:
            {
                cleanupRequest = eas_get_email_body_req_MessageComplete ( (EasGetEmailBodyReq *) req, doc, error);
            }
            break;
            case EAS_REQ_GET_EMAIL_ATTACHMENT:
            {
                cleanupRequest = eas_get_email_attachment_req_MessageComplete ( (EasGetEmailAttachmentReq *) req, doc, error);
            }
            break;
            case EAS_REQ_DELETE_MAIL:
            {
                cleanupRequest = eas_delete_email_req_MessageComplete ( (EasDeleteEmailReq *) req, doc, error);
            }
            break;
            case EAS_REQ_SEND_EMAIL:
            {
                cleanupRequest = eas_send_email_req_MessageComplete ( (EasSendEmailReq *) req, doc, error);
            }
            break;
            case EAS_REQ_UPDATE_MAIL:
            {
                cleanupRequest = eas_update_email_req_MessageComplete ( (EasUpdateEmailReq *) req, doc, error);
            }
            break;
            case EAS_REQ_MOVE_EMAIL:
            {
                /*cleanupRequest =*/ eas_move_email_req_MessageComplete ( (EasMoveEmailReq *) req, doc, error);
            }
            break;				
            case EAS_REQ_UPDATE_CALENDAR:
            {
                cleanupRequest = eas_update_calendar_req_MessageComplete ( (EasUpdateCalendarReq *) req, doc, error);
            }
            break;
            case EAS_REQ_ADD_CALENDAR:
            {
                cleanupRequest = eas_add_calendar_req_MessageComplete ( (EasAddCalendarReq *) req, doc, error);
            }
            break;
        }
        //if cleanupRequest is set - we are done with this request, and should clean it up
        if(cleanupRequest)
        {
            g_object_unref(req);
        }
    }
    else
    {
        EasProvisionReq *req = eas_provision_req_new (NULL, NULL);
        g_debug ("  handle_server_response - parsed provisioning required");

        // Don't delete this request and create a provisioning request.
        eas_provision_req_Activate (req, &error);   // TODO check return
    }
    g_debug ("eas_connection - handle_server_response--");
}


