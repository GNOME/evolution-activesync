#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "../libeasmail/src/libeasmail.h"
#include "../libeasmail/src/eas-folder.h"
#include "../libeasmail/src/eas-email-info.h"
#include "../libeasmail/src/eas-attachment.h"


static void testGetMailHandler(EasEmailHandler **email_handler, const char* accountuid){
  	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    *email_handler = eas_mail_handler_new(accountuid);

    // confirm that the handle object has been correctly setup
    fail_if(*email_handler == NULL,
    "eas_mail_handler_new returns NULL when given a valid ID");
    fail_if((*email_handler)->priv == NULL,
    "eas_mail_handler_new account ID object (EasEmailHandler *) member priv (EasEmailHandlerPrivate *) NULL"); 

}

static void testGetFolderHierarchy(EasEmailHandler *email_handler,
                                     gchar *sync_key,
                                     GSList **created,
                                     GSList **updated,
                                     GSList **deleted,
                                     GError **error){
    gboolean ret = FALSE;
 	mark_point();
    ret  = eas_mail_handler_sync_folder_hierarchy (email_handler,
                                                   sync_key,
                                                   created,
                                                   updated,
                                                   deleted,
                                                   error);
	mark_point();
	// if the call to the daemon returned an error, report and drop out of the test
    if((*error) != NULL){
		fail_if(ret == FALSE,"%s",(*error)->message);
	}
	fail_if(*created ==NULL);
	
	// the exchange server should increment the sync key and send back to the
	// client so that the client can track where it is with regard to sync.
	// therefore the key must not be zero as this is the seed value for this test
    fail_if(!g_strcmp0(sync_key, "0"),
		"Sync Key not updated by call the exchange server %s", sync_key);
		
}

static void testGetFolderInfo(EasEmailHandler *email_handler,
                                     gchar *folder_sync_key,
                                     const gchar *folder_id,
                                     GSList **emails_created,
                                     GSList **emails_updated,
                                     GSList **emails_deleted,
                                     gboolean *more_available,
                                     GError **error){
	gboolean ret = FALSE;									 
    ret = eas_mail_handler_sync_folder_email_info(email_handler, 
                                                 folder_sync_key,
                                                 folder_id,	
	                                             emails_created,
	                                             emails_updated,	
	                                             emails_deleted,
	                                             more_available,	
	                                             error);

    // if the call to the daemon returned an error, report and drop out of the test
    if((*error) != NULL){
		fail_if(ret == FALSE,"%s",&(*error)->message);
    } 
//    fail_if(folder_sync_key == 0,
//		"Folder Sync Key not updated by call the exchange server");
}

static void testSendEmail(EasEmailHandler *email_handler,
                                     const gchar *client_id,
                                     const gchar *mime_file,
                                     GError **error){
	gboolean ret = FALSE;									 
    ret = eas_mail_handler_send_email(email_handler, 	
                                      client_id,
                                      mime_file,
	                                  &(*error));

    // if the call to the daemon returned an error, report and drop out of the test
    if((*error) != NULL){
		fail_if(ret == FALSE,"%s",&(*error)->message);
	}                                       
	// TODO - what does success look like for sent email when automated?
}


START_TEST (test_eas_mail_handler_read_email_metadata)
{
    const gchar* accountuid = "123456789";
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0\0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0\0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;

	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

	// TODO - send the email we're expecting to verify below rather than relying on it being created manually

    mark_point();
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);
    mark_point();

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    mark_point();
    testGetFolderInfo(email_handler,
                      folder_sync_key, 
                      "5", // Inbox
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
        email = g_slist_nth(emails_created, 0)->data;
        fail_if(!email, "Unable to get first email in emails_created GSList");

		// verify that it's unread
		fail_if(email->flags & EAS_EMAIL_READ, "First Email in Inbox is expected to be unread");

		// TODO read the expected metadata from config
		        
		//verify that it has a single attachment_separator
		fail_if(! (g_slist_length(email->attachments) == 1), "First Email in Inbox is expected to have a single attachment");
	
		//verify that it's marked with high importance
		headers = email->headers;
		for (headers = email->headers; headers != NULL; headers = headers->next)
		{
			header = headers->data;
			if(!g_strcmp0((char *)header->name, "Importance"))
			{
				importance = atoi(header->value);
				g_debug("Importance = %d", importance);
				break;
			}
		}

		fail_unless(importance == 2);
		
	}

	g_object_unref(email_handler);
	return;
}
END_TEST

START_TEST (test_eas_mail_handler_update_email)
{
    const gchar* accountuid = "123456789";
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0\0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0";
	GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;    
    gboolean more_available = FALSE; 

	// get a handle to the DBus interface 
    testGetMailHandler(&email_handler, accountuid);
    
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

	// Get folder email info for Inbox: 
    testGetFolderInfo(email_handler, folder_sync_key, "5", &emails_created, &emails_updated, &emails_deleted, &more_available, &error);

	// if the emails_created list contains email
	if(emails_created)
	{
		EasEmailInfo *email = NULL;
		gboolean rtn = FALSE;
		GSList *emails = NULL;
		GSList *emails2_created = NULL;	//can't reuse emails_created since that isn't empty now
        
		// get email info for first email in the folder and see if it's been read
		email = (g_slist_nth(emails_created, 0))->data;

		// toggle the read flag:
		if(email->flags & EAS_EMAIL_READ)
		{
			email->flags &= ~EAS_EMAIL_READ;
		}
		else
		{
			email->flags |= EAS_EMAIL_READ;
		}

		// TODO - something a bit more exciting than toggle of read flag (add/remove a category)?		

		mark_point();

		emails = g_slist_append(emails, email);

		// update the first mail in the folder
		rtn = eas_mail_handler_update_email(email_handler, folder_sync_key, "5", emails, &error);
		if(error){
			fail_if(rtn == FALSE,"%s",error->message);
		}

		mark_point();

		// verify that we get updates with the next sync:
		testGetFolderInfo(email_handler, folder_sync_key, "5", &emails2_created, &emails_updated, &emails_deleted, &more_available, &error);

		fail_if(emails2_created, "Not expecting any new emails");
		fail_if(emails_deleted, "Not expecting any deletions");
		fail_unless(emails_updated, "No updates from exchange server sync after we updated");		
		        
		// free email object list before reusing
		g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
		g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
		g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);

		g_slist_free(emails_deleted);
		g_slist_free(emails_updated);
		g_slist_free(emails_created);	

		emails_deleted = NULL;
		emails_updated = NULL;
		emails_created = NULL;
	}
		
    //  free email objects in lists of email objects
    g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);
		                                             
	//  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);	

	g_object_unref(email_handler);
	
}
END_TEST

START_TEST (test_eas_mail_handler_send_email)
{
    EasEmailHandler *email_handler = NULL;
	const gchar *mime_file = g_strconcat(getenv("HOME"), "/int07/testdata/mime_file.txt", NULL);
	const gchar client_id[21];	
    const gchar* accountuid = "123456789";
    GError *error = NULL;
	guint random_num;
	/* initialize random generator */
	srand ( time(NULL) );
	random_num = rand();

	snprintf(client_id, sizeof(client_id)/sizeof(client_id[0]), "%d", random_num);	

	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

	mark_point();
	
	// call into the daemon to send email to the exchange server
	testSendEmail(email_handler, client_id, mime_file, &error);	
    g_free(mime_file);

	g_object_unref(email_handler);
}
END_TEST

START_TEST (test_get_mail_handler)
{
    const gchar* accountuid = "123456789";
    EasEmailHandler *email_handler = NULL;
  
    testGetMailHandler(&email_handler, accountuid);  

	g_object_unref(email_handler);
}
END_TEST

START_TEST (test_get_init_eas_mail_sync_folder_hierarchy)
{
	// This value needs to make sense in the daemon.  in the first instance
	// it should be hard coded to the value used by the daemon but later 
	// there should be a mechanism for getting the value from the same place
	// that the daemon uses
    const gchar* accountuid = "123456789";
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar sync_key[64] = "0\0";
    GError *error = NULL;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

	mark_point();
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, sync_key, &created, &updated, &deleted,&error);
		
    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");
		
	//  free everything!
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);

	g_object_unref(email_handler);
}
END_TEST

START_TEST (test_get_eas_mail_info_in_folder)
{
    const gchar* accountuid = "123456789";
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
    testGetMailHandler(&email_handler, accountuid);
    
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);
    
    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    
    // get the object for the first folder returned in the hierarchy
    folder = g_slist_nth_data(created, 0);
    fail_if(folder ==NULL, "Folder is null");
    // get the folder info for the first folder returned in the hierarchy
    testGetFolderInfo(email_handler,folder_sync_key,"5",&emails_created,&emails_updated,&emails_deleted,&more_available,&error);
	
	//  free email objects in lists of email objects
    g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);
		                                             
	//  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);

	g_object_unref(email_handler);	
}
END_TEST

START_TEST (test_get_eas_mail_info_in_inbox)
{
    const gchar* accountuid = "123456789";
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
    testGetMailHandler(&email_handler, accountuid);

	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);
    
    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    // get the folder info for the inbox 
    testGetFolderInfo(email_handler,folder_sync_key,"5",&emails_created,&emails_updated,&emails_deleted,&more_available,&error);
	
	//  free email objects in lists of email objects
    g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);
		                                             
	//  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);

	g_object_unref(email_handler);	
	
}
END_TEST


START_TEST (test_eas_mail_handler_fetch_email_body)
{
    const gchar* accountuid = "123456789";
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0\0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0\0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
	EasFolder *folder = NULL;
	gboolean testMailFound = FALSE;
	guint folderIndex;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);


    mark_point();
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);
    mark_point();

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    mark_point();
    testGetFolderInfo(email_handler,
                      folder_sync_key, 
                      "5", // Inbox
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
        gchar *mime_directory = g_strconcat(getenv("HOME"), "/mimeresponses/", NULL);
        gchar mime_file[256];
        FILE *hBody = NULL;

        mark_point();
        // get body for first email in the folder
        email = g_slist_nth(emails_created, 0)->data;
        fail_if(!email, "Unable to get first email in emails_created GSList");

        // destination directory for the mime file

        mark_point();
        strcpy(mime_file, mime_directory);
        fail_if(!email->server_id, "Email has no server_id!");
        strcat(mime_file, email->server_id);

        mark_point();
        // check if the email body file exists
        hBody = fopen(mime_file,"r");
        if (hBody) 
        {
            // if the email body file exists delete it
            fclose(hBody);
            hBody = NULL;
            remove(mime_file);
            hBody = fopen(mime_file,"r");
            if(hBody)
            {
                fclose(hBody);
                fail_if(1,"unable to clear existing body file");
            }
        }

        mark_point();
        // call method to get body
        rtn = eas_mail_handler_fetch_email_body(email_handler,
                                                "5", // Inbox
                                                email->server_id,
                                                mime_directory,
                                                &error);
        g_free(mime_directory);

        if(rtn == TRUE)
        {
            testMailFound = TRUE;
            // if reported success check if body file exists
            hBody = fopen (mime_file,"r");
            fail_if (hBody == NULL,"email body file not created in specified directory");
            fclose (hBody);
        }
        else
        {
            fail_if(1,"%s",error->message);
        }
    }
    else
    {
        fail_unless(emails_created, "Emails created is NULL for fetch_email_body");
    }


    // fail the test if there is no email in the folder hierarchy as this means the 
    // test has not exercised the code to get the email body as required by this test case
    fail_if(testMailFound == FALSE,"no mail found in the folder hierarchy");

    //  free email objects in lists of email objects
    g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
	g_slist_free(updated);	
	
	g_object_unref(email_handler);
}
END_TEST

START_TEST (test_eas_mail_handler_fetch_email_attachments)
{
    const gchar* accountuid = "123456789";
    EasEmailHandler *email_handler = NULL;
    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64] = "0\0";
    GError *error = NULL;
    gchar folder_sync_key[64] = "0\0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
	EasFolder *folder = NULL;
	gboolean testMailFound = FALSE;
	guint folderIndex;
    gboolean attachmentFound =FALSE;
    EasEmailInfo *email = NULL;
    gint number_of_emails_created = 0;
    gint i = 0; 
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

    mark_point();
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);
    mark_point();

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    mark_point();
    testGetFolderInfo(email_handler,
                      folder_sync_key, 
                      "5", // Inbox
                      &emails_created,
                      &emails_updated,
                      &emails_deleted,
                      &more_available,
                      &error);
    mark_point();

    //Search for an attachment in emails
    number_of_emails_created = g_slist_length(emails_created);
    for( i; i<number_of_emails_created; i++)
        {
        email = g_slist_nth(emails_created, i)->data;
        if ( g_slist_length(email->attachments) )
            {
            attachmentFound =TRUE;
            break;
            }
        }
       
  
    // if the emails_created list contains an email with attachment
    if (attachmentFound)
    {
  	EasAttachment *attachmentObj = NULL; 
 	gchar* file_reference = NULL;
    gboolean rtn = FALSE;

    mark_point();
    //Get all attachments for every email_created.
    i=0;
    for(i; i<number_of_emails_created; i++)
        {
        gint number_of_attachments = 0;
        gint j =0;
            
        email = g_slist_nth(emails_created, i)->data;
        //fail_if(!email, "Unable to get first email in emails_created GSList");
        number_of_attachments = g_slist_length(email->attachments);
        for  (j; j< number_of_attachments; j++)
            {
            gchar *mime_directory = g_strconcat(getenv("HOME"), "/mimeresponses/", NULL);
            gchar mime_file[256];
            FILE *hAttachment = NULL;
                
            attachmentObj = g_slist_nth(email->attachments, j)->data;
             fail_if(attachmentObj->file_reference == NULL, "attachment has no file_reference!");

            // destination directory for the mime file

            mark_point();
            strcpy(mime_file, mime_directory);
            strcat(mime_file, attachmentObj->file_reference);

            mark_point();
            // check if the email body file exists
            hAttachment = fopen(mime_file,"r");
            if (hAttachment) 
                {
                    // if the email body file exists delete it
                    fclose(hAttachment);
                    hAttachment = NULL;
                    remove(mime_file);
                    hAttachment = fopen(mime_file,"r");
                    if(hAttachment)
                        {
                        fclose(hAttachment);
                        fail_if(1,"unable to clear existing attachment file");
                        }
                }

            mark_point();
            // call method to get attachment
            rtn = eas_mail_handler_fetch_email_attachment(email_handler, 
                                                    attachmentObj->file_reference, 	
                                                    mime_directory,	 // "$HOME/mimeresponses/"
                                                    &error);

            g_free(mime_directory);

            if(rtn == TRUE)
                {
                testMailFound = TRUE;
                // if reported success check if attachment file exists
                hAttachment = fopen (mime_file,"r");
                fail_if (hAttachment == NULL,"email attachment file not created in specified directory");
                fclose (hAttachment);
                }
           else
                {
                fail_if(1,"%s",error->message);
                }
                
            } //-end of attachment loop
        }//-end of emails_created loop
    }

    //  free email objects in lists of email objects
    g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);

    //  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
   
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);	

	g_object_unref(email_handler);
}
END_TEST

START_TEST (test_eas_mail_handler_delete_email)
{
    const gchar* accountuid = "123456789";
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
    gchar folder_sync_key[64];
	strcpy(folder_sync_key,"0");
	GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;    
    gboolean more_available = FALSE;  
	EasFolder *folder = NULL;
	gboolean testMailFound = FALSE;
	guint folderIndex;
	
	// get a handle to the DBus interface 
    testGetMailHandler(&email_handler, accountuid);

	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

	// loop through the folders in the hierarchy to find a folder with an email in it
	for(folderIndex = 0;g_slist_length(created);folderIndex++){
		// get the folder info for the current folderIndex
		// since the sync id is zero only the created list will contain folders
	    folder = (g_slist_nth(created, folderIndex))->data;
        testGetFolderInfo(email_handler,folder_sync_key,"5",&emails_created,&emails_updated,&emails_deleted,&more_available,&error);

		// if the emails_created list contains email
		if(emails_created){
			GSList *emailToDel = NULL;
			gchar serveridToDel[64];
			EasEmailInfo *email = NULL;
			gboolean rtn = FALSE;
			EasEmailInfo *reportedDeletedEmail = NULL;			

    		// get email info for first email in the folder
			email = (g_slist_nth(emails_created, 0))->data;

			emailToDel = g_slist_append(emailToDel, g_strdup(email->server_id));
			g_stpcpy(serveridToDel,email->server_id);

			// delete the first mail in the folder
			rtn = eas_mail_handler_delete_email(email_handler, folder_sync_key,"5", emailToDel,&error);
			if(error){
				fail_if(rtn == FALSE,"%s",error->message);
			}

			
			// free email object list before reusing
			g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
			g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
			g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);

			g_slist_free(emails_deleted);
			g_slist_free(emails_updated);
			g_slist_free(emails_created);	

			emails_deleted = NULL;
			emails_updated = NULL;
			emails_created = NULL;
			
			// get email info for the folder using the saved sync key
			testGetFolderInfo(email_handler,folder_sync_key,"5",&emails_created,&emails_updated,&emails_deleted,&more_available,&error);
			
			fail_unless(emails_deleted,"No email reported as deleted");
			
			reportedDeletedEmail = (g_slist_nth(emails_deleted, 0))->data;
			
			// fail the test if the server_id for the mail reported as deleted 
			// does not match the server_id of the email for which the 
			// eas_mail_handler_delete_email call was made
			fail_if(g_strcmp0(reportedDeletedEmail->server_id,serveridToDel), 
			    "Deleted email not reported back by Exchange server as deleted. deleted id was %s and reported id was %s",
			        reportedDeletedEmail->server_id,serveridToDel);

			g_slist_free(emailToDel);
			
			// after getting the body for the first mail, drop out of the loop
			testMailFound = TRUE;
			break;
		}
		// else, go round the loop again
	}
	
	// fail the test if there is no email in the folder hierarchy as this means the 
	// test has not exercised the code to get the email body as required by this test case
	fail_if(testMailFound == FALSE,"no mail found in the folder hierarchy");
		
    //  free email objects in lists of email objects
    g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
    g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);
		                                             
	//  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);	

	g_object_unref(email_handler);	
	
}
END_TEST

Suite* eas_libeasmail_suite (void)
{
  Suite* s = suite_create ("libeasmail");

  /* libeasmail test case */
  TCase *tc_libeasmail = tcase_create ("core");
  suite_add_tcase (s, tc_libeasmail);

  tcase_add_test (tc_libeasmail, test_get_mail_handler);
  tcase_add_test (tc_libeasmail, test_get_init_eas_mail_sync_folder_hierarchy);
  tcase_add_test (tc_libeasmail, test_get_eas_mail_info_in_inbox);
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_fetch_email_body);
  //tcase_add_test (tc_libeasmail, test_get_eas_mail_info_in_folder); // only uncomment this test if the folders returned are filtered for email only
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_fetch_email_attachments);
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_delete_email);
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_send_email);
  //need an unread, high importance email with a single attachment at top of inbox for this to pass:
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_read_email_metadata);
  // need at least one email in the inbox for this to pass:
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_update_email);		

  return s;
}
