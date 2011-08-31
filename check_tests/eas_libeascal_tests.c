#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <check.h>
#include <dbus/dbus-glib.h>

#include "eas_test_user.h"

#include "../libeassync/src/libeassync.h"
#include "../libeassync/src/eas-item-info.h"
#include "../eas-daemon/libeas/eas-cal-info-translator.h"
#include "../libeastest/src/libeastest.h"
static gchar * g_account_id = (gchar*)TEST_ACCOUNT_ID;
Suite* eas_libeascal_suite (void);
const char* TEST_VCALENDAR = "BEGIN:VCALENDAR\n\
PRODID:-//Microsoft Corporation//Outlook 14.0 MIMEDIR//EN\n\
VERSION:2.0\n\
METHOD:PUBLISH\n\
X-MS-OLK-FORCEINSPECTOROPEN:TRUE\n\
BEGIN:VTIMEZONE\n\
TZID:Pacific Standard Time\n\
BEGIN:STANDARD\n\
DTSTART:16011028T020000\n\
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10\n\
TZOFFSETFROM:-0700\n\
TZOFFSETTO:-0800\n\
END:STANDARD\n\
BEGIN:DAYLIGHT\n\
DTSTART:16010325T010000\n\
RRULE:FREQ=YEARLY;BYDAY=2SU;BYMONTH=3\n\
TZOFFSETFROM:-0800\n\
TZOFFSETTO:-0700\n\
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
UID: %s\n\
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
END:VCALENDAR\n";


const char* TEST_VCAL_WITH_RRULE_1 = "BEGIN:VCALENDAR\n\
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
CREATED:20110613T085657Z\n\
DESCRIPTION:\n\n\
DTEND;TZID=\"GMT Standard Time\":20110613T163000\n\
DTSTAMP:20110613T085657Z\n\
DTSTART;TZID=\"GMT Standard Time\":20110613T160000\n\
LAST-MODIFIED:20110613T085657Z\n\
PRIORITY:5\n\
RRULE:FREQ=WEEKLY;COUNT=34;BYDAY=MO,WE,FR,SA\n\
SEQUENCE:0\n\
SUMMARY;LANGUAGE=en-gb:Outlook recur test (MWFSa\, end after 34)\n\
TRANSP:OPAQUE\n\
UID:040000008200E00074C5B7101A82E00800000000E0E40835B029CC01000000000000000\n\
	010000000D9B4DA1A66FA5F47AEF323BD095A49DA\n\
X-ALT-DESC;FMTTYPE=text/html:<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//E\n\
	N\">\n<HTML>\n<HEAD>\n<META NAME=\"Generator\" CONTENT=\"MS Exchange Server ve\n\
	rsion 08.01.0240.003\">\n<TITLE></TITLE>\n</HEAD>\n<BODY>\n<!-- Converted f\n\
	rom text/rtf format -->\n\n<P DIR=LTR><SPAN LANG=\"en-gb\"></SPAN></P>\n\n</\n\
	BODY>\n</HTML>\n\
X-MICROSOFT-CDO-BUSYSTATUS:BUSY\n\
X-MICROSOFT-CDO-IMPORTANCE:1\n\
X-MICROSOFT-DISALLOW-COUNTER:FALSE\n\
X-MS-OLK-CONFTYPE:0\n\
BEGIN:VALARM\n\
TRIGGER:-PT15M\n\
ACTION:DISPLAY\n\
DESCRIPTION:Reminder\n\
END:VALARM\n\
END:VEVENT\n\
END:VCALENDAR\n";


const char* TEST_VCAL_WITH_EXDATE_1 = "BEGIN:VCALENDAR\n\
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
CREATED:20110615T092123Z\n\
DESCRIPTION:\n\n\
DTEND;TZID=\"GMT Standard Time\":20110615T133000\n\
DTSTAMP:20110615T092123Z\n\
DTSTART;TZID=\"GMT Standard Time\":20110615T130000\n\
EXDATE;TZID=\"GMT Standard Time\":20110617T130000,20110618T130000\n\
LAST-MODIFIED:20110615T092123Z\n\
PRIORITY:5\n\
RRULE:FREQ=DAILY;COUNT=5\n\
SEQUENCE:0\n\
SUMMARY;LANGUAGE=en-gb:Outlook recur daily with exception\n\
TRANSP:OPAQUE\n\
UID:040000008200E00074C5B7101A82E00800000000B01F93EA452BCC01000000000000000\n\
	0100000001C3D980A6BFC0043926D50E6C3809BAB\n\
X-ALT-DESC;FMTTYPE=text/html:<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//E\n\
	N\">\n<HTML>\n<HEAD>\n<META NAME=\"Generator\" CONTENT=\"MS Exchange Server ve\n\
	rsion 08.01.0240.003\">\n<TITLE></TITLE>\n</HEAD>\n<BODY>\n<!-- Converted f\n\
	rom text/rtf format -->\n\n<P DIR=LTR><SPAN LANG=\"en-gb\"></SPAN></P>\n\n</\n\
	BODY>\n</HTML>\n\
X-MICROSOFT-CDO-BUSYSTATUS:BUSY\n\
X-MICROSOFT-CDO-IMPORTANCE:1\n\
X-MICROSOFT-DISALLOW-COUNTER:FALSE\n\
X-MS-OLK-CONFTYPE:0\n\
BEGIN:VALARM\n\
TRIGGER:-PT15M\n\
ACTION:DISPLAY\n\
DESCRIPTION:Reminder\n\
END:VALARM\n\
END:VEVENT\n\
END:VCALENDAR\n";
static void setMockNegTestGoodHttp(const gchar *mockedfile)
{
	guint status_code = 200;
	GArray *status_codes = g_array_new(FALSE, FALSE, sizeof(guint));
	const gchar *mocks[] = {mockedfile, 0};
 EasTestHandler *test_handler = eas_test_handler_new ();

	g_array_append_val(status_codes, status_code);    
    if (test_handler)
    {
		//eas_test_handler_add_mock_responses (test_handler, mocks, NULL);
        eas_test_handler_add_mock_responses (test_handler, mocks, status_codes);
        g_object_unref (test_handler);
        test_handler = NULL;
    }
	g_array_free(status_codes, TRUE);
}
static void negativeTestGetLatestCalendar (EasSyncHandler *sync_handler,
                                   gchar *sync_key_in,
                                   gchar **sync_key_out,
                                   GSList **created,
                                   GSList **updated,
                                   GSList **deleted,
                                   GError **error)
{
    gboolean ret = FALSE;
    gboolean more = FALSE;	
    mark_point();
	
    ret  = eas_sync_handler_get_items (sync_handler, sync_key_in, sync_key_out, EAS_ITEM_CALENDAR, NULL,
                                                & (*created),
                                                & (*updated),
                                                & (*deleted),
                                                & more,
                                               & (*error));
    mark_point();  
	 if ( (*error) == NULL)
	{
		fail_if (ret == TRUE, "%s","Function call should return FALSE");
	}
}
static gchar *
random_uid_new (void)
{
	static gint serial;
	static gchar *hostname;

	if (!hostname) {
		hostname = (gchar *) g_get_host_name ();
	}

	return g_strdup_printf ("%luA%luB%dC%s",
				(gulong) time (NULL),
				(gulong) getpid (),
				serial++,
				hostname);
}


static void testGetCalendarHandler (EasSyncHandler **sync_handler, const gchar* accountuid)
{
    // get a handle to the DBus interface and associate the account ID with
    // this object
    *sync_handler = eas_sync_handler_new (accountuid);

    // confirm that the handle object has been correctly setup
    fail_if (*sync_handler == NULL,
             "eas_mail_handler_new returns NULL when given a valid ID");
    fail_if ( (*sync_handler)->priv == NULL,
              "eas_mail_handler_new account ID object (EasEmailHandler *) member priv (EasEmailHandlerPrivate *) NULL");
}

static void testGetLatestCalendar (EasSyncHandler *sync_handler,
                                   gchar *sync_key_in,
                                   gchar **sync_key_out,
                                   GSList **created,
                                   GSList **updated,
                                   GSList **deleted,
                                   GError **error)
{
    gboolean ret = FALSE;
    gboolean more = FALSE;
    mark_point();
    ret  = eas_sync_handler_get_items (sync_handler, sync_key_in, sync_key_out, EAS_ITEM_CALENDAR, NULL,
                                                & (*created),
                                                & (*updated),
                                                & (*deleted),
                                                & more,
                                                & (*error));
    mark_point();
    // if the call to the daemon returned an error, report and drop out of the test
    fail_if (ret == FALSE, "%s", (error && *error ? (*error)->message : "NULL GError"));

    // the exchange server should increment the sync key and send back to the
    // client so that the client can track where it is with regard to sync.
    // therefore the key must not be zero as this is the seed value for this test
    fail_if (*sync_key_out==NULL, "Sync Key not updated by call the exchange server");
    fail_if (g_slist_length (*created) == 0, "list length =0");
    

}


START_TEST (test_cal)
{
//... calendar test case

}
END_TEST

START_TEST (test_get_sync_handler)
{
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;

    testGetCalendarHandler (&sync_handler, accountuid);

    g_object_unref (sync_handler);
}
END_TEST

START_TEST (test_get_latest_calendar_items)
{
    // This value needs to make sense in the daemon.  in the first instance
    // it should be hard coded to the value used by the daemon but later
    // there should be a mechanism for getting the value from the same place
    // that the daemon uses
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
     // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar* sync_key_in = NULL;
	gchar* sync_key_out = NULL;

    GError *error = NULL;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

   

    mark_point();
    // call into the daemon to get the folder hierarchy from the exchange server
    testGetLatestCalendar (sync_handler, sync_key_in, &sync_key_out, &created, &updated, &deleted, &error);

    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST

START_TEST (test_translate_ical_to_xml)
{
    EasItemInfo cal_info;// = eas_cal_info_new();
     xmlDocPtr doc = xmlNewDoc ((xmlChar *) "1.0");
    xmlNodePtr node;
    cal_info.data = (gchar*)TEST_VCALENDAR;
    cal_info.server_id = (gchar*)"1.0 (test value)";
    doc->children = xmlNewDocNode (doc, NULL, (xmlChar*) "temp_root", NULL);
    node = xmlNewChild (doc->children, NULL, (xmlChar*)"ApplicationData", NULL);

    fail_if (!eas_cal_info_translator_parse_request (doc, node, &cal_info),
             "Calendar translation failed (iCal => XML)");

//  g_object_unref(cal_info);
    xmlFree (doc); // TODO: need to explicitly free the node too??
}
END_TEST

START_TEST (test_eas_sync_handler_delete_cal)
{
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
    GError *error = NULL;
    gboolean testCalFound = FALSE;
     gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);


   

    testGetLatestCalendar (sync_handler,
                           folder_sync_key_in,
                         (gchar **)  &folder_sync_key_out,
                           &calitems_created,
                           &calitems_updated,
                           &calitems_deleted,
                           &error);

    // if the calitems_created list contains a calendar item
    if (calitems_created)
    {
        GSList *calitemToDel = NULL;
        EasItemInfo *calitem = NULL;
        gboolean rtn = FALSE;

        // get calendar item info for first calendar item in the folder
        calitem = (g_slist_nth (calitems_created, 0))->data;

        calitemToDel = g_slist_append (calitemToDel, calitem->server_id);

		g_free(folder_sync_key_in);
		folder_sync_key_in = g_strdup(folder_sync_key_out);
		g_free(folder_sync_key_out);
		folder_sync_key_out = NULL;
        // delete the first calendar item in the folder
        rtn = eas_sync_handler_delete_items (sync_handler, folder_sync_key_in,(gchar**) &folder_sync_key_out, EAS_ITEM_CALENDAR, "1", calitemToDel, &error);
        if (error)
        {
            fail_if (rtn == FALSE, "%s", error->message);
        }

        g_slist_free (calitemToDel);
		g_free(folder_sync_key_in);
		g_free(folder_sync_key_out);

        // free calendar item objects list before reusing
        g_slist_foreach (calitems_deleted, (GFunc) g_object_unref, NULL);
        g_slist_foreach (calitems_updated, (GFunc) g_object_unref, NULL);
        g_slist_foreach (calitems_created, (GFunc) g_object_unref, NULL);

        g_slist_free (calitems_deleted);
        g_slist_free (calitems_updated);
        g_slist_free (calitems_created);

        calitems_deleted = NULL;
        calitems_updated = NULL;
        calitems_created = NULL;

        testCalFound = TRUE;
    }

    // fail the test if there is no cal item as this means the
    // test has not exercised the code to get the email body as required by this test case
    fail_if (testCalFound == FALSE, "no cal item found");

    //  free calendar item objects in lists of calendar items objects
    g_slist_foreach (calitems_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (calitems_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (calitems_created, (GFunc) g_object_unref, NULL);

    g_object_unref (sync_handler);

}
END_TEST

START_TEST (test_eas_sync_handler_update_cal)
{
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
    GError *error = NULL;
    gboolean testCalFound = FALSE;
	gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);


    

    testGetLatestCalendar (sync_handler,
                           folder_sync_key_in,
                           &folder_sync_key_out,
                           &calitems_created,
                           &calitems_updated,
                           &calitems_deleted,
                           &error);

    // if the calitems_created list contains a calendar item
    if (calitems_created)
    {
        GSList *calitemToUpdate = NULL;
        EasItemInfo *calitem = NULL;
        EasItemInfo *updatedcalitem = NULL;
        gboolean rtn = FALSE;

        // get calendar item info for first calendar item in the folder
        calitem = (g_slist_nth (calitems_created, 0))->data;


        updatedcalitem = eas_item_info_new();
        updatedcalitem->server_id = g_strdup (calitem->server_id);
        updatedcalitem->data = g_strdup (TEST_VCALENDAR2);

        calitemToUpdate = g_slist_append (calitemToUpdate, updatedcalitem);
		g_free(folder_sync_key_in);
		folder_sync_key_in = g_strdup(folder_sync_key_out);
		g_free(folder_sync_key_out);
		folder_sync_key_out = NULL;
        // update the first calendar item in the folder
        rtn = eas_sync_handler_update_items (sync_handler, folder_sync_key_in, (gchar**)folder_sync_key_out, EAS_ITEM_CALENDAR, "1", calitemToUpdate, &error);
        if (error)
        {
            fail_if (rtn == FALSE, "%s", error->message);
        }

        g_slist_free (calitemToUpdate);

		g_free(folder_sync_key_in);
		g_free(folder_sync_key_out);

        // free calendar item objects list before reusing
        g_slist_foreach (calitems_deleted, (GFunc) g_object_unref, NULL);
        g_slist_foreach (calitems_updated, (GFunc) g_object_unref, NULL);
        g_slist_foreach (calitems_created, (GFunc) g_object_unref, NULL);

        g_slist_free (calitems_deleted);
        g_slist_free (calitems_updated);
        g_slist_free (calitems_created);

        calitems_deleted = NULL;
        calitems_updated = NULL;
        calitems_created = NULL;

        testCalFound = TRUE;
    }

    // fail the test if there is no cal item as this means the
    // test has not exercised the code to get the email body as required by this test case
    fail_if (testCalFound == FALSE, "no cal item found");

    //  free calendar item objects in lists of calendar items objects
    g_slist_foreach (calitems_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (calitems_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (calitems_created, (GFunc) g_object_unref, NULL);

    g_object_unref (sync_handler);
}
END_TEST


START_TEST (test_eas_sync_handler_add_cal)
{
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
    GError *error = NULL;
    gboolean testCalFound = FALSE;
    gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;
GSList *calitemToUpdate = NULL;
    EasItemInfo *updatedcalitem = NULL;
    gboolean rtn = FALSE;
    gchar *random_uid = random_uid_new();
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);


    

    testGetLatestCalendar (sync_handler,
                           folder_sync_key_in,
                           &folder_sync_key_out,
                           &calitems_created,
                           &calitems_updated,
                           &calitems_deleted,
                           &error);

    

    updatedcalitem = eas_item_info_new();
    updatedcalitem->client_id = random_uid; // Pass ownership
    updatedcalitem->data = g_strdup_printf("%s%s",(gchar*)TEST_VCALENDAR3, (gchar*)random_uid);

    g_debug("Random UID:     [%s]", random_uid);
    g_debug("TEST_VCALENDAR3 [%s]", updatedcalitem->data);

    calitemToUpdate = g_slist_append (calitemToUpdate, updatedcalitem);

	g_free(folder_sync_key_in);
	folder_sync_key_in = g_strdup(folder_sync_key_out);
	g_free(folder_sync_key_out);
	folder_sync_key_out = NULL;

    rtn = eas_sync_handler_add_items (sync_handler, folder_sync_key_in, (gchar**)&folder_sync_key_out, EAS_ITEM_CALENDAR, "1", calitemToUpdate, &error);
    if (error)
    {
        fail_if (rtn == FALSE, "%s", error->message);
    }
    updatedcalitem = calitemToUpdate->data;
    fail_if (updatedcalitem->server_id == NULL, "Not got new id for item");

    g_slist_free (calitemToUpdate);

	g_free(folder_sync_key_in);
	g_free(folder_sync_key_out);


    testCalFound = TRUE;

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_get_invalid_sync_key)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
 // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
	gchar* sync_key_out = NULL;
GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

   
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    

    

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarGetInvalidSyncKey.xml");
   // mock Test
	 // mock Test
	   eas_sync_handler_add_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error from test_cal_get_invalid_sync_key is  %s",dbus_g_error_get_name(error));
	fail_if(g_strcmp0 (dbus_g_error_get_name(error),         
	                   "org.meego.activesyncd.SyncError.INVALIDSYNCKEY"),  
	        "The Error returned by the server is not correct.");
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST 

START_TEST (test_cal_add_invalid_sync_key)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
	 // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    gchar* sync_key_out = NULL;

    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);
   
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarAddInvalidSyncKey.xml");
   // mock Test
	   rtn = eas_sync_handler_add_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(g_strcmp0 (dbus_g_error_get_name(error),         
	                   "org.meego.activesyncd.SyncError.INVALIDSYNCKEY"),  
	        "The Error returned by the server is not correct.");
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_delete_invalid_sync_key)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
    gint rtn = TRUE;
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    gchar* sync_key_out = NULL;
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

    // declare lists to hold the folder information returned by active sync   
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarDeleteinvalidSyncKey.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(g_strcmp0 (dbus_g_error_get_name(error),         
	                   "org.meego.activesyncd.SyncError.INVALIDSYNCKEY"),  
	        "The Error returned by the server is not correct.");
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_delete_invalid_server_id)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
// declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    gchar* sync_key_out = NULL;
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);
    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarDeleteInvalidServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(error,"Error is returned%s",dbus_g_error_get_name(error));
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST	
START_TEST (test_cal_delete_valid_invalid_server_id)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    gchar* sync_key_out = NULL;
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);
    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarDeleteValidInvalidServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(error,"Error is returned%s",dbus_g_error_get_name(error));
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_delete_valid_calendar_server_id)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
	    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    gchar* sync_key_out = NULL;
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarDeleteValidCalendarServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(error,"Error is returned%s",dbus_g_error_get_name(error));
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_update_invalid_server_id)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
// declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
	gchar* sync_key_out = NULL;
	GError *error = NULL;
	GSList *calitemToUpdate = NULL;
        EasItemInfo *updatedcalitem = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    	

	updatedcalitem = eas_item_info_new();
	updatedcalitem->server_id = g_strdup ("wrong");
	updatedcalitem->data = g_strdup (TEST_VCALENDAR2);

        calitemToUpdate = g_slist_append (calitemToUpdate, updatedcalitem);
    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarUpdateInvalidServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_update_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      calitemToUpdate,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(error,"Error is returned%s",dbus_g_error_get_name(error));
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_delete_crash)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
// declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
gchar* sync_key_out = NULL;
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarDeleteCrash.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (NULL,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_update_crash)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
// declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
gchar* sync_key_out = NULL;
	GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);
    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	
	
    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarUpdateCrash.xml");
   // mock Test
	   rtn = eas_sync_handler_update_items (NULL,
                                      "wrong",
                                      (gchar **)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	

    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
START_TEST (test_cal_add_crash)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
  // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    gchar* sync_key_out = NULL;
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

  
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	

    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarAddCrash.xml");
   // mock Test
	   rtn = eas_sync_handler_add_items (NULL,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST

START_TEST (test_cal_get_crash)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
// declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	gchar* sync_key_out = NULL;

    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

    


    mark_point();
   // set mock
	setMockNegTestGoodHttp("CalendarGetCrash.xml");
   // mock Test
	negativeTestGetLatestCalendar (NULL,
		                   (gchar*)"wrong",
		                   (gchar**)&sync_key_out,
		                   NULL,
		                   NULL,
		                   NULL,
		                   &error);
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST 
START_TEST (test_consume_response)
{

    
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
	gint rtn = TRUE;
  // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    
	gchar* sync_key_out = NULL;

    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);
  

    mark_point();
   // mock Test
	   rtn = eas_sync_handler_add_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CALENDAR,
                                      NULL,
                                      NULL,
                                      &error);
	
	g_debug("error is %s",dbus_g_error_get_name(error));
	fail_if(g_strcmp0 (dbus_g_error_get_name(error),         
	                   "org.meego.activesyncd.SyncError.INVALIDSYNCKEY"),  
	        "The Error returned by the server is not correct.");
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (sync_handler);
}
END_TEST
Suite* eas_libeascal_suite (void)
{
    Suite* s = suite_create ("libeascal");

    /* tc_libeascal test case */
    TCase *tc_libeascal = tcase_create ("core");
    suite_add_tcase (s, tc_libeascal);
	if(getenv ("EAS_USE_MOCKS") && (atoi (g_getenv ("EAS_USE_MOCKS")) >= 1))
    	{
		tcase_add_test (tc_libeascal, test_cal_get_invalid_sync_key);
		tcase_add_test (tc_libeascal, test_cal_add_invalid_sync_key);
		tcase_add_test (tc_libeascal, test_cal_delete_invalid_sync_key);
		tcase_add_test (tc_libeascal, test_cal_delete_invalid_server_id);
		tcase_add_test (tc_libeascal, test_cal_delete_valid_invalid_server_id);
		tcase_add_test (tc_libeascal, test_cal_delete_valid_calendar_server_id);
		tcase_add_test (tc_libeascal, test_cal_update_invalid_server_id); 

		// crash functions will not consume mocked response.Dummy function call after each test will consume the mocked response.	
		tcase_add_test (tc_libeascal, test_cal_delete_crash);
		tcase_add_test (tc_libeascal, test_consume_response);				
		tcase_add_test (tc_libeascal, test_cal_update_crash);
		tcase_add_test (tc_libeascal, test_consume_response);	
		tcase_add_test (tc_libeascal, test_cal_add_crash);
		tcase_add_test (tc_libeascal, test_consume_response);		
		tcase_add_test (tc_libeascal, test_cal_get_crash);
		tcase_add_test (tc_libeascal, test_consume_response);
	
	}
//    tcase_add_test (tc_libeascal, test_cal);
//    tcase_add_test (tc_libeascal, test_get_sync_handler);
 //   tcase_add_test (tc_libeascal, test_get_latest_calendar_items);
//    tcase_add_test (tc_libeascal, test_translate_ical_to_xml);
//    tcase_add_test (tc_libeascal, test_eas_sync_handler_delete_cal);
//    tcase_add_test (tc_libeascal, test_eas_sync_handler_update_cal);
//    tcase_add_test (tc_libeascal, test_eas_sync_handler_add_cal);

    return s;
}
