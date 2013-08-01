ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = bbcode
bbcode_SOURCES = main.c strut.c strut.h bbcode.h parse.c print.c
bbcode_CFLAGS = $(AM_CFLAGS)

lib_LIBRARIES = libbbcode.a
libbbcode_a_SOURCES = strut.c bbcode.h strut.h parse.c print.c
libbbcode_a_CFLAGS = $(AM_CFLAGS)

lib_LTLIBRARIES = libbbcode.la
libbbcode_la_SOURCES = strut.c bbcode.h strut.h parse.c print.c

include_HEADERS = bbcode.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bbcode.pc
