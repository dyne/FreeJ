
INCLUDES = -I$(top_srcdir)/src/include

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	layer.cpp	\
	v4l.cpp		\
	linklist.cpp	\
	jutils.cpp	\
	jsync.cpp	\
	keyboard.cpp	\
	osd.cpp

SUBDIRS = \
	filters	\
	asm

freej_LDADD = @SDL_LIBS@ @EFENCE_LIBS@ -lpthread -lm filters/libfilters.a asm/liblubrify.a
