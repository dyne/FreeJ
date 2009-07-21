swig_source = freej_py.cpp
swig_header = freej_py.h

python_module = freej.py

BUILT_SOURCES = $(swig_source) $(swig_header)

$(swig_source): $(SWIG_IFACE)
		$(SWIG_COMMAND) $(SWIG_PYTHON_OPT) -threads -shadow -o $(swig_source) $(SWIG_IFACE)

pkgpython_PYTHON = __init__.py
nodist_pkgpython_PYTHON = $(python_module)
pkgpyexec_LTLIBRARIES = _freej.la
#pkgpython_instdir = $(pythondir)
#pkgpython_inst_PYTHON = freej.pth

nodist__freej_la_SOURCES = $(swig_source)
_freej_la_CXXFLAGS = $(FREEJ_CFLAGS) $(SWIG_PYTHON_CPPFLAGS)
_freej_la_LDFLAGS = -module -avoid-version
_freej_la_LIBADD = $(top_builddir)/src/libfreej.la

CLEANFILES = $(BUILT_SOURCES) $(python_module)
