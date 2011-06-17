/*
 *  Filename: eas-sync.c
 */

#include "eas-sync.h"
#include "eas-sync-stub.h"
#include "eas-sync-req.h"
#include "eas-delete-email-req.h"
#include "eas-update-calendar-req.h"
#include "eas-add-calendar-req.h"
#include "../../libeassync/src/eas-cal-info.h"

#include "../libeas/eas-connection.h"
#include "eas-mail.h"

G_DEFINE_TYPE (EasSync, eas_sync, G_TYPE_OBJECT);

#define EAS_SYNC_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC, EasSyncPrivate))


struct _EasSyncPrivate
{
    EasConnection* connection;
};

static void
eas_sync_init (EasSync *object)
{
    EasSyncPrivate *priv =NULL;
	g_debug("++ eas_sync_init()");
	object->priv = priv = EAS_SYNC_PRIVATE(object);
	priv->connection = NULL;
 	g_debug("-- eas_sync_init()");
}

static void
eas_sync_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_sync_parent_class)->finalize (object);
}

static void
eas_sync_class_init (EasSyncClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	(void)parent_class; // remove warning

	object_class->finalize = eas_sync_finalize;

	g_type_class_add_private (klass, sizeof (EasSyncPrivate));

	/* Binding to GLib/D-Bus" */ 
	dbus_g_object_type_install_info(EAS_TYPE_SYNC,
                                            &dbus_glib_eas_sync_object_info);
}

EasSync* eas_sync_new(void)
{
	EasSync* easCal = NULL;
	easCal = g_object_new(EAS_TYPE_SYNC, NULL);
	return easCal;
}


void eas_sync_set_eas_connection(EasSync* self, EasConnection* easConnObj)
{
   EasSyncPrivate* priv = self->priv;
   priv->connection = easConnObj;
}


EasConnection*
  eas_sync_get_eas_connection(EasSync* self)
{
    EasSyncPrivate* priv = self->priv;
    g_debug("eas_sync_get_eas_connection++");
    return priv->connection;
    g_debug("eas_sync_get_leas_connection--");
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
	gboolean ret = TRUE;
	guint i = 0;
	GSList *l = (GSList*)cal_list;
	guint array_len = g_slist_length((GSList*)cal_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
    
    g_debug("build cal arrays++");
    g_assert(serialised_cal_info_array);
    g_assert(*serialised_cal_info_array == NULL);

	*serialised_cal_info_array = g_malloc0(array_len * sizeof(gchar*));

	for(i = 0; i < array_len - 1; i++){
		gchar *tstring = g_strdup(l->data);
		g_assert(l != NULL);
		(*serialised_cal_info_array)[i]=tstring;
		l = g_slist_next (l);
	}

	return ret;
}

void 
eas_sync_get_latest_items(EasSync* self,
                                          guint64 account_uid,
                              			  guint64 type,
                                          const gchar* folder_id,
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
    // Create the request
    EasSyncReq *syncReqObj = NULL;

    g_debug("eas_sync_get_latest_calendar_items++");

    eflag = e_flag_new ();
    
    if(self->priv->connection)
    {
        eas_connection_set_account(self->priv->connection, account_uid);
    }


    g_debug("eas_sync_get_latest_calendar_items++");
    syncReqObj = g_object_new(EAS_TYPE_SYNC_REQ , NULL);

    eas_request_base_SetConnection (&syncReqObj->parent_instance, 
                                    self->priv->connection);
                                    

    g_debug("eas_sync_get_latest_calendar_items - new req");
    // Start the request
    eas_sync_req_Activate (syncReqObj, sync_key, account_uid, eflag, folder_id, type, &error);

    g_debug("eas_sync_get_latest_calendar_items  - activate req");
    // Set flag to wait for response
    e_flag_wait(eflag);
    e_flag_free (eflag);

    // TODO Check error
    
    g_debug("eas_sync_get_latest_calendar_items  - get results");

    eas_sync_req_ActivateFinish (syncReqObj,
                                 &ret_sync_key,
				 NULL, /* FIXME */
                                 &added_items,
                                 &updated_items,
                                 &deleted_items,
                                 &error);

    // TODO Check Error
            
	switch(type)
	{
		case EAS_ITEM_CALENDAR:
		{
			if(build_serialised_calendar_info_array (&ret_created_items_array, added_items, &error))
			{
				if(build_serialised_calendar_info_array(&ret_updated_items_array, updated_items, &error))
				{
				    build_serialised_calendar_info_array(&ret_deleted_items_array, deleted_items, &error);          
				}
			}
		}
		break;
		default:
		{
			//TODO: put some error in here for unknown type
		}
		break;
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
    g_debug("eas_sync_get_latest_calendar_items--");
}

gboolean 
eas_sync_delete_items(EasSync* self,
                                    guint64 account_uid,
                                    const gchar* folder_id,
                                    const gchar* sync_key, 
                                    const GSList *deleted_items_array,
                                    DBusGMethodInvocation* context)
{
    EFlag *flag = NULL;
    GError *error = NULL;
    gchar* ret_sync_key = NULL;
	EasDeleteEmailReq *req = NULL;

    g_debug("eas_sync_delete_calendar_items++");

    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(self->priv->connection, account_uid);
    }

    // Create the request
	req = eas_delete_email_req_new (account_uid, sync_key, folder_id, deleted_items_array, flag);

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
	g_debug("eas_sync_delete_calendar_items--");
	return TRUE;
}

gboolean 
eas_sync_update_items(EasSync* self,
                                    guint64 account_uid,
                                    guint64 type,
                                    const gchar* folder_id,
                                    const gchar* sync_key, 
                                    const gchar **calendar_items,
                                    DBusGMethodInvocation* context)
{
    GError* error = NULL;
    EFlag *flag = NULL;
    gchar* ret_sync_key = NULL;
    GSList *items = NULL;
	EasUpdateCalendarReq *req = NULL;

	g_debug("eas_sync_update_calendar_items++");
	 
    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(eas_sync_get_eas_connection(self), account_uid);
    }

	switch(type)
	{
		case EAS_ITEM_CALENDAR:
		{
    		build_calendar_list(calendar_items, &items, &error);
		}
		break;
		default:
		{
			//TODO: put unknown type error here.
		}
	}
    // Create the request
	req = eas_update_calendar_req_new (account_uid, sync_key, type, folder_id, items, flag);

	eas_request_base_SetConnection (&req->parent_instance, 
                                   eas_sync_get_eas_connection(self));

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
	g_debug("eas_sync_update_calendar_items--");
	return TRUE;
}

gboolean 
eas_sync_add_items(EasSync* self,
                                    guint64 account_uid,
                        			guint64 type,
                                    const gchar* folder_id,
                                    const gchar* sync_key, 
                                    const gchar **calendar_items,
                                    DBusGMethodInvocation* context)
{
    GError* error = NULL;
    EFlag *flag = NULL;
    gchar* ret_sync_key = NULL;
    GSList *items = NULL;
	GSList* added_items = NULL;
	gchar** ret_created_items_array = NULL;
	EasAddCalendarReq *req = NULL;

	g_debug("eas_sync_add_calendar_items++");
 
    flag = e_flag_new ();

    if(self->priv->connection)
    {
        eas_connection_set_account(eas_sync_get_eas_connection(self), account_uid);
    }

    switch(type)
	{
		case EAS_ITEM_CALENDAR:
		{
    		build_calendar_list(calendar_items, &items, &error);
		}
		break;
		default:
		{
			//TODO: put unknown type error here.
		}
		break;
	}

    // Create the request
	req = eas_add_calendar_req_new (account_uid, sync_key, folder_id, items, flag);

	eas_request_base_SetConnection (&req->parent_instance, 
                                   eas_sync_get_eas_connection(self));

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
		switch(type)
		{
			case EAS_ITEM_CALENDAR:
			{
				build_serialised_calendar_info_array (&ret_created_items_array, added_items, &error);
			}
			break;
			default:
			{
				//TODO: put unknown type error here.
			}
			break;
		}


		dbus_g_method_return (context,
                              ret_sync_key,
                              ret_created_items_array);
    }	
	g_debug("eas_sync_add_calendar_items--");
	return TRUE;
}


