INCLUDES = -I$(top_srcdir)/src/include
plugindir = ${prefix}/lib/freej

#SUFFIXES = .asm

#NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

#.asm.lo:
#	$(NASM) $<

plugin_LTLIBRARIES = delaygrab.la
delaygrab_la_SOURCES = delaygrab.c
delaygrab_la_LDFLAGS = -module -avoid-version -shared
