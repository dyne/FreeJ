INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

lib_LTLIBRARIES = rotozoom.la
rotozoom_la_SOURCES = rotozoom.c
rotozoom_la_LDFLAGS = -module -avoid-version
