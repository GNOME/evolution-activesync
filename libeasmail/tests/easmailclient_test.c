/**
 *  
 *  Filename: easmailclient_test.c
 *  Project: 
 *  Description: 
 *
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "../src/libeasmail.h"
#include "../src/eas-folder.h"
#include "../src/eas-attachment.h"

// display an EasFolderFree 
static void 
EasFolderDisplay(EasFolder *folder)
{
    g_print("EasFolderDisplay++");
    g_print("%s", folder->parent_id);
    g_print("%s", folder->folder_id);
    g_print("%s", folder->display_name);
    g_print("EasFolderDisplay--");
}


/**
 *
 */
int 
main(int argc, char** argv) {	
	
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
    email_handler = eas_mail_handler_new(accountuid);

    g_assert(email_handler != NULL);

	g_print("dbus initialised\n");

	/*
	// temp - test folder 'serialisation'/'deserialisation':
 	gchar *serialised = NULL;
	EasFolder *folder = eas_folder_new();
	
	folder->parent_id = g_strdup("5");	// TODO - test where this is an empty string or NULL!
	//folder->parent_id = NULL;
	//folder->parent_id = g_strdup("");
	folder->folder_id = g_strdup("1");
	folder->display_name = g_strdup("Inbox");
	folder->type = 2;	//EAS_FOLDER_TYPE_DEFAULT_INBOX

	if(!eas_folder_serialise(folder, &serialised))
	{
		g_print("call to eas_folder_serialise failed!");
	}
	else
	{
		g_print("result from serialise: %s\n", serialised);
	}

	g_object_unref(folder); // these don't have to be freed!

	
	EasFolder *folder2 = eas_folder_new();

	eas_folder_deserialise(folder2, serialised);

	g_print("result from serialise:\n");
	g_print("parent_id = %s\n", folder2->parent_id);
	g_print("folder_id = %s\n", folder2->folder_id);
	g_print("display name = %s\n", folder2->display_name);
	g_print("type = %d\n", folder2->type);

	g_object_unref(folder2); // these don't have to be freed!

	g_free(serialised);
	*/

	/*
// temp - test attachment 'serialisation'/'deserialisation':
	EasAttachment *attachment = eas_attachment_new();
	gchar *serialised = NULL;

	attachment->file_reference = g_strdup("\\my\\mime\\path");	
	attachment->display_name = g_strdup("myattachment.txt");// TODO - test where this is an empty string or NULL!
	//attachment->display_name = NULL;
	attachment->estimated_size = 23456789;	//EAS_FOLDER_TYPE_DEFAULT_INBOX

	if(!eas_attachment_serialise(attachment, &serialised))
	{
		g_print("call to eas_attachment_serialise failed!");
	}
	else
	{
		g_print("result from serialise: %s\n", serialised);
	}

	g_object_unref(attachment); 

	EasAttachment *attachment2 = eas_attachment_new();

	eas_attachment_deserialise(attachment2, serialised);

	g_print("result from serialise:\n");
	g_print("file_reference = %s\n", attachment2->file_reference);
	g_print("display name = %s\n", attachment2->display_name);
	g_print("estimated size = %d\n", attachment2->estimated_size);

	g_object_unref(attachment2); // these don't have to be freed!
	
	g_free(serialised);	
	*/

    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    gchar *sync_key = "0";
    GError *error = NULL;
    gboolean ret;

    ret  = eas_mail_handler_sync_folder_hierarchy(email_handler, sync_key, 	
	        &created,	
	        &updated,
	        &deleted,
	        &error);
	          
    g_print("new sync key = %s\n", sync_key);
    
    g_slist_foreach(created, (GFunc)EasFolderDisplay, NULL);

	//  free everything!
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);

	
    return 0;
}

