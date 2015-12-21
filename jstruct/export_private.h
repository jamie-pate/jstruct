#ifndef JSTRUCT_EXPORT_PRIVATE_H
#define JSTRUCT_EXPORT_PRIVATE_H

#include <json-c/json_object.h>
#include "jstruct_private.h"


typedef struct json_object *(*jstruct_export_ctor)
    (const void *, const void *, const struct jstruct_object_property *);

// stupid macro templating tricks!

#define json_ctor_name(name) jstruct_json_object_new_ ## name

#define json_ctor_decl(name) static inline struct json_object *json_ctor_name(name) \
    (const void *data, const void * ptr, const struct jstruct_object_property *property)

#define json_primitive_ctor(primitive_type, name) json_ctor_decl(name) { \
    if (property->type.extra != jstruct_extra_type_none) { \
        return extra_constructors[property->type.extra](data, ptr, property); \
    } \
    primitive_type value = *(primitive_type *)ptr; \
    return json_object_new_ ## name(value); \
}

#define json_extra_ctor(primitive_type, name) json_ctor_decl(primitive_type) { \
    primitive_type value = *(primitive_type *)ptr; \
    return json_object_new_ ## name(value); \
}

struct jt_ctor {
    json_type type;
    jstruct_export_ctor ctor;
};

struct jt_extra_ctor {
    jstruct_extra_type type;
    jstruct_export_ctor ctor;
};

#endif
