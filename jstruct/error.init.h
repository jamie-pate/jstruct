#ifndef JSTRUCT_ERROR_INIT_H
#define JSTRUCT_ERROR_INIT_H
// Just extern initializers. Include once in a single c file.

#include "error.h"

char *jstruct_error_str[] = { JSTRUCT_ERROR_STR_INIT };

// Returned by all fns that return jstruct_error when the operation succeeds
struct jstruct_error_info jstruct_errok = {
    .error=jstruct_error_none,
    .message=JSTRUCT_ERROR_STR_NONE
};

#endif
