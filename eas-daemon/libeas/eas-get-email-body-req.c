/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include "eas-get-email-body-msg.h"
#include "eas-get-email-body-req.h"


struct _EasGetEmailBodyReqPrivate
{
    EasGetEmailBodyMsg* emailBodyMsg;
    gchar* accountUid;
    gchar* serverId;
    gchar* collectionId;
    gchar* mimeDirectory;
};

#define EAS_GET_EMAIL_BODY_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_BODY_REQ, EasGetEmailBodyReqPrivate))



G_DEFINE_TYPE (EasGetEmailBodyReq, eas_get_email_body_req, EAS_TYPE_REQUEST_BASE);

static void
eas_get_email_body_req_init (EasGetEmailBodyReq *object)
{
    EasGetEmailBodyReqPrivate* priv;
    g_debug ("eas_get_email_body_req_init++");
    object->priv = priv = EAS_GET_EMAIL_BODY_REQ_PRIVATE (object);

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_GET_EMAIL_BODY);

    priv->emailBodyMsg = NULL;
    priv->accountUid = NULL;
	priv->mimeDirectory = NULL;
	priv->serverId = NULL;
	priv->collectionId = NULL;
	
    g_debug ("eas_get_email_body_req_init--");
}

static void
eas_get_email_body_req_dispose (GObject *object)
{
    EasGetEmailBodyReq* self = (EasGetEmailBodyReq *) object;
    EasGetEmailBodyReqPrivate *priv = self->priv;

    g_debug ("eas_get_email_body_req_dispose++");
	
    if (priv->emailBodyMsg)
    {
        g_object_unref (priv->emailBodyMsg);
		priv->emailBodyMsg = NULL;
    }
	
    G_OBJECT_CLASS (eas_get_email_body_req_parent_class)->dispose (object);
	
    g_debug ("eas_get_email_body_req_dispose--");
}


static void
eas_get_email_body_req_finalize (GObject *object)
{
    EasGetEmailBodyReq* self = (EasGetEmailBodyReq *) object;
    EasGetEmailBodyReqPrivate *priv = self->priv;

    g_debug ("eas_get_email_body_req_finalize++");

    g_free (priv->serverId);
    g_free (priv->collectionId);
    g_free (priv->mimeDirectory);
    g_free (priv->accountUid);

    G_OBJECT_CLASS (eas_get_email_body_req_parent_class)->finalize (object);
    g_debug ("eas_get_email_body_req_finalize--");
}

static void
eas_get_email_body_req_class_init (EasGetEmailBodyReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_get_email_body_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasGetEmailBodyReqPrivate));

    object_class->finalize = eas_get_email_body_req_finalize;
    object_class->dispose = eas_get_email_body_req_dispose;
    g_debug ("eas_get_email_body_req_class_init--");
}


EasGetEmailBodyReq*
eas_get_email_body_req_new (const gchar* account_uid,
                            const gchar *collection_id,
                            const gchar *server_id,
                            const gchar *mime_directory,
                            DBusGMethodInvocation* context)
{
    EasGetEmailBodyReq* req = NULL;
    EasGetEmailBodyReqPrivate *priv = NULL;

    g_debug ("eas_get_email_body_req_new++");

    req = g_object_new (EAS_TYPE_GET_EMAIL_BODY_REQ, NULL);
    priv = req->priv;

    priv->accountUid = g_strdup (account_uid);
    priv->collectionId = g_strdup (collection_id);
    priv->serverId = g_strdup (server_id);
    priv->mimeDirectory = g_strdup (mime_directory);
    eas_request_base_SetContext(&req->parent_instance, context);

    g_debug ("eas_get_email_body_req_new--");
    return req;
}


gboolean
eas_get_email_body_req_Activate (EasGetEmailBodyReq* self, GError** error)
{
    gboolean ret;
    EasGetEmailBodyReqPrivate *priv = self->priv;
    xmlDoc *doc = NULL;

    g_debug ("eas_get_email_body_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    priv->emailBodyMsg = eas_get_email_body_msg_new (priv->serverId, priv->collectionId, priv->mimeDirectory);
    doc = eas_get_email_body_msg_build_message (priv->emailBodyMsg);
	if(!doc)
	{
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
		ret = FALSE;
		goto finish;
	}
    ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                       "ItemOperations",
                                       doc,
                                       (struct _EasRequestBase *) self,
                                       error);

    g_debug ("eas_get_email_body_req_Activate--");

finish:	
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    return ret;
}

gboolean
eas_get_email_body_req_MessageComplete (EasGetEmailBodyReq* self, xmlDoc *doc, GError* error_in)
{
    gboolean ret = TRUE;
    GError *error = NULL;
    EasGetEmailBodyReqPrivate *priv = self->priv;

    g_debug ("eas_get_email_body_req_MessageComplete++");

    // if an error occurred, store it and signal client
    if (error_in)
    {
        ret = FALSE;
        error = error_in;
        goto finish;
    }

    ret = eas_get_email_body_msg_parse_response (priv->emailBodyMsg, doc, &error);

finish:
    xmlFree (doc);
    if (!ret)
    {
        g_assert (error != NULL);
        g_warning ("eas_mail_fetch_email_body - failed to get data from message");
        dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
    }
    else
    {
        g_debug ("eas_mail_fetch_email_body - return for dbus");
        dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance));
    }

    g_debug ("eas_get_email_body_req_MessageComplete--");
	//this is a one step request, so always needs cleaning up
	return TRUE;
}

