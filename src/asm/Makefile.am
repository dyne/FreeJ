INCLUDES = -I$(top_srcdir)/src/include

SUFFIXES = .asm

NASM = nasm -f elf

.asm.o:
	$(NASM) $<

noinst_LIBRARIES = liblubrify.a

liblubrify_a_SOURCES = \
	lubrify.c	\
	mmx_absdiff16.asm \
	mmx_absdiff8.asm \
	mmx_memcpy.asm	\
	mmx_blit.asm	\
	vline.asm	\
	hline.asm	\
	clearscr.asm	\
	yuv2rgb.c	\
	yuv2rgb_mmx.c	\
	cpu_accel.c