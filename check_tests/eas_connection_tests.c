#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>

#include "../eas-daemon/libeas/eas-connection.h"
#include "../eas-daemon/libeas/eas-connection-errors.h"
#include "eas_test_user.h"

static GMainLoop *loop = NULL;

START_TEST (test_fetch_server_protocols)
{
	EasConnection *cnc;
	GError *error = NULL;
	gboolean ret;
	
	//loop = g_main_loop_new (NULL, FALSE);	

	g_debug("find connection for %s", TEST_ACCOUNT_ID);
	
	cnc = eas_connection_find ((gchar*)TEST_ACCOUNT_ID);

	fail_if(!cnc, "Failed: no connection found for %s",TEST_ACCOUNT_ID);
	
    mark_point ();
    g_debug ("test_fetch_server_protocols");
    ret = eas_connection_fetch_server_protocols (cnc, &error);

	fail_if(!ret, "eas_connection_fetch_server_protocols returned %s", error->message);
	//g_main_loop_run(loop);
	
    mark_point ();
}
END_TEST


Suite* eas_connection_suite (void)
{
    Suite* s = suite_create ("connection");

    g_type_init();

    /* tc_autodiscover test case */
    TCase *tc_connection = tcase_create ("core");

    suite_add_tcase (s, tc_connection);

    tcase_add_test (tc_connection, test_fetch_server_protocols);

    return s;
}
