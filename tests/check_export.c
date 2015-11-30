#include <check.h>
#include <json-c/json_object.h>
#include "check_export.h"
#include "../lib/jstruct.h"
#include "../lib/export.h"
#include "data/basic.h"

START_TEST(export_basic_data) {
    char *data_tags[] = {"main", "data", "sample"};
    struct my_json_data data = {
        .id=1,
        ._id=2,
        .ratio=3.5,
        .name="main_data",
        .tags=data_tags,
    };
    struct json_object *obj = jstruct_export(&data, my_json_data);
    struct json_object *prop;
    ck_assert(json_object_object_get_ex(obj, "id", &prop) == true);
    ck_assert_int_eq(json_object_get_int64(prop), 1);
    ck_assert(json_object_object_get_ex(obj, "_id", &prop) == false);

    ck_assert(json_object_object_get_ex(obj, "ratio", &prop) == true);
    ck_assert(json_object_get_double(prop) == 3.5);

    ck_assert(json_object_object_get_ex(obj, "name", &prop) == true);
    ck_assert_str_eq(json_object_get_string(prop), "main_data");

    ck_assert(json_object_object_get_ex(obj, "tags", &prop) == true);
    ck_assert_int_eq(json_object_array_length(prop), 3);
    ck_assert_str_eq(json_object_array_get_idx(prop, 0), "main");
    ck_assert_str_eq(json_object_array_get_idx(prop, 1), "data");
    ck_assert_str_eq(json_object_array_get_idx(prop, 2), "sample");

} END_TEST

TCase *export_test_case(void) {
    TCase *tc = tcase_create("export");

    tcase_add_test(tc, export_basic_data);

    return tc;
};
