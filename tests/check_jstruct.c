#include "check_jstruct.h"
#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include JSON_OBJECT_H
#include "../jstruct/jstruct.h"
#include "data/basic.h"

// for ARRAYSIZE, assert.h
#include <jstruct/jstruct_private.h>
#include "check_export.h"
#include "check_import.h"

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
	static int id = 0;
    static char *data_tags[] = {"main", "data", "sample"};
    struct my_json_data data = {
        .id=BIG_INT64,
        ._id=++id,
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

struct my_json_container make_container() {
	struct my_json_data data = make_data();
	struct my_json_container c = {0};
	int i;
	int id = data.id;
	c.main_data = data;
	for (i = 0; i < ARRAYSIZE(c.array_data); ++i) {
		c.array_data[i] = data;
		c.array_data[i].id = ++id;
	}
	/* malloc macro (automatically sets container.array_data__length__ = 2) */
	jstruct_array_malloc(c, alloc_array_data, struct my_json_data, 2);
	for (i = 0; i < c.alloc_array_data__length__; ++i) {
		++data.id;
		c.alloc_array_data[i] = data;
	}
	return c;
}

struct json_object *make_json_container_obj() {
	struct my_json_container c = make_container();
	struct json_object *result = jstruct_export(&c, my_json_container);
	free(c.alloc_array_data);
	return result;
}

struct my_json_basic_container make_basic_container() {
	struct my_json_basic_container c = {0};
	c.main_data = make_data();
	return c;
}

struct json_object *make_json_basic_container_obj() {
	struct my_json_basic_container c = make_basic_container();
	struct json_object *result = jstruct_export(&c, my_json_basic_container);
	return result;
}

// test that the data matches
void test_data(struct my_json_data data, struct json_object *obj) {
    struct json_object *prop;
    ck_assert_ptr_ne(obj, NULL);
	fprintf(stdout, "TEST_ID: %llu:%d %s\n", data.id, data._id, json_object_to_json_string(obj));
	fflush(stdout);

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

void test_container(struct my_json_container c, struct json_object *obj) {
	struct json_object *prop;
    struct json_object *item;
	int i;

    fflush(stdout);
    ck_assert(json_object_object_get_ex(obj, "main_data", &prop) == true);
	fprintf(stdout, "\tJSON TEST (struct): %s\n", json_object_to_json_string(prop));
	fflush(stdout);
    test_data(c.main_data, prop);

    ck_assert(json_object_object_get_ex(obj, "array_data", &prop) == true);
    ck_assert(json_object_array_length(prop) == ARRAYSIZE(c.array_data));
	int len = ARRAYSIZE(c.array_data);
    for (i = 0; i < len; ++i) {

	    fflush(stdout);
        item = json_object_array_get_idx(prop, i);
		fprintf(stdout, "\tJSON test (struct) %d: %s\n", i, json_object_to_json_string(item));
		fflush(stdout);
        ck_assert_ptr_ne(item, NULL);
        test_data(c.array_data[i], item);
    }
    ck_assert(json_object_object_get_ex(obj, "alloc_array_data", &prop) == true);
    ck_assert(json_object_array_length(prop) == c.alloc_array_data__length__);
    for (i = 0; i < c.alloc_array_data__length__; ++i) {
        item = json_object_array_get_idx(prop, i);
		fprintf(stdout, "\tJSON INPUT (struct) %d: %s\n", i, json_object_to_json_string(item));
		fflush(stdout);
		ck_assert_ptr_ne(item, NULL);
        test_data(c.alloc_array_data[i], item);
    }
}
