INCLUDES = -I$(top_srcdir)/src/include \
           -I$(top_srcdir)/lib/shout \
	   -I$(top_srcdir)/lib/sdl_ttf \
	   -I$(top_srcdir)/lib/javascript \
	   -I$(top_srcdir)/lib/javascript/obj


plugindir = ${prefix}/lib/freej

BUILT_SOURCES = freej_py.cpp

# SWIG_SOURCES = freej.i
# SWIG_CFLAGS = -modern ${SWIG_ADD_CFLAGS}

pkgpython_PYTHON = $(top_srcdir)/py/freej.py
pkgpyexec_LTLIBRARIES = _freej.la
pkgpython_instdir = $(pythondir)
pkgpython_inst_PYTHON = freej.pth


freej_py.cpp:
	swig -I../src/include/ -python -threads -c++ -shadow -O -o freej_py.cpp freej.i

# lib_LTLIBRARIES = libfreej_py.la
_freej_la_SOURCES = freej_py.cpp
_freej_la_CXXFLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ @FFMPEG_CFLAGS@ @PYTHON_CPPFLAGS@
_freej_la_LDFLAGS = -module -avoid-version
_freej_la_LIBADD = $(PY_LIBS) -L$(top_srcdir)/src/.libs -lfreej

#$(top_srcdir)/py/freej_py.cpp : $(SWIG_SOURCES)
#	$(SWIG) $(SWIG_PYTHON_OPT) $(SWIG_CFLAGS) -I$(top_srcdir)/py -o $@ $<

clean:
	rm -rf .libs _libs
	rm -f freej_py.cpp freej.py*
	test -z "_freej.la" || rm -f _freej.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
