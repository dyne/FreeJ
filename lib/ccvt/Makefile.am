# We need config.h so use top_builddir instead of top_srcdir
AM_CPPFLAGS = -I$(top_builddir)/
AM_CCASFLAGS = -I$(top_builddir)/

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libccvt.la

libccvt_la_SOURCES = \
		ccvt_c2.c \
		ccvt_mmx.S \
		ccvt_rgb2yuv.S \
		ccvt_misc.c

noinst_HEADERS = ccvt.h ccvt_types.h
