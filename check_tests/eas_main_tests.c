#include <stdlib.h>
#include <check.h>

Suite* eas_daemon_suite (void);
Suite* eas_autodiscover_suite (void);
Suite* eas_libeasmail_suite (void);
Suite* eas_libeascal_suite (void);
Suite* eas_libeassync_suite (void);
Suite* eas_libeascon_suite (void);
Suite* eas_folderhierarchy_suite (void);
Suite* eas_con_info_translator_suite (void);
Suite* eas_cal_info_translator_suite (void);
Suite* eas_gobjectunittest_suite (void);
Suite* eas_email_info_translator_suite (void);
Suite* eas_connection_suite (void);

int main (void)
{
    int number_failed;

    SRunner* sr = srunner_create (eas_daemon_suite());

    /** Only to be used manually, updating the sync key to see manual 
     *  changes on the server reflected to the client
     */
//    srunner_add_suite (sr, eas_folderhierarchy_suite());

//    srunner_add_suite (sr, eas_autodiscover_suite());
    srunner_add_suite (sr, eas_libeasmail_suite());
    srunner_add_suite (sr, eas_libeascal_suite());
//    srunner_add_suite (sr, eas_libeassync_suite());
    srunner_add_suite (sr, eas_libeascon_suite()); 
	srunner_add_suite (sr, eas_gobjectunittest_suite()); 
   	srunner_add_suite (sr, eas_con_info_translator_suite()); 
    	srunner_add_suite (sr, eas_cal_info_translator_suite()); 
    srunner_add_suite (sr, eas_email_info_translator_suite()); 
	//srunner_add_suite (sr, eas_connection_suite());
    srunner_set_xml (sr, "eas-daemon_test.xml");
    srunner_set_log (sr, "eas-daemon_test.log");
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
