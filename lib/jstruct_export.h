#ifndef LIBJSTRUCT_EXPORT_H
#define LIBJSTRUCT_EXPORT_H
// Functions used by generated export functions
#include <json-c/json_object.h>

// call _jstruct_export with the correct property list for a specified pointer and struct type.
#define jstruct_export(data, type) \
    _jstruct_export((void *)data, type ## __jstruct_properties__)

#define jstruct_array_malloc(obj, member, type, length) \
    assert(obj.member ## __length__ == 0); \
    obj.member ## __length__ = length; \
    obj.member = malloc(sizeof(type) * length)

// returns NULL for any failure. Check errno (set by json-c)
struct json_object *_jstruct_export(const void *data,
        const struct jstruct_object_property *properties);

#endif
