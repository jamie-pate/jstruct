# jstruct

C Library and code generator that declaratively transforms data between JSON and C.

The jstruct preprocessor generates C code that transforms data between C structures and JSON strings.
It reads annotated comments from the structure declarations in your header,
and creates c data structures that allow it to efficiently and automatically import/export JSON data.

## Development Roadmap

 * ~~*M1*~~ Export structs containing primitive types and arrays to same. (char*, int, bool, uint64_t etc)
 * ~~*M2*~~ Mechanical parsing of annotated header files to produce augmented struct declarations and data tables used by the export process.
 * ~~*M3*~~ Export structs containing other structures and arrays (struct* and struct[])
 * *M4* Import structs containing primitive types and arrays
 * *M5* Import nested structs and arrays of structs

### Future
Mix json_object * structs into annotated structs in a sane manner.

# Sample

```
//main_jstruct.hstruct (converted to main_jstruct.h)
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

    /* automatically parsed as an array atm*/
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

//main.c
#include "main_jstruct.h"
#include "json_object.h"

int main() {
    char *data_tags[] = {"main", "data", "sample"};
    struct my_json_container container={
        .main_data={
            .id=1,
            ._id=2,
            .ratio=3.5,
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
    jstruct_init_malloc(container, array_data, struct my_json_data, 2)

    struct json_object *obj = jstruct_export(&container, my_json_container);
    if (obj) {
        printf("%s", json_object_to_json_string(obj));
    }
}
```

# Build and Install

## From git

 Required Packages: `autoconf`,`libtool`,`libjson-c-dev`, `python-pycparser`

 * install check: http://check.sourceforge.net/web/install.html (needed for autoreconf) or `svn checkout svn://svn.code.sf.net/p/check/code/trunk check-code && cd check-code` and follow the instructions in `README`
 * git clone *repo*
 * `libtoolize`
 * `autoreconf --install`
 * `./configure`
 * `make && sudo make install`

## From release tarball

Requires libjson-c

(Coming soon)

 * `tar -zxf jstruct.tar.gz`
 * `cd jstruct`
 * `./ configure && make && sudo make install`

# Tests

## C `check` Tests

Requires `check` http://check.sourceforge.net/web/install.html

 * `make check` - checks the functionality of the runtime library

## Python `unittest2` Tests

Requires `python-unittest2` package

 * `tests/test_all.py` - checks the functionality of the jstruct annotation parser
