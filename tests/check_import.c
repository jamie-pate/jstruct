#include <stdio.h>
#include <check.h>
#include <json-c/json_object.h>
#include <json-c/arraylist.h>
#include "check_jstruct.h"
#include "check_import.h"
#include <jstruct/jstruct.h>
#include <jstruct/jstruct_private.h>
#include <jstruct/import.h>
#include "data/basic.h"

START_TEST(import_basic_data) {
    struct my_json_data data = make_data();
    struct my_json_data imported = {0};
    struct json_object *obj = make_json_obj();
    fprintf(stdout, "JSON INPUT (basic): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    struct jstruct_result status = jstruct_import(obj, &imported, my_json_data, NULL);
    fprintf(stdout, "IMPORT STATUS %d %d\n", status.error, jstruct_error_none);
    fflush(stdout);
    ck_assert(status.error == jstruct_error_none);
    fprintf(stdout, "my_data %lu\n", data.id);
    fflush(stdout);
    test_data(imported, obj);
    json_object_put(obj);
    ck_assert(status.allocated != NULL);
    array_list_free(status.allocated);
} END_TEST

/*
START_TEST(import_struct_data_with_errors) {
    // TODO: do this
} END_TEST
*/

START_TEST(import_basic_struct_data) {
    struct my_json_basic_container c = make_basic_container();
    struct json_object *obj = jstruct_export(&c, my_json_basic_container);
    struct my_json_basic_container c_imported = {0};
    fprintf(stdout, "JSON_IMPORT (basic struct): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    struct jstruct_result status = jstruct_import(obj, &c_imported, my_json_basic_container, NULL);
    struct json_object *prop;
    ck_assert(json_object_object_get_ex(obj, "main_data", &prop));
    fprintf(stdout, "JSON OUTPUT (imported basic struct): %s\n", json_object_to_json_string(prop));
    fflush(stdout);
    test_data(c_imported.main_data, prop);
    json_object_put(obj);;
    array_list_free(status.allocated);
} END_TEST

START_TEST(import_struct_data) {
    struct my_json_container c = make_container();
    struct json_object *obj = jstruct_export(&c, my_json_container);
    struct my_json_container c_imported = {0};
    fprintf(stdout, "JSON INPUT (struct): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    struct jstruct_result status = jstruct_import(obj, &c_imported, my_json_container, NULL);
    test_container(c_imported, obj);
    // TODO: autofree
    free(c.alloc_array_data);
    json_object_put(obj);
    array_list_free(status.allocated);
} END_TEST

TCase *import_test_case(void) {
    TCase *tc = tcase_create("import");

    tcase_add_test(tc, import_basic_data);
    //tcase_add_test(tc, import_struct_data_with_errors);
    tcase_add_test(tc, import_basic_struct_data);
    tcase_add_test(tc, import_struct_data);

    return tc;
}
