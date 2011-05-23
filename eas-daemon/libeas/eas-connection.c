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

#include <libwbxml-1.0/wbxml/wbxml.h>
#include <libedataserver/e-flag.h>
#include <libxml/xmlreader.h> // xmlDoc

// List of includes for each request type
#include "eas-sync-folder-hierarchy.h"
#include "eas-provision-req.h"
#include "eas-sync-req.h"

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
};

#define EAS_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_CONNECTION, EasConnectionPrivate))

static void connection_authenticate (SoupSession *sess, SoupMessage *msg, 
                                     SoupAuth *auth, gboolean retrying, 
                                     gpointer data);
static gpointer eas_soup_thread (gpointer user_data);
static void handle_server_response (SoupSession *session, SoupMessage *msg, gpointer data);


G_DEFINE_TYPE (EasConnection, eas_connection, G_TYPE_OBJECT);

static void
eas_connection_init (EasConnection *self)
{
	EasConnectionPrivate *priv;
	self->priv = priv = EAS_CONNECTION_PRIVATE(self);

	g_print("eas_connection_init++\n");

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

    if (getenv("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 2)) {
        SoupLogger *logger;
        logger = soup_logger_new (SOUP_LOGGER_LOG_HEADERS, -1);
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

	g_print("eas_connection_init--\n");
}

static void
eas_connection_finalize (GObject *object)
{
	EasConnection *cnc = (EasConnection *)object;
	EasConnectionPrivate *priv = cnc->priv;
	
	g_print("eas_connection_finalize++\n");

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

	G_OBJECT_CLASS (eas_connection_parent_class)->finalize (object);
	
	g_print("eas_connection_finalize--\n");
}

static void
eas_connection_class_init (EasConnectionClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	g_print("eas_connection_class_init++\n");
	
	g_type_class_add_private (klass, sizeof (EasConnectionPrivate));

	object_class->finalize = eas_connection_finalize;
	
	g_print("eas_connection_class_init--\n");
}

static void 
connection_authenticate (SoupSession *sess, 
                         SoupMessage *msg, 
                         SoupAuth *auth, 
                         gboolean retrying, 
                         gpointer data)
{
    EasConnection* cnc = (EasConnection *)data;
	g_print("  eas_connection - connection_authenticate++\n");
    soup_auth_authenticate (auth, cnc->priv->username, cnc->priv->password);
	g_print("  eas_connection - connection_authenticate--\n");
}

static gpointer 
eas_soup_thread (gpointer user_data)
{
    EasConnectionPrivate *priv = user_data;
	
	g_print("  eas_connection - eas_soup_thread++\n");
	
    g_main_context_push_thread_default (priv->soup_context);
    g_main_loop_run (priv->soup_loop);
    g_main_context_pop_thread_default (priv->soup_context);
	
	g_print("  eas_connection - eas_soup_thread--\n");
    return NULL;
}

void eas_connection_set_policy_key(EasConnection* self, gchar* policyKey)
{
	EasConnectionPrivate *priv = self->priv;
	
	g_print("eas_connection_set_policy_key++\n");

	g_free(priv->policy_key);
	priv->policy_key = g_strdup(policyKey);
	
	g_print("eas_connection_set_policy_key--\n");
}

void eas_connection_resume_request(EasConnection* self) 
{
	EasConnectionPrivate *priv = self->priv;
	gchar *_cmd;
	EasRequestBase *_request;
	xmlDoc *_doc;

	g_print("eas_connection_resume_request++\n");

	// If these aren't set it's all gone horribly wrong
	g_assert(priv->request_cmd);
	g_assert(priv->request);
	g_assert(priv->request_doc);

	_cmd = priv->request_cmd;
	_request = priv->request;
	_doc = priv->request_doc;

	priv->request_cmd = NULL;
	priv->request = NULL;
	priv->request_doc = NULL;
		
	eas_connection_send_request(self, _cmd, _doc, _request);
	g_free(_cmd);
	
	g_print("eas_connection_resume_request--\n");
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
 */
void 
eas_connection_send_request(EasConnection* self, gchar* cmd, xmlDoc* doc, EasRequestBase *request)
{
	EasConnectionPrivate *priv = self->priv;
    SoupMessage *msg = NULL;
	gchar* uri = NULL;
    WB_UTINY *wbxml = NULL;
    WB_ULONG wbxml_len = 0;
    WBXMLError ret = WBXML_OK;
    WBXMLConvXML2WBXML *conv = NULL;
    xmlChar* dataptr = NULL;
    int data_len = 0;

	g_print("eas_connection_send_request++\n");
	// If not the provision request, store the request
	if (g_strcmp0(cmd,"Provision"))
	{
		priv->request_cmd = g_strdup(cmd);
		priv->request_doc = doc;
		priv->request = request;
	}

	// If we need to provision, and not the provisioning msg
	if (!priv->policy_key && g_strcmp0(cmd,"Provision"))
	{
		g_print("  eas_connection_send_request - Provisioning required\n");
		
		EasProvisionReq *req = eas_provision_req_new (NULL, NULL);
		eas_request_base_SetConnection (&req->parent_instance, self);
		eas_provision_req_Activate (req);
		return;
	}

    ret = wbxml_create_conv_xml2wbxml(&conv);
    if (ret != WBXML_OK) {
        g_print("%s\n", wbxml_errors_string(ret));
        return;
    }
	
    uri = g_strconcat (priv->server_uri,
                       "?Cmd=", cmd,
                       "&User=", priv->username,
                       "&DeviceID=", priv->device_id,
                       "&DeviceType=", priv->device_type, 
                       NULL);
    
    msg = soup_message_new("POST", uri);
    g_free(uri);

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
    ret = wbxml_run_conv_xml2wbxml(conv, dataptr, data_len, &wbxml, &wbxml_len);

    if (getenv("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 2)) 
	{
		gchar* tmp = g_strndup((gchar*)dataptr, data_len);
		g_print("=== XML Input ===\n");
		g_print("\n%s\n", tmp);
		g_print("=== XML Input ===\n\n");
		g_free(tmp);

		g_print("wbxml_run_conv_xml2wbxml [Ret:%s],  wbxml_len = [%d]\n", wbxml_errors_string(ret), wbxml_len);
	}
	
    if (dataptr) {
        xmlFree(dataptr);
        dataptr = NULL;
    }

    if (WBXML_OK == ret)
    {
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
    }
    
    if (wbxml) free(wbxml);
    if (conv) wbxml_destroy_conv_xml2wbxml(conv);
	g_print("eas_connection_send_request--\n");
}


static gboolean 
isResponseValid(SoupMessage *msg)
{
    const gchar *content_type = NULL;
    goffset header_content_length = 0;
	
	g_print("eas_connection - isResponseValid++\n");
	
    if (200 != msg->status_code) 
	{
        g_print ("Failed with status [%d] : %s\n", msg->status_code, (msg->reason_phrase?msg->reason_phrase:"-"));
        return FALSE;
    }

	content_type = soup_message_headers_get_one (msg->response_headers, "Content-Type");
    if (0 != g_strcmp0("application/vnd.ms-sync.wbxml", content_type))
    {
		g_print ("  Failed: Content-Type did not match WBXML\n");
        return FALSE;
    }

    if (SOUP_ENCODING_CONTENT_LENGTH != soup_message_headers_get_encoding(msg->response_headers))
    {
		g_print("  Failed: Content-Length was not found\n");
        return FALSE;
    }

    header_content_length = soup_message_headers_get_content_length(msg->response_headers);
    if (header_content_length != msg->response_body->length)
    {
        g_print ("  Failed: Header[%ld] and Body[%ld] Content-Length do not match\n", 
                (long)header_content_length, (long)msg->response_body->length);
        return FALSE;
    }

	g_print("eas_connection - isResponseValid--\n");

	return TRUE;
}


/**
 * Converts from Microsoft Exchange Server protocol WBXML to XML
 * @param wbxml input buffer
 * @param wbxml_len input buffer length
 * @param xml output buffer [full transfer]
 * @param xml_len length of the output buffer in bytes
 */
static void 
wbxml2xml(WB_UTINY *wbxml, WB_LONG wbxml_len, WB_UTINY **xml, WB_ULONG *xml_len)
{
    WBXMLConvWBXML2XML *conv = NULL;
    WBXMLError ret = WBXML_OK;

	g_print("eas_connection - wbxml2xml++\n");
	
    *xml = NULL;
    *xml_len = 0;
    
    ret = wbxml_create_conv_wbxml2xml(&conv);
    
    if (ret != WBXML_OK)
    {
		g_print ("  Failed to create conv! %s\n", wbxml_errors_string(ret));
        return;
    }

    if (NULL == wbxml)
    {
		g_print ("  wbxml is NULL!\n");
        goto failed;
    }

    if (0 == wbxml_len)
    {
        g_print("  wbxml_len is 0!\n");
        goto failed;
    }

    wbxml_conv_wbxml2xml_set_language(conv, WBXML_LANG_ACTIVESYNC);
    wbxml_conv_wbxml2xml_set_indent(conv, 1);
    
    ret = wbxml_run_conv_wbxml2xml(conv,
                                   wbxml,
                                   wbxml_len,
                                   xml, 
                                   xml_len);

    if (WBXML_OK != ret) 
	{
        g_print ("  wbxml2xml Ret = %s", wbxml_errors_string(ret));
    }

failed:
    if (conv) wbxml_destroy_conv_wbxml2xml(conv);
}

/**
 * @param wbxml binary XML to be dumped
 * @param wbxml_len length of the binary
 */
static void 
dump_wbxml_response(WB_UTINY *wbxml, WB_LONG wbxml_len)
{
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;
    gchar* tmp = NULL;

    wbxml2xml(wbxml, wbxml_len, &xml, &xml_len);
    
    tmp = g_strndup((gchar*)xml, xml_len);
    g_print("=== dump start: xml_len [%d] ===\n",xml_len);
    g_print("%s\n",tmp);
    g_print("=== dump end ===\n");
	
    if (tmp) g_free(tmp);
    if (xml) free(xml);
}

////////////////////////////////////////////////////////////////////////////////
//
// Public functions
//
////////////////////////////////////////////////////////////////////////////////

void
eas_connection_autodiscover (const gchar* email, 
                             const gchar* username, 
                             const gchar* password, 
                             gchar** serverUri, 
                             GError** error)
{
	/* TODO: Add public function implementation here */
	g_print("eas_connection_autodiscover++\n");
	g_print("eas_connection_autodiscover--\n");
}

EasConnection* eas_connection_new(const gchar* serverUri,
                                  const gchar* username,
                                  const gchar* password,
                                  GError** error)
{
	EasConnection *cnc = NULL;
	
	g_print("eas_connection_new++\n");

	*error = NULL; 

    cnc = g_object_new (EAS_TYPE_CONNECTION, NULL);
    
    cnc->priv->username = g_strdup (username);
    cnc->priv->password = g_strdup (password);
    cnc->priv->server_uri = g_strdup (serverUri);

    // May need to allow a connection to set its own authentication

	g_print("eas_connection_new--\n");
    return cnc;
}

void 
handle_server_response(SoupSession *session, SoupMessage *msg, gpointer data)
{
	EasRequestBase *req = (EasRequestBase *)data;
	EasConnection *self = (EasConnection *)eas_request_base_GetConnection (req);
	EasConnectionPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;
	gboolean isProvisioningRequired = FALSE;

	g_print("eas_connection - handle_server_response++\n");

	g_print("  eas_connection - handle_server_response self[%x]\n", (unsigned int)self);
	g_print("  eas_connection - handle_server_response priv[%x]\n", (unsigned int)self->priv);
	
	if (FALSE == isResponseValid(msg)) {
		return;
	}

    if (getenv("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 2)) {
		dump_wbxml_response(msg->response_body->data, 
		                    msg->response_body->length);
	}

    wbxml2xml ((WB_UTINY*)msg->response_body->data,
               msg->response_body->length,
               &xml,
               &xml_len);

	g_print("  handle_server_response - pre-xmlReadMemory\n");
	
    doc = xmlReadMemory ((const char*)xml, 
                         xml_len,
                         "sync.xml", 
                         NULL, 
                         0);
	if (xml) free(xml);

	// TODO Pre-process response status to see if provisioning is required
	// isProvisioningRequired = ????
	
	if (!isProvisioningRequired)
	{
		EasRequestType request_type = eas_request_base_GetRequestType(req);

		g_print("  handle_server_response - no parsed provisioning required\n");
		g_print("  handle_server_response - Handling request [%d]\n", request_type);

		if (request_type != EAS_REQ_PROVISION) 
		{
			// Clean up request data
			g_free(priv->request_cmd);
			priv->request_cmd = NULL;

			xmlFree(priv->request_doc);
			priv->request_doc = NULL;

			priv->request = NULL;
		}

		switch (request_type) 
		{
			default:
			{
				g_print("  Unknown RequestType [%d]\n", request_type);
			}
			break;

			case EAS_REQ_PROVISION:
			{
				eas_provision_req_MessageComplete ((EasProvisionReq *)req, doc);
			}
			break;
				
			case EAS_REQ_SYNC_FOLDER_HIERARCHY:
			{
				eas_sync_folder_hierarchy_MessageComplete((EasSyncFolderHierarchy *)req, doc);
			}
			break;
			
			case EAS_REQ_SYNC:
			{
				eas_sync_req_MessageComplete ((EasSyncReq *)req, doc);
			}
			break;
		}
	}
	else
	{
		g_print("  handle_server_response - parsed provisioning required\n");
		
		// Don't delete this request and create a provisioning request.
		EasProvisionReq *req = eas_provision_req_new (NULL, NULL);
		eas_provision_req_Activate (req);
	}
	g_print("eas_connection - handle_server_response--\n");
}
