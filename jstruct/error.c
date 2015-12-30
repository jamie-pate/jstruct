#include "error.h"
#include "error.init.h"
#include "result.init.h"
#include "jstruct_private.h"
#include <errno.h>
#include <assert.h>

struct jstruct_result jstruct_error_new(enum jstruct_error error, char *property, int detail) {
    struct jstruct_result err = {0};
    jstruct_error_set(&err, error, property, detail);
    return err;
}

void jstruct_error_set(struct jstruct_result *result, enum jstruct_error error, char *property, int detail) {
    result->error = error;
    result->property = property;
    result->message = jstruct_error_str[error];
    result->last_errno = errno;
    result->detail = detail;
}

void jstruct_error_set_err(struct jstruct_result *result, struct jstruct_result *err) {
    jstruct_error_set(result, err->error, err->property, err->detail);
}

// clone a previous error, but set a new error code and update errno
struct jstruct_result jstruct_error_clone(struct jstruct_result *result, enum jstruct_error error) {
    struct jstruct_result err = *result;
    err.error = error;
    err.last_errno = errno;
    return err;
}

struct jstruct_result
jstruct_error_array_add_err(struct json_object *errors, struct jstruct_result *err) {
    if (errors == NULL) {
        if (err->_inner_errors != NULL) {
            json_object_put(err->_inner_errors);
            err->_inner_errors = NULL;
        }
        return *err;
    }
    struct json_object *obj_err = jstruct_export(err, jstruct_result);
    if (obj_err == NULL) {
        // not sure what good this assert will do. Once we are failing to create json_objects
        // any error reporting that uses them is surely going to explode.
        assert(err->error != jstruct_error_json_c_op_failed);
        if (err->_inner_errors != NULL) {
            json_object_put(err->_inner_errors);
            err->_inner_errors = NULL;
        }
        return jstruct_error_clone(err, jstruct_error_json_c_op_failed);
    } else {
        if (err->_inner_errors) {
            // TODO: 0.12 of json-c should return -1 on error but 0.11 is void (autoconf it?)
            json_object_object_add(obj_err, "inner_errors", err->_inner_errors);
        }
        if (json_object_array_add(errors, obj_err) == -1) {
            return jstruct_error_clone(err, jstruct_error_json_c_op_failed);
        };
    }
    return *err;
}

struct jstruct_result
jstruct_error_array_add(struct json_object *errors, enum jstruct_error error, char *property, int detail) {
    struct jstruct_result err = jstruct_error_new(error, property, detail);
    return jstruct_error_array_add_err(errors, &err);
}

void jstruct_error_consume(struct jstruct_result *consumer, struct jstruct_result *result,
        struct json_object *errors, char *property, int index) {
    if (result->error != jstruct_error_none) {
        *result = jstruct_error_array_add_err(errors, result);
        jstruct_error_set(consumer, jstruct_error_inner_error, property, index);
    }
    if (result->allocated) {
        assert(consumer->allocated);
        if (!jstruct_allocated_add(consumer->allocated,
                jstruct_allocated_type_arraylist,
                result->allocated)) {
            result->error = jstruct_error_json_c_op_failed;
            jstruct_error_set_err(consumer, result);
        }
        result->allocated = NULL;
    }
}
