#ifndef LIBJSTRUCT_JSTRUCT_H
#define LIBJSTRUCT_JSTRUCT_H

#include <json-c/json_object.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum jstruct_extra_type {
    jstruct_extra_type_none = 0,
    jstruct_extra_type_uint,
    jstruct_extra_type_int64,
    jstruct_extra_type_uint64,
    jstruct_extra_type_float,
    jstruct_extra_type_end
} jstruct_extra_type;

struct jstruct_object_property {
    char *name;
    // Useful for exporting eg: a self documenting REST api
    char *comment;
    struct type_t {
        json_type json;
        // if the c type doesn't exactly match the json type, it's covered here.
        jstruct_extra_type extra;
        json_type member;
        // this is really a jstruct_type, but that enum is generated separately for each consumer
        int jstruct;
    } type;
    unsigned int offset;
    bool nullable;
    // non-zero for a static array size.
    unsigned int length;
};

// public headers
#include "export.h"

#endif
