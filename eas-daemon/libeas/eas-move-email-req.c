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

#include <stdio.h>

#include "../../libeasmail/src/libeasmail.h"
#include "eas-move-email-req.h"
#include "eas-move-email-msg.h"



G_DEFINE_TYPE (EasMoveEmailReq, eas_move_email_req, EAS_TYPE_REQUEST_BASE);

#define EAS_MOVE_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MOVE_EMAIL_REQ, EasMoveEmailReqPrivate))

struct _EasMoveEmailReqPrivate
{
    EasMoveEmailMsg *move_email_msg;
    gchar* account_id;
	GSList *server_ids_list;
    gchar *src_folder_id;
    gchar* dest_folder_id;
    GError *error;
};

static void
eas_move_email_req_init (EasMoveEmailReq *object)
{
    /* initialization code */
    EasMoveEmailReqPrivate *priv;
    g_debug ("eas_move_email_req_init++");

    object->priv = priv = EAS_MOVE_EMAIL_REQ_PRIVATE (object);

    priv->account_id = NULL;
    priv->move_email_msg = NULL;
	priv->server_ids_list = NULL;
	priv->dest_folder_id = NULL;
	priv->src_folder_id = NULL;
    priv->error = NULL;

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_MOVE_EMAIL);

    g_debug ("eas_move_email_req_init--");
}

static void
eas_move_email_req_dispose (GObject *object)
{
    /* deinitalization code */
    EasMoveEmailReq *req = (EasMoveEmailReq *) object;

    EasMoveEmailReqPrivate *priv = req->priv;

	if(priv->move_email_msg)
	{
	    g_object_unref (priv->move_email_msg);
		priv->move_email_msg = NULL;
	}

    G_OBJECT_CLASS (eas_move_email_req_parent_class)->dispose (object);
}

static void
eas_move_email_req_finalize (GObject *object)
{
    /* deinitalization code */
    EasMoveEmailReq *req = (EasMoveEmailReq *) object;

    EasMoveEmailReqPrivate *priv = req->priv;
    g_free (priv->dest_folder_id);
    g_free (priv->src_folder_id);
	g_slist_foreach (priv->server_ids_list, (GFunc) g_free, NULL);
	g_slist_free(priv->server_ids_list);
    g_free (priv->account_id);
    if (priv->error)
    {
        g_error_free (priv->error);
    }

    req->priv = NULL;

    G_OBJECT_CLASS (eas_move_email_req_parent_class)->finalize (object);
}

static void
eas_move_email_req_class_init (EasMoveEmailReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

    // get rid of warnings about above 2 lines
    void *temp = (void*) object_class;
    temp = (void*) parent_class;

    g_debug ("eas_move_email_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasMoveEmailReqPrivate));

    object_class->finalize = eas_move_email_req_finalize;
	object_class->dispose = eas_move_email_req_dispose;

    g_debug ("eas_move_email_req_class_init--");
}

EasMoveEmailReq *
eas_move_email_req_new (const gchar* account_id, EFlag *flag, const GSList* server_ids_list, const gchar* src_folder_id, const gchar* dest_folder_id)
{
    EasMoveEmailReq *self = g_object_new (EAS_TYPE_MOVE_EMAIL_REQ, NULL);
    EasMoveEmailReqPrivate* priv = self->priv;
	const GSList *l;
	
    g_debug ("eas_move_email_req_new++");

    eas_request_base_SetFlag (&self->parent_instance, flag);

    priv->account_id = g_strdup (account_id);
	priv->dest_folder_id = g_strdup(dest_folder_id);
	priv->src_folder_id = g_strdup(src_folder_id);

	// copy the gslist
	for (l = server_ids_list; l != NULL; l = g_slist_next (l))	
	{
		gchar *server_id = g_strdup((gchar *) l->data);
		priv->server_ids_list = g_slist_append(priv->server_ids_list, server_id);
	}
	
    g_debug ("eas_move_email_req_new--");

    return self;
}

// uses the message object to build xml and sends it to the connection object
gboolean
eas_move_email_req_Activate (EasMoveEmailReq *self, GError** error)
{
    gboolean ret = TRUE;
    EasMoveEmailReqPrivate* priv = self->priv;
    xmlDoc *doc;

    g_debug ("eas_move_email_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    g_debug ("create msg object");
    //create msg object
    priv->move_email_msg = eas_move_email_msg_new (priv->account_id, priv->server_ids_list, priv->src_folder_id, priv->dest_folder_id);

    g_debug ("build messsage");
    //build request msg
    doc = eas_move_email_msg_build_message (priv->move_email_msg);
    if (!doc)
    {
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        ret = FALSE;
        goto finish;
    }
    g_debug ("send message");
    ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                       "MoveItems",
                                       doc,
                                       (struct _EasRequestBase *) self,
                                       error);
finish:
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_move_email_req_Activate--");
    return ret;
}


void
eas_move_email_req_MessageComplete (EasMoveEmailReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret;
    GError *error = NULL;
    EasMoveEmailReqPrivate *priv = self->priv;

    g_debug ("eas_move_email_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
        priv->error = error_in;
        goto finish;
    }

	// parse the response (for status)
    ret = eas_move_email_msg_parse_response (priv->move_email_msg, doc, &error);
    xmlFree (doc);
    if (!ret)
    {
        g_assert (error != NULL);
        self->priv->error = error;
    }

finish:
    // signal daemon we're done
    e_flag_set (eas_request_base_GetFlag (&self->parent_instance));

    g_debug ("eas_move_email_req_MessageComplete--");

    return;
}


gboolean
eas_move_email_req_ActivateFinish (EasMoveEmailReq* self, GError **error, GSList **updated_ids)
{
    gboolean ret = TRUE;
    EasMoveEmailReqPrivate *priv = self->priv;

    g_debug ("eas_move_email_req_ActivateFinish++");

    if (priv->error != NULL) // propogate any preceding error
    {
        /* store priv->error in error, if error != NULL,
        * otherwise call g_error_free() on priv->error
        */
        g_propagate_error (error, priv->error);
        priv->error = NULL;

        ret = FALSE;
    }

	g_assert(*updated_ids == NULL);
	
	*updated_ids = eas_move_email_get_updated_ids(priv->move_email_msg);
	g_debug ("updated ids list size = %d", g_slist_length (*updated_ids));

    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_move_email_req_ActivateFinish--");

    return ret;
}