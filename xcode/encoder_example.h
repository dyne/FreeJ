#ifndef INOUTENCODER_H
#define INOUTENCODER_H

///////////////////////////////// HEADERs FOR OGGFWD
#include <sys/types.h>
#include <sys/param.h>
#include <errno.h>
#include <signal.h>

#ifndef NO_UNISTD_H
#  include <unistd.h>
#endif /* no-NO_UNISTD_H */
#include <shout/shout.h>
// For the videopage:
#include "theora/theora.h"

#define BUFFERSIZE	4096

int myOggfwd_process(ogg_page inputVideoPage) ;

///////////////////////////////// HEADERs FOR THEORA ENCODING
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE
#endif
#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE
#endif
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

/* Define to give performance data win32 only*/
//#define THEORA_PERF_DATA 
#ifdef THEORA_PERF_DATA
#include <windows.h>
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>

#endif

#ifndef _REENTRANT
# define _REENTRANT
#endif

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#ifndef _WIN32
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "vorbis/codec.h"
#include "vorbis/vorbisenc.h"

#ifdef _WIN32
/* supply missing headers and functions to Win32 */

#include <fcntl.h>

static double rint(double x)
{
  if (x < 0.0)
    return (double)(int)(x - 0.5);
  else
    return (double)(int)(x + 0.5);
}
#endif

// ObjC includes to use pixelBuffers
#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>

theora_info     myTi;
CVPixelBufferRef sourcePixelBuffer ;

#endif

