#include "jstruct.h"
#include "export.h"
#include "export_private.h"
#include "jstruct_private.h"


json_ctor_decl(null);
json_ctor_decl(boolean);
json_ctor_decl(double);
json_ctor_decl(int);
json_ctor_decl(object);
json_ctor_decl(array);
json_ctor_decl(string);

json_ctor_decl(uint);
json_ctor_decl(int64);
json_ctor_decl(uint64);
json_ctor_decl(float);
// long long, unsigned, float, char, short etc

struct jt_ctor constructor_list[] = {
    { json_type_null, json_ctor(null) },
    { json_type_boolean, json_ctor(boolean) },
    { json_type_double, json_ctor(double) },
    { json_type_int, json_ctor(int) },
    { json_type_object, json_ctor(object) },
    { json_type_array, json_ctor(array) },
    { json_type_string, json_ctor(string) },
};

struct jt_extra_ctor extra_constructor_list[] = {
    { jstruct_extra_type_none, NULL },
    { jstruct_extra_type_uint, json_ctor(uint) },
    { jstruct_extra_type_int64, json_ctor(int64) },
    { jstruct_extra_type_uint64, json_ctor(uint64) },
    { jstruct_extra_type_float, json_ctor(float) },
};

jstruct_export_ctor constructors[ARRAYSIZE(constructor_list)] = { NULL };
jstruct_export_ctor extra_constructors[ARRAYSIZE(extra_constructor_list)] = { NULL };

static inline void _init_constructors() {
    int first_idx = json_type_index(json_type_first);
    if (!constructors[first_idx]) {
        int i;
        for (i = 0; i < ARRAYSIZE(constructor_list); ++i) {
            struct jt_ctor *type_ctor = &constructor_list[i];
            int idx = json_type_index(type_ctor->type);
            constructors[idx] = type_ctor->ctor;
        }
        assert(constructors[first_idx]);
        for (i = 0; i < ARRAYSIZE(extra_constructor_list); ++i) {
            struct jt_extra_ctor *extra_type_ctor = &extra_constructor_list[i];
            extra_constructors[extra_type_ctor->type] = extra_type_ctor->ctor;
        }
        assert(extra_constructors[jstruct_extra_type_uint]);
    }
}

json_ctor_decl(null) {
    return NULL;
}

json_primitive_ctor(json_bool, boolean);
json_primitive_ctor(double, double);
json_primitive_ctor(int32_t, int);

json_ctor_decl(object) {
    return json_object_new_object();
}

json_ctor_decl(array) {
    return json_object_new_array();
}

json_primitive_ctor(char *, string);

json_extra_ctor(uint32_t, uint, int64);
json_extra_ctor(int64_t, int64, int64);
// daes this even work+
json_extra_ctor(uint64_t, uint64, int64);
json_extra_ctor(float, float, double);

struct json_object *_jstruct_export(const void *data,
        const struct jstruct_object_property *properties) {

    _init_constructors();
    struct jstruct_object_property *property;
    struct json_object *obj;
    struct json_object *obj_prop;
    obj = json_object_new_object();
    for (property = properties; property->name; ++property) {
        obj_prop = constructors[json_type_index(property->type.json)](data, property);
        json_object_object_add(obj, property->name, obj_prop);
    }
    return obj;
}
