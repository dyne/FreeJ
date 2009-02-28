INCLUDES = -I$(top_srcdir)/
# Add INCLUDES to CFLAGS because it seems ignored while compiling .S files.
CFLAGS += $(INCLUDES)

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libccvt.la

libccvt_la_SOURCES = \
		ccvt_c2.c \
		ccvt_mmx.S \
		ccvt_rgb2yuv.S \
		ccvt_misc.c 
