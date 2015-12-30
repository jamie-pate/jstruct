#include "jstruct.h"
#include "jstruct_private.h"
#include "export.h"
#include "export_private.h"

json_ctor_decl(null);
json_ctor_decl(boolean);
json_ctor_decl(double);
json_ctor_decl(int);
json_ctor_decl(object);
json_ctor_decl(array);
json_ctor_decl(string);

json_ctor_decl(int8_t);
json_ctor_decl(uint8_t);
json_ctor_decl(int16_t);
json_ctor_decl(uint16_t);
json_ctor_decl(int32_t);
json_ctor_decl(uint32_t);
json_ctor_decl(int64_t);
json_ctor_decl(uint64_t);
json_ctor_decl(float);

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
    { jstruct_extra_type_int8_t, json_ctor_name(int8_t) },
    { jstruct_extra_type_uint8_t, json_ctor_name(uint8_t) },
    { jstruct_extra_type_int16_t, json_ctor_name(int16_t) },
    { jstruct_extra_type_uint16_t, json_ctor_name(uint16_t) },
    { jstruct_extra_type_int32_t, json_ctor_name(int32_t) },
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
        assert(extra_constructors[jstruct_extra_type_int8_t]);
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
    // arrays of arrays are not allowed currently
    assert(property->type.member != json_type_array);
    struct json_object *arr = json_object_new_array();
    struct json_object *arr_member;
    if (arr) {
        struct jstruct_object_property member_prop = { NULL };
        int i;
        int len = property->length ? property->length : jstruct_length_get(data, property);

        assert(len);
        member_prop = *property;
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

json_ctor_decl(string) {
    if (*(char **)ptr == NULL) {
        assert(property->nullable);
        return NULL;
    } else {
        assert(property->type.extra == jstruct_extra_type_none);
        char *value = *(char **)ptr;
        return json_object_new_string(value);
    }
}

json_extra_ctor(int8_t, int)
json_extra_ctor(uint8_t, int)
json_extra_ctor(int16_t, int)
json_extra_ctor(uint16_t, int)
json_extra_ctor(int32_t, int)
json_extra_ctor(uint32_t, int64)
json_extra_ctor(int64_t, int64)
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
            assert(obj_prop || property->nullable);
            if (!obj_prop && !property->nullable) {
                json_object_put(obj);
                return NULL;
            }
            json_object_object_add(obj, property->name, obj_prop);
        }
    }
    return obj;
}
