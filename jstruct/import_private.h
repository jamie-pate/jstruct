#ifndef JSTRUCT_IMPORT_PRIVATE_H
#define JSTRUCT_IMPORT_PRIVATE_H

#include <errno.h>
#include <json-c/json_object.h>
#include "error.h"
#include "jstruct_private.h"

typedef struct jstruct_result(*jstruct_import_importer)
    (struct json_object *, const void *, const void *, const struct jstruct_object_property *);

// stupid macro templating tricks!

#define json_importer_name(name) jstruct_json_object_get_ ## name

#define json_importer_decl(name) static inline struct jstruct_result json_importer_name(name) \
    (struct json_object *prop, const void *data, const void *ptr, \
        const struct jstruct_object_property *property)

#define json_primitive_importer(primitive_type, name) json_importer_decl(name) { \
    if (property->type.extra != jstruct_extra_type_none) { \
        return extra_importers[property->type.extra](prop, data, ptr, property); \
    } \
    primitive_type *value = (primitive_type *)ptr; \
    *value = (primitive_type)json_object_get_ ## name(prop); \
    return JSTRUCT_OK; \
}

#define json_extra_importer(primitive_type, name) json_importer_decl(primitive_type) { \
    primitive_type *value = (primitive_type *)ptr; \
    *value = (primitive_type)json_object_get_ ## name(prop); \
    return JSTRUCT_OK; \
}

struct jt_importer {
    json_type type;
    jstruct_import_importer importer;
};

struct jt_extra_importer {
    jstruct_extra_type type;
    jstruct_import_importer importer;
};

#endif
