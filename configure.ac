AC_INIT([libjstruct.la], [0.1], [jamie.pate@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CHECK_LIB([json-c], [json_object_object_get_ex])
AM_PATH_CHECK
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile lib/Makefile tests/Makefile])
AC_OUTPUT
