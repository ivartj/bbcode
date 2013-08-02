ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = bbcode
bbcode_SOURCES = main.c bbcode.h bbcode.c
bbcode_CFLAGS = $(AM_CFLAGS)

lib_LIBRARIES = libbbcode.a
libbbcode_a_SOURCES = bcode.h bbcode.c
libbbcode_a_CFLAGS = $(AM_CFLAGS)

lib_LTLIBRARIES = libbbcode.la
libbbcode_la_SOURCES = bbcode.h bbcode.c

include_HEADERS = bbcode.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bbcode.pc
