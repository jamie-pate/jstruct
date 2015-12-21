## Process this file with automake to produce Makefile.in
AM_CPPFLAGS=-Wpedantic -std=c11

lib_LTLIBRARIES = libjstruct.la
libjstruct_la_SOURCES = jstruct.c jstruct.h \
    jstruct_private.h jstruct_private.c \
    jstruct_error.h jstruct_error.c \
    export.c jstruct_export.h jstruct_export_private.h \
    import.c jstruct_import.h jstruct_import_private.h \
include_HEADERS = jstruct.h jstruct_export.h jstruct_import.h jstruct_error.h
