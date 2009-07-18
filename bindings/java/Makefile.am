AM_CPPFLAGS = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@ @OPENCV_CFLAGS@

# XXX(godog) what's missing? compilation and jar packaging?
swig_source = freej_java.cpp
swig_header = freej_java.h

BUILT_SOURCES = $(swig_source) $(swig_header)

$(swig_source): $(SWIG_IFACE)
	$(SWIG_COMMAND) -java -o $(swig_source) $(SWIG_IFACE)

pkgjavaexecdir = ${libdir}/jni
pkgjavaexec_LTLIBRARIES = libfreej_g.la
libfreej_g_la_CXXFLAGS = $(FREEJ_FLAGS) $(JAVA_CFLAGS)
libfreej_g_la_LDFLAGS = $(JAVA_LDFLAGS) 
nodist_libfreej_g_la_SOURCES = $(swig_source)

CLEANFILES = $(BUILT_SOURCES)

clean-local:
	rm -f *.class
	rm -f *.java
