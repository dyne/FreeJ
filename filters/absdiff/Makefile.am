INCLUDES = -I$(top_srcdir)/src/include
libdir = ${prefix}/lib/freej

SUFFIXES = .nas

NASM = nasm -f elf
# fix to compile with nasm and older libtool, no more needed:  -o `echo $<|cut -d. -f1`.lo

.nas.o:
	$(NASM) $<

lib_LTLIBRARIES = absdiff.la
absdiff_la_SOURCES = absdiff.c mmx_absdiff32.nas
absdiff_la_LDFLAGS = -module -avoid-version
