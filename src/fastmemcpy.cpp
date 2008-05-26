/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   (c) Copyright 2002       convergence GmbH.
   
   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <sven@convergence.de>.
	      Silvano Galliani aka kysucix <kysucix@dyne.org> sse2 version

   Fast memcpy code was taken from xine (see below).

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
 * Copyright (C) 2001 the xine project
 *
 * This file is part of xine, a unix video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * These are the MMX/MMX2/SSE optimized versions of memcpy
 *
 * This code was adapted from Linux Kernel sources by Nick Kurshev to
 * the mplayer program. (http://mplayer.sourceforge.net)
 *
 * Miguel Freitas split the #ifdefs into several specialized functions that
 * are benchmarked at runtime by xine. Some original comments from Nick
 * have been preserved documenting some MMX/SSE oddities.
 * Also added kernel memcpy function that seems faster than glibc one.
 *
 */

/* Original comments from mplayer (file: aclib.c) This part of code
   was taken by me from Linux-2.4.3 and slightly modified for MMX, MMX2,
   SSE instruction set. I have done it since linux uses page aligned
   blocks but mplayer uses weakly ordered data and original sources can
   not speedup them. Only using PREFETCHNTA and MOVNTQ together have
   effect!

   From IA-32 Intel Architecture Software Developer's Manual Volume 1,

  Order Number 245470:
  "10.4.6. Cacheability Control, Prefetch, and Memory Ordering Instructions"

  Data referenced by a program can be temporal (data will be used
  again) or non-temporal (data will be referenced once and not reused
  in the immediate future). To make efficient use of the processor's
  caches, it is generally desirable to cache temporal data and not
  cache non-temporal data. Overloading the processor's caches with
  non-temporal data is sometimes referred to as "polluting the
  caches".  The non-temporal data is written to memory with
  Write-Combining semantics.

  The PREFETCHh instructions permits a program to load data into the
  processor at a suggested cache level, so that it is closer to the
  processors load and store unit when it is needed. If the data is
  already present in a level of the cache hierarchy that is closer to
  the processor, the PREFETCHh instruction will not result in any data
  movement.  But we should you PREFETCHNTA: Non-temporal data fetch
  data into location close to the processor, minimizing cache
  pollution.

  The MOVNTQ (store quadword using non-temporal hint) instruction
  stores packed integer data from an MMX register to memory, using a
  non-temporal hint.  The MOVNTPS (store packed single-precision
  floating-point values using non-temporal hint) instruction stores
  packed floating-point data from an XMM register to memory, using a
  non-temporal hint.

  The SFENCE (Store Fence) instruction controls write ordering by
  creating a fence for memory store operations. This instruction
  guarantees that the results of every store instruction that precedes
  the store fence in program order is globally visible before any
  store instruction that follows the fence. The SFENCE instruction
  provides an efficient way of ensuring ordering between procedures
  that produce weakly-ordered data and procedures that consume that
  data.

  If you have questions please contact with me: Nick Kurshev:
  nickols_k@mail.ru.
*/

/*  mmx v.1 Note: Since we added alignment of destinition it speedups
    of memory copying on PentMMX, Celeron-1 and P2 upto 12% versus
    standard (non MMX-optimized) version.
    Note: on K6-2+ it speedups memory copying upto 25% and
          on K7 and P3 about 500% (5 times).
*/

/* Additional notes on gcc assembly and processors: [MF]
   prefetch is specific for AMD processors, the intel ones should be
   prefetch0, prefetch1, prefetch2 which are not recognized by my gcc.
   prefetchnta is supported both on athlon and pentium 3.

   therefore i will take off prefetchnta instructions from the mmx1
   version to avoid problems on pentium mmx and k6-2.

   quote of the day:
    "Using prefetches efficiently is more of an art than a science"
*/

#include <sys/time.h>
#include <time.h>

#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <jutils.h>
#include <cpu_accel.h>
#include <fastmemcpy.h>



#ifdef ARCH_X86

/* for small memory blocks (<256 bytes) this version is faster */
#define small_memcpy(to,from,n)\
{\
register unsigned long int dummy;\
__asm__ __volatile__(\
  "rep; movsb"\
  :"=&D"(to), "=&S"(from), "=&c"(dummy)\
  :"0" (to), "1" (from),"2" (n)\
  : "memory");\
}
/* On K6 femms is faster of emms. On K7 femms is directly mapped on emms. */
#ifdef HAVE_3DNOW
#define EMMS     "femms"
#else
#define EMMS     "emms"
#endif

#ifdef HAVE_MMX2
#define PREFETCH "prefetchnta"
#elif defined ( HAVE_3DNOW )
#define PREFETCH  "prefetch"
#else
#define PREFETCH "/nop"
#endif


#undef MOVNTQ
#ifdef HAVE_MMX2
#define MOVNTQ "movntq"
#else
#define MOVNTQ "movq"
#endif

#undef MIN_LEN
#ifdef HAVE_MMX1
#define MIN_LEN 0x800  /* 2K blocks */
#else
#define MIN_LEN 0x40  /* 64-byte blocks */
#endif


static void * agp_memcpy(void *to, const void *from , size_t len) {
	void *retval;
	size_t i;
	retval = to;
       if(len >= MIN_LEN)
	{
	  register unsigned long int delta;
          /* Align destinition to MMREG_SIZE -boundary */
          delta = ((unsigned long int)to)&7;
          if(delta)
	  {
	    delta=8-delta;
	    len -= delta;
	    small_memcpy(to, from, delta);
	  }
	  i = len >> 6; /* len/64 */
	  len &= 63;
        /*
           This algorithm is top effective when the code consequently
           reads and writes blocks which have size of cache line.
           Size of cache line is processor-dependent.
           It will, however, be a minimum of 32 bytes on any processors.
           It would be better to have a number of instructions which
           perform reading and writing to be multiple to a number of
           processor's decoders, but it's not always possible.
        */
	for(; i>0; i--)
	{
		__asm__ __volatile__ (
        	PREFETCH" 320(%0)\n"
		"movq (%0), %%mm0\n"
		"movq 8(%0), %%mm1\n"
		"movq 16(%0), %%mm2\n"
		"movq 24(%0), %%mm3\n"
		"movq 32(%0), %%mm4\n"
		"movq 40(%0), %%mm5\n"
		"movq 48(%0), %%mm6\n"
		"movq 56(%0), %%mm7\n"
		MOVNTQ" %%mm0, (%1)\n"
		MOVNTQ" %%mm1, 8(%1)\n"
		MOVNTQ" %%mm2, 16(%1)\n"
		MOVNTQ" %%mm3, 24(%1)\n"
		MOVNTQ" %%mm4, 32(%1)\n"
		MOVNTQ" %%mm5, 40(%1)\n"
		MOVNTQ" %%mm6, 48(%1)\n"
		MOVNTQ" %%mm7, 56(%1)\n"
		:: "r" (from), "r" (to) : "memory");
		from = ((const unsigned char *)from)+64;
		to   = ((unsigned char *)to)+64;
	}
#ifdef HAVE_MMX2
                /* since movntq is weakly-ordered, a "sfence"
		 * is needed to become ordered again. */
		__asm__ __volatile__ ("sfence":::"memory");
#endif
		/* enables to use FPU */
		__asm__ __volatile__ (EMMS:::"memory");
	}
	/*
	 *	Now do the tail of the block
	 */
	if(len) small_memcpy(to, from, len);
	return retval;
}


/* linux kernel __memcpy (from: /include/asm/string.h) */
static inline void * __memcpy(void * to, const void * from, size_t n)
{
     int d0, d1, d2;

     if ( n < 4 ) {
          small_memcpy(to,from,n);
     }
     else
          __asm__ __volatile__(
                              "rep ; movsl\n\t"
                              "testb $2,%b4\n\t"
                              "je 1f\n\t"
                              "movsw\n"
                              "1:\ttestb $1,%b4\n\t"
                              "je 2f\n\t"
                              "movsb\n"
                              "2:"
                              : "=&c" (d0), "=&D" (d1), "=&S" (d2)
                              :"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
                              : "memory");

     return(to);
}

#ifdef HAVE_MMX

#define MMX_MMREG_SIZE 8

#define MMX1_MIN_LEN 0x800  /* 2K blocks */
#define MIN_LEN 0x40  /* 64-byte blocks */

static void * mmx_memcpy(void * to, const void * from, size_t len)
{
     void *retval;
     size_t i;
     retval = to;

     if (len >= MMX1_MIN_LEN) {
          register unsigned long int delta;
          /* Align destinition to MMREG_SIZE -boundary */
          delta = ((unsigned long int)to)&(MMX_MMREG_SIZE-1);
          if (delta) {
               delta=MMX_MMREG_SIZE-delta;
               len -= delta;
               small_memcpy(to, from, delta);
          }
          i = len >> 6; /* len/64 */
          len&=63;
          for (; i>0; i--) {
               __asm__ __volatile__ (
                                    "movq (%0), %%mm0\n"
                                    "movq 8(%0), %%mm1\n"
                                    "movq 16(%0), %%mm2\n"
                                    "movq 24(%0), %%mm3\n"
                                    "movq 32(%0), %%mm4\n"
                                    "movq 40(%0), %%mm5\n"
                                    "movq 48(%0), %%mm6\n"
                                    "movq 56(%0), %%mm7\n"
                                    "movq %%mm0, (%1)\n"
                                    "movq %%mm1, 8(%1)\n"
                                    "movq %%mm2, 16(%1)\n"
                                    "movq %%mm3, 24(%1)\n"
                                    "movq %%mm4, 32(%1)\n"
                                    "movq %%mm5, 40(%1)\n"
                                    "movq %%mm6, 48(%1)\n"
                                    "movq %%mm7, 56(%1)\n"
                                    :: "r" (from), "r" (to) : "memory");
	       from = ((const unsigned char *)from)+64;
		to   = ((unsigned char *)to)+64;
          }
          __asm__ __volatile__ ("emms":::"memory");
     }
     /*
      * Now do the tail of the block
      */
     if (len) __memcpy(to, from, len);
     return retval;
}

/* we might want to write optimized versions of these later */
#define __constant_count_memset(s,c,count) __memset_generic((s),(c),(count))

/*
 * memset(x,0,y) is a reasonably common thing to do, so we want to fill
 * things 32 bits at a time even when we don't know the size of the
 * area at compile-time..
 */
void mymemzero(void * s, unsigned long c ,size_t count)
{
int d0, d1;
__asm__ __volatile__(
	"rep ; stosl\n\t"
	"testb $2,%b3\n\t"
	"je 1f\n\t"
	"stosw\n"
	"1:\ttestb $1,%b3\n\t"
	"je 2f\n\t"
	"stosb\n"
	"2:"
	: "=&c" (d0), "=&D" (d1)
	:"a" (c), "q" (count), "0" (count/4), "1" ((long) s)
	:"memory");
}

#ifdef HAVE_SSE

#define SSE_MMREG_SIZE 16

static void * mmx2_memcpy(void * to, const void * from, size_t len)
{
     void *retval;
     size_t i;
     retval = to;

     /* PREFETCH has effect even for MOVSB instruction ;) */
     __asm__ __volatile__ (
                          "   prefetchnta (%0)\n"
                          "   prefetchnta 64(%0)\n"
                          "   prefetchnta 128(%0)\n"
                          "   prefetchnta 192(%0)\n"
                          "   prefetchnta 256(%0)\n"
                          : : "r" (from) );

     if (len >= MIN_LEN) {
          register unsigned long int delta;
          /* Align destinition to MMREG_SIZE -boundary */
          delta = ((unsigned long int)to)&(MMX_MMREG_SIZE-1);
          if (delta) {
               delta=MMX_MMREG_SIZE-delta;
               len -= delta;
               small_memcpy(to, from, delta);
          }
          i = len >> 6; /* len/64 */
          len&=63;
          for (; i>0; i--) {
               __asm__ __volatile__ (
                                    "prefetchnta 320(%0)\n"
                                    "movq (%0), %%mm0\n"
                                    "movq 8(%0), %%mm1\n"
                                    "movq 16(%0), %%mm2\n"
                                    "movq 24(%0), %%mm3\n"
                                    "movq 32(%0), %%mm4\n"
                                    "movq 40(%0), %%mm5\n"
                                    "movq 48(%0), %%mm6\n"
                                    "movq 56(%0), %%mm7\n"
                                    "movntq %%mm0, (%1)\n"
                                    "movntq %%mm1, 8(%1)\n"
                                    "movntq %%mm2, 16(%1)\n"
                                    "movntq %%mm3, 24(%1)\n"
                                    "movntq %%mm4, 32(%1)\n"
                                    "movntq %%mm5, 40(%1)\n"
                                    "movntq %%mm6, 48(%1)\n"
                                    "movntq %%mm7, 56(%1)\n"
                                    :: "r" (from), "r" (to) : "memory");
	       from = ((const unsigned char *)from)+64;
		to   = ((unsigned char *)to)+64;
          }
          /* since movntq is weakly-ordered, a "sfence"
          * is needed to become ordered again. */
          __asm__ __volatile__ ("sfence":::"memory");
          __asm__ __volatile__ ("emms":::"memory");
     }
     /*
      * Now do the tail of the block
      */
     if (len) __memcpy(to, from, len);
     return retval;
}

/* SSE note: i tried to move 128 bytes a time instead of 64 but it
didn't make any measureable difference. i'm using 64 for the sake of
simplicity. [MF] */
static void * sse_memcpy(void * to, const void * from, size_t len)
{
     void *retval;
     size_t i;
     retval = to;

     /* PREFETCH has effect even for MOVSB instruction ;) */
     __asm__ __volatile__ (
                          "   prefetchnta (%0)\n"
                          "   prefetchnta 64(%0)\n"
                          "   prefetchnta 128(%0)\n"
                          "   prefetchnta 192(%0)\n"
                          "   prefetchnta 256(%0)\n"
                          : : "r" (from) );

     if (len >= MIN_LEN) {
          register unsigned long int delta;
          /* Align destinition to MMREG_SIZE -boundary */
          delta = ((unsigned long int)to)&(SSE_MMREG_SIZE-1);
          if (delta) {
               delta=SSE_MMREG_SIZE-delta;
               len -= delta;
               small_memcpy(to, from, delta);
          }
          i = len >> 6; /* len/64 */
          len&=63;
          if (((unsigned long)from) & 15)
               /* if SRC is misaligned */
		  for (; i>0; i--) {
			  __asm__ __volatile__ (
					  "prefetchnta 320(%0)\n"
					  "movups (%0), %%xmm0\n"
					  "movups 16(%0), %%xmm1\n"
					  "movups 32(%0), %%xmm2\n"
					  "movups 48(%0), %%xmm3\n"
					  "movntps %%xmm0, (%1)\n"
					  "movntps %%xmm1, 16(%1)\n"
					  "movntps %%xmm2, 32(%1)\n"
					  "movntps %%xmm3, 48(%1)\n"
					  :: "r" (from), "r" (to) : "memory");
			  from = ((const unsigned char *)from)+64;
			  to   = ((unsigned char *)to)+64;
		  }
          else
               /*
                  Only if SRC is aligned on 16-byte boundary.
                  It allows to use movaps instead of movups, which required
                  data to be aligned or a general-protection exception (#GP)
                  is generated.
               */
               for (; i>0; i--) {
                    __asm__ __volatile__ (
                                         "prefetchnta 320(%0)\n"
                                         "movaps (%0), %%xmm0\n"
                                         "movaps 16(%0), %%xmm1\n"
                                         "movaps 32(%0), %%xmm2\n"
                                         "movaps 48(%0), %%xmm3\n"
                                         "movntps %%xmm0, (%1)\n"
                                         "movntps %%xmm1, 16(%1)\n"
                                         "movntps %%xmm2, 32(%1)\n"
                                         "movntps %%xmm3, 48(%1)\n"
                                         :: "r" (from), "r" (to) : "memory");
			  from = ((const unsigned char *)from)+64;
			  to   = ((unsigned char *)to)+64;
               }
          /* since movntq is weakly-ordered, a "sfence"
           * is needed to become ordered again. */
          __asm__ __volatile__ ("sfence":::"memory");
          /* enables to use FPU */
          __asm__ __volatile__ ("emms":::"memory");
     }
     /*
      * Now do the tail of the block
      */
     if (len) __memcpy(to, from, len);
     return retval;
}

static void * sse2_memcpy(void * to, const void * from, size_t len)
{
     void *retval;
     size_t i;
     retval = to;

     /* PREFETCH has effect even for MOVSB instruction ;) */
     /* Is that useful ? kysucix */

     __asm__ __volatile__ (
                          "   prefetchnta (%0)\n"
                          "   prefetchnta 64(%0)\n"
                          "   prefetchnta 128(%0)\n"
                          "   prefetchnta 192(%0)\n"
                          "   prefetchnta 256(%0)\n"
			  /*
                          "   prefetchnta 320(%0)\n"
                          "   prefetchnta 384(%0)\n"
                          "   prefetchnta 448(%0)\n"
                          "   prefetchnta 512(%0)\n"
			  */
                          : : "r" (from) );

     if (len >= MIN_LEN) {
          register unsigned long int delta;
          /* Align destinition to MMREG_SIZE -boundary */
          delta = ((unsigned long int)to)&(SSE_MMREG_SIZE-1);
          if (delta) {
               delta=SSE_MMREG_SIZE-delta;
               len -= delta;
               small_memcpy(to, from, delta);
          }
          i = len >> 7; /* len/128 */
          len&=127;
          if (((unsigned long)from) & 15)
               /* if SRC is misaligned */
               for (; i>0; i--) {
                    __asm__ __volatile__ (
                                         "prefetchnta 640(%0)\n"

                                         "movdqu (%0), %%xmm0\n"
                                         "movdqu 16(%0), %%xmm1\n"
                                         "movdqu 32(%0), %%xmm2\n"
                                         "movdqu 48(%0), %%xmm3\n"

                                         "movntdq %%xmm0, (%1)\n"
                                         "movntdq %%xmm1, 16(%1)\n"
                                         "movntdq %%xmm2, 32(%1)\n"
                                         "movntdq %%xmm3, 48(%1)\n"

                                         "movdqu 64(%0), %%xmm4\n"
                                         "movdqu 80(%0), %%xmm5\n"
                                         "movdqu 96(%0), %%xmm6\n"
                                         "movdqu 112(%0), %%xmm7\n"

                                         "movntdq %%xmm4, 64(%1)\n"
                                         "movntdq %%xmm5, 80(%1)\n"
                                         "movntdq %%xmm6, 96(%1)\n"
                                         "movntdq %%xmm7, 112(%1)\n"
                                         :: "r" (from), "r" (to) : "memory");
			  from = ((const unsigned char *)from)+128;
			  to   = ((unsigned char *)to)+128;
               }
          else
               /*
                  Only if SRC is aligned on 16-byte boundary.
                  It allows to use movaps instead of movups, which required
                  data to be aligned or a general-protection exception (#GP)
                  is generated.
               */
               for (; i>0; i--) {
                    __asm__ __volatile__ (
                                         "prefetchnta 640(%0)\n"
					 
                                         "movapd (%0), %%xmm0\n"
                                         "movapd 16(%0), %%xmm1\n"
                                         "movapd 32(%0), %%xmm2\n"
                                         "movapd 48(%0), %%xmm3\n"
					 
                                         "movntdq %%xmm0, (%1)\n"
                                         "movntdq %%xmm1, 16(%1)\n"
                                         "movntdq %%xmm2, 32(%1)\n"
                                         "movntdq %%xmm3, 48(%1)\n"
					 
                                         "movapd 64(%0), %%xmm4\n"
                                         "movapd 80(%0), %%xmm5\n"
                                         "movapd 96(%0), %%xmm6\n"
                                         "movapd 112(%0), %%xmm7\n"

                                         "movntdq %%xmm4, 64(%1)\n"
                                         "movntdq %%xmm5, 80(%1)\n"
                                         "movntdq %%xmm6, 96(%1)\n"
                                         "movntdq %%xmm7, 112(%1)\n"
                                         :: "r" (from), "r" (to) : "memory");
			  from = ((const unsigned char *)from)+128;
			  to   = ((unsigned char *)to)+128;
               }
          /* since movntq is weakly-ordered, a "sfence"
           * is needed to become ordered again. */
          __asm__ __volatile__ ("mfence":::"memory");
          /* enables to use FPU */
          __asm__ __volatile__ ("emms":::"memory");
     }
     /*
      * Now do the tail of the block
      */
     if (len) __memcpy(to, from, len);
     return retval;
}
#endif /* USE_SSE */
#endif /* USE_MMX */


static void *linux_kernel_memcpy(void *to, const void *from, size_t len) {
     return __memcpy(to,from,len);
}

#endif /* ARCH_X86 */

/* save library size on platforms without special memcpy impl. */

static struct {
     const char         *name;
     void               *(*function)(void *to, const void *from, size_t len);
     unsigned long long    time;
     __u32                 cpu_require;
} memcpy_method[] =
{
     { NULL, NULL, 0, 0},
     { "glibc memcpy()",            memcpy, 0, 0},
#ifdef ARCH_X86
     { "linux kernel memcpy()",     linux_kernel_memcpy, 0, 0},
     { "agp optimized memcpy()",    agp_memcpy,0,0},
#ifdef HAVE_MMX
     { "MMX optimized memcpy()",    mmx_memcpy, 0, MM_MMX},
#ifdef HAVE_SSE
     { "MMXEXT optimized memcpy()", mmx2_memcpy, 0, MM_MMXEXT},
     { "SSE optimized memcpy()",    sse_memcpy, 0, MM_MMXEXT|MM_SSE},
#ifdef HAVE_SSE2
     { "SSE2 optimized memcpy()",    sse2_memcpy, 0, MM_MMXEXT|MM_SSE|MM_SSE2},
#endif /* USE_SSE2  */
#endif /* USE_SSE  */
#endif /* USE_MMX  */
#endif /* ARCH_X86 */
    { NULL, NULL, 0, 0},
};


#ifdef ARCH_X86
static inline unsigned long long int rdtsc()
{
     unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}
#else
static inline unsigned long long int rdtsc()
{
     struct timeval tv;
   
     gettimeofday (&tv, NULL);
     return (tv.tv_sec * 1000000 + tv.tv_usec);
}
#endif


//void *(* jmemcpy)(void *to, const void *from, size_t len) = memcpy;

#define BUFSIZE 1024

void find_best_memcpy()
{
     /* save library size on platforms without special memcpy impl. */

     unsigned long long t;
     char *buf1, *buf2;
     int i, j, best = 0;
     __u32 config_flags = detect_mm_accel();

     if (!(buf1 = (char*) malloc( BUFSIZE * 2000 * sizeof(char) )))
          return;

     if (!(buf2 = (char*) malloc( BUFSIZE * 2000 * sizeof(char) ))) {
          free( buf1 );
          return;
     }
	
     memset(buf1,0, BUFSIZE*2000);
     memset(buf2,0, BUFSIZE*2000);

     /* make sure buffers are present on physical memory */
     memcpy( buf1, buf2, BUFSIZE * 2000 );
     memcpy( buf2, buf1, BUFSIZE * 2000 );
     func("Finding best memory copy function");
     for (i=1; memcpy_method[i].name; i++) {
          if (memcpy_method[i].cpu_require & ~config_flags)
               continue;

          t = rdtsc();

          for (j=0; j<2000; j++)
               memcpy_method[i].function( buf1 + j*BUFSIZE, buf2 + j*BUFSIZE, BUFSIZE );

          t = rdtsc() - t;
          memcpy_method[i].time = t;

          func("%s : time %2.2f",
	       memcpy_method[i].name, (float) ( (float) t / 1000000.0));

          if (best == 0 || t < memcpy_method[best].time)
               best = i;
     }

     if (best) {
       notice("Using memory-to-memory copy method : %s",
	   memcpy_method[best].name);

       jmemcpy = memcpy_method[best].function;
     }

     free( buf1 );
     free( buf2 );
}

