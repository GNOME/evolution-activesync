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


G_DEFINE_TYPE (EasCalHandler, eas_cal_handler, G_TYPE_OBJECT);

struct _EasCalHandlerPrivate{
    DBusGProxy *remoteEas;
    guint64 account_uid;		// TODO - is it appropriate to have a dbus proxy per account if we have multiple accounts making requests at same time?
};

// TODO - how much verification of args should happen 

static void
eas_cal_handler_init (EasCalHandler *cnc)
{
	g_print("eas_cal_handler_init++\n");
    EasCalHandlerPrivate *priv;
    
	/* allocate internal structure */
	priv = g_new0 (EasCalHandlerPrivate, 1);
	
	priv->remoteEas = NULL;
	priv->account_uid = 0;
	cnc->priv = priv;	
	g_print("eas_cal_handler_init--\n");
}

static void
eas_cal_handler_finalize (GObject *object)
{
	g_print("eas_cal_handler_finalize++\n");
	EasCalHandler *cnc = (EasCalHandler *) object;
	EasCalHandlerPrivate *priv;

	priv = cnc->priv;
	
	g_free (priv);
	cnc->priv = NULL;

	G_OBJECT_CLASS (eas_cal_handler_parent_class)->finalize (object);
	g_print("eas_cal_handler_finalize--\n");
}

static void
eas_cal_handler_class_init (EasCalHandlerClass *klass)
{
	g_print("eas_cal_handler_class_init++\n");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_cal_handler_finalize;
	g_print("eas_cal_handler_class_init--\n");
}

EasCalHandler *
eas_cal_handler_new(guint64 account_uid)
{
	g_print("eas_cal_handler_new++\n");
	DBusGConnection* bus;
	DBusGProxy* remoteEas;
	GMainLoop* mainloop;
	GError* error = NULL;
	EasCalHandler *object = NULL;

	g_type_init();

	mainloop = g_main_loop_new(NULL, TRUE);

	if (mainloop == NULL) {
		g_printerr("Error: Failed to create the mainloop\n");
		return NULL;
	}

	g_print("Connecting to Session D-Bus.\n");
	bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_printerr("Error: Couldn't connect to the Session bus (%s) \n", error->message);
		return NULL;
	}

	g_print("Creating a GLib proxy object for Eas.\n");
	remoteEas =  dbus_g_proxy_new_for_name(bus,
		      EAS_SERVICE_NAME,
		      EAS_SERVICE_CALENDAR_OBJECT_PATH,
		      EAS_SERVICE_CALENDAR_INTERFACE);
	if (remoteEas == NULL) {
		g_printerr("Error: Couldn't create the proxy object\n");
		return NULL;
	}

	object = g_object_new (EAS_TYPE_CAL_HANDLER , NULL);

	if(object == NULL){
		g_printerr("Error: Couldn't create mail\n");
		g_print("eas_cal_handler_new--\n");
		return NULL;  
	}

	object->priv->remoteEas = remoteEas; 
	object->priv->account_uid = account_uid;

	g_print("eas_cal_handler_new--\n");
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
    g_debug("eas_cal_handler_get_calendar_items++\n");

	g_assert(this);
	g_assert(sync_key);

	gboolean ret = TRUE;
	DBusGProxy *proxy = this->priv->remoteEas; 

    g_debug("eas_cal_handler_sync_folder_hierarch - dbus proxy ok\n");
    
	g_assert(g_slist_length(*items_created) == 0);
	g_assert(g_slist_length(*items_updated) == 0);
	g_assert(g_slist_length(*items_deleted) == 0);
	
	gchar **created_item_array = NULL;
	gchar **deleted_item_array = NULL;
	gchar **updated_item_array = NULL;

	// call DBus API
	ret = dbus_g_proxy_call(proxy, "get_latest_calendar_items",
		          error,
				  G_TYPE_UINT64, 
	              this->priv->account_uid,
		          G_TYPE_STRING,
		          sync_key,
		          G_TYPE_INVALID, 
		          G_TYPE_STRING, &sync_key,
		          G_TYPE_STRV, &created_item_array,
		          G_TYPE_STRV, &deleted_item_array,
		          G_TYPE_STRV, &updated_item_array,
		          G_TYPE_INVALID);

    g_debug("eas_cal_handler_get_calendar_items - dbus proxy called\n");
    if (*error) {
        g_error(" Error: %s\n", (*error)->message);
    }
    
	if(ret)
	{
		g_debug("get_latest_calendar_items called successfully\n");
		
		//TODO: any serialisation gubbins that it turns out we need to do.
	}
	
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
	
	g_debug("eas_mail_handler_sync_folder_hierarchy--\n");
	return ret;
}









