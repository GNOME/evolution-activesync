/*
 *  Filename: libeasmail.c
 *
 */

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlsave.h>
#include <libedataserver/e-flag.h>

#include "../../eas-daemon/src/activesyncd-common-defs.h"
#include "libeasmail.h"
#include "eas-mail-client-stub.h"
#include "eas-folder.h"
#include "eas-mail-errors.h"

#include "../../logger/eas-logger.h"

G_DEFINE_TYPE (EasEmailHandler, eas_mail_handler, G_TYPE_OBJECT);

struct _EasEmailHandlerPrivate{
	DBusGConnection *bus;
    DBusGProxy *remoteEas;
    gchar* account_uid;		// TODO - is it appropriate to have a dbus proxy per account if we have multiple accounts making requests at same time?
	GMainLoop* main_loop;
};

// TODO - how much verification of args should happen 

static void
eas_mail_handler_init (EasEmailHandler *cnc)
{
    EasEmailHandlerPrivate *priv;
	g_debug("eas_mail_handler_init++");
    
	/* allocate internal structure */
	priv = g_new0 (EasEmailHandlerPrivate, 1);
	
	priv->remoteEas = NULL;
	priv->bus = NULL;
	priv->account_uid = NULL;
	priv->main_loop = NULL;	
	cnc->priv = priv;	
	g_debug("eas_mail_handler_init--");
}

static void
eas_mail_handler_finalize (GObject *object)
{
	EasEmailHandler *cnc = (EasEmailHandler *) object;
	EasEmailHandlerPrivate *priv;
	g_debug("eas_mail_handler_finalize++");

	priv = cnc->priv;
    g_free(priv->account_uid);

	g_main_loop_quit(priv->main_loop);	
	dbus_g_connection_unref(priv->bus);
	// nothing to do to 'free' proxy
	g_free (priv);
	cnc->priv = NULL;

	G_OBJECT_CLASS (eas_mail_handler_parent_class)->finalize (object);
	g_debug("eas_mail_handler_finalize--");
}

static void
eas_mail_handler_class_init (EasEmailHandlerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	// get rid of warnings about above 2 lines
    void *temp = (void*)object_class;
	temp = (void*)parent_class;
    
    g_debug("eas_mail_handler_class_init++");

	object_class->finalize = eas_mail_handler_finalize;
	g_debug("eas_mail_handler_class_init--");
}

EasEmailHandler *
eas_mail_handler_new(const char* account_uid)
{
	GError* error = NULL;
	EasEmailHandler *object = NULL;

	g_type_init();
    g_log_set_default_handler(eas_logger, NULL);

    g_debug("eas_mail_handler_new++ : account_uid[%s]", (account_uid?account_uid:"NULL"));
    
    // TODO This needs to take a GError ** argument, not use g_error().
    if (!account_uid) return NULL;

	object = g_object_new (EAS_TYPE_EMAIL_HANDLER , NULL);

	if(object == NULL){
		g_error("Error: Couldn't create mail");
		g_debug("eas_mail_handler_new--");
		return NULL;  
	}

	object->priv->main_loop = g_main_loop_new(NULL, TRUE);

	if (object->priv->main_loop == NULL) {
		g_error("Error: Failed to create the mainloop");
		return NULL;
	}

	g_debug("Connecting to Session D-Bus.");
	object->priv->bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Error: Couldn't connect to the Session bus (%s) ", error->message);
		return NULL;
	}

	g_debug("Creating a GLib proxy object for Eas.");
	object->priv->remoteEas =  dbus_g_proxy_new_for_name(object->priv->bus,
		      EAS_SERVICE_NAME,
		      EAS_SERVICE_MAIL_OBJECT_PATH,
		      EAS_SERVICE_MAIL_INTERFACE);
	if (object->priv->remoteEas == NULL) {
		g_error("Error: Couldn't create the proxy object");
		return NULL;
	}

	object->priv->account_uid = g_strdup(account_uid);

	g_debug("eas_mail_handler_new--");
	return object;

}

// takes an NULL terminated array of serialised folders and creates a list of EasFolder objects
static gboolean 
build_folder_list(const gchar **serialised_folder_array, GSList **folder_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
    
    g_debug("build_folder_list++");
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    g_assert(folder_list);
	g_assert(g_slist_length(*folder_list) == 0);
	
	while(serialised_folder_array[i])
	{
		EasFolder *folder = eas_folder_new();
		if(folder)
		{
			*folder_list = g_slist_append(*folder_list, folder);	// add it to the list first to aid cleanup
			if(!folder_list)
			{
				g_free(folder);
				ret = FALSE;
				goto cleanup;
			}				
			if(!eas_folder_deserialise(folder, serialised_folder_array[i]))
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
	if(!ret)
	{
		// set the error
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		// clean up on error
		g_slist_foreach(*folder_list,(GFunc)g_free, NULL);
		g_slist_free(*folder_list);
	}
	
	g_debug("list has %d items", g_slist_length(*folder_list));
	g_debug("build_folder_list++");
	return ret;
}


// takes an NULL terminated array of serialised emailinfos and creates a list of EasEmailInfo objects
static gboolean 
build_emailinfo_list(const gchar **serialised_emailinfo_array, GSList **emailinfo_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	g_debug("build_emailinfo_list++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
	g_assert(g_slist_length(*emailinfo_list) == 0);
	
	while(serialised_emailinfo_array[i])
	{
		EasEmailInfo *emailinfo = eas_email_info_new ();
		if(emailinfo)
		{
			*emailinfo_list = g_slist_append(*emailinfo_list, emailinfo);
			if(!*emailinfo_list)
			{
				g_free(emailinfo);
				ret = FALSE;
				goto cleanup;
			}			
			if(!eas_email_info_deserialise(emailinfo, serialised_emailinfo_array[i]))
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
	if(!ret)
	{
		// set the error
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));		
		// clean up on error
		g_slist_foreach(*emailinfo_list,(GFunc)g_free, NULL);
		g_slist_free(*emailinfo_list);
		*emailinfo_list = NULL;
	}

	g_debug("build_emailinfo_list++");	
	return ret;
}
				

// 
static void 
free_string_array(gchar **array)
{
	guint i = 0;
    
	if(array == NULL)
		return;
	
	while(array[i])
	{	
		g_free(array[i]);
		i++;
	}
	g_free(array);

}

// pulls down changes in folder structure (folders added/deleted/updated). Supplies lists of EasFolders
gboolean 
eas_mail_handler_sync_folder_hierarchy(EasEmailHandler* self, 
                                       gchar *sync_key, 
                                       GSList **folders_created,
                                       GSList **folders_updated,
                                       GSList **folders_deleted,
                                       GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas; 
	gchar **created_folder_array = NULL;
	gchar **deleted_folder_array = NULL;
	gchar **updated_folder_array = NULL;
	gchar *updatedSyncKey = NULL;

	g_debug("eas_mail_handler_sync_folder_hierarchy++ : account_uid[%s]", 
            (self->priv->account_uid?self->priv->account_uid:"NULL"));

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_assert(self);
	g_assert(sync_key);
	g_assert(g_slist_length(*folders_created) == 0);
	g_assert(g_slist_length(*folders_updated) == 0);
	g_assert(g_slist_length(*folders_deleted) == 0);
	
	// call DBus API
	ret = dbus_g_proxy_call(proxy, "sync_email_folder_hierarchy",
		          error,
				  G_TYPE_STRING, self->priv->account_uid,
		          G_TYPE_STRING, sync_key,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
		          G_TYPE_STRV, &created_folder_array,
		          G_TYPE_STRV, &deleted_folder_array,
		          G_TYPE_STRV, &updated_folder_array,
		          G_TYPE_INVALID);

    g_debug("eas_mail_handler_sync_folder_hierarchy - dbus proxy called");
	
	if(!ret)
	{
        if (error && *error)
        {
            g_warning("[%s][%d][%s]", 
                      g_quark_to_string((*error)->domain),
                      (*error)->code, 
                      (*error)->message);
        }
            g_warning ("DBus dbus_g_proxy_call failed");
		goto cleanup;
	}

	g_debug("sync_email_folder_hierarchy called successfully");

	// put the updated sync key back into the original string for tracking this
    strcpy (sync_key, updatedSyncKey);
	
	// get 3 arrays of strings of 'serialised' EasFolders, convert to EasFolder lists:
	ret = build_folder_list((const gchar **)created_folder_array, folders_created, error);
	if(!ret)
	{
		goto cleanup;		
	}
	ret = build_folder_list((const gchar **)deleted_folder_array, folders_deleted, error);
	if(!ret)
	{
		goto cleanup;		
	}	
	ret = build_folder_list((const gchar **)updated_folder_array, folders_updated, error);
	if(!ret)
	{
		goto cleanup;		
	}	


cleanup:	

	g_free(updatedSyncKey);
	free_string_array(created_folder_array);
	free_string_array(updated_folder_array);
	free_string_array(deleted_folder_array);
	
	if(!ret)	// failed - cleanup lists
	{
		g_assert (error == NULL || *error != NULL);		
		if(error)
		{
			g_error(" Error: %s", (*error)->message);
		}		
	   	g_debug("eas_mail_handler_sync_folder_hierarchy failure - cleanup lists");
		g_slist_foreach(*folders_created, (GFunc)g_free, NULL);
		g_free(*folders_created);
		*folders_created = NULL;
		g_slist_foreach(*folders_updated, (GFunc)g_free, NULL);
		g_free(*folders_updated);
		*folders_updated = NULL;
		g_slist_foreach(*folders_deleted, (GFunc)g_free, NULL);
		g_free(*folders_deleted);
		*folders_deleted = NULL;
	}
	
	g_debug("eas_mail_handler_sync_folder_hierarchy--");
	return ret;
}


/* sync emails in a specified folder (no bodies retrieved). 
Returns lists of EasEmailInfos. 
Max changes in one sync = 100 (configurable via a config api)
In the case of created emails all fields are filled in.
In the case of deleted emails only the serverids are valid. 
In the case of updated emails only the serverids, flags and categories are valid.
*/
gboolean 
eas_mail_handler_sync_folder_email_info(EasEmailHandler* self, 
    gchar *sync_key,
    const gchar *collection_id,	// folder to sync
	GSList **emailinfos_created,
	GSList **emailinfos_updated,	
	GSList **emailinfos_deleted,
	gboolean *more_available,	// if there are more changes to sync (window_size exceeded)
	GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas; 
	gchar **created_emailinfo_array = NULL;
	gchar **deleted_emailinfo_array = NULL;
	gchar **updated_emailinfo_array = NULL;
	gchar *updatedSyncKey = NULL;
    
	g_debug("eas_mail_handler_sync_folder_email_info++");
	g_debug("sync_key = %s", sync_key);
	g_assert(self);
	g_assert(sync_key);
	g_assert(collection_id);
	g_assert(more_available);
	
	g_debug("eas_mail_handler_sync_folder_email_info about to call dbus proxy");
	// call dbus api with appropriate params
	ret = dbus_g_proxy_call(proxy, "sync_folder_email", error,
							G_TYPE_STRING, self->priv->account_uid,
							G_TYPE_STRING, sync_key,
	                        G_TYPE_STRING, collection_id,			// folder 
							G_TYPE_INVALID, 
							G_TYPE_STRING, &updatedSyncKey,
							G_TYPE_BOOLEAN, more_available,
							G_TYPE_STRV, &created_emailinfo_array,
							G_TYPE_STRV, &deleted_emailinfo_array,  
							G_TYPE_STRV, &updated_emailinfo_array,
							G_TYPE_INVALID);
	
	g_debug("eas_mail_handler_sync_folder_email_info called proxy");
	// convert created/deleted/updated emailinfo arrays into lists of emailinfo objects (deserialise results)
	if(ret)
	{
		g_debug("sync_folder_email called successfully");
		
		// put the updated sync key back into the original string for tracking this
		strcpy(sync_key,updatedSyncKey);
		
		// get 3 arrays of strings of 'serialised' EasEmailInfos, convert to EasEmailInfo lists:
		ret = build_emailinfo_list((const gchar **)created_emailinfo_array, emailinfos_created, error);
		if(ret)
		{
			ret = build_emailinfo_list((const gchar **)deleted_emailinfo_array, emailinfos_deleted, error);
		}
		if(ret)
		{
			ret = build_emailinfo_list((const gchar **)updated_emailinfo_array, emailinfos_updated, error);
		}
	}

	if(updatedSyncKey){
	    g_free(updatedSyncKey);
	}
	free_string_array(created_emailinfo_array);
	free_string_array(updated_emailinfo_array);
	free_string_array(deleted_emailinfo_array);
	
	if(!ret)	// failed - cleanup lists
	{
		g_slist_foreach(*emailinfos_created, (GFunc)g_free, NULL);
		g_free(*emailinfos_created);
		*emailinfos_created = NULL;
		g_slist_foreach(*emailinfos_updated, (GFunc)g_free, NULL);
		g_free(*emailinfos_updated);
		*emailinfos_updated = NULL;
		g_slist_foreach(*emailinfos_deleted, (GFunc)g_free, NULL);
		g_free(*emailinfos_deleted);
		*emailinfos_deleted = NULL;
	}	
	g_debug("eas_mail_handler_sync_folder_email_info--");
	g_debug("sync_key = %s", sync_key);
	
	return ret;
}


// get the entire email body for listed emails
// each email body will be written to a file with the emailid as its name
gboolean 
eas_mail_handler_fetch_email_body(EasEmailHandler* self, 
        const gchar *folder_id, 		                                        
        const gchar *server_id, 		
		const gchar *mime_directory,
		GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas; 
    
	g_debug("eas_mail_handler_fetch_email_bodies++");
	g_assert(self);
	g_assert(folder_id);
	g_assert(server_id);
	g_assert(mime_directory);

	// call dbus api
	ret = dbus_g_proxy_call(proxy, "fetch_email_body", error,
	                        G_TYPE_STRING, self->priv->account_uid, 
	                        G_TYPE_STRING, folder_id,
	                        G_TYPE_STRING, server_id,
	                        G_TYPE_STRING, mime_directory,
	                        G_TYPE_INVALID,
	                        G_TYPE_INVALID);

	// nothing else to do 
	
	g_debug("eas_mail_handler_fetch_email_bodies--");

	return ret;
}


gboolean 
eas_mail_handler_fetch_email_attachment(EasEmailHandler* self, 
        const gchar *file_reference, 	
		const gchar *mime_directory,	
		GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas; 

    g_debug("eas_mail_handler_fetch_email_attachment++");
	g_assert(self);
	g_assert(file_reference);
	g_assert(mime_directory);

	// call dbus api
	ret = dbus_g_proxy_call(proxy, "fetch_attachment", error,
	                        G_TYPE_STRING, self->priv->account_uid, 
	                        G_TYPE_STRING, file_reference,
	                        G_TYPE_STRING, mime_directory,
	                        G_TYPE_INVALID,
	                        G_TYPE_INVALID);

	// nothing else to do 	
	
	g_debug("eas_mail_handler_fetch_email_attachments--");

	return ret;
}


// Delete specified emails from a single folder
gboolean 
eas_mail_handler_delete_email(EasEmailHandler* self, 
								gchar *sync_key,			// sync_key for the folder containing these emails
								const gchar *folder_id,		// folder that contains email to delete
                                const GSList *items_deleted,		// emails to delete
								GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas; 
	gchar *updatedSyncKey = NULL;
    gchar **deleted_items_array = NULL;
    // Build string array from items_deleted GSList
    guint list_length = g_slist_length((GSList*)items_deleted);
    int loop = 0;
    
	g_debug("eas_mail_handler_delete_emails++");
	g_assert(self);
	g_assert(sync_key);	
	g_assert(items_deleted);

    deleted_items_array = g_malloc0((list_length+1) * sizeof(gchar*));

    for (; loop < list_length; ++loop)
    {
        deleted_items_array[loop] = g_strdup(g_slist_nth_data((GSList*)items_deleted, loop));
        g_debug("Deleted Id: [%s]",deleted_items_array[loop]);
    }

	ret = dbus_g_proxy_call(proxy, "delete_email", error,
				  G_TYPE_STRING, self->priv->account_uid,
		          G_TYPE_STRING, sync_key,
		          G_TYPE_STRING, folder_id,
		          G_TYPE_STRV, deleted_items_array,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
		          G_TYPE_INVALID);

    // Clean up string array
    for (loop = 0; loop < list_length; ++loop)
    {
        g_free(deleted_items_array[loop]);
        deleted_items_array[loop] = NULL;
    }
    g_free(deleted_items_array);
    deleted_items_array = NULL;

	if(ret)
	{
		// put the updated sync key back into the original string for tracking this
//		strcpy(sync_key,updatedSyncKey);
	}	
	
	if(updatedSyncKey){
	    g_free(updatedSyncKey);
	}
	
	g_debug("eas_mail_handler_delete_emails--");	
	return ret;
}


/* 
'push' email updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
TODO - should this be changed to support updating multiple emails at once?
*/
gboolean 
eas_mail_handler_update_email(EasEmailHandler* self, 
								gchar *sync_key,            // sync_key for the folder containing the emails  
								const gchar *folder_id,		// folder that contains email to delete                              
                              	const GSList *update_emails,		// emails to update
								GError **error)
{
	gboolean ret = TRUE;	
	DBusGProxy *proxy = self->priv->remoteEas; 
	// serialise the emails
	guint num_emails = g_slist_length((GSList *)update_emails);
	gchar **serialised_email_array = g_malloc0((num_emails * sizeof(gchar*)) + 1);	// null terminated array of strings
	gchar *serialised_email = NULL;
	guint i;
	GSList *l = (GSList *)update_emails;

	g_debug("eas_mail_handler_update_emails++");
	g_assert(self);
	g_assert(sync_key);	
	g_assert(update_emails);
	g_debug("sync_key = %s", sync_key);
	g_debug("%d emails to update", num_emails);

	for(i = 0; i < num_emails; i++)
	{
		EasEmailInfo *email = l->data;

		g_debug("serialising email %d", i);
		ret = eas_email_info_serialise(email, &serialised_email);

		serialised_email_array[i]= serialised_email;
		g_debug("serialised_email_array[%d] = %s", i, serialised_email_array[i]);

		l = l->next;
	}
	serialised_email_array[i] = NULL;
	
	// call dbus api
	ret = dbus_g_proxy_call(proxy, "update_emails", error,
							G_TYPE_STRING, self->priv->account_uid, 		
							G_TYPE_STRING, sync_key,
							G_TYPE_STRING, folder_id,
							G_TYPE_STRV, serialised_email_array,		
							G_TYPE_INVALID, 
							G_TYPE_INVALID);	                        

	// free all strings in the array
	for(i = 0; i < num_emails; i++)
	{
		g_free(serialised_email_array[i]);
	}
	g_free(serialised_email_array);

	g_debug("eas_mail_handler_update_emails--");	

	return ret;
}


gboolean 
eas_mail_handler_send_email(EasEmailHandler* self, 
    const gchar *client_email_id,	// unique message identifier supplied by client
	const gchar *mime_file,			// the full path to the email (mime) to be sent
	GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas; 
    
	g_debug("eas_mail_handler_send_email++");
	g_assert(self);
	g_assert(client_email_id);	
	g_assert(mime_file);	

	// call dbus api
	ret = dbus_g_proxy_call(proxy, "send_email", error,
	                        G_TYPE_STRING, self->priv->account_uid, 		
	                        G_TYPE_STRING, client_email_id,		
	                        G_TYPE_STRING, mime_file,
	                        G_TYPE_INVALID,
	                        G_TYPE_INVALID);	// no out params

	// nothing else to do  
	
	g_debug("eas_mail_handler_send_email--");	

	return ret;
}


gboolean 
eas_mail_handler_move_to_folder(EasEmailHandler* self, 
    EasEmailInfo *email,
	const gchar *src_folder_id,
	const gchar *dest_folder_id,
	GError **error)
{
	gboolean ret = TRUE;
	g_debug("eas_mail_handler_move_to_folder++");
	g_debug("eas_mail_handler_move_to_folder--");	
/* TODO */
	return ret;
}

// How supported in AS?
gboolean 
eas_mail_handler_copy_to_folder(EasEmailHandler* self, 
    const GSList *email_ids,
	const gchar *src_folder_id,
	const gchar *dest_folder_id,
	GError **error)
{
	gboolean ret = TRUE;
	g_debug("eas_mail_handler_copy_to_folder++");
	g_debug("eas_mail_handler_copy_to_folder--");	

	return ret;
/* TODO */
}



