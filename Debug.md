#Debugging

Some command lines for debugging the unit tests:

 * configure with symbols and disable optimization: `CFLAGS="-g -O0 -std=c11 -Wpedantic" CHECK_CFLAGS=$CFLAGS ./configure`
 * attach ddd/gdb to `check_jstruct`: `LD_LIBRARY_PATH=jstruct/.libs/ CK_FORK="no" ddd tests/.libs/check_jstruct`
