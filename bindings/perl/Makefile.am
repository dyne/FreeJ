swig_source = freej_perl.cpp
perl_module = freej.pm

BUILT_SOURCES = $(swig_source) $(perl_module)

$(swig_source): $(SWIG_IFACE)
	$(SWIG_COMMAND) -perl -o $(swig_source) $(SWIG_IFACE)

perllibdir = $(PERL_INSTALLLIB)
perllib_DATA = $(perl_module)

# XXX(godog) what's /auto/ for?
# XXX(godog) shouldn't Freej be lowercased to freej?
perlexecdir = $(PERL_INSTALLARCH)/auto/Freej
perlexec_LTLIBRARIES = Freej.la

# XXX(godog) is Freej.so correct? in freej_perl.pm it is freejc
nodist_Freej_la_SOURCES = $(swig_source)
Freej_la_CXXFLAGS = $(FREEJ_CFLAGS) $(PERL_CFLAGS)
Freej_la_LDFLAGS = -module -avoid-version
Freej_la_LIBADD = $(top_builddir)/src/libfreej.la

CLEANFILES = $(BUILT_SOURCES)
