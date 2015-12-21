#ifndef JSTRUCT_ERROR_H
#define JSTRUCT_ERROR_H
// Generated automatically by libjstruct. Do Not Modify.

#include <jstruct.h>
#include <json-c/json_object.h>
#define JSTRUCT_ERROR_STR_NONE "No Error"
typedef enum jstruct_error {jstruct_error_none = 0, jstruct_error_json_c_op_failed, jstruct_error_not_nullable, jstruct_error_inner_error, jstruct_error_invalid_type} jstruct_error;
char *jstruct_error_str[] = {"No Error", "Error occurred in json-c. Check errno for details", "Property not nullable but value missing or null", "Child property error: see inner_error", "Invalid property type. (This shouldn't happen)"};
struct jstruct_error_info
{
  enum jstruct_error error;
  char *message;
  char *property;
  int last_errno;
  struct json_object *inner_errors;
};
struct jstruct_object_property jstruct_error_info__jstruct_properties__[] = {{.name = "error", .type = {.json = json_type_int, .extra = jstruct_enum_extra_type(enum jstruct_error)}, .offset = offsetof(struct jstruct_error_info, error)}, {.name = "message", .type = {.json = json_type_string}, .offset = offsetof(struct jstruct_error_info, message)}, {.name = "property", .type = {.json = json_type_string}, .offset = offsetof(struct jstruct_error_info, property)}, {.name = "last_errno", .type = {.json = json_type_int}, .offset = offsetof(struct jstruct_error_info, last_errno)}, {0}};
struct jstruct_error_info jstruct_errok = {.error = jstruct_error_none, .message = "No Error"};
struct jstruct_error_info jstruct_error_array_add_err(struct json_object *errors, struct jstruct_error_info *err);
struct jstruct_error_info jstruct_error_array_add(struct json_object *errors, enum jstruct_error error, char *property);
struct jstruct_error_info jstruct_error_new(enum jstruct_error error, char *property);

#endif
