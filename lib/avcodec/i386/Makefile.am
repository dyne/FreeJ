
INCLUDES = -I$(top_srcdir)/lib/avcodec

AM_CFLAGS= -O -DHAVE_AV_CONFIG_H -I.. -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE

AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libavcodecmmx.a

libavcodecmmx_a_SOURCES = \
cputest.c \
dsputil_mmx.c \
fdct_mmx.c \
idct_mmx.c \
motion_est_mmx.c \
mpegvideo_mmx.c \
simple_idct_mmx.c \
vp3dsp_mmx.c \
vp3dsp_sse2.c

