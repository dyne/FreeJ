INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

lib_LTLIBRARIES = delaygrab.la
delaygrab_la_SOURCES = delaygrab.c mmx_memcpy.asm
delaygrab_la_LDFLAGS = -module -avoid-version
