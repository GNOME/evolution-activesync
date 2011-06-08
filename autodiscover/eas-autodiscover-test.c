#include <stdio.h>
#include <stdlib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "../eas-daemon/libeas/eas-connection.h"

static GMainLoop *loop = NULL;

#define EMAIL    0
#define PASSWORD 1
#define USERNAME 2

enum EasAdTestcases {
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
gchar* testdata[][3] = { 
                         {0,0,0},
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
static int g_test_index = 0;

#define MAX_TESTS (sizeof(testdata)/sizeof(testdata[0]))

static void autodiscover_cb(char* serverUri, gpointer data, GError *error);


static gboolean 
try_autodiscover(gpointer data) 
{
    gchar *email    = testdata[g_test_index][EMAIL];
    gchar *password = testdata[g_test_index][PASSWORD];
    gchar *username = testdata[g_test_index][USERNAME];

    g_message("Autodiscover - TESTCASE[%d]",g_test_index);

    eas_connection_autodiscover(autodiscover_cb,
                                NULL,
                                email,
                                username,
                                password);

    return FALSE;
}

static void
autodiscover_cb(char* serverUri, gpointer data, GError *error)
{
    g_message("autodiscover_cb++");

    switch(g_test_index)
    {
        case TC_ALL_NULL:
        case TC_NULL_EMAIL:
        case TC_NULL_PASSWORD:
        case TC_GOOD_EMAIL_PASSWORD_BAD_USER:
        case TC_GOOD_EMAIL_BAD_PASSWORD_NULL_USER:
        case TC_GOOD_EMAIL_USER_BAD_PASSWORD:
        case TC_GOOD_EMAIL_BAD_PASSWORD_USER:
        case TC_GOOD_PASSWORD_USER_BAD_EMAIL:
        {
            g_assert(error);
            g_assert(NULL == serverUri);

            g_message("Domain [%s]", g_quark_to_string(error->domain));
            g_message("Code   [%d]", error->code);
            g_message("Msg    [%s]", error->message);
        }
        break;

        case TC_GOOD_EMAIL_PASSWORD_NULL_USER:
        case TC_GOOD_EMAIL_PASSWORD_USER:
        {
            g_assert(NULL == error);
            g_debug("serverUri [%s]", (serverUri?serverUri:"NULL"));
        }
        break;
            
        default:
            g_error("Unexpected test case!");
            break;
    }

    if (error)
    {
        g_error_free(error);
    }

    ++g_test_index;
    if (g_test_index < MAX_TESTS)
    {
        g_idle_add((GSourceFunc)try_autodiscover, NULL);
    }
    else
    {
        g_main_loop_quit (loop);
    }
    g_message("autodiscover_cb--");
}

int main()
{
    g_type_init();

    loop = g_main_loop_new(NULL, FALSE); // Create a glib main loop using the default context.

    g_idle_add((GSourceFunc)try_autodiscover, NULL);

    g_message("Starting main loop - [%d] Tests to run", MAX_TESTS);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    g_message("Exiting main loop");

    return 0;
}

