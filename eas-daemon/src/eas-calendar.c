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
        
         eas_sync_req_Activate_Finish (syncReqObj,
                                                    &ret_sync_key,
                                                    &added_items,
                                                    &updated_items,
                                                    &deleted_items);
         e_flag_free (eflag);
         g_debug("eas_mail_sync_email_folder_hierarchy - serialise objects");
         
         
         //serialise the calendar objects from GSList* to char** and populate  :
        //TODO: make sure this stuff is ok to go over dbus.
         
         // Return the error or the requested data to the mail client
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

       g_debug("eas_mail_sync_email_folder_hierarchy--");
}

gboolean 
eas_calendar_delete_calendar_items(EasCalendar* self,
                                    const gchar* sync_key, 
                                    const gchar **server_id,
                                    GError **error)
{
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


