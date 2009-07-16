INCLUDES = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@ @OPENCV_CFLAGS@

BUILT_SOURCES = freej_java.cpp

freej_java.cpp: $(top_srcdir)/bindings/freej.i
	@SWIG_COMMAND@ -java -o freej_java.cpp @SWIG_IFACE@

pkgjavaexecdir = /usr/lib/jni
pkgjavaexec_LTLIBRARIES = libfreej_g.la
libfreej_g_la_CXXFLAGS = $(FREEJ_FLAGS) @JAVA_CFLAGS@
libfreej_g_la_LDFLAGS = $(JAVA_LDFLAGS) 
nodist_libfreej_g_la_SOURCES = freej_java.cpp


clean-local:
	rm -rf .libs _libs
	test -z "libfreej_g.la" || rm -f libfreej_g.la
	rm -f "./so_locations"
	rm -f *.o
	rm -f *.lo
	rm -f *.class
	rm -f *.java
	rm -f $(BUILT_SOURCES)


