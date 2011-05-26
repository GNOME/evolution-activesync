#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "../libeascal/src/libeascal.h"
#include "../libeascal/src/eas-cal-info.h"


static void testGetCalendarHandler(EasCalHandler **cal_handler, guint64 accountuid){
  	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    *cal_handler = eas_cal_handler_new(accountuid);

    // confirm that the handle object has been correctly setup
    fail_if(*cal_handler == NULL,
    "eas_mail_handler_new returns NULL when given a valid ID");
    fail_if((*cal_handler)->priv == NULL,
    "eas_mail_handler_new account ID object (EasEmailHandler *) member priv (EasEmailHandlerPrivate *) NULL"); 
}

static void testGetLatestCalendar(EasCalHandler *cal_handler,
                                     gchar *sync_key,
                                     GSList **created,
                                     GSList **updated,
                                     GSList **deleted,
                                     GError **error){
    gboolean ret = FALSE;
 	mark_point();
    ret  = eas_cal_handler_get_calendar_items (cal_handler, sync_key, 	
	        &(*created),	
	        &(*updated),
	        &(*deleted),
	        &(*error));
	mark_point();
	// if the call to the daemon returned an error, report and drop out of the test
    if((*error) != NULL){
		fail_if(ret == FALSE,"%s",(*error)->message);
	}
	
	// the exchange server should increment the sync key and send back to the
	// client so that the client can track where it is with regard to sync.
	// therefore the key must not be zero as this is the seed value for this test          
    fail_if(sync_key == 0,"Sync Key not updated by call the exchange server");
	fail_if(g_slist_length(*created)==0,"list length =0");
	EasCalInfo *cal = (*created)->data;
	
}


START_TEST (test_cal)
{
//... calendar test case 

}
END_TEST

START_TEST (test_get_cal_handler)
{
    guint64 accountuid = 123456789;
    EasCalHandler *cal_handler = NULL;
  
    testGetCalendarHandler(&cal_handler, accountuid);  
}
END_TEST

START_TEST (test_get_latest_calendar_items)
{
	// This value needs to make sense in the daemon.  in the first instance
	// it should be hard coded to the value used by the daemon but later 
	// there should be a mechanism for getting the value from the same place
	// that the daemon uses
    guint64 accountuid = 123456789;
    EasCalHandler *cal_handler = NULL;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetCalendarHandler(&cal_handler, accountuid);

    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar *sync_key = "0";
    
    GError *error = NULL;

	mark_point();
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetLatestCalendar (cal_handler,sync_key,&created,&updated,&deleted,&error);
		
	//  free everything!
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);
}
END_TEST


Suite* eas_libeascal_suite (void)
{
  Suite* s = suite_create ("libeascal");

  /* tc_libeascal test case */
  TCase *tc_libeascal = tcase_create ("core");
  suite_add_tcase (s, tc_libeascal);
  tcase_add_test (tc_libeascal, test_cal);
  
  tcase_add_test (tc_libeascal, test_get_cal_handler);
  tcase_add_test (tc_libeascal, test_get_latest_calendar_items);

  return s;
}
