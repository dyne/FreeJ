INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/lib \
           -I$(top_srcdir)/lib/ccvt \
           -I$(top_srcdir)/lib/shout \
           -I$(top_srcdir)/lib/lo \
           -I$(top_srcdir)/lib/sdl_image \
           -I$(top_srcdir)/lib/sdl_gfx \
	   -I$(top_srcdir)/lib/sdl_ttf \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj \
	   -I$(top_srcdir)/lib/flash \
	   -I$(top_srcdir)/lib/goom \
	   -I$(top_srcdir)/lib/cwiid \
           -I/usr/include/slang

#-freg-struct-return is to compile an host compatible with freeframe dso
AM_CXXFLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ \
	      -freg-struct-return \
              -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

SUBDIRS = include

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	context_js.cpp  \
	layer.cpp	\
	layer_js.cpp    \
	blitter.cpp	\
	iterator.cpp	\
	linklist.cpp	\
	jsync.cpp	\
	osd.cpp		\
	console.cpp	\
	cpu_accel.cpp	\
	plugin.cpp	\
	plugger.cpp	\
	frei0r.cpp	\
	freeframe.cpp   \
	filter.cpp	\
	filter_js.cpp   \
	jsparser.cpp	\
	callbacks_js.cpp \
	jutils.cpp	\
	fastmemcpy.cpp  \
	ringbuffer.cpp  \
	convertvid.cpp  \
\
	v4l_layer.cpp tvfreq.c	\
	v4l_layer_js.cpp \
	video_layer.cpp	\
	video_layer_js.cpp \
	image_layer.cpp \
	image_layer_js.cpp \
	avi_layer.cpp	\
	text_layer.cpp \
	text_layer_js.cpp \
	gen_f0r_layer.cpp \
	geo_layer.cpp	\
	geo_layer_js.cpp \
	flash_layer.cpp \
	flash_layer_js.cpp \
	scroll_layer.cpp \
	scroll_layer_js.cpp \
	xhacks_layer.cpp \
        goom_layer.cpp \
	goom_layer_js.cpp \
	impl_layers.cpp \
\
	screen.cpp	\
	sdl_screen.cpp 	\
	sdlgl_screen.cpp \
	gl_screen.cpp   \
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
	audio_collector_js.cpp \
\
	video_encoder.cpp \
	video_encoder_js.cpp \
	ffmpeg_encoder.cpp \
	theorautils.cpp \
	oggtheora_encoder.cpp \
	impl_video_encoders.cpp \
	xgrab_layer.cpp

noinst_LIBRARIES = libfreej.a
libfreej_a_SOURCES = $(freej_SOURCES)

freej_LDADD = $(top_srcdir)/lib/ccvt/libccvt.a \
              $(top_srcdir)/lib/sdl_image/libsdl_image.a \
              $(top_srcdir)/lib/sdl_gfx/libsdl_gfx.a \
	      $(top_srcdir)/lib/sdl_ttf/libsdl_ttf.a \
	      $(top_srcdir)/lib/javascript/obj/libjs.a \
	      $(top_srcdir)/lib/flash/libflash.a \
	      $(top_srcdir)/lib/goom/libgoom2.a \
              $(top_srcdir)/lib/shout/libshout.a \
              $(top_srcdir)/lib/lo/liblo.a \
              $(top_srcdir)/lib/cwiid/libwiimote.a \
              -ldl -lpthread -lm -lslang -lpng -ljpeg -lrt \
	         @X11_LIBS@ \
		 @XIPH_LIBS@ \
		 @STATIC_LIBS@ \
		 @OPENGL_LIB@ \
		 @SDL_LIBS@ \
		 @FLASH_LIBS@ \
		 @FT2_LIBS@ \
                 @ALSA_LIBS@ \
		 @JACK_LIBS@ \
                 @FFTW_LIBS@ \
		 @FFMPEG_LIBS@ \
		 @X11_LIBS@ \
		 @BLUEZ_LIBS@ \
		 @OPENGL_LIB@

