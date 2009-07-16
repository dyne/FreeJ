dnl ==============================================================
dnl Process this file with autoconf to produce a configure script.
dnl ==============================================================

dnl Define the sub-version numbers for this release
m4_define([FREEJ_MAJ_VER], 0)
m4_define([FREEJ_MIN_VER], 11)
m4_define([FREEJ_REL_VER], 0)
dnl Construct the final version number out of the sub-version values specified above
m4_define([FREEJ_RELEASE_VERSION], [FREEJ_MAJ_VER.FREEJ_MIN_VER.FREEJ_REL_VER])


AC_INIT([freej],[FREEJ_RELEASE_VERSION])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

dnl ==============================================================
dnl Get the operating system and version number...
dnl ==============================================================
AC_MSG_CHECKING(for which platform we are compiling)
case $host_os in
     *linux*)
     AC_MSG_RESULT(Linux)
     AC_DEFINE(HAVE_LINUX,1,[define if compiling for Linux])
     DL_LIBS="-ldl"
     have_linux=yes

     case $host_cpu in
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
     AC_MSG_RESULT(Darwin/OSX)
     AC_DEFINE(HAVE_DARWIN,1,[define if compiling for Apple Darwin OSX])
     AC_DEFINE(CONFIG_DARWIN,1,[define if compiling for Apple Darwin OSX])
     have_darwin=yes
     ;;

     *freebsd*)
     AC_MSG_RESULT(FreeBSD)
     AC_DEFINE(HAVE_FREEBSD,1,[define if compiling for FreeBSD])
     AC_DEFINE(HAVE_SYS_SOCKET_H,1,[define if compiling for FreeBSD])
     CFLAGS="$CFLAGS -I/usr/local/include -L/usr/local/lib"
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
AC_PROG_LEX
AC_PROG_YACC
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

# Checks for header files.
AC_HEADER_STDC
AC_PATH_X
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h limits.h locale.h \
		memory.h netdb.h netinet/in.h sgtty.h stddef.h stdint.h \
		stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/socket.h \
		sys/time.h sys/timeb.h termio.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_STRUCT_TIMEZONE
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit bzero clock_gettime floor ftime getcwd gethostbyaddr \
		gethostbyname gethostname gettimeofday inet_ntoa localeconv \
		localtime_r memmove memset munmap pow putenv rint select setenv \
		socket sqrt strcasecmp strchr strdup strerror strncasecmp \
		strpbrk strrchr strstr strtol])

# compatibility with older libtool
m4_ifdef([LT_INIT],
	[LT_INIT([disable-static dlopen])],
	[AC_LIBTOOL_DLOPEN
	 AC_DISABLE_STATIC
	 AC_PROG_LIBTOOL])

dnl Check sizes and shift
AC_CHECK_SIZEOF(void*)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

dnl versioning info for libtool
FREEJ_CURRENT=0
FREEJ_REVISION=0
FREEJ_AGE=0
FREEJ_VERSION_INFO="$FREEJ_CURRENT:$FREEJ_REVISION:$FREEJ_AGE"
dnl Note this is the ABI version which is not the same as our actual library version
AC_SUBST(FREEJ_VERSION_INFO)


dnl ==============================================================
dnl check for python to compile the module with swig
dnl ==============================================================
AC_PROG_SWIG
SWIG_ENABLE_CXX

if test -z "$SWIG"; then
   AC_MSG_RESULT(No language bindings will be created)
else
SWIG_COMMAND='swig -c++ -O -Wall -I$(top_builddir) -I$(top_srcdir)/src/include -I$(top_builddir)/doc'
AC_SUBST(SWIG_COMMAND)
SWIG_IFACE='$(top_srcdir)/bindings/freej.i'
AC_SUBST(SWIG_IFACE)

AC_ARG_ENABLE(python,
	[  --enable-python           enable Python language bindings (no)],
	[
if test x$enableval = xyes; then
   AC_MSG_RESULT(yes)
   AM_PATH_PYTHON(2.4)
   SWIG_PYTHON
   have_python=true
fi
],[have_python=false])

dnl and ruby?
EL_CONFIG_SCRIPTING_RUBY
dnl and java?
BIND_JAVA_SCRIPTING
dnl and perl?
PERL_SWIG_BINDINGS

if test x$have_python = xtrue; then
   BINDING_DIRS="$BINDING_DIRS python"
fi
if test x$have_ruby = xtrue; then
   BINDING_DIRS="$BINDING_DIRS ruby"
fi
if test x$have_java = xtrue; then
   BINDING_DIRS="$BINDING_DIRS java"
fi
if test x$have_perl = xtrue; then
   BINDING_DIRS="$BINDING_DIRS perl"
fi

fi # swig found?
AC_SUBST(BINDING_DIRS)


dnl ==============================================================
dnl Add the local include path and some flags
dnl ==============================================================

#-freg-struct-return is to compile an host compatible with freeframe dso
CFLAGS="$CFLAGS -pipe -D_REENTRANT -freg-struct-return -fPIC"
if test x$have_darwin = xyes; then
  # add the fink default path to find headers
  CFLAGS="$CFLAGS -I/sw/include"
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
X11_LIBS=""
X11_INCLUDE=""
PKG_CHECK_MODULES(X11, x11, have_x11=yes, have_x11=no)
AC_SUBST(X11_LIBS)
AC_SUBST(X11_INCLUDE)


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

FFMPEG_LIBS=""
SWSCALE_LIBS=""
PKG_CHECK_MODULES(FFMPEG, libavcodec libavformat libavutil,
	                  have_ffmpeg=yes, have_ffmpeg=no)
if test x$have_ffmpeg = xyes; then
   AC_DEFINE(WITH_FFMPEG,1,
	[define if compiling movie layer linking to ffmpeg libavcodec])
   PKG_CHECK_MODULES(SWSCALE, libswscale, have_swscale=yes, have_swscale=no)
   FFMPEG_LIBS="$FFMPEG_LIBS $SWSCALE_LIBS"
   AC_DEFINE(WITH_SWSCALE,1,
	[define if using new swscale library from ffmpeg])
else
   AC_MSG_RESULT(not found)
fi

dnl ffmpeg include path changes dirty workaround
if test -z "$FFMPEG_CFLAGS" ; then
   FFMPEG_CFLAGS="-I/usr/include"
fi
FFMPEG_CFLAGS="$FFMPEG_CFLAGS \$(addprefix \$(firstword $FFMPEG_CFLAGS),/libavcodec /libavformat /libswscale)"

AC_SUBST(FFMPEG_LIBS)
AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(SWSCALE_LIBS)


dnl ==============================================================
dnl ENABLE LIBFLASH
dnl ==============================================================
AC_ARG_ENABLE(flash,
[  --enable-flash          compile with Flash animation layer (default: yes)],
[ if test x$enableval = xno; then
    have_flash=no
  else
    have_flash=yes;
  fi ],[ have_flash=yes ])
AC_MSG_CHECKING(compiling flash - v3 animation only - layer)
if test x$have_flash = xyes; then
   AC_DEFINE(WITH_FLASH,1,[define if compiling flash layer])
   FLASH_LIBS="\$(top_builddir)/lib/flash/libflash.la"
   AM_CONDITIONAL(BUILD_FLASH, true)
   AC_MSG_RESULT(yes)
else
   AM_CONDITIONAL(BUILD_FLASH, false)
   AC_MSG_RESULT(no)
fi
AC_SUBST(FLASH_LIBS)


dnl ==============================================================
dnl CHECK if there is BLUETOOTH
dnl ==============================================================
BLUEZ_LIBS=""
CWIID_LIBS=""
PKG_CHECK_MODULES(BLUEZ, bluez, have_bluez=yes, have_bluez=no)
if test x$have_bluez = xyes; then
   AC_DEFINE(WITH_BLUEZ,1,[define if compiling bluetooth cwiid controller])
   CWIID_LIBS="\$(top_builddir)/lib/cwiid/libwiimote.la"
   AM_CONDITIONAL(BUILD_CWIID, true)
else
   AM_CONDITIONAL(BUILD_CWIID, false)
fi
AC_SUBST(BLUEZ_LIBS)
AC_SUBST(CWIID_LIBS)


dnl ==============================================================
dnl CHECK if there is VIDEO4LINUX
dnl ==============================================================
AC_CHECK_HEADER(linux/videodev.h)
if test "$ac_cv_header_linux_videodev_h" = "yes"; then
   HAVE_V4L=yes
fi
AC_ARG_ENABLE(v4l,[  --enable-v4l            compile with Video4Linux Layer (autodetect)], [
if test x$HAVE_V4L = xyes -a ! x$enableval = xno; then
   AC_DEFINE(WITH_V4L,1,[define if compiling video4linux layer])
   have_video4linux=yes
fi], [
if test x$HAVE_V4L = xyes; then
   AC_DEFINE(WITH_V4L,1,[define if compiling video4linux layer])
   have_video4linux=yes
fi])


dnl ==============================================================
dnl compile with full warnings and debugging symbols
dnl ==============================================================
AC_ARG_ENABLE(debug,
	[  --enable-debug          compile with debug symbols (no)],
	[case "${enableval}" in
		yes) debug=true ;;
		no) debug=false ;;
		*) AC_MSG_ERROR(--enable-debug expects either yes or no) ;;
	esac], [debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)
if test x$debug = xtrue; then
   CFLAGS="$CFLAGS -Wall -g -ggdb"
fi


dnl ==============================================================================
dnl CHECK to use profiling flags when compiling, for execution analysis with gprof
dnl ==============================================================================
AC_ARG_ENABLE(profiling,
[  --enable-profiling      compile using gprof flags for execution analysis (no)],
[ if test ! x$enableval = xyes; then
    have_profiling=no
  else
    have_profiling=yes;
  fi ],[ have_profiling=no ])
if test x$have_profiling = xyes; then
  CFLAGS="$CFLAGS -pg"
fi


dnl ==============================================================
dnl CHECK to use processor specific optimizations to compile
dnl ==============================================================
AC_ARG_ENABLE(cpuflags,
	       [  --enable-cpuflags       compile with advanced cpu instructions (yes)],
	       [ 
		if test ! x$enableval = xyes; then
		    have_cpuflags=no
		else
		    have_cpuflags=yes;
		fi ],[ have_cpuflags=yes ])

if test x$have_cpuflags = xyes; then
  if test x$have_linux = xyes; then
     CPUFLAGS=`cat /proc/cpuinfo | grep flags`
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
            have_mmx=yes
            AC_DEFINE(HAVE_MMX,1,[define if enabling MMX acceleration])
        fi
  fi
fi

AC_ARG_ENABLE(lubrify,
	       [  --enable-lubrify        compile using processor lubrifications (yes)],
	       [ 
		if test ! x$enableval = xyes; then
		    have_lubrifications=no
		else
		    have_lubrifications=yes;
		fi ],[ have_lubrifications=yes ])

if test x$have_lubrifications = xyes; then
  if test x$have_profiling = xyes -o x$debug = xtrue; then
    AC_MSG_WARN([cannot lubrify code if debug or profiling are enabled])
    have_lubrifications=no
  else
    if test x$have_darwin = xyes; then

       if test x$have_ppc = xyes; then
      # use optimization flags for darwin
            CFLAGS="$CFLAGS -O2 -mabi=altivec -maltivec -mpowerpc-gfxopt"
       else
            CFLAGS="$CFLAGS -O2 -mfpmath=sse -mtune=pentium4"
       fi
	    
      # adding these provoke 'illegal instruction' errors in some
      # ppc/gcc combinations: -mpowerpc-gfxopt -faltivec"
    elif test x$have_ppc = xyes; then
      CFLAGS="$CFLAGS -O2 -fomit-frame-pointer -ffast-math -mabi=altivec -maltivec -mpowerpc-gfxopt"
    elif test x$host_cpu = xx86_64; then
	# should complete this with all 64bit CPUs (AMD too)
	if test x$have_ssse3 = xyes; then
		# multiple core cpu
		CFLAGS="$CFLAGS -O2 -mfpmath=sse -mtune=core2"
	else
		# single core 64bit cpu
		CFLAGS="$CFLAGS -O2 -mfpmath=sse -mtune=nocona"
	fi
    fi
    # any other platform optimizations
    AC_MSG_WARN([no special optimisation flags found for your cpu])
    AC_MSG_WARN([please suggest some on our mailinglist if you know better!])
    CFLAGS="$CFLAGS -O2"
  fi
else
  if test x$have_profiling = xno -a x$debug = xfalse; then
    AC_MSG_NOTICE([enabling generic compiler optimizations])
    CFLAGS="$CFLAGS -O2"
  fi
fi
AC_SUBST(CFLAGS)


dnl ==============================================================
dnl CHECK TO USE OPENGL VIDEO OUTPUT
dnl ==============================================================
OPENGL_LIB=""
AC_ARG_ENABLE(opengl,
	       [  --enable-opengl         experimental opengl rendering (no)],
	       [ 
		if test x$enableval = xyes; then
		    use_opengl=yes 
		    OPENGL_LIB="-lGL -lGLU"
		    AC_DEFINE(WITH_OPENGL,1,
				[define if using experimental opengl rendering])
		    else
			use_opengl=no
		    fi
		    ],
		    [ use_opengl=yes ]
		    )
AC_SUBST(OPENGL_LIB)


dnl ==============================================================
dnl Audio
dnl ==============================================================
ALSA_LIBS=""
PKG_CHECK_MODULES(ALSA, alsa, have_alsa=yes, have_alsa=no)
AC_SUBST(ALSA_LIBS)

# TODO(shammash): is this enough to enable midi on alsa?
if test x$have_alsa = xyes; then
  AC_DEFINE(WITH_MIDI,1,[define if compiling midi controller])
  have_midi=yes
fi

# TODO(shammash): before this refactoring we were enabling jack
# only if alsa was present, check inconsistencies in the code.
JACK_LIBS=""
PKG_CHECK_MODULES(JACK, jack, have_jack=yes, have_jack=no)
AC_SUBST(JACK_LIBS)

FFTW_LIBS=""
PKG_CHECK_MODULES(FFTW, fftw3, have_fftw=yes, have_fftw=no)
AC_SUBST(FFTW_LIBS)

# TODO(shammash): here we enable audio only if FFTW is present which is
# probably wrong. Maybe it's better to split checks, but this needs
# code refactoring as well. When changing this policy please remember
# to change output summary at the end of this file.
if test x$have_jack = xyes && test x$have_fftw = xyes; then
  AC_DEFINE(WITH_SOUND,1,[define if compiling sound system])
  have_sound=yes
fi

GOOM_PPC_FILES=""
GOOM_LIBS=""
# goom removed because gives compilation problems
# replicated on Niels ubuntu 7.10
if test x$have_sound = xyes && test 1 = 0; then
  GOOM_LIBS="\$(top_builddir)/lib/goom/libgoom2.a"
  if test x$host_cpu = xpowerpc; then
    GOOM_PPC_FILES="ppc_zoom_ultimate.s ppc_drawings.s"
  fi
  AM_CONDITIONAL(BUILD_GOOM, true)
else
  AM_CONDITIONAL(BUILD_GOOM, false)
fi
AC_SUBST(GOOM_LIBS)
AC_SUBST(GOOM_PPC_FILES)


dnl ==============================================================
dnl compile including the javascript interpreter
dnl ==============================================================

MOZJS_LIBS=""
MOZJS_CFLAGS=""
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
   AM_CONDITIONAL(BUILD_JAVASCRIPT, true)
   AC_MSG_NOTICE([Using freej version of MOZJS])
else
   AM_CONDITIONAL(BUILD_JAVASCRIPT, false)
fi

if test x$debug = xyes ; then
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

AC_DEFINE(WITH_JAVASCRIPT,1,[Define if using libmozjs for spidermonkey javascript])
AC_SUBST(MOZJS_LIBS)
AC_SUBST(MOZJS_CFLAGS)


dnl ==============================================================
dnl unicap lib
dnl ==============================================================
UNICAP_LIBS=""
UNICAP_CFLAGS=""
PKG_CHECK_MODULES(UNICAP, libunicap, have_unicap=yes, have_unicap=no)
if test x$have_unicap = xyes; then
   AC_DEFINE(WITH_UNICAP,1,[Define if to link libunicap for camera capture])
fi
AC_SUBST(UNICAP_LIBS)
AC_SUBST(UNICAP_CFLAGS)


dnl ==============================================================
dnl shout lib
dnl ==============================================================
SHOUT_LIBS="\$(top_builddir)/lib/shout/libshout.la"
SHOUT_CFLAGS="-I\$(top_builddir)/lib/shout"
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
if test x$have_sdlgfx = xyes; then
   AM_CONDITIONAL(BUILD_SDLGFX, false)
else
   AC_MSG_NOTICE([Using freej version of SDLGFX])
   AM_CONDITIONAL(BUILD_SDLGFX, true)
   SDLGFX_LIBS="\$(top_builddir)/lib/sdl_gfx/libsdl_gfx.la"
   SDLGFX_CFLAGS="-I\$(top_srcdir)/lib/sdl_gfx"
fi


dnl ==============================================================
dnl frei0r plugins
dnl ==============================================================
PKG_CHECK_MODULES(FREI0R, frei0r, have_frei0r=yes, have_frei0r=no)
if test x$have_frei0r = xyes; then
   AM_CONDITIONAL(BUILD_FREI0R, false)
else
   AC_MSG_NOTICE([Using freej version of FREI0R])
   AM_CONDITIONAL(BUILD_FREI0R, true)
   FREI0R_CFLAGS="-I\$(top_srcdir)/lib/frei0r"
fi


dnl ==============================================================
dnl link with memory debugging library dmalloc
dnl ==============================================================
# TODO(godog): automake provides AM_WITH_DMALLOC
AC_CHECK_HEADER(dmalloc.h)
if test "$ac_cv_header_dmalloc_h" = "yes"; then

AC_MSG_CHECKING(if malloc debugging is wanted)
AC_ARG_WITH(dmalloc,
[  --with-dmalloc          use dmalloc, as in
                          ftp://ftp.letters.com/src/dmalloc/dmalloc.tar.gz],
[if test "$withval" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_DMALLOC,1,
            [Define if using the dmalloc debugging malloc package])
  LIBS="$LIBS -ldmallocthcxx"
  DMALLOCC="dmallocc.cpp"
  have_dmalloc=yes
else
  AC_MSG_RESULT(no)
  DMALLOCC=""
  have_dmalloc=no
fi], [AC_MSG_RESULT(no)])

fi

AC_PROG_PERL_MODULES([HTML::Template],
		 AM_CONDITIONAL(BUILD_JSAPIDOC, true),
		 AM_CONDITIONAL(BUILD_JSAPIDOC, false))

# select CCVT CONVERSION TYPE
CCVT_CONV="ccvt_c2.c"
AC_SUBST(CCVT_CONV)

CXXFLAGS="$CFLAGS"


dnl ==============================================================
dnl in-tree static libraries
dnl ==============================================================
FREEJ_STATIC_INCLUDES="-I\$(top_builddir) \
    -I\$(top_srcdir)/src/include \
    -I\$(top_srcdir)/lib \
    -I\$(top_srcdir)/lib/ccvt \
    -I\$(top_srcdir)/lib/shout \
    -I\$(top_srcdir)/lib/lo \
    -I\$(top_srcdir)/lib/sdl_image \
    -I\$(top_srcdir)/lib/sdl_ttf \
    -I\$(top_srcdir)/lib/flash \
    -I\$(top_srcdir)/lib/goom \
    -I\$(top_srcdir)/lib/cwiid \
    -I\$(top_srcdir)/lib/slw"

FREEJ_STATIC_LIBS="\$(top_builddir)/lib/ccvt/libccvt.la \
    \$(top_builddir)/lib/sdl_image/libsdl_image.la \
    \$(top_builddir)/lib/sdl_ttf/libsdl_ttf.la \
    \$(top_builddir)/lib/lo/liblo.la \
    \$(top_builddir)/lib/slw/libslw.la"

AC_SUBST(FREEJ_STATIC_INCLUDES)
AC_SUBST(FREEJ_STATIC_LIBS)

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

if test x$use_opengl = xyes; then
echo "= OpenGL"
echo "  LIBS  : $OPENGL_LIB"
else
echo "= OpenGL disabled"
fi

echo "= Language bindings"
if test x$have_python = xtrue; then
echo "  Python LIBS   : $PYTHON_EXTRA_LIBS"
echo "  Python CFLAGS : $PYTHON_CPPFLAGS"
else
echo "  Python disabled"
fi
if test x$have_ruby = xtrue; then
echo "  Ruby LIBS     : $RUBY_LIBS"
echo "  Ruby CFLAGS   : $RUBY_CFLAGS"
else
echo "  Ruby disabled"
fi
if test x$have_java = xtrue; then
echo "  Java LIBS     : $JAVA_LIBS"
echo "  Java CFLAGS   : $JAVA_CFLAGS"
else
echo "  Java disabled"
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

if test x$have_ffmpeg = xyes; then
echo "= ffmpeg (movie layer)"
echo "  CFLAGS: $FFMPEG_CFLAGS"
echo "  LIBS  : $FFMPEG_LIBS"
if test x$have_swscale = xyes; then
echo "  using new swscale api"
fi
else
echo "= ffmpeg (movie layer) disabled"
fi

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
if test x$have_cpuflags = xyes; then
echo "= Building binary with support for machine specific advanced instructions"
else
echo "= Not building binary with support for machine specific advanced instructions"
fi
if test x$have_lubrifications = xyes; then
echo "= Building machine specific lubrificated binary"
else
echo "= Not building machine specific lubrificated binary"
fi
if test x$debug = xtrue; then
echo "= Compiling with debugging symbols"
else
echo "= Not compiling with debugging symbols"
fi
if test x$have_profiling = xyes; then
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
else
   echo "= Sound bus disabled"
fi

echo "= COMPILER FLAGS : $CFLAGS"
echo "= LINKER FLAGS   : $LDFLAGS"



echo "= INSTALL PREFIX : $prefix"
echo "============================== now type make, may the source be with you!"
echo

