INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .nas

NASM = nasm -f elf
# -o `echo $<|cut -d. -f1`.lo

.nas.o:
	$(NASM) $<


lib_LTLIBRARIES = blurzoom.la
blurzoom_la_SOURCES = blurzoom.c blurzoomcore.nas
blurzoom_la_LDFLAGS = -module -avoid-version
