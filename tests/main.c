
#include <stdlib.h>
#include <check.h>

#include "stream.c"

Suite *
ramsey_suite (void)
{
  Suite *s = suite_create ("RamseyScript");

  TCase *tc_stream = tcase_create ("Stream");
  tcase_add_test (tc_stream, stdio_test);
  suite_add_tcase (s, tc_stream);

  return s;
}

int main (void)
{
  int number_failed;
  Suite *s = ramsey_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

