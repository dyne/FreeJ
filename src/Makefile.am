INCLUDES = -I$(top_srcdir)/src/include -I$(top_srcdir)/ccvt
AM_CXXFLAGS = @PNG_CFLAGS@ @GLADE2_CFLAGS@ @SDL_CFLAGS@ @AVIFILE_CXXFLAGS@ @FT2_CFLAGS@

SUBDIRS = include

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	layer.cpp	\
	linklist.cpp	\
	jsync.cpp	\
	osd.cpp		\
	plugin.cpp	\
	plugger.cpp	\
	jutils.cpp	\
	v4l_layer.cpp	\
	avi_layer.cpp	\
	png_layer.cpp	\
	txt_layer.cpp	\
	impl_layers.cpp \
	sdl_screen.cpp \
	sdlxv_screen.cpp \
	keyboard.cpp	\
	gtk_ctrl.cpp	\
	tvfreq.c

freej_LDADD = $(top_srcdir)/ccvt/libccvt.a \
		 -ldl -lpthread -lm \
		@SDL_LIBS@ @AVIFILE_LIBS@ @FT2_LIBS@ @PNG_LIBS@ @GLADE2_LIBS@

bundle_contents = @PACKAGE@.app/Contents
@PACKAGE@_bundle: freej
	mkdir -p $(bundle_contents)/MacOS
	mkdir -p $(bundle_contents)/Resources
	echo "APPL????" > $(bundle_contents)/PkgInfo
	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/
