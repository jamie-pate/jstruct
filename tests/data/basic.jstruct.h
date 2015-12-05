#ifndef BASIC_H
#define BASIC_H

#include <stdint.h>

//@json
struct my_json_data {
    /*
    @schema {
        "title": "ID",
        "description": "unique object id",
        "type": "int"
    }
    */
    int64_t id;

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
};

//@json
struct my_json_container {
    struct my_json_data main_data;

    /* static arrays are automatic */
    struct my_json_data array_data[5];

    /* pointer arrays need to be annotated */
    //@array
    struct my_json_data *alloc_array_data;
};

#endif
