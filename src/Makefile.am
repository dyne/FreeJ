INCLUDES = -I$(top_srcdir)/src/include
AM_CXXFLAGS = @PNG_CFLAGS@ @GLADE2_CFLAGS@ @SDL_CFLAGS@ @AVIFILE_CXXFLAGS@ @FT2_CFLAGS@

SUBDIRS = include

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	layer.cpp	\
	v4l_layer.cpp	\
	avi_layer.cpp	\
	png_layer.cpp	\
	txt_layer.cpp	\
	impl_layers.cpp \
	linklist.cpp	\
	jsync.cpp	\
	keyboard.cpp	\
	gtk_ctrl.cpp	\
	osd.cpp		\
	plugin.cpp	\
	plugger.cpp	\
	jutils.cpp	\
	tvfreq.c	\
	cpu_accel.c	\
	yuv2rgb.c yuv2rgb_mmx.c

freej_LDADD = -ldl -lpthread -lm @SDL_LIBS@ @AVIFILE_LIBS@ @FT2_LIBS@ @PNG_LIBS@ @GLADE2_LIBS@

bundle_contents = @PACKAGE@.app/Contents
@PACKAGE@_bundle: freej
	mkdir -p $(bundle_contents)/MacOS
	mkdir -p $(bundle_contents)/Resources
	echo "APPL????" > $(bundle_contents)/PkgInfo
	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/
