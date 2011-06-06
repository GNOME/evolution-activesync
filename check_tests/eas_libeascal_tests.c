#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "../libeascal/src/libeascal.h"
#include "../libeascal/src/eas-cal-info.h"
#include "../eas-daemon/libeas/eas-cal-info-translator.h"


const char* TEST_VCALENDAR = "BEGIN:VCALENDAR\n\
PRODID:-//Microsoft Corporation//Outlook 14.0 MIMEDIR//EN\n\
VERSION:2.0\n\
METHOD:PUBLISH\n\
X-MS-OLK-FORCEINSPECTOROPEN:TRUE\n\
BEGIN:VTIMEZONE\n\
TZID:GMT Standard Time\n\
BEGIN:STANDARD\n\
DTSTART:16011028T020000\n\
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10\n\
TZOFFSETFROM:+0100\n\
TZOFFSETTO:-0000\n\
END:STANDARD\n\
BEGIN:DAYLIGHT\n\
DTSTART:16010325T010000\n\
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=3\n\
TZOFFSETFROM:-0000\n\
TZOFFSETTO:+0100\n\
END:DAYLIGHT\n\
END:VTIMEZONE\n\
BEGIN:VEVENT\n\
CLASS:PUBLIC\n\
CATEGORIES:Category 1,Category 2,Category 3\n\
CREATED:20110527T100028Z\n\
DTEND;TZID=\"GMT Standard Time\":20110525T203000\n\
DTSTAMP:20110527T100028Z\n\
DTSTART;TZID=\"GMT Standard Time\":20110525T190000\n\
LAST-MODIFIED:20110527T100028Z\n\
LOCATION:Town Hall\n\
PRIORITY:5\n\
SEQUENCE:0\n\
SUMMARY:Music recital\n\
DESCRIPTION:This is a multi-line\\ndescription of the event.\n\
TRANSP:OPAQUE\n\
UID:040000008200E00074C5B7101A82E0080000000080AC475E870ACC01000000000000000\n\
	010000000D3EAF3B001E09E42B0EC9A435026826E\n\
X-MICROSOFT-CDO-BUSYSTATUS:BUSY\n\
X-MICROSOFT-CDO-IMPORTANCE:1\n\
X-MICROSOFT-DISALLOW-COUNTER:FALSE\n\
X-MS-OLK-ALLOWEXTERNCHECK:TRUE\n\
X-MS-OLK-AUTOFILLLOCATION:FALSE\n\
X-MS-OLK-CONFTYPE:0\n\
BEGIN:VALARM\n\
TRIGGER:-P2D\n\
ACTION:DISPLAY\n\
DESCRIPTION:Reminder\n\
END:VALARM\n\
END:VEVENT\n\
END:VCALENDAR";


const char* TEST_VCALENDAR2 = "BEGIN:VCALENDAR\n\
PRODID:-//Meego//ActiveSyncD 1.0//EN\n\
VERSION:2.0\n\
METHOD:PUBLISH\n\
BEGIN:VEVENT\n\
DTSTART:20110628T103000Z\n\
SUMMARY:this is test item 1 - updated\n\
UID:040000008200E00074C5B7101A82E008000000009A5FBE467D1FCC01000000000000000\n\
 0100000000B9C7DF6EBB049448E3D99B8CC68E560\n\
LOCATION:\n\
DTEND:20110628T113000Z\n\
DESCRIPTION:\n\
CLASS:PUBLIC\n\
TRANSP:OPAQUE\n\
BEGIN:VALARM\n\
ACTION:DISPLAY\n\
DESCRIPTION:Reminder\n\
TRIGGER:-P15M\n\
END:VALARM\n\
END:VEVENT\n\
END:VCALENDAR";

const char* TEST_VCALENDAR3 = "BEGIN:VCALENDAR\n\
PRODID:-//Meego//ActiveSyncD 1.0//EN\n\
VERSION:2.0\n\
METHOD:PUBLISH\n\
BEGIN:VEVENT\n\
DTSTART:20110628T103000Z\n\
SUMMARY:this is test item 1 - updated\n\
UID:040001008200E00074C5B7601A82Esdfgfd7D1FCCasdfdsfasd\n\
 0100000000B9C7DF6EBB049448E3D99B8CC68E560\n\
LOCATION:\n\
DTEND:20110628T113000Z\n\
DESCRIPTION:\n\
CLASS:PUBLIC\n\
TRANSP:OPAQUE\n\
BEGIN:VALARM\n\
ACTION:DISPLAY\n\
DESCRIPTION:Reminder\n\
TRIGGER:-P15M\n\
END:VALARM\n\
END:VEVENT\n\
END:VCALENDAR";



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
    gchar sync_key[64];
	strcpy(sync_key,"0");
    
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

START_TEST(test_translate_ical_to_xml)
{
	EasCalInfo cal_info;// = eas_cal_info_new();
	cal_info.icalendar = TEST_VCALENDAR;
	cal_info.server_id = "1.0 (test value)";

	xmlDocPtr doc = xmlNewDoc("1.0");
	doc->children = xmlNewDocNode(doc, NULL, "temp_root", NULL);
	xmlNodePtr node = xmlNewChild(doc->children, NULL, "ApplicationData", NULL);

	fail_if(!eas_cal_info_translator_parse_request(doc, node, &cal_info),
	        "Calendar translation failed (iCal => XML)");

//	g_object_unref(cal_info);
	xmlFree(doc); // TODO: need to explicitly free the node too??
}
END_TEST

START_TEST (test_eas_cal_handler_delete_cal)
{
    guint64 accountuid = 123456789;
    EasCalHandler *cal_handler = NULL;
    GError *error = NULL;
	gboolean testCalFound = FALSE;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetCalendarHandler(&cal_handler, accountuid);


    gchar folder_sync_key[64];
	strcpy(folder_sync_key,"0");
	GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;    

    testGetLatestCalendar(cal_handler,
                      folder_sync_key,
                      &calitems_created,
                      &calitems_updated,
                      &calitems_deleted,
                      &error);

	// if the calitems_created list contains a calendar item
	if(calitems_created){
		GSList *calitemToDel = NULL;
		EasCalInfo *calitem = NULL;
		gboolean rtn = FALSE;

		// get calendar item info for first calendar item in the folder
		calitem = (g_slist_nth(calitems_created, 0))->data;

		calitemToDel = g_slist_append(calitemToDel, calitem->server_id);

		// delete the first calendar item in the folder
		rtn = eas_cal_handler_delete_items(cal_handler, folder_sync_key, calitemToDel,&error);
		if(error){
			fail_if(rtn == FALSE,"%s",error->message);
		}

		g_slist_free(calitemToDel);
		
		// free calendar item objects list before reusing
		g_slist_foreach(calitems_deleted, (GFunc)g_object_unref, NULL);
		g_slist_foreach(calitems_updated, (GFunc)g_object_unref, NULL);
		g_slist_foreach(calitems_created, (GFunc)g_object_unref, NULL);

		g_slist_free(calitems_deleted);
		g_slist_free(calitems_updated);
		g_slist_free(calitems_created);	

		calitems_deleted = NULL;
		calitems_updated = NULL;
		calitems_created = NULL;

		testCalFound = TRUE;
	}

	// fail the test if there is no cal item as this means the 
	// test has not exercised the code to get the email body as required by this test case
	fail_if(testCalFound == FALSE,"no cal item found");
		
    //  free calendar item objects in lists of calendar items objects
    g_slist_foreach(calitems_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(calitems_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(calitems_created, (GFunc)g_object_unref, NULL);
}
END_TEST

START_TEST (test_eas_cal_handler_update_cal)
{
    guint64 accountuid = 123456789;
    EasCalHandler *cal_handler = NULL;
    GError *error = NULL;
	gboolean testCalFound = FALSE;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetCalendarHandler(&cal_handler, accountuid);


    gchar folder_sync_key[64];
	strcpy(folder_sync_key,"0");
	GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;    

    testGetLatestCalendar(cal_handler,
                      folder_sync_key,
                      &calitems_created,
                      &calitems_updated,
                      &calitems_deleted,
                      &error);

	// if the calitems_created list contains a calendar item
	if(calitems_created){
		GSList *calitemToUpdate = NULL;
		EasCalInfo *calitem = NULL;
		EasCalInfo *updatedcalitem = NULL;
		gboolean rtn = FALSE;

		// get calendar item info for first calendar item in the folder
		calitem = (g_slist_nth(calitems_created, 0))->data;
		
		
		updatedcalitem = eas_cal_info_new();
		updatedcalitem->server_id = g_strdup(calitem->server_id);	
		updatedcalitem->icalendar = g_strdup(TEST_VCALENDAR2);
		
		calitemToUpdate = g_slist_append(calitemToUpdate, updatedcalitem);

		// update the first calendar item in the folder
		rtn = eas_cal_handler_update_items(cal_handler, folder_sync_key, calitemToUpdate,&error);
		if(error){
			fail_if(rtn == FALSE,"%s",error->message);
		}

		g_slist_free(calitemToUpdate);
		
		// free calendar item objects list before reusing
		g_slist_foreach(calitems_deleted, (GFunc)g_object_unref, NULL);
		g_slist_foreach(calitems_updated, (GFunc)g_object_unref, NULL);
		g_slist_foreach(calitems_created, (GFunc)g_object_unref, NULL);

		g_slist_free(calitems_deleted);
		g_slist_free(calitems_updated);
		g_slist_free(calitems_created);	

		calitems_deleted = NULL;
		calitems_updated = NULL;
		calitems_created = NULL;

		testCalFound = TRUE;
	}

	// fail the test if there is no cal item as this means the 
	// test has not exercised the code to get the email body as required by this test case
	fail_if(testCalFound == FALSE,"no cal item found");
		
    //  free calendar item objects in lists of calendar items objects
    g_slist_foreach(calitems_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(calitems_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(calitems_created, (GFunc)g_object_unref, NULL);
}
END_TEST


START_TEST (test_eas_cal_handler_add_cal)
{
    guint64 accountuid = 123456789;
    EasCalHandler *cal_handler = NULL;
    GError *error = NULL;
	gboolean testCalFound = FALSE;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetCalendarHandler(&cal_handler, accountuid);


    gchar folder_sync_key[64];
	strcpy(folder_sync_key,"0");
	GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;    

    testGetLatestCalendar(cal_handler,
                      folder_sync_key,
                      &calitems_created,
                      &calitems_updated,
                      &calitems_deleted,
                      &error);

	

	GSList *calitemToUpdate = NULL;
	EasCalInfo *updatedcalitem = NULL;
	gboolean rtn = FALSE;


		
	updatedcalitem = eas_cal_info_new();
	updatedcalitem->client_id = g_strdup("sdfasdfsdf");
	updatedcalitem->icalendar = g_strdup(TEST_VCALENDAR3);
		
	calitemToUpdate = g_slist_append(calitemToUpdate, updatedcalitem);


	rtn = eas_cal_handler_add_items(cal_handler, folder_sync_key, calitemToUpdate,&error);
	if(error){
		fail_if(rtn == FALSE,"%s",error->message);
	}
	updatedcalitem = calitemToUpdate->data;
	fail_if(updatedcalitem->server_id == NULL, "Not got new id for item");

	g_slist_free(calitemToUpdate);
		

	testCalFound = TRUE;

}
END_TEST



Suite* eas_libeascal_suite (void)
{
  Suite* s = suite_create ("libeascal");

  /* tc_libeascal test case */
  TCase *tc_libeascal = tcase_create ("core");
  suite_add_tcase (s, tc_libeascal);
  //tcase_add_test (tc_libeascal, test_cal);
  
  tcase_add_test (tc_libeascal, test_get_cal_handler);
  tcase_add_test (tc_libeascal, test_get_latest_calendar_items);
//  tcase_add_test (tc_libeascal, test_translate_ical_to_xml);
  //tcase_add_test (tc_libeascal, test_eas_cal_handler_delete_cal);
  //tcase_add_test (tc_libeascal, test_eas_cal_handler_update_cal);
	//tcase_add_test (tc_libeascal, test_eas_cal_handler_add_cal);

  return s;
}
