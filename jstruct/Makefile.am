## Process this file with automake to produce Makefile.in
AM_CPPFLAGS=-Wpedantic -std=c11

lib_LTLIBRARIES = libjstruct.la
libjstruct_la_SOURCES = jstruct.c jstruct.h \
    jstruct_private.h jstruct_private.c \
    error.h error.c \
    export.c export.h export_private.h \
    import.c import.h import_private.h \
include_HEADERS = jstruct.h export.h import.h error.h
