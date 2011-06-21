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

#include "eas-send-email-req.h"
#include "eas-send-email-msg.h"



G_DEFINE_TYPE (EasSendEmailReq, eas_send_email_req, EAS_TYPE_REQUEST_BASE);

#define EAS_SEND_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqPrivate))

struct _EasSendEmailReqPrivate
{
    EasSendEmailMsg *send_email_msg;
    gchar* account_id;
    gchar *mime_string;
    gchar* client_id;
    gchar* mime_file;
    GError *error;
};

static void
eas_send_email_req_init (EasSendEmailReq *object)
{
    /* initialization code */
    EasSendEmailReqPrivate *priv;
    g_debug ("eas_send_email_req_init++");

    object->priv = priv = EAS_SEND_EMAIL_REQ_PRIVATE (object);

    priv->account_id = NULL;
    priv->send_email_msg = NULL;
    priv->mime_string = NULL;
    priv->mime_file = NULL;
    priv->client_id = NULL;
    priv->error = NULL;

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_SEND_EMAIL);

    g_debug ("eas_send_email_req_init--");
}

static void
eas_send_email_req_finalize (GObject *object)
{
    /* deinitalization code */
    EasSendEmailReq *req = (EasSendEmailReq *) object;

    EasSendEmailReqPrivate *priv = req->priv;
    g_free (priv->mime_string);
    g_free (priv->mime_file);
    g_free (priv->client_id);
    g_free (priv->account_id);
    g_object_unref (priv->send_email_msg);
    if (priv->error)
    {
        g_error_free (priv->error);
    }

    G_OBJECT_CLASS (eas_send_email_req_parent_class)->finalize (object);
}

static void
eas_send_email_req_class_init (EasSendEmailReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

    // get rid of warnings about above 2 lines
    void *temp = (void*) object_class;
    temp = (void*) parent_class;

    g_debug ("eas_send_email_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasSendEmailReqPrivate));

    object_class->finalize = eas_send_email_req_finalize;

    g_debug ("eas_send_email_req_class_init--");
}

EasSendEmailReq *
eas_send_email_req_new (const gchar* account_id, EFlag *flag, const gchar* client_id, const gchar* mime_file)
{
    EasSendEmailReq *self = g_object_new (EAS_TYPE_SEND_EMAIL_REQ, NULL);
    EasSendEmailReqPrivate* priv = self->priv;

    g_debug ("eas_send_email_req_new++");

    eas_request_base_SetFlag (&self->parent_instance, flag);

    priv->mime_file = g_strdup (mime_file);
    priv->account_id = g_strdup (account_id);
    priv->client_id = g_strdup (client_id);

    g_debug ("eas_send_email_req_new--");

    return self;
}

// uses the message object to build xml and sends it to the connection object
gboolean
eas_send_email_req_Activate (EasSendEmailReq *self, GError** error)
{
    gboolean ret = TRUE;
    EasSendEmailReqPrivate* priv = self->priv;
    xmlDoc *doc;
    FILE *file = NULL;
    guint64 size = 0;
    size_t result = 0;

    // store flag in base (doesn't set the flag as in signal the waiters)
    g_debug ("eas_send_email_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    // open file containing mime data to be sent:
    file = fopen (priv->mime_file, "r"); // mime file can be read as text (attachments will be base 64 encoded)
    if (file == NULL)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FILEERROR,
                     ("failed to open file %s"), priv->mime_file);
        goto finish;
    }

    // read data from the file to mime_string
    // obtain file size:
    fseek (file , 0 , SEEK_END);
    size = ftell (file);
    g_debug ("file size = %llu", size);
    rewind (file);

    // allocate memory to contain the whole file:
    priv->mime_string = (gchar*) g_malloc0 (sizeof (gchar) * size);
    if (priv->mime_string == NULL)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        goto finish;
    }

    // copy the file into the buffer:
    result = fread (priv->mime_string, 1, size, file);
    if (result != size)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FILEERROR,
                     ("failed to open file %s"), priv->mime_file);
        goto finish;
    }

    g_debug ("create msg object");
    //create msg object
    priv->send_email_msg = eas_send_email_msg_new (priv->account_id, priv->client_id, priv->mime_string);

    g_debug ("build messsage");
    //build request msg
#ifdef ACTIVESYNC_14
    //Activesync 14 base64 encodes the Mime and builds an xml message
    doc = eas_send_email_msg_build_message (priv->send_email_msg);
#else
	//Activesync 12.1 just uses the mime string in the body of the message
	doc = (xmlDoc*)priv->mime_string;
#endif
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
                                       "SendMail",
                                       doc,
                                       (struct _EasRequestBase *) self,
                                       error);
finish:
    if (file == NULL)
    {
        fclose (file);
    }
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_send_email_req_Activate--");
    return ret;
}


void
eas_send_email_req_MessageComplete (EasSendEmailReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret;
    GError *error = NULL;
    EasSendEmailReqPrivate *priv = self->priv;

    g_debug ("eas_send_email_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
        priv->error = error_in;
        goto finish;
    }

    ret = eas_send_email_msg_parse_response (priv->send_email_msg, doc, &error);
    xmlFree (doc);
    if (!ret)
    {
        g_assert (error != NULL);
        self->priv->error = error;
    }

finish:
    // signal daemon we're done
    e_flag_set (eas_request_base_GetFlag (&self->parent_instance));

    g_debug ("eas_send_email_req_MessageComplete--");

    return;
}


gboolean
eas_send_email_req_ActivateFinish (EasSendEmailReq* self, GError **error)
{
    gboolean ret = TRUE;
    EasSendEmailReqPrivate *priv = self->priv;

    g_debug ("eas_send_email_req_ActivateFinish++");

    if (priv->error != NULL) // propogate any preceding error
    {
        /* store priv->error in error, if error != NULL,
        * otherwise call g_error_free() on priv->error
        */
        g_propagate_error (error, priv->error);
        priv->error = NULL;

        ret = FALSE;
    }
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_send_email_req_ActivateFinish--");

    return ret;
}
