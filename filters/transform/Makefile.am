INCLUDES = -I$(top_srcdir)/src/include
plugindir = ${prefix}/lib/freej

SUFFIXES = .asm

NASM = nasm -f elf -o `echo $<|cut -d. -f1`.lo

.asm.lo:
	$(NASM) $<

plugin_LTLIBRARIES = squaretrans.la randtrans.la quadtrans.la

squaretrans_la_SOURCES = transform.c squares.c
squaretrans_la_LDFLAGS = -module -avoid-version -shared

randtrans_la_SOURCES = transform.c rand.c
randtrans_la_LDFLAGS = -module -avoid-version -shared

quadtrans_la_SOURCES = transform.c quad.c
quadtrans_la_LDFLAGS = -module -avoid-version -shared

