AC_DEFUN([FREEJ_CHECK_STATIC_SDK], [
  AS_IF([test "x$FREEJ_NATIVE_PROFILE" = xstatic], [
    AS_IF([test -z "$with_native_sdk" || test "x$with_native_sdk" = xyes ||
           test "x$with_native_sdk" = xno],
      [AC_MSG_ERROR([the static profile requires --with-native-sdk=PREFIX])])
    FREEJ_STATIC_SDK=`cd "$with_native_sdk" && pwd -P` ||
      AC_MSG_ERROR([cannot open the native archive SDK $with_native_sdk])
    AS_CASE([$FREEJ_STATIC_SDK],
      [*[[!abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_./-]]*],
      [AC_MSG_ERROR([the native archive SDK path must be free of spaces or shell metacharacters])])

    AS_IF([test "x$have_mozjs" = xyes],
      [AC_MSG_ERROR([the static profile requires the bundled JavaScript interpreter (--enable-static-mozjs)])])

    freej_archives=""
    freej_stage_archives=""
    freej_require_archives () {
      freej_capability=$[]1
      shift
      for freej_name
      do
        freej_archive="$FREEJ_STATIC_SDK/lib/lib$freej_name.a"
        AS_IF([test ! -f "$freej_archive" || ! ar t "$freej_archive" >/dev/null 2>&1],
          [AC_MSG_ERROR([$freej_capability requires archive $freej_archive])])
        AS_CASE([" $freej_archives "], [*" $freej_archive "*], [], [
          freej_archives="$freej_archives $freej_archive"
          freej_stage_archives="$freej_stage_archives \${libdir}/freej-sdk/lib$freej_name.a"
        ])
      done
    }

    freej_require_archives "SDL video/joystick" SDL FreejSDL2 X11 Xext Xrender Xcursor Xi Xfixes Xrandr Xss xcb Xau Xdmcp
    AS_IF([! $NM "$FREEJ_STATIC_SDK/lib/libSDL.a" | grep -q ' U FreejSDL2_Init$'],
      [AC_MSG_ERROR([SDL video/joystick requires the direct SDL2 binding in libSDL.a])])
    AS_IF([test "x$have_sdlgfx" != xyes || test "x$have_sdlttf" != xyes],
      [AC_MSG_ERROR([the static profile requires the pinned SDL_gfx and SDL_ttf SDK metadata])])
    freej_require_archives "SDL image, geometry and text" SDL_image SDL_gfx SDL_ttf png16 jpeg tiff webp sharpyuv jbig deflate lzma Lerc zstd z freetype bz2 brotlidec brotlicommon
    freej_require_archives "S-Lang console" slang
    freej_require_archives "historical FFmpeg media" avcodec avformat avutil swscale sndio asound
    freej_require_archives "recording and streaming" ogg vorbis vorbisenc theora shout speex ssl crypto cairo pixman-1 fontconfig expat xcb-render xcb-shm
    freej_require_archives "JACK capture and playback" jack db
    AS_IF([! $NM "$FREEJ_STATIC_SDK/lib/libjack.a" | grep -q ' T jack_get_tmpdir$'],
      [AC_MSG_ERROR([JACK capture and playback requires the archived JACK client])])
    AS_IF([$NM -u "$FREEJ_STATIC_SDK/lib/libjack.a" | grep -q ' U popen$'],
      [AC_MSG_ERROR([JACK capture and playback requires the direct runtime directory client])])
    freej_require_archives "audio FFT and resampling" fftw3 samplerate
    freej_require_archives "OSC transport" lo
    AS_IF([test "x$have_gd" = xyes], [freej_require_archives "PNG screenshots" gd])
    AS_IF([test "x$have_aa" = xyes], [freej_require_archives "ASCII display" aa])
    AS_IF([test "x$have_cwiid" = xyes], [freej_require_archives "WiiMote controls" cwiid bluetooth])
    AS_IF([test "x$have_unicap" = xyes || test "x$enable_unicap" = xyes],
      [AC_MSG_ERROR([unicap capture has no verified static SDK closure])])
    AS_IF([test "x$have_opencv" = xyes || test "x$enable_opencv" = xyes],
      [AC_MSG_ERROR([OpenCV capture has no verified static SDK closure])])
    AS_IF([test "x$enable_opengl" = xyes],
      [AC_MSG_ERROR([the OpenGL loader has no verified static SDK closure])])
    AS_IF([test "x$with_dmalloc" = xyes], [freej_require_archives "dmalloc instrumentation" dmallocthcxx])

    FREEJ_STATIC_ARCHIVES=$freej_archives
    FREEJ_STATIC_STAGE_ARCHIVES=$freej_stage_archives
    FREEJ_STATIC_SYSTEM_LIBS="-pthread -lm -ldl -lrt"
    FREEJ_STATIC_LOADER_FLAGS="-Wl,--wrap=dlopen -Wl,--wrap=dlmopen"
    FREEJ_STATIC_CLI_LIBS='-Wl,--whole-archive $(builddir)/.libs/libfreej.a -Wl,--no-whole-archive -Wl,--start-group $(MOZJS_LIBS) $(FREEJ_STATIC_ARCHIVES) $(FREEJ_STATIC_SYSTEM_LIBS) -Wl,--end-group'
    AC_MSG_CHECKING([whether the archive SDK links with a static C++ runtime])
    freej_save_LIBS=$LIBS
    freej_save_LDFLAGS=$LDFLAGS
    LIBS="-Wl,--start-group $FREEJ_STATIC_ARCHIVES $FREEJ_STATIC_SYSTEM_LIBS -Wl,--end-group"
    LDFLAGS="$LDFLAGS -static"
    AC_LANG_PUSH([C++])
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[
        #include <exception>
        extern "C" int SDL_Init(unsigned int);
        extern "C" void *jack_ringbuffer_create(unsigned long);
        extern "C" const char *src_get_version(void);
      ]], [[
        return SDL_Init(0) || !jack_ringbuffer_create(32) || !src_get_version()
          || !std::exception().what();
      ]])
    ], [AC_MSG_RESULT([yes])], [
      AC_MSG_FAILURE([the SDL/JACK/resampling archive closure cannot link statically. See config.log])
    ])
    AC_LANG_POP([C++])
    LIBS=$freej_save_LIBS
    LDFLAGS=$freej_save_LDFLAGS
    AC_DEFINE([FREEJ_STRICT_STATIC], [1], [Build the static Linux stage profile])
  ])
  AC_SUBST([FREEJ_STATIC_ARCHIVES])
  AC_SUBST([FREEJ_STATIC_STAGE_ARCHIVES])
  AC_SUBST([FREEJ_STATIC_SYSTEM_LIBS])
  AC_SUBST([FREEJ_STATIC_LOADER_FLAGS])
  AC_SUBST([FREEJ_STATIC_CLI_LIBS])
])
