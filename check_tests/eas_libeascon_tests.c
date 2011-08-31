#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <unistd.h>

#include "eas_test_user.h"

#include "../libeassync/src/libeassync.h"
#include "../libeassync/src/eas-item-info.h"
#include "../eas-daemon/libeas/eas-con-info-translator.h"
 #include "../libeastest/src/libeastest.h"
gchar * g_account_id = (gchar *)TEST_ACCOUNT_ID;
Suite* eas_libeascon_suite (void);
const char* TEST_VCARD_FROM_EVO = "BEGIN:VCARD\n\
VERSION:3.0\n\
LABEL;TYPE=OTHER:myOtherAddress \nmyOtherCity, myOtherState\nmyOtherZip\nm\
yOtherPOBox\nmyOtherCountry\n\
ADR;TYPE=OTHER:myOtherPOBox;;myOtherAddress ;myOtherCity;myOtherState;myOth\
erZip;myOtherCountry\n\
LABEL;TYPE=HOME:myHomeAddress \nmyHomeCity, myHomeState\nmyHomeZip\nmyHome\
POBox\nmyHomeCountry\n\
ADR;TYPE=HOME:myHomePOBox;;myHomeAddress ;myHomeCity;myHomeState;myHomeZip;\
myHomeCountry\n\
LABEL;TYPE=WORK:myWorkAddress\nmyWorkCity, myWorkState\nmyWorkZip\nmyWorkP\
OBox\nmyWorkCountry\n\
ADR;TYPE=WORK:myWorkPOBox;;myWorkAddress;myWorkCity;myWorkState;myWorkZip;m\
yWorkCountry\n\
X-ICQ;X-EVOLUTION-UI-SLOT=4;TYPE=HOME:myICQ\n\
X-MSN;X-EVOLUTION-UI-SLOT=3;TYPE=HOME:myMSN\n\
X-YAHOO;X-EVOLUTION-UI-SLOT=2;TYPE=HOME:myYahoo\n\
X-AIM;X-EVOLUTION-UI-SLOT=1;TYPE=HOME:myAIM\n\
TEL;X-EVOLUTION-UI-SLOT=4;TYPE=HOME,VOICE:20123456789\n\
TEL;X-EVOLUTION-UI-SLOT=3;TYPE=WORK,FAX:40123456789\n\
TEL;X-EVOLUTION-UI-SLOT=2;TYPE=CELL:30123456789\n\
TEL;X-EVOLUTION-UI-SLOT=1;TYPE=WORK,VOICE:10123456789\n\
EMAIL;X-EVOLUTION-UI-SLOT=4;TYPE=OTHER:myemail@Other.com\n\
EMAIL;X-EVOLUTION-UI-SLOT=3;TYPE=HOME:myemail2@Home.com\n\
EMAIL;X-EVOLUTION-UI-SLOT=2;TYPE=HOME:myemail@Home.com\n\
EMAIL;X-EVOLUTION-UI-SLOT=1;TYPE=WORK:myemail@Work.com\n\
PHOTO;ENCODING=b;TYPE=\"X-EVOLUTION-UNKNOWN\":/9j/4AAQSkZJRgABAQEASABIAAD/4QM\
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\
//9k=\n\
X-MOZILLA-HTML:FALSE\n\
X-EVOLUTION-VIDEO-URL:myVideoChat\n\
FBURL:myFreeOrBusy\n\
CALURI:myCalendar\n\
ROLE:myProfession\n\
X-COUCHDB-APPLICATION-ANNOTATIONS:{ \"Evolution\" : { \"revision\" : \"2011-07-1\
0T00:28:58Z\" } }\n\
REV:2011-07-10T00:28:58Z\n\
X-EVOLUTION-ANNIVERSARY:2011-07-13\n\
BDAY:1995-07-13\n\
X-EVOLUTION-BLOG-URL;X-COUCHDB-UUID=\"8930d503-0794-454b-9c4e-f30b093654fc\":\
myWebLog\n\
URL;X-COUCHDB-UUID=\"84fb728f-9e33-4dbb-af66-e62dedfbcc1c\":http://www.myHome\
Page.com\n\
NOTE:myNotes this is a test contact for the Contacts translator\n\
CATEGORIES:Competition,Favorites,Birthday,Anniversary,Business\n\
X-EVOLUTION-ASSISTANT:myAssistant\n\
X-EVOLUTION-MANAGER:myManager\n\
TITLE:myTitle\n\
ORG:myCompany;myDepartment;myOffice\n\
X-EVOLUTION-SPOUSE:mySpouse\n\
NICKNAME:myNickname\n\
FN:Mr. myFirstName myMiddleName myLastName Sr.\n\
X-EVOLUTION-FILE-AS:myLastName, myFirstName\n\
N:myLastName;myFirstName;myMiddleName;Mr.;Sr.\n\
UID:pas-id-4E1E1CFD00000000\n\
X-COUCHDB-REVISION:3-27f3c9f29571145454ce17f436b948c3\n\
END:VCARD\n";

/* removed the PHOTO, changed the NOTE and N*/
const char* TEST_VCARD_FROM_EVO_UPDATED = "BEGIN:VCARD\n\
VERSION:3.0\n\
LABEL;TYPE=OTHER:myOtherAddress \nmyOtherCity\, myOtherState\nmyOtherZip\nm\
yOtherPOBox\nmyOtherCountry\n\
ADR;TYPE=OTHER:myOtherPOBox;;myOtherAddress ;myOtherCity;myOtherState;myOth\
erZip;myOtherCountry\n\
LABEL;TYPE=HOME:myHomeAddress \nmyHomeCity\, myHomeState\nmyHomeZip\nmyHome\
POBox\nmyHomeCountry\n\
ADR;TYPE=HOME:myHomePOBox;;myHomeAddress ;myHomeCity;myHomeState;myHomeZip;\
myHomeCountry\n\
LABEL;TYPE=WORK:myWorkAddress\nmyWorkCity\, myWorkState\nmyWorkZip\nmyWorkP\
OBox\nmyWorkCountry\n\
ADR;TYPE=WORK:myWorkPOBox;;myWorkAddress;myWorkCity;myWorkState;myWorkZip;m\
yWorkCountry\n\
X-ICQ;X-EVOLUTION-UI-SLOT=4;TYPE=HOME:myICQ\n\
X-MSN;X-EVOLUTION-UI-SLOT=3;TYPE=HOME:myMSN\n\
X-YAHOO;X-EVOLUTION-UI-SLOT=2;TYPE=HOME:myYahoo\n\
X-AIM;X-EVOLUTION-UI-SLOT=1;TYPE=HOME:myAIM\n\
TEL;X-EVOLUTION-UI-SLOT=4;TYPE=HOME,VOICE:20123456789\n\
TEL;X-EVOLUTION-UI-SLOT=3;TYPE=WORK,FAX:40123456789\n\
TEL;X-EVOLUTION-UI-SLOT=2;TYPE=CELL:30123456789\n\
TEL;X-EVOLUTION-UI-SLOT=1;TYPE=WORK,VOICE:10123456789\n\
EMAIL;X-EVOLUTION-UI-SLOT=4;TYPE=OTHER:myemail@Other.com\n\
EMAIL;X-EVOLUTION-UI-SLOT=3;TYPE=HOME:myemail2@Home.com\n\
EMAIL;X-EVOLUTION-UI-SLOT=2;TYPE=HOME:myemail@Home.com\n\
EMAIL;X-EVOLUTION-UI-SLOT=1;TYPE=WORK:myemail@Work.com\n\
X-MOZILLA-HTML:FALSE\n\
X-EVOLUTION-VIDEO-URL:myVideoChat\n\
FBURL:myFreeOrBusy\n\
CALURI:myCalendar\n\
ROLE:myProfession\n\
X-COUCHDB-APPLICATION-ANNOTATIONS:{ \"Evolution\" : { \"revision\" : \"2011-07-1\
0T00:28:58Z\" } }\n\
REV:2011-07-10T00:28:58Z\n\
X-EVOLUTION-ANNIVERSARY:2011-07-13\n\
BDAY:1995-07-13\n\
X-EVOLUTION-BLOG-URL;X-COUCHDB-UUID=\"8930d503-0794-454b-9c4e-f30b093654fc\":\
myWebLog\n\
URL;X-COUCHDB-UUID=\"84fb728f-9e33-4dbb-af66-e62dedfbcc1c\":http://www.myHome\
Page.com\n\
NOTE:Update Contact test\n\
CATEGORIES:Competition,Favorites,Birthday,Anniversary,Business\n\
X-EVOLUTION-ASSISTANT:myAssistant\n\
X-EVOLUTION-MANAGER:myManager\n\
TITLE:myTitle\n\
ORG:myCompany;myDepartment;myOffice\n\
X-EVOLUTION-SPOUSE:mySpouse\n\
NICKNAME:myNickname\n\
FN:Mr. myFirstName myMiddleName myLastName Sr.\n\
X-EVOLUTION-FILE-AS:myLastName\, myFirstName\n\
N:UmyLastName;UmyFirstName;UmyMiddleName;Mr.;Sr.\n\
UID:pas-id-4E1E1CFD00000000\n\
X-COUCHDB-REVISION:3-27f3c9f29571145454ce17f436b948c3\n\
END:VCARD\n";


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
    gboolean more = FALSE;
    mark_point();
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
//    fail_if (g_slist_length (*created) == 0, "list length =0"); /* list can be empty should we fail!?*/
}
static void negativeTestGetLatestContacts (EasSyncHandler *sync_handler,
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
    ret  = eas_sync_handler_get_items (sync_handler, sync_key_in, sync_key_out, EAS_ITEM_CONTACT, NULL,
                                                & (*created),
                                                & (*updated),
                                                & (*deleted),
                                                & more,
                                                & (*error));
    mark_point();
    // if the call to the daemon returned an error, report and drop out of the test
    if ( (*error) == NULL)
    {
        fail_if (ret == TRUE, "%s","Function call should return FALSE");
    }

  
}                                   

START_TEST (test_eas_sync_handler_add_con)
{
  	const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
    GError *error = NULL;
    gboolean testCalFound = FALSE;
     gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
    GSList *calitems_created = NULL; 
    GSList *calitems_updated = NULL;
    GSList *calitems_deleted = NULL;
    GSList *calitemToUpdate = NULL;
    EasItemInfo *updatedcalitem = NULL;
    gboolean rtn = FALSE;
    gchar *random_uid = random_uid_new();
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetContactsHandler (&sync_handler, accountuid);


    testGetLatestContacts (sync_handler,
                           folder_sync_key_in,
                           &folder_sync_key_out,
                           &calitems_created,
                           &calitems_updated,
                           &calitems_deleted,
                           &error);

    

    updatedcalitem = eas_item_info_new();
    updatedcalitem->client_id = random_uid; // Pass ownership
    updatedcalitem->data = g_strdup_printf("%s%s",TEST_VCARD_FROM_EVO, random_uid);//TEST_VCARD

    g_debug("Random UID:     [%s]", random_uid);
    g_debug("TEST_VCARD_FROM_EVO	     [%s]", updatedcalitem->data);

    calitemToUpdate = g_slist_append (calitemToUpdate, updatedcalitem);

	g_free(folder_sync_key_in);
	folder_sync_key_in = g_strdup(folder_sync_key_out);
	g_free(folder_sync_key_out);
	folder_sync_key_out = NULL;

    rtn = eas_sync_handler_add_items (sync_handler,
                                      folder_sync_key_in,
                                      (gchar**)&folder_sync_key_out,
                                      EAS_ITEM_CONTACT,
                                      NULL,
                                      calitemToUpdate,
                                      &error);
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



START_TEST (test_get_latest_contacts_items)
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
    testGetContactsHandler (&sync_handler, accountuid);


    mark_point();
    // call into the daemon to get the folder hierarchy from the exchange server
    testGetLatestContacts ( sync_handler, sync_key_in, &sync_key_out, &created, &updated, &deleted, &error);

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
	xmlDocPtr doc;
	xmlNodePtr node;
    contactInfo.data = (gchar*)TEST_VCARD_FROM_EVO;
    contactInfo.server_id = (gchar*)"1.0 (test value)";

    doc = xmlNewDoc ((xmlChar*)"1.0");
    doc->children = xmlNewDocNode (doc, NULL, (xmlChar*)"temp_root", NULL);
    node = xmlNewChild (doc->children, NULL, (xmlChar*)"ApplicationData", NULL);

    fail_if (!eas_con_info_translator_parse_request (doc, node, &contactInfo),
             "Calendar translation failed (iCal => XML)");

//  g_object_unref(contactInfo);
    xmlFree (doc); // TODO: need to explicitly free the node too??
}
END_TEST

START_TEST (test_eas_sync_handler_delete_all_created_con)
{
	const char* accountuid = g_account_id;
	EasSyncHandler *sync_handler = NULL;
	GError *error = NULL;
	gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
	GSList *contactitems_created = NULL, *l = NULL;
	GSList *contactitems_updated = NULL;
	GSList *contactitems_deleted = NULL;
	GSList *conitemToDel = NULL;
	EasItemInfo *conitem = NULL;
	// get a handle to the DBus interface and associate the account ID with
	// this object
	testGetContactsHandler (&sync_handler, accountuid);

	
	testGetLatestContacts (sync_handler,
		                   folder_sync_key_in,
		                   &folder_sync_key_out,
		                   &contactitems_created,
		                   &contactitems_updated,
		                   &contactitems_deleted,
		                   &error);

	g_free(folder_sync_key_in); folder_sync_key_in = NULL;
	
	/* we are only interested in the created contacts so get rid of updated + deleted */
    g_slist_foreach (contactitems_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (contactitems_updated, (GFunc) g_object_unref, NULL);
    g_slist_free (contactitems_deleted); contactitems_deleted = NULL;
    g_slist_free (contactitems_updated); contactitems_updated = NULL;

	
	for(l= contactitems_created; l ; l= l->next){
		conitem = l->data;
		conitemToDel = g_slist_append (conitemToDel, conitem->server_id);
	}

	// if the contactitems_created list contains a contact item
	if (conitemToDel){
		gboolean rtn = FALSE;

		folder_sync_key_in = g_strdup(folder_sync_key_out);
		g_free(folder_sync_key_out); folder_sync_key_out = NULL;
		
		// delete all contacts items in the folder
		rtn = eas_sync_handler_delete_items (sync_handler, folder_sync_key_in,
				                             (gchar**)&folder_sync_key_out,
				                             EAS_ITEM_CONTACT, NULL,
				                             conitemToDel,
				                             &error);
		if (error)
		{
			fail_if (rtn == FALSE, "%s", error->message);
		}

		g_free(folder_sync_key_in); folder_sync_key_in = NULL;
		g_free(folder_sync_key_out); folder_sync_key_out = NULL;

		g_slist_foreach (conitemToDel, (GFunc) g_object_unref, NULL);
		g_slist_free (conitemToDel); conitemToDel = NULL;

	}

	if(folder_sync_key_out)
		g_free(folder_sync_key_out); folder_sync_key_out = NULL;

	// free contacts item objects list
	g_slist_foreach (contactitems_created, (GFunc) g_object_unref, NULL);
	g_slist_free (contactitems_created); contactitems_created = NULL;

	g_object_unref (sync_handler);
}
END_TEST


START_TEST (test_eas_sync_handler_update_con)
{
    const char* accountuid = g_account_id;
    EasSyncHandler *sync_handler = NULL;
    GError *error = NULL;


    // get a handle to the DBus interface and associate the account ID with
    // this object

    gchar* folder_sync_key_in = NULL;
	gchar* folder_sync_key_out = NULL;
	GSList *contactitems_created = NULL, *l = NULL;
	GSList *contactitems_updated = NULL;
	GSList *contactitems_deleted = NULL;
	GSList *conItemToUpdate = NULL;
	EasItemInfo *conItem = NULL;
	EasItemInfo *updatedConItem = NULL;

        testGetContactsHandler (&sync_handler, accountuid);

	testGetLatestContacts (sync_handler,
		                   folder_sync_key_in,
		                   &folder_sync_key_out,
		                   &contactitems_created,
		                   &contactitems_updated,
		                   &contactitems_deleted,
		                   &error);
	
	g_free(folder_sync_key_in); folder_sync_key_in = NULL;
	
	/* we are only interested in the created contacts so get rid of updated + deleted */
    g_slist_foreach (contactitems_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (contactitems_updated, (GFunc) g_object_unref, NULL);
    g_slist_free (contactitems_deleted); contactitems_deleted = NULL;
    g_slist_free (contactitems_updated); contactitems_updated = NULL;

	
	for(l= contactitems_created; l ; l= l->next){
		conItem = l->data;

        updatedConItem = eas_item_info_new();
        updatedConItem->server_id = g_strdup (conItem->server_id);
        updatedConItem->data = g_strdup (TEST_VCARD_FROM_EVO_UPDATED);
		conItemToUpdate = g_slist_append (conItemToUpdate, updatedConItem);
	}

	// if the contactitems_created list contains a contact item
	if (conItemToUpdate){
		gboolean rtn = FALSE;

		folder_sync_key_in = g_strdup(folder_sync_key_out);
		g_free(folder_sync_key_out); folder_sync_key_out = NULL;
		
		// update all contacts items in the folder
        rtn = eas_sync_handler_update_items (sync_handler,
                                             folder_sync_key_in,
                                             (gchar**)&folder_sync_key_out,
                                             EAS_ITEM_CONTACT,
                                             NULL,
                                             conItemToUpdate,
                                             &error);
		if (error)
		{
			fail_if (rtn == FALSE, "%s", error->message);
		}

		g_free(folder_sync_key_in); folder_sync_key_in = NULL;
		g_free(folder_sync_key_out); folder_sync_key_out = NULL;

		g_slist_foreach (conItemToUpdate, (GFunc) g_object_unref, NULL);
		g_slist_free (conItemToUpdate); conItemToUpdate = NULL;
	}

	if(folder_sync_key_out)
		g_free(folder_sync_key_out); folder_sync_key_out = NULL;

	// free contacts item objects list
	g_slist_foreach (contactitems_created, (GFunc) g_object_unref, NULL);
	g_slist_free (contactitems_created); contactitems_created = NULL;

	g_object_unref (sync_handler);
}
END_TEST

START_TEST (test_con_get_invalid_sync_key)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactGetInvalidSyncKey.xml");
   // mock Test
	eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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

START_TEST (test_con_add_invalid_sync_key)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactAddInvalidSyncKey.xml");
   // mock Test
	   rtn = eas_sync_handler_add_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
START_TEST (test_con_delete_invalid_sync_key)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactDeleteinvalidSyncKey.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
START_TEST (test_con_delete_invalid_server_id)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactDeleteInvalidServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
START_TEST (test_con_delete_valid_invalid_server_id)
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
    testGetContactsHandler (&sync_handler, accountuid);
    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactDeleteValidInvalidServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
START_TEST (test_con_delete_valid_calendar_server_id)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactDeleteValidCalendarServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (sync_handler,
                                      "wrong",
                                     (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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

START_TEST (test_con_update_invalid_server_id)
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
	GSList *conItemToUpdate = NULL;
	EasItemInfo *updatedConItem = NULL;
// get a handle to the DBus interface and associate the account ID with
    // this object
    testGetContactsHandler (&sync_handler, accountuid);

  	updatedConItem = eas_item_info_new();
        updatedConItem->server_id = g_strdup ("wrong");
        updatedConItem->data = g_strdup (TEST_VCARD_FROM_EVO_UPDATED);
		conItemToUpdate = g_slist_append (conItemToUpdate, updatedConItem);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactUpdateInvalidServerId.xml");
   // mock Test
	   rtn = eas_sync_handler_update_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
                                      NULL,
                                      conItemToUpdate,
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
START_TEST (test_con_delete_crash)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactDeleteCrash.xml");
   // mock Test
	   rtn = eas_sync_handler_delete_items (NULL,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
START_TEST (test_con_update_crash)
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
	GSList *conItemToUpdate = NULL;
	EasItemInfo *updatedConItem = NULL;

   // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetContactsHandler (&sync_handler, accountuid);

  	updatedConItem = eas_item_info_new();
        updatedConItem->server_id = g_strdup ("wrong");
        updatedConItem->data = g_strdup (TEST_VCARD_FROM_EVO_UPDATED);
		conItemToUpdate = g_slist_append (conItemToUpdate, updatedConItem);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactUpdateCrash.xml");
   // mock Test
	   rtn = eas_sync_handler_update_items (NULL,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
START_TEST (test_con_add_crash)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactAddCrash.xml");
   // mock Test
	   rtn = eas_sync_handler_add_items (NULL,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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

START_TEST (test_con_get_crash)
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
    testGetContactsHandler (&sync_handler, accountuid);

    mark_point();
   // set mock
	setMockNegTestGoodHttp("ContactGetCrash.xml");
   // mock Test
	negativeTestGetLatestContacts (sync_handler,
		                   (gchar*)"wrong",
		                   &sync_key_out,
		                   NULL,
		                   &updated,
		                   &deleted,
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
    testGetContactsHandler (&sync_handler, accountuid);

                                                                                                                                                                                                                                                                                                                                                 
    mark_point();
   // mock Test
	   rtn = eas_sync_handler_add_items (sync_handler,
                                      "wrong",
                                      (gchar**)&sync_key_out,
                                      EAS_ITEM_CONTACT,
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
Suite* eas_libeascon_suite (void)
{
	Suite* s = suite_create ("libeascon");

	/* tc_libeascon test case */
	TCase *tc_libeascon = tcase_create ("core");
	suite_add_tcase (s, tc_libeascon);
	if(getenv ("EAS_USE_MOCKS") && (atoi (g_getenv ("EAS_USE_MOCKS")) >= 1))
    	{
		tcase_add_test (tc_libeascon, test_con_get_invalid_sync_key);
		tcase_add_test (tc_libeascon, test_con_add_invalid_sync_key);
		tcase_add_test (tc_libeascon, test_con_delete_invalid_sync_key);
		tcase_add_test (tc_libeascon, test_con_delete_invalid_server_id);
		tcase_add_test (tc_libeascon, test_con_delete_valid_invalid_server_id);
		tcase_add_test (tc_libeascon, test_con_delete_valid_calendar_server_id);
		tcase_add_test (tc_libeascon, test_con_update_invalid_server_id);

	// crash functions will not consume mocked response.Dummy function call after each test will consume the mocked response.	
		tcase_add_test (tc_libeascon, test_con_delete_crash);	
		tcase_add_test (tc_libeascon, test_consume_response);					
		tcase_add_test (tc_libeascon, test_con_update_crash);	
		tcase_add_test (tc_libeascon, test_consume_response);	
		tcase_add_test (tc_libeascon, test_con_add_crash);
		tcase_add_test (tc_libeascon, test_consume_response);
		tcase_add_test (tc_libeascon, test_con_get_crash);
		tcase_add_test (tc_libeascon, test_consume_response);

	}
	//tcase_add_test (tc_libeascon, test_translate_vcard_to_xml);
	//tcase_add_test (tc_libeascon, test_get_sync_handler);
//	tcase_add_test (tc_libeascon, test_get_latest_contacts_items);
//	tcase_add_test (tc_libeascon, test_eas_sync_handler_delete_all_created_con);
//	tcase_add_test (tc_libeascon, test_eas_sync_handler_add_con);
//	tcase_add_test (tc_libeascon, test_eas_sync_handler_update_con);

	return s;
}
