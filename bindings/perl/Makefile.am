INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/lib/shout \
	   -I$(top_srcdir)/lib/sdl_ttf \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ @FFMPEG_CFLAGS@

PERL_SOURCE = freej_perl.cpp

BUILT_SOURCES = $(PERL_SOURCE)

perl_archdir = @PERL_ARCHLIB@

freej_perl.cpp: $(top_srcdir)/bindings/freej.i
	swig -I$(top_srcdir)/src/include -perl -c++ -O \
		-o $(PERL_SOURCE) $(top_srcdir)/bindings/freej.i

pkgperl_PERL = freej.pm

pkgperlexecdir = $(perl_archdir)/auto/Freej
pkgperlexec_LTLIBRARIES = Freej.la
Freej_la_LDFLAGS = -module -avoid-version
Freej_la_CXXFLAGS = -I$(perl_archdir)/CORE $(FREEJ_FLAGS) @PERL_CFLAGS@
Freej_la_SOURCES = $(PERL_SOURCE)
Freej_la_LIBADD = -L$(top_srcdir)/src/.libs -lfreej

clean:
	rm -rf .libs _libs
	test -z "Freej.la" || rm -f Freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f freej_perl.cpp freej.pm 
