#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "../libeasmail/src/libeasmail.h"
#include "../libeasmail/src/eas-folder.h"
#include "../libeasmail/src/eas-email-info.h"
#include "../libeasmail/src/eas-attachment.h"


static void testGetMailHandler(EasEmailHandler **email_handler, guint64 accountuid){
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
    ret  = eas_mail_handler_sync_folder_hierarchy(email_handler, sync_key, 	
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
	// therefore the key must not be zero as this is the seed value for the tests       									 
    fail_unless(strcmp(sync_key,"0"),
		"Sync Key not updated by call the exchange server");
		
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
	                                             &(*emails_created),
	                                             &(*emails_updated),	
	                                             &(*emails_deleted),
	                                             more_available,	
	                                             &(*error));

    // if the call to the daemon returned an error, report and drop out of the test
    if((*error) != NULL){
		fail_if(ret == FALSE,"%s",&(*error)->message);

    fail_if(folder_sync_key == 0,
		"Folder Sync Key not updated by call the exchange server");
	fail_if(g_slist_length(*emails_created)==0, "no emails added");
	} 
	
	                                       
}

START_TEST (test_get_mail_handler)
{
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
  
    testGetMailHandler(&email_handler, accountuid);  
}
END_TEST

START_TEST (test_get_init_eas_mail_sync_folder_hierarchy)
{
	// This value needs to make sense in the daemon.  in the first instance
	// it should be hard coded to the value used by the daemon but later 
	// there should be a mechanism for getting the value from the same place
	// that the daemon uses
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

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
	testGetFolderHierarchy(email_handler,sync_key,&created,&updated,&deleted,&error);
		
    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");
		
	//  free everything!
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);
}
END_TEST

START_TEST (test_get_eas_mail_info_in_folder)
{
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar *folder_hierarchy_sync_key = "0";
    GError *error = NULL;

	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);
    
    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    gchar *folder_sync_key = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;    
    EasFolder *folder = NULL;
    gboolean more_available = FALSE;
    
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
}
END_TEST

START_TEST (test_eas_mail_handler_fetch_email_body)
{
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar *folder_hierarchy_sync_key = "0";
    GError *error = NULL;

	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    gchar *folder_sync_key = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;
    gboolean more_available = FALSE;
	EasFolder *folder = NULL;
	gboolean testMailFound = FALSE;
	guint folderIndex;
    gboolean useFakeData = FALSE;

    mark_point();

    testGetFolderInfo(email_handler,
                      folder_sync_key, 
                      "5", // Inbox
                      &emails_created,
                      &emails_updated,
                      &emails_deleted,
                      &more_available,
                      &error);

    if (!emails_created) 
    {
        useFakeData = TRUE;
    }

    // if the emails_created list contains email
    if (TRUE/* emails_created */)
    {
        EasEmailInfo *email = NULL;
        gboolean rtn = FALSE;

        // get body for first email in the folder
        if (!useFakeData) 
        {
            email = g_slist_nth(emails_created, 0);
        }

        // destination directory for the mime file
        gchar *mime_directory = "/eastests/";
        gchar mime_file[256];

        strcpy(mime_file, mime_directory);
        strcat(mime_file, (useFakeData?"5:1":email->server_id));

        mark_point();
        // check if the email body file exists
        FILE *hBody = NULL;
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
                                                "5"/*folder->folder_id*/, // Inbox
                                                (useFakeData?"5:1":email->server_id),
                                                mime_directory,
                                                &error);
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
}
END_TEST

START_TEST (test_eas_mail_handler_fetch_email_attachments)
{
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
	// get a handle to the DBus interface and associate the account ID with 
	// this object 
    testGetMailHandler(&email_handler, accountuid);

    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar *folder_hierarchy_sync_key = "0";
    GError *error = NULL;

	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    gchar *folder_sync_key = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;    
    gboolean more_available = FALSE;
	EasFolder *folder = NULL;
	guint folderIndex;
	gboolean attachmentFound = FALSE;

	// loop through the folders in the hierarchy to find a folder with an email in it
	for(folderIndex = 0;folderIndex < g_slist_length(created);folderIndex++){
		// get the folder info for the current folderIndex
		// since the sync id is zero only the created list will contain folders
	    folder = g_slist_nth(created, folderIndex);
        testGetFolderInfo(email_handler,folder_sync_key,folder->folder_id,&emails_created,&emails_updated,&emails_deleted,&more_available,&error);

		// if the emails_created list contains email
    	// loop through the emails in the folder untill we find a mail with attachment
		if(emails_created){
			EasEmailInfo *email = NULL;
			gboolean rtn = FALSE;
            guint mailIndex;
            
            for(mailIndex = 0;mailIndex <g_slist_length(emails_created);mailIndex++){
        		// get header info for next email in the folder
	    		email = g_slist_nth(emails_created, mailIndex);
	    		
	    		if(email->attachments){
					EasAttachment *attachment = NULL;
					attachment = g_slist_nth(email->attachments, 0);
					
					// destination directory for the attachment file
					gchar *mime_directory = "/eastests/";
					gchar attachment_file[256];

					strcpy(attachment_file,mime_directory);
					strcat(attachment_file,attachment->file_reference);
					
					// check if the attachment file exists
					FILE *hAtt = NULL;
					hAtt = fopen(attachment_file,"r");
					if(hAtt){
						// if the attachment file exists delete it
						fclose(hAtt);
						hAtt = NULL;
						remove(attachment_file);
						hAtt = fopen(attachment_file,"r");
						if(hAtt){
							fclose(hAtt);
							fail_if(1,"unable to clear existing attachment file");
						}
					}
					// call method to get attachment
					rtn = eas_mail_handler_fetch_email_attachment(email_handler,attachment->file_reference,mime_directory,&error);
					if(rtn == TRUE){		
						// if reported success check if attachment exists
						hAtt = fopen(attachment_file,"r");
						fail_if(hAtt == NULL,"attachment file not created in specified directory");
						fclose(hAtt);		
					}	
					else{
						fail_if(1,"%s",error->message);
					}
					// after getting the body for the first mail, drop out of the loop
					attachmentFound = TRUE;
					break;
				}
				if(attachmentFound)
				    break;
			}
		}
		// else, go round the loop again
	}
	// fail the test if there attachments in the folder hierarchy as this means the 
	// test has not exercised the code to get an attachment as required by this test case
	fail_if(attachmentFound == FALSE,"no mail found in the folder hierarchy");

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
}
END_TEST

START_TEST (test_eas_mail_handler_delete_email)
{
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
	// get a handle to the DBus interface 
    testGetMailHandler(&email_handler, accountuid);

    // declare lists to hold the folder information returned by active sync
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    // Sync Key set to Zero.  This means that this is the first time the sync is being done,
    // there is no persisted sync key from previous sync's, the returned information will be 
    // the complete folder hierarchy rather than a delta of any changes
    gchar folder_hierarchy_sync_key[64];
	strcpy(folder_hierarchy_sync_key,"0");
    GError *error = NULL;
	
	// call into the daemon to get the folder hierarchy from the exchange server
	testGetFolderHierarchy(email_handler, folder_hierarchy_sync_key,&created,&updated,&deleted,&error);

    // fail the test if there is no folder information
	fail_unless(created, "No folder information returned from exchange server");

    gchar *folder_sync_key = "0";
    GSList *emails_created = NULL; //receives a list of EasMails
    GSList *emails_updated = NULL;
    GSList *emails_deleted = NULL;    
    gboolean more_available = FALSE;  
	EasFolder *folder = NULL;
	gboolean testMailFound = FALSE;
	guint folderIndex;

/*
        testGetFolderInfo(email_handler,folder_sync_key,folder->folder_id,&emails_created,&emails_updated,&emails_deleted,&more_available,&error);
			gboolean rtn = FALSE;
			rtn = eas_mail_handler_delete_email(email_handler, folder_sync_key,folder->folder_id, email->server_id,&error);
			if(error){
				fail_if(rtn == FALSE,"%s",error->message);
			}

*/
/*	
	// loop through the folders in the hierarchy to find a folder with an email in it
	for(folderIndex = 0;g_slist_length(created);folderIndex++){
		// get the folder info for the current folderIndex
		// since the sync id is zero only the created list will contain folders
	    folder = g_slist_nth(created, folderIndex);
        testGetFolderInfo(email_handler,folder_sync_key,folder->folder_id,&emails_created,&emails_updated,&emails_deleted,&more_available,&error);

		// if the emails_created list contains email
		if(emails_created){
			EasEmailInfo *email = NULL;
			gboolean rtn = FALSE;

    		// get email info for first email in the folder
			email = g_slist_nth(emails_created, 0);
			
			// delete the first mail in the folder
			rtn = eas_mail_handler_delete_email(email_handler, folder_sync_key,folder->folder_id, email->server_id,&error);
			if(error){
				fail_if(rtn == FALSE,"%s",error->message);
			}
			
			// free email object list before reusing
			g_slist_foreach(emails_deleted, (GFunc)g_object_unref, NULL);
			g_slist_foreach(emails_updated, (GFunc)g_object_unref, NULL);
			g_slist_foreach(emails_created, (GFunc)g_object_unref, NULL);
			
			// get email info for the folder using the saved sync key
			testGetFolderInfo(email_handler,folder_sync_key,folder->folder_id,&emails_created,&emails_updated,&emails_deleted,&more_available,&error);
			
			fail_if(emails_deleted,"No email reported as deleted");
			
			EasEmailInfo *deletedEmail = NULL;			
			deletedEmail = g_slist_nth(emails_deleted, 0);
			
			// fail the test if the server_id for the mail reported as deleted 
			// does not match the server_id of the email for which the 
			// eas_mail_handler_delete_email call was made
			fail_if(strcmp(email->server_id,deletedEmail->server_id), 
			    "Deleted email not reported back by Exchange server as deleted");
			
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
	*/	                                             
	//  free folder objects in lists of folder objects
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);	
	
}
END_TEST


START_TEST (test_delete_email)
{
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;	
	
	// get a handle to the DBus interface 
    testGetMailHandler(&email_handler, accountuid);

	EasEmailInfo email;
	gboolean rtn = FALSE;
	GError *error = NULL;
	char synckey[9];
	strcpy(synckey,"9");
	    
	rtn = eas_mail_handler_delete_email(email_handler, synckey,"bob", "bert",&error);
	if(error){
		fail_if(rtn == FALSE,"%s",error->message);
	}
	
	
}
END_TEST

Suite* eas_libeasmail_suite (void)
{
  Suite* s = suite_create ("libeasmail");

  /* libeasmail test case */
  TCase *tc_libeasmail = tcase_create ("core");
  suite_add_tcase (s, tc_libeasmail);
  //tcase_add_test (tc_libeasmail, test_get_mail_handler);
  //tcase_add_test (tc_libeasmail, test_get_init_eas_mail_sync_folder_hierarchy);
  //tcase_add_test (tc_libeasmail, test_get_eas_mail_info_in_folder);
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_fetch_email_body);
  //tcase_add_test (tc_libeasmail, test_eas_mail_handler_fetch_email_attachments);
  tcase_add_test (tc_libeasmail, test_eas_mail_handler_delete_email);
  //tcase_add_test (tc_libeasmail, test_delete_email);

  return s;
}
