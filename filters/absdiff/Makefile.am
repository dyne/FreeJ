INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

lib_LTLIBRARIES = absdiff.la
absdiff_la_SOURCES = absdiff.c mmx_absdiff32.asm
absdiff_la_LDFLAGS = -module -avoid-version
