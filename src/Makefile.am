INCLUDES = -I$(top_srcdir)/src/include -I$(top_srcdir)/ccvt
AM_CXXFLAGS = @PNG_CFLAGS@ @GLADE2_CFLAGS@ \
              @SDL_CFLAGS@ @AVIFILE_CXXFLAGS@ \
	      @FT2_CFLAGS@ @LUBRIFY_CFLAGS@ \
	      @AVCODEC_CFLAGS@ \
              -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

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
	video_layer.cpp	\
	avi_layer.cpp	\
	png_layer.cpp	\
	txt_layer.cpp	\
	gen_layer.cpp	\
	impl_layers.cpp \
	screen.cpp	\
	sdl_screen.cpp 	\
	sdl_blit.cpp 	\
	kbd_ctrl.cpp	\
	gtk_ctrl.cpp	\
	joy_ctrl.cpp	\
	tvfreq.c	\
	xhacks_layer.cpp 

freej_LDADD = $(top_srcdir)/ccvt/libccvt.a \
		 -ldl -lpthread -lm \
		@SDL_LIBS@ @AVIFILE_LIBS@ @FT2_LIBS@ @PNG_LIBS@ @GLADE2_LIBS@ @AVCODEC_LIBS@

bundle_contents = @PACKAGE@.app/Contents
@PACKAGE@_bundle: freej
	mkdir -p $(bundle_contents)/MacOS
	mkdir -p $(bundle_contents)/Resources
	echo "APPL????" > $(bundle_contents)/PkgInfo
	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/
