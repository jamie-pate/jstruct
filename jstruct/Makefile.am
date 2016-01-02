## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libjstruct.la

libjstructincludedir = $(includedir)/jstruct
libjstruct_la_SOURCES = jstruct.c jstruct_private.c error.c export.c import.c
# libtool -version-info c:r:a
# no interface change: bump revision only, don’t touch current nor age.
# interface change: set revision to 0, bump current and age.
# recompile/link change:  Bump current, set revision and age to 0.
libjstruct_la_LDFLAGS = -version-info 0:0:0
libjstructinclude_HEADERS = jstruct.h export.h import.h error.h result.h
noinst_HEADERS = jstruct_private.h export_private.h import_private.h error.init.h result.init.h
