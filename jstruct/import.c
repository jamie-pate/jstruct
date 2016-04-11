#include "jstruct.h"
#include "jstruct_private.h"
#include "import.h"
#include "import_private.h"

/* helpers for import */
static inline bool set_null(void *data, const struct jstruct_object_property *property) {
    if (property->nullable) {
        jstruct_null_set(data, property, true);
    } else {
        return false;
    }
    return true;
}

/* importers */
json_importer_decl(null);
json_importer_decl(boolean);
json_importer_decl(double);
json_importer_decl(int);
json_importer_decl(object);
json_importer_decl(array);
json_importer_decl(string);

json_importer_decl(int8_t);
json_importer_decl(uint8_t);
json_importer_decl(int16_t);
json_importer_decl(uint16_t);
json_importer_decl(int32_t);
json_importer_decl(uint32_t);
json_importer_decl(int64_t);
json_importer_decl(uint64_t);
json_importer_decl(unsigned_int);
json_importer_decl(long_long);
json_importer_decl(unsigned_long_long);
json_importer_decl(float);

struct jt_importer importer_list[] = {
    { json_type_null, json_importer_name(null) },
    { json_type_boolean, json_importer_name(boolean) },
    { json_type_double, json_importer_name(double) },
    { json_type_int, json_importer_name(int) },
    { json_type_object, json_importer_name(object) },
    { json_type_array, json_importer_name(array) },
    { json_type_string, json_importer_name(string) },
};

struct jt_extra_importer extra_importer_list[] = {
    { jstruct_extra_type_none, NULL },
    { jstruct_extra_type_int8_t, json_importer_name(int8_t) },
    { jstruct_extra_type_uint8_t, json_importer_name(uint8_t) },
    { jstruct_extra_type_int16_t, json_importer_name(int16_t) },
    { jstruct_extra_type_uint16_t, json_importer_name(uint16_t) },
    { jstruct_extra_type_int32_t, json_importer_name(int32_t) },
    { jstruct_extra_type_uint32_t, json_importer_name(uint32_t) },
    { jstruct_extra_type_int64_t, json_importer_name(int64_t) },
    { jstruct_extra_type_uint64_t, json_importer_name(uint64_t) },
    { jstruct_extra_type_unsigned_int, json_importer_name(unsigned_int) },
    { jstruct_extra_type_long_long, json_importer_name(long_long) },
    { jstruct_extra_type_unsigned_long_long, json_importer_name(unsigned_long_long) },
    { jstruct_extra_type_float, json_importer_name(float) },
};

jstruct_import_importer importers[ARRAYSIZE(importer_list)] = { NULL };
jstruct_import_importer extra_importers[ARRAYSIZE(extra_importer_list)] = { NULL };

static inline void _init_importers() {
    int first_idx = json_type_index(json_type_first);
    if (!importers[first_idx]) {
        int i;
        for (i = 0; i < ARRAYSIZE(importer_list); ++i) {
            struct jt_importer *type_importer = &importer_list[i];
            int idx = json_type_index(type_importer->type);
            importers[idx] = type_importer->importer;
        }
        assert(importers[first_idx]);
        for (i = 0; i < ARRAYSIZE(extra_importer_list); ++i) {
            struct jt_extra_importer *extra_type_importer = &extra_importer_list[i];
            extra_importers[extra_type_importer->type] = extra_type_importer->importer;
        }
        assert(extra_importers[jstruct_extra_type_int8_t]);
    }
}

json_importer_decl(null) {
    // not sure why this fn exists except to take up a spot in importer_list
    assert(false);
    return jstruct_error_new(jstruct_error_invalid_type, property->name, 0);
}

json_primitive_importer(bool, boolean)
json_primitive_importer(double, double)
json_primitive_importer(int32_t, int)

json_importer_decl(object) {
    assert(property->type.jstruct != NULL);
    struct json_object *errors = json_object_new_array();
    if (errors == NULL) {
        return jstruct_error_new(jstruct_error_json_c_op_failed, property->name, 0);
    }
    struct jstruct_result result = _jstruct_import(prop, ptr, property->type.jstruct, errors);
    if (result.error != jstruct_error_none && json_object_array_length(errors)) {
        jstruct_error_set(&result, jstruct_error_inner_error, property->name, -1);
        result._inner_errors = errors;
    } else {
        json_object_put(errors);
    }
    return result;
}

json_importer_decl(array) {
    // arrays of arrays are not allowed currently
    assert(property->type.member != json_type_array);
    if (json_object_get_type(prop) != json_type_array) {
        return jstruct_error_new(jstruct_error_incorrect_type, property->name, -1);
    }
    struct jstruct_result result = JSTRUCT_OK;
    result.allocated = array_list_new(jstruct_allocated_free);
    if (!result.allocated) {
        return jstruct_error_new(jstruct_error_json_c_op_failed, property->name, -1);
    }
    int i;
    int len = json_object_array_length(prop);
    struct json_object *arr_member;
    struct json_object *errors = json_object_new_array();
    if (!errors) {
        array_list_free(result.allocated);
        return jstruct_error_new(jstruct_error_json_c_op_failed, property->name, -1);
    }
    struct jstruct_result member_result;
    jstruct_import_importer import = importers[json_type_index(property->type.member)];
    struct jstruct_object_property member_property = *property;
    member_property.type.json = member_property.type.member;
    // property's constant length won't be set if it's a dynamic array/pointer
    if (property->length == 0) {
        jstruct_length_set(data, property, len);
    } else {
        if (len != property->length) {
            jstruct_error_set(&result, jstruct_error_incorrect_length, property->name, property->length);
            jstruct_error_array_add_err(errors, &result);
            len = MAX(len, property->length);
        }
    }
    void *members = calloc(len, property->stride);
    *(void **)ptr = members;

    for (i = 0; i < len; ++i) {
        arr_member = json_object_array_get_idx(prop, i);
        if (json_object_get_type(arr_member) != member_property.type.json) {
            member_result = jstruct_error_new(jstruct_error_incorrect_type, property->name, i);
        } else {
            void *member_ptr = jstruct_prop_ptr(data, property, i);
            member_result = import(arr_member, data, member_ptr, &member_property);
        }
        jstruct_error_consume(&result, &member_result, errors, NULL, i);
    }
    if (!jstruct_allocated_add(result.allocated, jstruct_allocated_type_raw, members)) {
        jstruct_error_set(&result, jstruct_error_json_c_op_failed, property->name, -1);
    }
    if (json_object_array_length(errors) > 0) {
        result._inner_errors = errors;
    } else {
        json_object_put(errors);
    }
    return result;
}

json_primitive_importer(char *, string)

json_extra_importer(int8_t, int)
json_extra_importer(uint8_t, int)
json_extra_importer(int16_t, int)
json_extra_importer(uint16_t, int)
json_extra_importer(int32_t, int)
json_extra_importer(uint32_t, int64)
json_extra_importer(int64_t, int64)
json_extra_importer(uint64_t, int64)
json_extra_importer_camel(unsigned_int, unsigned int, int64)
json_extra_importer_camel(long_long, long long, int64)
json_extra_importer_camel(unsigned_long_long, unsigned long long, int64)
json_extra_importer(float, double)

struct jstruct_result
_jstruct_import(struct json_object *obj, const void *data,
        const struct jstruct_object_property *properties, struct json_object *errors) {
    _init_importers();
    if (errors != NULL && json_object_get_type(errors) != json_type_array) {
        return jstruct_error_new(jstruct_error_errors_not_array_or_null, NULL, json_object_get_type(errors));
    }
    const struct jstruct_object_property *property;
    struct json_object *prop;
    struct jstruct_result result = JSTRUCT_OK;
    result.allocated = array_list_new(jstruct_allocated_free);
    for (property = properties; property->name; ++property) {
        void *ptr = jstruct_prop_ptr(data, property, JSTRUCT_PROP_PTR_GET_NO_DEREF);
        struct jstruct_result err;
        if (json_object_object_get_ex(obj, property->name, &prop)) {
            if (json_object_get_type(prop) != property->type.json) {
                err = jstruct_error_new(jstruct_error_incorrect_type, property->name, json_object_get_type(prop));
            } else {
                jstruct_import_importer import = importers[json_type_index(property->type.json)];
                err = import(prop, data, ptr, property);
            }
        } else {
            if (!set_null(ptr, property)) {
                err = jstruct_error_array_add(errors, jstruct_error_not_nullable, property->name, 0);
            }
        }
        jstruct_error_consume(&result, &err, errors, property->name, -1);
    }
    if (result.allocated->length == 0) {
        array_list_free(result.allocated);
        result.allocated = NULL;
    }
    return result;
}
