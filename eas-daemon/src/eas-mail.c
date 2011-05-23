/*
 * Filename: eas-mail.c
 */
 
#include <libedataserver/e-flag.h>
#include "eas-mail.h"
#include "eas-mail-stub.h"
#include "eas-folder.h"
#include "eas-sync-folder-hierarchy.h"


G_DEFINE_TYPE (EasMail, eas_mail, G_TYPE_OBJECT);

static void
eas_mail_init (EasMail *object)
{
	/* TODO: Add initialization code here */
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

	object_class->finalize = eas_mail_finalize;
	
	 /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_MAIL,
                                            &dbus_glib_eas_mail_object_info);
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

 	g_print(">> eas_mail_test_001()\n");
        GError *error = NULL;
        gchar *ok_str = g_strdup ("OK");
         // ...

        if (error) {
		g_print(">> eas_mail_test_001 -error-\n");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
		g_print(">> eas_mail_test_001 -No error-\n");
                dbus_g_method_return (context, ok_str);
	}

        g_free (ok_str);
}

gboolean eas_mail_set_eas_connection(EasMail* easMailObj, EasConnection* easConnObj)
{
  gboolean ret= FALSE;

  if(easConnObj != NULL){
  	easMailObj->easConnection = easConnObj;
	ret= TRUE;
  }
  return ret;
}

// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
static gboolean 
build_serialised_folder_array(gchar ***serialised_folder_array, const GSList *folder_list, GError **error)
{ 
	gboolean ret = TRUE;
	g_assert(*serialised_folder_array == NULL);
	//g_assert(folder_list != NULL);

	guint array_len = g_slist_length((GSList*)folder_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
	*serialised_folder_array = g_malloc0(array_len * sizeof(gchar*));
	GSList *l = (GSList*)folder_list;
	guint i = 0;
	for(i = 0; i < array_len - 1; i++)
	{
		g_assert(l != NULL);
		gchar *serialised;
		EasFolder *folder;
		folder = l->data;
		if(!eas_folder_serialise(folder, &serialised))
		{
			ret = FALSE;
			goto cleanup;
		}
		*serialised_folder_array[i] = serialised;		
		l = g_slist_next (l);		
	}
	*serialised_folder_array[i] = NULL;
	
cleanup:
	if(!ret)
	{
		for(i = 0; i < array_len - 1; i++)
		{
			g_free(serialised_folder_array[i]);
		}
		g_free(serialised_folder_array);
		// TODO cleanup strings and array and set error
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

        gchar* ret_sync_key = NULL;
        gchar** ret_created_folders_array = NULL;
        gchar** ret_updated_folders_array = NULL;
        gchar** ret_deleted_folders_array = NULL;

        EFlag * eflag =NULL;
        
        eflag = e_flag_new ();
	  
        // Create the request
        EasSyncFolderHierarchy *folderHierarchyObj =NULL;

        g_print("eas_mail_sync_email_folder_hierarchy++\n");
        folderHierarchyObj = g_object_new(EAS_TYPE_SYNC_FOLDER_HIERARCHY , NULL);

        eas_request_base_SetConnection (&folderHierarchyObj->parent_instance, 
                                        easMailObj->easConnection);

        g_print("eas_mail_sync_email_folder_hierarchy - new req\n");
	    // Start the request
        eas_sync_folder_hierarchy_Activate (folderHierarchyObj, 
                                            sync_key,
                                            account_uid,
                                            eflag);
        g_print("eas_mail_sync_email_folder_hierarchy - activate req\n");
	    // Set flag to wait for response
	    e_flag_wait(eflag);

        g_print("eas_mail_sync_email_folder_hierarchy - get results\n");
         eas_sync_folder_hierarchy_Activate_Finish (folderHierarchyObj,
                                                    &ret_sync_key,
                                                    &added_folders,
                                                    &updated_folders,
                                                    &deleted_folders);
         e_flag_free (eflag);
         g_print("eas_mail_sync_email_folder_hierarchy - serialise objects\n");
         //serialise the folder objects from GSList* to char** and populate  :

		if(build_serialised_folder_array(&ret_created_folders_array, added_folders, &error))
		{
			if(build_serialised_folder_array(&ret_updated_folders_array, updated_folders, &error))
			{
				build_serialised_folder_array(&ret_deleted_folders_array, deleted_folders, &error);
			}
		}
         
         // Return the error or the requested data to the mail client
        if (error) {
		        g_print(">> Daemon : Error \n");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
		        g_print(">> Daemon : Success-\n");
                dbus_g_method_return (context,
                                 	ret_sync_key,
                                  	ret_created_folders_array,
						            ret_updated_folders_array,
						            ret_deleted_folders_array);
        }

    g_print("eas_mail_sync_email_folder_hierarchy--\n");

}


gboolean eas_mail_sync_folder_email(EasMail* easMailObj, 
                                    guint64 account_uid,
									const gchar* sync_key,   
                                    gboolean get_server_changes,
									const gchar *collection_id,	//folder to sync
									const gchar* deleted_email_array,
									const gchar* changed_email_array,                                    
									gchar *ret_sync_key,                                    
									gboolean *ret_more_available,
									gchar **ret_added_email_array,
									gchar **ret_deleted_email_array,
									gchar **ret_changed_email_array,	
									GError** error)
{
	// TODO
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
	return TRUE;
}

// TODO - finalise this API
gboolean eas_mail_send_email(EasMail* easMailObj, 
								guint64 account_uid,                             
								const gchar* clientid,
								const gchar* accountid,
								gboolean save_in_sent_items,
								const gchar *mime,
								GError** error)
{
  return TRUE;								
}


