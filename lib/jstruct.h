#ifndef LIBJSTRUCT_JSTRUCT_H
#define LIBJSTRUCT_JSTRUCT_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <json-c/json_object.h>

typedef enum jstruct_extra_type {
    jstruct_extra_type_none = 0,
    jstruct_extra_type_uint32_t,
    jstruct_extra_type_int64_t,
    jstruct_extra_type_uint64_t,
    jstruct_extra_type_float,
    jstruct_extra_type_end
} jstruct_extra_type;

struct jstruct_object_property {
    char *name;
    // Useful for exporting eg: a self documenting REST api
    char *comment;
    // json-schema
    char *schema;
    struct type_t {
        json_type json;
        // if the c type doesn't exactly match the json type, it's covered here.
        jstruct_extra_type extra;
        json_type member;
        // this is really an annotated struct. Pointer to the property list for that struct
        struct jstruct_object_property *jstruct;
    } type;
    unsigned int offset;
    bool nullable;

    // Array stuff
    // non-zero for a static array size.
    unsigned int length;
    // offset of __length__ member for this property in generated struct
    unsigned int length_offset;
    // distance between array elements (in bytes)
    unsigned int stride;
    // true to dereference members before constructing json (char ** -> char *)
    bool dereference;
};

// public headers
#include "export.h"

#endif
