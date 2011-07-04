#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "eas_test_user.h"
#include "../libeasmail/src/libeasmail.h"
#include "../libeasmail/src/eas-folder.h"

extern gchar * g_account_id;
Suite* eas_folderhierarchy_suite (void);

static void
log_and_free (gpointer data, gpointer user_data)
{
    EasFolder *item = data;

    if (item)
    {
        g_message("%s:%u:%s:%s", 
                item->display_name, 
                item->type, 
                item->folder_id, 
                item->parent_id);
        
        g_object_unref (item);
    }
}

START_TEST (test_get_folder_hierarchy)
{
    GSList *added = NULL, *updated = NULL, *deleted = NULL;
    EasEmailHandler *email_handler = NULL;
    gchar syncKey[64] = "1";
    GError *error = NULL;
    gboolean result = FALSE;

    email_handler = eas_mail_handler_new (g_account_id, &error);

    if (!email_handler)
    {
        g_critical("Failed to create email handler [%d:%s]",
                   error->code, error->message);
        g_error_free (error);
    }

    result = eas_mail_handler_sync_folder_hierarchy (email_handler,
                                                     syncKey,
                                                     &added,
                                                     &updated,
                                                     &deleted,
                                                     &error);

    if (!result)
    {
        g_critical("Failed to sync folder hierarchy [%d:%s]",
                   error->code, error->message);
        g_error_free (error);
    }

    g_message("Updated sync key = [%s]", syncKey);

    g_message("======== Added   ========");
    g_slist_foreach (added,   (GFunc)log_and_free, NULL);
    g_message("======== Updated ========");
    g_slist_foreach (updated, (GFunc)log_and_free, NULL);
    g_message("======== Deleted ========");
    g_slist_foreach (deleted, (GFunc)log_and_free, NULL);

    g_slist_free (added);
    g_slist_free (updated);
    g_slist_free (deleted);
}
END_TEST

Suite* eas_folderhierarchy_suite (void)
{
    Suite* s = suite_create ("folderhierarchy");
    TCase *tc_folderhierarchy = tcase_create ("core");
    
    suite_add_tcase (s, tc_folderhierarchy);

    tcase_add_test (tc_folderhierarchy, test_get_folder_hierarchy);

    return s;
}
