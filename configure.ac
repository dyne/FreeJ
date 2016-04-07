dnl ==============================================================
dnl Process this file with autoconf to produce a configure script.
dnl ==============================================================

dnl require autoconf 2.62 (AS_ECHO/AS_ECHO_N)
AC_PREREQ([2.62])

AC_INIT([FreeJ],[1.0],[freej@lists.dyne.org],[freej])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

dnl backwards compatibility for autoconf >= 2.64
dnl PACKAGE_URL should be the fifth argument of AC_INIT
m4_define([AC_PACKAGE_URL], [http://freej.dyne.org])
AC_DEFINE(PACKAGE_URL, "AC_PACKAGE_URL", [Package URL])
AC_SUBST(PACKAGE_URL, AC_PACKAGE_URL)

dnl ==============================================================
dnl Get the operating system and version number...
dnl ==============================================================
AC_MSG_CHECKING([for which platform we are compiling])
case "$host_cpu" in
  powerpc*)
    have_ppc=yes
  ;;
  x86_64*)
    have_x86_64=yes
  ;;
  i*86*)
    have_x86_32=yes
  ;;
esac

case "$host_os" in
  *linux*)
    AC_MSG_RESULT([Linux])
    AC_DEFINE(HAVE_LINUX,1,[define if compiling for Linux])
    DL_LIBS="-ldl"
    have_linux=yes

    if test x$have_ppc = xyes; then
        AC_DEFINE(ARCH_PPC,1,[define if compiling for Linux/PPC])
        AC_DEFINE(ARCH_POWERPC,1,[define if compiling for Linux/PPC])
        dnl TODO(godog) what about ppc64?
        AC_DEFINE(POWERPC_MODE_32BITS,1,[define if compiling for PPC])
        AC_DEFINE(HAVE_ALTIVEC,1,[define if cpu supports Altivec instruction set])
        AC_DEFINE(HAVE_ALTIVEC_H,1,[define if cpu supports Altivec instruction set])
        AC_DEFINE(WORDS_BIGENDIAN,1,[define if compiling for Big Endian CPU])
        AC_DEFINE(PA_BIG_ENDIAN,1,[define big endian arch for portaudio])
dnl        AC_DEFINE(SDL_BYTEORDER,SDL_BIG_ENDIAN,[define big endian arch for SDL])
    fi

    if test x$have_x86_64 = xyes; then
        AC_DEFINE(ARCH_X86,1,[define if compiling for Linux/x86])
        AC_DEFINE(HAVE_64BIT,1,[define if host has 64 bit])
    fi

    if test x$have_x86_32 = xyes; then
        AC_DEFINE(ARCH_X86,1,[define if compiling for Linux/x86])
        AC_DEFINE(PA_LITTLE_ENDIAN,1,[define little endian arch for portaudio])
dnl        AC_DEFINE(SDL_BYTEORDER,SDL_LIL_ENDIAN,[define little endian arch for SDL])
    fi
  ;;

  *darwin*)
    AC_MSG_RESULT([Darwin/OSX])
    AC_DEFINE(HAVE_DARWIN,1,[define if compiling for Apple Darwin OSX])
    have_darwin=yes
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


dnl ==============================================================
dnl Setup for automake
dnl ==============================================================

AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/freej.cpp])
AC_CONFIG_HEADERS([config.h])

dnl never run autotools and configure automatically
dnl AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_AS
dnl compatibility with older libtool
dnl m4_ifndef([LT_INIT], [AC_PROG_RANLIB])


dnl Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h memory.h stdint.h stdlib.h \
  string.h strings.h sys/file.h sys/ioctl.h sys/time.h termios.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
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

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero floor gettimeofday memmove memset munmap rint select \
  setenv strcasecmp strdup strerror strncasecmp strrchr strstr])

dnl compatibility with older libtool
m4_ifdef([LT_INIT],
  [LT_INIT([dlopen])],
  [AC_LIBTOOL_DLOPEN
   AC_PROG_LIBTOOL])

dnl versioning info for libtool
FREEJ_CURRENT=0
FREEJ_REVISION=0
FREEJ_AGE=0
FREEJ_VERSION_INFO="$FREEJ_CURRENT:$FREEJ_REVISION:$FREEJ_AGE"
dnl Note this is the ABI version which is not the same as our actual
dnl library version
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

AC_ARG_ENABLE(python,
    AS_HELP_STRING([--enable-python],[enable Python bindings (no)]),
	[enable_python=$enableval],
	[enable_python=no])

    if test x"$enable_python" = xyes; then
	AC_PYTHON_DEVEL
	AM_PATH_PYTHON([$ac_python_version])
	AC_SUBST([SWIG_PYTHON_OPT],[-python$swig_shadow])
	AC_SUBST([SWIG_PYTHON_CPPFLAGS],[$PYTHON_CPPFLAGS])

    fi


#    ENABLE_SWIG_PYTHON
    ENABLE_SWIG_RUBY
    ENABLE_SWIG_JAVA
    ENABLE_SWIG_PERL

    if test x$enable_perl = xyes; then
      AC_CHECK_FUNCS([strtoul])
      AC_FUNC_STRTOD
    fi
fi

AM_CONDITIONAL([BUILD_SWIG_PYTHON], [test x$enable_python = xyes])
AM_CONDITIONAL([BUILD_SWIG_RUBY], [test x$enable_ruby = xyes])
AM_CONDITIONAL([BUILD_SWIG_JAVA], [test x$enable_java = xyes])
AM_CONDITIONAL([BUILD_SWIG_PERL], [test x$enable_perl = xyes])

dnl swig not found but bindings requested is fatal
if test x$have_swig = xno && {
      test x$enable_python = xyes ||
      test x$enable_java = xyes   ||
      test x$enable_ruby = xyes   ||
      test x$enable_perl = xyes
   }; then
    AC_MSG_ERROR([*** bindings requested but swig not found!])
fi

if test x$enable_python = xyes; then
   AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen])
   if test -z "$DOXYGEN"; then
      AC_MSG_ERROR([*** Doxygen is required to build python docstrings.])
   fi
fi

dnl ==============================================================
dnl Add the local include path and some flags
dnl ==============================================================
dnl -freg-struct-return is to compile an host compatible with freeframe dso
GLOBAL_CFLAGS="$GLOBAL_CFLAGS -pipe -D_REENTRANT -freg-struct-return -fPIC"
if test x$have_darwin = xyes; then
  dnl add the fink default path to find headers
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -I/sw/include"
fi

dnl ==============================================================
dnl Check for slang
dnl ==============================================================
PKG_CHECK_MODULES([SLANG], [slang], [have_slang=yes], [have_slang=no])
if test x$have_slang = xno; then
   SLANG_CFLAGS="-I/usr/include/slang"
   SLANG_LIBS="-lslang"
   FREEJ_SAVE_FLAGS
   CFLAGS="$SLANG_CFLAGS"
   CPPFLAGS="$SLANG_CFLAGS"
   LDFLAGS=""
   FREEJ_CHECK_LIB_HEADER([slang], [SLang_getkey], [slang.h],
                           [have_slang=yes], [have_slang=no])
   FREEJ_RESTORE_FLAGS
fi
if test x$have_slang = xno; then
   AC_MSG_ERROR([*** SLANG development files not found!])
fi

dnl Check for GD
GD_LIBS=""
FREEJ_CHECK_LIB_HEADER([gd], [gdImageCreateTrueColor], [gd.h],
                       [have_gd=yes], [have_gd=no])
if test x$have_gd = xyes; then
   GD_LIBS="-lgd"
   AC_DEFINE(WITH_GD,1,[define if compiling with GD to support screenshot taking])
   AC_SUBST([GD_LIBS])
fi

dnl ==============================================================
dnl Check for SDL
dnl ==============================================================
PKG_CHECK_MODULES(SDL, sdl > 1.2, :,
  AC_MSG_ERROR([*** SDL development files not found!]))


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

dnl TODO(shammash): heritage from conditional build, still used somewhere.
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

dnl FFmpeg changed include path policy, let's check which one is in use.
FREEJ_SAVE_FLAGS
CPPFLAGS="$CPPFLAGS $FFMPEG_CFLAGS"
CFLAGS="$CFLAGS $FFMPEG_CFLAGS"
AC_CHECK_HEADERS(libavcodec/avcodec.h ffmpeg/avcodec.h)
AC_CHECK_HEADERS(libavformat/avformat.h ffmpeg/avformat.h)
AC_CHECK_HEADERS(libavutil/avutil.h ffmpeg/avutil.h)
AC_CHECK_HEADERS(libswscale/swscale.h ffmpeg/swscale.h)
FREEJ_RESTORE_FLAGS
dnl Then in sources we have to include in the following way:
dnl    #ifdef HAVE_LIBAVCODEC_AVCODEC_H
dnl    #   include <libavcodec/avcodec.h>
dnl    #elif defined(HAVE_FFMPEG_AVCODEC_H)
dnl    #   include <ffmpeg/avcodec.h>
dnl    #else
dnl    #   include <avcodec.h>
dnl    #endif

dnl TODO(shammash): heritage from conditional build, still used somewhere.
AC_DEFINE(WITH_FFMPEG,1, [define if compiling movie layer linking to ffmpeg libavcodec])
AC_DEFINE(WITH_SWSCALE,1, [define if using new swscale library from ffmpeg])


dnl ==============================================================
dnl LIBLO
dnl ==============================================================
PKG_CHECK_MODULES([LIBLO], [liblo], [:],
  AC_MSG_ERROR([*** Liblo development files not found!]))


dnl ==============================================================
dnl shout lib
dnl ==============================================================
PKG_CHECK_MODULES([SHOUT], [shout], [:],
  AC_MSG_ERROR([*** Libshout development files not found!]))


dnl ==============================================================
dnl Font handling (text layer)
dnl ==============================================================
PKG_CHECK_MODULES(FT2, freetype2,
                  have_freetype2=yes, have_freetype2=no)
PKG_CHECK_MODULES(FC, fontconfig,
                  have_fontconfig=yes, have_fontconfig=no)
if test x$have_freetype2 = xyes && test x$have_fontconfig = xyes; then
   have_textlayer=yes
   AC_DEFINE(WITH_TEXTLAYER,1,[define if having fontconfig and freetype])
fi


dnl ==============================================================
dnl Check X11 for xgrab
dnl ==============================================================
PKG_CHECK_MODULES(X11, x11, have_xgrab=yes, have_xgrab=no)
if test x$have_xgrab = xyes; then
   AC_DEFINE(WITH_XGRAB,1,[define if using xgrab layer])
   AC_DEFINE(WITH_XSCREENSAVER,1,[define if using xscreensaver layer])
fi


dnl ==============================================================
dnl ENABLE LIBFLASH
dnl ==============================================================
AC_ARG_ENABLE(flash,
    AS_HELP_STRING([--enable-flash],[compile with Flash animation layer (yes)]),
    [have_flash=$enableval],
    [have_flash=no])
AC_MSG_CHECKING([flash - v3 animation only - layer])
if test x$have_flash = xyes; then
    AC_MSG_RESULT(yes)
    AC_CHECK_FUNCS([sqrt])
    AC_DEFINE(WITH_FLASH,1,[define if compiling flash layer])
    FLASH_LIBS="\$(top_builddir)/lib/flash/libflash.la"
    FLASH_CFLAGS="-I\$(top_srcdir)/lib/flash"
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(BUILD_FLASH, [test x$have_flash = xyes])
AC_SUBST(FLASH_LIBS)
AC_SUBST(FLASH_CFLAGS)


dnl ==============================================================
dnl CHECK if there is CWIID
dnl ==============================================================
PKG_CHECK_MODULES(CWIID, cwiid, have_cwiid=yes, have_cwiid=no)
if test x$have_cwiid = xno; then
   FREEJ_CHECK_LIB_HEADER([cwiid], [cwiid_open], [cwiid.h],
                           [have_cwiid=yes], [have_cwiid=no])
fi
if test x$have_cwiid = xyes; then
   CWIID_CFLAGS=""
   CWIID_LIBS="-lcwiid"
   AC_DEFINE(WITH_CWIID,1,[define if compiling cwiid WiiMote controller])
fi


dnl ==============================================================
dnl CHECK if there is VIDEO4LINUX
dnl ==============================================================
AC_CHECK_HEADERS([linux/videodev2.h], [have_videodev=yes], [have_videodev=no])
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
         dnl XXX(godog) is this MMX or SSE?
         have_mmx=yes
         AC_DEFINE(HAVE_MMX,1,[define if enabling MMX acceleration])
     fi
  fi
fi

AC_ARG_ENABLE(lubrication,
    AS_HELP_STRING([--enable-lubrication],[compile using processor lubricants (yes)]),
    [enable_lubrication=$enableval],
    [enable_lubrication=yes])

dnl XXX(godog) integrate with AX_GCC_ARCHFLAG
if test x$enable_lubrication = xyes; then
  if test x$enable_profiling = xyes || test x$enable_debug = xyes; then
    AC_MSG_WARN([cannot lubricate code if debug or profiling are enabled])
    enable_lubrication=no
  else
    if test x$have_darwin = xyes; then
      if test x$have_ppc = xyes; then
        dnl use optimization flags for darwin
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mabi=altivec -maltivec -mpowerpc-gfxopt"
      else
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mfpmath=sse -mtune=pentium4"
      fi
    dnl adding these provoke 'illegal instruction' errors in some
    dnl ppc/gcc combinations: -mpowerpc-gfxopt -faltivec"
    elif test x$have_ppc = xyes; then
      GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -fomit-frame-pointer -ffast-math -mabi=altivec -maltivec -mpowerpc-gfxopt"
    elif test x$have_x86_64 = xyes; then
      dnl should complete this with all 64bit CPUs (AMD too)
      if test x$have_ssse3 = xyes; then
        dnl multiple core cpu
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mfpmath=sse -mtune=core2"
      else
        dnl single core 64bit cpu
        GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O2 -mfpmath=sse -mtune=nocona"
      fi
    else
      dnl any other platform optimizations
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
dnl CHECK if we have aalib
dnl ==============================================================
FREEJ_CHECK_LIB_HEADER([aa], [aa_init], [aalib.h], [have_aa=yes], [have_aa=no])
if test x$have_aa = xyes; then
   AC_DEFINE(WITH_AALIB,1,[define if to compile aalib ascii screen])
   AA_CFLAGS=""
   AA_LIBS="-laa"
fi
AC_SUBST(AA_CFLAGS)
AC_SUBST(AA_LIBS)



dnl ==============================================================
dnl CHECK TO USE OPENGL VIDEO OUTPUT
dnl ==============================================================
AC_ARG_ENABLE(opengl,
    AS_HELP_STRING([--enable-opengl],[experimental opengl rendering (no)]),
    [enable_opengl=$enableval],
    [enable_opengl=no])

if test x$enable_opengl = xyes; then
   dnl note: checking for GLU only, this brings in also GL
   PKG_CHECK_MODULES(GLU, glu,
                     have_glu=yes, have_glu=no)

   dnl pkg-config failed, try headers
   if test x$have_glu = xno; then
      FREEJ_CHECK_LIB_HEADER([GLU], [gluNewQuadric], [GL/glu.h],
                             [have_glu=yes], [have_glu=no])
      if test x$have_glu = xyes; then
         GLU_LIBS="-lGL -lGLU"
         GLU_CFLAGS=""
      fi
   fi

   if test x$have_glu = xno; then
      AC_MSG_ERROR([*** opengl requested but not found!])
   else
      AC_DEFINE(WITH_OPENGL,1,[define if using experimental opengl rendering])
   fi
fi


dnl ==============================================================
dnl Audio
dnl ==============================================================
PKG_CHECK_MODULES(ALSA, alsa,
    AC_DEFINE(WITH_MIDI,1,[define if compiling midi controller])
    have_midi=yes,
    have_midi=no)

dnl TODO(shammash): before this refactoring we were enabling jack
dnl only if alsa was present, check inconsistencies in the code.
PKG_CHECK_MODULES(JACK, jack, :,
  AC_MSG_ERROR([*** Jack development files not found!]))

PKG_CHECK_MODULES(FFTW, fftw3, :,
  AC_MSG_ERROR([*** Fftw3 development files not found!]))

PKG_CHECK_MODULES(SAMPLERATE, samplerate, :,
  AC_MSG_ERROR([*** Samplerate development files not found!]))

AC_DEFINE(WITH_AUDIO,1,[define if compiling audio system])

dnl ==============================================================
dnl compile including the javascript interpreter
dnl ==============================================================

AC_MSG_NOTICE([checking if mozilla-js is ok])
PKG_CHECK_MODULES([MOZJS_JS],
                  [mozilla-js >= 1.9],
                  [have_mozjs_js=yes], [have_mozjs_js=no])
if test x$have_mozjs_js = xyes; then
  FREEJ_SAVE_FLAGS
  CFLAGS="$MOZJS_JS_CFLAGS"
  CPPFLAGS="$MOZJS_JS_CFLAGS"
  LDFLAGS="$MOZJS_JS_LIBS"
  FREEJ_CHECK_LIB_HEADER([mozjs], [JS_NewContext], [jsapi.h],
                          [have_mozjs_js=yes], [have_mozjs_js=no])
  FREEJ_RESTORE_FLAGS
fi
if test x$have_mozjs_js = xyes; then
  AC_MSG_NOTICE([using mozilla-js])
  MOZJS_CFLAGS="$MOZJS_JS_CFLAGS"
  MOZJS_LIBS="$MOZJS_JS_LIBS"
  have_mozjs=yes
else
  AC_MSG_NOTICE([NOT using mozilla-js])
  have_mozjs=no
fi

if test x$have_mozjs = xno; then
  AC_MSG_NOTICE([checking if mozilla-js && libxul-embedding-stable are ok])
  dnl test if https://bugzilla.mozilla.org/show_bug.cgi?id=500645 has been closed
  PKG_CHECK_MODULES([MOZJS_STABLE],
                    [mozilla-js >= 1.9 libxul-embedding-stable >= 1.9],
                    [have_mozjs_stable=yes], [have_mozjs_stable=no])
  if test x$have_mozjs_stable = xyes; then
    FREEJ_SAVE_FLAGS
    CFLAGS="$MOZJS_STABLE_CFLAGS"
    CPPFLAGS="$MOZJS_STABLE_CFLAGS"
    LDFLAGS="$MOZJS_STABLE_LIBS"
    FREEJ_CHECK_LIB_HEADER([mozjs], [JS_NewContext], [jsapi.h],
                            [have_mozjs_stable=yes], [have_mozjs_stable=no])
    FREEJ_RESTORE_FLAGS
  fi
  if test x$have_mozjs_stable = xyes; then
    AC_MSG_NOTICE([using mozilla-js && libxul-embedding-stable])
    MOZJS_CFLAGS="$MOZJS_STABLE_CFLAGS"
    MOZJS_LIBS="$MOZJS_STABLE_LIBS"
    have_mozjs=yes
  else
    AC_MSG_NOTICE([NOT using mozilla-js && libxul-embedding-stable])
    have_mozjs=no
  fi
fi

if test x$have_mozjs = xno; then
  dnl test if adding unstable is enough
  AC_MSG_NOTICE([checking if mozilla-js && libxul-embedding-unstable are ok])
  PKG_CHECK_MODULES([MOZJS_UNSTABLE],
                    [mozilla-js >= 1.9 libxul-embedding-unstable >= 1.9],
                    [have_mozjs_unstable=yes], [have_mozjs_unstable=no])
  if test x$have_mozjs_unstable = xyes; then
    FREEJ_SAVE_FLAGS
    CFLAGS="$MOZJS_UNSTABLE_CFLAGS"
    CPPFLAGS="$MOZJS_UNSTABLE_CFLAGS"
    LDFLAGS="$MOZJS_UNSTABLE_LIBS"
    FREEJ_CHECK_LIB_HEADER([mozjs], [JS_NewContext], [jsapi.h],
                            [have_mozjs_unstable=yes], [have_mozjs_unstable=no])
    FREEJ_RESTORE_FLAGS
  fi
  if test x$have_mozjs_unstable = xyes; then
    AC_MSG_NOTICE([using mozilla-js && libxul-embedding-unstable])
    MOZJS_CFLAGS="$MOZJS_UNSTABLE_CFLAGS"
    MOZJS_LIBS="$MOZJS_UNSTABLE_LIBS"
    have_mozjs=yes
  else
    AC_MSG_NOTICE([NOT using mozilla-js && libxul-embedding-unstable])
    have_mozjs=no
  fi
fi

if test x$have_mozjs = xyes; then
  AC_DEFINE(JS_THREADSAFE,1,[define if compiling with threadsafe libjs])
  AC_MSG_CHECKING([if libmozjs is in linker search path])

  FREEJ_SAVE_FLAGS
  CFLAGS="$MOZJS_CFLAGS"
  CPPFLAGS="$MOZJS_CFLAGS"
  dnl strip -L<arg> from MOZJS_LIBS
  LDFLAGS=`echo $MOZJS_LIBS | sed 's/-L \?[^ ]*//g'`
  AC_RUN_IFELSE([
#include <jsapi.h>
int main() {
 JSRuntime *jsr = JS_NewRuntime(8L * 1024L * 1024L);
return 0;
}],
  [have_mozjs_linkable=yes], [have_mozjs_linkable=no],
  [have_mozjs_linkable=no]) dnl action if cross-compiling
  FREEJ_RESTORE_FLAGS

  if test x$have_mozjs_linkable = xno; then
    AC_MSG_RESULT([no, compiling static version])
    have_mozjs=no
  else
    AC_MSG_RESULT([yes])
  fi
fi

dnl this flag is supposed to be for users having problem with xulrunner
dnl installed on the system and want to force a static linking with our shipped
dnl xulrunner
AC_ARG_ENABLE(static-mozjs,
    AS_HELP_STRING([--enable-static-mozjs],[force use of shipped mozjs]),
    [static_mozjs=$enableval],
    [static_mozjs=no])
if test x$have_mozjs = xno || test x$static_mozjs = xyes; then
  AC_MSG_NOTICE([fallback to static mozjs])
  MOZJS_CFLAGS="-I\$(top_srcdir)/lib/javascript -I\$(top_builddir)/lib/javascript"
  dnl libs in top_srcdir instead of top_builddir because js is not under autotools
  MOZJS_LIBS="\$(top_builddir)/lib/javascript/libmozjs.a"

  dnl run lib/javascript/configure after freej's configure, building it static
  dnl XXX config.cache can be shared between the two configures? is it safe?
  AC_CONFIG_COMMANDS([lib/javascript/.xulrunner-subdir],
                     [
    (cd lib/javascript &&
      CXXFLAGS="$GLOBAL_CFLAGS $CXXFLAGS -fPIC" \
      CFLAGS="$GLOBAL_CFLAGS $CFLAGS -fPIC" \
      $ac_srcdir/configure --enable-static \
        --enable-js-static-build --disable-jit) || exit $?
  ])
  have_mozjs=no
fi

if test x$enable_debug = xyes ; then
   MOZJS_CFLAGS="$MOZJS_CFLAGS -DJS_GCMETER"
fi

# I don't think this is needed (shammash)
#if test x$have_freebsd = xyes; then
#   MOZJS_CFLAGS="-I/usr/local/include -L/usr/local/lib -I../lib/javascript"
#   MOZJS_LIBS="-ljs"
#   AC_DEFINE(XP_UNIX,1,[Define use of UNIX types in javascript])
#fi

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
AC_ARG_ENABLE(unicap,
    AS_HELP_STRING([--enable-unicap],[enable use of unicap library for video capture]),
 [
    PKG_CHECK_MODULES(UNICAP, libunicap, have_unicap=yes, have_unicap=no)
    if test x$have_unicap = xyes; then
    AC_DEFINE(WITH_UNICAP,1,[Define if to link libunicap for camera capture])
    fi
 ])

dnl ==============================================================
dnl opencv lib
dnl ==============================================================
AC_ARG_ENABLE(opencv,
    AS_HELP_STRING([--enable-opencv],[enable use of opencv library for video capture]),
 [
    PKG_CHECK_MODULES(OPENCV, opencv, have_opencv=yes, have_opencv=no)
    if test x$have_opencv = xyes; then
    AC_DEFINE(WITH_OPENCV,1,[Define if to link opencv framework])
    fi
 ])

dnl ==============================================================
dnl SDL_gfx
dnl ==============================================================
PKG_CHECK_MODULES(SDLGFX, SDL_gfx, have_sdlgfx=yes, have_sdlgfx=no)
if test x$have_sdlgfx = xno; then
   SDLGFX_CFLAGS="$SDL_CFLAGS"
   SDLGFX_LIBS="$SDL_LIBS -lSDL_gfx"
   FREEJ_SAVE_FLAGS
   CFLAGS="$SDLGFX_CFLAGS"
   CPPFLAGS="$SDLGFX_CFLAGS"
   LDFLAGS="$SDLGFX_LIBS"
   FREEJ_CHECK_LIB_HEADER([SDL_gfx], [pixelColor], [SDL_gfxPrimitives.h],
                           [have_sdlgfx=yes], [have_sdlgfx=no])
   FREEJ_RESTORE_FLAGS
fi
if test x$have_sdlgfx = xno; then
   AC_MSG_NOTICE([Using freej version of SDLGFX])

   AC_CHECK_FUNCS([pow])
   SDLGFX_LIBS="\$(top_builddir)/lib/sdl_gfx/libsdl_gfx.la"
   SDLGFX_CFLAGS="-I\$(top_srcdir)/lib/sdl_gfx"
fi

AM_CONDITIONAL([BUILD_SDLGFX], [test x$have_sdlgfx = xno])


dnl ==============================================================
dnl SDL_ttf
dnl ==============================================================
PKG_CHECK_MODULES([SDLTTF], [SDL_ttf], [have_sdlttf=yes], [have_sdlttf=no])
if test x$have_sdlttf = xno; then
   SDLTTF_CFLAGS="$SDL_CFLAGS"
   SDLTTF_LIBS="$SDL_LIBS -lSDL_ttf"
   FREEJ_SAVE_FLAGS
   CFLAGS="$SDLTTF_CFLAGS"
   CPPFLAGS="$SDLTTF_CFLAGS"
   LDFLAGS="$SDLTTF_LIBS"
   FREEJ_CHECK_LIB_HEADER([SDL_ttf], [TTF_Init], [SDL_ttf.h],
                           [have_sdlttf=yes], [have_sdlttf=no])
   FREEJ_RESTORE_FLAGS
fi
if test x$have_sdlttf = xno; then
   AC_MSG_NOTICE([Using freej version of SDLTTF])
   SDLTTF_CFLAGS="-I\$(top_srcdir)/lib/sdl_ttf"
   SDLTTF_LIBS="\$(top_builddir)/lib/sdl_ttf/libsdl_ttf.la"
fi

AM_CONDITIONAL([BUILD_SDLTTF], [test x$have_sdlttf = xno])


dnl ==============================================================
dnl SDL_image
dnl ==============================================================
PKG_CHECK_MODULES([SDLIMAGE], [SDL_image],
                  [have_sdlimage=yes],
                  [have_sdlimage=no])
if test x$have_sdlimage = xno; then
   SDLIMAGE_CFLAGS="$SDL_CFLAGS"
   SDLIMAGE_LIBS="$SDL_LIBS -lSDL_image -ljpeg"
   FREEJ_SAVE_FLAGS
   CFLAGS="$SDLIMAGE_CFLAGS"
   CPPFLAGS="$SDLIMAGE_CFLAGS"
   LDFLAGS="$SDLIMAGE_LIBS"
   FREEJ_CHECK_LIB_HEADER([SDL_image], [IMG_Load], [SDL_image.h],
                           [have_sdlimage=yes], [have_sdlimage=no])
   FREEJ_RESTORE_FLAGS
fi
if test x$have_sdlimage = xno; then
   FREEJ_CHECK_LIB_HEADER([jpeg], [jpeg_std_error], [jpeglib.h],
                           [have_libjpeg=yes], [have_libjpeg=no])
   if test x$have_libjpeg = xno; then
      AC_MSG_ERROR([*** Libjpeg development files are required to build static SDL_image])
   fi
   PKG_CHECK_MODULES(LIBPNG, libpng, have_libpng=yes, have_libpng=no)
   if test x$have_libpng = xno; then
      AC_MSG_ERROR([*** Libpng development files are required to build static SDL_image])
   fi

   SDLIMAGE_CFLAGS="-I\$(top_srcdir)/lib/sdl_image"
   SDLIMAGE_LIBS="-lpng -ljpeg \$(top_builddir)/lib/sdl_image/libsdl_image.la"
fi

AM_CONDITIONAL([BUILD_SDLIMAGE], [test x$have_sdlimage = xno])


dnl ==============================================================
dnl frei0r plugins
dnl ==============================================================
#
# frei0r should be supported in any case:
# plugins will be detected runtime
# frei0r.h header is redistributed (see LiViDO/frei0r spec.)
# this is a normal behaviour for plugins
# support should be compiled in even if plugins aren't present.
# -jrml
#
#PKG_CHECK_MODULES(FREI0R, frei0r, have_frei0r=yes, have_frei0r=no)
#if test x$have_frei0r = xyes; then
#   AC_DEFINE(WITH_FREI0R,1,[Define if using frei0r plugins])
#fi
AC_DEFINE(WITH_FREI0R,1,[Define as using frei0r plugins])


dnl ==============================================================
dnl freeframe plugins
dnl ==============================================================
#
# freeframe should be supported in any case:
# plugins will be detected runtime
# freeframe.h header is redistributed
# this is a normal behaviour for plugins
# support should be compiled in even if plugins aren't present.
# -xnt
#
AC_DEFINE(WITH_FREEFRAME,1,[Define as using freeframe plugins])

dnl ==============================================================
dnl Check Cairo 2d vector graphics library for VectorLayer
dnl ==============================================================
PKG_CHECK_MODULES(CAIRO, cairo, have_cairo=yes, have_cairo=no)
if test x$have_cairo = xyes; then
   AC_DEFINE(WITH_CAIRO,1,[define if using cairo vector library])
fi



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

dnl select CCVT CONVERSION TYPE
CCVT_CONV="ccvt_c2.c"
AC_SUBST(CCVT_CONV)

CFLAGS="$GLOBAL_CFLAGS $CFLAGS"
AC_SUBST(CFLAGS)

CXXFLAGS="$GLOBAL_CFLAGS $CXXFLAGS"
AC_SUBST(CXXFLAGS)

dnl ==============================================================
dnl freej specific flags, explanation:
dnl STATIC_CFLAGS cflags regarding static libraries from lib/
dnl CFLAGS        ordinary cflags from shared libraries _plus static_cflags_
dnl STATIC_LIBS   libtool static libraries plus some optional ones
dnl LIBS          ordinary shared libraries used to link libfreej
dnl ==============================================================
FREEJ_STATIC_CFLAGS="-I\$(top_srcdir)/src/include \
    -I\$(top_srcdir)/lib/ccvt        \
    -I\$(top_srcdir)/lib/slw         \
    \$(FLASH_CFLAGS)"
AC_SUBST(FREEJ_STATIC_CFLAGS)

FREEJ_CFLAGS="\$(FREEJ_STATIC_CFLAGS) \
    \$(AA_CFLAGS)       \
    \$(CWIID_CFLAGS)    \
    \$(FC_CFLAGS)       \
    \$(FFMPEG_CFLAGS)   \
    \$(FT2_CFLAGS)      \
    \$(GLU_CFLAGS)      \
    \$(LIBLO_CFLAGS)    \
    \$(MOZJS_CFLAGS)    \
    \$(OPENCV_CFLAGS)   \
    \$(QTGUI_CFLAGS)	\
    \$(SDL_CFLAGS)      \
    \$(SDLGFX_CFLAGS)   \
    \$(SDLIMAGE_CFLAGS) \
    \$(SDLTTF_CFLAGS)   \
    \$(SHOUT_CFLAGS)    \
    \$(SLANG_CFLAGS)    \
    \$(UNICAP_CFLAGS)   \
    \$(CAIRO_CFLAGS)    \
    \$(XIPH_CFLAGS)"
AC_SUBST(FREEJ_CFLAGS)

FREEJ_STATIC_LIBS="\$(top_builddir)/lib/ccvt/libccvt.la \
    \$(top_builddir)/lib/slw/libslw.la \
    \$(FLASH_LIBS)"
AC_SUBST(FREEJ_STATIC_LIBS)

FREEJ_LIBS="-lpthread -lm -lrt $DL_LIBS \
    \$(AA_LIBS)         \
    \$(ALSA_LIBS)       \
    \$(BLUEZ_LIBS)      \
    \$(CWIID_LIBS)      \
    \$(FC_LIBS)         \
    \$(FFMPEG_LIBS)     \
    \$(FFTW_LIBS)       \
    \$(FT2_LIBS)        \
    \$(GLU_LIBS)        \
    \$(JACK_LIBS)       \
    \$(LIBLO_LIBS)      \
    \$(MOZJS_LIBS)      \
    \$(OPENCV_LIBS)     \
    \$(QTGUI_LIBS)	\
    \$(SAMPLERATE_LIBS) \
    \$(SDLGFX_LIBS)     \
    \$(SDLIMAGE_LIBS)   \
    \$(SDL_LIBS)        \
    \$(SDLTTF_LIBS)     \
    \$(SHOUT_LIBS)      \
    \$(SLANG_LIBS)      \
    \$(UNICAP_LIBS)     \
    \$(X11_LIBS)        \
    \$(CAIRO_LIBS)      \
    \$(XIPH_LIBS)       \
    \$(GD_LIBS)"
AC_SUBST(FREEJ_LIBS)

dnl ###########################################################################
dnl ###### now about compile time paths
if test "x${prefix}" = "xNONE"; then
   prefix=${ac_default_prefix}
fi
PACKAGE_LIB_DIR='${prefix}/lib/freej'
AC_SUBST(PACKAGE_LIB_DIR)
PACKAGE_DATA_DIR='${prefix}/share/freej'
AC_SUBST(PACKAGE_DATA_DIR)

dnl alphabetic order on dir/subdir, but Makefile sorts before everything
AC_CONFIG_FILES([
Makefile
freej.pc
bindings/Makefile
bindings/java/Makefile
bindings/perl/Makefile
bindings/python/Makefile
bindings/ruby/Makefile
doc/Makefile
doc/freej.dox
doc/scripting/Makefile
lib/Makefile
lib/ccvt/Makefile
lib/flash/Makefile
lib/sdl_gfx/Makefile
lib/sdl_image/Makefile
lib/sdl_ttf/Makefile
lib/slw/Makefile
lib/javascript/GNUmakefile
scripts/Makefile
src/Makefile
src/include/Makefile
tests/Makefile
])
AC_OUTPUT

dnl function to print verbose configure options only if V=1 is passed to
dnl configure
AC_DEFUN([VRB],
         AS_IF([test x"$V" == x1], INFO([$1])))

dnl autoconf < 2.63 compatibility
m4_ifndef([AS_VAR_APPEND],
          AC_DEFUN([AS_VAR_APPEND], $1=$$1$2))

dnl convenience function so that INFO messages go to config.log and to stdout,
dnl useful when debugging user problems only config.log is needed
AC_DEFUN([INFO],
         AS_ECHO(["$1"]) >&AS_MESSAGE_LOG_FD
         AS_ECHO(["$1"]) >&AS_MESSAGE_FD)

dnl as above, but no newline at the end
AC_DEFUN([INFO_N],
         AS_ECHO_N(["$1"]) >&AS_MESSAGE_LOG_FD
         AS_ECHO_N(["$1"]) >&AS_MESSAGE_FD)

echo
echo
INFO([Compile $PACKAGE $VERSION for ${host}])
echo "configuration summary, pass V=1 to configure for verbose output"

dnl mandatory libs, outputted only if verbose
VRB([= SDL])
VRB([  LIBS   : $SDL_LIBS])
VRB([  CFLAGS : $SDL_CFLAGS])

VRB([= ogg-theora (xiph.org)])
VRB([  LIBS  : $XIPH_LIBS])
VRB([  CFLAGS: $XIPH_CFLAGS])

VRB([= Jack])
VRB([  LIBS  : $JACK_LIBS])
VRB([  CFLAGS: $JACK_CFLAGS])

VRB([= Fftw / harmonics analysis])
VRB([  LIBS  : $FFTW_LIBS])
VRB([  CFLAGS: $FFTW_CFLAGS])

VRB([= Samplerate])
VRB([  LIBS  : $SAMPLERATE_LIBS])
VRB([  CFLAGS: $SAMPLERATE_CFLAGS])

VRB([= ffmpeg])
VRB([  CFLAGS: $FFMPEG_CFLAGS])
VRB([  LIBS  : $FFMPEG_LIBS])

VRB([= OSC controller (liblo)])
VRB([  LIBS   : $LIBLO_LIBS])
VRB([  CFLAGS : $LIBLO_CFLAGS])

VRB([= Shout library])
VRB([  LIBS  : $SHOUT_LIBS])
VRB([  CFLAGS: $SHOUT_CFLAGS])

dnl #### bindings

enabled_bindings=""
disabled_bindings=""

if test x$enable_python = xyes; then
AS_VAR_APPEND(enabled_bindings, " python")
INFO([= Python version $ac_python_version])
VRB([    LIBS   : $PYTHON_LDFLAGS])
VRB([    CFLAGS : $PYTHON_CPPFLAGS])
else
AS_VAR_APPEND(disabled_bindings, " python")
fi

if test x$enable_ruby = xyes; then
AS_VAR_APPEND(enabled_bindings, " ruby")
VRB([= Ruby:])
VRB([    LIBS     : $RUBY_LDFLAGS])
VRB([    CFLAGS   : $RUBY_CPPFLAGS])
else
AS_VAR_APPEND(disabled_bindings, " ruby")
fi

if test x$enable_java = xyes; then
AS_VAR_APPEND(enabled_bindings, " java")
VRB([= Java:])
VRB([    LIBS     : $JAVA_LDFLAGS])
VRB([    CFLAGS   : $JAVA_CFLAGS])
else
AS_VAR_APPEND(disabled_bindings, " java")
fi

if test x$enable_perl = xyes; then
AS_VAR_APPEND(enabled_bindings, " perl")
VRB([= Perl:])
VRB([    LIBS     : $PERL_LDFLAGS])
VRB([    CFLAGS   : $PERL_CFLAGS])
else
AS_VAR_APPEND(disabled_bindings, " perl")
fi

VRB([= Enabled language bindings  : $enabled_bindings])
VRB([= Disabled language bindings : $disabled_bindings])


dnl #### shipped libraries, can be static or dynamic

if test x$have_mozjs = xyes; then
INFO([= javascript interpreter (dynamic)])
else
INFO([= javascript interpreter (static)])
fi
VRB([  LIBS  : $MOZJS_LIBS])
VRB([  CFLAGS: $MOZJS_CFLAGS])

if test x$have_sdlgfx = xyes; then
INFO([= SDL_gfx (dynamic)])
else
INFO([= SDL_gfx (static)])
fi
VRB([  LIBS  : $SDLGFX_LIBS])
VRB([  CFLAGS: $SDLGFX_CFLAGS])

if test x$have_sdlttf = xyes; then
INFO([= SDL_ttf (dynamic)])
else
INFO([= SDL_ttf (static)])
fi
VRB([  LIBS  : $SDLTTF_LIBS])
VRB([  CFLAGS: $SDLTTF_CFLAGS])

if test x$have_sdlimage = xyes; then
INFO([= SDL_image (dynamic)])
else
INFO([= SDL_image (static)])
fi
VRB([  LIBS  : $SDLIMAGE_LIBS])
VRB([  CFLAGS: $SDLIMAGE_CFLAGS])

if test x$have_gd = xyes; then
INFO([= libGD (dynamic)])
VRB([  LIBS  : $GD_LIBS])
fi

dnl #### optional stuff
INFO_N([= AAlib  : ])
if test x$have_aa = xyes; then
   INFO(yes)
VRB([  AA  LIBS   : $AA_LIBS])
else
   INFO(no)
fi

INFO_N([= OpenGL : ])
if test x$enable_opengl = xyes; then
   INFO(yes)
VRB([  GLU LIBS   : $GLU_LIBS])
VRB([  GLU CFLAGS : $GLU_CFLAGS])
else
   INFO(no)
fi

INFO_N([= unicap library : ])
if test x$have_unicap = xyes; then
   INFO(yes)
VRB([  LIBS  : $UNICAP_LIBS])
VRB([  CFLAGS: $UNICAP_CFLAGS])
else
   INFO(no)
fi


dnl #### layers

INFO_N([= vector layer : ])
if test x$have_cairo = xyes; then
   INFO(yes)
VRB([= cairo])
VRB([  LIBS  : $CAIRO_LIBS])
VRB([  CFLAGS: $CAIRO_CFLAGS])
else
   INFO(no)
fi


INFO_N([= text layer : ])
if test x$have_textlayer = xyes; then
   INFO(yes)
VRB([= freetype2])
VRB([  LIBS  : $FT2_LIBS])
VRB([  CFLAGS: $FT2_CFLAGS])
VRB([= fontconfig])
VRB([  LIBS  : $FC_LIBS])
VRB([  CFLAGS: $FC_CFLAGS])
else
   INFO(no)
fi

INFO_N([= xgrab layer : ])
if test x$have_xgrab = xyes; then
   INFO(yes)
else
   INFO(no)
fi

INFO_N([= Flash layer : ])
if test x$have_flash = xyes; then
   INFO([yes (v3 animations only)])
VRB([  LIBS  : $FLASH_LIBS (static)])
else
   INFO(no)
fi

INFO_N([= OpenCV cam layer : ])
if test x$have_opencv = xyes; then
   INFO(yes)
else
   INFO(no)
fi


dnl #### controllers

INFO_N([= ALSA / MidiController : ])
if test x$have_midi = xyes; then
   INFO(yes)
VRB([  LIBS  : $ALSA_LIBS])
VRB([  CFLAGS: $ALSA_CFLAGS])
else
   INFO(no)
fi

INFO_N([= WiiMote controller (cwiid) : ])
if test x$have_cwiid = xyes; then
   INFO(yes)
VRB([  LIBS   : $CWIID_LIBS])
VRB([  CFLAGS : $CWIID_CFLAGS])
else
   INFO(no)
fi


dnl #### misc stuff

INFO_N([= Building binary with support for machine specific advanced instructions : ])
if test x$enable_cpuflags = xyes; then
   INFO(yes)
else
   INFO(no)
fi

INFO_N([= Building machine specific lubricated binary : ])
if test x$enable_lubrication = xyes; then
   INFO(yes)
else
   INFO(no)
fi

INFO_N([= Compiling with debugging symbols : ])
if test x$enable_debug = xyes; then
   INFO(yes)
else
   INFO(no)
fi

INFO_N([= Including support for the GNU Profiler : ])
if test x$enable_profiling = xyes; then
   INFO(yes)
else
   INFO(no)
fi


INFO([= COMPILER FLAGS : $CFLAGS])
INFO([= LINKER FLAGS   : $LDFLAGS])
INFO([= INSTALL PREFIX : $prefix])
echo "============================== now type make, may the source be with you!"
echo

# vim:et:ts=3:sw=3
