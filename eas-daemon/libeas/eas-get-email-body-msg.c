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
#include "eas-get-email-body-msg.h"
#include "eas-cal-info-translator.h"
#include "eas-con-info-translator.h"
#include <string.h>

struct _EasGetEmailBodyMsgPrivate {
	gchar* serverUid;
	gchar* collectionId;
	gchar* directoryPath;
	gchar* item;
	gchar* fullFilePath;
};

#define EAS_GET_EMAIL_BODY_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgPrivate))



G_DEFINE_TYPE (EasGetEmailBodyMsg, eas_get_email_body_msg, EAS_TYPE_MSG_BASE);

static void
eas_get_email_body_msg_init (EasGetEmailBodyMsg *object)
{
	EasGetEmailBodyMsgPrivate* priv;
	g_debug ("eas_get_email_body_msg_init++");

	object->priv = priv = EAS_GET_EMAIL_BODY_MSG_PRIVATE (object);

	priv->serverUid = NULL;
	priv->collectionId = NULL;
	priv->directoryPath = NULL;
	priv->item = NULL;
	priv->fullFilePath = NULL;

	g_debug ("eas_get_email_body_msg_init--");
}

static void
eas_get_email_body_msg_finalize (GObject *object)
{
	EasGetEmailBodyMsg* self = (EasGetEmailBodyMsg *) object;
	EasGetEmailBodyMsgPrivate* priv = self->priv;

	g_debug ("eas_get_email_body_msg_finalize++");

	g_free (priv->serverUid);
	g_free (priv->collectionId);
	g_free (priv->directoryPath);
	g_free (priv->item);
	g_free (priv->fullFilePath);

	G_OBJECT_CLASS (eas_get_email_body_msg_parent_class)->finalize (object);
	g_debug ("eas_get_email_body_msg_finalize--");
}

static void
eas_get_email_body_msg_class_init (EasGetEmailBodyMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	g_debug ("eas_get_email_body_msg_class_init++");

	g_type_class_add_private (klass, sizeof (EasGetEmailBodyMsgPrivate));

	object_class->finalize = eas_get_email_body_msg_finalize;
	g_debug ("eas_get_email_body_msg_class_init--");
}


EasGetEmailBodyMsg*
eas_get_email_body_msg_new (const gchar* serverUid, const gchar* collectionId, const char* directoryPath)
{
	EasGetEmailBodyMsg* self = NULL;
	EasGetEmailBodyMsgPrivate* priv = NULL;
	self = g_object_new (EAS_TYPE_GET_EMAIL_BODY_MSG, NULL);
	priv = self->priv;

	g_debug ("eas_get_email_body_msg_new++");

	priv->serverUid  = g_strdup (serverUid);
	priv->collectionId  = g_strdup (collectionId);
	priv->directoryPath = g_strdup (directoryPath);

	priv->fullFilePath = g_build_filename (priv->directoryPath, priv->serverUid, NULL);

	g_debug ("eas_get_email_body_msg_new--");

	return self;
}

xmlDoc*
eas_get_email_body_msg_build_message (EasGetEmailBodyMsg* self)
{
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	xmlDoc* doc = NULL;
	xmlNode *root = NULL;
	xmlNode *fetch = NULL,
		*options = NULL,
		*body_pref = NULL;

	g_debug ("eas_get_email_body_msg_build_message++");

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
	xmlNewChild (fetch, NULL, (xmlChar *) "airsync:CollectionId", (xmlChar*) priv->collectionId);
	xmlNewChild (fetch, NULL, (xmlChar *) "airsync:ServerId", (xmlChar*) priv->serverUid);

	//if we provide a mime directory location, then this will be an email request, otherwise,
	// don't include mime options, as we don't want that included
	if (priv->directoryPath) {

		options = xmlNewChild (fetch, NULL, (xmlChar *) "Options", NULL);

		xmlNewChild (options, NULL, (xmlChar *) "airsync:MIMESupport", (xmlChar*) "2"); // gives a protocol error in 12.1
		body_pref = xmlNewChild (options, NULL, (xmlChar *) "airsyncbase:BodyPreference", NULL);

		xmlNewChild (body_pref, NULL, (xmlChar *) "airsyncbase:Type", (xmlChar*) "4");  // Plain text 1, HTML 2, MIME 4
	} else {
		options = xmlNewChild (fetch, NULL, (xmlChar *) "Options", NULL);

		xmlNewChild (options, NULL, (xmlChar *) "airsync:MIMESupport", (xmlChar*) "2"); // gives a protocol error in 12.1
		body_pref = xmlNewChild (options, NULL, (xmlChar *) "airsyncbase:BodyPreference", NULL);

		xmlNewChild (body_pref, NULL, (xmlChar *) "airsyncbase:Type", (xmlChar*) "1");  // Plain text 1, HTML 2, MIME 4

	}
	g_debug ("eas_get_email_body_msg_build_message--");
	return doc;
}

gboolean
eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, xmlDoc *doc, GError** error)
{
	EasError error_details;
	gboolean ret = TRUE;
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	gchar *class = NULL;

	g_debug ("eas_get_email_body_msg_parse_response++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_warning ("no XML doc to parse");
		// Note not setting error here as empty doc is valid
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
		// Note not setting error here as this is valid
		goto finish;
	}

	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Fetch")) {
			break;
		}
	}
	if (!node) {
		g_warning ("Could not find Fetch node");
		// Note not setting error here as this is valid
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
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "CollectionId")) {
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ServerID")) {
			gchar *xmlTmp = (gchar *) xmlNodeGetContent (node);
			priv->serverUid = g_strdup (xmlTmp);
			xmlFree (xmlTmp);
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Class")) {
			class = (gchar *) xmlNodeGetContent (node);
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Properties")) {
			break;
		}
	}

	if (!node) {
		g_warning ("Failed to find Properties node");
		// Note not setting error here as this is valid
		goto finish;
	}


	if (!g_strcmp0 (class, "Email")) {
		for (node = node->children; node; node = node->next) {

			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Body")) {
				break;
			}
		}

		if (!node) {
			g_warning ("Failed to find Body node");
			// Note not setting error here as this is valid
			goto finish;
		}

		for (node = node->children; node; node = node->next) {
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Type")) {
				gchar *xmlTmp = (gchar *) xmlNodeGetContent (node);
				if (g_strcmp0 (xmlTmp, "4")) {
					//g_critical("Email type returned by server is not MIME");
					g_set_error (error, EAS_CONNECTION_ERROR,
						     EAS_CONNECTION_ERROR_FAILED,       // TODO worth adding special error code?
						     ("Email type returned by server is not MIME!"));
					xmlFree (xmlTmp);
					ret = FALSE;
					goto finish;
				}
				xmlFree (xmlTmp);
				continue;
			}

			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Data")) {
				gchar *xmlTmp = (gchar *) xmlNodeGetContent (node);
				if (!eas_get_email_body_msg_write_file (self, xmlTmp)) {
					g_critical ("Failed to open file!");
					g_set_error (error, EAS_CONNECTION_ERROR,
						     EAS_CONNECTION_ERROR_FILEERROR,
						     "Failed to open file ");
					ret = FALSE;
					xmlFree (xmlTmp);
					goto finish;
				}
				xmlFree (xmlTmp);
				break;
			}
		}
	} else if (!g_strcmp0 (class, "Calendar")) {
		g_debug ("calendar parsing");
		priv->item = eas_cal_info_translator_parse_response (node, g_strdup (priv->serverUid));
		g_debug ("calinfo = %s", priv->item);
	} else if (!g_strcmp0 (class, "Contacts")) {
		g_debug ("contact parsing");
		priv->item = eas_con_info_translator_parse_response (node, g_strdup (priv->serverUid));
	}

finish:
	xmlFree (class);
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_get_email_body_msg_parse_response--");
	return ret;

}

gboolean
eas_get_email_body_msg_write_file (EasGetEmailBodyMsg* self, gchar* data)
{
	FILE *hBody = NULL;
	EasGetEmailBodyMsgPrivate *priv = self->priv;

	g_message ("Attempting to write email to file [%s]", priv->fullFilePath);
	if ( (hBody = fopen (priv->fullFilePath, "wb"))) {
		fputs (data, hBody);
		fclose (hBody);
	} else {
		return FALSE;
	}

	return TRUE;
}

gboolean 
eas_get_email_msg_write_chunk_to_file (EasGetEmailBodyMsg* self,const guchar* chunk, gsize size)
{
	gboolean success = TRUE;
	FILE *hBody = NULL;
	EasGetEmailBodyMsgPrivate *priv = EAS_GET_EMAIL_BODY_MSG_PRIVATE (self);

	g_message ("Attempting to write email chunk to file [%s]", priv->fullFilePath);
	
	// Append to the file if it exists, otherwise create it.
	if (!(hBody = fopen (priv->fullFilePath, "ab")) ) {
		hBody = fopen (priv->fullFilePath, "wb");
		g_message ("Creating file [%s]", priv->fullFilePath);
	}

	if (hBody)
	{
		gint written = fwrite (chunk, 
			               sizeof (guchar), 
			               size, 
			               hBody);

		if (written != size)
		{
			g_warning ("Problem writing mime data segment to file!");
			success = FALSE;
		}
		g_debug ("Wrote %u of %zu bytes to file.", written, size);

		fclose (hBody);
	}
	else
	{
		success = FALSE;
	}
	
	return success;
}

gchar * eas_get_email_body_msg_get_item (EasGetEmailBodyMsg* self)
{
	return self->priv->item;
}
