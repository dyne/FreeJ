INCLUDES = -I$(top_srcdir)/src/include

SUFFIXES = .asm

NASM = nasm -f elf

.asm.o:
	$(NASM) $<

noinst_LIBRARIES = libfilters.a

libfilters_a_SOURCES = \
	bitcolor.cpp	\
	strobe.cpp	\
	delaygrab.cpp	\
	absdiff.cpp	\
	rotozoom.cpp	\
	judenburg.cpp	\
	asm_judenburg.asm
