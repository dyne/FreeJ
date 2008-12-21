INCLUDES = -I$(top_srcdir)/src/include \
	   -I$(top_srcdir)/lib \
	   -I$(top_srcdir)/lib/lo \
	   -I$(top_srcdir)/lib/cwiid \
           -I$(top_srcdir)/lib/shout \
	   -I$(top_srcdir)/lib/sdl_ttf \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj \
           -I/usr/include/slang


FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ @FFMPEG_CFLAGS@

BUILT_SOURCES = freej_py.cpp

freej_py.cpp: $(top_srcdir)/bindings/freej.i
		swig -I$(top_srcdir)/src/include/ -python -threads -c++ -shadow -O \
	     	-o freej_py.cpp $(top_srcdir)/bindings/freej.i

pkgpython_PYTHON = freej.py
pkgpyexec_LTLIBRARIES = _freej.la
pkgpython_instdir = $(pythondir)
pkgpython_inst_PYTHON = freej.pth


_freej_la_SOURCES = freej_py.cpp
_freej_la_CXXFLAGS =  $(FREEJ_FLAGS) @PYTHON_CPPFLAGS@
_freej_la_LDFLAGS = -module -avoid-version
_freej_la_LIBADD = $(PY_LIBS) -L$(top_srcdir)/src/.libs -lfreej

clean:
	rm -rf .libs _libs
	test -z "_freej.la" || rm -f _freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f freej_py.cpp freej.py
