# XXX(godog) what's missing? compilation and jar packaging?
swig_source = freej_java.cpp
swig_header = freej_java.h

BUILT_SOURCES = $(swig_source) $(swig_header)

$(swig_source): $(SWIG_IFACE)
	$(SWIG_COMMAND) -java -o $(swig_source) $(SWIG_IFACE)

pkgjavaexecdir = ${libdir}/jni
pkgjavaexec_LTLIBRARIES = libfreej_g.la

nodist_libfreej_g_la_SOURCES = $(swig_source)
libfreej_g_la_CXXFLAGS = $(FREEJ_CFLAGS) $(JAVA_CFLAGS)
libfreej_g_la_LDFLAGS = $(JAVA_LDFLAGS)
libfreej_g_la_LIBADD = $(top_builddir)/src/libfreej.la

CLEANFILES = $(BUILT_SOURCES)

clean-local:
	rm -f *.class
	rm -f *.java
