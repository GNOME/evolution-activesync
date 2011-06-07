/**
 *  
 *  Filename: libeascal.c
 *  Project:  
 *  Description: client side library for eas calendar (wraps dbus api)
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
#include "libeascal.h"
#include "eas-cal-info.h"

#include "../../logger/eas-logger.h"

G_DEFINE_TYPE (EasCalHandler, eas_cal_handler, G_TYPE_OBJECT);

struct _EasCalHandlerPrivate{
    DBusGProxy *remoteEas;
    guint64 account_uid;		// TODO - is it appropriate to have a dbus proxy per account if we have multiple accounts making requests at same time?
};

static gboolean 
build_serialised_calendar_info_array(gchar ***serialised_cal_info_array, const GSList *cal_list, GError **error);

// TODO - how much verification of args should happen 

static void
eas_cal_handler_init (EasCalHandler *cnc)
{
	g_debug("eas_cal_handler_init++");
    EasCalHandlerPrivate *priv;
    
	/* allocate internal structure */
	priv = g_new0 (EasCalHandlerPrivate, 1);
	
	priv->remoteEas = NULL;
	priv->account_uid = 0;
	cnc->priv = priv;	
	g_debug("eas_cal_handler_init--");
}

static void
eas_cal_handler_finalize (GObject *object)
{
	g_debug("eas_cal_handler_finalize++");
	EasCalHandler *cnc = (EasCalHandler *) object;
	EasCalHandlerPrivate *priv;

	priv = cnc->priv;
	
	g_free (priv);
	cnc->priv = NULL;

	G_OBJECT_CLASS (eas_cal_handler_parent_class)->finalize (object);
	g_debug("eas_cal_handler_finalize--");
}

static void
eas_cal_handler_class_init (EasCalHandlerClass *klass)
{
	g_debug("eas_cal_handler_class_init++");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_cal_handler_finalize;
	g_debug("eas_cal_handler_class_init--");
}

EasCalHandler *
eas_cal_handler_new(guint64 account_uid)
{
	DBusGConnection* bus;
	DBusGProxy* remoteEas;
	GMainLoop* mainloop;
	GError* error = NULL;
	EasCalHandler *object = NULL;

	g_type_init();

    g_log_set_default_handler(eas_logger, NULL);
	g_debug("eas_cal_handler_new++");

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
		      EAS_SERVICE_CALENDAR_OBJECT_PATH,
		      EAS_SERVICE_CALENDAR_INTERFACE);
	if (remoteEas == NULL) {
		g_error("Error: Couldn't create the proxy object");
		return NULL;
	}

	object = g_object_new (EAS_TYPE_CAL_HANDLER , NULL);

	if(object == NULL){
		g_error("Error: Couldn't create mail");
		return NULL;  
	}

	object->priv->remoteEas = remoteEas; 
	object->priv->account_uid = account_uid;

	g_debug("eas_cal_handler_new--");
	return object;

}

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

gboolean eas_cal_handler_get_calendar_items(EasCalHandler* this, 
                                                 gchar *sync_key, 
                                                 GSList **items_created,	
                                                 GSList **items_updated,
                                                 GSList **items_deleted,
                                                 GError **error)
{
    g_debug("eas_cal_handler_get_calendar_items++");

	g_assert(this);
	g_assert(sync_key);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this->priv->remoteEas; 

    g_debug("eas_cal_handler_sync_folder_hierarch - dbus proxy ok");
    
	g_assert(g_slist_length(*items_created) == 0);
	g_assert(g_slist_length(*items_updated) == 0);
	g_assert(g_slist_length(*items_deleted) == 0);
	
	gchar **created_item_array = NULL;
	gchar **deleted_item_array = NULL;
	gchar **updated_item_array = NULL;
	
	gchar *updatedSyncKey;

	// call DBus API
	ret = dbus_g_proxy_call(proxy, "get_latest_calendar_items", error,
				  G_TYPE_UINT64, this->priv->account_uid,
		          G_TYPE_STRING, sync_key,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
		          G_TYPE_STRV, &created_item_array,
		          G_TYPE_STRV, &deleted_item_array,
		          G_TYPE_STRV, &updated_item_array,
		          G_TYPE_INVALID);

    g_debug("eas_cal_handler_get_calendar_items - dbus proxy called");
    if (*error) {
        g_error(" Error: %s", (*error)->message);
    }
    
	if(ret)
	{
		g_debug("get_latest_calendar_items called successfully");
		guint i=0;
		while(created_item_array[i])
	    {
	        g_debug("created item = %s", created_item_array[i]);
	        EasCalInfo *cal = eas_cal_info_new ();
	        eas_cal_info_deserialise(cal, created_item_array[i]);
	         g_debug("created item server id = %s", cal->server_id);
	        *items_created = g_slist_append(*items_created, cal);
	        i++;
	    }
	    i=0;
		while(updated_item_array[i])
	    {
	        g_debug("created item = %s", updated_item_array[i]);
	        EasCalInfo *cal = eas_cal_info_new ();
	        eas_cal_info_deserialise(cal, updated_item_array[i]);
	         g_debug("created item server id = %s", cal->server_id);
	        *items_updated = g_slist_append(*items_updated, cal);
	        i++;
	    }
	    i=0;
		while(deleted_item_array[i])
	    {
	        g_debug("deleted item = %s", deleted_item_array[i]);
	        *items_deleted = g_slist_append(*items_deleted, deleted_item_array[i]);
	        i++;
	    }
		
		// put the updated sync key back into the original string for tracking this
		strcpy(sync_key,updatedSyncKey);
	}
	
	g_free(updatedSyncKey);
	free_string_array(created_item_array);
	free_string_array(updated_item_array);
	free_string_array(deleted_item_array);
	
	if(!ret)	// failed - cleanup lists
	{
		g_slist_foreach(*items_created, (GFunc)g_free, NULL);
		g_free(*items_created);
		*items_created = NULL;
		g_slist_foreach(*items_updated, (GFunc)g_free, NULL);
		g_free(*items_updated);
		*items_updated = NULL;
		g_slist_foreach(*items_deleted, (GFunc)g_free, NULL);
		g_free(*items_deleted);
		*items_deleted = NULL;
	}

	g_debug("eas_cal_handler_get_calendar_items--");
	return ret;
}

gboolean 
eas_cal_handler_delete_items(EasCalHandler* this, 
                                                 gchar *sync_key, 
                                                 GSList *items_deleted,
                                                 GError **error)
{
    g_debug("eas_cal_handler_delete_items++");

	g_assert(this);
	g_assert(sync_key);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this->priv->remoteEas; 

    g_debug("eas_cal_handler_delete_items - dbus proxy ok");
    
    gchar *updatedSyncKey;

	// call DBus API
	ret = dbus_g_proxy_call(proxy, "delete_calendar_items", error,
				  G_TYPE_UINT64, this->priv->account_uid,
		          G_TYPE_STRING, sync_key,
   		          G_TYPE_STRV, items_deleted,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
		          G_TYPE_INVALID);

    g_debug("eas_cal_handler_delete_items - dbus proxy called");
    if (*error) {
        g_error(" Error: %s", (*error)->message);
    }
    
	if(ret)
	{
		g_debug("delete_calendar_items called successfully");
		g_free(updatedSyncKey);
	}

	g_debug("eas_cal_handler_delete_items--");
	return ret;
}

gboolean 
eas_cal_handler_update_items(EasCalHandler* this, 
                                                 gchar *sync_key, 
                                                 GSList *items_updated,
                                                 GError **error)
{
    g_debug("eas_cal_handler_update_items++");

	g_assert(this);
	g_assert(sync_key);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this->priv->remoteEas; 

    g_debug("eas_cal_handler_update_items - dbus proxy ok");
    
    gchar *updatedSyncKey;
    
   gchar **updated_item_array = NULL;
   
   
   g_debug("server_id = %s", ((EasCalInfo*)(items_updated->data))->server_id);

    build_serialised_calendar_info_array (&updated_item_array, items_updated, error);
    
	// call DBus API
	ret = dbus_g_proxy_call(proxy, "update_calendar_items", error,
				  G_TYPE_UINT64, this->priv->account_uid,
		          G_TYPE_STRING, sync_key,
   		          G_TYPE_STRV, updated_item_array,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
		          G_TYPE_INVALID);

    g_debug("eas_cal_handler_update_items - dbus proxy called");
    if (*error) {
        g_error(" Error: %s", (*error)->message);
    }
    
	if(ret)
	{
		g_debug("update_calendar_items called successfully");
		g_free(updatedSyncKey);
	}

	g_debug("eas_cal_handler_update_items--");
	return ret;
}

// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
static gboolean 
build_serialised_calendar_info_array(gchar ***serialised_cal_info_array, const GSList *cal_list, GError **error)
{
    g_debug("build cal arrays++");
	gboolean ret = TRUE;
	guint i = 0;

    g_assert(serialised_cal_info_array);
    g_assert(*serialised_cal_info_array == NULL);

	guint array_len = g_slist_length((GSList*)cal_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
    
	*serialised_cal_info_array = g_malloc0(array_len * sizeof(gchar*));

    
	GSList *l = (GSList*)cal_list;

	for(i = 0; i < array_len - 1; i++){
		g_assert(l != NULL);
		EasCalInfo *calInfo =l->data;
		gchar *tstring = NULL;
		eas_cal_info_serialise(calInfo, &tstring);
		(*serialised_cal_info_array)[i]=tstring;
		l = g_slist_next (l);
	}
    
	return ret;
}

gboolean 
eas_cal_handler_add_items(EasCalHandler* this, 
                                                 gchar *sync_key, 
                                                 GSList *items_added,
                                                 GError **error)
{
    g_debug("eas_cal_handler_add_items++");

	g_assert(this);
	g_assert(sync_key);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this->priv->remoteEas; 

    g_debug("eas_cal_handler_updaddate_items - dbus proxy ok");
    
    gchar *updatedSyncKey;
    
   gchar **added_item_array = NULL;

   gchar **created_item_array = NULL;
   
   
   g_debug("server_id = %s", ((EasCalInfo*)(items_added->data))->server_id);

    build_serialised_calendar_info_array (&added_item_array, items_added, error);
    
	// call DBus API
	ret = dbus_g_proxy_call(proxy, "add_calendar_items", error,
				  G_TYPE_UINT64, this->priv->account_uid,
		          G_TYPE_STRING, sync_key,
   		          G_TYPE_STRV, added_item_array,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &updatedSyncKey,
	              G_TYPE_STRV, &created_item_array,
		          G_TYPE_INVALID);

    g_debug("eas_cal_handler_add_items - dbus proxy called");
    if (*error) {
        g_error(" Error: %s", (*error)->message);
    }
    
	if(ret)
	{
		g_debug("add_calendar_items called successfully");
		guint i=0;
		guint length = g_slist_length(items_added);
		while(created_item_array[i] && i < length)
	    {
			
	        g_debug("created item = %s", created_item_array[i]);
	        EasCalInfo *cal = eas_cal_info_new ();
			EasCalInfo *updated = g_slist_nth(items_added, i)->data;
	        eas_cal_info_deserialise(cal, created_item_array[i]);
	        g_debug("created item server id = %s", cal->server_id);
			updated->server_id = g_strdup(cal->server_id);
			g_debug("created updated server id in list = %s", cal->server_id);
	        i++;
	    }
		if(i==length && created_item_array[i]){
			g_debug("added list is not the same length as input list - problem?");
		}
		g_free(updatedSyncKey);
	}

	g_debug("eas_cal_handler_add_items--");
	return ret;
}







