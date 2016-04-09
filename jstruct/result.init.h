// Generated automatically by libjstruct. Do Not Modify.
// This file must be included directly in a single c file.

#include "result.h"
#include "jstruct.h"
#include JSON_OBJECT_H
#include <errno.h>
struct jstruct_object_property jstruct_result__jstruct_properties__[] = {{.name = "error", .type = {.json = json_type_int, .extra = jstruct_enum_extra_type(enum jstruct_error)}, .offset = offsetof(struct jstruct_result, error)}, {.name = "message", .type = {.json = json_type_string}, .offset = offsetof(struct jstruct_result, message)}, {.nullable = 1, .name = "property", .type = {.json = json_type_string}, .offset = offsetof(struct jstruct_result, property), .null_offset = offsetof(struct jstruct_result, property__null__)}, {.name = "detail", .type = {.json = json_type_int}, .offset = offsetof(struct jstruct_result, detail)}, {.name = "last_errno", .type = {.json = json_type_int}, .offset = offsetof(struct jstruct_result, last_errno)}, {0}};
