#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "eas_test_user.h"

#include "../libeassync/src/libeassync.h"
#include "../libeassync/src/eas-item-info.h"
#include "../eas-daemon/libeas/eas-cal-info-translator.h"

static gchar * g_account_id = TEST_ACCOUNT_ID;

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
    mark_point();
	gboolean more = FALSE;
    ret  = eas_sync_handler_get_items (sync_handler, sync_key_in, sync_key_out, EAS_ITEM_CALENDAR, "1",
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
    EasItemInfo *cal = (*created)->data;

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

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);

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
    cal_info.data = TEST_VCALENDAR;
    cal_info.server_id = "1.0 (test value)";

    xmlDocPtr doc = xmlNewDoc ("1.0");
    doc->children = xmlNewDocNode (doc, NULL, "temp_root", NULL);
    xmlNodePtr node = xmlNewChild (doc->children, NULL, "ApplicationData", NULL);

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

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);


    gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;

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
        rtn = eas_sync_handler_delete_items (sync_handler, folder_sync_key_in, folder_sync_key_out, EAS_ITEM_CALENDAR, "1", calitemToDel, &error);
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

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);


    gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;

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
        rtn = eas_sync_handler_update_items (sync_handler, folder_sync_key_in, folder_sync_key_out, EAS_ITEM_CALENDAR, "1", calitemToUpdate, &error);
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

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetCalendarHandler (&sync_handler, accountuid);


    gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; //receives a list of EasMails
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;

    testGetLatestCalendar (sync_handler,
                           folder_sync_key_in,
                           &folder_sync_key_out,
                           &calitems_created,
                           &calitems_updated,
                           &calitems_deleted,
                           &error);

    GSList *calitemToUpdate = NULL;
    EasItemInfo *updatedcalitem = NULL;
    gboolean rtn = FALSE;



    updatedcalitem = eas_item_info_new();
    updatedcalitem->client_id = g_strdup ("sdfasdfsdf");
    updatedcalitem->data = g_strdup (TEST_VCALENDAR3);

    calitemToUpdate = g_slist_append (calitemToUpdate, updatedcalitem);

	
	g_free(folder_sync_key_in);
	folder_sync_key_in = g_strdup(folder_sync_key_out);
	g_free(folder_sync_key_out);
	folder_sync_key_out = NULL;

    rtn = eas_sync_handler_add_items (sync_handler, folder_sync_key_in, folder_sync_key_out, EAS_ITEM_CALENDAR, "1", calitemToUpdate, &error);
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



Suite* eas_libeascal_suite (void)
{
    Suite* s = suite_create ("libeascal");

    /* tc_libeascal test case */
    TCase *tc_libeascal = tcase_create ("core");
    suite_add_tcase (s, tc_libeascal);

    tcase_add_test (tc_libeascal, test_cal);
    tcase_add_test (tc_libeascal, test_get_sync_handler);
    tcase_add_test (tc_libeascal, test_get_latest_calendar_items);
    tcase_add_test (tc_libeascal, test_translate_ical_to_xml);
    tcase_add_test (tc_libeascal, test_eas_sync_handler_delete_cal);
    tcase_add_test (tc_libeascal, test_eas_sync_handler_update_cal);
    tcase_add_test (tc_libeascal, test_eas_sync_handler_add_cal);

    return s;
}
