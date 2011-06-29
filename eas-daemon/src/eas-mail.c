/*
 * Filename: eas-mail.c
 */

#include <libedataserver/e-flag.h>
#include "eas-mail.h"
#include "eas-mail-stub.h"
#include "eas-folder.h"
#include "libeasmail.h"
#include "eas-sync-folder-hierarchy-req.h"
#include "eas-sync-req.h"
#include "eas-ping-req.h"
#include "eas-delete-email-req.h"
#include "eas-get-email-body-req.h"
#include "eas-send-email-req.h"
#include "eas-update-email-req.h"
#include "eas-connection-errors.h"
#include "eas-get-email-attachment-req.h"
#include "eas-move-email-req.h"

G_DEFINE_TYPE (EasMail, eas_mail, G_TYPE_OBJECT);

struct _EasMailPrivate
{
    EasConnection* connection;
};

#define EAS_MAIL_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MAIL, EasMailPrivate))


static void
eas_mail_init (EasMail *object)
{
    EasMailPrivate *priv;
    g_debug ("eas_mail_init++");
    object->priv = priv = EAS_MAIL_PRIVATE (object);

    priv->connection = NULL;

    g_debug ("eas_mail_init--");
}

static void
eas_mail_finalize (GObject *object)
{
    /* TODO: Add deinitalization code here */

    G_OBJECT_CLASS (eas_mail_parent_class)->finalize (object);
}

static void
eas_mail_class_init (EasMailClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);

    // get rid of warnings about above 2 lines
    void *temp = (void*) object_class;
    temp = (void*) parent_class;

    g_debug ("eas_mail_class_init++");

    object_class->finalize = eas_mail_finalize;
    g_debug (">>eas_mail_class_init 01");
    g_type_class_add_private (klass, sizeof (EasMailPrivate));
    g_debug (">>eas_mail_class_init 02");
    /* Binding to GLib/D-Bus" */
    dbus_g_object_type_install_info (EAS_TYPE_MAIL,
                                     &dbus_glib_eas_mail_object_info);
    g_debug ("eas_mail_class_init--");
}

EasMail* eas_mail_new (void)
{
    EasMail* easMail = NULL;
    easMail = g_object_new (EAS_TYPE_MAIL, NULL);
    return easMail;
}

#if 0
void eas_mail_set_eas_connection (EasMail* self, EasConnection* easConnObj)
{
    EasMailPrivate* priv = self->priv;
    priv->connection = easConnObj;
}
#endif

gboolean
eas_mail_start_sync (EasMail* easMailObj, gint valueIn, GError** error)
{
    /*
    guint64 account_uid = 12345;
    const gchar* sync_key =NULL;
    gchar **ret_sync_key =NULL;
    gchar **ret_created_folders_array =NULL;
    gchar **ret_updated_folders_array =NULL;
    gchar **ret_deleted_folders_array =NULL;


    eas_connection_folder_sync(easMailObj->easConnection,
                                                account_uid,
                            sync_key,
                            ret_sync_key,
                            ret_created_folders_array,
                            ret_updated_folders_array,
                            ret_deleted_folders_array,
                            error);
    */
    return TRUE;
}

void
eas_mail_test_001 (EasMail* obj, DBusGMethodInvocation* context)
{

    GError *error = NULL;
    gchar *ok_str = g_strdup ("OK");

    g_debug (">> eas_mail_test_001()");

    if (error)
    {
        g_debug (">> eas_mail_test_001 -error-");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    else
    {
        g_debug (">> eas_mail_test_001 -No error-");
        dbus_g_method_return (context, ok_str);
    }

    g_free (ok_str);
}



static gboolean 
build_serialised_idupdates_array(gchar ***serialised_idupdates_array, const GSList *idupdates_list, GError **error)
{
    gboolean ret = TRUE;
    guint i = 0;
    GSList *l = (GSList*) idupdates_list;
	guint array_len = g_slist_length (l) + 1;  // +1 to allow terminating null
	
    g_debug ("build_serialised_idupdates_array++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    g_assert (serialised_idupdates_array);
	g_assert (*serialised_idupdates_array == NULL);

    *serialised_idupdates_array = g_malloc0 (array_len * sizeof (gchar*));
    if (!serialised_idupdates_array)
    {
        ret = FALSE;
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        goto finish;
    }

    for (i = 0; i < array_len - 1; i++)
    {
        EasIdUpdate *id_update;
        g_assert (l != NULL);
        id_update = l->data;

        if (!eas_updatedid_serialise (id_update, & (*serialised_idupdates_array)[i]))
        {
            g_debug ("failed!");
            ret = FALSE;
            goto finish;
        }

        l = g_slist_next (l);		
    }

finish:
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
		g_strfreev(*serialised_idupdates_array);	
    }

	g_debug ("build_serialised_idupdates_array--");
    return ret;	

}

gboolean
eas_mail_sync_email_folder_hierarchy (EasMail* self,
                                      const gchar* account_uid,
                                      const gchar* sync_key,
                                      DBusGMethodInvocation* context)
{
    EasMailPrivate* priv = self->priv;
    GError *error = NULL;
    EasSyncFolderHierarchyReq *req = NULL;
    gboolean ret;

    g_debug ("eas_mail_sync_email_folder_hierarchy++ : account_uid[%s]",
             (account_uid ? account_uid : "NULL"));

    priv->connection = eas_connection_find (account_uid);
    if (!priv->connection)
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

    g_debug ("eas_mail_sync_email_folder_hierarchy++ 1");

    req = eas_sync_folder_hierarchy_req_new (sync_key, account_uid, context);

    g_debug ("eas_mail_sync_email_folder_hierarchy++ 2");

    eas_request_base_SetConnection (&req->parent_instance,
                                    priv->connection);

    g_debug ("eas_mail_sync_email_folder_hierarchy++ 3");

    // Activate the request
    ret = eas_sync_folder_hierarchy_req_Activate (req, &error);

    g_debug ("eas_mail_sync_email_folder_hierarchy--");
	return TRUE;
}

/**
 * Get email header information from the exchange server for the folder
 * identified by the collection_id.
 *
 * @param[in,out] self                      the instance of the GObject
 * @param[in]     account_uid               the exchange server account UID
 * @param[in]     sync_key                  the current sync_key
 * @param[in]     collection_id             identifer for the target folder
 * @param[in]     context                   dbus context
 */
gboolean
eas_mail_sync_folder_email (EasMail* self,
                            const gchar* account_uid,
                            const gchar* sync_key,
                            const gchar *collection_id,
                            DBusGMethodInvocation* context)
{
    EasMailPrivate* priv = self->priv;
    GError *error = NULL;
    EasSyncReq *req = NULL;
    gboolean ret = TRUE;

    g_debug ("eas_mail_sync_folder_email++");

    priv->connection = eas_connection_find (account_uid);
    if (!priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

    // Create Request
    req = eas_sync_req_new (sync_key,
                                 account_uid,
                                 collection_id,
                                 EAS_ITEM_MAIL,
                                 context);

    eas_request_base_SetConnection (&req->parent_instance, priv->connection);

    // Activate Request

    ret = eas_sync_req_Activate (req,                                 
                                 &error);
    

finish:

    if (!ret)
    {
        g_debug ("returning error %s", error->message);
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
 
    return ret;
}

gboolean
eas_mail_delete_email (EasMail *easMailObj,
                       const gchar* account_uid,
                       const gchar *sync_key,
                       const gchar *folder_id,
                       const gchar **server_ids_array,
                       DBusGMethodInvocation* context)
{
    gboolean ret = TRUE;
    GError *error = NULL;
    GSList *server_ids_list = NULL;
    int index = 0;
    const gchar* id = NULL;
    EasDeleteEmailReq *req = NULL;
    GSList *item = NULL;

    g_debug ("eas_mail_delete_email++");
    g_assert (server_ids_array);

    easMailObj->priv->connection = eas_connection_find (account_uid);
    if (!easMailObj->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

    // Convert server_ids_array into GSList
    while ( (id = server_ids_array[index++]))
    {
        server_ids_list = g_slist_prepend (server_ids_list, g_strdup (id));
    }

    // Create the request
    req = eas_delete_email_req_new (account_uid, sync_key, folder_id, server_ids_list, context);

    // Cleanup the gslist
    item = server_ids_list;
    for (; item; item = item->next)
    {
        g_free (item->data);
        item->data = NULL;
    }
    g_slist_free (server_ids_list);

    eas_request_base_SetConnection (&req->parent_instance,
                                    easMailObj->priv->connection);

    // Start the request
    ret = eas_delete_email_req_Activate (req, &error);

finish:
    if (!ret)
    {
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    g_debug ("eas_mail_delete_email--");
    return ret;
}

gboolean
eas_mail_update_emails (EasMail *self,
                        const gchar* account_uid,
                        const gchar *sync_key,
                        const gchar *folder_id,
                        const gchar **serialised_email_array,
                        DBusGMethodInvocation* context)
{
    GError *error = NULL;
    EasUpdateEmailReq *req = NULL;
    gboolean ret = TRUE;

    g_debug ("eas_mail_update_email++");

    self->priv->connection = eas_connection_find (account_uid);
    if (!self->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

    // Create the request
    g_debug ("create request");
    req = eas_update_email_req_new (account_uid, sync_key, folder_id, serialised_email_array, context);
    if (!req)
    {
        g_set_error (&error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        ret = FALSE;
        goto finish;
    }

    eas_request_base_SetConnection (&req->parent_instance,
                                    self->priv->connection);

    // Start the request
    g_debug ("start request");
    ret = eas_update_email_req_Activate (req, &error);

finish:
	g_object_unref (req);	
    if (!ret)
    {
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    g_debug ("eas_mail_update_email--");
    return ret;
}

/**
 * Fetches the email body identified by the server_id from the exchange server
 * in MIME format and writes it as a file named 'server_id' in the directory
 * specified by the path in 'mime_directory'.
 *
 * @param[in,out] self            the instance of the GObject
 * @param[in]     account_uid     the exchange server account UID
 * @param[in]     collection_id   folder id on the server
 * @param[in]     server_id       email id on the server - this forms the name of the mime file to be created
 * @param[in]     mime_directory  full path to directory on the client where the mime email is to be stored
 * @param[in]     context         dbus context
 */
gboolean
eas_mail_fetch_email_body (EasMail* self,
                           const gchar* account_uid,
                           const gchar* collection_id,
                           const gchar *server_id,
                           const gchar *mime_directory,
                           DBusGMethodInvocation* context)
{
    gboolean ret;
    EasMailPrivate *priv = self->priv;
    GError *error = NULL;
    EasGetEmailBodyReq *req = NULL;

    g_debug ("eas_mail_fetch_email_body++");

    priv->connection = eas_connection_find (account_uid);
    if (!priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
		goto finish;
    }

    // Create Request
    req = eas_get_email_body_req_new (account_uid,
                                      collection_id,
                                      server_id,
                                      mime_directory,
                                      context);

    eas_request_base_SetConnection (&req->parent_instance, priv->connection);

    ret = eas_get_email_body_req_Activate (req, &error);


finish:

    if (!ret)
    {
        g_assert (error != NULL);
        g_warning ("eas_mail_fetch_email_body - failed to get data from message");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }

    g_debug ("eas_mail_fetch_email_body--");
    return TRUE;
}

gboolean
eas_mail_fetch_attachment (EasMail* self,
                           const gchar* account_uid,
                           const gchar *file_reference,
                           const gchar *mime_directory,
                           DBusGMethodInvocation* context)
{
    gboolean ret;
    EasMailPrivate *priv = self->priv;
    EFlag *flag = NULL;
    GError *error = NULL;
    EasGetEmailAttachmentReq *req = NULL;

    g_debug ("eas_mail_fetch_attachment++");

    priv->connection = eas_connection_find (account_uid);
    if (!priv->connection)
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

    // Create Request
    flag = e_flag_new ();
    req = eas_get_email_attachment_req_new (account_uid,
                                            file_reference,
                                            mime_directory,
                                            context);

    eas_request_base_SetConnection (&req->parent_instance, priv->connection);

	ret = eas_get_email_attachment_req_Activate (req, &error);


 if (!ret)
    {
        g_assert (error != NULL);
        g_warning ("eas_mail_fetch_attachment - failed to get data from message");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
  
 g_debug ("eas_mail_fetch_attachment--");
    return TRUE;
}


gboolean
eas_mail_send_email (EasMail* easMailObj,
                     const gchar* account_uid,
                     const gchar* clientid,
                     const gchar *mime_file,
                     DBusGMethodInvocation* context)
{
    gboolean ret = TRUE;
    GError *error = NULL;
    EasSendEmailReq *req = NULL;

    g_debug ("eas_mail_send_email++");

    easMailObj->priv->connection = eas_connection_find (account_uid);
    if (!easMailObj->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

    // Create Request
    req = eas_send_email_req_new (account_uid, context, clientid, mime_file);

    eas_request_base_SetConnection (&req->parent_instance,
                                    easMailObj->priv->connection);

    // Activate Request
    ret = eas_send_email_req_Activate (req, &error);


finish:
    if (!ret)
    {
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    g_debug ("eas_mail_send_email--");
    return ret;
}


gboolean
eas_mail_move_emails_to_folder (EasMail* easMailObj,
                     const gchar* account_uid,
                     const gchar** server_ids_array,
                     const gchar *src_folder_id,
                     const gchar *dest_folder_id,
                     DBusGMethodInvocation* context)
{
    gboolean ret = TRUE;
    EFlag *flag = NULL;
    GError *error = NULL;
    EasMoveEmailReq *req = NULL;
	GSList *server_ids_list = NULL, *l = NULL;
    int index = 0;
    const gchar* id = NULL;
	GSList *updated_ids_list = NULL;
	gchar **ret_updated_ids_array = NULL;
		
	g_debug ("eas_mail_move_emails_to_folder++");

    easMailObj->priv->connection = eas_connection_find (account_uid);
    if (!easMailObj->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

    // Convert server_ids_array into GSList
    while ( (id = server_ids_array[index++]))
    {
        server_ids_list = g_slist_prepend (server_ids_list, g_strdup (id));
    }
	
    // Create Request
    flag = e_flag_new ();
    req = eas_move_email_req_new (account_uid, flag, server_ids_list, src_folder_id, dest_folder_id);

    // Cleanup the gslist
    l = server_ids_list;
    for (; l; l = l->next)
    {
        g_free (l->data);
        l->data = NULL;
    }
    g_slist_free (server_ids_list);
	
    eas_request_base_SetConnection (&req->parent_instance,
                                    easMailObj->priv->connection);

    // Activate Request
    ret = eas_move_email_req_Activate (req, &error);
    if (!ret)
    {
        goto finish;
    }
    // Wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    ret = eas_move_email_req_ActivateFinish (req, &error, &updated_ids_list);
	if(!ret)
	{
		goto finish;			
	}
	// convert list to array for transport over dbus:
	ret = build_serialised_idupdates_array (&ret_updated_ids_array, updated_ids_list, &error);	
	g_slist_foreach (updated_ids_list, (GFunc) g_free, NULL);
	g_slist_free(updated_ids_list);	
	
finish:
	g_object_unref (req);	
    if (!ret)
    {
		g_debug("dbus_g_method_return_error");
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    else
    {
        dbus_g_method_return (context, ret_updated_ids_array);			
    }
    g_debug ("eas_mail_move_emails_to_folder--");
    return ret;
}

gboolean 
eas_mail_watch_email_folders(EasMail* easMailObj,
                                const gchar* account_uid,
                                const gchar* heartbeat,
                                const gchar **folder_array,
                                DBusGMethodInvocation* context)
{
    gboolean ret = TRUE;
    GError *error = NULL;
    GSList *folder_ids_list = NULL;
    int index = 0;
    const gchar* id = NULL;
    EasPingReq *req = NULL;
    GSList *item = NULL;

    g_debug ("eas_mail_watch_email_folders++");
    g_assert (folder_array);

    easMailObj->priv->connection = eas_connection_find (account_uid);
    if (!easMailObj->priv->connection)
    {
        g_set_error (&error,
                     EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
                     "Failed to find account [%s]",
                     account_uid);
        ret = FALSE;
        goto finish;
    }

	g_debug ("eas_mail_watch_email_folders1");
    // Convert server_ids_array into GSList
    while ( (id = folder_array[index++]))
    {
		g_debug("Folder id = [%s]", id);
        folder_ids_list = g_slist_prepend (folder_ids_list, g_strdup (id));
    }
	g_debug ("eas_mail_watch_email_folders2");

    // Create the request
    req = eas_ping_req_new (account_uid, heartbeat, folder_ids_list, context);

	g_debug ("eas_mail_watch_email_folders3");

    // Cleanup the gslist
    item = folder_ids_list;
    for (; item; item = item->next)
    {
        g_free (item->data);
        item->data = NULL;
    }
    g_slist_free (folder_ids_list);

    eas_request_base_SetConnection (&req->parent_instance,
                                    easMailObj->priv->connection);

	g_debug ("eas_mail_watch_email_folder4");
    // Start the request
    ret = eas_ping_req_Activate (req, &error);

	g_debug ("eas_mail_watch_email_folders5");

finish:
    if (!ret)
    {
		g_debug ("eas_mail_watch_email_folders6");
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    }
    g_debug ("eas_mail_watch_email_folders--");
    return ret;
}
