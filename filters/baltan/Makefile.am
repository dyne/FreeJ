INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

lib_LTLIBRARIES = baltan.la
baltan_la_SOURCES = baltan.c
baltan_la_LDFLAGS = -module -avoid-version
