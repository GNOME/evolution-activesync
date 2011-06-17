/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 *
 */

#include "eas-delete-email-req.h"
#include "eas-sync-msg.h"

struct _EasDeleteEmailReqPrivate
{
    EasSyncMsg* syncMsg;
    gchar* accountID;
    gchar* syncKey;
    gchar* folder_id;
    GSList *server_ids_array;
    EasItemType ItemType;
    GError *error;
};

#define EAS_DELETE_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReqPrivate))

G_DEFINE_TYPE (EasDeleteEmailReq, eas_delete_email_req, EAS_TYPE_REQUEST_BASE);

static void
eas_delete_email_req_init (EasDeleteEmailReq *object)
{
    EasDeleteEmailReqPrivate *priv;

    object->priv = priv = EAS_DELETE_EMAIL_REQ_PRIVATE (object);

    g_debug ("eas_delete_email_req_init++");
    priv->accountID = NULL;
    priv->syncKey = NULL;
    priv->error = NULL;
    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_DELETE_MAIL);

    g_debug ("eas_delete_email_req_init--");
}

static void
eas_delete_email_req_finalize (GObject *object)
{
    EasDeleteEmailReq *req = (EasDeleteEmailReq *) object;
    EasDeleteEmailReqPrivate *priv = req->priv;

    g_debug ("eas_delete_email_req_finalize++");

    g_free (priv->syncKey);
    g_slist_foreach (priv->server_ids_array, (GFunc) g_object_unref, NULL);
    g_slist_free (priv->server_ids_array);
    g_free (priv->accountID);
    if (priv->error)
    {
        g_error_free (priv->error);
    }
    g_debug ("eas_delete_email_req_finalize--");
    G_OBJECT_CLASS (eas_delete_email_req_parent_class)->finalize (object);
}

static void
eas_delete_email_req_class_init (EasDeleteEmailReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);
    void *tmp = parent_class;
    tmp = object_class;

    g_debug ("eas_delete_email_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasDeleteEmailReqPrivate));

    object_class->finalize = eas_delete_email_req_finalize;
    g_debug ("eas_delete_email_req_class_init--");

}

gboolean
eas_delete_email_req_Activate (EasDeleteEmailReq *self, GError** error)
{
    gboolean ret;
    EasDeleteEmailReqPrivate *priv = self->priv;
    xmlDoc *doc;
    gboolean getChanges = FALSE;

    g_debug ("eas_delete_email_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    //create sync  msg type
    priv->syncMsg = eas_sync_msg_new (priv->syncKey, priv->accountID, priv->folder_id, priv->ItemType);

    g_debug ("eas_delete_email_req_Activate- syncKey = %s", priv->syncKey);

    g_debug ("eas_delete_email_req_Activate - build messsage");
    //build request msg
    doc = eas_sync_msg_build_message (priv->syncMsg, getChanges, NULL, NULL, priv->server_ids_array);
	if(!doc)
	{
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
		ret = FALSE;
		goto finish;
	}
    g_debug ("eas_delete_email_req_Activate - send message");
    ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                       "Sync",
                                       doc,
                                       (struct _EasRequestBase *) self,
                                       error);

finish:	
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }

    g_debug ("eas_delete_email_req_Activate--");

    return ret;
}

EasDeleteEmailReq *eas_delete_email_req_new (const gchar* accountId, const gchar *syncKey, const gchar *folderId, const GSList *server_ids_array, EFlag *flag)
{
    EasDeleteEmailReq* self = g_object_new (EAS_TYPE_DELETE_EMAIL_REQ, NULL);
    EasDeleteEmailReqPrivate *priv = self->priv;
    guint listCount;
    guint listLen = g_slist_length ( (GSList*) server_ids_array);
    gchar *server_id = NULL;

    g_debug ("eas_delete_email_req_new++");
    g_assert (syncKey);

    priv->syncKey = g_strdup (syncKey);
    priv->folder_id = g_strdup (folderId);

    for (listCount = 0; listCount < listLen; listCount++)
    {
        server_id = g_slist_nth_data ( (GSList*) server_ids_array, listCount);
        priv->server_ids_array = g_slist_append (priv->server_ids_array, g_strdup (server_id));
    }

    priv->accountID = g_strdup (accountId);
    eas_request_base_SetFlag (&self->parent_instance, flag);

    g_debug ("eas_delete_email_req_new--");
    return self;
}

gboolean
eas_delete_email_req_ActivateFinish (EasDeleteEmailReq* self, gchar** ret_sync_key, GError** error)
{
    gboolean ret = TRUE;
    EasDeleteEmailReqPrivate *priv = self->priv;

    g_debug ("eas_delete_email_req_ActivateFinish++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    if (priv->error != NULL) // propogate any preceding error
    {
        /* store priv->error in error, if error != NULL,
        * otherwise call g_error_free() on priv->error
        */
        g_propagate_error (error, priv->error);
        priv->error = NULL;

        ret = FALSE;
    }
    *ret_sync_key = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));

    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_delete_email_req_ActivateFinish--");

    return ret;
}

void eas_delete_email_req_MessageComplete (EasDeleteEmailReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret;
    EasDeleteEmailReqPrivate *priv = self->priv;
    GError *error = NULL;

    g_debug ("eas_delete_email_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
        priv->error = error_in;
        goto finish;
    }

    ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
    xmlFree (doc);
    if (!ret)
    {
        g_assert (error != NULL);
        self->priv->error = error;
    }

finish:
    e_flag_set (eas_request_base_GetFlag (&self->parent_instance));

    g_debug ("eas_delete_email_req_MessageComplete--");
}

