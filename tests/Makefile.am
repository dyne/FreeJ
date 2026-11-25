
CXXTEST_TESTSUITES = $(srcdir)/testClosure.h

CXXTESTHOME = $(top_srcdir)/tests/cxxtest
CXXTESTFLAGS = --have-eh --error-printer
CXXTESTGEN = $(CXXTESTHOME)/cxxtestgen.pl $(CXXTESTFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
              -I$(CXXTESTHOME)

noinst_HEADERS = testClosure.h

check_PROGRAMS = cxxtests
TESTS = $(check_PROGRAMS) show-replay/test_timing.py
CLEANFILES = cxxtests.cpp

cxxtests_SOURCES = cxxtests.cpp
cxxtests_LDADD = $(top_builddir)/src/libfreej.la

cxxtests.cpp: $(CXXTEST_TESTSUITES)
	$(CXXTESTGEN) -o $@ $^

EXTRA_DIST = cxxtest freej-build-test.sh legacy-linux-sdk.py legacy-linux-sdk.lock \
             show-replay/README.md show-replay/replay.py show-replay/observe.cpp \
             show-replay/pulse.cpp show-replay/scene.js show-replay/video.js \
             show-replay/osc.js show-replay/audio.js show-replay/reset.js \
             show-replay/reset-target.js show-replay/timing.js show-replay/timing.py \
             show-replay/timing-observer.cpp show-replay/test_timing.py \
             show-replay/timing-budget.md
