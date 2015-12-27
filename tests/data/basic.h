#ifndef BASIC_H
#define BASIC_H
// Generated automatically by libjstruct. Do Not Modify.

#include <jstruct/jstruct.h>
#include <json-c/json_object.h>
#include <stdint.h>
#include <stdbool.h>
#include <jstruct/error.h>
struct my_json_data
{
  uint64_t id;
  int _id;
  enum jstruct_error err;
  bool active;
  double ratio;
  char *name;
  char **tags;
  int tags__length__;
  bool ratio__null__;
};
extern struct jstruct_object_property my_json_data__jstruct_properties__[];
struct my_json_basic_container
{
  struct my_json_data main_data;
};
extern struct jstruct_object_property my_json_basic_container__jstruct_properties__[];
struct my_json_container
{
  struct my_json_data main_data;
  struct my_json_data array_data[5];
  struct my_json_data *alloc_array_data;
  int alloc_array_data__length__;
};
extern struct jstruct_object_property my_json_container__jstruct_properties__[];

#endif
