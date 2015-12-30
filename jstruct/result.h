#ifndef JSTRUCT_RESULT_H
#define JSTRUCT_RESULT_H
// Generated automatically by libjstruct. Do Not Modify.

#include <jstruct/jstruct.h>
#include <json-c/json_object.h>
#include <json-c/json_object.h>
#include <errno.h>
struct jstruct_result
{
  struct array_list *allocated;
  enum jstruct_error error;
  char *message;
  char *property;
  int detail;
  int last_errno;
  struct json_object *_inner_errors;
  bool property__null__;
};
extern struct jstruct_object_property jstruct_result__jstruct_properties__[];

#endif
