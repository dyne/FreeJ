
CXXTEST_TESTSUITES = $(srcdir)/testClosure.h

CXXTESTHOME = $(top_srcdir)/tests/cxxtest
CXXTESTFLAGS = --have-eh --error-printer
CXXTESTGEN = $(CXXTESTHOME)/cxxtestgen.pl $(CXXTESTFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
              -I$(CXXTESTHOME)

noinst_HEADERS = testClosure.h

native_test_programs = cxxtests frame-step-test lifetime-test movie-replay-test jack-output-contention-test audio-collector-test jack-capture-test
check_PROGRAMS = $(native_test_programs)
TESTS = $(native_test_programs) show-replay/test_timing.py
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
audio_collector_test_SOURCES = show-replay/audio-collector-test.cpp ../src/audio_collector.cpp
audio_collector_test_CPPFLAGS = $(AM_CPPFLAGS) $(JACK_CFLAGS) $(FFTW_CFLAGS)
audio_collector_test_LDADD = $(FFTW_LIBS) -lpthread -lm
jack_capture_test_SOURCES = show-replay/jack-capture-test.cpp ../src/audio_jack.cpp ../src/audio_collector.cpp ../src/ringbuffer.cpp
jack_capture_test_CPPFLAGS = $(audio_collector_test_CPPFLAGS)
jack_capture_test_LDADD = $(audio_collector_test_LDADD)
check_PROGRAMS += frei0r-host-test
frei0r_host_test_SOURCES = show-replay/frei0r-host-test.cpp
frei0r_host_test_CPPFLAGS = $(frame_step_test_CPPFLAGS)
if FREEJ_STATIC_PROFILE
TESTS += frei0r-host-test
frei0r_host_test_SOURCES += show-replay/builtin-fixture.c
frei0r_host_test_LDFLAGS = $(frame_step_test_LDFLAGS)
frei0r_host_test_LINK = $(frame_step_test_LINK)
frei0r_host_test_DEPENDENCIES = $(frame_step_test_DEPENDENCIES)
frei0r_host_test_LDADD = $(frame_step_test_LDADD)
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
TESTS += show-replay/test-effects.py
AM_TESTS_ENVIRONMENT += CC='$(CC)'; export CC; \
                        FREEJ_EFFECT_CLI="$(abs_top_builddir)/src/freej"; export FREEJ_EFFECT_CLI;
frei0r_host_test_LINK = $(CXXLINK)
frei0r_host_test_LDADD = $(top_builddir)/src/libfreej.la
movie_replay_test_LINK = $(CXXLINK)
movie_replay_test_LDADD = $(top_builddir)/src/libfreej.la
frame_step_test_LINK = $(CXXLINK)
frame_step_test_LDADD = $(top_builddir)/src/libfreej.la
lifetime_test_LINK = $(CXXLINK)
lifetime_test_LDADD = $(frame_step_test_LDADD)
endif

EXTRA_DIST = frei0r-linux-sdk.sh show-replay/frei0r-fixture.c show-replay/freeframe-fixture.c show-replay/test-effects.py \
             cxxtest freej-build-test.sh legacy-linux-sdk.py legacy-linux-sdk.lock \
             native-linux-archives.py native-linux-archives.lock check-native-linux-archives.py \
             native-static-sdk.py native-static-sdk.lock native-static-sdk.patch \
             native-profile-factory.cpp native-profile-sandbox.c native-profile-runtime.py \
             movie-linux-sdk.sh movie-linux-sdk.lock \
             native-ci-prepare.sh native-ci-run.sh native-ci-coverage.py native-ci.lock \
             native_profile_services.py \
             show-replay/equalizer-observer.cpp show-replay/equalizer.py \
             show-replay/audio-response.js show-replay/audio-response.py \
             show-replay/README.md show-replay/replay.py show-replay/observe.cpp \
             show-replay/pulse.cpp show-replay/scene.js show-replay/video.js \
             show-replay/osc.js show-replay/osc-lifecycle.js show-replay/audio.js show-replay/reset.js \
             show-replay/reset-target.js show-replay/timing.js show-replay/timing.py \
             show-replay/timing-observer.cpp show-replay/test_timing.py \
             show-replay/timing-budget.md show-replay/movie-av.mov
