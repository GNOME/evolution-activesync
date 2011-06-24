/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 *
 */

#include "eas-sync-req.h"
#include "eas-sync-msg.h"
#include "eas-connection-errors.h"
#include "serialise_utils.h"

G_DEFINE_TYPE (EasSyncReq, eas_sync_req, EAS_TYPE_REQUEST_BASE);

#define EAS_SYNC_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_REQ, EasSyncReqPrivate))

typedef enum
{
    EasSyncReqStep1 = 0,
    EasSyncReqStep2
} EasSyncReqState;


struct _EasSyncReqPrivate
{
    EasSyncMsg* syncMsg;
    EasSyncReqState state;
    gchar* accountID;
    gchar* folderID;
    EasItemType ItemType;
    GError *error;
};



static void
eas_sync_req_init (EasSyncReq *object)
{
    EasSyncReqPrivate *priv;

    object->priv = priv = EAS_SYNC_REQ_PRIVATE (object);

    g_debug ("eas_sync_req_init++");
    priv->syncMsg = NULL;
    priv->state = EasSyncReqStep1;
    priv->accountID = NULL;
    priv->folderID = NULL;
    priv->error = NULL;
    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_SYNC);

    g_debug ("eas_sync_req_init--");
}

static void
eas_sync_req_finalize (GObject *object)
{
    EasSyncReq *req = (EasSyncReq*) object;
    EasSyncReqPrivate *priv = req->priv;

    g_debug ("eas_sync_req_finalize++");

    if (priv->syncMsg)
    {
        g_object_unref (priv->syncMsg);
    }
    if (priv->error)
    {
        g_error_free (priv->error);
    }

    g_free (priv->accountID);
    g_free (priv->folderID);

    G_OBJECT_CLASS (eas_sync_req_parent_class)->finalize (object);

    g_debug ("eas_sync_req_finalize--");

}

static void
eas_sync_req_class_init (EasSyncReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);
    void *tmp = parent_class;
    tmp = object_class;

    g_type_class_add_private (klass, sizeof (EasSyncReqPrivate));

    object_class->finalize = eas_sync_req_finalize;
}


gboolean
eas_sync_req_Activate (EasSyncReq *self, const gchar* syncKey, const gchar* accountID, DBusGMethodInvocation *context, const gchar* folderId, EasItemType type, GError** error)
{
    gboolean ret = FALSE;
    EasSyncReqPrivate* priv;
    xmlDoc *doc;
    gboolean getChanges = FALSE;

    g_debug ("eas_sync_req_activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    priv = self->priv;

    eas_request_base_SetContext (&self->parent_instance, context);

    priv->accountID = g_strdup (accountID);

    priv->ItemType = type;

    priv->folderID = g_strdup (folderId);

    g_debug ("eas_sync_req_activate - new Sync  mesg");
    //create sync  msg type
    priv->syncMsg = eas_sync_msg_new (syncKey, accountID, folderId, type);
    if (!priv->syncMsg)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        goto finish;
    }

    g_debug ("eas_sync_req_activate- syncKey = %s", syncKey);

    //if syncKey is not 0, then we are not doing a first time sync and only need to send one message
    // so we  move state machine forward
    if (g_strcmp0 (syncKey, "0"))
    {
        g_debug ("switching state");
        priv->state = EasSyncReqStep2;
        getChanges = TRUE;
    }

    g_debug ("eas_sync_req_activate - build messsage");
    //build request msg
    doc = eas_sync_msg_build_message (priv->syncMsg, getChanges, NULL, NULL, NULL);
    if (!doc)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        g_free (priv->syncMsg);
        priv->syncMsg = NULL;
        goto finish;
    }

    g_debug ("eas_sync_req_activate - send message");
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
    g_debug ("eas_sync_req_activate--");

    return ret;
}

gboolean
eas_sync_req_MessageComplete (EasSyncReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean cleanup = FALSE;
    gboolean ret = TRUE;
    GError *error = NULL;
    EasSyncReqPrivate* priv = self->priv;
	gchar *ret_sync_key = NULL;
    gboolean ret_more_available = FALSE;
    gchar** ret_added_item_array = NULL;
    gchar** ret_deleted_item_array = NULL;
    gchar** ret_changed_item_array = NULL;

	GSList* added_items = NULL;
    GSList* updated_items = NULL;
    GSList* deleted_items = NULL;

    g_debug ("eas_sync_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
    	ret = FALSE;
        priv->error = error_in;
        goto finish;
    }

    ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
    xmlFree (doc);
    if (!ret)
    {
        g_assert (error != NULL);
        priv->error = error;
        goto finish;
    }

    switch (priv->state)
    {
        default:
        {
            g_assert (0);
        }
        break;

        //We have started a first time sync, and need to get the sync Key from the result, and then do the proper sync
        case EasSyncReqStep1:
        {
            //get syncKey
            gchar* syncKey = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));

            g_debug ("eas_sync_req synckey = %s", syncKey);

            //clean up old message
            if (priv->syncMsg)
            {
                g_object_unref (priv->syncMsg);
            }

            //create new message with new syncKey
            priv->syncMsg = eas_sync_msg_new (syncKey, priv->accountID, priv->folderID, priv->ItemType);
			g_free(syncKey);
            //build request msg
            doc = eas_sync_msg_build_message (priv->syncMsg, TRUE, NULL, NULL, NULL);
			if (!doc)
			{
				g_set_error (&priv->error, EAS_CONNECTION_ERROR,
				             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				             ("out of memory"));
				ret = FALSE;
				goto finish;
			}
            //move to new state
            priv->state = EasSyncReqStep2;

            ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                               "Sync",
                                               doc, (struct _EasRequestBase *) self,
                                               &error);
            if (!ret)
            {
                g_assert (error != NULL);
                priv->error = error;
                goto finish;
            }

        }
        break;

        //we did a proper sync, so we need to inform the daemon that we have finished, so that it can continue and get the data
        case EasSyncReqStep2:
        {
			//finished state machine - req needs to be cleanup up by connection object
			cleanup = TRUE;
            g_debug ("eas_sync_req_MessageComplete step 2");
			ret_more_available = eas_sync_msg_get_more_available(priv->syncMsg);
			ret_sync_key  = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));
			added_items   = eas_sync_msg_get_added_items (priv->syncMsg);
			updated_items = eas_sync_msg_get_updated_items (priv->syncMsg);
			deleted_items = eas_sync_msg_get_deleted_items (priv->syncMsg);

			switch(priv->ItemType)
			{
				case EAS_ITEM_MAIL:
				{
					ret = build_serialised_email_info_array (&ret_added_item_array, added_items, &error);
					if (ret)
					{
						ret = build_serialised_email_info_array (&ret_changed_item_array, updated_items, &error);
						if (ret)
						{
							ret = build_serialised_email_info_array (&ret_deleted_item_array, deleted_items, &error);
						}
					}
					 if (!ret)
					{
						g_set_error (&priv->error, EAS_CONNECTION_ERROR,
				             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				             ("out of memory"));
						goto finish;
					}
				}
				break;
				case EAS_ITEM_CALENDAR:
				case EAS_ITEM_CONTACT:
				{
					if (build_serialised_calendar_info_array (&ret_added_item_array, added_items, &error))
					{
						if (build_serialised_calendar_info_array (&ret_changed_item_array, updated_items, &error))
						{
							build_serialised_calendar_info_array (&ret_deleted_item_array, deleted_items, &error);
						}
					}
				}
				break;
				default:
				{
					//TODO: put some error in here for unknown type
				}
			}
			dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance),
                              ret_sync_key,
                              ret_more_available,
                              ret_added_item_array,
                              ret_deleted_item_array,
                              ret_changed_item_array);

        }
        break;
    }

finish:
	if (!ret)
    {
        g_debug ("returning error %s", error->message);
        g_assert (error != NULL);
        dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
		cleanup = TRUE;
    }
	g_free(ret_sync_key);
	g_strfreev(ret_added_item_array);
	g_strfreev(ret_deleted_item_array);
	g_strfreev(ret_changed_item_array);
    g_debug ("eas_sync_req_MessageComplete--");

	return cleanup;
}
