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
#include "eas-sync-msg.h"
#include "eas-email-info-translator.h"
#include "eas-cal-info-translator.h"
#include "eas-con-info-translator.h"

#include <string.h>

struct _EasSyncMsgPrivate {
	GSList* added_items;
	GSList* updated_items;
	GSList* deleted_items;
	GSList* update_responses;
	gboolean more_available;
	gchar* sync_key_in;
	gchar* sync_key_out;
	gchar* folderID;
	EasConnection *connection;

	EasItemType ItemType;
};

#define EAS_SYNC_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_MSG, EasSyncMsgPrivate))


G_DEFINE_TYPE (EasSyncMsg, eas_sync_msg, EAS_TYPE_MSG_BASE);

//static void eas_sync_parse_item_add(EasSyncMsg *self, xmlNode *node, GError** error);

static void
eas_sync_msg_init (EasSyncMsg *object)
{
	EasSyncMsgPrivate *priv;
	g_debug ("eas_sync_msg_init++");

	object->priv = priv = EAS_SYNC_MSG_PRIVATE (object);

	priv->more_available = FALSE;
	priv->sync_key_in = NULL;
	priv->sync_key_out = NULL;
	priv->folderID = NULL;
	priv->connection = NULL;
	priv->added_items = NULL;
	priv->updated_items = NULL;
	priv->deleted_items = NULL;
	priv->update_responses = NULL;

	g_debug ("eas_sync_msg_init--");
}

static void
eas_sync_msg_finalize (GObject *object)
{
	EasSyncMsg *msg = (EasSyncMsg *) object;
	EasSyncMsgPrivate *priv = msg->priv;

	g_debug ("eas_sync_msg_finalize++");

	g_free (priv->sync_key_in);
	g_free (priv->sync_key_out);
	g_free (priv->folderID);
	if (priv->connection)
		g_object_unref (priv->connection);

	g_slist_foreach (priv->added_items, (GFunc) g_free, NULL);
	g_slist_foreach (priv->updated_items, (GFunc) g_free, NULL);
	g_slist_foreach (priv->deleted_items, (GFunc) xmlFree, NULL);

	g_slist_free (priv->added_items);
	g_slist_free (priv->updated_items);
	g_slist_free (priv->deleted_items);

	G_OBJECT_CLASS (eas_sync_msg_parent_class)->finalize (object);
	g_debug ("eas_sync_msg_finalize--");
}

static void
eas_sync_msg_class_init (EasSyncMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasSyncMsgPrivate));

	object_class->finalize = eas_sync_msg_finalize;
}

EasSyncMsg*
eas_sync_msg_new (const gchar* syncKey, EasConnection *conn, const gchar *folderID, const EasItemType type)
{
	EasSyncMsg* msg = NULL;
	EasSyncMsgPrivate *priv = NULL;

	msg = g_object_new (EAS_TYPE_SYNC_MSG, NULL);
	priv = msg->priv;

	priv->more_available = FALSE;
	priv->sync_key_in = g_strdup (syncKey);
	priv->connection = g_object_ref (conn);
	g_debug ("new sync message for folder '%s'", folderID);
	priv->folderID = g_strdup (folderID);
	priv->ItemType = type;

	return msg;
}

xmlDoc*
eas_sync_msg_build_message (EasSyncMsg* self, guint filter_type, gboolean getChanges, GSList *added, GSList *updated, GSList *deleted)
{
	EasSyncMsgPrivate *priv = self->priv;
	xmlDoc  *doc   = NULL;
	xmlNode *node  = NULL,
		 *child = NULL,
		  *collection = NULL,
		   *options = NULL,
		    *body_pref = NULL;
	gchar filter[2] = "0";
	GSList * iterator;

	int protover = eas_connection_get_protocol_version (priv->connection);

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	node = xmlNewDocNode (doc, NULL, (xmlChar*) "Sync", NULL);
	xmlDocSetRootElement (doc, node);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	xmlNewNs (node, (xmlChar *) "AirSync:", NULL);
	xmlNewNs (node, (xmlChar *) "AirSyncBase:", (xmlChar *) "airsyncbase");
	child = xmlNewChild (node, NULL, (xmlChar *) "Collections", NULL);
	collection = xmlNewChild (child, NULL, (xmlChar *) "Collection", NULL);


	if (protover <= 120) {
		// Include <Class> element; protocol 12.0 seems to require it */
		if (priv->ItemType == EAS_ITEM_MAIL)
			xmlNewChild (collection, NULL, (xmlChar *) "Class", (xmlChar *) "Email");
		else if (priv->ItemType == EAS_ITEM_CONTACT)
			xmlNewChild (collection, NULL, (xmlChar *) "Class", (xmlChar *) "Contact");
		else if (priv->ItemType == EAS_ITEM_CALENDAR)
			xmlNewChild (collection, NULL, (xmlChar *) "Class", (xmlChar *) "Calendar");
	}
	xmlNewChild (collection, NULL, (xmlChar *) "SyncKey", (xmlChar*) priv->sync_key_in);
	g_debug ("sync message for collection '%s'", priv->folderID);
	xmlNewChild (collection, NULL, (xmlChar *) "CollectionId", (xmlChar*) priv->folderID);
	xmlNewChild (collection, NULL, (xmlChar *) "DeletesAsMoves", (xmlChar*) "1");

	// TODO Refactor this function into subfunctions for each aspect.

	// if get changes = true - means we are pulling from the server
	if (getChanges) {
		xmlNewChild (collection, NULL, (xmlChar *) "GetChanges", (xmlChar*) "1");
		xmlNewChild (collection, NULL, (xmlChar *) "WindowSize", (xmlChar*) "100");

		if (priv->ItemType == EAS_ITEM_MAIL) {
			g_assert (filter_type <= 5);
			snprintf (filter, sizeof (filter) / sizeof (filter[0]), "%d", filter_type);

			options = xmlNewChild (collection, NULL, (xmlChar *) "Options", NULL);
			xmlNewChild (options, NULL, (xmlChar *) "FilterType", (xmlChar*) filter);
			xmlNewChild (options, NULL, (xmlChar *) "MIMESupport", (xmlChar*) "2");
			xmlNewChild (options, NULL, (xmlChar *) "MIMETruncation", (xmlChar*) "4"); // Fetch first 10KiB to get all the mail headers

			body_pref = xmlNewChild (options, NULL, (xmlChar *) "airsyncbase:BodyPreference", NULL);
			xmlNewChild (body_pref, NULL, (xmlChar *) "airsyncbase:Type", (xmlChar*) "4"); // Plain text 1, HTML 2, MIME 4
			xmlNewChild (body_pref, NULL, (xmlChar *) "airsyncbase:TruncationSize", (xmlChar*) "200000");
		} else if (priv->ItemType == EAS_ITEM_CALENDAR || priv->ItemType == EAS_ITEM_CONTACT) {
			g_assert ( (filter_type == 0) || (4 <= filter_type && filter_type <= 7)); // TODO verify that we enforce this at the public api
			snprintf (filter, sizeof (filter) / sizeof (filter[0]), "%d", filter_type);

			options = xmlNewChild (collection, NULL, (xmlChar *) "Options", NULL);
			xmlNewChild (options, NULL, (xmlChar *) "FilterType", (xmlChar*) filter);

			body_pref = xmlNewChild (options, NULL, (xmlChar *) "airsyncbase:BodyPreference", NULL);
			xmlNewChild (body_pref, NULL, (xmlChar *) "airsyncbase:Type", (xmlChar*) "1"); // Plain text 1, HTML 2, MIME 4
			xmlNewChild (body_pref, NULL, (xmlChar *) "airsyncbase:TruncationSize", (xmlChar*) "200000");
		}
	} else {
		// In protocol 12.0, do not include <GetChanges> node when SyncKey is zero.
		// The server doesn't like it.
		if (protover > 120 || strcmp (priv->sync_key_in, "0"))
			xmlNewChild (collection, NULL, (xmlChar *) "GetChanges", (xmlChar*) "0");
	}

	//if any of the lists are not null we need to add commands element
	if (added || updated || deleted) {
		xmlNode *command = xmlNewChild (collection, NULL, (xmlChar *) "Commands", NULL);
		if (added) {
			for (iterator = added; iterator; iterator = iterator->next) {
				//choose translator based on data type
				switch (priv->ItemType) {
				default: {
					g_debug ("Unknown Data Type  %d", priv->ItemType);
				}
				break;
				case EAS_ITEM_MAIL: {
					g_critical ("Trying to do Add with Mail type - This is not allowed");
				}
				break;
				case EAS_ITEM_CALENDAR: {
					xmlNode *added = xmlNewChild (command, NULL, (xmlChar *) "Add", NULL);
					xmlNewNs (node, (xmlChar *) "Calendar:", (xmlChar *) "calendar");
					if (iterator->data) {
						//TODO: call translator to get client ID and  encoded application data
						//gchar *serialised_calendar = (gchar *)iterator->data;
						xmlNode *app_data = NULL;
						EasItemInfo *cal_info = (EasItemInfo*) iterator->data;

						// create the server_id node
						xmlNewChild (added, NULL, (xmlChar *) "ClientId", (xmlChar*) cal_info->client_id);
						app_data = xmlNewChild (added, NULL, (xmlChar *) "ApplicationData", NULL);
						// translator deals with app data
						eas_cal_info_translator_parse_request (doc, app_data, cal_info);
						// TODO error handling and freeing
					}
				}
				break;
				case EAS_ITEM_CONTACT: {
					xmlNode *added = xmlNewChild (command, NULL, (xmlChar *) "Add", NULL);
					xmlNewNs (node, (xmlChar *) "Contacts2:", (xmlChar *) "contacts2");
					if (iterator->data) {
						//TODO: call translator to get client ID and  encoded application data
						//gchar *serialised_calendar = (gchar *)iterator->data;
						xmlNode *app_data = NULL;
						EasItemInfo *cal_info = (EasItemInfo*) iterator->data;

						// create the server_id node
						xmlNewChild (added, NULL, (xmlChar *) "ClientId", (xmlChar*) cal_info->client_id);
						app_data = xmlNewChild (added, NULL, (xmlChar *) "ApplicationData", NULL);
						// translator deals with app data
						eas_con_info_translator_parse_request (doc, app_data, cal_info);
						// TODO error handling and freeing
					}
				}
				break;

				}

			}
		}
		if (updated) {
			for (iterator = updated; iterator; iterator = iterator->next) {
				xmlNode *update = xmlNewChild (command, NULL, (xmlChar *) "Change", NULL);
				//choose translator based on data type
				switch (priv->ItemType) {
				default: {
					g_debug ("Unknown Data Type  %d", priv->ItemType);
				}
				break;
				case EAS_ITEM_MAIL: {
					gchar *serialised_email = (gchar *) iterator->data;
					EasEmailInfo *email_info = eas_email_info_new ();

					xmlNewNs (node, (xmlChar *) "Email:", (xmlChar *) "email");

					if (eas_email_info_deserialise (email_info, serialised_email)) {
						xmlNode *app_data = NULL;
						// create the server_id node
						xmlNewChild (update, NULL, (xmlChar *) "ServerId", (xmlChar*) email_info->server_id);
						app_data = xmlNewChild (update, NULL, (xmlChar *) "ApplicationData", NULL);
						// call translator to get encoded application data
						eas_email_info_translator_build_update_request (doc, app_data, email_info);
					}
					g_object_unref (email_info);
					// TODO error handling
				}
				break;
				case EAS_ITEM_CALENDAR: {
					xmlNewNs (node, (xmlChar *) "Calendar:", (xmlChar *) "calendar");
					if (iterator->data) {
						//TODO: call translator to get client ID and  encoded application data
						//gchar *serialised_calendar = (gchar *)iterator->data;

						EasItemInfo *cal_info = (EasItemInfo*) iterator->data;
						xmlNode *app_data = NULL;
						// create the server_id node
						xmlNewChild (update, NULL, (xmlChar *) "ServerId", (xmlChar*) cal_info->server_id);
						app_data = xmlNewChild (update, NULL, (xmlChar *) "ApplicationData", NULL);
						// translator deals with app data
						eas_cal_info_translator_parse_request (doc, app_data, cal_info);
						// TODO error handling and freeing
					}
				}
				break;
				case EAS_ITEM_CONTACT: {
					xmlNewNs (node, (xmlChar *) "Contacts2:", (xmlChar *) "contacts2");
					if (iterator->data) {
						//TODO: call translator to get client ID and  encoded application data
						//gchar *serialised_calendar = (gchar *)iterator->data;

						EasItemInfo *cal_info = (EasItemInfo*) iterator->data;
						xmlNode *app_data = NULL;
						// create the server_id node
						xmlNewChild (update, NULL, (xmlChar *) "ServerId", (xmlChar*) cal_info->server_id);
						app_data = xmlNewChild (update, NULL, (xmlChar *) "ApplicationData", NULL);
						// translator deals with app data
						//TODO: add contact translator
						eas_con_info_translator_parse_request (doc, app_data, cal_info);
						// TODO error handling and freeing
					}
				}
				}
			}
		}
		if (deleted) {
			for (iterator = deleted; iterator; iterator = iterator->next) {
				xmlNode *delete = xmlNewChild (command, NULL, (xmlChar *) "Delete", NULL);
				xmlNewChild (delete, NULL, (xmlChar *) "ServerId", iterator->data);
			}

		}
	}// end if (added/deleted/updated)

	return doc;
}


gboolean
eas_sync_msg_parse_response (EasSyncMsg* self, xmlDoc *doc, GError** error)
{
	gboolean ret = TRUE;
	EasSyncMsgPrivate *priv = self->priv;
	xmlNode *node = NULL,
		 *appData = NULL;

	gchar *item_server_id = NULL;
	gchar *item_client_id = NULL;
	gchar *item_status = NULL;

	g_debug ("eas_sync_msg_parse_response ++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_warning ("folder_sync response XML is empty");
		// Note not setting error here as empty doc is valid, but copying old sync key so we
		//don't lose it
		g_free (priv->sync_key_out);
		priv->sync_key_out = g_strdup (priv->sync_key_in);
		goto finish;
	}
	node = xmlDocGetRootElement (doc);

	//TODO: parse response correctly

	if (g_strcmp0 ( (char *) node->name, "Sync")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <Sync> element"));
		ret = FALSE;
		goto finish;
	}
	for (node = node->children; node; node = node->next) {
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
					if (status_num > EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = sync_status_error_map[status_num];
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Collections")) {
			g_debug ("Collections:");
			break;
		}

	}
	if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <Collections> element"));
		ret = FALSE;
		goto finish;
	}

	for (node = node->children; node; node = node->next) {

		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Collection")) {
			g_debug ("Collection:");
			break;
		}

	}
	if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <Collection> element"));
		ret = FALSE;
		goto finish;
	}

	for (node = node->children; node; node = node->next) {
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
					if (status_num > EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = sync_status_error_map[status_num];
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "SyncKey")) {
			xmlChar* xmlSyncKeyOut = xmlNodeGetContent (node);

			g_free (priv->sync_key_out);
			priv->sync_key_out = g_strdup ( (gchar *) xmlSyncKeyOut);
			xmlFree (xmlSyncKeyOut);

			g_debug ("Got SyncKey = %s", priv->sync_key_out);
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "MoreAvailable")) {
			priv->more_available = TRUE;
			g_debug ("Got <MoreAvailable/>");
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Responses")) {
			g_debug ("Responses:\n");
			break;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Commands")) {
			g_debug ("Commands:\n");
			break;
		}

	}

	if (!node) {
		g_warning ("Found no <Responses> element or <Commands> element>");
		// Note not setting error here as this is valid
		goto finish;
	}

	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Commands")) {
		for (node = node->children; node; node = node->next) {
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Add")) {
				appData = node;

				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ServerId")) {
						if (item_server_id) xmlFree (item_server_id);
						item_server_id = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}

					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ApplicationData")) {
						gchar *flatItem = NULL;
						g_debug ("Found AppliicationData - about to parse and flatten");
						//choose translator based on data type
						switch (priv->ItemType) {
						default: {
							g_debug ("Unknown Data Type  %d", priv->ItemType);
						}
						break;
						case EAS_ITEM_MAIL: {
							flatItem = eas_email_info_translator_parse_add_response (appData, g_strdup (item_server_id));
						}
						break;
						case EAS_ITEM_CALENDAR: {
							flatItem = eas_cal_info_translator_parse_response (appData, g_strdup (item_server_id));
						}
						break;
						case EAS_ITEM_CONTACT: {
							flatItem = eas_con_info_translator_parse_response (appData, g_strdup (item_server_id));
						}
						break;
						}

						g_debug ("FlatItem = %s", flatItem);
						if (flatItem) {
							g_debug ("appending to added_items");
							priv->added_items = g_slist_append (priv->added_items, flatItem);
						}
					}

				} // End for
				if (item_server_id) {
					xmlFree (item_server_id);
					item_server_id = NULL;
				}
				continue;
			}

			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Delete")) {
				appData = node;
				// TODO Parse deleted folders
				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ServerId")) {
						if (item_server_id) xmlFree (item_server_id);
						item_server_id = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						priv->deleted_items = g_slist_append (priv->deleted_items, g_strdup (item_server_id));
						continue;
					}
				}
				continue;
			}

			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Change")) {
				// TODO Parse updated folders
				appData = node;

				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ServerId")) {
						if (item_server_id) xmlFree (item_server_id);
						item_server_id = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ApplicationData")) {
						gchar *flatItem = NULL;
						g_debug ("Found AppliicationData - about to parse and flatten");
						//choose translator based on data type
						switch (priv->ItemType) {
						default: {
							g_debug ("Unknown Data Type  %d", priv->ItemType);
						}
						break;
						case EAS_ITEM_MAIL: {
							g_debug ("calling email appdata translator for update");
							flatItem = eas_email_info_translator_parse_update_response (appData, g_strdup (item_server_id));
						}
						break;
						case EAS_ITEM_CALENDAR: {
							flatItem = eas_cal_info_translator_parse_response (appData, g_strdup (item_server_id));
						}
						break;
						case EAS_ITEM_CONTACT: {
							//TODO: add contact translator
							flatItem = eas_con_info_translator_parse_response (appData, g_strdup (item_server_id));
						}
						break;

						}

						g_debug ("FlatItem = %s", flatItem);
						if (flatItem) {
							g_debug ("appending to updated_items");
							priv->updated_items = g_slist_append (priv->updated_items, flatItem);
						}

						continue;
					}
				}
				continue;
			}
		}
	} else if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Responses")) {
		for (node = node->children; node; node = node->next) {

			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Add")) {
				gchar *flatItem = NULL;
				EasItemInfo *info = NULL;
				appData = node;
				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ClientId")) {
						if (item_client_id) xmlFree (item_client_id);
						item_client_id = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found clientID for Item = %s", item_client_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ServerId")) {
						if (item_server_id) xmlFree (item_server_id);
						item_server_id = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "Status")) {
						if (item_status) xmlFree (item_status);
						item_status = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found Status for Item  = %s", item_status);
						continue;
					}
				}
				info = eas_item_info_new ();
				info->client_id = item_client_id;
				item_client_id = NULL;
				info->server_id = item_server_id;
				item_server_id = NULL;
				info->status = item_status;
				item_status = NULL;
				eas_item_info_serialise (info, &flatItem);
				g_object_unref (info);
				if (flatItem) {
					g_debug ("appending to added_items");
					priv->added_items = g_slist_append (priv->added_items, flatItem);
				}
				continue;
			}   // end add
			else if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Change")) {
				gchar *flat_item = NULL;
				appData = node;

				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "ServerId")) {
						if (item_server_id) xmlFree (item_server_id);
						item_server_id = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) appData->name, "Status")) {
						if (item_status) xmlFree (item_status);
						item_status = (gchar *) xmlNodeGetContent (appData);
						g_debug ("Found Status for Item  = %s", item_status);
						continue;
					}
				}
				// create a flattened item of appropriate type:
				switch (priv->ItemType) {
				case EAS_ITEM_MAIL: {
					EasEmailInfo *email_info = eas_email_info_new();

					g_debug ("got status of %s for email with server id %s", item_status, item_server_id);
					email_info->server_id = g_strdup (item_server_id);
					email_info->status = g_strdup (item_status);
					if (!eas_email_info_serialise (email_info, &flat_item)) {
						g_warning ("Failed to serialise email");
					}
					g_object_unref (email_info);
				}
				break;
				case EAS_ITEM_CALENDAR: {
					// TODO
				}
				break;
				case EAS_ITEM_CONTACT: {
					// TODO
				}
				break;
				default: {
					g_debug ("Unknown Data Type  %d", priv->ItemType);
				}
				break;
				}

				if (flat_item) {
					g_debug ("appending %s to updated_items", flat_item);
					priv->update_responses = g_slist_append (priv->update_responses, flat_item);
				}
				continue;
			}   // end Change
		} // end for node = node->children
	}   // end Responses

	g_debug ("eas_sync_msg_parse_response--");

finish:

	if (item_server_id) {
		xmlFree (item_server_id);
	}
	if (item_status) {
		xmlFree (item_status);
	}
	if (item_client_id) {
		xmlFree (item_client_id);
	}
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	return ret;
}


GSList*
eas_sync_msg_get_added_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	g_debug ("eas added items list size = %d", g_slist_length (priv->added_items));
	return priv->added_items;
}

GSList*
eas_sync_msg_get_updated_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->updated_items;
}

GSList*
eas_sync_msg_get_update_responses (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->update_responses;
}


GSList*
eas_sync_msg_get_deleted_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->deleted_items;
}

gchar*
eas_sync_msg_get_syncKey (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	g_debug ("eas_sync_msg_getSyncKey +-");
	return priv->sync_key_out;
}

gboolean
eas_sync_msg_get_more_available (EasSyncMsg *self)
{
	EasSyncMsgPrivate *priv = self->priv;
	g_debug ("eas_sync_msg_get_more_available +-");
	return priv->more_available;

}

