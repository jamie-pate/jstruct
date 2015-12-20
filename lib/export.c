#include "jstruct.h"
#include "jstruct_export.h"
#include "jstruct_export_private.h"
#include "jstruct_private.h"

/* helpers that don't need to be macros */

static inline int
jstruct_length_get(const void *data, const struct jstruct_object_property *property) {
    assert(data);assert(property);
    return *(int *)((unsigned char *)data + property->length_offset);
}

static inline void *
jstruct_prop_ptr(const void *data, const struct jstruct_object_property *property, int index) {
    unsigned char *ptr = ((unsigned char *)data + property->offset);
    if (property->dereference) {
        ptr = *(void **)ptr;
    }
    if (index) {
        assert(property->stride);
        ptr += index * property->stride;
    }
    return ptr;
}

json_ctor_decl(null);
json_ctor_decl(boolean);
json_ctor_decl(double);
json_ctor_decl(int);
json_ctor_decl(object);
json_ctor_decl(array);
json_ctor_decl(string);

json_ctor_decl(uint32_t);
json_ctor_decl(int64_t);
json_ctor_decl(uint64_t);
json_ctor_decl(float);
// long long, unsigned, float, char, short etc

struct jt_ctor constructor_list[] = {
    { json_type_null, json_ctor_name(null) },
    { json_type_boolean, json_ctor_name(boolean) },
    { json_type_double, json_ctor_name(double) },
    { json_type_int, json_ctor_name(int) },
    { json_type_object, json_ctor_name(object) },
    { json_type_array, json_ctor_name(array) },
    { json_type_string, json_ctor_name(string) },
};

struct jt_extra_ctor extra_constructor_list[] = {
    { jstruct_extra_type_none, NULL },
    { jstruct_extra_type_uint32_t, json_ctor_name(uint32_t) },
    { jstruct_extra_type_int64_t, json_ctor_name(int64_t) },
    { jstruct_extra_type_uint64_t, json_ctor_name(uint64_t) },
    { jstruct_extra_type_float, json_ctor_name(float) },
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
        assert(extra_constructors[jstruct_extra_type_uint32_t]);
    }
}

json_ctor_decl(null) {
    return NULL;
}

json_primitive_ctor(bool, boolean)
json_primitive_ctor(double, double)
json_primitive_ctor(int32_t, int)

json_ctor_decl(object) {
    assert(property->type.jstruct != NULL);
    return _jstruct_export(ptr, property->type.jstruct);
}

json_ctor_decl(array) {
    struct json_object *arr = json_object_new_array();
    struct json_object *arr_member;
    if (arr) {
        struct jstruct_object_property member_prop = { NULL };
        int i;
        int len = property->length ? property->length : jstruct_length_get(data, property);

        assert(len);
        memcpy(&member_prop, property, sizeof(member_prop));
        member_prop.type.json = member_prop.type.member;
        for (i = 0; i < len; ++i) {
            void *ptr = jstruct_prop_ptr(data, property, i);
            arr_member = constructors[json_type_index(member_prop.type.json)](data, ptr, &member_prop);
            if (!arr_member) {
                json_object_put(arr);
                return NULL;
            }
            json_object_array_add(arr, arr_member);
        }
    }
    return arr;
}

json_primitive_ctor(char *, string)

json_extra_ctor(uint32_t, int64)
json_extra_ctor(int64_t, int64)
// daes this even work+
json_extra_ctor(uint64_t, int64)
json_extra_ctor(float, double)

struct json_object *_jstruct_export(const void *data,
        const struct jstruct_object_property *properties) {

    _init_constructors();
    const struct jstruct_object_property *property;
    struct json_object *obj;
    struct json_object *obj_prop;
    obj = json_object_new_object();
    if (obj) {
        for (property = properties; property->name; ++property) {
            void *ptr = jstruct_prop_ptr(data, property, 0);
            obj_prop = constructors[json_type_index(property->type.json)](data, ptr, property);
            if (!obj_prop) {
                json_object_put(obj);
                return NULL;
            }
            json_object_object_add(obj, property->name, obj_prop);
        }
    }
    return obj;
}
