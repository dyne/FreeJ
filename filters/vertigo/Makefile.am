INCLUDES = -I$(top_srcdir)/src/include
plugindir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

plugin_LTLIBRARIES = vertigo.la
vertigo_la_SOURCES = vertigo.c
vertigo_la_LDFLAGS = -module -avoid-version -shared
