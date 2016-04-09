#ifndef LIBJSTRUCT_JSTRUCT_H
#define LIBJSTRUCT_JSTRUCT_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include JSON_OBJECT_H

typedef enum jstruct_extra_type {
    jstruct_extra_type_none = 0,
    jstruct_extra_type_int8_t,
    jstruct_extra_type_uint8_t,
    jstruct_extra_type_int16_t,
    jstruct_extra_type_uint16_t,
    jstruct_extra_type_int32_t,
    jstruct_extra_type_uint32_t,
    jstruct_extra_type_int64_t,
    jstruct_extra_type_uint64_t,
    jstruct_extra_type_float,
    // When expanding this list also add to:
    // export.c:extra_constructor_list
    // import.c:extra_importer_list
    jstruct_extra_type_end
} jstruct_extra_type;

#define jstruct_enum_extra_type(enumtype) \
    sizeof(enumtype) == sizeof(uint8_t) ? jstruct_extra_type_uint8_t : \
    sizeof(enumtype) == sizeof(uint16_t) ? jstruct_extra_type_uint16_t : \
    sizeof(enumtype) == sizeof(uint32_t) ? jstruct_extra_type_uint32_t : \
    sizeof(enumtype) == sizeof(uint64_t) ? jstruct_extra_type_uint64_t : \
    jstruct_extra_type_none  /* this should fail with an assertion if used */

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
    ptrdiff_t offset;
    bool nullable;
    ptrdiff_t null_offset;

    // Array stuff
    // non-zero for a static array size.
    unsigned int length;
    // offset of __length__ member for this property in generated struct
    ptrdiff_t length_offset;
    // distance between array elements (in bytes)
    ptrdiff_t stride;
    // true to dereference members before constructing json (char ** -> char *)
    bool dereference;
};

// Allocate an array of type into member, and keep track of the array length
#define jstruct_array_malloc(obj, member, type, length) \
    assert(obj.member ## __length__ == 0); \
    obj.member ## __length__ = length; \
    obj.member = malloc(sizeof(type) * length)

// public headers
#include "error.h"
#include "export.h"
#include "import.h"

#endif
