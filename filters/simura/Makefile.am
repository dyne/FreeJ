INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

lib_LTLIBRARIES = simura.la
simura_la_SOURCES = simura.c
simura_la_LDFLAGS = -module -avoid-version
