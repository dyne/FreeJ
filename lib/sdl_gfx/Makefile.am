INCLUDES = -I$(top_srcdir)
CFLAGS = @SDL_CFLAGS@ @CFLAGS@

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libsdl_gfx.la

libsdl_gfx_la_SOURCES = \
	SDL_gfxPrimitives.c	\
	SDL_rotozoom.c	\
	SDL_framerate.c \
	SDL_imageFilter.c

noinst_HEADERS= \
	SDL_framerate.h \
	SDL_gfxPrimitives_font.h \
	SDL_gfxPrimitives.h \
	SDL_imageFilter.h \
	SDL_rotozoom.h
