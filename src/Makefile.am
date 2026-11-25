AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = $(FREEJ_CFLAGS) -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""
AM_CXXFLAGS = $(FREEJ_CFLAGS) -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\"" \
			  -DPACKAGE_DATA_DIR="\"$(PACKAGE_DATA_DIR)\"" \
			  -DPACKAGE_LIB_DIR="\"$(PACKAGE_LIB_DIR)\""

SUBDIRS = include

ENGINE_SOURCES = \
	context.cpp layer.cpp screen.cpp geometry.cpp \
	engine/frame_step.cpp engine/frame_step.h \
	engine/frame_storage.cpp engine/frame_storage.h engine/surface_lock.h \
	fps.cpp blitter.cpp linear_blits.cpp iterator.cpp linklist.cpp \
	jsync.cpp closure.cpp callback.cpp \
	filter.cpp filter_instance.cpp parameter.cpp \
	plugin.cpp plugger.cpp frei0r.cpp freeframe.cpp \
	jutils.cpp fastmemcpy.cpp cpu_accel.cpp logging.cpp color.cpp ringbuffer.cpp

MEDIA_SOURCES = \
	image_layer.cpp generator_layer.cpp geo_layer.cpp text_layer.cpp cairo_layer.cpp \
	flash_layer.cpp audio_layer.cpp tvfreq.c unicap_layer.cpp v4l2_layer.cpp \
	xscreensaver_layer.cpp xgrab_layer.cpp opencv_cam_layer.cpp \
	audio_jack.cpp audio_collector.cpp convertvid.cpp \
	video_encoder.cpp ffmpeg_encoder.cpp theorautils.cpp oggtheora_encoder.cpp video_layer.cpp \
	impl_video_encoders.cpp

CONTROLLER_SOURCES = \
	controller.cpp console_ctrl.cpp console_calls_ctrl.cpp console_readline_ctrl.cpp \
	console_widgets_ctrl.cpp kbd_ctrl.cpp mouse_ctrl.cpp vimo_ctrl.cpp \
	joy_ctrl.cpp midi_ctrl.cpp trigger_ctrl.cpp osc_ctrl.cpp wiimote_ctrl.cpp \
	sdl_controller.cpp

DISPLAY_SOURCES = \
	sdl_blits.cpp sdl_screen.cpp sdlgl_screen.cpp gl_screen.cpp soft_screen.cpp aa_screen.cpp

JS_SOURCES = \
	context_js.cpp parameter_js.cpp layer_js.cpp filter_js.cpp screen_js.cpp \
	jsparser.cpp callbacks_js.cpp video_encoder_js.cpp cam_layer_js.cpp image_layer_js.cpp \
	text_layer_js.cpp geo_layer_js.cpp generator_layer_js.cpp flash_layer_js.cpp video_layer_js.cpp \
	cairo_layer_js.cpp audio_collector_js.cpp kbd_ctrl_js.cpp

lib_LTLIBRARIES = libfreej.la
libfreej_la_SOURCES = $(ENGINE_SOURCES) $(MEDIA_SOURCES) $(CONTROLLER_SOURCES) $(DISPLAY_SOURCES) $(JS_SOURCES)
libfreej_la_LIBADD = $(FREEJ_STATIC_LIBS) libmovie_decoder.la

noinst_LTLIBRARIES = libmovie_decoder.la
libmovie_decoder_la_SOURCES = media/movie_decoder.cpp media/movie_decoder.h media/movie_registration.h
libmovie_decoder_la_CPPFLAGS = $(AM_CPPFLAGS) $(FFMPEG_CFLAGS)

bin_PROGRAMS = freej
freej_SOURCES = freej.cpp

if FREEJ_STATIC_PROFILE
libfreej_la_SOURCES += static_loader.cpp
libfreej_la_LDFLAGS = -static -version-info $(FREEJ_VERSION_INFO)
freej_LDFLAGS = -static -Wl,-Map,freej.map $(FREEJ_STATIC_LOADER_FLAGS)
freej_LINK = $(CXX) $(AM_CXXFLAGS) $(CXXFLAGS) $(freej_LDFLAGS) $(LDFLAGS) -o $@
freej_DEPENDENCIES = libfreej.la $(MOZJS_LIBS) $(FREEJ_STATIC_ARCHIVES)
freej_LDADD = $(FREEJ_STATIC_CLI_LIBS)

install-data-local:
	$(MKDIR_P) "$(DESTDIR)$(libdir)/freej-sdk"
	$(INSTALL_DATA) $(MOZJS_LIBS) "$(DESTDIR)$(libdir)/freej-sdk/libfreej-mozjs.a"
	for archive in $(FREEJ_STATIC_ARCHIVES); do \
		$(INSTALL_DATA) "$$archive" "$(DESTDIR)$(libdir)/freej-sdk" || exit $$?; \
	done

uninstall-local:
	rm -f "$(DESTDIR)$(libdir)/freej-sdk/libfreej-mozjs.a"
	for archive in $(FREEJ_STATIC_ARCHIVES); do \
		rm -f "$(DESTDIR)$(libdir)/freej-sdk/$${archive##*/}" || exit $$?; \
	done
else
libfreej_la_LDFLAGS = $(FREEJ_LIBS) -version-info $(FREEJ_VERSION_INFO)
freej_LINK = $(CXXLINK)
freej_LDADD = libfreej.la $(X11_LIBS)
endif
