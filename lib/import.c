#include "jstruct.h"
#include "jstruct_import.h"
#include "jstruct_import_private.h"
#include "jstruct_private.h"

/* helpers for import */
static inline bool set_null(void *data, const struct jstruct_object_property *property) {
    if (property->nullable) {
        jstruct_null_set(data, property, true);
    } else {
        return false;
    }
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
    return jstruct_error_new(jstruct_error_invalid_type, property->name);
}

json_primitive_importer(bool, boolean)
json_primitive_importer(double, double)
json_primitive_importer(int32_t, int)

json_importer_decl(object) {
    assert(property->type.jstruct != NULL);
    struct json_object *errors = json_object_new_object();
    if (errors == NULL) {
        return jstruct_error_new(jstruct_error_json_c_op_failed, property->name);
    }
    struct jstruct_error_info err = _jstruct_import(prop, ptr, property->type.jstruct, errors);
    if (err.error != jstruct_error_none) {
        err.inner_errors = errors;
    } else {
        json_object_put(errors);
    }
    return err;
}

json_importer_decl(array) {
    return jstruct_errok;
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
json_extra_importer(float, double)

struct jstruct_error_info
_jstruct_import(struct json_object *obj, const void *data,
        const struct jstruct_object_property *properties, struct json_object *errors) {

    const struct jstruct_object_property *property;
    struct json_object *prop;
    struct jstruct_error_info result = jstruct_errok;
    for (property = properties; property->name; ++property) {
        void *ptr = jstruct_prop_ptr(data, property, 0);
        if (json_object_object_get_ex(obj, property->name, &prop)) {
            struct jstruct_error_info err = importers[json_type_index(property->type.json)](prop, data, ptr, property);
            if (err.error != jstruct_error_none) {
                result = jstruct_error_array_add_err(errors, &err);
            }
        } else {
            if (!set_null(ptr, property)) {
                result = jstruct_error_array_add(errors, jstruct_error_not_nullable, property->name);
            }
        }
    }
    return result;
}
