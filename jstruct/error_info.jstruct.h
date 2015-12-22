#ifndef JSTRUCT_ERROR_INFO_H
#define JSTRUCT_ERROR_INFO_H

/*
TODO: create a make task for this and remove error_info.h and error_info.init.h from git.
For now run the following instead:
python parse/jstruct_parse.py jstruct/error_info.jstruct.h -- jstruct util/fake_libc_include
*/
#import <json-c/json_object.h>
#import <errno.h>
// @json
struct jstruct_error_info {
    enum jstruct_error error;
    char *message;
    char *property;
    errno_t last_errno;
    // TODO: M6: automatically get a ref to inner json objects and import/export them?
    // @private
    struct json_object *inner_errors;
};

#endif
