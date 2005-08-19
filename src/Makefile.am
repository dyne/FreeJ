INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/lib/ccvt \
           -I$(top_srcdir)/lib/theora-mmx \
           -I$(top_srcdir)/lib/avformat \
           -I$(top_srcdir)/lib/avcodec \
           -I$(top_srcdir)/lib/shout \
           -I$(top_srcdir)/lib/portaudio \
           -I$(top_srcdir)/lib/sdl_image \
           -I$(top_srcdir)/lib/sdl_gfx \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj \
	   -I$(top_srcdir)/lib/flash \
           -I/usr/include/slang

#           -I$(top_srcdir)/lib/portaudio-v19/pa_common

AM_CXXFLAGS = @SDL_CFLAGS@ \
	      @FT2_CFLAGS@ @LUBRIFY_CFLAGS@ \
              -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

SUBDIRS = include

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	blitter.cpp	\
	iterator.cpp	\
	linklist.cpp	\
	jsync.cpp	\
	osd.cpp		\
	console.cpp	\
	cpu_accel.cpp	\
	plugin.cpp	\
	plugger.cpp	\
	jsparser.cpp	\
	jutils.cpp	\
	fastmemcpy.cpp  \
	shouter.cpp 	\
	pipe.cpp	\
\
	layer.cpp	\
	v4l_layer.cpp tvfreq.c	\
	video_layer.cpp	\
	image_layer.cpp \
	avi_layer.cpp	\
	txt_layer.cpp	\
	gen_layer.cpp	\
	geo_layer.cpp	\
	flash_layer.cpp \
	scroll_layer.cpp \
	xhacks_layer.cpp \
	impl_layers.cpp \
\
	screen.cpp	\
	sdl_screen.cpp 	\
	sdlgl_screen.cpp \
\
	kbd_ctrl.cpp	\
	joy_ctrl.cpp	\
\
	video_encoder.cpp \
	ffmpeg_encoder.cpp \
	oggtheora_encoder.cpp \
	impl_video_encoders.cpp

freej_LDADD = $(top_srcdir)/lib/ccvt/libccvt.a \
              $(top_srcdir)/lib/theora-mmx/libtheora.a \
		 @XIPH_LIBS@ \
              $(top_srcdir)/lib/avformat/libavformat.a \
              $(top_srcdir)/lib/avcodec/libavcodec.a \
              $(top_srcdir)/lib/shout/libshout.a \
              $(top_srcdir)/lib/portaudio/libportaudio.a \
		 @PORTAUDIO_LIBS@ \
              $(top_srcdir)/lib/sdl_image/libsdl_image.a \
              $(top_srcdir)/lib/sdl_gfx/libsdl_gfx.a \
	      $(top_srcdir)/lib/javascript/obj/libjs.a \
	      $(top_srcdir)/lib/flash/libflash.a \
              -ldl -lpthread -lm -lslang -lpng -ljpeg \
		 @STATIC_LIBS@ \
		 @OPENGL_LIB@ \
		 @SDL_LIBS@ \
		 @FT2_LIBS@

