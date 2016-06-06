## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_srcdir)/

TESTS = check_jstruct
check_PROGRAMS = check_jstruct
check_jstruct_SOURCES = data/basic.init.c check_jstruct.c check_export.c check_import.c \
    data/basic.h
check_jstruct_CFLAGS = @CHECK_CFLAGS@
check_jstruct_LDADD = $(top_builddir)/jstruct/libjstruct.la @CHECK_LIBS@

PARSE_DEFINES = 
EXTRA_INCLUDE_PATHS = $(top_builddir)/ $(top_builddir)/util/fake_libc_include/


data/basic.h data/basic.init.c: data/basic.jstruct.h
	$(PYTHON) $(top_builddir)/parse/jstruct_parse.py $(PARSE_DEFINES) $< $(EXTRA_INCLUDE_PATHS) 