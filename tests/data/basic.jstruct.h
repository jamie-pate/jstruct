#ifndef BASIC_H
#define BASIC_H

#include <stdint.h>
#include <stdbool.h>
#include <jstruct/error.h>

//@json
struct my_json_data {
    /*
    @schema {
        "title": "ID",
        "description": "unique object id",
        "type": "int"
    }
    */
    uint64_t id;

    /* don't include in json */
    //@private
    int _id;
    // treated as uintX where X is the sizeof the enum
    enum jstruct_error err;

    bool active;

    /* add the ability to null this field even though it's not a pointer */
    //@nullable
    double ratio;
    //@name other_name
    char *name;

    /* TODO: @dereference annotation in case this isn't an array? */
    char **tags;
};

//@json
struct my_json_container {
    struct my_json_data main_data;

    /* static arrays are automatic */
    struct my_json_data array_data[5];
    /* TODO: @array shouldn't be automatic here */
    struct my_json_data *alloc_array_data;
};

#endif // BASIC_H
