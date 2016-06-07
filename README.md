# jstruct

C Library and code generator that declaratively transforms data between JSON and C.

The jstruct preprocessor generates C code that transforms data between C structures and JSON strings.
It reads annotated comments from the structure declarations in your header,
and creates c data structures that allow it to efficiently and automatically import/export JSON data.

After calling `jstruct_import()` the caller *must* call `array_list_free(result.allocated)` once they are finished with the imported struct or suffer memory leakage.
The imported data may contain pointers to the `json_object` Those pointers shouldn't be accessed once all references have been removed (EG: with `json_object_put()`)

## Development Roadmap

 * ~~*M1*~~ Export structs containing primitive types and arrays to same. (char*, int, bool, uint64_t etc)
 * ~~*M2*~~ Mechanical parsing of annotated header files to produce augmented struct declarations and data tables used by the export process.
 * ~~*M3*~~ Export structs containing other structures and arrays (struct* and struct[])
 * ~~*M4*~~ Import structs containing primitive types and arrays
 * ~~*M5*~~ Import nested structs and arrays of structs (feature complete)
 * M6 ~~Import foreign (un-annotated) structs in a binary compatible way. ~~
    * ~~Annotate a whole struct with a single json object literal~~
    * Guarantee that `json_struct_value = foreign_struct_value` is always valid
    (generate assertions checking that all property sizes and offsets match)
 * Planned features
   * Automatically free pointers in nested structs which were allocated by jstruct_array_malloc
   * Hybrid export/import. Handle c structs with json_object members automatically

# Sample

main_jstruct.jstruct.h (generates main_jstruct.h and main_jstruct.init.h)
```C
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

    bool active;

    /* add the ability to null this field even though it's not a pointer */
    //@nullable
    double ratio;
    char *name;

    /* automatically parsed as an array atm */
    char **tags;
}

//@json
struct my_json_container {
    struct my_json_data main_data;

    /* static arrays are automatic */
    struct my_json_data array_data[5];
    /* as are struct arrays atm*/
    struct my_json_data *alloc_array_data;
}

// After M6

struct foreign_struct {
#ifndef DONT_USE_UINT64_T_FOR_SOME_REASON
    uint64_t id;
#else
    unsigned long long id;
#endif
    int _id;
    enum jstruct_error err;

    bool active;
    double ratio_double;
    char *name;
    unsigned long long ull;
    char **tags;  
};
/* @json {
    "_id": "@private",
    "ratio_double": {
        "@nullable": true,
        "@name": "ratio"
    },
    "@name": "other_name"
} */
struct my_json_foreign_struct {
    // place all members of a struct 'inline' inside this struct 
    // @inline
    struct foreign_struct fs;
};

```
main.c
```C
#include "main_jstruct.h"
// Initializers must be in a separate file annd included only in one c file
#include "main_jstruct.init.h"

#include <json-c/json_object.h>

int main() {
    char *data_tags[] = {"main", "data", "sample"};
    struct my_json_container container={
        .main_data={
            .id=1,
            ._id=2,
            .ratio_double=3.5,
            .name="main_data",
            .tags=data_tags
        },
        .array_data={
            {
                .id=3,
                .ratio__null__=true
            },{.id=5},{.id=6}
        }
    }
    /* malloc macro (automatically sets container.array_data__length__ = 2) */
    jstruct_array_malloc(container, alloc_array_data, struct my_json_data, 2)

    struct json_object *obj = jstruct_export(&container, my_json_container);
    if (obj) {
        printf("%s", json_object_to_json_string(obj));
    }


    // After M6
    struct foreign_struct foreign_data = {
        .id=1,
        ._id=2,
        .ratio_double=3.5,
        .name="foreign_data",
        .tags=data_tags
    };
    struct my_json_foreign_struct jforeign_data = foreign_data;

    struct json_object *fobj = jstruct_export(&jforeign_data, my_json_foreign_struct);
}
```

# Build and Install

## From git

 Required Packages: `autoconf`,`libtool`,`libjson-c-dev` v0.10+, `python-pycparser` v2.11+

 * install check: http://check.sourceforge.net/web/install.html (needed for autoreconf) or `git clone https://github.com/libcheck/check.git && cd check` and follow the instructions in `README`
 * git clone *repo*
 * `libtoolize`
 * `autoreconf --install`
 * `./configure`
 * `make && sudo make install`

## From release tarball

Requires `libjson-c-dev` with headers at `${INCLUDE_DIR}/json-c/*` >= `v0.10` (probably)

 * `tar -zxf jstruct.tar.gz`
 * `cd jstruct`
 * `./configure && make && sudo make install`

# Tests

## C `check` Tests

Requires `check` http://check.sourceforge.net/web/install.html

 * `make check` - checks the functionality of the runtime library

## Python `unittest2` Tests

Requires `python-unittest2` package

 * `tests/test_all.py` - checks the functionality of the jstruct annotation parser

<img src="https://travis-ci.org/jamie-pate/jstruct.svg">
