INCLUDES = -I$(top_srcdir)/lib/theora-mmx

AM_CFLAGS= -DHAVE_AV_CONFIG_H -I.. -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE


AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libavcodec.a

libavcodec_a_SOURCES = \
	bitstream.c utils.c mem.c allcodecs.c \
	mpegvideo.c jrevdct.c jfdctfst.c jfdctint.c\
	mpegaudio.c ac3enc.c mjpeg.c resample.c resample2.c dsputil.c \
	motion_est.c imgconvert.c imgresample.c \
	mpeg12.c mpegaudiodec.c pcm.c simple_idct.c \
	ratecontrol.c adpcm.c eval.c error_resilience.c \
	fft.c mdct.c raw.c golomb.c cabac.c\
	dpcm.c adx.c rational.c faandct.c parser.c g726.c \
	vp3dsp.c integer.c h264idct.c rangecoder.c pnm.c h263.c \
	msmpeg4.c h263dec.c dvdsub.c dvbsub.c \
	aasc.c \
	alac.c \
	asv1.c \
	cinepak.c \
	cljr.c \
	cyuv.c \
	dv.c \
	8bps.c \
	ffv1.c \
	flac.c \
	flicvideo.c \
	4xm.c \
        fraps.c               \
        h261.c \
        h264.c \
        huffyuv.c \
        idcinvideo.c \
        indeo2.c \
        indeo3.c \
        interplayvideo.c \
        lcl.c \
        loco.c \
        mace.c \
        msrle.c \
        msvideo1.c \
        png.c \
        qdrw.c \
        qpeg.c \
        qtrle.c \
        ra144.c \
        ra288.c \
        roqvideo.c \
        rpza.c \
        rv10.c \
        shorten.c \
        smc.c \
        snow.c \
        sonic.c \
        svq1.c \
        truemotion1.c \
        tscc.c \
        ulti.c \
        vc9.c \
        vcr1.c \
        vmdav.c \
        vorbis.c \
        vp3.c \
        vqavideo.c \
        wmadec.c \
        wnv1.c \
        ws-snd1.c \
        xan.c \
        xl.c \
        pthread.c \
        a52dec.c \
        liba52/bit_allocate.c liba52/bitstream_a52.c liba52/downmix.c \
        liba52/imdct.c  liba52/parse.c liba52/crc.c liba52/resample_a52.c \
        libpostproc/postprocess.c \
	oggvorbis.c \
        oggtheora.c \
        i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c \
	i386/idct_mmx.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/vp3dsp_mmx.c \
	i386/vp3dsp_sse2.c
# i386/fft_sse.c

# ifeq ($(CONFIG_FAAD),yes) OBJS+= faad.o -lfaad
# ifeq ($(CONFIG_FAAC),yes) OBJS+= faac.o -lfaac
# ifeq ($(CONFIG_XVID),yes) OBJS+= xvidff.o -lxvidcore
# ifeq ($(CONFIG_X264),yes) OBJS+= x264.o -lx264
# ifeq ($(CONFIG_MP3LAME),yes) OBJS += mp3lameaudio.o -lmp3lame
# ifeq ($(CONFIG_LIBOGG),yes)
# ifeq ($(CONFIG_LIBVORBIS),yes) OBJS += oggvorbis.o -lvorbisenc -lvorbis
# ifeq ($(CONFIG_LIBGSM),yes) OBJS += libgsm.o -lgsm

# i386 mmx specific stuff
# ifeq ($(TARGET_MMX),yes)
# OBJS += i386/fdct_mmx.o i386/cputest.o \
# 	i386/dsputil_mmx.o i386/mpegvideo_mmx.o \
# 	i386/idct_mmx.o i386/motion_est_mmx.o \
# 	i386/simple_idct_mmx.o i386/fft_sse.o i386/vp3dsp_mmx.o \
# 	i386/vp3dsp_sse2.o
# ifdef TARGET_BUILTIN_VECTOR
# i386/fft_sse.o: CFLAGS+= -msse
# depend: CFLAGS+= -msse
# endif

# ifeq ($(TARGET_ARCH_POWERPC),yes)
# OBJS += ppc/dsputil_ppc.o ppc/mpegvideo_ppc.o
# endif

# ifeq ($(TARGET_MMI),yes)
# OBJS += ps2/dsputil_mmi.o ps2/idct_mmi.o ps2/mpegvideo_mmi.o
# endif

# ifeq ($(TARGET_ALTIVEC),yes)
# OBJS += ppc/dsputil_altivec.o ppc/mpegvideo_altivec.o ppc/idct_altivec.o \
#         ppc/fft_altivec.o ppc/gmc_altivec.o ppc/fdct_altivec.o \
#         ppc/dsputil_h264_altivec.o
# endif

