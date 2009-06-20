INCLUDES = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@

BUILT_SOURCES = freej_py.cpp

freej_py.cpp: $(top_srcdir)/bindings/freej.i
		@SWIG_COMMAND@ -python -threads -shadow -o freej_py.cpp @SWIG_IFACE@

pkgpython_PYTHON = __init__.py
nodist_pkgpython_PYTHON = freej.py
pkgpyexec_LTLIBRARIES = _freej.la
pkgpython_instdir = $(pythondir)
#pkgpython_inst_PYTHON = freej.pth


nodist__freej_la_SOURCES = freej_py.cpp
_freej_la_CXXFLAGS =  $(FREEJ_FLAGS) @PYTHON_CPPFLAGS@
_freej_la_LDFLAGS = -module -avoid-version
_freej_la_LIBADD = $(PY_LIBS) -L$(top_builddir)/src/.libs -lfreej

clean-local:
	rm -rf .libs _libs
	test -z "_freej.la" || rm -f _freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f freej_py.cpp freej_py.h freej.py
