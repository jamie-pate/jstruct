#include <stdlib.h>
#include <check.h>
#include "../lib/jstruct.h"
#include "check_export.h"

Suite * jstruct_suite(void) {
	Suite *s = suite_create("jstruct");

	suite_add_tcase(s, export_test_case());

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = jstruct_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
