#include <stdio.h>
#include <check.h>
#include "check_jstruct.h"
#include "check_export.h"
#include <jstruct/jstruct.h>
#include <jstruct/jstruct_private.h>
#include <jstruct/export.h>
#include "data/basic.h"

START_TEST(export_basic_data) {
    struct my_json_data data = make_data();
    struct json_object *obj = jstruct_export(&data, my_json_data);
    fprintf(stdout, "JSON OUTPUT (basic): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    test_data(data, obj);
    json_object_put(obj);
} END_TEST

START_TEST(export_struct_data) {
    struct my_json_container c = make_container();
    struct json_object *obj = jstruct_export(&c, my_json_container);
    test_container(c, obj);
    // TODO: autofree
    free(c.alloc_array_data);
    json_object_put(obj);

} END_TEST

START_TEST(export_null) {
    struct my_json_data data = make_data();
    data.ratio__null__ = true;
    struct json_object *obj = jstruct_export(&data, my_json_data);
    struct json_object *prop;
    ck_assert(json_object_object_get_ex(obj, "ratio", &prop));
    ck_assert_int_eq(json_object_get_type(prop), json_type_null);
    ck_assert(prop == NULL);

    json_object_put(obj);
} END_TEST

TCase *export_test_case(void) {
    TCase *tc = tcase_create("export");

    tcase_add_test(tc, export_basic_data);
    tcase_add_test(tc, export_struct_data);
    tcase_add_test(tc, export_null);

    return tc;
}
