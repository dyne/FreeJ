INCLUDES = -I$(top_srcdir)/src/include           \
           -I$(top_srcdir)/lib                   \
           -I$(top_srcdir)/lib/ccvt              \
           -I$(top_srcdir)/lib/shout             \
           -I$(top_srcdir)/lib/lo                \
           -I$(top_srcdir)/lib/sdl_image         \
           -I$(top_srcdir)/lib/sdl_gfx           \
	   -I$(top_srcdir)/lib/sdl_ttf           \
	   -I$(top_srcdir)/lib/javascript        \
	   -I$(top_srcdir)/lib/javascript/obj    \
	   -I$(top_srcdir)/lib/flash             \
	   -I$(top_srcdir)/lib/goom              \
	   -I$(top_srcdir)/lib/cwiid             \
           -I/usr/include/slang

#-freg-struct-return is to compile an host compatible with freeframe dso
AM_CXXFLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @FC_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ @FFMPEG_CFLAGS@ \
	      -freg-struct-return \
              -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

SUBDIRS = include


SOURCES = \
	context.cpp	\
	layer.cpp	\
	fps.cpp		\
	blitter.cpp	\
	sdl_blits.cpp   \
	linear_blits.cpp \
	iterator.cpp	\
	linklist.cpp	\
	jsync.cpp	\
	closure.cpp     \
	console.cpp	\
	cpu_accel.cpp	\
	plugin.cpp	\
	plugger.cpp	\
	frei0r.cpp	\
	freeframe.cpp   \
	filter.cpp	\
	parameter.cpp   \
	jutils.cpp	\
	fastmemcpy.cpp  \
	ringbuffer.cpp  \
	convertvid.cpp  \
\
	v4l_layer.cpp tvfreq.c	\
	unicap_layer.cpp \
	video_layer.cpp	\
	image_layer.cpp \
	text_layer.cpp \
	gen_f0r_layer.cpp \
	geo_layer.cpp	\
	flash_layer.cpp \
	scroll_layer.cpp \
	xhacks_layer.cpp \
        goom_layer.cpp \
	impl_layers.cpp \
\
	screen.cpp	\
	sdl_screen.cpp 	\
	sdlgl_screen.cpp \
	gl_screen.cpp   \
	soft_screen.cpp \
\
	controller.cpp  \
	kbd_ctrl.cpp	\
	mouse_ctrl.cpp	\
	vimo_ctrl.cpp	\
	joy_ctrl.cpp	\
	midi_ctrl.cpp   \
	trigger_ctrl.cpp \
	osc_ctrl.cpp \
	wiimote_ctrl.cpp \
\
	audio_jack.cpp  \
	audio_collector.cpp \
\
	video_encoder.cpp \
	ffmpeg_encoder.cpp \
	theorautils.cpp \
	oggtheora_encoder.cpp \
	impl_video_encoders.cpp \
	xgrab_layer.cpp

JS_SOURCES = 	context_js.cpp  \
		layer_js.cpp    \
		filter_js.cpp   \
		jsparser.cpp	\
		callbacks_js.cpp \
		video_encoder_js.cpp \
		v4l_layer_js.cpp \
		unicap_layer_js.cpp \
		video_layer_js.cpp \
		image_layer_js.cpp \
		text_layer_js.cpp \
		geo_layer_js.cpp \
		flash_layer_js.cpp \
		scroll_layer_js.cpp \
		goom_layer_js.cpp \
		audio_collector_js.cpp


static_libs = $(top_srcdir)/lib/ccvt/libccvt.la \
              $(top_srcdir)/lib/sdl_image/libsdl_image.la \
              $(top_srcdir)/lib/sdl_gfx/libsdl_gfx.la \
	      $(top_srcdir)/lib/sdl_ttf/libsdl_ttf.la \
              $(top_srcdir)/lib/shout/libshout.la \
              $(top_srcdir)/lib/lo/liblo.la \
              $(top_srcdir)/lib/javascript/obj/libjs.a

shared_libs = -ldl -lpthread -lm -lslang -lpng -ljpeg -lrt \
	         @X11_LIBS@     \
		 @XIPH_LIBS@    \
		 @STATIC_LIBS@  \
		 @OPENGL_LIB@   \
		 @SDL_LIBS@     \
		 @FLASH_LIBS@   \
		 @FT2_LIBS@     \
		 @FC_LIBS@      \
                 @ALSA_LIBS@    \
		 @JACK_LIBS@    \
		 @GOOM_LIBS@    \
                 @FFTW_LIBS@    \
		 @FFMPEG_LIBS@  \
		 @SWSCALE_LIBS@ \
		 @X11_LIBS@     \
		 @BLUEZ_LIBS@   \
		 @CWIID_LIBS@   \
		 @OPENGL_LIB@   \
		 @UNICAP_LIBS@


lib_LTLIBRARIES = libfreej.la
libfreej_la_SOURCES = $(SOURCES) $(JS_SOURCES)
libfreej_la_LIBADD  = $(static_libs) $(shared_libs)
libfreej_la_LDFLAGS = -version-info @FREEJ_VERSION_INFO@

bin_PROGRAMS = freej
freej_SOURCES = freej.cpp
freej_LDADD = -L$(top_srcdir)/src/.libs -lfreej


