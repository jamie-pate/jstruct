#include "check_import.h"
#include <stdio.h>
#include JSON_OBJECT_H
#include ARRAYLIST_H
#include "check_jstruct.h"
#include "../jstruct/jstruct.h"
#include "../jstruct/jstruct_private.h"
#include "../jstruct/import.h"
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
    fprintf(stdout, "my_data %llu\n", data.id);
    fflush(stdout);
    test_data(imported, obj);
    json_object_put(obj);
    ck_assert(status.allocated != NULL);
    array_list_free(status.allocated);
} END_TEST

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
    ck_assert(status.error == jstruct_error_none);
    test_data(c_imported.main_data, prop);
    json_object_put(obj);
    array_list_free(status.allocated);
} END_TEST

static void free_container_all(struct my_json_container *c, struct json_object *obj, struct jstruct_result *status) {
    // TODO: autofree
    free(c->alloc_array_data);
    json_object_put(obj);
    array_list_free(status->allocated);
}

START_TEST(import_struct_data) {
    struct my_json_container c = make_container();
    struct json_object *obj = jstruct_export(&c, my_json_container);
    struct my_json_container c_imported = {0};
    struct json_object *errors = json_object_new_array();
    fprintf(stdout, "JSON INPUT (struct): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    struct jstruct_result status = jstruct_import(obj, &c_imported, my_json_container, errors);
    if (status.error != jstruct_error_none) {
        fprintf(stdout, "JSON INPUT (struct errors): %s\n", json_object_to_json_string(errors));
        fflush(stdout);
    }
    json_object_put(errors);
    ck_assert(status.error == jstruct_error_none);
    test_container(c_imported, obj);
    free_container_all(&c, obj, &status);
} END_TEST

#define ERROR_TEST_SETUP(test, setup) \
    struct my_json_container c = make_container(); \
    struct json_object *obj = jstruct_export(&c, my_json_container); \
    struct my_json_container c_imported = {0}; \
    struct json_object *obj_data; \
    struct json_object *errors = json_object_new_array(); \
    ck_assert(json_object_object_get_ex(obj, "main_data", &obj_data)); \
    setup \
    fprintf(stdout, "JSON INPUT (struct errors %s): %s\n", test, json_object_to_json_string(obj)); \
    fflush(stdout); \
    errno = 0; \
    struct jstruct_result status = jstruct_import(obj, &c_imported, my_json_container, errors);

#define ERROR_TEST_FREE \
    json_object_put(errors); \
    free_container_all(&c, obj, &status);

START_TEST(import_struct_data_inner_error) {
    ERROR_TEST_SETUP("wrong type",
        json_object_object_add(obj_data, "id", json_object_new_string("0"));
    )
    char expected_errors[] =
        "[ { \"error\": 3, \"message\": \"Child property error: see inner_error\", \"property\": \"main_data\", "
        "\"detail\": -1, \"last_errno\": 0, \"inner_errors\": [ "
        "{ \"error\": 4, \"message\": \"JSON Property is not of the correct type\", \"property\": \"id\", "
        "\"detail\": 6, \"last_errno\": 0 } ] } ]";
    const char *found_errors = json_object_to_json_string(errors);
    ck_assert(status.error == jstruct_error_inner_error);
    ck_assert_str_eq(found_errors, expected_errors);

    ERROR_TEST_FREE
} END_TEST

START_TEST(import_struct_foreign) {
    struct my_json_data data = make_data();
    struct my_json_foreign_struct fimported = {0};
    struct json_object *obj = make_json_obj();
    fprintf(stdout, "JSON INPUT (basic): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    struct jstruct_result status = jstruct_import(obj, &fimported, my_json_foreign_struct, NULL);
    fprintf(stdout, "IMPORT STATUS %d %d\n", status.error, jstruct_error_none);
    fflush(stdout);
    ck_assert(status.error == jstruct_error_none);
    fprintf(stdout, "my_data %llu\n", data.id);
    fflush(stdout);
    // fimported and imported are only coincidentally binary compatible.
    struct my_json_data imported = {
        .id=fimported.id,
        ._id=fimported._id,
        .err=fimported.err,
        .active=fimported.active,
        .ratio_double=fimported.ratio_double,
        .name=fimported.name,
        .ull=fimported.ull,
        .tags=fimported.tags,
        .tags__length__=fimported.tags__length__
    };
    test_data(imported, obj);
    json_object_put(obj);
    ck_assert(status.allocated != NULL);
    array_list_free(status.allocated);
} END_TEST

TCase *import_test_case(void) {
    TCase *tc = tcase_create("import");

    tcase_add_test(tc, import_basic_data);
    tcase_add_test(tc, import_basic_struct_data);
    tcase_add_test(tc, import_struct_data);
    tcase_add_test(tc, import_struct_data_inner_error);
    tcase_add_test(tc, import_struct_foreign);

    return tc;
}
