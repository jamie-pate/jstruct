#ifndef JSTRUCT_PRIVATE_H
#define JSTRUCT_PRIVATE_H

#include <assert.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof array / sizeof array[0])
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX_7
#define MAX_7(a1, a2, a3, a4, a5, a6, a7) MAX(MAX(MAX(MAX(MAX(MAX(a1, a2), a3), a4), a5), a6), a7)
#endif

#ifndef MIN_7
#define MIN_7(a1, a2, a3, a4, a5, a6, a7) MIN(MIN(MIN(MIN(MIN(MIN(a1, a2), a3), a4), a5), a6), a7)
#endif

#endif
