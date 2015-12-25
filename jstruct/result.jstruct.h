#ifndef JSTRUCT_RESULT_H
#define JSTRUCT_RESULT_H

/*
TODO: create a make task for this and remove result.h and result.init.h from git.
For now run the following instead:
parse/jstruct_parse.py jstruct/result.jstruct.h . util/fake_libc_include
*/
#include <json-c/json_object.h>
#include <errno.h>
// @json
struct jstruct_result {
    // @private
    struct array_list *allocated;
    enum jstruct_error error;
    char *message;
    char *property;
    int index;
    int last_errno;
    // TODO: M6: automatically get a ref to inner json_object properties and import/export them?
    // @private
    struct json_object *_inner_errors;
};

#endif
