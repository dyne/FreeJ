# Small makefile for x86 architecture dependent filters
# these are separated here also because they require the
# NASM assembler compiler which somehow conflicts with the
# libtool compilation mechanism (go figure out that crap!)
#
# $Id$

CC = gcc
NASM = nasm
LINKER = ld
INSTALL = install -c
PREFIX = /usr/local


all: blurzoom.so absdiff.so

blurzoom.so: blurzoom.o blurzoomcore.o

absdiff.so: absdiff.o mmx_absdiff32.o

%.o: %.c
	$(CC) -Wall -O2 -ffast-math -c -o $@ $<
%.o: %.nas
	$(NASM) -f elf $<
%.so:
	$(LINKER) -E -z now -shared $^ -o $@

install: all
	$(INSTALL) -s *.so $(PREFIX)/lib/freej/

clean:
	rm -f *.o *.so