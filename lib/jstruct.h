#ifndef LIBJSTRUCT_JSTRUCT_H
#define LIBJSTRUCT_JSTRUCT_H

#include <json-c/json_object.h>
#include <stdbool.h>
#include <stdlib.h>

struct jstruct_object_property {
    char *name;
    // Useful for exporting eg: a self documenting REST api
    char *comment;
    struct type_t {
        json_type json;
        json_type member;
        // this is really a jstruct_type, but that enum is generated separately for each consumer
        int jstruct;
    } type;
    unsigned int offset;
    bool nullable;
    // non-zero for a static array size.
    unsigned int length;
};

#include "export.h"

#endif
