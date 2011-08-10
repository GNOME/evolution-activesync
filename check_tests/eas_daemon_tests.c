#include <check.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <string.h>

#include "../eas-daemon/src/activesyncd-common-defs.h"



GMainLoop* mainloop = NULL;


Suite* eas_daemon_suite (void)
{
    Suite* s = suite_create ("eas-daemon");

    /* daemon test case */
    TCase *tc_deamon = tcase_create ("core");
//    suite_add_tcase (s, tc_deamon);
//    tcase_add_test (tc_deamon, test_daemon_connection);


    return s;
}




