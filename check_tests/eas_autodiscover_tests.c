#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>
#include <libedataserver/e-flag.h>

#include "../eas-daemon/libeas/eas-connection.h"
#include "../eas-daemon/libeas/eas-connection-errors.h"

EFlag *flag = NULL;

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
        g_message("%s",error->message);
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

static void
test_no_email_cb(char* serverUri, gpointer data, GError *error)
{
    fail_if(serverUri != NULL, "serverUri expected to be NULL");
    fail_if(data != NULL, "Data expected to be NULL");
    fail_if(NULL == error, "Expecting valid error response");

    if (error)
    {
        fail_if(error->domain != EAS_CONNECTION_ERROR, "Incorrect Error Domain");
        fail_if(error->code != EAS_CONNECTION_ERROR_FAILED, "Incorrect Error Code");
        fail_if(error->message == NULL, "Expected an error message");
        g_message("%s",error->message);
        g_error_free(error);
    }
}

START_TEST (test_no_email)
{
    gchar* email = NULL;
    gchar* password = "p@55word";
    gchar* username = "a.username";

    mark_point();
    eas_connection_autodiscover(test_no_email_cb,
                                NULL,
                                email,
                                username,
                                password);
    mark_point();
}
END_TEST

static void
test_no_password_cb(char* serverUri, gpointer data, GError *error)
{
    fail_if(serverUri != NULL, "serverUri expected to be NULL");
    fail_if(data != NULL, "Data expected to be NULL");
    fail_if(NULL == error, "Expecting valid error response");

    if (error)
    {
        fail_if(error->domain != EAS_CONNECTION_ERROR, "Incorrect Error Domain");
        fail_if(error->code != EAS_CONNECTION_ERROR_FAILED, "Incorrect Error Code");
        fail_if(error->message == NULL, "Expected an error message");
        g_message("%s",error->message);
        g_error_free(error);
    }
}

START_TEST (test_no_password)
{
    gchar* email = "a.username@email.com";
    gchar* password = NULL;
    gchar* username = "a.username";

    mark_point();
    eas_connection_autodiscover(test_no_password_cb,
                                NULL,
                                email,
                                username,
                                password);
    mark_point();
}
END_TEST

Suite* eas_autodiscover_suite (void)
{
    Suite* s = suite_create ("autodiscover");

    g_type_init();

    /* tc_autodiscover test case */
    TCase *tc_autodiscover = tcase_create ("core");

    suite_add_tcase (s, tc_autodiscover);

    tcase_add_test (tc_autodiscover, test_no_email_or_password);
    tcase_add_test (tc_autodiscover, test_no_email);
    tcase_add_test (tc_autodiscover, test_no_password);

    return s;
}
