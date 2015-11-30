# jstruct

Declaratively translate data between JSON and C.

The jstruct preprocessor generates C code that transforms data between C structures and JSON strings.
It reads annotated comments from the structure declarations in your header,
and creates custom macros and functions that efficiently and automatically do your dirty work.

# Sample

```
//main_jstruct.hstruct (converted to main_jstruct.h)
//@json
struct my_json_data {
  long long id;

  /* don't include in json */
  //@private
  int _id;

  /* add the ability to null this field even though it's not a pointer */
  //@nullable
  double ratio;
  char *name;

  /*TODO: (is @array necessary?)*/
  //@array
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
    jstruct_init_malloc(container, .array_data, struct my_json_data, 2)

    struct json_object *obj = jstruct_export(&container, my_json_container);
    if (obj) {
        printf("%s", json_object_to_json_string(obj));
    }
}
```

# Build and Install

## From git

 Required Packages: `autoconf`,`libtool`,`libjson-c-dev`

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
