#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>

#include "eas_test_user.h"

#include "../libeassync/src/libeassync.h"
#include "../libeassync/src/eas-item-info.h"
#include "../eas-daemon/libeas/eas-con-info-translator.h"

gchar * g_contacts_account_id = TEST_ACCOUNT_ID;


const char* TEST_VCARD_01 = "BEGIN:vCard\n\
VERSION:3.0\n\
FN:John Stevenson\n\
N:Stevenson;John;Philip,Paul;Dr.;Jr.,M.D.,A.C.P.\n\
ORG:Lotus Development Corporation\n\
ADR;TYPE=WORK,POSTAL,PARCEL:;;6544 Battleford Drive\
 ;Raleigh;NC;27613-3502;U.S.A.\n\
TEL;TYPE=VOICE,MSG,WORK:+1-919-676-9515\n\
TEL;TYPE=FAX,WORK:+1-919-676-9564\n\
EMAIL;TYPE=INTERNET,PREF:Frank_Dawson@Lotus.com\n\
EMAIL;TYPE=INTERNET:fdawson@earthlink.net\n\
URL:http://home.earthlink.net/~fdawson\n\
BDAY:1996-04-15\n\
PHOTO;VALUE=URL;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\
PHOTO;ENCODING=b;TYPE=JPEG:MIICajCCAdOgAwIBAgICBEUwDQYJKoZIhvcN\
AQEEBQAwdzELMAkGA1UEBhMCVVMxLDAqBgNVBAoTI05ldHNjYXBlIENvbW11bm\
ljYXRpb25zIENvcnBvcmF0aW9uMRwwGgYDVQQLExNJbmZvcm1hdGlvbiBTeXN0\n\
TITLE:Shrimp Man\n\
NOTE:This fax number is operational 0800 to 1715\n\
END:vCard";


const char* TEST_VCARD = "BEGIN:vCard\n\
VERSION:3.0\n\
N:Stevenson;John;Philip,Paul;Dr.;Jr.,M.D.,A.C.P.\n\
ADR;TYPE=WORK,POSTAL,PARCEL:;;6544 Battleford Drive \
;Raleigh;NC;27613-3502;U.S.A.\n\
ADR;TYPE=HOME,POSTAL,PARCEL:;;6544 Battleford Drive \
;Raleigh;NC;27613-3502;U.S.A.\n\
ADR;TYPE=HOME,POSTAL,PARCEL:;;6544 Battleford Drive \
;Raleigh;NC;27613-3502;U.S.A.\n\
END:vCard\n";


/*vCard from ActiveSync Response*/
const char* TEST_VCARD_FROM_AS ="BEGIN:VCARD\n\
VERSION:3.0\r\n\
ROLE:bricky\n\
TEL;TYPE=WORK:6574165464165\n\
TEL;TYPE=WORK:0161 111 2222\n\
TEL;TYPE=PAGER:pager #\n\
TEL;TYPE=CAR:645646346544\n\
ADR;TYPE=OTHER:other city;other country;other post code;other state;other street\n\
ORG:Company 1\n\
TEL;TYPE=CELL:0777 333 4444\n\
TEL;TYPE=HOME:567573\n\
TEL;TYPE=HOME:0161 222 3333\n\
TEL;TYPE=HOME,FAX:ahhhh\, he'll save everyone of us\n\
ADR;TYPE=HOME:Wilmslow;UK;SK9 1AY;Cheshire;Market House Church Street\n\
N:bob;the;builder\n\
EMAIL:\"bob.the.builder@cstylianou.com\" <bob.the.builder@cstylianou.com>\n\
ADR;TYPE=WORK:Business City;Business Country;Business Postal Code;Business \n\
 State;Business Address Street\n\
URL:www.bobthebuilder.com\n\
END:VCARD\n";

/*vCard from Evolution*/
const char* TEST_VCARD_FROM_EVO = "BEGIN:VCARD\n\
VERSION:3.0\n\
REV:2011-05-23T15:08:32Z\n\
UID:pas-id-4DDA786E00000000\n\
N:Abes;Brahim;;;\n\
FN:Brahim Abes\n\
NICKNAME:brahim\n\
X-EVOLUTION-FILE-AS:Abes\, Brahim\n\
URL:\n\
ADR;TYPE=HOME:;;;;;;\n\
END:VCARD\n";

static void testGetContactsHandler (EasSyncHandler **sync_handler, const gchar* accountuid)
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

static void testGetLatestContacts (EasSyncHandler *sync_handler,
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
    ret  = eas_sync_handler_get_items (sync_handler, sync_key_in, sync_key_out, EAS_ITEM_CONTACT, NULL,
                                                & (*created),
                                                & (*updated),
                                                & (*deleted),
                                                & more,
                                                & (*error));
    mark_point();
    // if the call to the daemon returned an error, report and drop out of the test
    if ( (*error) != NULL)
    {
        fail_if (ret == FALSE, "%s", (*error)->message);
    }

    // the exchange server should increment the sync key and send back to the
    // client so that the client can track where it is with regard to sync.
    // therefore the key must not be zero as this is the seed value for this test
    fail_if (*sync_key_out==NULL, "Sync Key not updated by call the exchange server");
    fail_if (g_slist_length (*created) == 0, "list length =0");
}


START_TEST (test_eas_sync_handler_add_con)
{
    EasSyncHandler *sync_handler = NULL;
    GError *error = NULL;
    gboolean testCalFound = FALSE;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetContactsHandler (&sync_handler, g_contacts_account_id);

    gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *conitems_created = NULL; //receives a list of EasMails
    GSList *conitems_updated = NULL;
    GSList *conitems_deleted = NULL;

    testGetLatestContacts (sync_handler,
                           folder_sync_key_in,
                           &folder_sync_key_out,
                           &conitems_created,
                           &conitems_updated,
                           &conitems_deleted,
                           &error);

	
    GSList *conitemToUpdate = NULL;
    EasItemInfo *updatedconitem = NULL;
    gboolean rtn = FALSE;


    updatedconitem = eas_item_info_new();
    updatedconitem->client_id = g_strdup ("sdfasdfsdf");
    updatedconitem->data = g_strdup (TEST_VCARD_01);//TEST_VCARD

    conitemToUpdate = g_slist_append (conitemToUpdate, updatedconitem);

	
	g_free(folder_sync_key_in);
	folder_sync_key_in = g_strdup(folder_sync_key_out);
	g_free(folder_sync_key_out);
	folder_sync_key_out = NULL;

    rtn = eas_sync_handler_add_items (sync_handler, folder_sync_key_in, folder_sync_key_out, EAS_ITEM_CONTACT, "1", conitemToUpdate, &error);
    if (error)
    {
        fail_if (rtn == FALSE, "%s", error->message);
    }
    updatedconitem = conitemToUpdate->data;
    fail_if (updatedconitem->server_id == NULL, "Not got new id for item");

    g_slist_free (conitemToUpdate);

	g_free(folder_sync_key_in);
	g_free(folder_sync_key_out);


    testCalFound = TRUE;

    g_object_unref (sync_handler);
}
END_TEST



START_TEST (test_get_latest_contacts_items)
{
    // This value needs to make sense in the daemon.  in the first instance
    // it should be hard coded to the value used by the daemon but later
    // there should be a mechanism for getting the value from the same place
    // that the daemon uses
	EasSyncHandler *sync_handler = NULL;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetContactsHandler (&sync_handler, g_contacts_account_id);

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
    testGetLatestContacts (sync_handler, sync_key_in, &sync_key_out, &created, &updated, &deleted, &error);

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


START_TEST (test_translate_vcard_to_xml)
{
	EasItemInfo contactInfo;// = eas_cal_info_new();
    contactInfo.data = TEST_VCARD_01; //TEST_VCARD
    contactInfo.server_id = "1.0 (test value)";

    xmlDocPtr doc = xmlNewDoc ("1.0");
    doc->children = xmlNewDocNode (doc, NULL, "temp_root", NULL);
    xmlNodePtr node = xmlNewChild (doc->children, NULL, "ApplicationData", NULL);

    fail_if (!eas_con_info_translator_parse_request (doc, node, &contactInfo),
             "Calendar translation failed (iCal => XML)");

//  g_object_unref(contactInfo);
    xmlFree (doc); // TODO: need to explicitly free the node too??
}
END_TEST

Suite* eas_libeascon_suite (void)
{
    Suite* s = suite_create ("libeascon");

    /* tc_libeascon test case */
    TCase *tc_libeascon = tcase_create ("core");
    suite_add_tcase (s, tc_libeascon);

    //tcase_add_test (tc_libeascon, test_get_sync_handler);
	//tcase_add_test (tc_libeascon, test_get_latest_contacts_items);
   tcase_add_test (tc_libeascon, test_translate_vcard_to_xml);
    //tcase_add_test (tc_libeascon, test_eas_sync_handler_delete_con);
    //tcase_add_test (tc_libeascon, test_eas_sync_handler_update_con);
    //tcase_add_test (tc_libeascon, test_eas_sync_handler_add_con);
    return s;
}