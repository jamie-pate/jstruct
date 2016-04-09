#ifndef JSTRUCT_RESULT_H
#define JSTRUCT_RESULT_H

/*
TODO: create a make task for this and remove result.h and result.init.h from git.
For now run the following instead:
parse/jstruct_parse.py jstruct/result.jstruct.h . util/fake_libc_include
*/
#include JSON_OBJECT_H
#include <errno.h>
// @json
struct jstruct_result {
    // @private
    struct array_list *allocated;
    enum jstruct_error error;
    char *message;
    // @nullable
    char *property;
    int detail;
    int last_errno;
    // TODO: M6: automatically get a ref to inner json_object properties and import/export them?
    // @private
    // _inner_errors may be assigned for any error type except jstruct_error_none! currently it's
    // the caller's responsibility to json_object_put() this property.
    // TODO: include this in *allocated (user could still json_object_get()) to retain a handle
    struct json_object *_inner_errors;
};

#endif
