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
#include "eas-ping-msg.h"
#include "eas-email-info-translator.h"
#include "eas-connection-errors.h"
#include <eas-folder.h>

struct _EasPingMsgPrivate {
	GSList* updated_folders;
	EasPingReqState state;
};

#define EAS_PING_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PING_MSG, EasPingMsgPrivate))


G_DEFINE_TYPE (EasPingMsg, eas_ping_msg, EAS_TYPE_MSG_BASE);

static void
eas_ping_msg_init (EasPingMsg *object)
{
	EasPingMsgPrivate *priv;
	g_debug ("eas_ping_msg_init++");

	object->priv = priv = EAS_PING_MSG_PRIVATE (object);

	g_debug ("eas_ping_msg_init--");
}

static void
eas_ping_msg_finalize (GObject *object)
{
	G_OBJECT_CLASS (eas_ping_msg_parent_class)->finalize (object);
}

static void
eas_ping_msg_class_init (EasPingMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasPingMsgPrivate));

	object_class->finalize = eas_ping_msg_finalize;
}

EasPingMsg*
eas_ping_msg_new ()
{
	EasPingMsg* msg = NULL;
	msg = g_object_new (EAS_TYPE_PING_MSG, NULL);
	return msg;
}

xmlDoc*
eas_ping_msg_build_message (EasPingMsg* self, const gchar* accountId, const gchar *heartbeat, GSList *folders)
{
	xmlDoc  *doc   = NULL;
	xmlNode *node  = NULL,
		 *child = NULL,
		  *folder = NULL;
	GSList * iterator;
	gchar *folder_id = NULL;

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	node = xmlNewDocNode (doc, NULL, (xmlChar*) "Ping", NULL);
	xmlDocSetRootElement (doc, node);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	xmlNewNs (node, (xmlChar *) "Ping:", NULL);
	child = xmlNewChild (node, NULL, (xmlChar *) "HeartbeatInterval", (xmlChar*) heartbeat);
	child = xmlNewChild (node, NULL, (xmlChar *) "Folders", NULL);
	for (iterator = folders; iterator; iterator = iterator->next) {
		folder_id = (gchar*) iterator->data;
		folder = xmlNewChild (child, NULL, (xmlChar *) "Folder", NULL);
		xmlNewChild (folder, NULL, (xmlChar *) "Id", (xmlChar*) (folder_id));
		//TODO:class needs to be set properly... need some sort of lookup from type probably
		xmlNewChild (folder, NULL, (xmlChar *) "Class", (xmlChar*) "Email");
	}

	return doc;
}


gboolean
eas_ping_msg_parse_response (EasPingMsg* self, xmlDoc *doc, GError** error)
{
	gboolean ret = TRUE;
	EasPingMsgPrivate *priv = self->priv;
	xmlNode *node = NULL,
		 *appData = NULL;
	EasError error_details;
	gchar* folderid = NULL;


	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Ping Response is empty"));
		ret = FALSE;
		goto finish;
	}
	node = xmlDocGetRootElement ( (xmlDoc*) doc);
	if (g_strcmp0 ( (char *) node->name, "Ping")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <Ping> element"));
		ret = FALSE;
		goto finish;
	}
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
			gchar *sync_status = (gchar *) xmlNodeGetContent (node);
			guint status_num = atoi (sync_status);
			xmlFree (sync_status);
			switch (status_num) {
				// Ping completed with no changes - re-issue command ( headers only)
			case EAS_COMMON_STATUS_OK: {
				g_debug ("Status 1 - resend heartbeat ");
				priv->state = EasPingReqSendHeartbeat;
				goto finish;
			}
			break;
			// Ping completed with some folder changes - notify client to sync folders
			case EAS_PING_STATUS_FOLDERS_UPDATED: {
				g_debug ("Status 2 - notify client ");
				priv->state = EasPingReqNotifyClient;
			}
			break;
			default: {
				ret = FALSE;
				g_debug ("error status - return error");
				if ( (EAS_PING_STATUS_FOLDERS_UPDATED < status_num) && (status_num <= EAS_PING_STATUS_FOLDER_HIERARCHY_ERROR)) { // it's a ping status code
					error_details = ping_status_error_map[status_num];
				} else {
					if (status_num > EAS_PING_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_PING_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = ping_status_error_map[status_num];
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
			}
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Folders")) {
			g_debug ("Got Folders ");
			break;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "HeartbeatInterval")) {
			g_debug ("Got <HeartbeatInterval/>");
			break;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "MaxFolders")) {
			g_debug ("Got <MaxFolders/>");
			break;
		}
	}
	g_debug ("about to parse  folders");
	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Folders")) {
		g_debug ("parsing folders");
		appData = node;
		for (appData = appData->children; appData; appData = appData->next) {
			if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "Folder")) {
				//TODO: fix memory cleanup for this...
				folderid = (gchar *) xmlNodeGetContent (appData);
				priv->updated_folders = g_slist_append (priv->updated_folders, folderid);
			}
		}
	}
	g_debug ("eas_ping_msg_parse_response--");

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	return ret;
}

EasPingReqState
eas_ping_msg_get_state (EasPingMsg *self)
{
	EasPingMsgPrivate *priv = self->priv;
	g_debug ("eas_ping_msg_get_state +-");
	return priv->state;

}

GSList*
eas_ping_msg_get_changed_folders (EasPingMsg* self)
{
	EasPingMsgPrivate *priv = self->priv;
	return priv->updated_folders;
}



