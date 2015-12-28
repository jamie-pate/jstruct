AC_INIT([libjstruct.la], [0.1.1], [jamie.pate@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_ASSERT

AC_CHECK_HEADER([errno.h])
AC_CHECK_LIB([json-c], [json_object_object_get_ex])
AX_APPEND_COMPILE_FLAGS([ -Wall -Wpedantic -pedantic -std=c99 ])
AC_SUBST(AM_CPP_CHECKEDFLAGS)

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
AM_INIT_AUTOMAKE([foreign -Werror])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile jstruct/Makefile tests/Makefile])
AC_OUTPUT
