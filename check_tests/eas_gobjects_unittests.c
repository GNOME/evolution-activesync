#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "../eas-daemon/libeas/eas-request-base.h"
#include "../eas-daemon/libeas/eas-add-item-req.h"


START_TEST (test_add_item_req_obj)
{
    // initise G type library to allow use of G type objects which provide
    // a higher level of programming syntax including the psuedo object
    // oriented approach supported by G types
    g_type_init();

	EasAddItemReq* self = g_object_new (EAS_TYPE_ADD_ITEM_REQ, NULL);
	g_object_ref(self);
	g_object_unref(self);
	g_object_unref(self);	
}
END_TEST

Suite* eas_gobjectunittest_suite (void)
{
    Suite* s = suite_create ("gobjectunittest");

    /* gobject unit test case */
    TCase *tc_gobjectunit = tcase_create ("core");
    suite_add_tcase (s, tc_gobjectunit);

	tcase_add_test (tc_gobjectunit, test_add_item_req_obj);

    return s;
}

