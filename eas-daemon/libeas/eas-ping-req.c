#include "eas-utils.h"
#include "eas-ping-msg.h"
#include "eas-ping-req.h"

G_DEFINE_TYPE (EasPingReq, eas_ping_req, EAS_TYPE_REQUEST_BASE);

#define EAS_PING_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PING_REQ, EasPingReqPrivate))

struct _EasPingReqPrivate
{
    EasPingMsg* ping_msg;
    gchar* account_id;
    gchar* heartbeat;
    gchar** serialised_folder_array;
};

static void
eas_ping_req_init (EasPingReq *object)
{
    /* initialization code */
    EasPingReqPrivate *priv;

    g_debug ("eas_ping_req_init++");

    object->priv = priv = EAS_PING_REQ_PRIVATE (object);

    priv->ping_msg = NULL;
    priv->account_id = NULL;
    priv->heartbeat = NULL;
    priv->serialised_folder_array = NULL;

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_PING);

    g_debug ("eas_ping_req_init++");

    return;

}

static void
eas_ping_req_finalize (GObject *object)
{
    /* deinitalization code */
    EasPingReq *req = (EasPingReq *) object;
    EasPingReqPrivate *priv = req->priv;

    g_debug ("eas_ping_req_finalize++");
    g_free (priv->account_id);
    g_free (priv->heartbeat);

    g_object_unref (priv->ping_msg);
    free_string_array (priv->serialised_folder_array);

    G_OBJECT_CLASS (eas_ping_req_parent_class)->finalize (object);

    g_debug ("eas_ping_req_finalize--");
}

static void
eas_ping_req_class_init (EasPingReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (EasPingReqPrivate));

    object_class->finalize = eas_ping_req_finalize;

    g_debug ("eas_ping_req_class_init--");
}

EasPingReq *eas_ping_req_new (const gchar* account_id, const gchar *heartbeat, const gchar **serialised_folder_array, DBusGMethodInvocation *context)
{
    EasPingReq* self = g_object_new (EAS_TYPE_PING_REQ, NULL);
    EasPingReqPrivate *priv = self->priv;
    guint i;
    guint num_serialised_folders = 0;

    g_debug ("eas_ping_req_new++");
    g_assert (heartbeat);
    g_assert (serialised_folder_array);

    num_serialised_folders = array_length (serialised_folder_array);
    priv->heartbeat = g_strdup (heartbeat);
    // TODO duplicate the string array
    priv->serialised_folder_array = g_malloc0 ( (num_serialised_folders * sizeof (gchar*)) + 1); // allow for null terminate
    if (!priv->serialised_folder_array)
    {
        goto cleanup;
    }
    for (i = 0; i < num_serialised_folders; i++)
    {
        priv->serialised_folder_array[i] = g_strdup (serialised_folder_array[i]);
    }
    priv->serialised_folder_array[i] = NULL;

    priv->account_id = g_strdup (account_id);

    eas_request_base_SetContext (&self->parent_instance, context);

cleanup:
    if (!priv->serialised_folder_array)
    {
        g_warning ("Failed to allocate memory!");
        g_free (priv->heartbeat);
        if (self)
        {
            g_object_unref (self);
            self = NULL;
        }
    }

    g_debug ("eas_ping_req_new--");
    return self;
}

gboolean
eas_ping_req_Activate (EasPingReq *self, GError** error)
{
    gboolean ret;
    EasPingReqPrivate *priv = self->priv;
    xmlDoc *doc;
    GSList *pings = NULL;   // ping msg expects a list, we have an array
    guint i = 0;

    g_debug ("eas_ping_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    while (priv->serialised_folder_array[i])
    {
        g_debug ("append email to list");
        pings = g_slist_append (pings, priv->serialised_folder_array[i]);
        i++;
    }

    //create sync msg object
    priv->ping_msg = eas_ping_msg_new ();

    g_debug ("build messsage");
    //build request msg
    doc = eas_ping_msg_build_message (priv->ping_msg, priv->account_id, priv->heartbeat, pings);
    g_slist_free (pings);
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
                                       "Ping",
                                       doc,
                                       (struct _EasRequestBase *) self,
                                       error);

finish:
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_ping_req_Activate--");
    return ret;
}


gboolean
eas_ping_req_MessageComplete (EasPingReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret = TRUE;
    GError *error = NULL;
    EasPingReqPrivate *priv = self->priv;
	gboolean finished = FALSE;

    g_debug ("eas_ping_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
		ret = FALSE;
        error = error_in;
        goto finish;
    }

    ret = eas_ping_msg_parse_response (priv->ping_msg, doc, &error);
    xmlFree (doc);
    if (!ret)
    {
        g_assert (error != NULL);
		goto finish;
	}
	//TODO: get status message - if status = 1 - re-issue message, if status = 
	//      2 signal folder IDs to client.
	

finish:
    if(!ret)
	{
        dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
    }
    else
    {
        dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance));
    }

    g_debug ("eas_ping_req_MessageComplete--");
	return finished;
}



