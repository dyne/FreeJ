
CXXTEST_TESTSUITES = $(srcdir)/testClosure.h

CXXTESTHOME = $(top_srcdir)/tests/cxxtest
CXXTESTFLAGS = --have-eh --error-printer
CXXTESTGEN = $(CXXTESTHOME)/cxxtestgen.pl $(CXXTESTFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
              -I$(CXXTESTHOME)

noinst_HEADERS = testClosure.h

check_PROGRAMS = cxxtests frame-step-test lifetime-test movie-replay-test jack-output-contention-test
TESTS = $(check_PROGRAMS) show-replay/test_timing.py
AM_TESTS_ENVIRONMENT = MOVIE_TEST_CLIP="$(abs_srcdir)/show-replay/movie-av.mov"; export MOVIE_TEST_CLIP;
CLEANFILES = cxxtests.cpp

cxxtests_SOURCES = cxxtests.cpp
cxxtests_LDADD = $(top_builddir)/src/libfreej.la

cxxtests.cpp: $(CXXTEST_TESTSUITES)
	$(CXXTESTGEN) -o $@ $^

frame_step_test_SOURCES = show-replay/frame-step-test.cpp
frame_step_test_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/engine $(FREEJ_CFLAGS)
lifetime_test_SOURCES = show-replay/lifetime-test.cpp
lifetime_test_CPPFLAGS = $(frame_step_test_CPPFLAGS)
movie_replay_test_SOURCES = show-replay/movie-replay-test.cpp
jack_output_contention_test_SOURCES = show-replay/jack-output-contention-test.cpp \
                                     ../src/audio_jack.cpp ../src/ringbuffer.cpp
jack_output_contention_test_CPPFLAGS = $(AM_CPPFLAGS) $(JACK_CFLAGS)
jack_output_contention_test_LDFLAGS = -pthread
if FREEJ_STATIC_PROFILE
frame_step_test_LDFLAGS = -static $(FREEJ_STATIC_LOADER_FLAGS)
frame_step_test_LINK = $(CXX) $(frame_step_test_LDFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@
frame_step_test_DEPENDENCIES = $(top_builddir)/src/libfreej.la $(MOZJS_LIBS) $(FREEJ_STATIC_ARCHIVES)
frame_step_test_LDADD = $(FREEJ_STATIC_CLI_LIBS)
lifetime_test_LDFLAGS = $(frame_step_test_LDFLAGS)
lifetime_test_LINK = $(frame_step_test_LINK)
lifetime_test_DEPENDENCIES = $(frame_step_test_DEPENDENCIES)
lifetime_test_LDADD = $(frame_step_test_LDADD)
movie_replay_test_LDFLAGS = $(frame_step_test_LDFLAGS)
movie_replay_test_LINK = $(frame_step_test_LINK)
movie_replay_test_DEPENDENCIES = $(frame_step_test_DEPENDENCIES)
movie_replay_test_LDADD = $(frame_step_test_LDADD)
else
movie_replay_test_LINK = $(CXXLINK)
movie_replay_test_LDADD = $(top_builddir)/src/libfreej.la
frame_step_test_LINK = $(CXXLINK)
frame_step_test_LDADD = $(top_builddir)/src/libfreej.la
lifetime_test_LINK = $(CXXLINK)
lifetime_test_LDADD = $(frame_step_test_LDADD)
endif

EXTRA_DIST = cxxtest freej-build-test.sh legacy-linux-sdk.py legacy-linux-sdk.lock \
             native-linux-archives.py native-linux-archives.lock check-native-linux-archives.py \
             native-static-sdk.py native-static-sdk.lock native-static-sdk.patch \
             native-profile-factory.cpp native-profile-sandbox.c native-profile-runtime.py \
             movie-linux-sdk.sh movie-linux-sdk.lock \
             native-ci-prepare.sh native-ci-run.sh native-ci-coverage.py native-ci.lock \
             native_profile_services.py \
             show-replay/README.md show-replay/replay.py show-replay/observe.cpp \
             show-replay/pulse.cpp show-replay/scene.js show-replay/video.js \
             show-replay/osc.js show-replay/audio.js show-replay/reset.js \
             show-replay/reset-target.js show-replay/timing.js show-replay/timing.py \
             show-replay/timing-observer.cpp show-replay/test_timing.py \
             show-replay/timing-budget.md show-replay/movie-av.mov
