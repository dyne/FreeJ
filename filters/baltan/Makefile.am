INCLUDES = -I$(top_srcdir)/src/include
plugindir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

plugin_LTLIBRARIES = baltan.la
baltan_la_SOURCES = baltan.c
baltan_la_LDFLAGS = -module -avoid-version -shared
