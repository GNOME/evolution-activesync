/*
 * Filename: eas-mail.c
 */
 
#include <libedataserver/e-flag.h>
#include "eas-mail.h"
#include "eas-mail-stub.h"
#include "eas-folder.h"

#include "eas-sync-folder-hierarchy-req.h"
#include "eas-sync-req.h"
#include "eas-delete-email-req.h"
#include "eas-get-email-body-req.h"
#include "eas-send-email-req.h"
#include "eas-update-email-req.h"
#include "eas-connection-errors.h"

#include "eas-get-email-attachment-req.h"

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
	g_debug("eas_mail_init++");
	object->priv = priv = EAS_MAIL_PRIVATE(object);

    priv->connection = NULL;

    g_debug("eas_mail_init--");
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
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
    
 	g_debug("eas_mail_class_init++");

	object_class->finalize = eas_mail_finalize;
		g_debug(">>eas_mail_class_init 01");	
	g_type_class_add_private (klass, sizeof (EasMailPrivate));
			g_debug(">>eas_mail_class_init 02");	
	 /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_MAIL,
                                    &dbus_glib_eas_mail_object_info);
    g_debug("eas_mail_class_init--");
}

EasMail* eas_mail_new(void)
{
	EasMail* easMail = NULL;
	easMail = g_object_new(EAS_TYPE_MAIL, NULL);
	return easMail;
}

void eas_mail_set_eas_connection(EasMail* self, EasConnection* easConnObj)
{
   EasMailPrivate* priv = self->priv;
   priv->connection = easConnObj;
}

gboolean eas_mail_start_sync(EasMail* easMailObj, gint valueIn, GError** error)
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

void eas_mail_test_001(EasMail* obj, DBusGMethodInvocation* context)
{

    GError *error = NULL;
    gchar *ok_str = g_strdup ("OK");
    
 	g_debug(">> eas_mail_test_001()");

    if (error) {
        g_debug(">> eas_mail_test_001 -error-");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } else {
        g_debug(">> eas_mail_test_001 -No error-");
        dbus_g_method_return (context, ok_str);
    }

    g_free (ok_str);
}


// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
static gboolean 
build_serialised_folder_array(gchar ***serialised_folder_array, const GSList *folder_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	guint array_len = g_slist_length((GSList*)folder_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
	GSList *l = (GSList*)folder_list;

    g_assert(serialised_folder_array);
    g_assert(*serialised_folder_array == NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	*serialised_folder_array = g_malloc0(array_len * sizeof(gchar*));

	for(i = 0; i < array_len - 1; i++)
	{
		EasFolder *folder;
		g_assert(l != NULL);
		folder = l->data;

		if(!eas_folder_serialise(folder, &(*serialised_folder_array)[i]))
		{
			g_debug("failed!");
			ret = FALSE;
			goto cleanup;
		}

		l = g_slist_next (l);
	}
	
cleanup:
	if(!ret)
	{
		for(i = 0; i < array_len - 1; i++)
		{
			g_free((*serialised_folder_array)[i]);
		}
		g_free(*serialised_folder_array);
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
	}

	return ret;
}

// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
static gboolean 
build_serialised_email_info_array(gchar ***serialised_email_info_array, const GSList *email_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	guint array_len = g_slist_length((GSList*)email_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
	GSList *l = (GSList*)email_list;

    g_debug("build email arrays++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    g_assert(serialised_email_info_array);
    g_assert(*serialised_email_info_array == NULL);

	*serialised_email_info_array = g_malloc0(array_len * sizeof(gchar*));
	if(!serialised_email_info_array)
	{
		ret = FALSE;
		g_set_error (error, EAS_CONNECTION_ERROR,
					EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
					("out of memory"));
		goto finish;
	}

	for(i = 0; i < array_len - 1; i++){
		gchar *tstring = g_strdup(l->data);
		g_assert(l != NULL);
		(*serialised_email_info_array)[i]=tstring;
		l = g_slist_next (l);
	}
	
finish:
	if(!ret)
		{
			g_assert(error == NULL || *error != NULL);
		}
    
	return ret;
}


void eas_mail_sync_email_folder_hierarchy(EasMail* self,
                                          guint64 account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context)
{
    EasMailPrivate* priv = self->priv;
    GError *error = NULL;
    GSList* added_folders = NULL;
    GSList* updated_folders  = NULL;
    GSList* deleted_folders  = NULL;
    EFlag * eflag = NULL;
    EasSyncFolderHierarchyReq *req = NULL;

    gchar*  ret_sync_key = NULL;
    gchar** ret_created_folders_array = NULL;
    gchar** ret_updated_folders_array = NULL;
    gchar** ret_deleted_folders_array = NULL;
	gboolean ret;

    eflag = e_flag_new ();
    
    g_debug("eas_mail_sync_email_folder_hierarchy++");

    eas_connection_set_account(priv->connection, account_uid);

    g_debug("eas_mail_sync_email_folder_hierarchy++ 1");
    
	// Create the request
    req = eas_sync_folder_hierarchy_req_new (sync_key, account_uid, eflag);

    g_debug("eas_mail_sync_email_folder_hierarchy++ 2");
    
    eas_request_base_SetConnection (&req->parent_instance, 
                                    priv->connection);
                                    
    g_debug("eas_mail_sync_email_folder_hierarchy++ 3");

    // Activate the request
    ret = eas_sync_folder_hierarchy_req_Activate (req, &error);
	if(!ret)
	{
		goto finish;
	}
    e_flag_wait(eflag);
    e_flag_free (eflag);
    
	// Fetch the response data from the message
    ret = eas_sync_folder_hierarchy_req_ActivateFinish (req,
                                                  &ret_sync_key,
                                                  &added_folders,
                                                  &updated_folders,
                                                  &deleted_folders,
                                                  &error);
	if(!ret)
	{
		goto finish;
	}
	
    // Serialise the response data from GSList* to char** for transmission over Dbus

	ret = build_serialised_folder_array(&ret_created_folders_array, added_folders, &error);
    if(ret)
    {
		ret = build_serialised_folder_array(&ret_updated_folders_array, updated_folders, &error);
        if(ret)
        {
            ret = build_serialised_folder_array(&ret_deleted_folders_array, deleted_folders, &error);
        }
    }

	
finish:	
     // Return the error or the requested data to the mail client
    if (!ret) 
    {
		g_assert (error != NULL);
		dbus_g_method_return_error (context, error);
		g_error_free (error);
    } 
    else
    {
		dbus_g_method_return (context,
                              ret_sync_key,
                              ret_created_folders_array,
                              ret_updated_folders_array,
                              ret_deleted_folders_array);
    }

    g_debug("eas_mail_sync_email_folder_hierarchy--");
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
gboolean eas_mail_sync_folder_email(EasMail* self,
                                    guint64 account_uid,
                                    const gchar* sync_key,
                                    const gchar *collection_id,
                                    DBusGMethodInvocation* context)
{
    EasMailPrivate* priv = self->priv;
    EFlag *flag = NULL;
    GError *error = NULL;
    EasSyncReq *req = NULL;
    gchar *ret_sync_key = NULL;
    gboolean ret_more_available = FALSE;
    gchar** ret_added_email_array = NULL;
    gchar** ret_deleted_email_array = NULL;
    gchar** ret_changed_email_array = NULL;
    // TODO ActivateFinish needs to be refactored to serialise the data.
    GSList *a = NULL, *b = NULL, *c = NULL;
    gboolean ret = TRUE;

    g_debug ("eas_mail_sync_folder_email++");

    flag = e_flag_new ();

    // Set the account Id into the connection
    eas_connection_set_account(priv->connection, account_uid);

    // Create Request
    req = g_object_new(EAS_TYPE_SYNC_REQ, NULL);

    eas_request_base_SetConnection (&req->parent_instance, priv->connection);

    // Activate Request
    ret = eas_sync_req_Activate (req,
                           sync_key,
                           account_uid,
                           flag,
                           collection_id,
                           EAS_ITEM_MAIL,
                           &error);
	if(!ret)
	{
		goto finish;
	}
    // Wait for response
    e_flag_wait (flag);
    e_flag_free (flag);



    // Fetch the serialised response for transmission over DBusresponse

    // TODO ActivateFinish needs to be refactored to serialise the data.
    ret = eas_sync_req_ActivateFinish(req,
                                &ret_sync_key,
                                &a /* &ret_add_email_array     */,
                                &b /* &ret_changed_email_array */,
                                &c /* &ret_deleted_email_array */,
                                &error);
	if(!ret)
	{
		goto finish;
	}

	ret = build_serialised_email_info_array(&ret_added_email_array, a, &error);
	if(ret)
	{
		ret = build_serialised_email_info_array(&ret_changed_email_array, b, &error);
		if(ret)
		{
		    ret = build_serialised_email_info_array(&ret_deleted_email_array, c, &error);
		}
	}

finish:
    if (!ret)
    {
		g_assert (error != NULL);		
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
        dbus_g_method_return (context,
                              ret_sync_key,
                              ret_more_available,
                              ret_added_email_array,
                              ret_deleted_email_array,
                              ret_changed_email_array);
    }

	g_object_unref(req);
    g_debug("eas_mail_sync_folder_email--");
    return TRUE;
}

gboolean eas_mail_delete_email(EasMail *easMailObj,
                                guint64 account_uid,
                                const gchar *sync_key, 
                                const gchar *folder_id,
                                const gchar **server_ids_array,
                                DBusGMethodInvocation* context)
{
    EFlag *flag = NULL;
    GError *error = NULL;
    gchar* ret_sync_key = NULL;	
    GSList *server_ids_list = NULL;
    int index = 0;
    const gchar* id = NULL;
	EasDeleteEmailReq *req = NULL;
    GSList *item = NULL;
        
    g_debug("eas_mail_delete_email++");
    g_assert(server_ids_array);
    
    flag = e_flag_new ();

    if(easMailObj->priv->connection)
    {
        eas_connection_set_account(easMailObj->priv->connection, account_uid);
    }

    // Convert server_ids_array into GSList
    while ( (id = server_ids_array[index++]) )
    {
        server_ids_list = g_slist_prepend(server_ids_list, g_strdup(id));
    }

    // Create the request
	req = eas_delete_email_req_new (account_uid, sync_key, folder_id, server_ids_list, flag);

    // Cleanup the gslist
    item = server_ids_list;
    for(;item; item = item->next)
    {
        g_free(item->data);
        item->data = NULL;
    }
    g_slist_free(server_ids_list);


	eas_request_base_SetConnection (&req->parent_instance, 
                                   easMailObj->priv->connection);

    // Start the request
    eas_delete_email_req_Activate (req, &error);

	    // Set flag to wait for response
    e_flag_wait(flag);
    e_flag_free(flag);

    // TODO check error

	eas_delete_email_req_ActivateFinish(req, &ret_sync_key, &error);
		
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
	g_debug("eas_mail_delete_email--");
	return TRUE;
}

gboolean eas_mail_update_emails(EasMail *self,
                                    guint64 account_uid,
                                    const gchar *sync_key, 
                                    const gchar *folder_id,
                                    const gchar **serialised_email_array,
                                    DBusGMethodInvocation* context)
{
    EFlag *flag = NULL;
    GError *error = NULL;
	EasUpdateEmailReq *req = NULL;
    
    g_debug("eas_mail_update_email++");

    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(self->priv->connection, account_uid);
    }

    // Create the request
	g_debug("create request");
	req = eas_update_email_req_new (account_uid, sync_key, folder_id, serialised_email_array, flag);

	eas_request_base_SetConnection (&req->parent_instance, 
                                   self->priv->connection);

	// Start the request
	g_debug("start request");
    eas_update_email_req_Activate (req, &error);

	// Set flag to wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    // TODO check error

	g_debug("finish");
		
    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
        dbus_g_method_return (context);
    }	
	g_debug("eas_mail_update_email--");
	return TRUE;
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
                           guint64 account_uid,
                           const gchar* collection_id,
                           const gchar *server_id, 
                           const gchar *mime_directory, 
                           DBusGMethodInvocation* context)
{
    EasMailPrivate *priv = self->priv;
    EFlag *flag = NULL;
    GError *error = NULL;
    EasGetEmailBodyReq *req = NULL;
        
    g_debug("eas_mail_fetch_email_body++");

    flag = e_flag_new ();
    
    // Set the account Id into the connection
    eas_connection_set_account(priv->connection, account_uid);

    // Create Request
    req = eas_get_email_body_req_new (account_uid,
                                      collection_id,
                                      server_id,
                                      mime_directory,
                                      flag);

    eas_request_base_SetConnection (&req->parent_instance, priv->connection);

    eas_get_email_body_req_Activate (req, &error);

    // Wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    // TODO Check

    eas_get_email_body_req_ActivateFinish (req, &error);

    if (error)
    {
        g_warning("eas_mail_fetch_email_body - failed to get data from message");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
        g_debug("eas_mail_fetch_email_body - return for dbus");
        dbus_g_method_return (context);
    }

    g_debug("eas_mail_fetch_email_body--");
	return TRUE;
}

gboolean
eas_mail_fetch_attachment (EasMail* self, 
                            guint64 account_uid, 
                            const gchar *file_reference, 
                            const gchar *mime_directory, 
                            DBusGMethodInvocation* context)
{
    EasMailPrivate *priv = self->priv;
    EFlag *flag = NULL;
    GError *error = NULL;
    EasGetEmailAttachmentReq *req = NULL;
        
    g_debug("eas_mail_fetch_attachment++");

    flag = e_flag_new ();
    
    // Set the account Id into the connection
    eas_connection_set_account(priv->connection, account_uid);

    // Create Request
    req = eas_get_email_attachment_req_new (account_uid,
                                            file_reference,
                                            mime_directory,
                                            flag);

    eas_request_base_SetConnection (&req->parent_instance, priv->connection);

    eas_get_email_attachment_req_Activate (req, &error);

    // Wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    // TODO check error

    eas_get_email_attachment_req_ActivateFinish (req, &error);

    if (error)
    {
        g_warning("eas_mail_fetch_attachment - failed to get data from message");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
        g_debug("eas_mail_fetch_attachment - return for dbus");
        dbus_g_method_return (context);
    }

    g_debug("eas_mail_fetch_attachment--");
	return TRUE;
}
    

// 
gboolean eas_mail_send_email(EasMail* easMailObj, 
								guint64 account_uid,
								const gchar* clientid,
								const gchar *mime_file,
								DBusGMethodInvocation* context)
{
    EFlag *flag = NULL;
    GError *error = NULL;
    EasSendEmailReq *req = NULL;
    
    g_debug("eas_mail_send_email++");

    flag = e_flag_new ();

    if(easMailObj->priv->connection)
    {
        eas_connection_set_account(easMailObj->priv->connection, account_uid);
    }

    // Create Request
	req = eas_send_email_req_new(account_uid, flag, clientid, mime_file);

	g_debug("request created");
    eas_request_base_SetConnection (&req->parent_instance, 
                                    easMailObj->priv->connection);

	g_debug("connection set ");
    // Activate Request
    eas_send_email_req_Activate (req,&error);

	g_debug ("request activated");
	
    // Wait for response
    e_flag_wait (flag);

	g_debug("request completed");
	
    e_flag_free (flag);

    if (error)
    {
		g_debug("returning dbus error");
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
		g_debug("returning dbus");
        dbus_g_method_return (context);
    }	
	
	g_debug("eas_mail_send_email--");
	return TRUE;
}
