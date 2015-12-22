#ifndef JSTRUCT_ERROR_INFO_H
#define JSTRUCT_ERROR_INFO_H
// Generated automatically by libjstruct. Do Not Modify.

#include <jstruct/jstruct.h>
#include <json-c/json_object.h>
#include <json-c/json_object.h>
#include <errno.h>
struct jstruct_error_info
{
  enum jstruct_error error;
  char *message;
  char *property;
  int last_errno;
  struct json_object *inner_errors;
};
extern struct jstruct_object_property jstruct_error_info__jstruct_properties__[];

#endif
