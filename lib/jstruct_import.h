#ifndef JSTRUCT_IMPORT_H
#define JSTRUCT_IMPORT_H

#include <stdbool.h>
#include <json-c/json_object.h>
#include "jstruct.h"

// call _jstruct_export with the correct property list for a specefied pointer and struct type
#define jstruct_import(obj, data, type, errs) \
    _jstruct_import(obj, data, type ## __jstruct_properties__)

// returns jstruct_error with the most recent error (or .error=jstruct_error_none)
// Errors will be populated with a json version of the entire error list.
// It may be a json array. If errors is NULL it will be ignored.
struct jstruct_error_info _jstruct_import(struct json_object *obj, const void *data,
        const struct jstruct_object_property *properties, struct json_object *errors);

#endif
