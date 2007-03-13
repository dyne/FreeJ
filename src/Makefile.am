INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/lib/ccvt \
           -I$(top_srcdir)/lib/shout \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj \
	   -I$(top_srcdir)/lib/flash \
	   -I$(top_srcdir)/lib/goom \
           -I/opt/gp2x-crosscompile/include

#           -I$(top_srcdir)/lib/portaudio-v19/pa_common

AM_CXXFLAGS = @SDL_CFLAGS@ \
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
	plugin.cpp	\
	plugger.cpp	\
	jsparser.cpp	\
	callbacks_js.cpp \
	jutils.cpp	\
	shouter.cpp 	\
	pipe.cpp	\
\
	v4l_layer.cpp tvfreq.c	\
	v4l_layer_js.cpp \
	image_layer.cpp \
	image_layer_js.cpp \
	gen_layer.cpp	\
	gen_layer_js.cpp \
	geo_layer.cpp	\
	geo_layer_js.cpp \
	flash_layer.cpp \
	flash_layer_js.cpp \
	scroll_layer.cpp \
	scroll_layer_js.cpp \
        goom_layer.cpp \
	impl_layers.cpp \
\
	screen.cpp	\
	sdl_screen.cpp 	\
\
	kbd_ctrl.cpp	\
	joy_ctrl.cpp

freej_LDADD = -L/opt/gp2x-crosscompile/lib \
              $(top_srcdir)/lib/ccvt/libccvt.a \
	      $(top_srcdir)/lib/javascript/obj/libjs.a \
	      $(top_srcdir)/lib/flash/libflash.a \
	      $(top_srcdir)/lib/goom/libgoom2.a \
              -lm -lpng -ljpeg -lSDL_image -lSDL_gfx -lSDL
        #      /opt/gp2x-crosscompile/sysroot/lib/libpthread.so.0

