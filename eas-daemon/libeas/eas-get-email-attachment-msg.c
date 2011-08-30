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

#include <wbxml/wbxml.h>
#include "eas-connection-errors.h"
#include "eas-get-email-attachment-msg.h"
#include <glib.h>

struct _EasGetEmailAttachmentMsgPrivate {
	gchar* fileReference;
	gchar* directoryPath;
};

#define EAS_GET_EMAIL_ATTACHMENT_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgPrivate))



G_DEFINE_TYPE (EasGetEmailAttachmentMsg, eas_get_email_attachment_msg, EAS_TYPE_MSG_BASE);

static void
eas_get_email_attachment_msg_init (EasGetEmailAttachmentMsg *object)
{
	EasGetEmailAttachmentMsgPrivate* priv = NULL;
	g_debug ("eas_get_email_attachment_msg_init++");

	object->priv = priv = EAS_GET_EMAIL_ATTACHMENT_MSG_PRIVATE (object);

	priv->fileReference = NULL;
	priv->directoryPath = NULL;

	g_debug ("eas_get_email_attachment_msg_init--");
}

static void
eas_get_email_attachment_msg_finalize (GObject *object)
{
	EasGetEmailAttachmentMsg* self = (EasGetEmailAttachmentMsg*) object;
	EasGetEmailAttachmentMsgPrivate* priv = self->priv;
	g_debug ("eas_get_email_attachment_msg_finalize++");

	g_free (priv->fileReference);
	g_free (priv->directoryPath);

	G_OBJECT_CLASS (eas_get_email_attachment_msg_parent_class)->finalize (object);
	g_debug ("eas_get_email_attachment_msg_finalize--");
}

static void
eas_get_email_attachment_msg_class_init (EasGetEmailAttachmentMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_get_email_attachment_msg_class_init++");
	g_type_class_add_private (klass, sizeof (EasGetEmailAttachmentMsgPrivate));

	object_class->finalize = eas_get_email_attachment_msg_finalize;
	g_debug ("eas_get_email_attachment_msg_class_init++");
}

EasGetEmailAttachmentMsg*
eas_get_email_attachment_msg_new (const gchar* fileReference, const char* directoryPath)
{
	EasGetEmailAttachmentMsg* self = NULL;
	EasGetEmailAttachmentMsgPrivate* priv = NULL;
	self = g_object_new (EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, NULL);
	priv = self->priv;

	g_debug ("eas_get_email_attachment_msg_new++");

	priv->fileReference  = g_strdup (fileReference);
	priv->directoryPath = g_strdup (directoryPath);

	g_debug ("eas_get_email_attachment_msg_new--");

	return self;
}

xmlDoc*
eas_get_email_attachment_msg_build_message (EasGetEmailAttachmentMsg* self)
{

	EasGetEmailAttachmentMsgPrivate *priv = self->priv;
	xmlDoc* doc = NULL;

	xmlNode *root = NULL;
	xmlNode *fetch = NULL;

	g_debug ("eas_get_email_attachment_msg_build_message++");

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar*) "ItemOperations", NULL);
	xmlDocSetRootElement (doc, root);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	xmlNewNs (root, (xmlChar *) "ItemOperations:", NULL);
	xmlNewNs (root, (xmlChar *) "AirSync:", (xmlChar *) "airsync");
	xmlNewNs (root, (xmlChar *) "AirSyncBase:", (xmlChar *) "airsyncbase");

	fetch = xmlNewChild (root, NULL, (xmlChar *) "Fetch", NULL);
	xmlNewChild (fetch, NULL, (xmlChar *) "Store", (xmlChar*) "Mailbox");
	xmlNewChild (fetch, NULL, (xmlChar *) "airsyncbase:FileReference", (xmlChar*) priv->fileReference);

	g_debug ("eas_get_email_attachment_msg_build_message--");
	return doc;
}

gboolean
eas_get_email_attachment_msg_parse_response (EasGetEmailAttachmentMsg* self,
					     xmlDoc *doc,
					     GError** error)
{
	EasError error_details;
	gboolean ret = TRUE;
	xmlNode *node = NULL;

	g_debug ("eas_get_email_attachment_msg_parse_response ++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_warning ("No XML Doc to parse");
		// not setting error since this is valid
		goto finish;
	}
	node = xmlDocGetRootElement (doc);
	if (g_strcmp0 ( (char *) node->name, "ItemOperations")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <ItemOperations> element"));
		ret = FALSE;
		goto finish;
	}

	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
			gchar *status = (gchar *) xmlNodeGetContent (node);
			guint status_num = atoi (status);
			xmlFree (status);
			if (status_num != EAS_COMMON_STATUS_OK) { // not success
				ret = FALSE;

				if ( (EAS_COMMON_STATUS_INVALIDCONTENT <= status_num) && (status_num <= EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED)) { // it's a common status code
					error_details = common_status_error_map[status_num - 100];
				} else {
					if (status_num > EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = itemoperations_status_error_map[status_num];
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Response")) {
			break;
		}
	}
	if (!node) {
		g_warning ("Could not find Response node");
		goto finish;
	}

	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Fetch")) {
			break;
		}
	}
	if (!node) {
		g_warning ("Could not find Fetch node");
		goto finish;
	}

	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
			gchar *status = (gchar *) xmlNodeGetContent (node);
			guint status_num = atoi (status);
			xmlFree (status);
			if (status_num != EAS_COMMON_STATUS_OK) { // not success
				ret = FALSE;

				if ( (EAS_COMMON_STATUS_INVALIDCONTENT <= status_num) && (status_num <= EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED)) { // it's a common status code
					error_details = common_status_error_map[status_num - 100];
				} else {
					if (status_num > EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = itemoperations_status_error_map[status_num];
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "FileReference")) {
			gchar *xmlTmp = (gchar *) xmlNodeGetContent (node);
			g_debug ("FileReference:[%s]", xmlTmp);
			xmlFree (xmlTmp);
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Class")) {
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Properties")) {
			break;
		}
	}

	if (!node) {
		g_warning ("Failed to find Properties node");
		goto finish;
	}


	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ContentType")) {
			gchar *xmlTmp = (gchar *) xmlNodeGetContent (node);
			g_debug ("ContentType:[%s]", xmlTmp);
			//TODO: do we need to handle the ContentType?
			xmlFree (xmlTmp);
			continue;
		}

		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Data")) {
			gchar *xmlTmp = (gchar *) xmlNodeGetContent (node);

			ret = eas_get_email_attachment_msg_write_file (self, xmlTmp, error);

			xmlFree (xmlTmp);
			break;
		}
	}

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	return ret;
	g_debug ("eas_get_email_attachment_msg_parse_response --");
}

gboolean
eas_get_email_attachment_msg_write_file (EasGetEmailAttachmentMsg *self, gchar * data, GError **error)
{
	gsize decoded_len = 0;
	guchar* decoded_buf = g_base64_decode ( (const gchar*) data, &decoded_len);
	gchar* fullFilePath = NULL;
	FILE *hAttachement = NULL;
	gboolean ret = TRUE;
	EasGetEmailAttachmentMsgPrivate *priv = self->priv;

	g_message ("data ecoded length  =--->:[%zu]",  strlen (data));
	g_message ("data encoded   =--->:[%s]",   data);

	if (!decoded_len) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_WBXMLERROR,
			     ("Failed to base64 decode attachment"));
		ret = FALSE;
		goto finish;
	}
	g_message ("data decoded   =--->:[%s]",   decoded_buf);
	g_message ("data decoded length =--->:[%zu]",  decoded_len);

	fullFilePath = g_build_filename (priv->directoryPath, priv->fileReference, NULL);
	g_message ("Attempting to write attachment to file [%s]", fullFilePath);
	if ( (hAttachement = fopen (fullFilePath, "wb"))) {
		fwrite ( (const WB_UTINY*) decoded_buf, decoded_len, 1, hAttachement);
		fclose (hAttachement);
	} else {
		//g_critical("Failed to open file!")
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_FILEERROR,
			     ("Failed to open file!"));
		ret = FALSE;
	}
finish:
	g_free (decoded_buf);
	g_free (fullFilePath);
	return ret;
}
