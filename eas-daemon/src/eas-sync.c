/*
 *  Filename: eas-sync.c
 */

#include "eas-sync.h"
#include "eas-sync-stub.h"
#include "eas-sync-req.h"
#include "eas-delete-email-req.h"
#include "eas-update-calendar-req.h"
#include "eas-add-calendar-req.h"
#include "../../libeassync/src/eas-item-info.h"

#include "../libeas/eas-connection.h"
#include "eas-mail.h"

G_DEFINE_TYPE (EasSync, eas_sync, G_TYPE_OBJECT);

#define EAS_SYNC_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC, EasSyncPrivate))


struct _EasSyncPrivate
{
    EasConnection* connection;
};

static void
eas_sync_init (EasSync *object)
{
    EasSyncPrivate *priv = NULL;
    g_debug ("++ eas_sync_init()");
    object->priv = priv = EAS_SYNC_PRIVATE (object);
    priv->connection = NULL;
    g_debug ("-- eas_sync_init()");
}

static void
eas_sync_finalize (GObject *object)
{
    /* TODO: Add deinitalization code here */

    G_OBJECT_CLASS (eas_sync_parent_class)->finalize (object);
}

static void
eas_sync_class_init (EasSyncClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);

    (void) parent_class; // remove warning

    object_class->finalize = eas_sync_finalize;

    g_type_class_add_private (klass, sizeof (EasSyncPrivate));

    /* Binding to GLib/D-Bus" */
    dbus_g_object_type_install_info (EAS_TYPE_SYNC,
                                     &dbus_glib_eas_sync_object_info);
}

EasSync* eas_sync_new (void)
{
    EasSync* easCal = NULL;
    easCal = g_object_new (EAS_TYPE_SYNC, NULL);
    return easCal;
}

#if 0
void eas_sync_set_eas_connection (EasSync* self, EasConnection* easConnObj)
{
    EasSyncPrivate* priv = self->priv;
    priv->connection = easConnObj;
}
#endif


EasConnection*
eas_sync_get_eas_connection (EasSync* self)
{
    EasSyncPrivate* priv = self->priv;
    g_debug ("eas_sync_get_eas_connection++");
    return priv->connection;
    g_debug ("eas_sync_get_leas_connection--");
}

// takes an NULL terminated array of serialised calendar items and creates a list of EasCalInfo objects
static gboolean
build_calendar_list (const gchar **serialised_cal_array, GSList **cal_list, GError **error)
{
    gboolean ret = TRUE;
    guint i = 0;

    g_assert (cal_list);

    g_assert (g_slist_length (*cal_list) == 0);

    while (serialised_cal_array[i])
    {
        EasItemInfo *calInfo = eas_item_info_new();
        if (calInfo)
        {
            *cal_list = g_slist_append (*cal_list, calInfo); // add it to the list first to aid cleanup
            if (!cal_list)
            {
                g_free (calInfo);
                ret = FALSE;
                goto cleanup;
            }
            if (!eas_item_info_deserialise (calInfo, serialised_cal_array[i]))
            {
                ret = FALSE;
                goto cleanup;
            }
        }
        else
        {
            ret = FALSE;
            goto cleanup;
        }
        i++;
    }

cleanup:
    if (!ret)
    {
        // set the error
        //g_set_error (error, EAS_MAIL_ERROR,
        //       EAS_MAIL_ERROR_NOTENOUGHMEMORY,
        //       ("out of memory"));
        // clean up on error
        g_slist_foreach (*cal_list, (GFunc) g_free, NULL);
        g_slist_free (*cal_list);
    }

    g_debug ("list has %d items", g_slist_length (*cal_list));
    return ret;
}


void
eas_sync_get_latest_items (EasSync* self,
                           const gchar* account_uid,
                           guint64 type,
                           const gchar* folder_id,
                           const gchar* sync_key,
                           DBusGMethodInvocation* context)
{
    GError *error = NULL;
    // Create the request
    EasSyncReq *syncReqObj = NULL;

    g_debug ("eas_sync_get_latest_items++");

    self->priv->connection = eas_connection_find (account_uid);
    if (!self->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return;
    }


    g_debug ("eas_sync_get_latest_calendar_items++");
    syncReqObj = g_object_new (EAS_TYPE_SYNC_REQ , NULL);

    eas_request_base_SetConnection (&syncReqObj->parent_instance,
                                    self->priv->connection);

    g_debug ("eas_sync_get_latest_items - new req");

    eas_sync_req_Activate (syncReqObj, sync_key, account_uid, context, folder_id, type, &error);

    g_debug ("eas_sync_get_latest_items  - activate req");


    // Return the error or the requested data to the calendar client
    if (error)
    {
        g_debug (">> Daemon : Error ");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }

    g_debug ("eas_sync_get_latest_items--");
}

gboolean
eas_sync_delete_items (EasSync* self,
                       const gchar* account_uid,
                       const gchar* folder_id,
                       const gchar* sync_key,
                       const GSList *deleted_items_array,
                       DBusGMethodInvocation* context)
{
    EFlag *flag = NULL;
    GError *error = NULL;
    gchar* ret_sync_key = NULL;
    EasDeleteEmailReq *req = NULL;

    g_debug ("eas_sync_delete_items++");

    self->priv->connection = eas_connection_find (account_uid);
    if (!self->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return FALSE;
    }

    // Create the request
    flag = e_flag_new ();
    req = eas_delete_email_req_new (account_uid, sync_key, folder_id, deleted_items_array, flag);

    eas_request_base_SetConnection (&req->parent_instance,
                                    self->priv->connection);

    // Start the request
    eas_delete_email_req_Activate (req, &error);

    // Set flag to wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    // TODO Check error

    eas_delete_email_req_ActivateFinish (req, &ret_sync_key, &error);

    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    else
    {
        //TODO: make sure this stuff is ok to go over dbus.

        dbus_g_method_return (context,
                              ret_sync_key);
    }
    g_debug ("eas_sync_delete_items--");
    return TRUE;
}

gboolean
eas_sync_update_items (EasSync* self,
                       const gchar* account_uid,
                       guint64 type,
                       const gchar* folder_id,
                       const gchar* sync_key,
                       const gchar **calendar_items,
                       DBusGMethodInvocation* context)
{
    GError* error = NULL;
    EFlag *flag = NULL;
    gchar* ret_sync_key = NULL;
    GSList *items = NULL;
    EasUpdateCalendarReq *req = NULL;

    g_debug ("eas_sync_update_calendar_items++");

    self->priv->connection = eas_connection_find (account_uid);
    if (!self->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return FALSE;
    }

    flag = e_flag_new ();

    switch (type)
    {
        case EAS_ITEM_CALENDAR:
        case EAS_ITEM_CONTACT:
        {
            build_calendar_list (calendar_items, &items, &error);
        }
        break;
        default:
        {
            //TODO: put unknown type error here.
        }
    }
    // Create the request
    req = eas_update_calendar_req_new (account_uid, sync_key, type, folder_id, items, flag);

    eas_request_base_SetConnection (&req->parent_instance,
                                    eas_sync_get_eas_connection (self));

    // Start the request
    eas_update_calendar_req_Activate (req, &error);

    // TODO Check error

    // Set flag to wait for response
    e_flag_wait (flag);

    eas_update_calendar_req_ActivateFinish (req, &ret_sync_key, &error);

    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return FALSE;
    }

    switch (type)
    {
        case EAS_ITEM_CALENDAR:
        case EAS_ITEM_CONTACT:
        {
            build_calendar_list (calendar_items, &items, &error);
        }
        break;
        default:
        {
            //TODO: put unknown type error here.
        }
    }
    // Create the request
    flag = e_flag_new ();
    req = eas_update_calendar_req_new (account_uid, sync_key, type, folder_id, items, flag);

    eas_request_base_SetConnection (&req->parent_instance,
                                    eas_sync_get_eas_connection (self));

    // Start the request
    eas_update_calendar_req_Activate (req, &error);

    // TODO Check error

    // Set flag to wait for response
    e_flag_wait (flag);

    eas_update_calendar_req_ActivateFinish (req, &ret_sync_key, &error);

    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    else
    {
        dbus_g_method_return (context,
                              ret_sync_key);
    }
    g_debug ("eas_sync_update_items--");
    return TRUE;
}

gboolean
eas_sync_add_items (EasSync* self,
                    const gchar* account_uid,
                    guint64 type,
                    const gchar* folder_id,
                    const gchar* sync_key,
                    const gchar **calendar_items,
                    DBusGMethodInvocation* context)
{
    GError* error = NULL;
    GSList *items = NULL;
    EasAddCalendarReq *req = NULL;

    g_debug ("eas_sync_add_items++");

    self->priv->connection = eas_connection_find (account_uid);
    if (!self->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return FALSE;
    }

    switch (type)
    {
        case EAS_ITEM_CALENDAR:
        case EAS_ITEM_CONTACT:
        {
            build_calendar_list (calendar_items, &items, &error);
        }
        break;
        default:
        {
            //TODO: put unknown type error here.
        }
        break;
    }

    // Create the request
    req = eas_add_calendar_req_new (account_uid, sync_key, folder_id, items, context);

    eas_request_base_SetConnection (&req->parent_instance,
                                    eas_sync_get_eas_connection (self));

    // Start the request
    eas_add_calendar_req_Activate (req, &error);

    // TODO Check for error
    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }

    g_debug ("eas_sync_add_items--");
    return TRUE;
}


