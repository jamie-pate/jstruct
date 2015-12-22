#ifndef CHECK_JSTRUCT_H
#define CHECK_JSTRUCT_H

#include <json-c/json_object.h>
#define BIG_INT64 0xFFFFFFFFF

// Helpers functions
struct my_json_data make_data();
struct json_object *make_json_obj();
void test_data(struct my_json_data data, struct json_object *obj);

#endif
