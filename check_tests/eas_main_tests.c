#include <stdlib.h>
#include <check.h>

int main (void)
{
    int number_failed;

    SRunner* sr = srunner_create (eas_daemon_suite());

    srunner_add_suite(sr, eas_autodiscover_suite());
    srunner_add_suite(sr, eas_libeasmail_suite());
    srunner_add_suite(sr, eas_libeascal_suite());
    srunner_set_xml(sr, "eas-daemon_test.xml");
    srunner_set_log (sr, "eas-daemon_test.log");
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}	
