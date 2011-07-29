#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <dbus/dbus-glib.h>

#include "eas_test_user.h"

#include "../libeasmail/src/libeasmail.h"
#include "../libeasmail/src/eas-folder.h"
#include "../libeasmail/src/eas-email-info.h"
#include "../libeasmail/src/eas-attachment.h"

/** Uncomment for to enable Mocks **/
 #include "../libeastest/src/libeastest.h"

static gchar * g_account_id = (gchar*)TEST_ACCOUNT_ID;

gchar * g_inbox_id = NULL;

static GMainLoop *loop = NULL;

static GThread *sync_calls_thread = NULL;
static GMainLoop *progress_loop = NULL;	// mainloop used to receive progress updates on for tests that require progress updates

Suite* eas_libeasmail_suite (void);

static void
test_push_email_cb (GSList * data, GError *error)
{
	GSList *folder_list = data;
	fail_if (data == NULL,
             "something has gone wrong!");
	g_debug ("Response for test [%s]", (gchar*)folder_list->data);

	g_main_loop_quit (loop);
    g_main_loop_unref (loop);
    loop = NULL;

}

static void
test_email_request_progress_cb (gpointer progress_data, guint percent)
{
	g_debug("test got progress update");
	if(percent == 100)
	{
		g_thread_join(sync_calls_thread); // don't quit until the sync call has completed
		g_debug("quit progress loop");
		g_main_loop_quit (progress_loop);
    	g_main_loop_unref (progress_loop);
    	progress_loop = NULL;
	}
}

static void testGetMailHandler (EasEmailHandler **email_handler, const char* accountuid)
{
    GError *error = NULL;
    // get a handle to the DBus interface and associate the account ID with
    // this object
    *email_handler = eas_mail_handler_new (accountuid, &error);

    if (error)
    {
        g_critical("%s", error->message);
        g_error_free (error);
    }
    
    // confirm that the handle object has been correctly setup
    fail_if (*email_handler == NULL,
             "eas_mail_handler_new returns NULL when given a valid ID");
    
    fail_if ( (*email_handler)->priv == NULL,
              "eas_mail_handler_new account ID object (EasEmailHandler *) member priv (EasEmailHandlerPrivate *) NULL");

}

static void testGetFolderHierarchy (EasEmailHandler *email_handler,
                                    gchar *sync_key,
                                    GSList **created,
                                    GSList **updated,
                                    GSList **deleted,
                                    GError **error)
{
    gboolean ret = FALSE;
    GSList *item = NULL;
    mark_point();
    ret  = eas_mail_handler_sync_folder_hierarchy (email_handler,
                                                   sync_key,
                                                   created,
                                                   updated,
                                                   deleted,
                                                   error);
    mark_point();
    // if the call to the daemon returned an error, report and drop out of the test
    if ( (*error) != NULL)
    {
        fail_if (ret == FALSE, "%s", (*error)->message);
    }
    fail_if (*created == NULL);

    // the exchange server should increment the sync key and send back to the
    // client so that the client can track where it is with regard to sync.
    // therefore the key must not be zero as this is the seed value for this test
    fail_if (!g_strcmp0 (sync_key, "0"),
             "Sync Key not updated by call the exchange server %s", sync_key);
             
    for (item = *created; item; item = item->next)
    {
        EasFolder *folder = item->data;
        if (!g_strcmp0("Inbox", folder->display_name))
        {
            g_free(g_inbox_id);
            g_inbox_id = g_strdup(folder->folder_id);
            break;
        }	
    }
    
    fail_if(g_inbox_id == NULL);
    g_message("Inbox Id = [%s]", g_inbox_id);
}

static void GetFolderHierarchy_negativetests (EasEmailHandler *email_handler,
                                    gchar *sync_key,
                                    GSList **created,
                                    GSList **updated,
                                    GSList **deleted,
                                    GError **error)
{
    gboolean ret = FALSE;
    GSList *item = NULL;
    ret  = eas_mail_handler_sync_folder_hierarchy (email_handler,
                                                   sync_key,
                                                   created,
                                                   updated,
                                                   deleted,
                                                   error);

	fail_if ((*error) == NULL, "negative test did not give error for folder hierarchy request");
}

static void testGetFolderInfo (EasEmailHandler *email_handler,
                               gchar *folder_sync_key,
                               const gchar *folder_id,
                               GSList **emails_created,
                               GSList **emails_updated,
                               GSList **emails_deleted,
                               gboolean *more_available,
                               GError **error)
{
    gboolean ret = FALSE;
    ret = eas_mail_handler_sync_folder_email_info (email_handler,
                                                   folder_sync_key,
                                                   folder_id,
                                                   emails_created,
                                                   emails_updated,
                                                   emails_deleted,
                                                   more_available,
                                                   error);

    // if the call to the daemon returned an error, report and drop out of the test
    if ( (*error) != NULL)
    {
        fail_if (ret == FALSE, "%s", (*error)->message);
    }
//    fail_if(folder_sync_key == 0,
//      "Folder Sync Key not updated by call the exchange server");
}

static void GetFolderInfo_negativetests (EasEmailHandler *email_handler,
                               gchar *folder_sync_key,
                               const gchar *folder_id,
                               GSList **emails_created,
                               GSList **emails_updated,
                               GSList **emails_deleted,
                               gboolean *more_available,
                               GError **error)
{
    gboolean ret = FALSE;
    ret = eas_mail_handler_sync_folder_email_info (email_handler,
                                                   folder_sync_key,
                                                   folder_id,
                                                   emails_created,
                                                   emails_updated,
                                                   emails_deleted,
                                                   more_available,
                                                   error);

	fail_if ((*error) == NULL, "negative test did not give error for email info request");
}

	
struct _TrySendEmailParams 
{
	EasEmailHandler *email_handler;
	EasProgressFn progress_fn;
	gpointer progress_data;
	GError **error;	
	const gchar *client_id;
	const gchar *mime_file;
};

typedef struct _TrySendEmailParams TrySendEmailParams;

static void try_send_email(gpointer data)
{
	TrySendEmailParams *params = data;
	gboolean rtn;

    rtn = eas_mail_handler_send_email (params->email_handler,
                                       params->client_id,
                                       params->mime_file,
                                       params->progress_fn,
                                       params->progress_data,		
                                       params->error);											 

	g_debug("eas_mail_handler_send_email returned");

	if (!rtn)
	{
		fail_if(TRUE, "%s", (*params->error)->message);
	}

	g_free(params);
	
	return;
}

static gboolean spawn_send_email_thread (gpointer data)
{
	sync_calls_thread = g_thread_create(try_send_email, data, TRUE, NULL);

	return FALSE;	// run once (don't put back on mainloop again)
}


static void testSendEmail (EasEmailHandler *email_handler,
                           const gchar *client_id,
                           const gchar *mime_file,
                           GError **error)
{
    gboolean ret = FALSE;
	EasProgressFn progress_cb = NULL;
	progress_loop = g_main_loop_new (NULL, FALSE);
	
	// comment out if progress reports not desired:
	progress_cb = test_email_request_progress_cb;

	TrySendEmailParams *send_params = g_new0 (TrySendEmailParams, 1);
	send_params->email_handler = email_handler;
	send_params->client_id = client_id;
	send_params->mime_file = mime_file;
	send_params->progress_fn = progress_cb;
	send_params->progress_data = NULL;
	send_params->error = error;		

	g_idle_add(spawn_send_email_thread, send_params);	// spawns a new thread to make the sync call

	mark_point();

	g_debug("run mainloop");
	
	g_main_loop_run (progress_loop);	// drop into main loop, quits when 100% progress feedback received
  
}

static void setMockNegTestGoodHttp(const gchar *mockedfile)
{
	guint status_code = 200;
	GArray *status_codes = g_array_new(FALSE, FALSE, sizeof(guint));
	g_array_append_val(status_codes, status_code);
    const gchar *mocks[] = {mockedfile, 0};
    EasTestHandler *test_handler = eas_test_handler_new ();
    if (test_handler)
    {
		//eas_test_handler_add_mock_responses (test_handler, mocks, NULL);
        eas_test_handler_add_mock_responses (test_handler, mocks, status_codes);
        g_object_unref (test_handler);
        test_handler = NULL;
    }
	g_array_free(status_codes, TRUE);
}


START_TEST (test_eas_mail_handler_read_email_metadata)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    // TODO - send the email we're expecting to verify below rather than relying on it being created manually

    mark_point();
    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);
    mark_point();

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");


    mark_point();
    testGetFolderInfo (email_handler,
                       folder_sync_key,
                       g_inbox_id, // Inbox
                       &emails_created,
                       &emails_updated,
                       &emails_deleted,
                       &more_available,
                       &error);
    mark_point();

    // if the emails_created list contains email
    if (emails_created)
    {
        EasEmailInfo *email = NULL;
        GSList *headers = NULL;
        EasEmailHeader *header = NULL;
        guint importance = 0;

        mark_point();
        // inspect metadata for first email in the folder
        email = g_slist_nth (emails_created, 0)->data;
        fail_if (!email, "Unable to get first email in emails_created GSList");

        // verify that it's unread
        fail_if (email->flags & EAS_EMAIL_READ, "First Email in Inbox is expected to be unread");

        // TODO read the expected metadata from config

        //verify that it has a single attachment_separator
        fail_if (! (g_slist_length (email->attachments) == 1), "First Email in Inbox is expected to have a single attachment");

        //verify that it's marked with high importance
        headers = email->headers;
        for (headers = email->headers; headers != NULL; headers = headers->next)
        {
            header = headers->data;
            if (!g_strcmp0 ( (char *) header->name, "Importance"))
            {
                importance = atoi (header->value);
                g_debug ("Importance = %d", importance);
                break;
            }
        }

        fail_unless (importance == 2);

    }

    g_object_unref (email_handler);
    return;
}
END_TEST

START_TEST (test_eas_mail_handler_update_email)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;

    // get a handle to the DBus interface
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");


    // Get folder email info for Inbox:
    testGetFolderInfo (email_handler, 
                       folder_sync_key, 
                       g_inbox_id, 
                       &emails_created, 
                       &emails_updated, 
                       &emails_deleted, 
                       &more_available, 
                       &error);

    // if the emails_created list contains email
    if (emails_created)
    {
        EasEmailInfo *email = NULL;
        gboolean rtn = FALSE;
        GSList *emails = NULL;
        GSList *emails2_created = NULL; //can't reuse emails_created since that isn't empty now

        // get email info for first email in the folder and see if it's been read
        email = (g_slist_nth (emails_created, 0))->data;

        // toggle the read flag:
        if (email->flags & EAS_EMAIL_READ)
        {
            email->flags &= ~EAS_EMAIL_READ;
        }
        else
        {
            email->flags |= EAS_EMAIL_READ;
        }

        // TODO - something a bit more exciting than toggle of read flag (add/remove a category)

        mark_point();

        emails = g_slist_append (emails, email);

		/*
		// add an email that doesn't exist to produce an item level error (update should still pass):
		EasEmailInfo *email2 = g_new0(EasEmailInfo, 1);

		email2->server_id = "5:123";
		email2->flags |= EAS_VALID_READ;
		email2->flags |= EAS_EMAIL_READ;
		emails = g_slist_append (emails, email2);
		*/
		
        // update the first mail in the folder
		gchar folder_sync_key_pre_update[64];
		strcpy(folder_sync_key_pre_update, folder_sync_key);
		
		g_debug("folder sync key = %s (before eas_mail_handler_update_email)", folder_sync_key);
        rtn = eas_mail_handler_update_email (email_handler, folder_sync_key, g_inbox_id, emails, &error);
        if (error)
        {
            fail_if (rtn == FALSE, "%s", error->message);
        }

		/*
		EasEmailInfo *temp = (g_slist_nth (emails, 0))->data;
		g_debug("email1 status = %s", temp->status);
		temp = (g_slist_nth (emails, 1))->data;
		g_debug("email2 status = %s", temp->status);
		
		g_free(email2);
		*/
		
        mark_point();

        // verify that we get the update if we sync with the pre-update sync_key:
        testGetFolderInfo (email_handler, folder_sync_key_pre_update, g_inbox_id, &emails2_created, &emails_updated, &emails_deleted, &more_available, &error);
		
        fail_if (emails2_created, "Not expecting any new emails");
        fail_if (emails_deleted, "Not expecting any deletions");
        fail_unless (NULL != emails_updated, "No updates from exchange server sync after we updated");

        // free email object list before reusing
        g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
        g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
        g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

        g_slist_free (emails_deleted);
        g_slist_free (emails_updated);
        g_slist_free (emails_created);

        emails_deleted = NULL;
        emails_updated = NULL;
        emails_created = NULL;
    }

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);

}
END_TEST

void print_update(gpointer data, gpointer user_data)
{
	EasIdUpdate *update = data;
	g_debug("update src id = %s", update->src_id);	
	g_debug("update dst id = %s", update->dest_id);	
	g_debug("update status = %s", update->status);	
}

/*
 Move the first email in the inbox to a 'temp' folder (at same level as Inbox folder) 
*/
START_TEST (test_eas_mail_handler_move_to_folder)
{
  	const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
	gchar temp_folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
	gchar *tempfolder_id = NULL;
	GSList *l = NULL;
	
    // get a handle to the DBus interface
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);
	
    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");
	fail_if (deleted, "Not expecting any deletions with folder sync");
	fail_if (updated, "Not expecting any updates with folder sync");

	// get the id for the 'Temp' folder
    for (l = created; l; l = l->next)
    {
        EasFolder *folder = l->data;
        if (!g_strcmp0("Temp", folder->display_name))
        {
            g_free(tempfolder_id);
            tempfolder_id = g_strdup(folder->folder_id);
            break;
        }		
    }

	fail_unless(NULL != tempfolder_id, "no Temp folder found");
	mark_point();
	
    // Get folder email info for Inbox:
    testGetFolderInfo (email_handler, folder_sync_key, g_inbox_id, &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

	fail_unless (NULL != emails_created, "This test requires at least two emails in the Inbox and there were none");
	fail_unless (g_slist_length(emails_created) > 1, "This test requires at least two emails in the Inbox and there were less than two");
	fail_if (emails_deleted, "Not expecting any deletions with first sync");
	fail_if (emails_updated, "Not expecting any updates with first sync");

    mark_point();	

	{
		GSList *emails2_created = NULL; //receives a list of EasMails
		GSList *emails2_deleted = NULL; 
		GSList *server_ids = NULL;
        EasEmailInfo *email = NULL;
        gboolean rtn = FALSE;
		GSList *updated_ids = NULL;
		
        // get email info for first two emails in the folder and pull out the server_ids:
        email = (g_slist_nth (emails_created, 0))->data;
		server_ids = g_slist_append(server_ids, email->server_id);
        email = (g_slist_nth (emails_created, 1))->data;
		server_ids = g_slist_append(server_ids, email->server_id);
//		server_ids = g_slist_append(server_ids, "bad server id");
		
        mark_point();

        // move the emails
        rtn = eas_mail_handler_move_to_folder (email_handler, server_ids, g_inbox_id, tempfolder_id, &updated_ids, &error);
        mark_point();
        if (error)
        {
            fail_if (rtn == FALSE, "%s", error->message);
        }

		g_slist_foreach(updated_ids, print_update, NULL);

        mark_point();

        // verify that we get a create (for Temp folder) and a delete (for Inbox) with the next sync:
        testGetFolderInfo (email_handler, folder_sync_key, g_inbox_id, &emails2_created, &emails_updated, &emails_deleted, &more_available, &error);

        fail_unless (NULL != emails_deleted, "Expecting deletions (from Inbox) after we moved an email");
        fail_if (emails_updated, "Not expecting updates in inbox after we moved an email");		
        fail_if (emails2_created, "Not expecting a new email in inbox after we move an email");

		// TODO sync the temp folder and verify we get an added email. what sync_key
        testGetFolderInfo (email_handler, temp_folder_sync_key, tempfolder_id, &emails2_created, &emails_updated, &emails2_deleted, &more_available, &error);		
        fail_unless (NULL != emails2_created, "Expecting created emails in Temp folder after we moved an email");
        fail_if (emails_updated, "Not expecting updates in Temp folder after we moved an email");		
        fail_if (emails2_deleted, "Not expecting deleted emails for Temp folder after we move an email");
		
		g_slist_foreach (emails2_created, (GFunc) g_object_unref, NULL);		
		g_slist_free (emails2_created);
	}
	
    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    g_slist_free (emails_deleted);
    g_slist_free (emails_updated);
    g_slist_free (emails_created);	
	
    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);
    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

	g_free(tempfolder_id);
	
    g_object_unref (email_handler);
}

END_TEST


START_TEST (test_eas_mail_handler_send_email)
{
    EasEmailHandler *email_handler = NULL;
    gchar *mime_file = g_strconcat (getenv ("HOME"), "/int07/testdata/mime_file.txt", NULL);
    gchar client_id[21];
    const gchar* accountuid = g_account_id;
    GError *error = NULL;
    guint random_num;
    /* initialize random generator */
    srand (time (NULL));
    random_num = rand();

    snprintf (client_id, sizeof (client_id) / sizeof (client_id[0]), "%d", random_num);

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    mark_point();

    // call into the daemon to send email to the exchange server
    testSendEmail (email_handler, client_id, mime_file, &error);
    g_free (mime_file);

    g_object_unref (email_handler);
}
END_TEST

START_TEST (test_get_mail_handler)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;

    testGetMailHandler (&email_handler, accountuid);

    g_object_unref (email_handler);
}
END_TEST

START_TEST (test_get_init_eas_mail_sync_folder_hierarchy)
{
    // This value needs to make sense in the daemon.  in the first instance
    // it should be hard coded to the value used by the daemon but later
    // there should be a mechanism for getting the value from the same place
    // that the daemon uses
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar sync_key[64] = "0";
    GError *error = NULL;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, sync_key, &created, &updated, &deleted, &error);

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");

    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);
}
END_TEST

START_TEST (test_eas_mail_sync_folder_hierarchy_bad_synckey)
{
	// init as if we were going to get the folder hierarchy correctly
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
	gchar sync_key[64] = "0";
    GError *error = NULL;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

	setMockNegTestGoodHttp("EmailFolderHierarchyInvalidSyncKey.xml");

	// call into the daemon to get the folder hierarchy from the exchange server
    GetFolderHierarchy_negativetests (email_handler, sync_key, &created, &updated, &deleted, &error);

	fail_if(g_strcmp0 (dbus_g_error_get_name(error),
	                   "org.meego.activesyncd.FolderSyncError.INVALIDSYNCKEY"),
	        "Incorrect handling of invalid sync key");
	
    //  free everything!
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);
}
END_TEST

START_TEST (test_get_eas_mail_info_in_folder)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    EasFolder *folder = NULL;
    gboolean more_available = FALSE;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);
    

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");


    // get the object for the first folder returned in the hierarchy
    folder = g_slist_nth_data (created, 0);
    fail_if (folder == NULL, "Folder is null");
    // get the folder info for the first folder returned in the hierarchy
    testGetFolderInfo (email_handler, folder_sync_key, g_inbox_id, &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);
}
END_TEST

START_TEST (test_get_eas_mail_info_in_inbox)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");

    // get the folder info for the inbox
    testGetFolderInfo (email_handler, folder_sync_key, g_inbox_id, &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);

}
END_TEST

START_TEST (test_get_eas_mail_info_bad_folder_id)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;

    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");

	setMockNegTestGoodHttp("EmailGetInfoBadFolderId.xml");
	// set mock
	{
		guint status_code = 200;
		GArray *status_codes = g_array_new(FALSE, FALSE, sizeof(guint));
		g_array_append_val(status_codes, status_code);
        const gchar *mocks[] = {"EmailGetInfoBadFolderId.xml", 0};
        EasTestHandler *test_handler = eas_test_handler_new ();
        if (test_handler)
        {
			//eas_test_handler_add_mock_responses (test_handler, mocks, NULL);
            eas_test_handler_add_mock_responses (test_handler, mocks, status_codes);
            g_object_unref (test_handler);
            test_handler = NULL;
        }
		g_array_free(status_codes, TRUE);
    }
    // get the folder info for the inbox
    GetFolderInfo_negativetests (email_handler, folder_sync_key, "wrong", &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

	fail_if(g_strcmp0 (dbus_g_error_get_name(error),
	                   "org.meego.activesyncd.SyncError.OBJECTNOTFOUND"),
	        "Incorrect handling of invalid sync key");

	g_debug(dbus_g_error_get_name(error));

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);

}
END_TEST

struct _TryFetchBodyParams 
{
	EasEmailHandler *email_handler;
	EasProgressFn progress_fn;
	gpointer progress_data;
	GError **error;	
	const gchar *folder_id;
	const gchar *server_id;
	const gchar *mime_directory;
};

typedef struct _TryFetchBodyParams TryFetchBodyParams;

static void try_fetch_email_body(gpointer data)
{
	TryFetchBodyParams *params = data;
	gboolean rtn;

	rtn = eas_mail_handler_fetch_email_body (params->email_handler,
                                                 params->folder_id, 
                                                 params->server_id,
                                                 params->mime_directory,
                                                 params->progress_fn,
                                                 params->progress_data,		
                                                 params->error);

	g_debug("eas_mail_handler_fetch_email_body");
	if (!rtn)
	{
		fail_if(TRUE, "%s", (*params->error)->message);
	}

	g_free(params);
	
	return;
}

// heartbeat
static gboolean temp_func()
{
	g_debug("running in mainloop");
	g_usleep(500000);
	return TRUE;	// (put back on mainloop again when done)
}

static gboolean spawn_fetch_email_thread (gpointer data)
{
	sync_calls_thread = g_thread_create(try_fetch_email_body, data, TRUE, NULL);

	//g_idle_add(temp_func, NULL);
	
	return FALSE;	// run once (don't put back on mainloop again)
}

START_TEST (test_eas_mail_handler_fetch_email_body)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
//    EasFolder *folder = NULL;
    gboolean testMailFound = FALSE;
//    guint folderIndex;
	progress_loop = g_main_loop_new (NULL, FALSE);	
	
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);


    mark_point();
    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);
    mark_point();

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");

    mark_point();
    testGetFolderInfo (email_handler,
                       folder_sync_key,
                       g_inbox_id, // Inbox
                       &emails_created,
                       &emails_updated,
                       &emails_deleted,
                       &more_available,
                       &error);
    mark_point();

    // if the emails_created list contains email
    if (emails_created)
    {
        EasEmailInfo *email = NULL;
        gboolean rtn = FALSE;
        gchar *mime_directory = g_strconcat (getenv ("HOME"), "/mimeresponses/", NULL);
        gchar mime_file[256];
        FILE *hBody = NULL;

        mark_point();
        // get body for first email in the folder
        email = g_slist_nth (emails_created, 0)->data;
        fail_if (!email, "Unable to get first email in emails_created GSList");

        // destination directory for the mime file

        mark_point();
        strcpy (mime_file, mime_directory);
        fail_if (!email->server_id, "Email has no server_id!");
        strcat (mime_file, email->server_id);

        mark_point();
        // check if the email body file exists
        hBody = fopen (mime_file, "r");
        if (hBody)
        {
            // if the email body file exists delete it
            fclose (hBody);
            hBody = NULL;
            remove (mime_file);
            hBody = fopen (mime_file, "r");
            if (hBody)
            {
                fclose (hBody);
                fail_if (1, "unable to clear existing body file");
            }
        }

        mark_point();

		EasProgressFn progress_cb = test_email_request_progress_cb;

		TryFetchBodyParams *fetch_params = g_new0 (TryFetchBodyParams, 1);
		fetch_params->email_handler = email_handler;
		fetch_params->folder_id = g_inbox_id;
		fetch_params->server_id = email->server_id;
		fetch_params->mime_directory = mime_directory;
		fetch_params->progress_fn = progress_cb;
		fetch_params->progress_data = NULL;
		fetch_params->error = &error;		

		g_idle_add(spawn_fetch_email_thread, fetch_params);	// spawns a new thread to make the sync call

		mark_point();
		
		g_main_loop_run (progress_loop);	// drop into main loop, quits when 100% progress feedback received

		mark_point();
		
        g_free (mime_directory);

        testMailFound = TRUE;
        // if reported success check if body file exists
        hBody = fopen (mime_file, "r");
        fail_if (hBody == NULL, "email body file not created in specified directory");
        fclose (hBody);
    }
    else
    {
        fail_unless (NULL != emails_created, "Emails created is NULL for fetch_email_body");
    }


    // fail the test if there is no email in the folder hierarchy as this means the
    // test has not exercised the code to get the email body as required by this test case
    fail_if (testMailFound == FALSE, "no mail found in the folder hierarchy");

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);
}
END_TEST

struct _TryFetchAttachmentParams {
	EasEmailHandler *email_handler;
	EasProgressFn progress_fn;
	gpointer progress_data;
	GError **error;
	const gchar *file_reference;
	const gchar *mime_directory;	
};

typedef struct _TryFetchAttachmentParams TryFetchAttachmentParams;

void try_fetch_email_attachment(gpointer data)
{
	TryFetchAttachmentParams *params = data;
	gboolean rtn;

	g_debug("call eas_mail_handler_fetch_email_attachment");
	
    rtn = eas_mail_handler_fetch_email_attachment (params->email_handler,
                                                   params->file_reference,
                                                   params->mime_directory,  
                                                   params->progress_fn,
                                                   params->progress_data,		
                                                   params->error);	

	g_debug("eas_mail_handler_fetch_email_attachment returned");
	
	if (!rtn)
	{
		fail_if(TRUE, "%s", (*params->error)->message);
	}

	g_free(params);
	
	return;
}

gboolean spawn_fetch_attachment_thread (gpointer data)
{
	sync_calls_thread = g_thread_create(try_fetch_email_attachment, data, TRUE, NULL);

	return FALSE;	// run once (don't put back on mainloop again)
}

START_TEST (test_eas_mail_handler_fetch_email_attachments)
{
    const gchar* accountuid = g_account_id;
	
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
//    EasFolder *folder = NULL;
    gboolean testMailFound = FALSE;
//    guint folderIndex;
    gboolean attachmentFound = FALSE;
    EasEmailInfo *email = NULL;
    gint number_of_emails_created = 0;
    gint i = 0;
	progress_loop = g_main_loop_new (NULL, FALSE);
	
    // get a handle to the DBus interface and associate the account ID with
    // this object
    testGetMailHandler (&email_handler, accountuid);

    mark_point();
    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);
    mark_point();
    

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");

    mark_point();
    testGetFolderInfo (email_handler,
                       folder_sync_key,
                       g_inbox_id, // Inbox
                       &emails_created,
                       &emails_updated,
                       &emails_deleted,
                       &more_available,
                       &error);
    mark_point();

    //Search for an attachment in emails
    number_of_emails_created = g_slist_length (emails_created);
    for (; i < number_of_emails_created; i++)
    {
        email = g_slist_nth (emails_created, i)->data;
        if (g_slist_length (email->attachments))
        {
            attachmentFound = TRUE;
            break;
        }
    }


    // if the emails_created list contains an email with attachment
    if (attachmentFound)
    {
        EasAttachment *attachmentObj = NULL;
//        gchar* file_reference = NULL;
        gboolean rtn = FALSE;

        mark_point();
        //Get all attachments for every email_created.
        i = 0;
        for (; i < number_of_emails_created; i++)
        {
            gint number_of_attachments = 0;
            gint j = 0;

            email = g_slist_nth (emails_created, i)->data;
            //fail_if(!email, "Unable to get first email in emails_created GSList");
            number_of_attachments = g_slist_length (email->attachments);
            for (; j < number_of_attachments; j++)
            {
                gchar *mime_directory = g_strconcat (getenv ("HOME"), "/mimeresponses/", NULL);
                gchar mime_file[256];
                FILE *hAttachment = NULL;

                attachmentObj = g_slist_nth (email->attachments, j)->data;
                fail_if (attachmentObj->file_reference == NULL, "attachment has no file_reference!");

                // destination directory for the mime file

                mark_point();
                strcpy (mime_file, mime_directory);
                strcat (mime_file, (gchar*)attachmentObj->file_reference);

                mark_point();
                // check if the email body file exists
                hAttachment = fopen (mime_file, "r");
                if (hAttachment)
                {
                    // if the email body file exists delete it
                    fclose (hAttachment);
                    hAttachment = NULL;
                    remove (mime_file);
                    hAttachment = fopen (mime_file, "r");
                    if (hAttachment)
                    {
                        fclose (hAttachment);
                        fail_if (1, "unable to clear existing attachment file");
                    }
                }

                mark_point();

				EasProgressFn progress_cb = NULL;
		
				// comment out if progress reports not desired:
				progress_cb = test_email_request_progress_cb;

				TryFetchAttachmentParams *fetch_params = g_new0 (TryFetchAttachmentParams, 1);
				fetch_params->email_handler = email_handler;
				fetch_params->file_reference = attachmentObj->file_reference;
				fetch_params->mime_directory = mime_directory;
				fetch_params->progress_fn = progress_cb;
				fetch_params->progress_data = NULL;
				fetch_params->error = &error;
				
				g_idle_add(spawn_fetch_attachment_thread, fetch_params);
				
				mark_point();

				g_debug("run mainloop");
				
				g_main_loop_run (progress_loop);

				mark_point();
				
                g_free (mime_directory);

                testMailFound = TRUE;
                // if reported success check if attachment file exists
                hAttachment = fopen (mime_file, "r");
                fail_if (hAttachment == NULL, "email attachment file not created in specified directory");
                fclose (hAttachment);

            } //-end of attachment loop
        }//-end of emails_created loop
    }

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);
}
END_TEST

START_TEST (test_eas_mail_handler_watch_email_folders)
{
	const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
	GSList *folder_list = NULL; //receives a list of EasMails
//    GSList *updates = NULL;
	GError *error = NULL;

	testGetMailHandler (&email_handler, accountuid);

	folder_list = g_slist_append(folder_list, (gpointer)"7");

	loop = g_main_loop_new (NULL, FALSE);

	eas_mail_handler_watch_email_folders (email_handler, folder_list, "60", test_push_email_cb, &error);

	if (loop) g_main_loop_run (loop);
}
END_TEST


struct _TrySyncFolderEmailParams 
{
	EasEmailHandler *email_handler;
	const gchar *sync_key_in;
	guint	time_window;
	const gchar *folder_id;
	GSList *delete_emails;
	GSList *change_emails; 
	gchar **sync_key_out;
	GSList **emails_created;
	GSList **emails_changed;
	GSList **emails_deleted;
	gboolean *more_available;
	EasProgressFn progress_fn;
	gpointer progress_data;                                          
	GError **error;
};

typedef struct _TrySyncFolderEmailParams TrySyncFolderEmailParams;

static void try_sync_folder_email(gpointer data)
{
	TrySyncFolderEmailParams *params = data;
	gboolean rtn;

	g_debug("sync_folder_email with %s", params->sync_key_in);
	
	rtn = eas_mail_handler_sync_folder_email (params->email_handler,
											  params->sync_key_in,
											  params->time_window,						// no time filter
											  params->folder_id,
											  params->delete_emails,						// emails to delete
											  params->change_emails, 					// emails to change
											  params->sync_key_out,
											  params->emails_created,
											  params->emails_changed, 
											  params->emails_deleted,
											  params->more_available,
											  params->progress_fn,			// progress function
											  params->progress_data,		// progress data
											  params->error); 	

	g_debug("eas_mail_handler_sync_folder_email");
	if (!rtn)
	{
		fail_if(TRUE, "%s", (*params->error)->message);
	}

	g_debug("more available = %s", (params->more_available? "TRUE" : "FALSE"));
	g_free(params);
	
	return;
}

static gboolean spawn_sync_folder_email_thread (gpointer data)
{
	sync_calls_thread = g_thread_create(try_sync_folder_email, data, TRUE, NULL);
	
	return FALSE;	// run once (don't put back on mainloop again)
}	

START_TEST (test_eas_mail_get_item_estimate)
{
	gboolean ret;
	const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
	GError *error = NULL;
    GSList *created = NULL;	// receives a list of folders
    GSList *updated = NULL;
    GSList *deleted = NULL;
	gchar hierarchy_sync_key[64] = "0";
	gchar folder_sync_key[64] = "0";
	gchar* folder_sync_key_out = NULL;
	gchar* folder_sync_key_out_2 = NULL;
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;	
	guint estimate = 0;
	progress_loop = g_main_loop_new (NULL, FALSE);
	
	testGetMailHandler (&email_handler, accountuid);
	testGetFolderHierarchy (email_handler, hierarchy_sync_key, &created, &updated, &deleted, &error);
	fail_if (NULL == created, "No folder information returned from exchange server");
	fail_unless (NULL == updated, "Not expecting updated folders on initial sync");
	fail_unless (NULL == deleted, "Not expecting deleted folders on initial sync");

	fail_if(g_inbox_id == NULL, "Failed to find inbox id");

	// sync with sync_key=0:
	g_debug("sync_folder_email with %s", folder_sync_key);
	ret = eas_mail_handler_sync_folder_email (email_handler,
						  folder_sync_key,
                          0,						// no time filter
						  g_inbox_id,
						  NULL,						// emails to delete
						  NULL, 					// emails to change
                          &folder_sync_key_out,
						  &emails_created,
	                      &emails_updated, 
	                      &emails_deleted,
						  &more_available,
	                      NULL,						// progress function
	                      NULL,						// progress data
						  &error); 

	if(!ret)
	{
		fail_if(TRUE, "eas_mail_handler_sync_folder_email returned %s", error->message);
	}

	fail_if(!more_available, "expect more_available to be TRUE on first sync");

	mark_point();
	g_debug("get_item_estimate with %s", folder_sync_key_out);
	ret = eas_mail_handler_get_item_estimate(email_handler, folder_sync_key_out, g_inbox_id, &estimate, &error);
	if(!ret)
	{
		if(error)
		{
			fail_if(TRUE, "get_item_estimate returned %s", error->message);
		}
	}
	g_debug("estimate = %d", estimate);	
	fail_if(estimate == 0, "Expected estimate to be non-zero before first proper sync");

	mark_point();

	// sync with sync_key!=0, with progress updates
	EasProgressFn progress_cb = test_email_request_progress_cb;	

	// set up params
	TrySyncFolderEmailParams *sync_params = g_new0 (TrySyncFolderEmailParams, 1);
	sync_params->email_handler = email_handler;
	sync_params->folder_id = g_inbox_id;
	sync_params->sync_key_in = folder_sync_key_out;
	sync_params->time_window = 0;
	sync_params->folder_id = g_inbox_id;
	sync_params->delete_emails = NULL;
	sync_params->change_emails = NULL;
	sync_params->sync_key_out = &folder_sync_key_out_2;
	sync_params->emails_created = &emails_created;
	sync_params->emails_changed = &emails_updated;
	sync_params->emails_deleted = &emails_deleted;
	sync_params->more_available = &more_available;	
	sync_params->progress_fn = progress_cb;
	sync_params->progress_data = NULL;
	sync_params->error = &error;		

	// spawn a thread to make the synchronous call so this thread free to receive progress updates:
	g_idle_add(spawn_sync_folder_email_thread, sync_params);	

	mark_point();
	
	g_main_loop_run (progress_loop);	// drop into main loop, quits when 100% progress feedback received

	mark_point();

	fail_if(more_available, "Expect more_available to be TRUE when syncing with sync_key of zero");

	mark_point();

	g_debug("get_item_estimate with %s", folder_sync_key_out_2);
	ret = eas_mail_handler_get_item_estimate(email_handler, folder_sync_key_out_2, g_inbox_id, &estimate, &error);
	if(!ret)
	{
		if(error)
		{
			fail_if(TRUE, "get_item_estimate returned %s", error->message);
		}
	}
	g_debug("estimate = %d", estimate);	
	fail_if(estimate != 0, "Expected estimate to be zero after second sync");

	
	mark_point();

}
END_TEST


START_TEST (test_eas_mail_handler_delete_email)
{
    const gchar* accountuid = g_account_id;
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
	gchar *temp_sync_key;
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
    EasFolder *folder = NULL;
    gboolean testMailFound = FALSE;
    guint folderIndex;

    // get a handle to the DBus interface
    testGetMailHandler (&email_handler, accountuid);

    // call into the daemon to get the folder hierarchy from the exchange server
    testGetFolderHierarchy (email_handler, folder_hierarchy_sync_key, &created, &updated, &deleted, &error);

    fail_if(g_inbox_id == NULL, "Failed to find inbox id");

    // fail the test if there is no folder information
    fail_unless (NULL != created, "No folder information returned from exchange server");

    // loop through the folders in the hierarchy to find a folder with an email in it
    for (folderIndex = 0; g_slist_length (created); folderIndex++)
    {
        // get the folder info for the current folderIndex
        // since the sync id is zero only the created list will contain folders
        folder = (g_slist_nth (created, folderIndex))->data;
        testGetFolderInfo (email_handler, folder_sync_key, g_inbox_id, &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

        // if the emails_created list contains email
		fail_if(!emails_created, "need at least one email in the inbox for this test to pass");
		
        if (emails_created)
        {
            GSList *emailToDel = NULL;
            gchar serveridToDel[64];
            EasEmailInfo *email = NULL;
            gboolean rtn = FALSE;
            EasEmailInfo *reportedDeletedEmail = NULL;

            // get email info for first email in the folder
            email = (g_slist_nth (emails_created, 0))->data;

            emailToDel = g_slist_append (emailToDel, g_strdup (email->server_id));
            g_stpcpy (serveridToDel, email->server_id);

            // delete the first mail in the folder
			temp_sync_key = g_strdup(folder_sync_key);
            rtn = eas_mail_handler_delete_email (email_handler, folder_sync_key, g_inbox_id, emailToDel, &error);
            if (error)
            {
                fail_if (rtn == FALSE, "%s", error->message);
            }


            // free email object list before reusing
            g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
            g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
            g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

            g_slist_free (emails_deleted);
            g_slist_free (emails_updated);
            g_slist_free (emails_created);

            emails_deleted = NULL;
            emails_updated = NULL;
            emails_created = NULL;

            
            // get email info for the folder using sync key from before the delete 
			// nb: because delete now updates the sync_key, a sync with the new key will 
            testGetFolderInfo (email_handler, temp_sync_key, g_inbox_id, &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

            fail_unless (NULL != emails_deleted, "No email reported as deleted");

            reportedDeletedEmail = (g_slist_nth (emails_deleted, 0))->data;

            // fail the test if the server_id for the mail reported as deleted
            // does not match the server_id of the email for which the
            // eas_mail_handler_delete_email call was made
            fail_if (g_strcmp0 (reportedDeletedEmail->server_id, serveridToDel),
                     "Deleted email not reported back by Exchange server as deleted. deleted id was %s and reported id was %s",
                     reportedDeletedEmail->server_id, serveridToDel);

            g_slist_free (emailToDel);

            // after getting the body for the first mail, drop out of the loop
            testMailFound = TRUE;
            break;
        }
        // else, go round the loop again
    }

    // fail the test if there is no email in the folder hierarchy as this means the
    // test has not exercised the code to get the email body as required by this test case
    fail_if (testMailFound == FALSE, "no mail found in the folder hierarchy");

    //  free email objects in lists of email objects
    g_slist_foreach (emails_deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_updated, (GFunc) g_object_unref, NULL);
    g_slist_foreach (emails_created, (GFunc) g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach (created, (GFunc) g_object_unref, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_foreach (updated, (GFunc) g_object_unref, NULL);

    g_slist_free (created);
    g_slist_free (deleted);
    g_slist_free (updated);

    g_object_unref (email_handler);

}
END_TEST

Suite* eas_libeasmail_suite (void)
{
    Suite* s = suite_create ("libeasmail");

    /* libeasmail test case */
    TCase *tc_libeasmail = tcase_create ("core");
    suite_add_tcase (s, tc_libeasmail);

//	tcase_add_test (tc_libeasmail, test_get_mail_handler);
//    tcase_add_test (tc_libeasmail, test_get_init_eas_mail_sync_folder_hierarchy);

	// mocked tests only
	if(getenv ("EAS_USE_MOCKS") && (atoi (g_getenv ("EAS_USE_MOCKS")) >= 1))
    {
	    tcase_add_test (tc_libeasmail, test_eas_mail_sync_folder_hierarchy_bad_synckey);
	    tcase_add_test (tc_libeasmail, test_get_eas_mail_info_bad_folder_id);
	}
//    tcase_add_test (tc_libeasmail, test_get_eas_mail_info_in_inbox);
//    tcase_add_test (tc_libeasmail, test_eas_mail_handler_fetch_email_body);
    //tcase_add_test (tc_libeasmail, test_get_eas_mail_info_in_folder); // only uncomment this test if the folders returned are filtered for email only
//    tcase_add_test (tc_libeasmail, test_eas_mail_handler_fetch_email_attachments);
    //tcase_add_test (tc_libeasmail, test_eas_mail_handler_delete_email);
//    tcase_add_test (tc_libeasmail, test_eas_mail_handler_send_email);
    
    /* Need an unread, high importance email with a single attachment at top of inbox for this to pass: */
    //tcase_add_test (tc_libeasmail, test_eas_mail_handler_read_email_metadata);
    
    /* need at least one email in the inbox for this to pass: */
//    tcase_add_test (tc_libeasmail, test_eas_mail_handler_update_email);
    
	/* need a 'temp' folder created at the same level as Inbox and at least two emails in the inbox for this test to work: */
	//tcase_add_test (tc_libeasmail, test_eas_mail_handler_move_to_folder);
    
	//tcase_add_test(tc_libeasmail, test_eas_mail_handler_watch_email_folders);
	// requires at least one email in inbox to pass
//	tcase_add_test(tc_libeasmail, test_eas_mail_get_item_estimate);
	
    g_free(g_inbox_id);
    g_inbox_id = NULL;

    return s;
}

