#ifndef JSTRUCT_IMPORT_PRIVATE_H
#define JSTRUCT_IMPORT_PRIVATE_H

#include <errno.h>
#include JSON_OBJECT_H
#include "error.h"
#include "jstruct_private.h"

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include <stdio.h>
#endif

typedef struct jstruct_result(*jstruct_import_importer)
    (struct json_object *, const void *, const void *, const struct jstruct_object_property *);

// stupid macro templating tricks!

#define json_importer_name(name) jstruct_json_object_get_ ## name

#define json_importer_decl(name) static inline struct jstruct_result json_importer_name(name) \
    (struct json_object *prop, const void *data, const void *ptr, \
        const struct jstruct_object_property *property)

#ifdef PRINT_DEBUG

#define debug_importer(primitive_type, json_type) \
    fprintf(stdout, "IMPORT " #json_type " -> " #primitive_type " %x %s %p\n" , *value, property->name, value); \
    fflush(stdout);
#else
#define debug_importer(primitive_type, json_type)
#endif

#define json_primitive_importer(primitive_type, name) \
json_importer_decl(name) { \
    if (property->type.extra != jstruct_extra_type_none) { \
        return extra_importers[property->type.extra](prop, data, ptr, property); \
    } \
    primitive_type *value = (primitive_type *)ptr; \
    *value = (primitive_type)json_object_get_ ## name(prop); \
    debug_importer(primitive_type, name) \
    return JSTRUCT_OK; \
}

#define json_extra_importer_camel(camel_type, primitive_type, name) \
json_importer_decl(camel_type) { \
    primitive_type *value = (primitive_type *)ptr; \
    *value = (primitive_type)json_object_get_ ## name(prop); \
    debug_importer(primitive_type, name) \
    return JSTRUCT_OK; \
}


#define json_extra_importer(primitive_type, name) \
    json_extra_importer_camel(primitive_type, primitive_type, name)

struct jt_importer {
    json_type type;
    jstruct_import_importer importer;
};

struct jt_extra_importer {
    jstruct_extra_type type;
    jstruct_import_importer importer;
};

#endif
