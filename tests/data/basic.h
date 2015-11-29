#ifndef BASIC_H
#define BASIC_H
// Generated automatically by libjstruct. Do not modify.

#include <jstruct.h>
#include <json_object.h>

//@json
struct my_json_data {
    int id;

    /* don't include in json */
    //@private
    int _id;

    /* add the ability to null this field even though it's not a pointer */
    //@nullable
    double ratio;
    char *name;

    /*TODO: (necessary?)*/
    //@array
    char **tags;
    /* GENERATED MEMBERS */
    bool ratio__null__;
    int tags__length__;
}

struct jstruct_object_property my_json_data__jstruct_properties__[] = {
    {
        .name="id",
        .type={json_type_int},
        .offset=OFFSETOF(my_json_data, id),
    },
    {
        .name="ratio",
        .type={json_type_double},
        .offset=OFFSETOF(my_json_data, ratio),
        .nullable=true,
    },
    {
        .name="name",
        .type={json_type_string},
        .offset=OFFSETOF(my_json_data, name),
    },
    {
        .name="tags",
        .offset=OFFSETOF(my_json_data, tags),
        .type={
            .json=json_type_array,
            .member=json_type_string,
        },
    }
    { NULL }
}

//@json
struct my_json_container {
    struct my_json_data main_data;

    /* static arrays are automatic */
    struct my_json_data array_data[5];

    /* pointer arrays need to be annotated */
    //@array
    struct my_json_data *alloc_array_data;
}

struct jstruct_object_property my_json_container__jstruct_properties__[] = {
    {
        .name="main_data",
        .type={
            .json=json_type_object,
            .jstruct=jstruct_type__my_json_data__,
        },
        .offset=OFFSETOF(my_json_container, main_data),
    },
    {
        .name="array_data",
        .type={
            .json=json_type_array,
            .member=json_type_object,
            .jstruct=jstruct_type__my_json_data__,
        },
        .offset=OFFSETOF(my_json_container, array_data),
        .length=5,
    },
    {
        .name="alloc_array_data",
        .type={
            .json=json_type_array,
            .member=json_type_object
            .jstruct=jstruct_type__my_json_data__,
        }
        .offset=OFFSETOF(my_json_container, alloc_array_data),
    }
}

#endif
