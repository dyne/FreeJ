AM_CPPFLAGS = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@

swig_source = freej_py.cpp
python_module = freej.py
python_header = freej_py.h

BUILT_SOURCES = $(swig_source)

$(swig_source): $(SWIG_IFACE)
		$(SWIG_COMMAND) $(SWIG_PYTHON_OPT) -threads -shadow -o $(swig_source) $(SWIG_IFACE)

pkgpython_PYTHON = __init__.py
nodist_pkgpython_PYTHON = $(python_module)
pkgpyexec_LTLIBRARIES = _freej.la
pkgpython_instdir = $(pythondir)
#pkgpython_inst_PYTHON = freej.pth

nodist__freej_la_SOURCES = $(swig_source)
_freej_la_CXXFLAGS =  $(FREEJ_FLAGS) $(SWIG_PYTHON_CPPFLAGS)
_freej_la_LDFLAGS = -module -avoid-version
_freej_la_LIBADD = -L$(top_builddir)/src/.libs -lfreej

CLEANFILES = $(swig_source) $(python_module) $(python_header)
