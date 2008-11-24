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

plugindir = ${prefix}/lib/freej

freej_py.cpp:
	swig -I../src/include/ -python -threads -c++ -shadow -O -o freej_py.cpp freej.i

plugin_LTLIBRARIES = libfreejpy.la

libfreejpy_la_CXXFLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ @FFMPEG_CFLAGS@ @PYTHON_CPPFLAGS@

libfreejpy_la_LDFLAGS = -shared

libfreejpy_la_SOURCES = freej_py.cpp


libfreejpy_la_LIBADD = $(top_srcdir)/src/libfreej.a \
              $(top_srcdir)/lib/ccvt/libccvt.a \
              $(top_srcdir)/lib/sdl_image/libsdl_image.a \
              $(top_srcdir)/lib/sdl_gfx/libsdl_gfx.a \
	      $(top_srcdir)/lib/sdl_ttf/libsdl_ttf.a \
	      $(top_srcdir)/lib/javascript/obj/libjs.a \
	      $(top_srcdir)/lib/flash/libflash.a \
              $(top_srcdir)/lib/shout/libshout.a \
              $(top_srcdir)/lib/lo/liblo.a \
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
		 @GOOM_LIBS@ \
                 @FFTW_LIBS@ \
		 @FFMPEG_LIBS@ \
		 @SWSCALE_LIBS@ \
		 @X11_LIBS@ \
		 @BLUEZ_LIBS@ \
		 @CWIID_LIBS@ \
		 @OPENGL_LIB@ \
		 @UNICAP_LIBS@

