INCLUDES = @FREEJ_STATIC_INCLUDES@

FREEJ_FLAGS = @SDL_CFLAGS@ @FT2_CFLAGS@ @XIPH_CFLAGS@ @UNICAP_CFLAGS@ \
              @FFMPEG_CFLAGS@ @MOZJS_CFLAGS@ @SDLGFX_CFLAGS@ @OPENCV_CFLAGS@

# SWIG_SOURCES = freej.i
# SWIG_CFLAGS = -modern ${SWIG_ADD_CFLAGS}

DIST_SUBDIRS = java perl python ruby

SUBDIRS = @BINDING_DIRS@

EXTRA_DIST = freej.i   \
             javapre.i \
             luapost.i \
             perlpre.i \
             pypost.i  \
             pypre.i   \
             rbpost.i  \
             rbpre.i   \
             test.lua

