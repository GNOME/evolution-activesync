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

#include "eas-connection-errors.h"
#include "eas-send-email-msg.h"
#include <wbxml/wbxml.h>
#include <glib.h>

G_DEFINE_TYPE (EasSendEmailMsg, eas_send_email_msg, EAS_TYPE_MSG_BASE);

#define EAS_SEND_EMAIL_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_MSG, EasSendEmailMsgPrivate))

struct _EasSendEmailMsgPrivate {
	gchar* account_id;
	gchar* client_id;
	gchar* mime_string;
};

static void
eas_send_email_msg_init (EasSendEmailMsg *object)
{
	EasSendEmailMsgPrivate *priv;

	/* initialization code: */
	g_debug ("eas_send_email_msg_init++");

	object->priv = priv = EAS_SEND_EMAIL_MSG_PRIVATE (object);

	priv->account_id = NULL;
	priv->client_id = NULL;
	priv->mime_string = NULL;

	g_debug ("eas_send_email_msg_init--");
}

static void
eas_send_email_msg_finalize (GObject *object)
{
	EasSendEmailMsg *msg = (EasSendEmailMsg *) object;
	EasSendEmailMsgPrivate *priv = msg->priv;

	g_debug ("eas_send_email_msg_finalize++");

	g_free (priv->mime_string);
	g_free (priv->client_id);
	g_free (priv->account_id);

	G_OBJECT_CLASS (eas_send_email_msg_parent_class)->finalize (object);
	g_debug ("eas_send_email_msg_finalize--");
}

static void
eas_send_email_msg_class_init (EasSendEmailMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasSendEmailMsgPrivate));

	object_class->finalize = eas_send_email_msg_finalize;
}

EasSendEmailMsg*
eas_send_email_msg_new (const char* account_id, const gchar* client_id, gchar* mime_string)
{
	EasSendEmailMsg* msg = NULL;
	EasSendEmailMsgPrivate *priv = NULL;

	g_debug ("eas_send_email_msg_new++");

	msg = g_object_new (EAS_TYPE_SEND_EMAIL_MSG, NULL);
	priv = msg->priv;

	priv->client_id = g_strdup (client_id);
	priv->mime_string = mime_string; // Take ownership
	priv->account_id = g_strdup (account_id);

	g_debug ("eas_send_email_msg_new--");
	return msg;
}

xmlDoc*
eas_send_email_msg_build_message (EasSendEmailMsg* self)
{
	EasSendEmailMsgPrivate *priv = self->priv;
	xmlDoc  *doc   = NULL;
	xmlNode *root  = NULL;
	gchar* base64data = NULL;

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar*) "SendMail", NULL);
	xmlDocSetRootElement (doc, root);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	// no namespaces required?
	xmlNewNs (root, (xmlChar *) "ComposeMail:", NULL);

	xmlNewChild (root, NULL, (xmlChar *) "ClientId", (xmlChar*) (priv->client_id));
	xmlNewChild (root, NULL, (xmlChar *) "SaveInSentItems", NULL); // presence indicates true

	if (priv->mime_string)
	{
		base64data = g_base64_encode ( (const guchar *) priv->mime_string, strlen (priv->mime_string));

		// Free this memory as soon as we are able.
		g_debug ("Original mime_string freed");
		g_free (priv->mime_string);
		priv->mime_string = NULL;

		// TODO: we're potentially adding a HUGE string here. Is there a libxml2 limit (as there was for receive)?
		xmlNewChild (root, NULL, (xmlChar *) "MIME", (xmlChar *) base64data);
	}
	else
	{
		g_warning("mime_string is NULL possible confusion of protocol versions.");
	}

	return doc;
}

gboolean
eas_send_email_msg_parse_response (EasSendEmailMsg* self, xmlDoc *doc, GError** error)
{
	gboolean ret = TRUE;
	xmlNode *root, *node = NULL;
	g_debug ("eas_send_email_msg_parse_response++\n");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_debug ("warning: no doc supplied");
		// Note not setting error here as empty doc is valid
		goto finish;
	}
	root = xmlDocGetRootElement (doc);
	if (g_strcmp0 ( (char *) root->name, "SendMail")) {
		g_debug ("Failed: not a SendMail response!");
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <SendMail> element"));
		ret = FALSE;
		goto finish;
	}
	for (node = root->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
			gchar *status = (gchar *) xmlNodeGetContent (node);
			guint status_num = atoi (status);
			xmlFree (status);
			if (status_num != EAS_COMMON_STATUS_OK) { // not success
				EasError error_details;
				ret = FALSE;

				// there are no sendmail-specific status codes
				if ( (EAS_COMMON_STATUS_INVALIDCONTENT <= status_num) && 
				     (status_num <= EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED)) 
				{ // it's a common status code
					error_details = common_status_error_map[status_num - 100];
				} else {
					g_warning ("unexpected send status %d", status_num);
					error_details = common_status_error_map[0];
				}

				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);

				goto finish;
			}

			continue;
		}
	}

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_send_email_msg_parse_response--\n");
	return ret;
}

