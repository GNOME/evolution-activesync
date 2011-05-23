#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

START_TEST (test_cal)
{
//... calendar test case 

}
END_TEST

Suite* eas_libeascal_suite (void)
{
  Suite* s = suite_create ("libeascal");

  /* tc_libeascal test case */
  TCase *tc_libeascal = tcase_create ("core");
  suite_add_tcase (s, tc_libeascal);
  tcase_add_test (tc_libeascal, test_cal);

  return s;
}
