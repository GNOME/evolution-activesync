/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 * eas-daemon is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * eas-daemon is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eas-connection.h"
#include <libsoup/soup.h>

#include <wbxml/wbxml.h>
#include <libedataserver/e-flag.h>
#include <libxml/xmlreader.h> // xmlDoc

#include "eas-accounts.h"
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

struct _EasConnectionPrivate
{
	SoupSession* soup_session;
	GThread* soup_thread;
	GMainLoop* soup_loop;
	GMainContext* soup_context;
	
	gchar* server_uri;
	gchar* username;
	gchar* password;

	gchar* device_type;
	gchar* device_id;

	gchar* policy_key;

	gchar* request_cmd;
	xmlDoc* request_doc;
	EasRequestBase* request;
	GError **request_error;
};

#define EAS_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_CONNECTION, EasConnectionPrivate))

static void connection_authenticate (SoupSession *sess, SoupMessage *msg, 
                                     SoupAuth *auth, gboolean retrying, 
                                     gpointer data);
static gpointer eas_soup_thread (gpointer user_data);
static void handle_server_response (SoupSession *session, SoupMessage *msg, gpointer data);
static gboolean wbxml2xml(const WB_UTINY *wbxml, const WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len);

G_DEFINE_TYPE (EasConnection, eas_connection, G_TYPE_OBJECT);

static void
eas_connection_init (EasConnection *self)
{
	EasConnectionPrivate *priv;
	self->priv = priv = EAS_CONNECTION_PRIVATE(self);

	g_debug("eas_connection_init++");

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
                      G_CALLBACK(connection_authenticate), 
                      self);

    if (getenv("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5)) {
        SoupLogger *logger;
        logger = soup_logger_new (SOUP_LOGGER_LOG_BODY, -1);
        soup_session_add_feature (priv->soup_session, SOUP_SESSION_FEATURE(logger));
    }

	// TODO Fetch the Device Type and Id from where ever it is stored.
	priv->device_type = g_strdup ("FakeDevice");
	priv->device_id = g_strdup ("1234567890");

	priv->server_uri = NULL;
	priv->username = NULL;
	priv->password = NULL;

	priv->policy_key = NULL;

	priv->request_cmd = NULL;
	priv->request_doc = NULL;
	priv->request = NULL;
	priv->request_error = NULL;

	g_debug("eas_connection_init--");
}

static void
eas_connection_finalize (GObject *object)
{
	EasConnection *cnc = (EasConnection *)object;
	EasConnectionPrivate *priv = cnc->priv;
	
	g_debug("eas_connection_finalize++");

    g_signal_handlers_disconnect_by_func (priv->soup_session, 
                                          connection_authenticate, 
                                          cnc);

    if (priv->soup_session) {
        g_object_unref (priv->soup_session);
        priv->soup_session = NULL;

        g_main_loop_quit(priv->soup_loop);
        g_thread_join(priv->soup_thread);
        priv->soup_thread = NULL;

        g_main_loop_unref(priv->soup_loop);
        priv->soup_loop = NULL;
        g_main_context_unref(priv->soup_context);
        priv->soup_context = NULL;
    }

	// g_free can handle NULL
    g_free (priv->server_uri);
    g_free (priv->username);
    g_free (priv->password);

    g_free (priv->device_type);
    g_free (priv->device_id);
	
    g_free (priv->policy_key);

	g_free(priv->request_cmd);

	if (priv->request_doc) {
		xmlFree(priv->request_doc);
	}

	if (priv->request) {
		// TODO - @@WARNING Check this is a valid thing to do.
		// It might only call the base gobject class finalize method not the
		// correct method. Not sure if gobjects are properly polymorphic.
		g_object_unref(priv->request);
	}

	if (priv->request_error && *priv->request_error)
	{
		g_error_free(*priv->request_error);
	}

	G_OBJECT_CLASS (eas_connection_parent_class)->finalize (object);
	
	g_debug("eas_connection_finalize--");
}

static void
eas_connection_class_init (EasConnectionClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	g_debug("eas_connection_class_init++");
	
	g_type_class_add_private (klass, sizeof (EasConnectionPrivate));

	object_class->finalize = eas_connection_finalize;
	
	g_debug("eas_connection_class_init--");
}

static void 
connection_authenticate (SoupSession *sess, 
                         SoupMessage *msg, 
                         SoupAuth *auth, 
                         gboolean retrying, 
                         gpointer data)
{
    EasConnection* cnc = (EasConnection *)data;
	g_debug("  eas_connection - connection_authenticate++");
	if (!retrying)
	{
		soup_auth_authenticate (auth, cnc->priv->username, cnc->priv->password);
	}
	g_debug("  eas_connection - connection_authenticate--");
}

static gpointer 
eas_soup_thread (gpointer user_data)
{
    EasConnectionPrivate *priv = user_data;
	
	g_debug("  eas_connection - eas_soup_thread++");
	
    g_main_context_push_thread_default (priv->soup_context);
    g_main_loop_run (priv->soup_loop);
    g_main_context_pop_thread_default (priv->soup_context);
	
	g_debug("  eas_connection - eas_soup_thread--");
    return NULL;
}

void eas_connection_set_policy_key(EasConnection* self, gchar* policyKey)
{
	EasConnectionPrivate *priv = self->priv;
	
	g_debug("eas_connection_set_policy_key++");

	g_free(priv->policy_key);
	priv->policy_key = g_strdup(policyKey);
	
	g_debug("eas_connection_set_policy_key--");
}

void eas_connection_resume_request(EasConnection* self) 
{
	EasConnectionPrivate *priv = self->priv;
	gchar *_cmd;
	EasRequestBase *_request;
	xmlDoc *_doc;
	GError **_error;

	g_debug("eas_connection_resume_request++");

	// If these aren't set it's all gone horribly wrong
	g_assert(priv->request_cmd);
	g_assert(priv->request);
	g_assert(priv->request_doc);

	_cmd = priv->request_cmd;
	_request = priv->request;
	_doc = priv->request_doc;
	_error = priv->request_error;

	priv->request_cmd = NULL;
	priv->request = NULL;
	priv->request_doc = NULL;
	priv->request_error = NULL;
	
	eas_connection_send_request(self, _cmd, _doc, _request, _error);
	g_free(_cmd);
	
	g_debug("eas_connection_resume_request--");
}

/**
 * WBXML encode the message and send to exchange server via libsoup.
 * May also be required to temporarily hold the request message whilst 
 * provisioning with the server occurs.
 *
 * @param self the EasConnection GObject
 * @param cmd ActiveSync command string [no transfer]
 * @param doc the message xml body [full transfer]
 * @param request the request GObject
 *
 * @return whether successful
 */
gboolean 
eas_connection_send_request(EasConnection* self, gchar* cmd, xmlDoc* doc, EasRequestBase *request, GError** error)
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

	g_debug("eas_connection_send_request++");
	// If not the provision request, store the request
	if (g_strcmp0(cmd,"Provision"))
	{
		priv->request_cmd = g_strdup(cmd);
		priv->request_doc = doc;
		priv->request = request;
		priv->request_error = error;
	}

	// If we need to provision, and not the provisioning msg
	if (!priv->policy_key && g_strcmp0(cmd,"Provision"))
	{
		g_debug("  eas_connection_send_request - Provisioning required");
		
		EasProvisionReq *req = eas_provision_req_new (NULL, NULL);
		eas_request_base_SetConnection (&req->parent_instance, self);
		ret = eas_provision_req_Activate (req, error);
		if(!ret)
		{
			g_assert (error == NULL || *error != NULL);
		}
		goto finish;
	}

    wbxml_ret = wbxml_conv_xml2wbxml_create(&conv);
    if (wbxml_ret != WBXML_OK) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     ("error %d returned from wbxml_conv_xml2wbxml_create"), wbxml_ret);		
        ret = FALSE; 
		goto finish;
    }

	g_assert(priv->server_uri);
	
    uri = g_strconcat (priv->server_uri,
                       "?Cmd=", cmd,
                       "&User=", priv->username,
                       "&DeviceID=", priv->device_id,
                       "&DeviceType=", priv->device_type, 
                       NULL);
    
    msg = soup_message_new("POST", uri);
    g_free(uri);
	if(!msg)
	{
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_SOUPERROR,
			     ("soup_message_new returned NULL"));		
		ret = FALSE;
		goto finish;
	}

    soup_message_headers_append(msg->request_headers, 
                                "MS-ASProtocolVersion",
                                "14.0");
    
    soup_message_headers_append(msg->request_headers, 
                                "User-Agent",
                                "libeas");

    soup_message_headers_append(msg->request_headers, 
                                "X-MS-PolicyKey",
                                (priv->policy_key?priv->policy_key:"0"));

	// Convert doc into a flat xml string
    xmlDocDumpFormatMemoryEnc(doc, &dataptr, &data_len, (gchar*)"utf-8",1);
    wbxml_conv_xml2wbxml_disable_public_id(conv);
	wbxml_conv_xml2wbxml_disable_string_table(conv);
    wbxml_ret = wbxml_conv_xml2wbxml_run(conv, dataptr, data_len, &wbxml, &wbxml_len);

    if (getenv("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5)) 
	{
		gchar* tmp = g_strndup((gchar*)dataptr, data_len);
		g_debug("\n=== XML Input ===\n%s=== XML Input ===", tmp);
		g_free(tmp);

		g_debug("wbxml_conv_xml2wbxml_run [Ret:%s],  wbxml_len = [%d]", wbxml_errors_string(wbxml_ret), wbxml_len);
	}

    wbxml_ret = wbxml_conv_xml2wbxml_run(conv, dataptr, data_len, &wbxml, &wbxml_len);
	if(wbxml_ret != WBXML_OK)
	{
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     ("error %d returned from wbxml_conv_xml2wbxml_run"), wbxml_ret);		
        ret = FALSE; 
		goto finish;
	}

    soup_message_headers_set_content_length(msg->request_headers, wbxml_len);

    soup_message_set_request(msg, 
                             "application/vnd.ms-sync.wbxml",
                             SOUP_MEMORY_COPY,
                             (gchar*)wbxml,
                             wbxml_len);

    soup_session_queue_message(priv->soup_session, 
                               msg, 
                               handle_server_response, 
                               request);
finish:	
    if (wbxml) free(wbxml);
    if (conv) wbxml_conv_xml2wbxml_destroy(conv);
    if (dataptr) xmlFree(dataptr);
 
	if(!ret)
	{
		g_assert (error == NULL || *error != NULL);
	}	
	g_debug("eas_connection_send_request--");	
	return ret;
}

typedef enum{
	INVALID = 0,
	VALID_NON_EMPTY,
	VALID_EMPTY,
}RequestValidity;


static RequestValidity 
isResponseValid(SoupMessage *msg)
{
    const gchar *content_type = NULL;
    goffset header_content_length = 0;
	
	g_debug("eas_connection - isResponseValid++");
	
    if (200 != msg->status_code) 
	{
        g_critical ("Failed with status [%d] : %s", msg->status_code, (msg->reason_phrase?msg->reason_phrase:"-"));
        return INVALID;
    }

    if (SOUP_ENCODING_CONTENT_LENGTH != soup_message_headers_get_encoding(msg->response_headers))
    {
		g_warning("  Failed: Content-Length was not found");
        return INVALID;
    }

    header_content_length = soup_message_headers_get_content_length(msg->response_headers);
    if (header_content_length != msg->response_body->length)
    {
        g_warning ("  Failed: Header[%ld] and Body[%ld] Content-Length do not match", 
                (long)header_content_length, (long)msg->response_body->length);
        return INVALID;
    }

	if(!header_content_length)
	{
		g_debug("Empty Content");
		return VALID_EMPTY;
	}
	
	content_type = soup_message_headers_get_one (msg->response_headers, "Content-Type");
	
    if (0 != g_strcmp0("application/vnd.ms-sync.wbxml", content_type))
    {
		g_warning ("  Failed: Content-Type did not match WBXML");
        return INVALID;
    }
	
	g_debug("eas_connection - isResponseValid--");

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
wbxml2xml(const WB_UTINY *wbxml, const WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len)
{
	gboolean ret = TRUE;
    WBXMLConvWBXML2XML *conv = NULL;
    WBXMLError wbxml_ret = WBXML_OK;

	g_debug("eas_connection - wbxml2xml++");
	
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
        g_warning("  wbxml_len is 0!");
		ret = FALSE;
		goto finish;		
    }
	
    wbxml_ret = wbxml_conv_wbxml2xml_create(&conv);
    
    if (wbxml_ret != WBXML_OK)
    {
		g_warning ("  Failed to create conv! %s", wbxml_errors_string(ret));
        ret = FALSE;
		goto finish;
    }

    wbxml_conv_wbxml2xml_set_language(conv, WBXML_LANG_ACTIVESYNC);
    wbxml_conv_wbxml2xml_set_indent(conv, 1);
    
    wbxml_ret = wbxml_conv_wbxml2xml_run(conv,
                                   (WB_UTINY *)wbxml,
                                   wbxml_len,
                                   xml, 
                                   xml_len);

    if (WBXML_OK != wbxml_ret) 
	{
        g_warning ("  wbxml2xml Ret = %s", wbxml_errors_string(wbxml_ret));
		ret = FALSE;
    }

finish:
    if (conv) wbxml_conv_wbxml2xml_destroy(conv);

	return ret;
}

/**
 * @param wbxml binary XML to be dumped
 * @param wbxml_len length of the binary
 */
static void 
dump_wbxml_response(const WB_UTINY *wbxml, const WB_LONG wbxml_len)
{
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;
    gchar* tmp = NULL;

    wbxml2xml(wbxml, wbxml_len, &xml, &xml_len);
    
    tmp = g_strndup((gchar*)xml, xml_len);
    g_debug("\n=== dump start: xml_len [%d] ===\n%s=== dump end ===",xml_len, tmp);
    if (tmp) g_free(tmp);
    if (xml) free(xml);
}

////////////////////////////////////////////////////////////////////////////////
//
// Autodiscover
//
////////////////////////////////////////////////////////////////////////////////
 
static xmlDoc *
autodiscover_as_xml(const gchar *email)
{
    xmlDoc *doc;
    xmlNode *node, *child;
    xmlNs *ns;

    doc = xmlNewDoc((xmlChar *) "1.0");
    node = xmlNewDocNode(doc, NULL, (xmlChar *)"Autodiscover", NULL);
    xmlDocSetRootElement(doc, node);
    ns = xmlNewNs (node, (xmlChar *)"http://schemas.microsoft.com/exchange/autodiscover/mobilesync/requestschema/2006", NULL);
    node = xmlNewChild(node, ns, (xmlChar *)"Request", NULL);
    child = xmlNewChild(node, ns, (xmlChar *)"EMailAddress", (xmlChar *)email);
    child = xmlNewChild(node, ns, (xmlChar *)"AcceptableResponseSchema", 
                        (xmlChar *)"http://schemas.microsoft.com/exchange/autodiscover/mobilesync/responseschema/2006");

    return doc;
}

static gchar*
autodiscover_parse_protocol(xmlNode *node)
{
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Url")) {
            char *asurl = (char *)xmlNodeGetContent(node);
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
	EasAutoDiscoverCallback cb;
	gpointer cbdata;
} EasAutoDiscoverData;

static void
autodiscover_soup_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	GError *error = NULL;
	EasAutoDiscoverData *adData = data;
	guint status = msg->status_code;
	xmlDoc *doc = NULL;
	xmlNode *node = NULL;
	gchar *serverUrl = NULL;
	gint idx = 0;

	g_debug("autodiscover_soup_cb++");

	for(; idx < 2; ++idx)
	{
		if (adData->msgs[idx] == msg)
			break;
	}

	if (idx == 2)
	{
		g_debug("Request got cancelled and removed");
		return;
	}

	adData->msgs[idx] = NULL;

	if (status != 200)
	{
		g_warning("Autodiscover HTTP response was not OK");
		g_set_error(&error,
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
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed to parse autodiscover response XML");
		goto failed;
	}

	node = xmlDocGetRootElement (doc);
	if (g_strcmp0(node->name, "Autodiscover"))
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed to find <Autodiscover> element");
		goto failed;
	}
	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0(node->name, "Response"))
			break;
	}
	if (!node)
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed to find <Response> element");
		goto failed;
	}
	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0(node->name, "Action"))
			break;
	}
	if (!node)
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed to find <Action> element");
		goto failed;
	}
	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0(node->name, "Settings"))
			break;
	}
	if (!node)
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed to find <Settings> element");
		goto failed;
	}
	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && 
		    !g_strcmp0(node->name, "Server") &&
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
			g_message("Autodiscover success - Cancelling outstanding msg[%d]",idx);
		}
	}
	g_simple_async_result_set_op_res_gpointer (adData->simple, serverUrl, NULL);
	g_simple_async_result_complete_in_idle (adData->simple);

	g_debug("autodiscover_soup_cb (Success)--");
	return;

failed:
	for (idx = 0; idx < 2; ++idx)
	{
		if (adData->msgs[idx])
		{
			/* Clear this error and hope the second one succeeds */
			g_warning("First autodiscover attempt failed");
			g_clear_error (&error);
			return;
		}
	}
	
	/* This is returning the last error, it may be preferable for the 
	 * first error to be returned.
	 */
	g_simple_async_result_set_from_error (adData->simple, error);
	g_simple_async_result_complete_in_idle (adData->simple);
	
	g_debug("autodiscover_soup_cb (Failed)--");
}

static void 
autodiscover_simple_cb (GObject *cnc, GAsyncResult *res, gpointer data)
{
	EasAutoDiscoverData *adData = data;
	GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (res);
	GError *error = NULL;
	gchar *url = NULL;

	g_debug("autodiscover_simple_cb++");
	
	if (!g_simple_async_result_propagate_error (simple, &error))
	{
		url = g_simple_async_result_get_op_res_gpointer (simple);
	}

	adData->cb (url, adData->cbdata, error);
	g_object_unref (G_OBJECT(adData->cnc));
	g_free (adData);
	
	g_debug("autodiscover_simple_cb--");
}


static SoupMessage *
autodiscover_as_soup_msg(gchar *url, xmlOutputBuffer *buf)
{
	SoupMessage *msg = NULL;

	g_debug("autodiscover_as_soup_msg++");

	msg = soup_message_new("POST", url);
	
	soup_message_headers_append (msg->request_headers,
	                             "User-Agent", "libeas");
	
	soup_message_set_request (msg, 
	                          "text/xml", 
	                          SOUP_MEMORY_COPY,
	                          (gchar*)buf->buffer->content,
	                          buf->buffer->use);

	g_debug("autodiscover_as_soup_msg--");
	return msg;
}

////////////////////////////////////////////////////////////////////////////////
//
// Public functions
//
////////////////////////////////////////////////////////////////////////////////

/**
 * @param[in]  cb        autodiscover response callback
 * @param[in]  cb_data   data to be passed into the callback
 * @param[in]  email     user's exchange email address
 * @param[in]  username  exchange username in the format DOMAIN\username
 * @param[in]  password  exchange server account password
 */
void
eas_connection_autodiscover (EasAutoDiscoverCallback cb,
                             gpointer cb_data,
                             const gchar* email,
                             const gchar* username,
                             const gchar* password)
{
	GError *error = NULL;
	gchar* domain = NULL;
	EasConnection *cnc = NULL;
	xmlDoc *doc = NULL;
	xmlOutputBuffer* txBuf = NULL;
	gchar* url = NULL;
	EasAutoDiscoverData *autoDiscData = NULL;
	
	g_debug("eas_connection_autodiscover++");

	if (!email || !password) 
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Email and password are mandatory and must be provided");
		cb(NULL, cb_data, error);
		return;
	}

	domain = strchr(email, '@');
	if (!(domain && *domain))
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed to extract domain from email address");
		cb(NULL, cb_data, error);
		return;
	}
	++domain; // Advance past the '@'

	doc = autodiscover_as_xml (email);
	txBuf = xmlAllocOutputBuffer (NULL);
	xmlNodeDumpOutput(txBuf, doc, xmlDocGetRootElement (doc), 0, 1, NULL);
	xmlOutputBufferFlush (txBuf);

	cnc = eas_connection_new();
	if (!cnc) 
	{
		g_set_error(&error,
		            EAS_CONNECTION_ERROR,
		            EAS_CONNECTION_ERROR_FAILED,
		            "Failed create connection");
		cb(NULL, cb_data, error);
		return;
	}

	autoDiscData = g_new0(EasAutoDiscoverData, 1);
	autoDiscData->cb = cb;
	autoDiscData->cbdata = cb_data;
	autoDiscData->cnc = cnc;
	autoDiscData->simple = g_simple_async_result_new (G_OBJECT(cnc),
	                                                  autodiscover_simple_cb,
	                                                  autoDiscData,
	                                                  eas_connection_autodiscover);

	// URL formats to be tried
	// 1 - https://<domain>/autodiscover/autodiscover.xml
	url = g_strdup_printf("https://%s/autodiscover/autodiscover.xml", domain);
	autoDiscData->msgs[0] = autodiscover_as_soup_msg(url, txBuf);
	g_free(url);
	
	// 2 - https://autodiscover.<domain>/autodiscover/autodiscover.xml
	url = g_strdup_printf("https://autodiscover.%s/autodiscover/autodiscover.xml", domain);
	autoDiscData->msgs[1] = autodiscover_as_soup_msg(url, txBuf);
	g_free(url);

	if (!username) // Use the front of the email as the username
	{
		gchar **split = g_strsplit(email, "@", 2);
		eas_connection_set_details(cnc, split[0], password);
		g_strfreev(split);
	}
	else // Use the supplied username
	{
		eas_connection_set_details(cnc, username, password);
	}

	soup_session_queue_message(cnc->priv->soup_session,
	                           autoDiscData->msgs[0],
	                           autodiscover_soup_cb,
	                           autoDiscData);
	                      
	soup_session_queue_message(cnc->priv->soup_session,
	                           autoDiscData->msgs[1],
	                           autodiscover_soup_cb,
	                           autoDiscData);

	g_object_unref (cnc); // GSimpleAsyncResult holds this now
	                      
	xmlOutputBufferClose (txBuf);
	xmlFreeDoc (doc);
	
	g_debug("eas_connection_autodiscover--");
}

EasConnection* eas_connection_new()
{
	EasConnection *cnc = NULL;
	
	g_debug("eas_connection_new++");

	g_type_init();

    cnc = g_object_new (EAS_TYPE_CONNECTION, NULL);

    // May need to allow a connection to set its own authentication

	g_debug("eas_connection_new--");
    return cnc;
}

int eas_connection_set_account(EasConnection* self, guint64 accountId)
{
	EasAccounts* accountsObj = NULL;
	g_debug("eas_connection_set_account++");

	accountsObj = eas_accounts_new ();

	g_debug("eas_accounts_read_accounts_info");
	int err = eas_accounts_read_accounts_info(accountsObj);
	if (err !=0)
	{
		g_debug("Error reading data from file accounts.cfg");
		return err;
	}

	g_debug("getting data from EasAccounts object"); 
	gchar* sUri = NULL;
	gchar* uname = NULL;
	gchar* pwd = NULL;
	sUri = eas_accounts_get_server_uri (accountsObj, accountId);
	uname = eas_accounts_get_user_id (accountsObj, accountId);
	pwd = eas_accounts_get_password (accountsObj, accountId);

	self->priv->username = g_strdup (uname);
	self->priv->password = g_strdup (pwd);
	self->priv->server_uri = g_strdup (sUri);

	g_object_unref (accountsObj);
	g_debug("eas_connection_set_account--");
}

void eas_connection_set_details(EasConnection* self, const gchar* username, const gchar* password)
{
	EasConnectionPrivate *priv = self->priv;
	
	g_debug("eas_connection_set_details++");

	g_free(priv->username);
	priv->username = NULL;
	g_free(priv->password);
	priv->password = NULL;

	priv->username = g_strdup (username);
	priv->password = g_strdup (password);

	g_debug("Details: U/n[%s], P/w[%s]", priv->username, priv->password);
	
	g_debug("eas_connection_set_details--");
}

void parse_for_status(xmlNode *node)
{
	xmlNode *child = NULL;
	xmlNode *sibling = NULL;

	if (!node) return;
	if (child = node->children) 
	{
		parse_for_status(child);
	}

	if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status"))
	{
		gchar* status = xmlNodeGetContent(node);
		if (!strcmp(status,"1"))
		{
			g_message("parent_name[%s] status = [%s]",(char*)node->parent->name , status);
		}
		else
		{
			g_critical("parent_name[%s] status = [%s]",(char*)node->parent->name , status);
		}
		xmlFree(status);
	}
	
	if (sibling = node->next)
	{
		parse_for_status(sibling);
	}
}


void 
handle_server_response(SoupSession *session, SoupMessage *msg, gpointer data)
{
	gboolean ret;
	EasRequestBase *req = (EasRequestBase *)data;
	EasConnection *self = (EasConnection *)eas_request_base_GetConnection (req);
	EasConnectionPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;
	gboolean isProvisioningRequired = FALSE;
	GError *error = NULL;

	g_debug("eas_connection - handle_server_response++");

	g_debug("  eas_connection - handle_server_response self[%x]", (unsigned int)self);
	g_debug("  eas_connection - handle_server_response priv[%x]", (unsigned int)self->priv);

	RequestValidity validity = isResponseValid(msg);
	if (INVALID == validity) {
		g_set_error (&error, EAS_CONNECTION_ERROR,
	     EAS_CONNECTION_ERROR_SOUPERROR,
	     ("Invalid soup message received"));
		goto complete_request;
	}

    if (getenv("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5)) {
		dump_wbxml_response(msg->response_body->data, 
		                    msg->response_body->length);
	}

	if(VALID_NON_EMPTY == validity)
	{
		if(!wbxml2xml ((WB_UTINY*)msg->response_body->data, 
		           msg->response_body->length,
		           &xml,
		           &xml_len))
		{
			g_set_error (&error, EAS_CONNECTION_ERROR,
			 EAS_CONNECTION_ERROR_WBXMLERROR,
			 ("Converting wbxml failed"));
			goto complete_request;			
		}

		g_debug("  handle_server_response - pre-xmlReadMemory");
	
		doc = xmlReadMemory ((const char*)xml,
		                     xml_len,
		                     "sync.xml", 
		                     NULL, 
		                     0);
	
		if (xml) free(xml);

		if (doc) 
		{
			xmlNode* node = xmlDocGetRootElement(doc);
			parse_for_status(node); 
		}
	}


	// TODO Pre-process response status to see if provisioning is required
	// isProvisioningRequired = ????

complete_request:	
	if (!isProvisioningRequired)
	{
		EasRequestType request_type = eas_request_base_GetRequestType(req);

		g_debug("  handle_server_response - no parsed provisioning required");
		g_debug("  handle_server_response - Handling request [%d]", request_type);

		if (request_type != EAS_REQ_PROVISION) 
		{
			// Clean up request data
			g_free(priv->request_cmd);
			priv->request_cmd = NULL;

			xmlFree(priv->request_doc);
			priv->request_doc = NULL;
			priv->request = NULL;
			priv->request_error = NULL;
		}

		switch (request_type) 
		{
			default:
			{
				g_debug("  Unknown RequestType [%d]", request_type);
			}
			break;

			case EAS_REQ_PROVISION:
			{
				eas_provision_req_MessageComplete ((EasProvisionReq *)req, doc, error);
			}
			break;
				
			case EAS_REQ_SYNC_FOLDER_HIERARCHY:
			{
				eas_sync_folder_hierarchy_req_MessageComplete((EasSyncFolderHierarchyReq *)req, doc, error);
			}
			break;
			
			case EAS_REQ_SYNC:
			{
				eas_sync_req_MessageComplete ((EasSyncReq *)req, doc, error);
			}
			break;
			case EAS_REQ_GET_EMAIL_BODY:
			{
				eas_get_email_body_req_MessageComplete ((EasGetEmailBodyReq *)req, doc, &error);// TODO update MessageComplete to take an error rather than pass one back
			}
			break;
			case EAS_REQ_GET_EMAIL_ATTACHMENT:
			{
				eas_get_email_attachment_req_MessageComplete ((EasGetEmailAttachmentReq *)req, doc, &error);// TODO
			}
			break;			
			case EAS_REQ_DELETE_MAIL:
			{
				eas_delete_email_req_MessageComplete((EasDeleteEmailReq *)req, doc, &error);// TODO
			}
			break;
			case EAS_REQ_SEND_EMAIL:
			{
				g_debug("EAS_REQ_SEND_EMAIL");
				eas_send_email_req_MessageComplete ((EasSendEmailReq *)req, doc, &error);// TODO
			}
			break;
			case EAS_REQ_UPDATE_MAIL:
			{
				g_debug("EAS_REQ_UPDATE_EMAIL");
				eas_update_email_req_MessageComplete ((EasUpdateEmailReq *)req, doc, &error);// TODO
			}
			break;
			case EAS_REQ_UPDATE_CALENDAR:
			{
				g_debug("EAS_REQ_UPDATE_CALENDAR");
				eas_update_calendar_req_MessageComplete ((EasUpdateCalendarReq *)req, doc, &error);// TODO
			}
			break;
			case EAS_REQ_ADD_CALENDAR:
			{
				g_debug("EAS_REQ_ADD_CALENDAR");
				eas_add_calendar_req_MessageComplete ((EasAddCalendarReq *)req, doc, &error);// TODO
			}
			break;				
		}
	}
	else
	{
		g_debug("  handle_server_response - parsed provisioning required");
		
		// Don't delete this request and create a provisioning request.
		EasProvisionReq *req = eas_provision_req_new (NULL, NULL);
		eas_provision_req_Activate (req, &error);   // TODO check return
	}
	g_debug("eas_connection - handle_server_response--");
}

GQuark eas_connection_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0)) {
		const gchar *string = "eas-connection-error-quark";
		quark = g_quark_from_static_string (string);
	}

	return quark;
}

