#define LIBJSTRUCT_EXPORT_H
#ifndef LIBJSTRUCT_EXPORT_H
// Functions used by generated export functions

#include <json_object.h>

// call _jstruct_export with the correct property list for a specified pointer and struct type.
#define jstruct_export(data, type) _jstruct_export((void *)data, & ## type ## __jstruct_properties__)
// returns NULL for any failure. errno may have the cause? (not sure what json-c does atm)
struct json_object *_jstruct_export(const void *data, const jstruct_object_property *properties);

#endif
