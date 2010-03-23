#ifndef __CONFIG_H__
#define __CONFIG_H__
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* define if compiling for Linux/PPC */
/* #undef ARCH_POWERPC */

/* define if compiling for Linux/PPC */
/* #undef ARCH_PPC */

/* define if compiling for Linux/x86 */
/* #undef ARCH_X86 */

/* define if compiling for Apple Darwin OSX */
#define CONFIG_DARWIN 1

/* define if compiling FFMpeg AvCoded with ogg */
#define CONFIG_LIBOGG 1

/* define if compiling FFMpeg AvCoded with theora */
#define CONFIG_LIBTHEORA 1

/* define if compiling FFMpeg AvCoded with vorbis */
#define CONFIG_LIBVORBIS 1

/* define if compiling FFMpeg with oggtheora decoder */
#define CONFIG_OGGTHEORA_DECODER 1

/* define if compiling FFMpeg with oggtheora encoder */
#define CONFIG_OGGTHEORA_ENCODER 1

/* define if compiling FFMpeg with oggvorbis decoder */
#define CONFIG_OGGVORBIS_DECODER 1

/* define if compiling FFMpeg with oggvorbis encoder */
#define CONFIG_OGGVORBIS_ENCODER 1

/* define if host has 64 bit */
#ifdef __x86_64
# define HAVE_64BIT 1
#else
# undef HAVE_64BIT
#endif

#ifdef __ppc__
#define HAVE_ALTIVEC 1
#define HAVE_ALTIVEC_H 1
#else
/* define if cpu supports Altivec instruction set */
#undef HAVE_ALTIVEC
/* define if cpu supports Altivec instruction set */
#undef HAVE_ALTIVEC_H
#endif

/* define if compiling for Apple Darwin OSX */
#define HAVE_DARWIN 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `ioctl' function. */
#define HAVE_IOCTL 1

/* define if compiling for Linux */
/* #undef HAVE_LINUX */

/* Define to 1 if you have the `malloc' function. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP 1

/* define if enabling MMX acceleration */
#ifndef __ppc__
#  define HAVE_MMX 1
#endif

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* define if enabling SSE acceleration */
#ifndef __ppc__
#define HAVE_SSE
#define HAVE_SSE2
#endif
/* #undef HAVE_SSE */

/* define if enabling SSE2 acceleration */
/* #undef HAVE_SSE2 */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Name of package */
#define PACKAGE "freej"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* data */
#define PACKAGE_DATA_DIR "/usr/local/share/freej"

/* libs */
#define PACKAGE_LIB_DIR "/usr/local/lib/freej"

/* locale */
#define PACKAGE_LOCALE_DIR "/usr/local/share/locale"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Flowmixer"

/* Prefix */
#define PACKAGE_PREFIX "/usr/local"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* define big endian arch for portaudio */
/* #undef PA_BIG_ENDIAN */

/* define little endian arch for portaudio */
/* #undef PA_LITTLE_ENDIAN */

/* define if compiling for PPC */
/* #undef POWERPC_MODE_32BITS */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* define if compiling Theora encoder */
#define THEORA_SUPPORT_ENCODE 1

/* define if compiling Theora float codec */
#define THEORA_SUPPORT_FLOAT 1

/* Version number of package */
#define VERSION "0.10"

/* define if compiling bluetooth cwiid controller */
#undef WITH_BLUEZ

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */

/* define if compiling movie layer linking to ffmpeg libavcodec */
#define WITH_FFMPEG 1

/* define if compiling flash layer */
/* #undef WITH_FLASH */

/* define if linking freetype for text rendering */
#define WITH_FT2 1

/* define if compiling glade gtk+-2 GUI */
/* #undef WITH_GLADE2 */

/* Define is using the javascript interpreter */
#define WITH_JAVASCRIPT 1

/* define if compiling midi controller */
/* #undef WITH_MIDI */

/* define if compiling with Ogg/Theora encoding */
#define WITH_OGGTHEORA 1

/* define if using experimental opengl rendering */
/* #undef WITH_OPENGL */

/* define if compiling sound system */
/* #undef WITH_SOUND */

/* define if compiling video4linux layer */
/* #undef WITH_V4L */

/* define if compiling for Big Endian CPU */
/* #undef WORDS_BIGENDIAN */

/* Define use of MAC types in javascript */
#define XP_MAC 1

/* Define use of UNIX types in javascript */
/* #undef XP_UNIX */

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#define YYTEXT_POINTER 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

#define XP_UNIX 1

#define WITH_COCOA 1

#define WITH_SWSCALE 1

#define THREADSAFE 1

#define WITH_FREI0R 1

/* Note: Freeframe causes problems w/ multiple architectures.
   since there are not that many freeframe "generator" plugins
   out there anyway, I choose to disable them..
*/
//#define WITH_FREEFRAME 1
#undef WITH_FREEFRAME

#define BUILD_NUMBER 56

#define OSX_VERSION 0.10

#define PACKAGE_URL "http://freej.dyne.org"

#define ICECASTSERVER "av.theartcollider.org"

#define ICECASTPASSWORD "inoutsource"

#define ICECASTPORT "8000"

#endif
