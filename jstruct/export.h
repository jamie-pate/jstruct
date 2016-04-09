#ifndef LIBJSTRUCT_EXPORT_H
#define LIBJSTRUCT_EXPORT_H
// Functions used by generated export functions
#include JSON_OBJECT_H

// call _jstruct_export with the correct property list for a specified pointer and struct type.
#define jstruct_export(data, type) \
    _jstruct_export((void *)data, type ## __jstruct_properties__)

// returns NULL for any failure. Check errno (set by json-c)
struct json_object *_jstruct_export(const void *data,
        const struct jstruct_object_property *properties);

#endif
