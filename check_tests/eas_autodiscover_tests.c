#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>

#include "../eas-daemon/libeas/eas-connection.h"
#include "../eas-daemon/libeas/eas-connection-errors.h"

static void
test_no_email_or_password_cb(char* serverUri, gpointer data, GError *error)
{
    fail_if(serverUri != NULL, "serverUri expected to be NULL");
    fail_if(data != NULL, "Data expected to be NULL");
    fail_if(NULL == error, "Expecting valid error response");

    if (error)
    {
        fail_if(error->domain != EAS_CONNECTION_ERROR, "Incorrect Error Domain");
        fail_if(error->code != EAS_CONNECTION_ERROR_FAILED, "Incorrect Error Code");
        fail_if(error->message == NULL, "Expected an error message");
        g_message(error->message);
        g_error_free(error);
    }
}

START_TEST (test_no_email_or_password)
{
    gchar* email = NULL;
    gchar* password = NULL;

    mark_point();
    eas_connection_autodiscover(test_no_email_or_password_cb,
                                NULL,
                                email,
                                NULL,
                                password);
    mark_point();
}
END_TEST

START_TEST (test_no_email)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_no_password)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_good_email_bad_password)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_good_username_bad_password)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_bad_email_good_password)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_bad_username_good_password)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_good_email_and_password_no_username)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST

START_TEST (test_good_email_username_and_password)
{
    fail_if(TRUE, "TODO Test case still unwritten");
}
END_TEST


Suite* eas_autodiscover_suite (void)
{
    Suite* s = suite_create ("autodiscover");

    /* tc_autodiscover test case */
    TCase *tc_autodiscover = tcase_create ("core");

    suite_add_tcase (s, tc_autodiscover);

    tcase_add_test (tc_autodiscover, test_no_email_or_password);

    tcase_add_test (tc_autodiscover, test_no_email);
    tcase_add_test (tc_autodiscover, test_no_password);

    tcase_add_test (tc_autodiscover, test_good_email_bad_password);
    tcase_add_test (tc_autodiscover, test_good_username_bad_password);
    
    tcase_add_test (tc_autodiscover, test_bad_email_good_password);
    tcase_add_test (tc_autodiscover, test_bad_username_good_password);

    tcase_add_test (tc_autodiscover, test_good_email_and_password_no_username);
    tcase_add_test (tc_autodiscover, test_good_email_username_and_password);

    return s;
}
