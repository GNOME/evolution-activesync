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
#include "eas-move-email-msg.h"
#include <libeasmail.h>

#include <wbxml/wbxml.h>
#include <glib.h>

G_DEFINE_TYPE (EasMoveEmailMsg, eas_move_email_msg, EAS_TYPE_MSG_BASE);

#define EAS_MOVE_EMAIL_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MOVE_EMAIL_MSG, EasMoveEmailMsgPrivate))

struct _EasMoveEmailMsgPrivate {
	gchar* account_id;
	GSList* server_ids_list;
	gchar* src_folder_id;
	gchar* dest_folder_id;
	GSList *updated_ids_list;   // list of EasIdUpdate
};

static void
eas_move_email_msg_init (EasMoveEmailMsg *object)
{
	EasMoveEmailMsgPrivate *priv;

	/* initialization code: */
	g_debug ("eas_move_email_msg_init++");

	object->priv = priv = EAS_MOVE_EMAIL_MSG_PRIVATE (object);

	priv->account_id = NULL;
	priv->server_ids_list = NULL;
	priv->dest_folder_id = NULL;
	priv->src_folder_id = NULL;
	priv->updated_ids_list = NULL;

	g_debug ("eas_move_email_msg_init--");
}

static void
eas_move_email_msg_dispose (GObject *object)
{
	// we don't own any refs yet

	G_OBJECT_CLASS (eas_move_email_msg_parent_class)->dispose (object);
}

static void
eas_move_email_msg_finalize (GObject *object)
{
	/* deinitalization code: */
	EasMoveEmailMsg *msg = (EasMoveEmailMsg *) object;
	EasMoveEmailMsgPrivate *priv = msg->priv;

	g_free (priv->account_id);
	g_free (priv->dest_folder_id);
	g_free (priv->src_folder_id);

	g_slist_foreach (priv->server_ids_list, (GFunc) g_free, NULL);
	g_slist_free (priv->server_ids_list);
	g_slist_foreach (priv->updated_ids_list, (GFunc) eas_updatedid_free, NULL);
	g_slist_free (priv->updated_ids_list);

	G_OBJECT_CLASS (eas_move_email_msg_parent_class)->finalize (object);
}

static void
eas_move_email_msg_class_init (EasMoveEmailMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasMoveEmailMsgPrivate));

	object_class->finalize = eas_move_email_msg_finalize;
	object_class->dispose = eas_move_email_msg_dispose;
}

EasMoveEmailMsg*
eas_move_email_msg_new (const char* account_id, const GSList* server_ids_list, const gchar* src_folder_id, const gchar* dest_folder_id)
{
	EasMoveEmailMsg* msg = NULL;
	EasMoveEmailMsgPrivate *priv = NULL;
	const GSList *l = NULL;

	g_debug ("eas_move_email_msg_new++");

	msg = g_object_new (EAS_TYPE_MOVE_EMAIL_MSG, NULL);
	priv = msg->priv;

	priv->account_id = g_strdup (account_id);
	// copy the gslist
	for (l = server_ids_list; l != NULL; l = g_slist_next (l)) {
		gchar *server_id = g_strdup ( (gchar *) l->data);
		priv->server_ids_list = g_slist_append (priv->server_ids_list, server_id);
	}
	priv->src_folder_id = g_strdup (src_folder_id);
	priv->dest_folder_id = g_strdup (dest_folder_id);

	g_debug ("eas_move_email_msg_new--");
	return msg;
}

xmlDoc*
eas_move_email_msg_build_message (EasMoveEmailMsg* self)
{
	EasMoveEmailMsgPrivate *priv = self->priv;
	const GSList *l = NULL;
	xmlDoc  *doc   = NULL;
	xmlNode *root  = NULL,
		*move = NULL;

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar*) "MoveItems", NULL);
	xmlDocSetRootElement (doc, root);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");


	// no namespaces required?
	xmlNewNs (root, (xmlChar *) "Move:", NULL);

	// for each email in the list create a Move entry:
	for (l = priv->server_ids_list; l != NULL; l = g_slist_next (l)) {
		move = xmlNewChild (root, NULL, (xmlChar *) "Move", NULL);
		xmlNewChild (move, NULL, (xmlChar *) "SrcMsgId", (xmlChar*) l->data);
		xmlNewChild (move, NULL, (xmlChar *) "SrcFldId", (xmlChar*) (priv->src_folder_id));
		xmlNewChild (move, NULL, (xmlChar *) "DstFldId", (xmlChar*) (priv->dest_folder_id));
	}

	return doc;
}

gboolean
eas_move_email_msg_parse_response (EasMoveEmailMsg* self, xmlDoc *doc, GError** error)
{
	gboolean ret = TRUE;
	xmlNode *root = NULL, *node = NULL, *response = NULL;
	EasMoveEmailMsgPrivate *priv = self->priv;
	g_debug ("eas_move_email_msg_parse_response++\n");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!doc) {
		g_debug ("Failed: no doc supplied");
		// Note not setting error here as empty doc is valid
		goto finish;
	}
	root = xmlDocGetRootElement (doc);
	if (g_strcmp0 ( (char *) root->name, "MoveItems")) {
		g_debug ("Failed: not a MoveItems response!");
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
			     ("Failed to find <MoveItems> element"));
		ret = FALSE;
		goto finish;
	}

	for (response = root->children; response; response = response->next) {
		EasIdUpdate *updated_id = NULL;

		if (response->type == XML_ELEMENT_NODE && g_strcmp0 ( (char *) response->name, "Response")) {
			g_debug ("Failed: not a MoveItems Response!, got %s", response->name);
			g_set_error (error, EAS_CONNECTION_ERROR,
				     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
				     ("Failed to find <Response> element"));
			ret = FALSE;
			goto finish;
		}

		updated_id = g_malloc0 (sizeof (EasIdUpdate));

		for (node = response->children; node; node = node->next) {
			g_debug ("got Response child %s", node->name);
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status")) {
				gchar *status = (gchar *) xmlNodeGetContent (node);
				guint status_num = atoi (status);
				if (status_num != 3) { // not success (MoveItems command status 3 is success (all other commands use 1 for success), doh!)
					// store the status
					updated_id->status = g_strdup (status);
				}
				xmlFree (status);
				continue;
			}
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "DstMsgId")) {
				gchar *dstmsgid = (gchar *) xmlNodeGetContent (node);
				updated_id->dest_id = g_strdup (dstmsgid);
				xmlFree (dstmsgid);
				continue;
			}
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "SrcMsgId")) {
				gchar *srcmsgid = (gchar *) xmlNodeGetContent (node);
				updated_id->src_id = g_strdup (srcmsgid);
				xmlFree (srcmsgid);
				continue;
			}
		}
		if (updated_id->src_id != NULL) { // TODO not sure why this is necessary, but we do appear to get empty Responses!?
			g_debug ("appending updated id. src = %s, dst = %s, status = %s", updated_id->src_id, updated_id->dest_id, updated_id->status);
			priv->updated_ids_list = g_slist_append (priv->updated_ids_list, updated_id);
		} else {
			eas_updatedid_free (updated_id);
		}
	}

finish:
	if (!ret) {
		g_debug ("returning error");
		g_assert (priv->updated_ids_list == NULL);
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_move_email_msg_parse_response--\n");

	return ret;
}

GSList*
eas_move_email_get_updated_ids (EasMoveEmailMsg* self)
{
	EasMoveEmailMsgPrivate *priv = self->priv;
	GSList *l = priv->updated_ids_list;
	priv->updated_ids_list = NULL; // we pass ownership back, so avoid trying to free
	return l;
}
