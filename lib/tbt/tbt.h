/*  Time Based Text - Recorder
 *
 *  (C) Copyright 2006 Denis Rojo <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __TIMEBASEDTEXT_H__
#define __TIMEBASEDTEXT_H__


#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>

#include <linklist.h>

//#define VERSION "0.5"


/*
  This is the format of a single entry:

  int key  - S-Lang key number (matches ASCII/ANSI)
  int sec  - time delta seconds
  int usec - time delta 1/100 seconds */
class TBTEntry : public Entry {
 public:
  TBTEntry();
  ~TBTEntry();

  uint64_t key;
  uint32_t sec;
  uint32_t usec;

  /* parse from *buf and return true on success */
  bool parse_uint64(void *buf);
  
  /* render in *buf and returns size in bytes */
  int render_uint64(void *buf);
  int render_ascii(void *buf);
  int render_javascript(void *buf);

};

class TBT {
  
 public:
  TBT();
  ~TBT();

  void append(uint64_t key);

  void clear(); ///< deletes all keys and frees memory

  uint64_t getkey(); ///< wait the time and returns the next entry

  int position; ///< incremented by getkey calls

  int load(char *filename);

  int save_bin(char *filename);
  int save_ascii(char *filename);
  int save_javascript(char *filename);

 private:

  Linklist buffer;

  // POSIX.1b time structures
  struct timespec timestamp; // nanosleep (nanosec)

  struct timeval now; // gettimeofday (microsec)
  struct timeval past;
  struct timeval delta;

};

#endif
