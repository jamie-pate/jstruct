#include "error.h"
#include "error.init.h"
#include "error_info.init.h"
#include <errno.h>
#include <assert.h>

struct jstruct_error_info jstruct_error_new(enum jstruct_error error, char *property) {
    struct jstruct_error_info err = {
        .error=error,
        .property=property,
        .message=jstruct_error_str[error],
        .last_errno=errno
    };
    return err;
}

struct jstruct_error_info
jstruct_error_array_add_err(struct json_object *errors, struct jstruct_error_info *err) {
    if (errors == NULL) {
        if (err->inner_errors != NULL) {
            json_object_put(err->inner_errors);
            err->inner_errors = NULL;
        }
        return *err;
    }
    struct json_object *obj_err = jstruct_export(err, jstruct_error_info);
    if (obj_err == NULL) {
        // not sure what good this assert will do. Once we are failing to create json_objects
        // any error reporting that uses them is surely going to explode.
        assert(err->error != jstruct_error_json_c_op_failed);
        if (err->inner_errors != NULL) {
            json_object_put(err->inner_errors);
            err->inner_errors = NULL;
        }
        return jstruct_error_new(jstruct_error_json_c_op_failed, err->property);
    } else {
        if (err->inner_errors) {
            // TODO: 0.12 of json-c should return -1 on error but 0.11 is void (autoconf it?)
            json_object_object_add(obj_err, "inner_errors", err->inner_errors);
        }
        if (json_object_array_add(errors, obj_err) == -1) {
            return jstruct_error_new(jstruct_error_json_c_op_failed, err->property);
        };
    }
    return *err;
}

struct jstruct_error_info
jstruct_error_array_add(struct json_object *errors, enum jstruct_error error, char *property) {
    struct jstruct_error_info err = jstruct_error_new(error, property);
    return jstruct_error_array_add_err(errors, &err);
}
