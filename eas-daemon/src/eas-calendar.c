/*
 *  Filename: eas-calendar.c
 */

#include "eas-calendar.h"
#include "eas-calendar-stub.h"
#include "eas-sync-req.h"
#include "eas-delete-email-req.h"
#include "eas-update-calendar-req.h"
#include "eas-add-calendar-req.h"
#include "../../libeascal/src/eas-cal-info.h"

#include "../libeas/eas-connection.h"
#include "eas-mail.h"

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
	object->priv = priv = EAS_CALENDAR_PRIVATE(object);                    
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

    (void)parent_class; // remove warning

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
   EasCalendarPrivate* priv = self->priv;
   priv->connection = easConnObj;
}


EasConnection*
  eas_calendar_get_eas_connection(EasCalendar* self)
{
    g_debug("eas_calendar_get_eas_connection++");
    EasCalendarPrivate* priv = self->priv;
    return priv->connection;
    g_debug("eas_calendar_get_leas_connection--");
}

// takes an NULL terminated array of serialised calendar items and creates a list of EasCalInfo objects
static gboolean 
build_calendar_list(const gchar **serialised_cal_array, GSList **cal_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;

    g_assert(cal_list);

	g_assert(g_slist_length(*cal_list) == 0);
	
	while(serialised_cal_array[i])
	{
		EasCalInfo *calInfo = eas_cal_info_new();
		if(calInfo)
		{
			*cal_list = g_slist_append(*cal_list, calInfo);	// add it to the list first to aid cleanup
			if(!cal_list)
			{
				g_free(calInfo);
				ret = FALSE;
				goto cleanup;
			}				
			if(!eas_cal_info_deserialise(calInfo, serialised_cal_array[i]))
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
		//g_set_error (error, EAS_MAIL_ERROR,
		//	     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
		//	     ("out of memory"));
		// clean up on error
		g_slist_foreach(*cal_list,(GFunc)g_free, NULL);
		g_slist_free(*cal_list);
	}
	
	g_debug("list has %d items", g_slist_length(*cal_list));
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
    g_debug("eas_calendar_get_latest_calendar_items++");
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
    
    if(self->priv->connection)
    {
        eas_connection_set_account(self->priv->connection, account_uid);
    }

    // Create the request
    EasSyncReq *syncReqObj =NULL;

    g_debug("eas_calendar_get_latest_calendar_items++");
    syncReqObj = g_object_new(EAS_TYPE_SYNC_REQ , NULL);

    eas_request_base_SetConnection (&syncReqObj->parent_instance, 
                                    self->priv->connection);
                                    

    g_debug("eas_calendar_get_latest_calendar_items - new req");
    // Start the request
    eas_sync_req_Activate (syncReqObj, sync_key, account_uid, eflag, "1", EAS_ITEM_CALENDAR, &error);

    g_debug("eas_calendar_get_latest_calendar_items  - activate req");
    // Set flag to wait for response
    e_flag_wait(eflag);
    e_flag_free (eflag);

    // TODO Check error
    
    g_debug("eas_calendar_get_latest_calendar_items  - get results");

     eas_sync_req_ActivateFinish (syncReqObj,
                                  &ret_sync_key,
                                  &added_items,
                                  &updated_items,
                                  &deleted_items,
                                  &error);

    // TODO Check Error
             
     //serialise the calendar objects from GSList* to char** and populate  :
    //TODO: make sure this stuff is ok to go over dbus.
    
    if(build_serialised_calendar_info_array (&ret_created_items_array, added_items, &error))
	{
        if(build_serialised_calendar_info_array(&ret_updated_items_array, updated_items, &error))
		{
            build_serialised_calendar_info_array(&ret_deleted_items_array, deleted_items, &error);          
        }
    }
        
         
    // Return the error or the requested data to the calendar client
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
                          	ret_created_items_array,
				            ret_updated_items_array,
				            ret_deleted_items_array);
    }

	g_object_unref(syncReqObj);
    g_debug("eas_calendar_get_latest_calendar_items--");
}

gboolean 
eas_calendar_delete_calendar_items(EasCalendar* self,
                                    guint64 account_uid,
                                    const gchar* sync_key, 
                                    const GSList *deleted_items_array,
                                    DBusGMethodInvocation* context)
{
    g_debug("eas_calendar_delete_calendar_items++");
    EFlag *flag = NULL;
    GError *error = NULL;
    gchar* ret_sync_key = NULL;
	 
    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(self->priv->connection, account_uid);
    }


    // Create the request
	EasDeleteEmailReq *req = NULL;
	req = eas_delete_email_req_new (account_uid, sync_key, "1", deleted_items_array, flag);

	eas_request_base_SetConnection (&req->parent_instance, 
                                    self->priv->connection);

	    // Start the request
    eas_delete_email_req_Activate (req, &error);

    // Set flag to wait for response
    e_flag_wait (flag);
    e_flag_free (flag);

    // TODO Check error
    
	eas_delete_email_req_ActivateFinish(req, &ret_sync_key, &error);
		
    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
		//TODO: make sure this stuff is ok to go over dbus.
    
		dbus_g_method_return (context,
                              ret_sync_key);
    }	
	g_debug("eas_calendar_delete_calendar_items--");
	return TRUE;
}

gboolean 
eas_calendar_update_calendar_items(EasCalendar* self,
                                    guint64 account_uid,
                                    const gchar* sync_key, 
                                    const gchar **calendar_items,
                                    DBusGMethodInvocation* context)
{
    GError* error = NULL;
	g_debug("eas_calendar_update_calendar_items++");
    EFlag *flag = NULL;
    gchar* ret_sync_key = NULL;
	 
    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(eas_calendar_get_eas_connection(self), account_uid);
    }
    
    GSList *items = NULL;
    
    build_calendar_list(calendar_items, &items, &error);


    // Create the request
	EasUpdateCalendarReq *req = NULL;
	req = eas_update_calendar_req_new (account_uid, sync_key, "1", items, flag);

	eas_request_base_SetConnection (&req->parent_instance, 
                                   eas_calendar_get_eas_connection(self));

	    // Start the request
    eas_update_calendar_req_Activate (req);

	    // Set flag to wait for response
    e_flag_wait(flag);

	eas_update_calendar_req_ActivateFinish(req, &ret_sync_key, &error);
		
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
	g_debug("eas_calendar_update_calendar_items--");
	return TRUE;
}

gboolean 
eas_calendar_add_calendar_items(EasCalendar* self,
                                    guint64 account_uid,
                                    const gchar* sync_key, 
                                    const gchar **calendar_items,
                                    DBusGMethodInvocation* context)
{
    GError* error = NULL;
	g_debug("eas_calendar_add_calendar_items++");
    EFlag *flag = NULL;
    gchar* ret_sync_key = NULL;
	 
    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(eas_calendar_get_eas_connection(self), account_uid);
    }
    
    GSList *items = NULL;
	GSList* added_items = NULL;
	gchar** ret_created_items_array = NULL;
	
    build_calendar_list(calendar_items, &items, &error);


    // Create the request
	EasAddCalendarReq *req = NULL;
	req = eas_add_calendar_req_new (account_uid, sync_key, "1", items, flag);

	eas_request_base_SetConnection (&req->parent_instance, 
                                   eas_calendar_get_eas_connection(self));

	    // Start the request
    eas_add_calendar_req_Activate (req);

	    // Set flag to wait for response
    e_flag_wait(flag);

	eas_add_calendar_req_ActivateFinish(req, 
	                                    &ret_sync_key,
	                                    &added_items,
	                                    &error);
		
    if (error)
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
    } 
    else
    {
	   	build_serialised_calendar_info_array (&ret_created_items_array, added_items, &error);
        dbus_g_method_return (context,
                              ret_sync_key,
                              ret_created_items_array);
    }	
	g_debug("eas_calendar_add_calendar_items--");
	return TRUE;
}


