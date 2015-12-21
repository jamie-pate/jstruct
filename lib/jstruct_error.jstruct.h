#ifndef JSTRUCT_ERROR_H
#define JSTRUCT_ERROR_H

typedef enum jstruct_error {
    jstruct_error_none=0,
    jstruct_error_json_c_op_failed,
    jstruct_error_not_nullable,
    jstruct_error_inner_error,
    jstruct_error_invalid_type
} jstruct_error;
#define JSTRUCT_ERROR_STR_NONE "No Error"
char *jstruct_error_str[] = {
    JSTRUCT_ERROR_STR_NONE,
    // Split into separate errors for different operations?
    "Error occurred in json-c. Check errno for details",
    "Property not nullable but value missing or null",
    "Child property error: see inner_error",
    "Invalid property type. (This shouldn't happen)"
};

//@json
struct jstruct_error_info {
    enum jstruct_error error;
    char *message;
    char *property;
    int last_errno;
    // TODO: automatically get a ref to inner json objects and import/export them?
    // @private
    struct json_object *inner_errors;
};

// Returned by all fns that return jstruct_error when the operation succeeds
struct jstruct_error_info jstruct_errok = {
    .error=jstruct_error_none,
    .message=JSTRUCT_ERROR_STR_NONE
};

// export a jstruct_error_info and append it to errors, which should be an Array
// if errors is NULL returns the input error
// strips err->inner_errors if present and nests them in the exported json
struct jstruct_error_info
jstruct_error_array_add_err(struct json_object *errors, struct jstruct_error_info *err);

// Create and export a jstruct_error_info and append it to errors, which should be an Array
// errors will contain a graph of any errors that occurred
// if errors is NULL returns the error struct created
// otherwise returns error struct for the error passed in
// if any error is encountered return the new error
// (probably jstruct_error_json_c_new_failed, errno may be set by json-c)
struct jstruct_error_info
jstruct_error_array_add(struct json_object *errors, enum jstruct_error error, char *property);

struct jstruct_error_info jstruct_error_new(enum jstruct_error error, char *property);

#endif
