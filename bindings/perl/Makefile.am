INCLUDES = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@

PERL_SOURCE = freej_perl.cpp

BUILT_SOURCES = $(PERL_SOURCE)

perl_archdir = @PERL_ARCHLIB@

freej_perl.cpp: $(top_srcdir)/bindings/freej.i
	@SWIG_COMMAND@ -perl -o $(PERL_SOURCE) @SWIG_IFACE@

pkgperl_PERL = freej.pm

pkgperlexecdir = $(perl_archdir)/auto/Freej
pkgperlexec_LTLIBRARIES = Freej.la
Freej_la_LDFLAGS = -module -avoid-version
Freej_la_CXXFLAGS = -I$(perl_archdir)/CORE $(FREEJ_FLAGS) @PERL_CFLAGS@
nodist_Freej_la_SOURCES = $(PERL_SOURCE)
Freej_la_LIBADD = -L$(top_builddir)/src/.libs -lfreej

clean-local:
	rm -rf .libs _libs
	test -z "Freej.la" || rm -f Freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f freej_perl.cpp freej.pm 
