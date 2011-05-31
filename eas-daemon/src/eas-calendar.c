/*
 *  Filename: eas-calendar.c
 */

#include "eas-calendar.h"
#include "eas-calendar-stub.h"
#include "eas-sync-req.h"

G_DEFINE_TYPE (EasCalendar, eas_calendar, G_TYPE_OBJECT);

#define EAS_CALENDAR_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_CALENDAR, EasCalendarPrivate))


struct _EasCalendarPrivate
{
    EasConnection* connection;
};

static void
eas_calendar_init (EasCalendar *object)
{
		g_debug("++ eas_calendar_init()");
    EasCalendarPrivate *priv =NULL;
	object->_priv = priv = EAS_CALENDAR_PRIVATE(object);                    
	priv->connection = NULL;
	 	g_debug("-- eas_calendar_init()");
}

static void
eas_calendar_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_calendar_parent_class)->finalize (object);
}

static void
eas_calendar_class_init (EasCalendarClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_calendar_finalize;

	g_type_class_add_private (klass, sizeof (EasCalendarPrivate));
	
    /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_CALENDAR,
                                            &dbus_glib_eas_calendar_object_info);
}

EasCalendar* eas_calendar_new(void)
{
	EasCalendar* easCal = NULL;
	easCal = g_object_new(EAS_TYPE_CALENDAR, NULL);
	return easCal;
}


void eas_calendar_set_eas_connection(EasCalendar* self, EasConnection* easConnObj)
{
   EasCalendarPrivate* priv = self->_priv;
   priv->connection = easConnObj;
}


EasConnection*
  eas_calendar_get_eas_connection(EasCalendar* self)
{
    g_debug("eas_calendar_get_eas_connection++");
    EasCalendarPrivate* priv = self->_priv;
    return priv->connection;
    g_debug("eas_calendar_get_leas_connection--");
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
		gchar *tstring = g_strdup(l->data);
		(*serialised_cal_info_array)[i]=tstring;
		l = g_slist_next (l);
	}
    
	return ret;
}

void 
eas_calendar_get_latest_calendar_items(EasCalendar* self,
                                          guint64 account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context)
{
        GError *error = NULL;
        GSList* added_items = NULL;
        GSList* updated_items  = NULL;
        GSList* deleted_items  = NULL;
        EFlag * eflag = NULL;

        gchar* ret_sync_key = NULL;
        gchar** ret_created_items_array = NULL;
        gchar** ret_updated_items_array = NULL;
        gchar** ret_deleted_items_array = NULL;

        eflag = e_flag_new ();
        
        EasCalendarPrivate* priv = self->_priv;


        // Set the account Id into the connection
        eas_connection_set_account(priv->connection, account_uid);

        // Create the request
        EasSyncReq *syncReqObj =NULL;

        g_debug("eas_calendar_get_latest_calendar_items++");
        syncReqObj = g_object_new(EAS_TYPE_SYNC_REQ , NULL);

        eas_request_base_SetConnection (&syncReqObj->parent_instance, 
                                        eas_calendar_get_eas_connection(self));
                                        

        g_debug("eas_calendar_get_latest_calendar_items - new req");
	    // Start the request
	    eas_sync_req_Activate (syncReqObj, sync_key, account_uid, eflag, "1", EAS_ITEM_CALENDAR);

        g_debug("eas_calendar_get_latest_calendar_items  - activate req");
	    // Set flag to wait for response
	    e_flag_wait(eflag);

        g_debug("eas_calendar_get_latest_calendar_items  - get results");
        
         eas_sync_req_ActivateFinish (syncReqObj,
                                      &ret_sync_key,
                                      &added_items,
                                      &updated_items,
                                      &deleted_items);
         e_flag_free (eflag);
                 
         //serialise the calendar objects from GSList* to char** and populate  :
        //TODO: make sure this stuff is ok to go over dbus.
        
        if(build_serialised_calendar_info_array (&ret_created_items_array, added_items, &error)){
            if(build_serialised_calendar_info_array(&ret_updated_items_array, updated_items, &error)){
                build_serialised_calendar_info_array(&ret_deleted_items_array, deleted_items, &error);          
        }
   }
        
         
         // Return the error or the requested data to the calendar client
        if (error) {
		        g_debug(">> Daemon : Error ");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
		        g_debug(">> Daemon : Success-");
                dbus_g_method_return (context,
                                 	ret_sync_key,
                                  	ret_created_items_array,
						            ret_updated_items_array,
						            ret_deleted_items_array);
        }

       g_debug("eas_calendar_sync_get latest--");
}

gboolean 
eas_calendar_delete_calendar_items(EasCalendar* self,
                                    const gchar* sync_key, 
                                    const gchar **server_id,
                                    GError **error)
{
        GSList* deleted_items  = NULL;
        EFlag * eflag = NULL;

        gchar* ret_sync_key = NULL;

        eflag = e_flag_new ();
        guint i = 0;
        while(server_id[i])
	    {
	        g_debug("deleted item = %s", (gchar*)(server_id[i]));
	        deleted_items = g_slist_append(deleted_items, server_id[i]);
	        i++;
	    }
	    

        // Create the request
        /*EasSyncReq *syncReqObj =NULL;

        g_debug("eas_calendar_get_latest_calendar_items++");
        syncReqObj = g_object_new(EAS_TYPE_SYNC_REQ , NULL);

        eas_request_base_SetConnection (&syncReqObj->parent_instance, 
                                        eas_calendar_get_eas_connection(self));
                                        

        g_debug("eas_calendar_get_latest_calendar_items - new req");
	    // Start the request
	    eas_sync_req_Activate (syncReqObj, sync_key, account_uid, eflag, "1", EAS_ITEM_CALENDAR);

        g_debug("eas_calendar_get_latest_calendar_items  - activate req");
	    // Set flag to wait for response
	    e_flag_wait(eflag);

        g_debug("eas_calendar_get_latest_calendar_items  - get results");
        
         eas_sync_req_ActivateFinish (syncReqObj,
                                      &ret_sync_key,
                                      &added_items,
                                      &updated_items,
                                      &deleted_items);
         e_flag_free (eflag);
               
         //serialise the calendar objects from GSList* to char** and populate  :
        //TODO: make sure this stuff is ok to go over dbus.
         
         // Return the error or the requested data to the calendar client
        if (error) {
		        g_debug(">> Daemon : Error ");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
		        g_debug(">> Daemon : Success-");
                dbus_g_method_return (context,
                                 	ret_sync_key,
                                  	ret_created_items_array,
						            ret_updated_items_array,
						            ret_deleted_items_array);
        }
        */
       g_debug("eas_calendar_sync_get latest--");

	return TRUE;
}

gboolean 
eas_calendar_update_calendar_items(EasCalendar* self,
                                    const gchar* sync_key, 
                                    const gchar **calendar_items,
                                    GError **error)
{
	return TRUE;
}


