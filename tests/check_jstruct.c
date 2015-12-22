#include <stdlib.h>
#include <check.h>
#include <json-c/json_object.h>
#include <jstruct/jstruct.h>
#include "check_jstruct.h"
#include "check_export.h"
#include "check_import.h"
// Only include once or there will be linker errors
#include "data/basic.init.h"

Suite * jstruct_suite(void) {
	Suite *s = suite_create("jstruct");

	suite_add_tcase(s, export_test_case());
	suite_add_tcase(s, import_test_case());

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = jstruct_suite();
    sr = srunner_create(s);
	srunner_print(sr, CK_VERBOSE);
    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

// Helper functions for tests

// generate some test data
struct my_json_data make_data() {
    static char *data_tags[] = {"main", "data", "sample"};
    struct my_json_data data = {
        .id=BIG_INT64,
        ._id=2,
        .ratio=3.5,
        .active=true,
        .name="main_data",
        .tags=data_tags,
        .tags__length__=3,
    };
    return data;
}

struct json_object *make_json_obj() {
	// THE CHEAT!!!! (is to the limit)
	struct my_json_data data = make_data();
	return jstruct_export(&data, my_json_data);
}

// test that the data matches
void test_data(struct my_json_data data, struct json_object *obj) {
    struct json_object *prop;
    ck_assert_ptr_ne(obj, NULL);

    ck_assert(json_object_object_get_ex(obj, "id", &prop) == true);
    ck_assert_int_eq(json_object_get_int64(prop), data.id);
    ck_assert(json_object_object_get_ex(obj, "_id", &prop) == false);

    ck_assert(json_object_object_get_ex(obj, "ratio", &prop) == true);
    ck_assert(json_object_get_double(prop) == data.ratio);

    ck_assert(json_object_object_get_ex(obj, "active", &prop) == true);
    ck_assert(json_object_get_boolean(prop) == true);

    ck_assert(json_object_object_get_ex(obj, "other_name", &prop) == true);
    ck_assert_str_eq(json_object_get_string(prop), data.name);

    ck_assert(json_object_object_get_ex(obj, "tags", &prop) == true);
    ck_assert_int_eq(json_object_array_length(prop), data.tags__length__);
    int i;
    for (i =0; i < data.tags__length__; ++i) {
        ck_assert_str_eq(json_object_get_string(json_object_array_get_idx(prop, i)), data.tags[i]);
    }
}
