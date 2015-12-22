// Generated automatically by libjstruct. Do Not Modify.
// This file must be included once only, in a single c file.

#include <jstruct/jstruct.h>
#include <json-c/json_object.h>
#include <stdint.h>
#include <stdbool.h>
#include "basic.h"

struct jstruct_object_property my_json_data__jstruct_properties__[] = {
    {
        .schema = "{\n        \"title\": \"ID\",\n        \"description\": \"unique object id\",\n        \"type\": \"int\"\n    }\n    ",
        .name = "id",
        .type = {
            .json=json_type_int,
            .extra=jstruct_extra_type_uint64_t
        },
        .offset=offsetof(struct my_json_data, id)
    },
    {
        .name="err",
        .type={
            .json=json_type_int,
            .extra=jstruct_enum_extra_type(enum jstruct_error)
        },
        .offset=offsetof(struct my_json_data, err)
    },
    {
        .name="active",
        .type={
            .json=json_type_boolean
        },
        .offset=offsetof(struct my_json_data, active)
    },
    {
        .nullable=1,
        .name="ratio",
        .type={
            .json=json_type_double
        },
        .offset=offsetof(struct my_json_data, ratio),
        .null_offset=offsetof(struct my_json_data, ratio__null__)
    },
    {
        .name="other_name",
        .type={
            .json=json_type_string
        },
        .offset=offsetof(struct my_json_data, name)
    },
    {
        .name="tags",
        .type={
            .member=json_type_string,
            .json=json_type_array
        },
        .offset=offsetof(struct my_json_data, tags),
        .length_offset=offsetof(struct my_json_data, tags__length__),
        .dereference=1,
        .stride=sizeof(char *)
    },
    {0}
};

struct jstruct_object_property my_json_container__jstruct_properties__[] = {
    {
        .name="main_data",
        .type={
            .json=json_type_object,
            .jstruct=my_json_data__jstruct_properties__
        },
        .offset=offsetof(struct my_json_container, main_data)
    },
    {
        .name="array_data",
        .type={
            .member=json_type_object,
            .json=json_type_array,
            .jstruct=my_json_data__jstruct_properties__
        },
        .offset=offsetof(struct my_json_container, array_data),
        .length=5,
        .stride=sizeof(struct my_json_data)
    },
    {
        .name="alloc_array_data",
        .type={
            .member=json_type_object,
            .json=json_type_array,
            .jstruct=my_json_data__jstruct_properties__
        },
        .offset=offsetof(struct my_json_container, alloc_array_data),
        .length_offset=offsetof(struct my_json_container, alloc_array_data__length__),
        .dereference=1,
        .stride=sizeof(struct my_json_data)
    },
    {0}
};
