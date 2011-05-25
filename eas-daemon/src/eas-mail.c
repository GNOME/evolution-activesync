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

G_DEFINE_TYPE (EasMail, eas_mail, G_TYPE_OBJECT);

struct _EasMailPrivate
{
    EasConnection* connection;
};

#define EAS_MAIL_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MAIL, EasMailPrivate))


static void
eas_mail_init (EasMail *object)
{
 	g_debug("eas_mail_init++");
    EasMailPrivate *priv =NULL;
	object->_priv = priv = EAS_MAIL_PRIVATE(object);
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
 	g_debug("eas_mail_class_init++");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
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
   EasMailPrivate* priv = self->_priv;
   priv->connection = easConnObj;
}

EasConnection*  eas_mail_get_eas_connection(EasMail* self)
{
    EasMailPrivate* priv = self->_priv;
    return priv->connection;
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

 	g_debug(">> eas_mail_test_001()");
        GError *error = NULL;
        gchar *ok_str = g_strdup ("OK");
         // ...

        if (error) {
		g_debug(">> eas_mail_test_001 -error-");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
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

    g_assert(serialised_folder_array);
    g_assert(*serialised_folder_array == NULL);

	guint array_len = g_slist_length((GSList*)folder_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
    
	*serialised_folder_array = g_malloc0(array_len * sizeof(gchar*));

	GSList *l = (GSList*)folder_list;
	for(i = 0; i < array_len - 1; i++)
	{
		g_assert(l != NULL);
		EasFolder *folder;
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
		// TODO set error
	}

	return ret;
}

void eas_mail_sync_email_folder_hierarchy(EasMail* easMailObj,
                                          guint64 account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context)
{
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

    eflag = e_flag_new ();

    // Create the request
    g_debug("eas_mail_sync_email_folder_hierarchy++");
    req = eas_sync_folder_hierarchy_req_new (sync_key, account_uid, eflag);

    eas_request_base_SetConnection (&req->parent_instance, 
                                    eas_mail_get_eas_connection(easMailObj));
                                    

    // Activate the request
    eas_sync_folder_hierarchy_req_Activate (req);
    e_flag_wait(eflag);
    e_flag_free (eflag);

    // Fetch the response data from the message
    eas_sync_folder_hierarchy_req_ActivateFinish (req,
                                                  &ret_sync_key,
                                                  &added_folders,
                                                  &updated_folders,
                                                  &deleted_folders);

    // Serialise the response data from GSList* to char** for transmission over Dbus

    if(build_serialised_folder_array(&ret_created_folders_array, added_folders, &error))
    {
        if(build_serialised_folder_array(&ret_updated_folders_array, updated_folders, &error))
        {
            build_serialised_folder_array(&ret_deleted_folders_array, deleted_folders, &error);
        }
    }
     
     // Return the error or the requested data to the mail client
    if (error) 
    {
       g_debug(">> Daemon : Error ");
       dbus_g_method_return_error (context, error);
       g_error_free (error);
    } 
    else
    {
        g_debug(">> Daemon : Success-");
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
 * @param[in,out] easMailObj                the instance of the GObject
 * @param[in]     account_uid               the exchange server account UID
 * @param[in]     sync_key                  the current sync_key
 * @param[in]     collection_id             identifer for the target folder
 * @param[in]     context                   dbus context
 */
gboolean eas_mail_sync_folder_email(EasMail* easMailObj,
                                    guint64 account_uid,
                                    const gchar* sync_key,
                                    const gchar *collection_id,
                                    DBusGMethodInvocation* context)
{
    EFlag *flag = NULL;
    GError *error = NULL;

    g_debug ("eas_mail_sync_folder_email++");

    flag = e_flag_new ();

    // Create Request
    EasSyncReq *req = g_object_new(EAS_TYPE_SYNC_REQ, NULL);
    
    eas_request_base_SetConnection (&req->parent_instance, 
                                    eas_mail_get_eas_connection(easMailObj));

    // Activate Request
    eas_sync_req_Activate (req,
                           sync_key,
                           account_uid,
                           flag,
                           collection_id,
                           EAS_ITEM_MAIL);

    // Wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    gchar *ret_sync_key = NULL;
    gboolean ret_more_available = FALSE;
    gchar** ret_added_email_array = NULL;
    gchar** ret_deleted_email_array = NULL;
    gchar** ret_changed_email_array = NULL;

    // Fetch the serialised response for transmission over DBusresponse

    // TODO ActivateFinish needs to be refactored to serialise the data.
    GSList *a, *b, *c;
    a = b = c = NULL;

    eas_sync_req_ActivateFinish(req,
                                &ret_sync_key,
                                &a /* &ret_add_email_array     */,
                                &b /* &ret_changed_email_array */,
                                &c /* &ret_deleted_email_array */
                                /*, &error */);

    g_warning("TODO Serialisation to be performed for sync_folder_email");
    
    if (error)
    {
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

    g_debug("eas_mail_sync_folder_email--");
    return TRUE;
}

gboolean eas_mail_delete_email(EasMail *easMailObj,
                                    guint64 account_uid,
                                    const gchar *sync_key, 
                                    const gchar *server_id,
                                    DBusGMethodInvocation* context)
{
    g_debug("eas_mail_delete_email++");
    EFlag *flag = NULL;
    GError *error = NULL;
    gchar* ret_sync_key = NULL;
	 
    flag = e_flag_new ();

	EasDeleteEmailReq *req = NULL;
	req = g_object_new(EAS_TYPE_DELETE_EMAIL_REQ, NULL);


	eas_request_base_SetConnection (&req->parent_instance, 
                                   eas_mail_get_eas_connection(easMailObj));
                                        


	    // Start the request
    eas_delete_email_req_Activate (req, 
                                    sync_key,
                                    server_id,
                                    flag);

	    // Set flag to wait for response
//    e_flag_wait(flag);

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

gboolean
eas_mail_fetch (EasMail* easMailObj, 
                guint64 account_uid, 
                const gchar *server_id, 
                const gchar *collection_id, 
                const gchar *file_reference, 
                const gchar *mime_directory, 
                GError **error)
{
	// TODO
    g_debug("eas_mail_fetch++");
    g_debug("eas_mail_fetch--");
	return TRUE;
}

// 
gboolean eas_mail_send_email(EasMail* easMailObj, 
								guint64 account_uid,                             
								const gchar* clientid,
								const gchar *mime_file,
								GError** error)
{
	g_debug("eas_mail_send_email++");
	
	// TODO

	g_debug("eas_mail_send_email--");
	return TRUE;								
}


