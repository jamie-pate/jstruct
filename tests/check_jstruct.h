#ifndef CHECK_JSTRUCT_H
#define CHECK_JSTRUCT_H

#include <json-c/json_object.h>
#define BIG_INT64 0xFFFFFFFFF

// Helpers functions
struct my_json_data make_data();
struct my_json_container make_container();
struct my_json_basic_container make_basic_container();
struct json_object *make_json_obj();
struct json_object *make_json_container_obj();
struct json_object *make_json_basic_container_obj();
void test_data(struct my_json_data data, struct json_object *obj);
void test_container(struct my_json_container, struct json_object *obj);

#endif
