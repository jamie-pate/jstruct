AC_INIT([libjstruct.la], [0.0.6], [jamie.pate@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_ASSERT

AC_CHECK_HEADER([errno.h])
AC_CHECK_HEADER([json-c/json_object.h], [AC_DEFINE([JSON_OBJECT_H], [<json-c/json_object.h>], [Location of json_object.h header])])
AC_CHECK_HEADER([json-c/arraylist.h], [AC_DEFINE([ARRAYLIST_H], [<json-c/arraylist.h>], [Location of arraylist.h header])])
AC_CHECK_LIB([json-c], [json_object_object_get_ex])

#check for broken gcc http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
my_save_cflags="$CFLAGS"
CFLAGS="-Werror -Wall -std=c99"
AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM(
        [[struct tstruct { int i; }; struct ttstruct { struct tstruct t; };]],
        [[struct ttstruct ttt = {0};if (ttt.t.i) { return ttt.t.i; }]]
    )],
    [WERR="-Werror"],
    [WERR=""]
)
CFLAGS=$my_save_cflags

AX_APPEND_COMPILE_FLAGS([ -Wall $WERR -Wpedantic -pedantic -std=c99 -std=c11 ])
AC_SUBST(AM_CPP_CHECKEDFLAGS)

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
AM_INIT_AUTOMAKE([foreign -Werror subdir-objects])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile jstruct/Makefile tests/Makefile])
AC_OUTPUT