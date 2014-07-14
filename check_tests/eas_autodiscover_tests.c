#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>
#include <libedataserver/e-flag.h>

#include "../eas-daemon/libeas/eas-connection.h"
#include "../eas-daemon/libeas/eas-connection-errors.h"

#define EMAIL    0
#define PASSWORD 1
#define USERNAME 2

enum EasAdTestcases
{
    TC_ALL_NULL = 0,
    TC_NULL_EMAIL,
    TC_NULL_PASSWORD,

    TC_GOOD_EMAIL_PASSWORD_BAD_USER,
    TC_GOOD_EMAIL_BAD_PASSWORD_NULL_USER,
    TC_GOOD_EMAIL_USER_BAD_PASSWORD,
    TC_GOOD_EMAIL_BAD_PASSWORD_USER,

    TC_GOOD_PASSWORD_USER_BAD_EMAIL,
    TC_GOOD_EMAIL_PASSWORD_NULL_USER,
    TC_GOOD_EMAIL_PASSWORD_USER
};

// email, pw, username
const gchar* testdata[][3] =
{
    {0, 0, 0},
    {0, "G00dP@55w0rd", 0},
    {"good.user@cstylianou.com", 0, 0},

    {"good.user@cstylianou.com", "G00dP@55w0rd", "badUser"},
    {"good.user@cstylianou.com", "badPassword", 0},
    {"good.user@cstylianou.com", "badPassword", "good.user"},
    {"good.user@cstylianou.com", "badPassword", "badUser"},

    {"bad.email@cstylianou.com", "G00dP@55w0rd", "good.user"},
    {"good.user@cstylianou.com", "G00dP@55w0rd", 0},
    {"good.user@cstylianou.com", "G00dP@55w0rd", "good.user"}
};

#define MAX_TESTS (sizeof(testdata)/sizeof(testdata[0]))

static GMainLoop *loop = NULL;

/*static void
test_expects_error_cb (char* serverUri, gpointer data, GError *error)
{
    g_debug ("Error response for test [%p]", data);

    if (error)
    {
        g_debug ("%s", error->message);
        fail_if (error->domain != EAS_CONNECTION_ERROR, "Incorrect Error Domain");
        fail_if (error->code != EAS_CONNECTION_ERROR_FAILED, "Incorrect Error Code");
        fail_if (error->message == NULL, "Expected an error message");
        g_error_free (error);
    }

    fail_if (serverUri != NULL, "serverUri expected to be NULL");
    fail_if (NULL == error, "Expecting valid error response");

    g_main_loop_quit (loop);
    g_main_loop_unref (loop);
    loop = NULL;
}*/

START_TEST (test_all_null)
{
/*    const gchar* email    = testdata[TC_ALL_NULL][EMAIL];
    const gchar* password = testdata[TC_ALL_NULL][PASSWORD];
    const gchar* username = testdata[TC_ALL_NULL][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_all_null");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_ALL_NULL,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_null_email)
{
/*    const gchar* email    = testdata[TC_NULL_EMAIL][EMAIL];
    const gchar* password = testdata[TC_NULL_EMAIL][PASSWORD];
    const gchar* username = testdata[TC_NULL_EMAIL][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_null_email");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_NULL_EMAIL,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_good_email_password_bad_user)
{
/*    const gchar* email    = testdata[TC_GOOD_EMAIL_PASSWORD_BAD_USER][EMAIL];
    const gchar* password = testdata[TC_GOOD_EMAIL_PASSWORD_BAD_USER][PASSWORD];
    const gchar* username = testdata[TC_GOOD_EMAIL_PASSWORD_BAD_USER][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_email_password_bad_user");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_GOOD_EMAIL_PASSWORD_BAD_USER,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_good_email_bad_password_null_user)
{
/*    const gchar* email    = testdata[TC_GOOD_EMAIL_BAD_PASSWORD_NULL_USER][EMAIL];
    const gchar* password = testdata[TC_GOOD_EMAIL_BAD_PASSWORD_NULL_USER][PASSWORD];
    const gchar* username = testdata[TC_GOOD_EMAIL_BAD_PASSWORD_NULL_USER][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_email_bad_password_null_user");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_GOOD_EMAIL_BAD_PASSWORD_NULL_USER,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_good_email_user_bad_password)
{
/*    const gchar* email    = testdata[TC_GOOD_EMAIL_USER_BAD_PASSWORD][EMAIL];
    const gchar* password = testdata[TC_GOOD_EMAIL_USER_BAD_PASSWORD][PASSWORD];
    const gchar* username = testdata[TC_GOOD_EMAIL_USER_BAD_PASSWORD][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_email_user_bad_password");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_GOOD_EMAIL_USER_BAD_PASSWORD,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_good_email_bad_password_user)
{
/*    const gchar* email    = testdata[TC_GOOD_EMAIL_BAD_PASSWORD_USER][EMAIL];
    const gchar* password = testdata[TC_GOOD_EMAIL_BAD_PASSWORD_USER][PASSWORD];
    const gchar* username = testdata[TC_GOOD_EMAIL_BAD_PASSWORD_USER][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_email_bad_password_user");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_GOOD_EMAIL_BAD_PASSWORD_USER,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_good_password_user_bad_email)
{
/*    const gchar* email    = testdata[TC_GOOD_PASSWORD_USER_BAD_EMAIL][EMAIL];
    const gchar* password = testdata[TC_GOOD_PASSWORD_USER_BAD_EMAIL][PASSWORD];
    const gchar* username = testdata[TC_GOOD_PASSWORD_USER_BAD_EMAIL][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_password_user_bad_email");
    eas_connection_autodiscover (test_expects_error_cb,
                                 TC_GOOD_PASSWORD_USER_BAD_EMAIL,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

/*static void
test_expects_success_cb (char* serverUri, gpointer data, GError *error)
{
    g_debug ("Found URL:[%p] [%s]", data, serverUri);

    if (error)
    {
        g_debug ("%s", error->message);
        fail_if (error->domain != EAS_CONNECTION_ERROR, "Incorrect Error Domain");
        fail_if (error->code != EAS_CONNECTION_ERROR_FAILED, "Incorrect Error Code");
        fail_if (error->message == NULL, "Expected an error message");
        g_error_free (error);
    }

    fail_if (serverUri == NULL, "Expected Valid Uri");
    fail_if (NULL != error, "Not expecting an error");

    g_main_loop_quit (loop);
    g_main_loop_unref (loop);
    loop = NULL;
}*/

START_TEST (test_good_email_password_null_user)
{
/*    const gchar* email    = testdata[TC_GOOD_EMAIL_PASSWORD_NULL_USER][EMAIL];
    const gchar* password = testdata[TC_GOOD_EMAIL_PASSWORD_NULL_USER][PASSWORD];
    const gchar* username = testdata[TC_GOOD_EMAIL_PASSWORD_NULL_USER][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_email_password_null_user");
    eas_connection_autodiscover (test_expects_success_cb,
                                 TC_GOOD_EMAIL_PASSWORD_NULL_USER,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST

START_TEST (test_good_email_password_user)
{
/*    const gchar* email    = testdata[TC_GOOD_EMAIL_PASSWORD_USER][EMAIL];
    const gchar* password = testdata[TC_GOOD_EMAIL_PASSWORD_USER][PASSWORD];
    const gchar* username = testdata[TC_GOOD_EMAIL_PASSWORD_USER][USERNAME];

    loop = g_main_loop_new (NULL, FALSE);

    mark_point ();
    g_debug ("test_good_email_password_user");
    eas_connection_autodiscover (test_expects_success_cb,
                                 TC_GOOD_EMAIL_PASSWORD_USER,
                                 email,
                                 username);
    mark_point ();
    if (loop) g_main_loop_run (loop);
    mark_point ();*/
}
END_TEST


Suite* eas_autodiscover_suite (void)
{
    Suite* s = suite_create ("autodiscover");

#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif

    /* tc_autodiscover test case */
    TCase *tc_autodiscover = tcase_create ("core");

    suite_add_tcase (s, tc_autodiscover);

    tcase_add_test (tc_autodiscover, test_all_null);
    tcase_add_test (tc_autodiscover, test_null_email);
    
    tcase_add_test (tc_autodiscover, test_good_email_password_null_user);

#if 0
    tcase_add_test (tc_autodiscover, test_good_email_password_bad_user);
    tcase_add_test (tc_autodiscover, test_good_email_bad_password_null_user);
    tcase_add_test (tc_autodiscover, test_good_email_user_bad_password);
    tcase_add_test (tc_autodiscover, test_good_email_bad_password_user);

    tcase_add_test (tc_autodiscover, test_good_password_user_bad_email);

    tcase_add_test (tc_autodiscover, test_good_email_password_user);
#endif
    return s;
}
