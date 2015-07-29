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

#include <gio/gio.h>

#include "eas-connection-errors.h"
#include "eas-sync-folder-msg.h"
#include <eas-folder.h>
#include "../../libeasaccount/src/eas-account-list.h"

struct _EasSyncFolderMsgPrivate {
	GSList* added_folders;
	GSList* updated_folders;
	GSList* deleted_folders;

	gchar* sync_key;
	gchar* account_id;
	gchar* def_cal_folder;
	gchar* def_con_folder;
};

#define EAS_SYNC_FOLDER_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgPrivate))



G_DEFINE_TYPE (EasSyncFolderMsg, eas_sync_folder_msg, EAS_TYPE_MSG_BASE);

static void eas_sync_folder_msg_parse_fs_add_or_update (EasSyncFolderMsg *self, xmlNode *node, const gchar* topNodeName, EasAccount *acc, EasAccountList *account_list);
static void eas_sync_folder_msg_parse_fs_delete (EasSyncFolderMsg *self, xmlNode *node);


static void
eas_sync_folder_msg_init (EasSyncFolderMsg *object)
{
	EasSyncFolderMsgPrivate *priv;
	g_debug ("eas_sync_folder_msg_init++");

	object->priv = priv = EAS_SYNC_FOLDER_MSG_PRIVATE (object);

	priv->sync_key = NULL;
	priv->account_id = NULL;
	priv->added_folders = NULL;
	priv->updated_folders = NULL;
	priv->deleted_folders = NULL;

	g_debug ("eas_sync_folder_msg_init--");

}

static void
eas_sync_folder_msg_dispose (GObject *object)
{
	EasSyncFolderMsg *msg = (EasSyncFolderMsg *) object;
	EasSyncFolderMsgPrivate *priv = msg->priv;

	g_debug ("eas_sync_folder_msg_dispose++");

	g_slist_foreach (priv->added_folders, (GFunc) g_object_unref, NULL);
	g_slist_foreach (priv->updated_folders, (GFunc) g_object_unref, NULL);
	g_slist_foreach (priv->deleted_folders, (GFunc) g_object_unref, NULL);

	G_OBJECT_CLASS (eas_sync_folder_msg_parent_class)->dispose (object);

	g_debug ("eas_sync_folder_msg_dispose--");
}


static void
eas_sync_folder_msg_finalize (GObject *object)
{
	EasSyncFolderMsg *msg = (EasSyncFolderMsg *) object;
	EasSyncFolderMsgPrivate *priv = msg->priv;

	g_debug ("eas_sync_folder_msg_finalize++");

	g_free (priv->sync_key);
	g_free (priv->account_id);
	g_free (priv->def_cal_folder);
	g_free (priv->def_con_folder);

	// unref'd in dispose
	g_slist_free (priv->added_folders);
	g_slist_free (priv->updated_folders);
	g_slist_free (priv->deleted_folders);

	G_OBJECT_CLASS (eas_sync_folder_msg_parent_class)->finalize (object);

	g_debug ("eas_sync_folder_msg_finalize--");
}

static void
eas_sync_folder_msg_class_init (EasSyncFolderMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_sync_folder_msg_class_init++");

	g_type_class_add_private (klass, sizeof (EasSyncFolderMsgPrivate));

	object_class->finalize = eas_sync_folder_msg_finalize;
	object_class->dispose = eas_sync_folder_msg_dispose;
	g_debug ("eas_sync_folder_msg_class_init--");
}


EasSyncFolderMsg*
eas_sync_folder_msg_new (const gchar* syncKey, const gchar* accountId)
{
	EasSyncFolderMsg* msg = NULL;
	EasSyncFolderMsgPrivate *priv = NULL;

	msg = g_object_new (EAS_TYPE_SYNC_FOLDER_MSG, NULL);
	priv = msg->priv;

	priv->sync_key = g_strdup (syncKey);
	priv->account_id = g_strdup (accountId);
	priv->def_cal_folder = NULL;
	priv->def_con_folder = NULL;

	return msg;
}

xmlDoc*
eas_sync_folder_msg_build_message (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	xmlDoc  *doc   = NULL;
	xmlNode *node  = NULL;

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	node = xmlNewDocNode (doc, NULL, (xmlChar*) "FolderSync", NULL);
	xmlDocSetRootElement (doc, node);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	xmlNewNs (node, (xmlChar *) "FolderHierarchy:", NULL);
	xmlNewChild (node, NULL, (xmlChar *) "SyncKey", (xmlChar*) priv->sync_key);

	return doc;
}


gboolean
eas_sync_folder_msg_parse_response (EasSyncFolderMsg* self, const xmlDoc *doc, GError** error)
{
	gboolean ret = TRUE;
	EasSyncFolderMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	EasError error_details;
	EasAccount * acc = NULL;
	EasAccountList *account_list = NULL;
	GSettings *g_gsetting = NULL;

	g_gsetting = g_settings_new ("org.meego.activesyncd");
	g_assert (g_gsetting != NULL);
	/* Get list of accounts from GSettings repository */
	account_list = eas_account_list_new (g_gsetting);
	g_assert (account_list != NULL);

	acc = eas_account_list_find (account_list, EAS_ACCOUNT_FIND_ACCOUNT_UID, priv->account_id);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_warning ("folder_sync response XML is empty");
		// Note not setting error here as empty doc is valid
		goto finish;
	}
	node = xmlDocGetRootElement ( (xmlDoc*) doc);
	if (g_strcmp0 ( (char *) node->name, "FolderSync")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <FolderSync> element"));
		ret = FALSE;
		goto finish;
	}
	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
			gchar *sync_status = (gchar *) xmlNodeGetContent (node);
			guint status_num = atoi (sync_status);
			xmlFree (sync_status);
			if (status_num != EAS_COMMON_STATUS_OK) { // not success
				ret = FALSE;

				if ( (EAS_COMMON_STATUS_INVALIDCONTENT <= status_num) && (status_num <= EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED)) { // it's a common status code
					error_details = common_status_error_map[status_num - 100];
				} else {
					if (status_num > EAS_FOLDER_SYNC_STATUS_EXCEEDSSTATUSLIMIT) // not pretty, but make sure we don't overrun array if new status added by msft in future
						status_num = EAS_FOLDER_SYNC_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = folder_sync_status_error_map[status_num];
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "SyncKey")) {
			xmlChar *xmlSyncKey = xmlNodeGetContent (node);
			g_free (priv->sync_key);
			priv->sync_key = g_strdup ( (gchar *) xmlSyncKey);
			xmlFree (xmlSyncKey);
			g_debug ("FolderSync syncKey:[%s]", priv->sync_key);
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Changes")) {
			break;
		}
	}
	if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <Changes> element"));
		ret = FALSE;
		goto finish;
	}

	for (node = node->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Count")) {
			continue;
		}

		if (node->type == XML_ELEMENT_NODE && (!g_strcmp0 ( (char *) node->name, "Add") || !g_strcmp0 ( (char *) node->name, "Update"))) {
			eas_sync_folder_msg_parse_fs_add_or_update (self, node, (const gchar*) node->name, acc, account_list);
			continue;
		}

		if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Delete")) {
			eas_sync_folder_msg_parse_fs_delete (self, node);
			continue;
		}
	}

finish:
	g_object_unref (G_OBJECT (acc));
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	return ret;
}

static void
eas_sync_folder_msg_parse_fs_delete (EasSyncFolderMsg *self, xmlNode *node)
{
	EasSyncFolderMsgPrivate *priv = self->priv;

	if (!node) return;
	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Delete")) {
		xmlNode *n = node;
		gchar *serverId = NULL;

		for (n = n->children; n; n = n->next) {
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "ServerId")) {
				serverId = (gchar *) xmlNodeGetContent (n);
				continue;
			}
		}

		if (serverId) {
			EasFolder *f = NULL;

			f = eas_folder_new ();
			f->folder_id = g_strdup (serverId);

			priv->deleted_folders = g_slist_append (priv->deleted_folders, f);
			xmlFree (serverId);
		}
	}
}

static void
eas_sync_folder_msg_parse_fs_add_or_update (EasSyncFolderMsg *self, xmlNode *node, const gchar* topNodeName , EasAccount* acc, EasAccountList* account_list)
{
	EasSyncFolderMsgPrivate *priv = self->priv;

	if (!node) return;
	if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, topNodeName)) { // Add or Update
		xmlNode *n = node;
		gchar *serverId = NULL,
		       *parentId = NULL,
			*displayName = NULL,
			 *type = NULL;

		for (n = n->children; n; n = n->next) {
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "ServerId")) {
				serverId = (gchar *) xmlNodeGetContent (n);
				continue;
			}
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "ParentId")) {
				parentId = (gchar *) xmlNodeGetContent (n);
				continue;
			}
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "DisplayName")) {
				displayName = (gchar *) xmlNodeGetContent (n);
				continue;
			}
			if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Type")) {
				type = (gchar *) xmlNodeGetContent (n);
				continue;
			}
		}

		if (serverId && parentId && displayName && type) {
			EasFolder *f = NULL;

			f = eas_folder_new ();

			// Memory ownership given to EasFolder
			f->parent_id = g_strdup (parentId);
			f->folder_id = g_strdup (serverId);
			f->display_name = g_strdup (displayName);
			f->type = atoi (type);
			g_debug ("analyzing folder info: name '%s', folder '%s', type %u",
				 f->display_name, f->folder_id, f->type);
			g_debug ("calendar folder '%s', contact folder '%s'",
				 eas_account_get_calendar_folder (acc),
				 eas_account_get_contact_folder (acc));

			if (f->type == EAS_FOLDER_TYPE_DEFAULT_CALENDAR && eas_account_get_calendar_folder (acc) == NULL) {
				g_debug ("setting default calendar account %s in sync folder msg %p (priv %p)",
					 f->folder_id, self, priv);
				eas_account_set_calendar_folder (acc, f->folder_id);
				eas_account_list_save_item (account_list,
							    acc,
							    EAS_ACCOUNT_CALENDAR_FOLDER);
				priv->def_cal_folder = g_strdup (f->folder_id);
			} else if (f->type == EAS_FOLDER_TYPE_DEFAULT_CONTACTS && eas_account_get_contact_folder (acc) == NULL) {
				g_debug ("setting default contact account %s in sync folder msg %p (priv %p)",
					 f->folder_id, self, priv);
				eas_account_set_contact_folder (acc, f->folder_id);
				eas_account_list_save_item (account_list,
							    acc,
							    EAS_ACCOUNT_CONTACT_FOLDER);
				priv->def_con_folder = g_strdup (f->folder_id);
			}

			if (!g_strcmp0 ("Add", topNodeName)) {
				priv->added_folders = g_slist_append (priv->added_folders, f);
			} else if (!g_strcmp0 ("Update", topNodeName)) {
				priv->updated_folders = g_slist_append (priv->updated_folders, f);
			} else {
				g_warning ("Node was not Add or Update");
				g_object_unref (f);
			}
		} else {
			g_debug ("Failed to parse folderSync Add");
		}

		xmlFree (parentId);
		xmlFree (serverId);
		xmlFree (displayName);
		xmlFree (type);
	}
}


GSList*
eas_sync_folder_msg_get_added_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->added_folders;
}

GSList*
eas_sync_folder_msg_get_updated_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->updated_folders;
}

GSList*
eas_sync_folder_msg_get_deleted_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->deleted_folders;
}

gchar*
eas_sync_folder_msg_get_syncKey (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->sync_key;
}

gchar*
eas_sync_folder_msg_get_def_con_folder (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	g_debug ("returning default contact folder %s from sync folder msg %p (priv %p)",
		 priv->def_con_folder, self, priv);
	return priv->def_con_folder;
}

gchar*
eas_sync_folder_msg_get_def_cal_folder (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	g_debug ("returning default calendar folder %s from sync folder msg %p (priv %p)",
		 priv->def_cal_folder, self, priv);
	return priv->def_cal_folder;
}
