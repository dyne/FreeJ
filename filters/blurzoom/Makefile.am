INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<


lib_LTLIBRARIES = blurzoom.la
blurzoom_la_SOURCES = blurzoom.c blurzoomcore.asm
blurzoom_la_LDFLAGS = -module -avoid-version
