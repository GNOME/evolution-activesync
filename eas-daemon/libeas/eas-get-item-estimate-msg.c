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
#include "eas-get-item-estimate-msg.h"
#include <libeasmail.h>

#include <wbxml/wbxml.h>
#include <glib.h>

G_DEFINE_TYPE (EasGetItemEstimateMsg, eas_get_item_estimate_msg, EAS_TYPE_MSG_BASE);

#define EAS_GET_ITEM_ESTIMATE_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_ITEM_ESTIMATE_MSG, EasGetItemEstimateMsgPrivate))

struct _EasGetItemEstimateMsgPrivate {
	gchar* folder_id;
	gchar* sync_key;
	guint estimate;
	EasConnection *connection;
};

static void
eas_get_item_estimate_msg_init (EasGetItemEstimateMsg *object)
{
	EasGetItemEstimateMsgPrivate *priv;

	/* initialization code: */
	g_debug ("eas_get_item_estimate_msg_init++");

	object->priv = priv = EAS_GET_ITEM_ESTIMATE_MSG_PRIVATE (object);

	priv->sync_key = NULL;
	priv->folder_id = NULL;
	priv->connection = NULL;
	priv->estimate = 0;

	g_debug ("eas_get_item_estimate_msg_init--");
}

static void
eas_get_item_estimate_msg_dispose (GObject *object)
{
	// unref any objects that might ref us (none currently)

	G_OBJECT_CLASS (eas_get_item_estimate_msg_parent_class)->dispose (object);
}

static void
eas_get_item_estimate_msg_finalize (GObject *object)
{
	/* deinitalization code: */
	EasGetItemEstimateMsg *msg = (EasGetItemEstimateMsg *) object;

	EasGetItemEstimateMsgPrivate *priv = msg->priv;

	g_free (priv->sync_key);
	g_free (priv->folder_id);
	if (priv->connection)
		g_object_unref (priv->connection);

	G_OBJECT_CLASS (eas_get_item_estimate_msg_parent_class)->finalize (object);
}

static void
eas_get_item_estimate_msg_class_init (EasGetItemEstimateMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasGetItemEstimateMsgPrivate));

	object_class->finalize = eas_get_item_estimate_msg_finalize;
	object_class->dispose = eas_get_item_estimate_msg_dispose;
}

EasGetItemEstimateMsg*
eas_get_item_estimate_msg_new (EasConnection *conn, const gchar *sync_key, const gchar* folder_id)
{
	EasGetItemEstimateMsg* msg = NULL;
	EasGetItemEstimateMsgPrivate *priv = NULL;

	g_debug ("eas_get_item_estimate_msg_new++");

	msg = g_object_new (EAS_TYPE_GET_ITEM_ESTIMATE_MSG, NULL);
	priv = msg->priv;

	priv->sync_key = g_strdup (sync_key);
	priv->folder_id = g_strdup (folder_id);
	priv->connection = g_object_ref (conn);

	g_debug ("eas_get_item_estimate_msg_new--");
	return msg;
}

xmlDoc*
eas_get_item_estimate_msg_build_message (EasGetItemEstimateMsg* self)
{
	EasGetItemEstimateMsgPrivate *priv = self->priv;
	xmlDoc  *doc   = NULL;
	xmlNode *root  = NULL,
		*collections = NULL,
		*collection = NULL;
	int protover = eas_connection_get_protocol_version (priv->connection);

	g_debug ("protover = %d", protover);
	doc = xmlNewDoc ( (xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar*) "GetItemEstimate", NULL);
	xmlDocSetRootElement (doc, root);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	xmlNewNs (root, (xmlChar *) "GetItemEstimate:", NULL);
	xmlNewNs (root, (xmlChar *) "AirSync:", (xmlChar *) "airsync");

	collections = xmlNewChild (root, NULL, (xmlChar *) "Collections", NULL);
	collection = xmlNewChild (collections, NULL, (xmlChar *) "Collection", NULL);
	if (protover <= 121) { // 12.1: options element not supported, SyncKey element is placed after the FilterType element
		xmlNewChild (collection, NULL, (xmlChar *) "CollectionId", (xmlChar*) (priv->folder_id));
		//xmlNewChild (collection, NULL, (xmlChar *) "Class", (xmlChar*) "Email");   // including class produces a bad collection status
		xmlNewChild (collection, NULL, (xmlChar *) "FilterType", (xmlChar*) "0");
		xmlNewChild (collection, NULL, (xmlChar *) "SyncKey", (xmlChar*) (priv->sync_key));
	} else {
		xmlNewChild (collection, NULL, (xmlChar *) "SyncKey", (xmlChar*) (priv->sync_key));
		xmlNewChild (collection, NULL, (xmlChar *) "CollectionId", (xmlChar*) (priv->folder_id));
		// TODO we may want to specify a time window in future (needs to match sync).
		/*
		options = xmlNewChild (collection, NULL, (xmlChar *) "Options", NULL);
		xmlNewChild (options, NULL, (xmlChar *) "FilterType", (xmlChar*) "0");
		xmlNewChild (options, NULL, (xmlChar *) "Class", (xmlChar*) "Email");
		*/
	}
	return doc;
}

gboolean
eas_get_item_estimate_msg_parse_response (EasGetItemEstimateMsg* self, xmlDoc *doc, GError** error)
{
	gboolean ret = TRUE;
	xmlNode *root = NULL, *node = NULL, *response = NULL, *leaf = NULL;
	EasGetItemEstimateMsgPrivate *priv = self->priv;

	g_debug ("eas_get_item_estimate_msg_parse_response++\n");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_debug ("Failed: no doc supplied");
		// Note not setting error here as empty doc is valid
		goto finish;
	}
	root = xmlDocGetRootElement (doc);
	if (g_strcmp0 ( (char *) root->name, "GetItemEstimate")) {
		g_debug ("Failed: not a GetItemEstimate response!");
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <GetItemEstimate> element"));
		ret = FALSE;
		goto finish;
	}

	for (response = root->children; response; response = response->next) {
		if (response->type == XML_ELEMENT_NODE && g_strcmp0 ( (char *) response->name, "Response")) {
			g_debug ("Failed: not a GetItemEstimate Response!, got %s", response->name);
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
				     ("Failed to find <Response> element"));
			ret = FALSE;
			goto finish;
		}

		for (node = response->children; node; node = node->next) {
			g_debug ("got Response child %s", node->name);
			// Status
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
				gchar *sync_status = (gchar *) xmlNodeGetContent (node);
				guint status_num = atoi (sync_status);
				xmlFree (sync_status);
				if (status_num != EAS_COMMON_STATUS_OK) { // not success
					EasError error_details;
					ret = FALSE;

					if ( (EAS_COMMON_STATUS_INVALIDCONTENT <= status_num) && (status_num <= EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED)) { // it's a common status code
						error_details = common_status_error_map[status_num - 100];
					} else {
						if (status_num > EAS_GETITEMESTIMATE_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added
							status_num = EAS_GETITEMESTIMATE_STATUS_EXCEEDSSTATUSLIMIT;

						error_details = get_item_estimate_status_error_map[status_num];
					}
					g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
					goto finish;
				}
				continue;
			}
			// Collection
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Collection")) {
				for (leaf = node->children; leaf; leaf = leaf->next) {
					// collectionid
					if (leaf->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) leaf->name, "CollectionId")) {
						continue;
					}
					// estimate
					if (leaf->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) leaf->name, "Estimate")) {
						gchar *estimate;

						estimate = (gchar*) xmlNodeGetContent (leaf);
						priv->estimate = atoi (estimate);
						g_debug ("got estimate %d", priv->estimate);

						xmlFree (estimate);
						continue;
					}
				}

				continue;
			}
		}
	}

finish:
	if (!ret) {
		g_debug ("returning error");
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_get_item_estimate_msg_parse_response--\n");

	return ret;
}

guint
eas_get_item_estimate_get_estimate (EasGetItemEstimateMsg* self)
{
	EasGetItemEstimateMsgPrivate *priv = self->priv;
	guint estimate = priv->estimate;

	return estimate;
}
