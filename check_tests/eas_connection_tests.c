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
	
	cnc = eas_connection_find ((gchar*)TEST_ACCOUNT_ID);

	fail_if(!cnc, "Failed: no connection found for %s",TEST_ACCOUNT_ID);
	
    mark_point ();

    ret = eas_connection_fetch_server_protocols (cnc, &error);

	mark_point();
	if(!ret)
	{
		fail_if(TRUE, "eas_connection_fetch_server_protocols returned error %s", error->message);
	}

	mark_point();
	ret = eas_connection_fetch_server_protocols (cnc, &error);	
	mark_point();
	
	// TODO verify that the server protocol list is in GSettings 
	g_object_unref(cnc);
    mark_point ();
}
END_TEST


Suite* eas_connection_suite (void)
{
    Suite* s = suite_create ("connection");

#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif

    /* tc_autodiscover test case */
    TCase *tc_connection = tcase_create ("core");

    suite_add_tcase (s, tc_connection);

    tcase_add_test (tc_connection, test_fetch_server_protocols);

    return s;
}
