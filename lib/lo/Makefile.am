INCLUDES = -I$(top_srcdir)/lib

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = liblo.la

liblo_la_SOURCES = address.c send.c message.c server.c method.c \
	server_thread.c blob.c bundle.c timetag.c pattern_match.c

noinst_HEADERS = lo_endian.h lo_errors.h lo.h lo_lowlevel.h \
	lo_macros.h lo_osc_types.h lo_throw.h lo_types.h lo_types_internal.h
