INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/avformat -I$(top_srcdir)/lib/avcodec

AM_CFLAGS = -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DCONFIG_ENCODERS

AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libavformat.a

libavformat_a_SOURCES = \
	utils.c cutils.c os_support.c allformats.c \
	mpeg.c mpegts.c mpegtsenc.c ffm.c crc.c img.c img2.c raw.c rm.c \
	avienc.c avidec.c wav.c swf.c au.c gif.c mov.c mpjpeg.c dv.c \
	yuv4mpeg.c 4xm.c flvenc.c flvdec.c movenc.c psxstr.c idroq.c ipmovie.c \
	nut.c wc3movie.c mp3.c westwood.c segafilm.c idcin.c flic.c \
	sierravmd.c matroska.c sol.c electronicarts.c nsvdec.c asf.c asf-enc.c \
	ogg2.c oggparsevorbis.c oggparsetheora.c oggparseflac.c \
	amr.c \
	pnm.c yuv.c png.c jpeg.c gifdec.c sgi.c \
	avio.c aviobuf.c file.c \
	framehook.c \
	dv1394.c \
	udp.c tcp.c http.c rtsp.c rtp.c rtpproto.c \
	ogg.c

#	dc1394.c
