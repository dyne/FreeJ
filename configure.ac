dnl ==============================================================
dnl Process this file with autoconf to produce a configure script.
dnl ==============================================================

AC_INIT([freej],[0.11.0])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

dnl ==============================================================
dnl Get the operating system and version number...
dnl ==============================================================
AC_MSG_CHECKING([for which platform we are compiling])
case "$host_os" in
     *linux*)
     AC_MSG_RESULT([Linux])
     AC_DEFINE(HAVE_LINUX,1,[define if compiling for Linux])
     DL_LIBS="-ldl"
     have_linux=yes

     case "$host_cpu" in
       powerpc*)
       AC_DEFINE(ARCH_PPC,1,[define if compiling for Linux/PPC])
       AC_DEFINE(ARCH_POWERPC,1,[define if compiling for Linux/PPC])
       # TODO(godog) what about ppc64?
       AC_DEFINE(POWERPC_MODE_32BITS,1,[define if compiling for PPC])
       AC_DEFINE(HAVE_ALTIVEC,1,[define if cpu supports Altivec instruction set])
       AC_DEFINE(HAVE_ALTIVEC_H,1,[define if cpu supports Altivec instruction set])
       AC_DEFINE(WORDS_BIGENDIAN,1,[define if compiling for Big Endian CPU])
       AC_DEFINE(PA_BIG_ENDIAN,1,[define big endian arch for portaudio])
#       AC_DEFINE(SDL_BYTEORDER,SDL_BIG_ENDIAN,[define big endian arch for SDL])
       have_ppc=yes
       ;;

       x86_64*)
       AC_DEFINE(ARCH_X86,1,[define if compiling for Linux/x86])
       AC_DEFINE(HAVE_64BIT,1,[define if host has 64 bit])
       ;;

       i*86*)
       AC_DEFINE(ARCH_X86,1,[define if compiling for Linux/x86])
       AC_DEFINE(PA_LITTLE_ENDIAN,1,[define little endian arch for portaudio])
#       AC_DEFINE(SDL_BYTEORDER,SDL_LIL_ENDIAN,[define little endian arch for SDL])
       ;;
     esac
     ;;

     *darwin*)
     AC_MSG_RESULT([Darwin/OSX])
     AC_DEFINE(HAVE_DARWIN,1,[define if compiling for Apple Darwin OSX])
     AC_DEFINE(CONFIG_DARWIN,1,[define if compiling for Apple Darwin OSX])
     have_darwin=yes

     # detect ppc also on darwin, for lubrifications
     case "$host_cpu" in
       powerpc*)
         have_ppc=yes
       ;;
     esac
     ;;

     *freebsd*)
     AC_MSG_RESULT([FreeBSD])
     AC_DEFINE(HAVE_FREEBSD,1,[define if compiling for FreeBSD])
     AC_DEFINE(HAVE_SYS_SOCKET_H,1,[define if compiling for FreeBSD])
     GLOBAL_CFLAGS="$GLOBAL_CFLAGS -I/usr/local/include -L/usr/local/lib"
     have_freebsd=yes
     ;;

     *)
     AC_MSG_RESULT([$host_os?!])
     AC_MSG_ERROR([[
[!] Your system architecture is not supported by FreeJ
[!] if you are interested in porting FreeJ to your architecture
[!] you are very welcome to contact me <jaromil@dyne.org>
]], 0)
     ;;
esac

AC_SUBST(DL_LIBS)


dnl ==============================================================
dnl Setup for automake
dnl ==============================================================

AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/freej.cpp])
AC_CONFIG_HEADERS([config.h])

# never run autotools and configure automatically
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_AS
# compatibility with older libtool
m4_ifndef([LT_INIT], [AC_PROG_RANLIB])

# from lib/goom
AM_PROG_LEX
AC_PROG_YACC

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h memory.h stdint.h stdlib.h \
	string.h strings.h sys/file.h sys/ioctl.h sys/time.h termios.h \
	unistd.h])

# from lib/cwiid
AC_CHECK_HEADERS([sys/socket.h])
# from lib/goom
AC_CHECK_HEADERS([stddef.h libintl.h malloc.h])
# from lib/lo
AC_CHECK_HEADERS([netdb.h netinet/in.h arpa/inet.h float.h])
# from lib/javascript
#AC_CHECK_HEADERS([sgtty.h termio.h locale.h sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# from lib/cwiid
AC_TYPE_SSIZE_T
# from lib/javascript
#AC_CHECK_TYPES([ptrdiff_t])
#AC_STRUCT_TIMEZONE

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero floor gettimeofday memmove memset munmap rint select \
		setenv strcasecmp strdup strerror strncasecmp strrchr strstr])

# bindings/perl
AC_CHECK_FUNCS([strtoul])
AC_FUNC_STRTOD
# lib/goom
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([strtol])
# lib/cwiid
AC_CHECK_FUNCS([socket clock_gettime])
# lib/flash
AC_CHECK_FUNCS([sqrt])
# lib/lo
AC_CHECK_FUNCS([strchr gethostname gethostbyname strpbrk atexit])
# lib/sdl_gfx
AC_CHECK_FUNCS([pow])
# lib/shout/net
AC_CHECK_FUNCS([gethostbyaddr inet_ntoa])
# lib/javascript
#AC_FUNC_MKTIME
#AC_CHECK_FUNCS([putenv getcwd localeconv ftime localtime_r])

# compatibility with older libtool
m4_ifdef([LT_INIT],
	[LT_INIT([disable-static dlopen])],
	[AC_LIBTOOL_DLOPEN
	 AC_DISABLE_STATIC
	 AC_PROG_LIBTOOL])

dnl versioning info for libtool
FREEJ_CURRENT=0
FREEJ_REVISION=0
FREEJ_AGE=0
FREEJ_VERSION_INFO="$FREEJ_CURRENT:$FREEJ_REVISION:$FREEJ_AGE"
dnl Note this is the ABI version which is not the same as our actual library version
AC_SUBST(FREEJ_VERSION_INFO)


dnl ==============================================================
dnl check for swig and whether to enable bindings
dnl ==============================================================
AC_PROG_SWIG([1.3])
SWIG_ENABLE_CXX

if test -z "$SWIG_LIB"; then
    have_swig=no

    AC_MSG_WARN([No language bindings will be created])
else
    have_swig=yes

    SWIG_COMMAND='$(SWIG) -c++ -O -Wall -I$(top_builddir) -I$(top_srcdir)/src/include -I$(top_builddir)/doc'
    AC_SUBST(SWIG_COMMAND)
    SWIG_IFACE='$(top_srcdir)/bindings/freej.i'
    AC_SUBST(SWIG_IFACE)

    ENABLE_SWIG_PYTHON
    ENABLE_SWIG_RUBY
    ENABLE_SWIG_JAVA
    ENABLE_SWIG_PERL
fi

AM_CONDITIONAL([BUILD_SWIG_PYTHON], [test x$enable_python = xyes])
AM_CONDITIONAL([BUILD_SWIG_RUBY], [test x$enable_ruby = xyes])
AM_CONDITIONAL([BUILD_SWIG_JAVA], [test x$enable_java = xyes])
AM_CONDITIONAL([BUILD_SWIG_PERL], [test x$enable_perl = xyes])

# swig not found but bindings requested is fatal
if test x$have_swig = xno && {
      test x$enable_python = xyes ||
      test x$enable_java = xyes   ||
      test x$enable_ruby = xyes   ||
      test x$enable_perl = xyes
   }; then
    AC_MSG_ERROR([bindings requested but swig not found!])
fi


dnl ==============================================================
dnl Add the local include path and some flags
dnl ==============================================================

#-freg-struct-return is to compile an host compatible with freeframe dso
GLOBAL_CFLAGS="$GLOBAL_CFLAGS -pipe -D_REENTRANT -freg-struct-return -fPIC"
if test x$have_darwin = xyes; then
  # add the fink default path to find headers
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -I/sw/include"
fi


dnl ==============================================================
dnl Check for SDL
dnl ==============================================================
PKG_CHECK_MODULES(SDL, sdl > 1.2, :,
	AC_MSG_ERROR([*** SDL development files not found!]))


dnl ==============================================================
dnl Check if there is FREETYPE2
dnl ==============================================================
PKG_CHECK_MODULES(FT2, freetype2,
                  have_freetype2=yes, have_freetype2=no)
if test x$have_freetype2 = xyes; then
   AC_DEFINE(WITH_FT2,1,[define if linking freetype for text rendering])
fi

   
dnl ==============================================================
dnl Check if there is FONTCONFIG
dnl ==============================================================
PKG_CHECK_MODULES(FC, fontconfig,
                  have_fontconfig=yes, have_fontconfig=no)
if test x$have_fontconfig = xyes; then
   AC_DEFINE(WITH_FC,1,[define if linking fontconfig for font indexing])
fi
   

dnl ==============================================================
dnl Check X11 for xgrab
dnl ==============================================================
PKG_CHECK_MODULES(X11, x11, have_x11=yes, have_x11=no)


dnl ==============================================================
dnl Check OGG VORBIS THEORA
dnl ==============================================================
PKG_CHECK_MODULES(OGG, ogg >= 1.1, :,
	AC_MSG_ERROR([*** Ogg >=1.1 development files not found!]))
PKG_CHECK_MODULES(VORBIS, vorbis, :,
	AC_MSG_ERROR([*** Vorbis development files not found!]))
PKG_CHECK_MODULES(VORBISENC, vorbisenc, :,
	AC_MSG_ERROR([*** Vorbisenc development files not found!]))
PKG_CHECK_MODULES(THEORA, theora, :,
	AC_MSG_ERROR([*** Theora development files not found!]))
XIPH_CFLAGS="$OGG_CFLAGS $VORBIS_CFLAGS $VORBISENC_CFLAGS $THEORA_CFLAGS"
XIPH_LIBS="$OGG_LIBS $VORBIS_LIBS $VORBISENC_LIBS $THEORA_LIBS"
AC_SUBST([XIPH_CFLAGS])
AC_SUBST([XIPH_LIBS])

# TODO(shammash): heritage from conditional build, still used somewhere.
AC_DEFINE(WITH_OGGTHEORA,1,[define if compiling with Ogg/Theora encoding])


dnl ==============================================================
dnl FFMPEG DYNAMIC
dnl ==============================================================
PKG_CHECK_MODULES(AVCODEC, libavcodec, :,
	AC_MSG_ERROR([*** Libavcodec development files not found!]))
PKG_CHECK_MODULES(AVFORMAT, libavformat, :,
	AC_MSG_ERROR([*** Libavformat development files not found!]))
PKG_CHECK_MODULES(AVUTIL, libavutil, :,
	AC_MSG_ERROR([*** Libavutil development files not found!]))
PKG_CHECK_MODULES(SWSCALE, libswscale, :,
	AC_MSG_ERROR([*** Libswscale development files not found!]))
FFMPEG_CFLAGS="$AVCODEC_CFLAGS $AVFORMAT_CFLAGS $AVUTIL_CFLAGS $SWSCALE_CFLAGS"
FFMPEG_LIBS="$AVCODEC_LIBS $AVFORMAT_LIBS $AVUTIL_LIBS $SWSCALE_LIBS"
AC_SUBST([FFMPEG_CFLAGS])
AC_SUBST([FFMPEG_LIBS])

# FFmpeg changed include path policy, let's check which one is in use.
FREEJ_SAVE_FLAGS
CPPFLAGS="$CPPFLAGS $FFMPEG_CFLAGS"
CFLAGS="$CFLAGS $FFMPEG_CFLAGS"
AC_CHECK_HEADERS(libavcodec/avcodec.h ffmpeg/avcodec.h)
AC_CHECK_HEADERS(libavformat/avformat.h ffmpeg/avformat.h)
AC_CHECK_HEADERS(libavutil/avutil.h ffmpeg/avutil.h)
AC_CHECK_HEADERS(libswscale/swscale.h ffmpeg/swscale.h)
FREEJ_RESTORE_FLAGS
# Then in sources we have to include in the following way:
#    #ifdef HAVE_LIBAVCODEC_AVCODEC_H
#    #   include <libavcodec/avcodec.h>
#    #elif defined(HAVE_FFMPEG_AVCODEC_H)
#    #   include <ffmpeg/avcodec.h>
#    #else
#    #   include <avcodec.h>
#    #endif

# TODO(shammash): heritage from conditional build, still used somewhere.
AC_DEFINE(WITH_FFMPEG,1, [define if compiling movie layer linking to ffmpeg libavcodec])
AC_DEFINE(WITH_SWSCALE,1, [define if using new swscale library from ffmpeg])

dnl ==============================================================
dnl ENABLE LIBFLASH
dnl ==============================================================
AC_ARG_ENABLE(flash,
    AS_HELP_STRING([--enable-flash],[compile with Flash animation layer (yes)]),
    [have_flash=$enableval],
    [have_flash=yes])
AC_MSG_CHECKING([flash - v3 animation only - layer])
if test x$have_flash = xyes; then
    AC_DEFINE(WITH_FLASH,1,[define if compiling flash layer])
    FLASH_LIBS="\$(top_builddir)/lib/flash/libflash.la"
    FLASH_CFLAGS="-I\$(top_srcdir)/lib/flash"
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(BUILD_FLASH, [test x$have_flash = xyes])
AC_SUBST(FLASH_LIBS)
AC_SUBST(FLASH_CFLAGS)


dnl ==============================================================
dnl CHECK if there is BLUETOOTH
dnl ==============================================================
PKG_CHECK_MODULES(BLUEZ, bluez, have_bluez=yes, have_bluez=no)
if test x$have_bluez = xyes; then
    AC_DEFINE(WITH_BLUEZ,1,[define if compiling bluetooth cwiid controller])
    CWIID_LIBS="\$(top_builddir)/lib/cwiid/libwiimote.la"
    CWIID_CFLAGS="-I\$(top_srcdir)/lib/cwiid"
fi
AM_CONDITIONAL(BUILD_CWIID, [test x$have_bluez = xyes])
AC_SUBST(CWIID_LIBS)
AC_SUBST(CWIID_CFLAGS)


dnl ==============================================================
dnl CHECK if there is VIDEO4LINUX
dnl ==============================================================
AC_CHECK_HEADERS([linux/videodev.h], [have_videodev=yes], [have_videodev=no])
AC_ARG_ENABLE(v4l,
    AS_HELP_STRING([--enable-v4l],[compile with Video4Linux Layer (autodetect)]),
    [enable_v4l=$enableval],
    [enable_v4l=autodetect])

if test x$have_videodev = xno && test x$enable_v4l = xyes; then
    AC_MSG_ERROR([*** v4l enabled but header videodev.h not found!])
fi

if test x$have_videodev = xyes && {
        test x$enable_v4l = xyes ||
        test x$enable_v4l = xautodetect
   }; then
        AC_DEFINE(WITH_V4L,1,[define if compiling video4linux layer])
fi

dnl ==============================================================
dnl compile with full warnings and debugging symbols
dnl ==============================================================
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],[compile with debug symbols (no)]),
    [enable_debug=$enableval],
    [enable_debug=no])

# XXX(godog) is this used?
AM_CONDITIONAL(DEBUG, [test x$enable_debug = xyes])

if test x$enable_debug = xyes; then
    GLOBAL_CFLAGS="$GLOBAL_CFLAGS -Wall -g -ggdb"
fi


dnl ==============================================================================
dnl CHECK to use profiling flags when compiling, for execution analysis with gprof
dnl ==============================================================================
AC_ARG_ENABLE(profiling,
    AS_HELP_STRING([--enable-profiling],[compile using gprof flags for execution analysis (no)]),
    [enable_profiling=$enableval],
    [enable_profiling=no])

if test x$enable_profiling = xyes; then
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -pg"
fi


dnl ==============================================================
dnl CHECK to use processor specific optimizations to compile
dnl ==============================================================
AC_ARG_ENABLE(cpuflags,
    AS_HELP_STRING([--enable-cpuflags],[compile with advanced cpu instructions (yes)]),
    [enable_cpuflags=$enableval],
    [enable_cpuflags=yes])

if test x$enable_cpuflags = xyes; then
  if test x$have_linux = xyes; then
     if grep "^flags.* mmx" /proc/cpuinfo > /dev/null; then
       have_mmx=yes
       AC_DEFINE(HAVE_MMX,1,[define if enabling MMX acceleration])
     fi
     if grep "^flags.* sse" /proc/cpuinfo > /dev/null; then
       have_sse=yes
       AC_DEFINE(HAVE_SSE,1,[define if enabling SSE acceleration])
     fi
     if grep "^flags.* sse2" /proc/cpuinfo > /dev/null; then
       have_sse=yes
       AC_DEFINE(HAVE_SSE2,1,[define if enabling SSE2 acceleration])
     fi
     if grep "^flags.* ssse3" /proc/cpuinfo > /dev/null; then
       have_ssse3=yes
       AC_DEFINE(HAVE_SSSE3,1,[define if enabling SSSE3 acceleration])
     fi
  fi
  if test x$have_freebsd = xyes; then
     if sysctl -n hw.instruction_sse; then
         # XXX(godog) is this MMX or SSE?
         have_mmx=yes
         AC_DEFINE(HAVE_MMX,1,[define if enabling MMX acceleration])
     fi
  fi
fi

AC_ARG_ENABLE(lubrify,
    AS_HELP_STRING([--enable-lubrify],[compile using processor lubrifications (yes)]),
    [enable_lubrifications=$enableval],
    [enable_lubrifications=yes])

# XXX(godog) integrate with AX_GCC_ARCHFLAG
if test x$enable_lubrifications = xyes; then
  if test x$enable_profiling = xyes || test x$enable_debug = xyes; then
    AC_MSG_WARN([cannot lubrify code if debug or profiling are enabled])
    enable_lubrifications=no
  else
    if test x$have_darwin = xyes; then
      if test x$have_ppc = xyes; then
        # use optimization flags for darwin
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mabi=altivec -maltivec -mpowerpc-gfxopt"
      else
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mfpmath=sse -mtune=pentium4"
      fi
    # adding these provoke 'illegal instruction' errors in some
    # ppc/gcc combinations: -mpowerpc-gfxopt -faltivec"
    elif test x$have_ppc = xyes; then
      GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -fomit-frame-pointer -ffast-math -mabi=altivec -maltivec -mpowerpc-gfxopt"
    elif test x$host_cpu = xx86_64; then
      # should complete this with all 64bit CPUs (AMD too)
      if test x$have_ssse3 = xyes; then
        # multiple core cpu
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mfpmath=sse -mtune=core2"
      else
        # single core 64bit cpu
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mfpmath=sse -mtune=nocona"
      fi
    else
      # any other platform optimizations
      AC_MSG_WARN([no special optimisation flags found for your cpu])
      AC_MSG_WARN([please suggest some on our mailinglist if you know better!])
      GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2"
    fi
  fi
else
  if test x$enable_profiling = xno && test x$enable_debug = xno; then
    AC_MSG_NOTICE([enabling generic compiler optimizations])
    GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2"
  fi
fi


dnl ==============================================================
dnl CHECK TO USE OPENGL VIDEO OUTPUT
dnl ==============================================================
AC_ARG_ENABLE(opengl,
    AS_HELP_STRING([--enable-opengl],[experimental opengl rendering (no)]),
    [enable_opengl=$enableval],
    [enable_opengl=no])

if test x$enable_opengl = xyes; then
    OPENGL_LIBS="-lGL -lGLU"
    AC_DEFINE(WITH_OPENGL,1,[define if using experimental opengl rendering])
fi
AC_SUBST(OPENGL_LIBS)


dnl ==============================================================
dnl Audio
dnl ==============================================================
PKG_CHECK_MODULES(ALSA, alsa, have_alsa=yes, have_alsa=no)

# TODO(shammash): is this enough to enable midi on alsa?
if test x$have_alsa = xyes; then
  AC_DEFINE(WITH_MIDI,1,[define if compiling midi controller])
fi

# TODO(shammash): before this refactoring we were enabling jack
# only if alsa was present, check inconsistencies in the code.
PKG_CHECK_MODULES(JACK, jack, have_jack=yes, have_jack=no)

PKG_CHECK_MODULES(FFTW, fftw3, have_fftw=yes, have_fftw=no)

# TODO(shammash): here we enable audio only if FFTW is present which is
# probably wrong. Maybe it's better to split checks, but this needs
# code refactoring as well. When changing this policy please remember
# to change output summary at the end of this file.
if test x$have_jack = xyes && test x$have_fftw = xyes; then
  AC_DEFINE(WITH_SOUND,1,[define if compiling sound system])
  have_sound=yes
fi

# goom removed because gives compilation problems
# replicated on Niels ubuntu 7.10
if test x$have_sound = xyes && test 1 = 0; then
  enable_goom=yes
  AC_DEFINE(WITH_GOOM,1,[define if goom is enabled])
  GOOM_LIBS="\$(top_builddir)/lib/goom/libgoom2.a"
  GOOM_CFLAGS="-I\$(top_srcdir)/lib/goom"
  if test x$host_cpu = xpowerpc; then
    GOOM_PPC_FILES="ppc_zoom_ultimate.s ppc_drawings.s"
  fi
else
  enable_goom=no
fi
AC_SUBST(GOOM_LIBS)
AC_SUBST(GOOM_CFLAGS)
AC_SUBST(GOOM_PPC_FILES)

AM_CONDITIONAL([BUILD_GOOM], [test x$enable_goom = xyes])


dnl ==============================================================
dnl compile including the javascript interpreter
dnl ==============================================================

PKG_CHECK_MODULES(MOZJS, mozilla-js libxul-embedding-unstable, have_mozjs=yes, have_mozjs=no)
if test x$have_mozjs = xno; then
PKG_CHECK_MODULES(MOZJS, xulrunner-js, have_mozjs=yes, have_mozjs=no)
 if test x$have_mozjs = xyes; then
  dnl see https://bugs.launchpad.net/ubuntu/+source/xulrunner-1.9/+bug/273956
  dnl    pps will fail to link libmozjs since the library is compiled without -DJS_THREADSAFE
  dnl    and to the contrary the .pc file xulrunner-js adopts -DJS_THREADSAFE in the CFLAGS
  dnl    this will generate such an error:
  dnl    undefined reference to `js_GetSlotThreadSafe(JSContext*, JSObject*, unsigned int)'
  MOZJS_CFLAGS="`echo $MOZJS_CFLAGS | sed 's/-DJS_THREADSAFE//'`"
  dnl    i really cannot figure out how comes the packaging of xulrunner is such a crap
  dnl    in debian, ubuntu and so on. we are stuck at static linking since years :/
 fi
fi
if test x$have_mozjs = xno; then
# test on fedora
PKG_CHECK_MODULES(MOZJS, libxul-embedding, have_mozjs=yes, have_mozjs=no)
 if test x$have_mozjs = xyes; then
    MOZJS_CFLAGS=`$PKG_CONFIG --cflags libxul-embedding --define-variable=includetype=js`
    MOZJS_LIBS=`$PKG_CONFIG --libs libxul-embedding`
    MOZJS_LIBS="$MOZJS_LIBS -lmozjs"
 fi
fi

if test x$have_mozjs = xno; then
   MOZJS_CFLAGS=" -I\$(top_srcdir)/lib/javascript  -I\$(top_srcdir)/lib/javascript/obj "
   # libs in top_srcdir instead of top_builddir because js is not under autotools
   MOZJS_LIBS="\$(top_srcdir)/lib/javascript/obj/libjs.a"
   AC_MSG_NOTICE([Using freej version of MOZJS])
fi

if test x$enable_debug = xyes ; then
   MOZJS_CFLAGS="$MOZJS_CFLAGS -DJS_GCMETER"
fi

if test x$have_freebsd = xyes; then 
   MOZJS_CFLAGS="-I/usr/local/include -L/usr/local/lib -I../lib/javascript"
   MOZJS_LIBS="-ljs"
   AC_DEFINE(XP_UNIX,1,[Define use of UNIX types in javascript])
fi

if test x$have_linux = xyes ; then
   AC_DEFINE(XP_UNIX,1,[Define use of UNIX types in javascript])
fi
if test x$have_darwin = xyes ; then
   AC_DEFINE(XP_MAC,1,[Define use of MAC types in javascript])
fi

AC_SUBST(MOZJS_CFLAGS)
AC_SUBST(MOZJS_LIBS)

AC_DEFINE(WITH_JAVASCRIPT,1,[Define if using libmozjs for spidermonkey javascript])

AM_CONDITIONAL([BUILD_JAVASCRIPT], [test x$have_mozjs = xno])


dnl ==============================================================
dnl unicap lib
dnl ==============================================================
PKG_CHECK_MODULES(UNICAP, libunicap, have_unicap=yes, have_unicap=no)
if test x$have_unicap = xyes; then
   AC_DEFINE(WITH_UNICAP,1,[Define if to link libunicap for camera capture])
fi

dnl ==============================================================
dnl opencv lib
dnl ==============================================================
PKG_CHECK_MODULES(OPENCV, opencv, have_opencv=yes, have_opencv=no)
if test x$have_opencv = xyes; then
   AC_DEFINE(WITH_OPENCV,1,[Define if to link opencv framework])
fi

dnl ==============================================================
dnl shout lib
dnl ==============================================================
SHOUT_LIBS="\$(top_builddir)/lib/shout/libshout.la"
SHOUT_CFLAGS="-I\$(top_srcdir)/lib/shout"
AM_CONDITIONAL(BUILD_SHOUT, true)
AC_SUBST(SHOUT_LIBS)
AC_SUBST(SHOUT_CFLAGS)
dnl PKG_CHECK_MODULES(SHOUT, shout, have_shout=yes, have_shout=no)
dnl if test x$have_shout = xyes; then
dnl    SHOUT_DIR=""
dnl    AC_MSG_RESULT([yes, libshout system wide library found])
dnl else
dnl    AC_MSG_RESULT([no, using static libshout])
dnl fi
dnl AC_SUBST(SHOUT_CFLAGS)
dnl AC_SUBST(SHOUT_LIBS)


dnl ==============================================================
dnl SDL_gfx
dnl ==============================================================
PKG_CHECK_MODULES(SDLGFX, SDL_gfx, have_sdlgfx=yes, have_sdlgfx=no)
if test x$have_sdlgfx = xno; then
   AC_MSG_NOTICE([Using freej version of SDLGFX])
   SDLGFX_LIBS="\$(top_builddir)/lib/sdl_gfx/libsdl_gfx.la"
   SDLGFX_CFLAGS="-I\$(top_srcdir)/lib/sdl_gfx"
fi
AC_SUBST(SDLGFX_CFLAGS)
AC_SUBST(SDLGFX_LIBS)

AM_CONDITIONAL([BUILD_SDLGFX], [test x$have_sdlgfx = xno])


dnl ==============================================================
dnl SDL_ttf
dnl ==============================================================
AC_CHECK_LIB(SDL_ttf, TTF_Init, have_sdlttf=yes, have_sdlttf=no)
if test x$have_sdlttf = xyes; then
   SDLTTF_CFLAGS="$SDL_CFLAGS"
   SDLTTF_LIBS="$SDL_LIBS -lSDL_ttf"
else
   SDLTTF_CFLAGS="-I\$(top_srcdir)/lib/sdl_ttf"
   SDLTTF_LIBS="\$(top_builddir)/lib/sdl_ttf/libsdl_ttf.la"
fi
AC_SUBST(SDLTTF_CFLAGS)
AC_SUBST(SDLTTF_LIBS)

AM_CONDITIONAL([BUILD_SDLTTF], [test x$have_sdlgfx = xno])


dnl ==============================================================
dnl SDL_image
dnl ==============================================================
AC_CHECK_LIB(SDL_image, IMG_Load, have_sdlimage=yes, have_sdlimage=no)
if test x$have_sdlimage = xyes; then
   SDLIMAGE_CFLAGS="$SDL_CFLAGS"
   SDLIMAGE_LIBS="$SDL_LIBS -lSDL_image"
else
   SDLIMAGE_CFLAGS="-I\$(top_srcdir)/lib/sdl_image"
   SDLIMAGE_LIBS="-lpng -ljpeg \$(top_builddir)/lib/sdl_image/libsdl_image.la"
fi
AC_SUBST(SDLIMAGE_CFLAGS)
AC_SUBST(SDLIMAGE_LIBS)

AM_CONDITIONAL([BUILD_SDLIMAGE], [test x$have_sdlgfx = xno])


dnl ==============================================================
dnl frei0r plugins
dnl ==============================================================
PKG_CHECK_MODULES(FREI0R, frei0r, have_frei0r=yes, have_frei0r=no)
if test x$have_frei0r = xno; then
   AC_MSG_NOTICE([Using freej version of FREI0R])
   FREI0R_CFLAGS="-I\$(top_srcdir)/lib/frei0r"
fi
AC_SUBST(FREIOR_CFLAGS)

AM_CONDITIONAL([BUILD_FREI0R],[test x$have_frei0r = xno])


dnl ==============================================================
dnl link with memory debugging library dmalloc
dnl ==============================================================
AC_CHECK_HEADERS([dmalloc.h], [have_dmalloc=yes], [have_dmalloc=no])
AC_ARG_WITH(dmalloc,
    AS_HELP_STRING([--with-dmalloc],[use dmalloc, as in ftp://ftp.letters.com/src/dmalloc/dmalloc.tar.gz]),
    [with_dmalloc=$withval],
    [with_dmalloc=no])

if test x$with_dmalloc = xyes; then
  if test x$have_dmalloc = xyes; then
      AC_DEFINE(WITH_DMALLOC,1,[Define if using the dmalloc debugging malloc package])
      LIBS="$LIBS -ldmallocthcxx"
  else
      AC_MSG_ERROR([*** dmalloc requested but header not found!])
  fi
fi

AC_PROG_PERL_MODULES([HTML::Template],
		 AM_CONDITIONAL(BUILD_JSAPIDOC, true),
		 AM_CONDITIONAL(BUILD_JSAPIDOC, false))

# select CCVT CONVERSION TYPE
CCVT_CONV="ccvt_c2.c"
AC_SUBST(CCVT_CONV)

CFLAGS="$GLOBAL_CFLAGS $CFLAGS"
AC_SUBST(CFLAGS)

dnl ==============================================================
dnl freej specific flags, explanation:
dnl STATIC_CFLAGS cflags regarding static libraries from lib/
dnl CFLAGS        ordinary cflags from shared libraries _plus static_cflags_
dnl STATIC_LIBS   libtool static libraries plus some optional ones
dnl LIBS          ordinary shared libraries used to link libfreej
dnl ==============================================================
FREEJ_STATIC_CFLAGS="-I\$(top_srcdir)/src/include \
    -I\$(top_srcdir)/lib/ccvt        \
dnl liblo needs both lib/ and lib/lo
    -I\$(top_srcdir)/lib             \
    -I\$(top_srcdir)/lib/lo          \
    -I\$(top_srcdir)/lib/slw         \
    \$(FLASH_CFLAGS)                 \
    \$(CWIID_CFLAGS)                 \
    \$(GOOM_CFLAGS)"
AC_SUBST(FREEJ_STATIC_CFLAGS)

FREEJ_CFLAGS="\$(FREEJ_STATIC_CFLAGS) \
    \$(SDL_CFLAGS)     \
    \$(FT2_CFLAGS)     \
    \$(FC_CFLAGS)      \
    \$(XIPH_CFLAGS)    \
    \$(UNICAP_CFLAGS)  \
    \$(OPENCV_CFLAGS)  \
    \$(FFMPEG_CFLAGS)  \
    \$(MOZJS_CFLAGS)   \
    \$(FREI0R_CFLAGS)  \
    \$(SHOUT_CFLAGS)   \
    \$(SDLGFX_CFLAGS)  \
    \$(SDLTTF_CFLAGS)  \
    \$(SDLIMAGE_CFLAGS)"
AC_SUBST(FREEJ_CFLAGS)

FREEJ_STATIC_LIBS="\$(top_builddir)/lib/ccvt/libccvt.la \
    \$(top_builddir)/lib/lo/liblo.la \
    \$(top_builddir)/lib/slw/libslw.la \
    \$(FLASH_LIBS) \
    \$(CWIID_LIBS) \
    \$(GOOM_LIBS)"
AC_SUBST(FREEJ_STATIC_LIBS)

FREEJ_LIBS="-lpthread -lm -lslang -lrt \
    \$(DL_LIBS)      \
    \$(X11_LIBS)     \
    \$(XIPH_LIBS)    \
    \$(MOZJS_LIBS)   \
    \$(OPENGL_LIBS)   \
    \$(SDL_LIBS)     \
    \$(FT2_LIBS)     \
    \$(FC_LIBS)      \
    \$(ALSA_LIBS)    \
    \$(JACK_LIBS)    \
    \$(FFTW_LIBS)    \
    \$(FFMPEG_LIBS)  \
    \$(X11_LIBS)     \
    \$(BLUEZ_LIBS)   \
    \$(UNICAP_LIBS)  \
    \$(OPENCV_LIBS)  \
    \$(SHOUT_LIBS)   \
    \$(SDLGFX_LIBS)  \
    \$(SDLTTF_LIBS)  \
    \$(SDLIMAGE_LIBS)"
AC_SUBST(FREEJ_LIBS)

dnl ###########################################################################
dnl ###### now about compile time paths
if test "x${prefix}" = "xNONE"; then
   prefix=${ac_default_prefix}
fi
AC_DEFINE_UNQUOTED(PACKAGE_PREFIX, "${prefix}", Prefix)
AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${prefix}/lib/freej", libs)
AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/freej", data)
AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${prefix}/share/locale", locale)
PACKAGE_DATA_DIR="${prefix}/share/freej"
AC_SUBST(PACKAGE_DATA_DIR)


AC_CONFIG_FILES([
Makefile
freej.pc
src/Makefile
src/include/Makefile
tests/Makefile
lib/Makefile
lib/ccvt/Makefile
lib/shout/Makefile
lib/lo/Makefile
lib/sdl_image/Makefile
lib/sdl_gfx/Makefile
lib/sdl_ttf/Makefile
lib/slw/Makefile
lib/flash/Makefile
lib/goom/Makefile
lib/cwiid/Makefile
lib/frei0r/Makefile
lib/frei0r/filter/Makefile
lib/frei0r/filter/3dflippo/Makefile
lib/frei0r/filter/brightness/Makefile
lib/frei0r/filter/bw0r/Makefile
lib/frei0r/filter/contrast0r/Makefile
lib/frei0r/filter/delay0r/Makefile
lib/frei0r/filter/distort0r/Makefile
lib/frei0r/filter/equaliz0r/Makefile
lib/frei0r/filter/flippo/Makefile
lib/frei0r/filter/gamma/Makefile
lib/frei0r/filter/hueshift0r/Makefile
lib/frei0r/filter/invert0r/Makefile
lib/frei0r/filter/mask0mate/Makefile
lib/frei0r/filter/nosync0r/Makefile
lib/frei0r/filter/pixeliz0r/Makefile
lib/frei0r/filter/saturat0r/Makefile
lib/frei0r/filter/scanline0r/Makefile
lib/frei0r/filter/sobel/Makefile
lib/frei0r/filter/squareblur/Makefile
lib/frei0r/filter/tehroxx0r/Makefile
lib/frei0r/filter/threshold0r/Makefile
lib/frei0r/filter/twolay0r/Makefile
lib/frei0r/filter/glow/Makefile
lib/frei0r/filter/water/Makefile
lib/frei0r/filter/cartoon/Makefile
lib/frei0r/filter/rotozoom/Makefile
lib/frei0r/filter/vertigo/Makefile
lib/frei0r/generator/Makefile
lib/frei0r/generator/ising0r/Makefile
lib/frei0r/generator/lissajous0r/Makefile
lib/frei0r/generator/nois0r/Makefile
lib/frei0r/generator/onecol0r/Makefile
lib/frei0r/generator/partik0l/Makefile
bindings/Makefile
bindings/python/Makefile
bindings/ruby/Makefile
bindings/java/Makefile
bindings/perl/Makefile
scripts/Makefile
doc/Makefile
doc/scripting/Makefile
doc/freej.dox
])
AC_OUTPUT

echo
echo
echo "Compile $PACKAGE $VERSION for ${host}"
echo "= SDL"
echo "  LIBS  : $SDL_LIBS"
echo "  CFLAGS: $SDL_CFLAGS"

if test x$have_x11 = xyes; then
echo "= X11"
echo "  LIBS  : $X11_LIBS"
else
echo "= X11 disabled"
fi

if test x$enable_opengl = xyes; then
echo "= OpenGL"
echo "  LIBS  : $OPENGL_LIBS"
else
echo "= OpenGL disabled"
fi

echo "= Language bindings"
if test x$enable_python = xyes; then
echo "  Python LIBS   : $PYTHON_LDFLAGS"
echo "  Python CFLAGS : $PYTHON_CPPFLAGS"
else
echo "  Python disabled"
fi
if test x$enable_ruby = xyes; then
echo "  Ruby LIBS     : $RUBY_LDFLAGS"
echo "  Ruby CFLAGS   : $RUBY_CPPFLAGS"
else
echo "  Ruby disabled"
fi
if test x$enable_java = xyes; then
echo "  Java LIBS     : $JAVA_LDFLAGS"
echo "  Java CFLAGS   : $JAVA_CFLAGS"
else
echo "  Java disabled"
fi
if test x$enable_perl = xyes; then
echo "  Perl LIBS     : $PERL_LDFLAGS"
echo "  Perl CFLAGS   : $PERL_CFLAGS"
else
echo "  Perl disabled"
fi

if test x$have_freetype2 = xyes; then
echo "= FreeType2"
echo "  LIBS  : $FT2_LIBS"
echo "  CFLAGS: $FT2_CFLAGS"
else
echo "= FreeType2 disabled"
fi

if test x$have_fontconfig = xyes; then
echo "= Fontconfig"
echo "  LIBS  : $FC_LIBS"
echo "  CFLAGS: $FC_CFLAGS"
else
echo "= Fontconfig disabled"
fi

echo "= ffmpeg (movie layer)"
echo "  CFLAGS: $FFMPEG_CFLAGS"
echo "  LIBS  : $FFMPEG_LIBS"

echo "= ogg-theora (xiph.org)"
echo "  LIBS  : $XIPH_LIBS"
echo "  CFLAGS: $XIPH_CFLAGS"

echo "= javascript interpreter"
echo "  LIBS  : $MOZJS_LIBS"
echo "  CFLAGS: $MOZJS_CFLAGS"

if test x$have_unicap = xyes; then
echo "= unicap library"
echo "  LIBS  : $UNICAP_LIBS"
echo "  CFLAGS: $UNICAP_CFLAGS"
else
echo "= unicap library disabled"
fi

if test x$have_sdlgfx = xyes; then
echo "= SDL_gfx (dynamic)"
else
echo "= SDL_gfx (static)"
fi
echo "  LIBS  : $SDLGFX_LIBS"
echo "  CFLAGS: $SDLGFX_CFLAGS"

if test x$have_sdlttf = xyes; then
echo "= SDL_ttf (dynamic)"
else
echo "= SDL_ttf (static)"
fi
echo "  LIBS  : $SDLTTF_LIBS"
echo "  CFLAGS: $SDLTTF_CFLAGS"

if test x$have_sdlimage = xyes; then
echo "= SDL_image (dynamic)"
else
echo "= SDL_image (static)"
fi
echo "  LIBS  : $SDLIMAGE_LIBS"
echo "  CFLAGS: $SDLIMAGE_CFLAGS"

if test x$have_frei0r = xyes; then
echo "= frei0r plugins (system provided)"
else
echo "= frei0r plugins (freej provided)"
fi
if test x$have_flash = xyes; then
echo "= Flash (ver.3 animations only)"
echo "  LIBS  : $FLASH_LIBS (static)"
else
echo "= Flash (ver.3 animations only) disabled"
fi
if test x$enable_cpuflags = xyes; then
echo "= Building binary with support for machine specific advanced instructions"
else
echo "= Not building binary with support for machine specific advanced instructions"
fi
if test x$enable_lubrifications = xyes; then
echo "= Building machine specific lubrificated binary"
else
echo "= Not building machine specific lubrificated binary"
fi
if test x$enable_debug = xyes; then
echo "= Compiling with debugging symbols"
else
echo "= Not compiling with debugging symbols"
fi
if test x$enable_profiling = xyes; then
echo "= Including support for the GNU Profiler (see man gprof)"
else
echo "= Not including support for the GNU Profiler (see man gprof)"
fi

if test x$have_sound = xyes; then
   echo "= Sound bus enabled"
   if test x$have_alsa = xyes; then
      echo "  device libraries: $ALSA_LIBS"
      echo "  midi controller: alsa"
   fi
   if test x$have_jack = xyes; then
      echo "  input connection $JACK_LIBS"
   fi
   if test x$have_fftw = xyes; then
      echo "  harmonics analisys: $FFTW_LIBS"
   fi
   if test x$enable_goom = xyes; then
      echo "  goom: yes"
   else
      echo "  goom: no"
   fi
else
   echo "= Sound bus disabled"
fi

echo "= COMPILER FLAGS : $CFLAGS"
echo "= LINKER FLAGS   : $LDFLAGS"


echo "= INSTALL PREFIX : $prefix"
echo "============================== now type make, may the source be with you!"
echo

