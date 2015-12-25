#ifndef JSTRUCT_PRIVATE_H
#define JSTRUCT_PRIVATE_H

#include <assert.h>
#include <json-c/json_object.h>
#include <json-c/arraylist.h>
#include "jstruct.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof array / sizeof array[0])
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX_7
#define MAX_7(a1, a2, a3, a4, a5, a6, a7) MAX(MAX(MAX(MAX(MAX(MAX(a1, a2), a3), a4), a5), a6), a7)
#endif

#ifndef MIN_7
#define MIN_7(a1, a2, a3, a4, a5, a6, a7) MIN(MIN(MIN(MIN(MIN(MIN(a1, a2), a3), a4), a5), a6), a7)
#endif

// i'm probably being paranoid
#define json_type_first MIN_7(json_type_null, json_type_boolean, \
    json_type_double, json_type_int, json_type_object, json_type_array, json_type_string)
#define json_type_last MAX_7(json_type_null, json_type_boolean, \
    json_type_double, json_type_int, json_type_object, json_type_array, json_type_string)

#define json_type_index(t) (t - json_type_first)
#define json_type_count = json_type_last - json_type_first + 1

#define jstruct_prop_get(type, data, property) *(type *)(jstruct_prop_addr(data, property))

#define JSTRUCT_PROP_PTR_GET_NO_DEREF -1
// Get the pointer of the property at the specified array index.
// if index == JSTRUCT_PROP_PTR_GET_NO_DEREF and the property is a dereferenced pointer, return the actual array pointer
void *jstruct_prop_ptr(const void *data, const struct jstruct_object_property *property, int index);
int jstruct_length_get(const void *data, const struct jstruct_object_property *property);
void jstruct_length_set(const void *data, const struct jstruct_object_property *property, int length);
bool jstruct_null_get(const void *data, const struct jstruct_object_property *property);
void jstruct_null_set(const void *data, const struct jstruct_object_property *property, bool value);

typedef enum jstruct_allocated_type {
    jstruct_allocated_type_arraylist,
    jstruct_allocated_type_raw
} jstruct_allocated_type;

struct jstruct_allocated {
    enum jstruct_allocated_type type;
    void *data;
};

void jstruct_allocated_free(void *data);
bool jstruct_allocated_add(struct array_list *arr, enum jstruct_allocated_type type, void *data);

#endif
