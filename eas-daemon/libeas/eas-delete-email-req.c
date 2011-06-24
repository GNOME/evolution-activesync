/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include "eas-delete-email-req.h"
#include "eas-sync-msg.h"

struct _EasDeleteEmailReqPrivate
{
    EasSyncMsg* syncMsg;
    gchar* accountID;
    gchar* syncKey;
    gchar* folder_id;
    GSList *server_ids_array;
    EasItemType itemType;
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
    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_DELETE_MAIL);

    g_debug ("eas_delete_email_req_init--");
}

static void
eas_delete_email_req_dispose (GObject *object)
{
    EasDeleteEmailReq *req = (EasDeleteEmailReq *) object;
    EasDeleteEmailReqPrivate *priv = req->priv;
	GSList *item = NULL;

    g_debug ("eas_delete_email_req_dispose++");

	for (item = priv->server_ids_array; item; item = item->next)
	{
		if (item->data)
		{
			g_object_unref(G_OBJECT(item->data));
			item->data = NULL;
		}
	}

    g_debug ("eas_delete_email_req_dispose--");
    G_OBJECT_CLASS (eas_delete_email_req_parent_class)->dispose (object);
}

static void
eas_delete_email_req_finalize (GObject *object)
{
    EasDeleteEmailReq *req = (EasDeleteEmailReq *) object;
    EasDeleteEmailReqPrivate *priv = req->priv;

    g_debug ("eas_delete_email_req_finalize++");

    g_free (priv->syncKey);
    g_slist_free (priv->server_ids_array);
    g_free (priv->accountID);
    g_debug ("eas_delete_email_req_finalize--");
    G_OBJECT_CLASS (eas_delete_email_req_parent_class)->finalize (object);
}

static void
eas_delete_email_req_class_init (EasDeleteEmailReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_debug ("eas_delete_email_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasDeleteEmailReqPrivate));

    object_class->finalize = eas_delete_email_req_finalize;
    object_class->dispose = eas_delete_email_req_dispose;
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
    priv->syncMsg = eas_sync_msg_new (priv->syncKey, priv->accountID, priv->folder_id, priv->itemType);

    g_debug ("eas_delete_email_req_Activate- syncKey = %s", priv->syncKey);

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
	
    ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                       "Sync",
                                       doc, // full transfer
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

EasDeleteEmailReq *eas_delete_email_req_new (const gchar* accountId, const gchar *syncKey, const gchar *folderId, const GSList *server_ids_array, DBusGMethodInvocation *context)
{
    EasDeleteEmailReq* self = g_object_new (EAS_TYPE_DELETE_EMAIL_REQ, NULL);
    EasDeleteEmailReqPrivate *priv = self->priv;
    guint listCount;
    guint listLen = g_slist_length ( (GSList*) server_ids_array);
    gchar *server_id = NULL;

    g_debug ("eas_delete_email_req_new++");
	g_return_val_if_fail(syncKey != NULL, NULL);

    priv->syncKey = g_strdup (syncKey);
    priv->folder_id = g_strdup (folderId);

    for (listCount = 0; listCount < listLen; listCount++)
    {
        server_id = g_slist_nth_data ( (GSList*) server_ids_array, listCount);
        priv->server_ids_array = g_slist_append (priv->server_ids_array, g_strdup (server_id));
    }

    priv->accountID = g_strdup (accountId);
    eas_request_base_SetContext (&self->parent_instance, context);

    g_debug ("eas_delete_email_req_new--");
    return self;
}


gboolean eas_delete_email_req_MessageComplete (EasDeleteEmailReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret = TRUE;
    EasDeleteEmailReqPrivate *priv = self->priv;
    GError *error = NULL;
	gchar *ret_sync_key = NULL;
	
    g_debug ("eas_delete_email_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
		ret = FALSE;
        error = error_in;
        goto finish;
    }

    ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
    if (!ret)
    {
        g_assert (error != NULL);
		goto finish;
    }

	ret_sync_key = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));

finish:
    if(!ret)
	{
		dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
	}
    else
    {
        //TODO: make sure this stuff is ok to go over dbus.

        dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance),
                              ret_sync_key);
    }
    xmlFree (doc);
	g_free(ret_sync_key);

    g_debug ("eas_delete_email_req_MessageComplete--");
	return TRUE;
}

