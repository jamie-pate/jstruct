## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(srcdir)/../lib

TESTS = check_jstruct
check_PROGRAMS = check_jstruct
check_jstruct_SOURCES = check_jstruct.c $(top_builddir)/lib/jstruct.h
check_jstruct_CFLAGS = @CHECK_CFLAGS@
check_jstruct_LDADD = $(top_builddir)/lib/libjstruct.la @CHECK_LIBS@
