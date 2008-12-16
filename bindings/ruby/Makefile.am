INCLUDES = -I$(top_srcdir)/src/include \
	   -I$(top_srcdir)/lib \
	   -I$(top_srcdir)/lib/lo \
	   -I$(top_srcdir)/lib/cwiid \
           -I$(top_srcdir)/lib/shout \
	   -I$(top_srcdir)/lib/sdl_ttf \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj 



FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ @FFMPEG_CFLAGS@

BUILT_SOURCES = freej_ruby.cpp

freej_ruby.cpp: $(top_srcdir)/bindings/freej.i
	swig -I$(top_srcdir)/src/include/ -ruby -initname Freej -c++ -O \
	     -o freej_ruby.cpp $(top_srcdir)/bindings/freej.i

pkgrubyexecdir = @RUBY_ARCHDIR@
pkgrubyexec_LTLIBRARIES = Freej.la

Freej_la_SOURCES = freej_ruby.cpp
Freej_la_CXXFLAGS =  $(FREEJ_FLAGS) @RUBY_CFLAGS@
Freej_la_LDFLAGS = -module -avoid-version
Freej_la_LIBADD = $(RUBY_LIBS) -L$(top_srcdir)/src/.libs -lfreej


clean:
	rm -rf .libs _libs
	test -z "Freej.la" || rm -f Freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f freej_ruby.*
