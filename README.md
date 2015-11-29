# jstruct

Declaratively translate data between JSON and C.

The jstruct preprocessor generates C code that transforms data between C structures and JSON strings.
It reads annotated comments from the structure declarations in your header,
and creates custom macros and functions that efficiently and automatically do your dirty work.

# Sample

```
//main_jstruct.hstruct -> main_jstruct.h
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
  //@array /*TODO: (necessary?)*/
  char **tags;
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

//main.c
#include "main_jstruct.h"
#include "json_object.h"

int main() {
    struct my_json_container container={
        .main_data={
            .id=1,
            ._id=2,
            .ratio=3.5,
            .name="main_data",
            .tags={"main", "data", "sample"}
        },
        .array_data={
            {
                .id=3,
                .ratio__null__=true
            },{.id=5},{.id=6}
        },
        /* inline malloc macro? */
        jstruct_init_malloc(.alloc_array_data, my_json_data, 2)
    }

    struct json_object *obj = jstruct_export(&container, my_json_container);
    if (obj) {
        printf("%s", json_object_to_json_string(obj));
    }
}
```

# Build and Install

## From git

 * install check: http://check.sourceforge.net/web/install.html (needed for autoreconf)
 * git clone *repo*
 * `libtoolize`
 * `autoreconf --install [-I /usr/local/share/aclocal]` (check.m4 may be installed somewhere that autoreconf can't find it)
 * `./configure`
 * `make && sudo make install`

## From release tarball

Requires libjson-c ?

(Coming soon)

 * `tar -zxf jstruct.tar.gz`
 * `cd jstruct`
 * `./ configure && make && sudo make install`
