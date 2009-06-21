
CXXTEST_TESTSUITES = $(srcdir)/testClosure.h

CXXTESTHOME = $(top_srcdir)/tests/cxxtest
CXXTESTFLAGS = --have-eh --error-printer
CXXTESTGEN = $(CXXTESTHOME)/cxxtestgen.pl $(CXXTESTFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
              -I$(CXXTESTHOME)

check_PROGRAMS = cxxtests
TESTS = $(check_PROGRAMS)
CLEANFILES = cxxtests.cpp

cxxtests_SOURCES = cxxtests.cpp
cxxtests_LDADD = $(top_builddir)/src/libfreej.la

cxxtests.cpp: $(CXXTEST_TESTSUITES)
	$(CXXTESTGEN) -o $@ $^
