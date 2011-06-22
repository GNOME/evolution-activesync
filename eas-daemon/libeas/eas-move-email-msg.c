/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 *
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eas-connection-errors.h"
#include "eas-move-email-msg.h"
#include <wbxml/wbxml.h>
#include <glib.h>

G_DEFINE_TYPE (EasMoveEmailMsg, eas_move_email_msg, EAS_TYPE_MSG_BASE);

#define EAS_MOVE_EMAIL_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MOVE_EMAIL_MSG, EasMoveEmailMsgPrivate))

struct _EasMoveEmailMsgPrivate
{
    gchar* account_id;
    GSList* server_ids_list;
    gchar* src_folder_id;
    gchar* dest_folder_id;	
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

    g_debug ("eas_move_email_msg_init--");
}

static void
eas_move_email_msg_dispose (GObject *object)
{
	// we don't own any refs
	
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
	g_slist_free(priv->server_ids_list);
	
    G_OBJECT_CLASS (eas_move_email_msg_parent_class)->finalize (object);
}

static void
eas_move_email_msg_class_init (EasMoveEmailMsgClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);

    // get rid of warnings about above 2 lines
    void *temp = (void*) object_class;
    temp = (void*) parent_class;

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
	for (l = server_ids_list; l != NULL; l = g_slist_next (l))	
	{
		gchar *server_id = g_strdup((gchar *) l->data);
		priv->server_ids_list = g_slist_append(priv->server_ids_list, server_id);		
	}	
    priv->src_folder_id = g_strdup (src_folder_id);
	priv->dest_folder_id = g_strdup(dest_folder_id);

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
            *leaf = NULL,
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
    for(l = priv->server_ids_list; l != NULL; l = g_slist_next(l))
    {
		move = xmlNewChild (root, NULL, (xmlChar *) "Move", NULL);                        
		leaf = xmlNewChild (move, NULL, (xmlChar *) "SrcMsgId", (xmlChar*) l->data);
		leaf = xmlNewChild (move, NULL, (xmlChar *) "SrcFldId", (xmlChar*) (priv->src_folder_id));                        
		leaf = xmlNewChild (move, NULL, (xmlChar *) "DstFldId", (xmlChar*) (priv->dest_folder_id));                        
	}
                        
    return doc;
}

gboolean
eas_move_email_msg_parse_response (EasMoveEmailMsg* self, xmlDoc *doc, GError** error)
{
    gboolean ret = TRUE;
    xmlNode *root, *node = NULL;
    g_debug ("eas_move_email_msg_parse_response++\n");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    if (!doc)
    {
        g_debug ("Failed: no doc supplied");
        // Note not setting error here as empty doc is valid
        goto finish;
    }
    root = xmlDocGetRootElement (doc);
    if (g_strcmp0 ( (char *) root->name, "MoveItems"))
    {
        g_debug ("Failed: not a MoveItems response!");
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,
                     ("Failed to find <MoveItems> element"));
        ret = FALSE;
        goto finish;
    }
    for (node = root->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status"))
        {
            gchar *status = (gchar *) xmlNodeGetContent (node);
            guint status_num = atoi (status);
            xmlFree (status);
			// lrm TODO - for the MoveItems command status 3 is success, doh!
            if (status_num != EAS_COMMON_STATUS_OK) // not success
            {
                EasError error_details;
                ret = FALSE;

                // there are no movemail-specific status codes
                if ( (EAS_COMMON_STATUS_INVALIDCONTENT <= status_num) && (status_num <= EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED)) // it's a common status code
                {
                    error_details = common_status_error_map[status_num - 100];
                }
                else
                {
                    g_warning ("unexpected move status %d", status_num);
                    error_details = common_status_error_map[0];
                }
                g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
                goto finish;
            }

            continue;
        }
		// lrm TODO look for updated msg ids
    }

finish:
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_move_email_msg_parse_response++\n");
    return ret;
}

