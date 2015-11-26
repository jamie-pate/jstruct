# jstruct

Declaratively translate data between JSON and C.

The jstruct preprocessor generates C code that transforms data between C structures and JSON strings.
It reads annotated comments from the structure declarations in your header,
and creates custom macros and functions that efficiently and automatically do your dirty work.

# sample

```
//main.h
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
```
