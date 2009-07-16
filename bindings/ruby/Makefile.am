INCLUDES = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@ @OPENCV_CFLAGS@

BUILT_SOURCES = freej_ruby.cpp

freej_ruby.cpp: $(top_srcdir)/bindings/freej.i $(top_srcdir)/bindings/rbpre.i $(top_srcdir)/bindings/rbpost.i
	@SWIG_COMMAND@ -ruby -initname Freej -o freej_ruby.cpp @SWIG_IFACE@

pkgrubydir = $(exec_prefix)/@RUBY_INSTDIR@
dist_pkgruby_DATA = freej_extensions.rb

pkgrubyarchdir = $(exec_prefix)/@RUBY_ARCHINSTDIR@
pkgrubyarch_LTLIBRARIES = Freej.la

nodist_Freej_la_SOURCES = freej_ruby.cpp
Freej_la_CXXFLAGS =  $(FREEJ_FLAGS) @RUBY_CFLAGS@
Freej_la_LDFLAGS = $(RUBY_LDFLAGS) -module -avoid-version
Freej_la_LIBADD = $(RUBY_LIBS) -L$(top_builddir)/src/.libs -lfreej

EXTRA_DIST = list_classes.rb list_classes.sh

clean-local:
	rm -rf .libs _libs
	test -z "Freej.la" || rm -f Freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f freej_ruby.*
