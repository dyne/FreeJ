AM_CFLAGS = $(FREEJ_CFLAGS) -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""
AM_CXXFLAGS = $(FREEJ_CFLAGS) -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\"" \
			  -DPACKAGE_DATA_DIR="\"$(PACKAGE_DATA_DIR)\"" \
			  -DPACKAGE_LIB_DIR="\"$(PACKAGE_LIB_DIR)\""

SUBDIRS = include

SOURCES = \
	context.cpp		layer.cpp	\
	fps.cpp			blitter.cpp	\
	sdl_blits.cpp		linear_blits.cpp \
	iterator.cpp		linklist.cpp	\
	jsync.cpp		closure.cpp	\
	callback.cpp		cpu_accel.cpp	\
	plugin.cpp		plugger.cpp	\
	frei0r.cpp		freeframe.cpp   \
	filter.cpp		parameter.cpp   \
	filter_instance.cpp \
	jutils.cpp		fastmemcpy.cpp  \
	ringbuffer.cpp  	convertvid.cpp  \
	logging.cpp geometry.cpp color.cpp \
\
        tvfreq.c		unicap_layer.cpp \
	v4l2_layer.cpp \
				image_layer.cpp \
	text_layer.cpp 		generator_layer.cpp \
	geo_layer.cpp		flash_layer.cpp \
	xscreensaver_layer.cpp   \
	xgrab_layer.cpp		opencv_cam_layer.cpp \
	audio_layer.cpp 	cairo_layer.cpp \
\
	screen.cpp		screen_js.cpp      \
	sdl_screen.cpp 		sdlgl_screen.cpp   \
	gl_screen.cpp		soft_screen.cpp    \
	aa_screen.cpp \
\
	controller.cpp  	console_ctrl.cpp  \
	console_calls_ctrl.cpp 	console_readline_ctrl.cpp \
	console_widgets_ctrl.cpp 	kbd_ctrl.cpp	\
	mouse_ctrl.cpp		vimo_ctrl.cpp	\
	joy_ctrl.cpp		midi_ctrl.cpp   \
	trigger_ctrl.cpp 	osc_ctrl.cpp \
	wiimote_ctrl.cpp 	sdl_controller.cpp \
\
	audio_jack.cpp  \
	audio_collector.cpp \
\
	video_encoder.cpp 	ffmpeg_encoder.cpp \
	theorautils.cpp 	oggtheora_encoder.cpp \
	impl_video_encoders.cpp

JS_SOURCES = 	context_js.cpp  \
		parameter_js.cpp \
		layer_js.cpp    \
		filter_js.cpp   \
		jsparser.cpp	\
		callbacks_js.cpp \
		video_encoder_js.cpp \
		cam_layer_js.cpp \
		image_layer_js.cpp \
		text_layer_js.cpp \
		geo_layer_js.cpp \
		generator_layer_js.cpp \
		flash_layer_js.cpp \
		 \
		cairo_layer_js.cpp \
		audio_collector_js.cpp \
		kbd_ctrl_js.cpp

lib_LTLIBRARIES = libfreej.la
libfreej_la_SOURCES = $(SOURCES) $(JS_SOURCES)
libfreej_la_LIBADD  = $(FREEJ_STATIC_LIBS)
libfreej_la_LDFLAGS = $(FREEJ_LIBS) -version-info $(FREEJ_VERSION_INFO)

bin_PROGRAMS = freej
freej_SOURCES = freej.cpp
freej_LDADD = libfreej.la $(X11_LIBS)
