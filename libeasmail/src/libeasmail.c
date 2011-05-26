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

G_DEFINE_TYPE (EasEmailHandler, eas_mail_handler, G_TYPE_OBJECT);

struct _EasEmailHandlerPrivate{
    DBusGProxy *remoteEas;
    guint64 account_uid;		// TODO - is it appropriate to have a dbus proxy per account if we have multiple accounts making requests at same time?
};

// TODO - how much verification of args should happen 

static void
eas_mail_handler_init (EasEmailHandler *cnc)
{
	g_debug("eas_mail_handler_init++");
    EasEmailHandlerPrivate *priv;
    
	/* allocate internal structure */
	priv = g_new0 (EasEmailHandlerPrivate, 1);
	
	priv->remoteEas = NULL;
	priv->account_uid = 0;
	cnc->priv = priv;	
	g_debug("eas_mail_handler_init--");
}

static void
eas_mail_handler_finalize (GObject *object)
{
	g_debug("eas_mail_handler_finalize++");
	EasEmailHandler *cnc = (EasEmailHandler *) object;
	EasEmailHandlerPrivate *priv;

	priv = cnc->priv;
	
	g_free (priv);
	cnc->priv = NULL;

	G_OBJECT_CLASS (eas_mail_handler_parent_class)->finalize (object);
	g_debug("eas_mail_handler_finalize--");
}

static void
eas_mail_handler_class_init (EasEmailHandlerClass *klass)
{
	g_debug("eas_mail_handler_class_init++");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_mail_handler_finalize;
	g_debug("eas_mail_handler_class_init--");
}

EasEmailHandler *
eas_mail_handler_new(guint64 account_uid)
{
	g_debug("eas_mail_handler_new++");
	DBusGConnection* bus;
	DBusGProxy* remoteEas;
	GMainLoop* mainloop;
	GError* error = NULL;
	EasEmailHandler *object = NULL;

	g_type_init();

	mainloop = g_main_loop_new(NULL, TRUE);

	if (mainloop == NULL) {
		g_error("Error: Failed to create the mainloop");
		return NULL;
	}

	g_debug("Connecting to Session D-Bus.");
	bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Error: Couldn't connect to the Session bus (%s) ", error->message);
		return NULL;
	}

	g_debug("Creating a GLib proxy object for Eas.");
	remoteEas =  dbus_g_proxy_new_for_name(bus,
		      EAS_SERVICE_NAME,
		      EAS_SERVICE_MAIL_OBJECT_PATH,
		      EAS_SERVICE_MAIL_INTERFACE);
	if (remoteEas == NULL) {
		g_error("Error: Couldn't create the proxy object");
		return NULL;
	}

	object = g_object_new (EAS_TYPE_EMAIL_HANDLER , NULL);

	if(object == NULL){
		g_error("Error: Couldn't create mail");
		g_debug("eas_mail_handler_new--");
		return NULL;  
	}

	object->priv->remoteEas = remoteEas; 
	object->priv->account_uid = account_uid;

	g_debug("eas_mail_handler_new--");
	return object;

}

// takes an NULL terminated array of serialised folders and creates a list of EasFolder objects
static gboolean 
build_folder_list(const gchar **serialised_folder_array, GSList **folder_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;

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
	return ret;
}


// takes an NULL terminated array of serialised emailinfos and creates a list of EasEmailInfo objects
static gboolean 
build_emailinfo_list(const gchar **serialised_emailinfo_array, GSList **emailinfo_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;

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
	
	return ret;
}
				

// 
static void 
free_string_array(gchar **array)
{
	guint i = 0;
	while(array[i])
	{	
		g_free(array[i]);
		i++;
	}
	g_free(array);

}

// pulls down changes in folder structure (folders added/deleted/updated). Supplies lists of EasFolders
gboolean 
eas_mail_handler_sync_folder_hierarchy(EasEmailHandler* this_g, 
                                       gchar *sync_key, 	
                                       GSList **folders_created,	
                                       GSList **folders_updated,
                                       GSList **folders_deleted,
                                       GError **error)
{
	g_debug("eas_mail_handler_sync_folder_hierarchy++");

	g_assert(this_g);
	g_assert(sync_key);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this_g->priv->remoteEas; 

    g_debug("eas_mail_handler_sync_folder_hierarch - dbus proxy ok");
    
	g_assert(g_slist_length(*folders_created) == 0);
	g_assert(g_slist_length(*folders_updated) == 0);
	g_assert(g_slist_length(*folders_deleted) == 0);
	
	gchar **created_folder_array = NULL;
	gchar **deleted_folder_array = NULL;
	gchar **updated_folder_array = NULL;

	gchar *updatedSyncKey;
	
	// call DBus API
	ret = dbus_g_proxy_call(proxy, "sync_email_folder_hierarchy",
		          error,
				  G_TYPE_UINT64, 
	              this_g->priv->account_uid,
		          G_TYPE_STRING,
		          sync_key,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
		          G_TYPE_STRV, &created_folder_array,
		          G_TYPE_STRV, &deleted_folder_array,
		          G_TYPE_STRV, &updated_folder_array,
		          G_TYPE_INVALID);
   
    g_debug("eas_mail_handler_sync_folder_hierarch - dbus proxy called");
    if (*error) {
        g_error(" Error: %s", (*error)->message);
    }
    
	if(ret)
	{
		g_debug("sync_email_folder_hierarchy called successfully");

		// put the updated sync key back into the original string for tracking this
		strcpy(sync_key,updatedSyncKey);
		// get 3 arrays of strings of 'serialised' EasFolders, convert to EasFolder lists:
		ret = build_folder_list((const gchar **)created_folder_array, folders_created, error);
		if(ret)
		{
			ret = build_folder_list((const gchar **)deleted_folder_array, folders_deleted, error);
		}
		if(ret)	
		{
			ret = build_folder_list((const gchar **)updated_folder_array, folders_updated, error);
		}
	}

	g_free(updatedSyncKey);
	free_string_array(created_folder_array);
	free_string_array(updated_folder_array);
	free_string_array(deleted_folder_array);
	
	if(!ret)	// failed - cleanup lists
	{
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
	if(*folders_created ==NULL)
	{
	    g_debug("created list not correctly created--");

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
eas_mail_handler_sync_folder_email_info(EasEmailHandler* this_g, 
    gchar *sync_key,
    const gchar *collection_id,	// folder to sync
	GSList **emailinfos_created,
	GSList **emailinfos_updated,	
	GSList **emailinfos_deleted,
	gboolean *more_available,	// if there are more changes to sync (window_size exceeded)
	GError **error)
{
	g_debug("eas_mail_handler_sync_folder_email_info++");


	g_assert(this_g);
	g_assert(sync_key);
	g_assert(collection_id);
	g_assert(more_available);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this_g->priv->remoteEas; 

	gchar **created_emailinfo_array = NULL;
	gchar **deleted_emailinfo_array = NULL;
	gchar **updated_emailinfo_array = NULL;
	
	g_debug("eas_mail_handler_sync_folder_email_info abotu to call dbus proxy");
	// call dbus api with appropriate params
	ret = dbus_g_proxy_call(proxy, "sync_folder_email", error,
							G_TYPE_UINT64, this_g->priv->account_uid,                   
							G_TYPE_STRING, sync_key,
	                        G_TYPE_STRING, collection_id,			// folder 
							G_TYPE_INVALID, 
							G_TYPE_STRING, &sync_key,
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

	
	return ret;
}


// get the entire email body for listed emails
// each email body will be written to a file with the emailid as its name
gboolean 
eas_mail_handler_fetch_email_body(EasEmailHandler* this_g, 
        const gchar *folder_id, 		                                        
        const gchar *server_id, 		
		const gchar *mime_directory,
		GError **error)
{
	g_debug("eas_mail_handler_fetch_email_bodies++");

	g_assert(this_g);
	g_assert(folder_id);
	g_assert(server_id);
	g_assert(mime_directory);
	
	gboolean ret = TRUE;
	DBusGProxy *proxy = this_g->priv->remoteEas; 

	// call dbus api
	ret = dbus_g_proxy_call(proxy, "fetch_email_body", error,
	                        G_TYPE_UINT64, this_g->priv->account_uid, 
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
eas_mail_handler_fetch_email_attachment(EasEmailHandler* this_g, 
        const gchar *file_reference, 	
		const gchar *mime_directory,	
		GError **error)
{
	g_debug("eas_mail_handler_fetch_email_attachment++");

	g_assert(this_g);
	g_assert(file_reference);	
	g_assert(mime_directory);	
	
	gboolean ret = TRUE;
	DBusGProxy *proxy = this_g->priv->remoteEas; 

	// call dbus api
	ret = dbus_g_proxy_call(proxy, "fetch_attachment", error,
	                        G_TYPE_UINT64, this_g->priv->account_uid, 
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
eas_mail_handler_delete_email(EasEmailHandler* this_g, 
								gchar *sync_key,			// sync_key for the folder containing these emails
								const gchar *folder_id,		// folder that contains email to delete
                                const gchar *server_id,		// email to delete
								GError **error)
{
	g_debug("eas_mail_handler_delete_emails++");
	gboolean ret = TRUE;	
	
	g_assert(this_g);
	g_assert(sync_key);	
	g_assert(server_id);
		
	DBusGProxy *proxy = this_g->priv->remoteEas; 

	ret = dbus_g_proxy_call(proxy, "delete_email", error,
				  G_TYPE_UINT64, this_g->priv->account_uid,
		          G_TYPE_STRING, sync_key,
		          G_TYPE_STRING, folder_id,
		          G_TYPE_STRING, server_id,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &sync_key,
		          G_TYPE_INVALID);

	g_debug("eas_mail_handler_delete_emails--");	
	return ret;
}


/* 
'push' email updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
*/
gboolean 
eas_mail_handler_update_email(EasEmailHandler* this_g, 
								gchar *sync_key,            // sync_key for the folder containing the emails                   
								EasEmailInfo *update_email,		// EasEmailInfo to update
								GError **error)
{
	gboolean ret = TRUE;	
	g_debug("eas_mail_handler_update_emails++");
	/* TODO */
	g_debug("eas_mail_handler_update_emails--");	

	return ret;
}


gboolean 
eas_mail_handler_send_email(EasEmailHandler* this_g, 
    const gchar *client_email_id,	// unique message identifier supplied by client
	const gchar *mime_file,			// the full path to the email (mime) to be sent
	GError **error)
{
	gboolean ret = TRUE;	
	g_debug("eas_mail_handler_send_email++");

	g_assert(this_g);
	g_assert(client_email_id);	
	g_assert(mime_file);	
	
	DBusGProxy *proxy = this_g->priv->remoteEas; 

	// call dbus api
	ret = dbus_g_proxy_call(proxy, "send_email", error,
	                        G_TYPE_UINT64, this_g->priv->account_uid, 		
	                        G_TYPE_STRING, client_email_id,		
	                        G_TYPE_STRING, mime_file,
	                        G_TYPE_INVALID,
	                        G_TYPE_INVALID);	// no out params

	// nothing else to do  
	
	g_debug("eas_mail_handler_send_email--");	

	return ret;
}


gboolean 
eas_mail_handler_move_to_folder(EasEmailHandler* this_g, 
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
eas_mail_handler_copy_to_folder(EasEmailHandler* this_g, 
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



