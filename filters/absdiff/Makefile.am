INCLUDES = -I$(top_srcdir)/src/include
plugindir = ${prefix}/lib/freej


SUFFIXES = .nas

# fix to compile nasm on libtool (goddamn autoconf/make/libtool crap!)
NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.nas.lo:
	$(NASM) $<

plugin_LTLIBRARIES = absdiff.la
absdiff_la_SOURCES = absdiff.c mmx_absdiff32.nas
absdiff_la_LDFLAGS = -module -avoid-version -shared
