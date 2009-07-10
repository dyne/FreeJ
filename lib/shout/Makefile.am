INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/theora-mmx

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libshout.la

libshout_la_SOURCES = \
	shout.c util.c ogg.c vorbis.c mp3.c theora.c \
	avl/avl.c httpp/httpp.c net/resolver.c net/sock.c timing/timing.c

noinst_HEADERS = shout_ogg.h shout_private.h util.h

# XXX(godog) this should be SUBDIRS
EXTRA_DIST = avl httpp shout net timing httpp
