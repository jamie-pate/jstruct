#include <stdio.h>
#include <check.h>
#include <json-c/json_object.h>
#include "check_export.h"
#include <jstruct/jstruct.h>
#include <jstruct/jstruct_private.h>
#include <jstruct/export.h>
#include "data/basic.h"

#define BIG_INT64 0xFFFFFFFFF

struct my_json_data get_data() {
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

START_TEST(export_basic_data) {
    struct my_json_data data = get_data();
    struct json_object *obj = jstruct_export(&data, my_json_data);
    fprintf(stdout, "JSON OUTPUT: %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    test_data(data, obj);

} END_TEST

START_TEST(export_struct_data) {
    struct my_json_data data = get_data();
    struct my_json_container c = {0};
    int i;
    c.main_data = data;
    for (i = 0; i < ARRAYSIZE(c.array_data); ++i) {
        ++data.id;
        c.array_data[i] = data;
    }
    /* malloc macro (automatically sets container.array_data__length__ = 2) */
    jstruct_array_malloc(c, alloc_array_data, struct my_json_data, 2);
    for (i = 0; i < c.alloc_array_data__length__; ++i) {
        ++data.id;
        c.alloc_array_data[i] = data;
    }

    struct json_object *obj = jstruct_export(&c, my_json_container);
    struct json_object *prop;
    struct json_object *item;

    fprintf(stdout, "JSON OUTPUT: %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    ck_assert(json_object_object_get_ex(obj, "main_data", &prop) == true);
    test_data(c.main_data, prop);

    ck_assert(json_object_object_get_ex(obj, "array_data", &prop) == true);
    ck_assert(json_object_array_length(prop) == ARRAYSIZE(c.array_data));
    for (i = 0; i < ARRAYSIZE(c.array_data); ++i) {
        item = json_object_array_get_idx(prop, i);
        ck_assert_ptr_ne(item, NULL);
        test_data(c.array_data[i], item);
    }
    ck_assert(json_object_object_get_ex(obj, "alloc_array_data", &prop) == true);
    ck_assert(json_object_array_length(prop) == c.alloc_array_data__length__);
    for (i = 0; i < c.alloc_array_data__length__; ++i) {
        item = json_object_array_get_idx(prop, i);
        test_data(c.alloc_array_data[i], item);
    }
    // TODO: autofree
    free(c.alloc_array_data);

} END_TEST

TCase *export_test_case(void) {
    TCase *tc = tcase_create("export");

    tcase_add_test(tc, export_basic_data);
    tcase_add_test(tc, export_struct_data);

    return tc;
};
