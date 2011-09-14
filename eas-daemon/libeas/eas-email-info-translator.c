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

#define _XOPEN_SOURCE
#define _BSD_SOURCE

#include <time.h>
#include "eas-email-info-translator.h"
#include <eas-folder.h>
#include <eas-attachment.h>
#include <eas-email-info.h>
#include <libxml/tree.h>
#include <string.h>
#include <ctype.h>

/**
 * @param[in]  str
 *	  String to be parsed.
 * @param[out] headers
 *	  If set a GSList of EasEmailHeader structures. Caller is responsible for
 *	  freeing elements in the list with g_free(), and the list itself with
 *	  g_slist_free(). [full transfer]
 *
 * @returns NULL or position navigated through 'str'.
 */
static gchar *
parse_header (gchar *str, GSList **headers)
{
	EasEmailHeader *header;

	char *eol, *colon, *next;

	colon = strchr (str, ':');
	if (!colon)
		return NULL;

	eol = strchr (str, '\n');
	if (eol < colon)
		return NULL;
more:
	if (!eol)
		return NULL;

	next = eol + 1; // The \r or first char of next line
	if (*next == '\r')
		* (next++) = ' ';

	if (*next == ' ' || *next == '\t') { // Header continuation
		*eol = ' ';
		eol = strchr (next, '\n');
		goto more;
	}


	*eol = 0;
	* (colon++) = 0;
	while (*colon && isspace (*colon))
		colon++;

	header = g_malloc0 (sizeof (EasEmailHeader));
	header->name = g_strdup (str);
	header->value = g_strdup (colon);
	*headers = g_slist_append (*headers, header);
	return next;
}

gchar *
eas_email_info_translator_parse_add_response (const xmlNode *node, gchar *server_id)
{
	gchar *result = NULL;
	g_debug ("eas_email_info_translator_add++");

	if (!server_id) return NULL;
	if (!node) {
		g_free (server_id);
		return NULL;
	}

	// Parse the email specific (applicationdata) part of a sync response
	// and generate the app-specific parts of the EasEmailInfo structure
	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ApplicationData")) {
		EasEmailInfo *email_info = eas_email_info_new();
		xmlNode *n = (xmlNode *) node;
		GSList *headers = NULL;
		GSList *attachments = NULL;
		guint32  flags = 0;
		int importance = 1;
		GSList *categories = NULL;

		g_debug ("found ApplicationData root");

		for (n = n->children; n; n = n->next) {
			xmlChar* xmlNodeContent = xmlNodeGetContent (n);

			//Received TODO date-received is NOT a standard MIME header and should be put into a separate field in email info
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "DateReceived")) {
				struct tm tm;

				if (strptime ( (gchar*) xmlNodeContent, "%Y-%m-%dT%H:%M:%S", &tm))
					email_info->date_received = timegm (&tm);
			}
			//DisplayTo	 - is there an equivalent standard email header?
			//Importance
			else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Importance")) {
				importance = strtol ( (gchar*) xmlNodeContent, NULL, 0);
				g_debug ("importance = %d", importance);
				flags |= EAS_VALID_IMPORTANCE;
			}
			//Read
			else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Read")) {
				if (g_strcmp0 ( (gchar*) xmlNodeContent, "0")) { // not 0, therefore read
					flags |= EAS_EMAIL_READ;
				}
				flags |= EAS_VALID_READ;
			}
			// TODO which if any of these other headers are standard email headers?
			//ThreadTopic
			//MessageClass			-   ?
			//MeetingRequest stuff  -   ignoring, not MIME header
			//InternetCPID			-   ignoring, EAS specific
			//Task 'Flag' stuff		-   ignoring, not MIME header
			//ContentClass			-   ?

			//Attachments
			else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Attachments")) {
				xmlNode *s = n;
				g_debug ("found attachments");
				for (s = s->children; s; s = s->next) {
					if (s->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) s->name, "Attachment")) {
						EasAttachment *attachment = eas_attachment_new();
						xmlNode *t = s;
						g_debug ("found attachment");
						for (t = t->children; t; t = t->next) {
							//DisplayName
							if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) t->name, "DisplayName")) {
								attachment->display_name = xmlNodeGetContent (t);
								g_debug ("attachment name = %s", attachment->display_name);
							}
							//EstimatedDataSize
							if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) t->name, "EstimatedDataSize")) {
								char *tmp = (gchar *) xmlNodeGetContent (t);
								attachment->estimated_size = strtol (tmp, NULL, 0);
								xmlFree (tmp);
								g_debug ("attachment size = %d", attachment->estimated_size);
							}
							//FileReference
							if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) t->name, "FileReference")) {
								attachment->file_reference = xmlNodeGetContent (t);
								g_debug ("file reference = %s", attachment->file_reference);
							}
							//Method            - not storing
						}
						attachments = g_slist_append (attachments, attachment);
					}
				}
			}

			//Categories
			else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Categories")) {
				xmlNode *s = n;
				g_debug ("found categories");
				for (s = s->children; s; s = s->next) {
					if (s->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) s->name, "Category")) {
						categories = g_slist_append (categories, (char *) xmlNodeGetContent (s));
					}
				}
			}

			// Body
			else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Body")) {
				xmlNode *t;
				g_debug ("found body");
				for (t = n->children; t; t = t->next) {
					// DisplayName
					if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) t->name, "EstimatedDataSize")) {
						char *tmp = (gchar *) xmlNodeGetContent (t);
						email_info->estimated_size = strtol (tmp, NULL, 0);
						xmlFree (tmp);
						g_debug ("body size = %zd", email_info->estimated_size);
					}
					// Data
					if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) t->name, "Data")) {
						gchar *data = (gchar *) xmlNodeGetContent (t);
						gchar *p = data;
						while (p && *p != '\n' && *p != '\r')
							p = parse_header (p, &headers);

						xmlFree (data);
					}
				}
			}

			xmlFree (xmlNodeContent);
		} // end for

		email_info->server_id = server_id; // full transfer
		email_info->headers = headers;
		email_info->attachments = attachments;
		email_info->categories = categories;
		email_info->flags = flags;
		email_info->importance = importance;

		// serialise the emailinfo
		if (!eas_email_info_serialise (email_info, &result)) {
			g_warning ("Failed to serialise email info");
		}

		g_object_unref (email_info);
	} else {
		g_error ("Failed! Expected ApplicationData node at root");
	}

	g_debug ("eas_email_info_translator_add--");
	return result;
}

gchar *
eas_email_info_translator_parse_update_response (const xmlNode *node, gchar *server_id)
{
	gchar *result = NULL;
	g_debug ("eas_email_info_translator_update++");

	if (!server_id) return NULL;
	if (!node) {
		g_free (server_id);
		return NULL;
	}

	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ApplicationData")) {
		EasEmailInfo *email_info = eas_email_info_new();
		GSList *categories = NULL;
		guint flags = 0;
		xmlNode *n = (xmlNode *) node;

		g_debug ("found ApplicationData root");

		for (n = n->children; n; n = n->next) {
			// TODO - figure out if/where other flags are stored (eg replied to/forwarded in ConversationIndex?)
			//Read
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Read")) {
				xmlChar * xmlNodeContent = xmlNodeGetContent (n);

				g_debug ("found read node");
				if (g_strcmp0 ( (gchar *) xmlNodeContent, "0")) { // not 0, therefore read
					flags |= EAS_EMAIL_READ;
				}
				flags |= EAS_VALID_READ;
				xmlFree (xmlNodeContent);
				continue;
			}
			//Categories
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Categories")) {
				xmlNode *s = n;
				for (s = s->children; s; s = s->next) {
					if (s->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) s->name, "Category")) {
						// Full transfer of xmlNodeGetContents()
						categories = g_slist_append (categories, (char *) xmlNodeGetContent (s));
					}
				}
				continue;
			}
		} // end for

		email_info->server_id = server_id; // full transfer
		email_info->categories = categories;
		email_info->flags = flags;

		// serialise the emailinfo
		if (!eas_email_info_serialise (email_info, &result)) {
			g_warning ("Failed to serialise email info");
		}

		g_object_unref (email_info);
	} else {
		g_error ("Failed! Expected ApplicationData node at root");
	}

	g_debug ("eas_email_info_translator_update--");
	return result;
}


gchar *
eas_email_info_translator_parse_delete_response (const xmlNode *node, gchar *server_id)
{
	gchar *result = NULL;

	if (!server_id) return NULL;
	if (!node) {
		g_free (server_id);
		return NULL;
	}

	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ApplicationData")) {
		EasEmailInfo *email_info = eas_email_info_new();

		email_info->server_id = server_id; // full transfer
		// no other data supplied for deleted email, done

		if (!eas_email_info_serialise (email_info, &result)) {
			g_warning ("Failed to serialise email info");
		}

		g_object_unref (email_info);
	} else {
		g_critical ("Failed! Expected ApplicationData node at root");
	}

	return result;
}

static gboolean
create_node_from_categorylist (xmlNode *app_data, const GSList* categories)
{
	gboolean ret = TRUE;

	xmlNode *categories_node, *leaf = NULL;
	if (categories) {
		g_debug ("Creating Categories node");
		// Create the categories collection node
		// TODO - can namespace name (email) be included in name like this rather than as a xmlNsPtr param?
		categories_node = xmlNewChild (app_data, NULL, (xmlChar *) "email:Categories", NULL);
	}

	while (categories != NULL) {
		g_debug ("Creating Categorys node for %s", (gchar *) categories->data);
		leaf = xmlNewTextChild (categories_node, NULL, (xmlChar *) "email:Category", (xmlChar*) categories->data);
		if (!leaf) {
			ret = FALSE;
		}
		categories = categories->next;
	}

	return ret;
}


// translate the other way: take the emailinfo object and populate the ApplicationData node
gboolean
eas_email_info_translator_build_update_request (const xmlDocPtr doc, xmlNode *app_data, const EasEmailInfo *email_info)
{
	gboolean ret = TRUE;
	g_debug ("eas_email_info_translator_build_update_request++");

	if (! (doc &&
	       app_data &&
	       email_info &&
	       (app_data->type == XML_ELEMENT_NODE) &&
	       (g_strcmp0 ( (char*) (app_data->name), "ApplicationData") == 0))) {
		g_debug ("invalid input");
		ret = FALSE;
	} else {
		// Note that the only fields it's valid to update are flags and categories!

		// flags
		if (email_info->flags & EAS_VALID_READ) {
			if (email_info->flags & EAS_EMAIL_READ) {
				g_debug ("setting Read to 1");
				xmlNewChild (app_data, NULL, (xmlChar *) "Read", (xmlChar*) "1");
			} else {
				g_debug ("setting Read to 0");
				xmlNewChild (app_data, NULL, (xmlChar *) "Read", (xmlChar*) "0");
			}
		}
		if(email_info->categories)
		{
			//categories
			ret = create_node_from_categorylist (app_data, email_info->categories);
		}
	}

	if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 5)) {
		xmlChar* dump_buffer;
		int dump_buffer_size;
		xmlIndentTreeOutput = 1;
		xmlDocDumpFormatMemory (doc, &dump_buffer, &dump_buffer_size, 1);
		g_debug ("XML DOCUMENT DUMPED:\n%s", dump_buffer);
		xmlFree (dump_buffer);
	}

	g_debug ("eas_email_info_translator_build_update_request--");
	return ret;
}
