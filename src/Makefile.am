INCLUDES = -I$(top_srcdir)/src/include

SUBDIRS = include

SUFFIXES = .asm

NASM = nasm -f elf

.asm.o:
	$(NASM) $<

bin_PROGRAMS = freej

freej_SOURCES = \
	freej.cpp	\
	context.cpp	\
	layer.cpp	\
	v4l_layer.cpp	\
	avi_layer.cpp	\
	png_layer.cpp	\
	linklist.cpp	\
	jsync.cpp	\
	keyboard.cpp	\
	osd.cpp		\
	plugin.cpp	\
	plugger.cpp	\
	jutils.cpp	\
	tvfreq.c	\
	lubrify.c	\
	cpu_accel.c	\
	yuv2rgb.c yuv2rgb_mmx.c	\
	vline16.asm vline32.asm \
	hline16.asm hline32.asm \
	mmx_memcpy.asm 	\
	mmx_blit.asm	\
	mmx_blit_add.asm \
	mmx_blit_sub.asm \
	mmx_blit_and.asm \
	mmx_blit_or.asm \
	mmx_osd.asm \
	clearscr.asm

freej_LDADD = @SDL_LIBS@ @LIBS@ -lpthread -lm -ldl -lpng