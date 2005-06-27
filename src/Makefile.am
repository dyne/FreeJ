INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/ccvt \
           -I$(top_srcdir)/lib/portaudio-v19/pa_common \
           -I$(top_srcdir)/libflash \
           -I$(top_srcdir)/lib/theora-mmx/include/theora \
           -I/usr/include/slang

AM_CXXFLAGS = @PNG_CFLAGS@ @GLADE2_CFLAGS@ \
              @SDL_CFLAGS@ @AVIFILE_CXXFLAGS@ \
	      @FT2_CFLAGS@ @LUBRIFY_CFLAGS@ \
	      @AVCODEC_CFLAGS@ @JS_CFLAGS@ \
	      @XIPH_CFLAGS@ \
              -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

SUBDIRS = include

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	layer.cpp	\
	blitter.cpp	\
	iterator.cpp	\
	linklist.cpp	\
	jsync.cpp	\
	osd.cpp		\
	console.cpp	\
	plugin.cpp	\
	plugger.cpp	\
	jutils.cpp	\
	v4l_layer.cpp	\
	video_layer.cpp	\
	avi_layer.cpp	\
	png_layer.cpp	\
	txt_layer.cpp	\
	gen_layer.cpp	\
	flash_layer.cpp \
	scroll_layer.cpp \
	impl_layers.cpp \
	screen.cpp	\
	sdl_screen.cpp 	\
	sdlgl_screen.cpp \
	imagefilter.cpp \
	rotozoomer.cpp  \
	kbd_ctrl.cpp	\
	gtk_ctrl.cpp	\
	joy_ctrl.cpp	\
	tvfreq.c	\
	xhacks_layer.cpp \
	jsparser.cpp \
	cpu_accel.cpp \
	ffmpeg_encoder.cpp \
	oggtheora_encoder.cpp \
	video_encoder.cpp \
	impl_video_encoders.cpp \
	image_layer.cpp \
	fastmemcpy.cpp \
	shouter.cpp \
	pipe.cpp

freej_LDADD = $(top_srcdir)/ccvt/libccvt.a \
	      $(top_srcdir)/lib/portaudio-v19/lib/libportaudio.a \
	      $(top_srcdir)/lib/js/src/Linux_All_DBG.OBJ/libjs.a \
	      $(top_srcdir)/lib/ffmpeg/libavformat/libavformat.a \
	      $(top_srcdir)/lib/ffmpeg/libavcodec/libavcodec.a \
	      $(top_srcdir)/lib/theora-mmx/lib/.libs/libtheora.a \
		 -ldl -lpthread -lm -lslang \
		 -lGL -lGLU  \
		@PORTAUDIO_LIB@ \
		@SDL_LIBS@ \
		@AVIFILE_LIBS@ \
		@FT2_LIBS@ \
		@PNG_LIBS@ \
		@GLADE2_LIBS@ \
		@AVCODEC_LIBS@ \
		@JS_LIBS@ \
		@SDLIMAGE_LIBS@ \
		@FLASH_LIBS@ \
		@XIPH_LIBS@

bundle_contents = @PACKAGE@.app/Contents
@PACKAGE@_bundle: freej
	mkdir -p $(bundle_contents)/MacOS
	mkdir -p $(bundle_contents)/Resources
	echo "APPL????" > $(bundle_contents)/PkgInfo
	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/
