## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(srcdir)/../lib

TESTS = check_jstruct
check_PROGRAMS = check_jstruct
check_jstruct_SOURCES = check_jstruct.c \
    check_export.c check_export.h \
    check_import.c check_import.h \
    $(top_builddir)/lib/jstruct.h \
    $(top_builddir)/lib/export.h \
    $(top_builddir)/lib/import.h \
    data/basic.h
check_jstruct_CFLAGS = @CHECK_CFLAGS@
check_jstruct_LDADD = $(top_builddir)/lib/libjstruct.la @CHECK_LIBS@
