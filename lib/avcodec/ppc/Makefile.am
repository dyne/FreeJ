
INCLUDES = -I$(top_srcdir)/lib/avcodec

AM_CFLAGS= -DHAVE_AV_CONFIG_H -I.. -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE

AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libavcodecppc.a

libavcodecppc_a_SOURCES = \
dsputil_altivec.c \
dsputil_h264_altivec.c \
dsputil_ppc.c \
fdct_altivec.c \
fft_altivec.c \
gmc_altivec.c \
idct_altivec.c \
mpegvideo_altivec.c \
mpegvideo_ppc.c
