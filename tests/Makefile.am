## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_srcdir)/

TESTS = check_jstruct
check_PROGRAMS = check_jstruct
check_jstruct_SOURCES = check_jstruct.c \
    check_export.c check_export.h \
    check_import.c check_import.h \
    $(top_builddir)/jstruct/jstruct.h \
    $(top_builddir)/jstruct/errors.h \
    $(top_builddir)/jstruct/export.h \
    $(top_builddir)/jstruct/import.h \
    data/basic.h data/basic.init.h
check_jstruct_CFLAGS = @CHECK_CFLAGS@
check_jstruct_LDADD = $(top_builddir)/jstruct/libjstruct.la @CHECK_LIBS@
