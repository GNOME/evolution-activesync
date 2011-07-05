/*
 * ActiveSync DBus dæmon
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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
#include "activesyncd-common-defs.h"
#include "eas-marshal.h"

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

	// lrm
	// create the progress signal we emit 
	klass->signal_id = g_signal_new ( EAS_MAIL_SIGNAL_PROGRESS,				// name of the signal
	G_OBJECT_CLASS_TYPE ( klass ),  										// type this signal pertains to
	G_SIGNAL_RUN_LAST,														// flags used to specify a signal's behaviour
	0,																		// class offset
	NULL,																	// accumulator
	NULL,																	// user data for accumulator
    eas_marshal_VOID__UINT_UINT,
	//g_cclosure_marshal_VOID__UINT,   // Function to marshal the signal data into the parameters of the signal call
	G_TYPE_NONE,															// handler return type
	2,																		// Number of parameter GTypes to follow
	// GTypes of the parameters
	G_TYPE_UINT,
	G_TYPE_UINT);
	
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
                           guint request_id,			// lrm passed back with progress signal
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

	eas_request_base_SetInterfaceObject (&req->parent_instance, self);		// lrm this should be stored elsewhere?

	eas_request_base_SetRequestId (&req->parent_instance, request_id);

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
                           guint request_id,
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

	eas_request_base_SetInterfaceObject (&req->parent_instance, self);		// lrm: should be stored elsewhere?

	eas_request_base_SetRequestId (&req->parent_instance, request_id);
	
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
    GError *error = NULL;
    EasMoveEmailReq *req = NULL;
	GSList *server_ids_list = NULL, *l = NULL;
    int index = 0;
    const gchar* id = NULL;
		
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
    req = eas_move_email_req_new (account_uid, server_ids_list, src_folder_id, dest_folder_id, context);

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

	
finish:
    if (!ret)
    {
		g_debug("dbus_g_method_return_error");
        g_assert (error != NULL);
        dbus_g_method_return_error (context, error);
        g_error_free (error);
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
