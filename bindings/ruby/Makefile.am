AM_CPPFLAGS = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@ @OPENCV_CFLAGS@

swig_source = freej_ruby.cpp
swig_header = freej_ruby.h

BUILT_SOURCES = $(swig_source) $(swig_header)

$(swig_source): $(SWIG_IFACE) $(top_srcdir)/bindings/rbpre.i $(top_srcdir)/bindings/rbpost.i
	$(SWIG_COMMAND) -ruby -initname Freej -o $(swig_source) $(SWIG_IFACE)

pkgrubydir = $(RUBY_SITE_LIB)
dist_pkgruby_DATA = freej_extensions.rb

pkgrubyarchdir = $(RUBY_SITE_PKG)
pkgrubyarch_LTLIBRARIES = Freej.la

nodist_Freej_la_SOURCES = $(swig_source)
Freej_la_CXXFLAGS =  $(FREEJ_FLAGS) $(RUBY_CPPFLAGS)
Freej_la_LDFLAGS = $(RUBY_LDFLAGS) -module -avoid-version
Freej_la_LIBADD = $(RUBY_EXTRA_LIBS) -L$(top_builddir)/src/.libs -lfreej

EXTRA_DIST = list_classes.rb list_classes.sh

CLEANFILES = $(BUILT_SOURCES)
