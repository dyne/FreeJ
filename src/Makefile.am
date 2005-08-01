INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/lib/ccvt \
           -I$(top_srcdir)/lib/theora-mmx \
           -I$(top_srcdir)/lib/avformat \
           -I$(top_srcdir)/lib/avcodec \
           -I$(top_srcdir)/lib/shout \
           -I$(top_srcdir)/lib/portaudio \
           -I$(top_srcdir)/lib/sdl_image \
           -I/usr/include/slang

#           -I$(top_srcdir)/lib/portaudio-v19/pa_common

AM_CXXFLAGS = @GLADE2_CFLAGS@ \
              @SDL_CFLAGS@ \
	      @FT2_CFLAGS@ @LUBRIFY_CFLAGS@ \
	      @JS_CFLAGS@ \
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

freej_LDADD = $(top_srcdir)/lib/ccvt/libccvt.a \
              $(top_srcdir)/lib/theora-mmx/libtheora.a \
		 @XIPH_LIBS@ \
              $(top_srcdir)/lib/avformat/libavformat.a \
              $(top_srcdir)/lib/avcodec/libavcodec.a \
              $(top_srcdir)/lib/shout/libshout.a \
              $(top_srcdir)/lib/portaudio/libportaudio.a \
              $(top_srcdir)/lib/sdl_image/libsdl_image.a \
              -ldl -lpthread -lm -lslang -lpng -ljpeg \
		 @STATIC_LIBS@ \
		 @OPENGL_LIB@ \
		 @SDL_LIBS@ \
		 @FT2_LIBS@ \
		 @GLADE2_LIBS@ \
		 @JS_LIBS@ 

#		 @PORTAUDIO_LIB@
#		 @AVIFILE_LIBS@
#		 @AVCODEC_LIBS@

bundle_contents = @PACKAGE@.app/Contents
@PACKAGE@_bundle: freej
	mkdir -p $(bundle_contents)/MacOS
	mkdir -p $(bundle_contents)/Resources
	echo "APPL????" > $(bundle_contents)/PkgInfo
	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/
