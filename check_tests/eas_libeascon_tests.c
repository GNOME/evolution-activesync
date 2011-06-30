#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>

#include "../libeassync/src/libeassync.h"

gchar * g_contacts_account_id = "good.user@cstylianou.com";

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


Suite* eas_libeascon_suite (void)
{
    Suite* s = suite_create ("libeascon");

    /* tc_libeascon test case */
    TCase *tc_libeascon = tcase_create ("core");
    suite_add_tcase (s, tc_libeascon);

    tcase_add_test (tc_libeascon, test_get_latest_contacts_items);

    return s;
}