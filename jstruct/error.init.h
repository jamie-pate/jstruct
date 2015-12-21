// Generated automatically by libjstruct. Do Not Modify.
#include "error.h"
char *jstruct_error_str[] = {"No Error", "Error occurred in json-c. Check errno for details", "Property not nullable but value missing or null", "Child property error: see inner_error", "Invalid property type. (This shouldn't happen)"};
struct jstruct_object_property jstruct_error_info__jstruct_properties__[] = {{.name = "error", .type = {.json = json_type_int, .extra = jstruct_enum_extra_type(enum jstruct_error)}, .offset = offsetof(struct jstruct_error_info, error)}, {.name = "message", .type = {.json = json_type_string}, .offset = offsetof(struct jstruct_error_info, message)}, {.name = "property", .type = {.json = json_type_string}, .offset = offsetof(struct jstruct_error_info, property)}, {.name = "last_errno", .type = {.json = json_type_int}, .offset = offsetof(struct jstruct_error_info, last_errno)}, {0}};
struct jstruct_error_info jstruct_errok = {.error = jstruct_error_none, .message = "No Error"};
