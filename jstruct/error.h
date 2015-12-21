#ifndef JSTRUCT_ERROR_H
#define JSTRUCT_ERROR_H
// Generated automatically by libjstruct. Do Not Modify.

#include "jstruct.h"
#include <json-c/json_object.h>
#define JSTRUCT_ERROR_STR_NONE "No Error"
typedef enum jstruct_error {jstruct_error_none = 0, jstruct_error_json_c_op_failed, jstruct_error_not_nullable, jstruct_error_inner_error, jstruct_error_invalid_type} jstruct_error;
extern char *jstruct_error_str[];
struct jstruct_error_info
{
  enum jstruct_error error;
  char *message;
  char *property;
  int last_errno;
  struct json_object *inner_errors;
};
extern struct jstruct_object_property jstruct_error_info__jstruct_properties__[];
extern struct jstruct_error_info jstruct_errok;
struct jstruct_error_info jstruct_error_array_add_err(struct json_object *errors, struct jstruct_error_info *err);
struct jstruct_error_info jstruct_error_array_add(struct json_object *errors, enum jstruct_error error, char *property);
struct jstruct_error_info jstruct_error_new(enum jstruct_error error, char *property);

#endif
