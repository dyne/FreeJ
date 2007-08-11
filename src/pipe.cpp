/*
  Copyright (c) 2001 Charles Samuels <charles@kde.org>
  Copyright (c) 2002 - 2007 Denis Rojo <jaromil@dyne.org>
  
this pipe class was first written by Charles Samuels
and then heavily mutilated and optimized by Denis "jaromil" Rojo

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
  
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
   
You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

"$Id$"

*/

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

//#include <audioproc.h>
#include <pipe.h>
#include <jutils.h>
#include <config.h>


#define MIN(a,b) (a<=b) ? a : b; 

#define _SIZE(val) \
  if ((char*)end > (char*)start) \
    val = (char*)end-(char*)start; \
  else  \
    val = ((char*)bufferEnd-(char*)start)+((char*)end-(char*)buffer);

#define _SPACE(val) \
  _SIZE(val); \
  val = ((char*)bufferEnd-(char*)buffer)-val;

// COPY AND MIX CALLBACKS
// they provide function that are moving data
// handling it in different ways. it is an optimization when we do
// conversions while copying the buffer around, or mixing it directly
// from the pipe to a buffer.

// samples are double if stereo (1 sample is only left or right)
// multiplying them by the samplesize we can obtain sizes in bytes

static inline void copy_byte(void *dst, void *src, int samples) {
  jmemcpy(dst,src,samples);
  }

static inline void copy_int16_to_float(void *dst, void *src, int samples) {
  register int c;
  for( c = samples; c>0 ; c-- ) {
    ((float*)dst)[c] = ((int16_t*)src)[c] / 32768.0f;
  }
}

static inline void copy_float_to_int16(void *dst, void *src, int samples) {
  register int c;
  for( c = samples; c>0 ; c-- ) {
    ((int16_t*)dst)[c] = (int16_t) lrintf( ((float*)src)[c] );
  }
}

static inline void mix_int16_to_int32(void *dst, void *src, int samples) {
  register int c;
  for( c = samples ; c>0 ; c-- ) {
    ((int32_t*)dst)[c]
      +=
      ((int16_t*)src)[c];
  }
}

///// now register all the copy callbacks functions
// // this is also a list for the available types

static struct pipe_copy_list callbacks[] = {
  //  name                    function             read src samplesize  write dst samplesize (in bytes)
  { "copy_byte",              copy_byte,           1,                   1 },
  { "copy_int16_to_float",    copy_int16_to_float, sizeof(int16_t),     sizeof(float) },
  { "copy_float_to_int16",    copy_float_to_int16, sizeof(float),       sizeof(int16_t) },
  { "mix_int16_to_int32",    mix_int16_to_int32,   sizeof(int16_t),     sizeof(int32_t) },
  { 0, 0 }
};

/*
  start is a pointer to the first character that goes out
  end is a pointer to the last character to go out
*/

bool Pipe::set_input_type(char *name) {
  int c;
  for(c=0 ; callbacks[c].callback ; c++) {
    if(strcasecmp(name,callbacks[c].name)==0) {
      write_copy_cb = &callbacks[c];
      return true;
    }
  }
  error("can't set input type \"%s\" on pipe",name);
  return false;
}

bool Pipe::set_output_type(char *name) {
  int c;
  for(c=0 ; callbacks[c].callback ; c++) {
    if(strcasecmp(name,callbacks[c].name)==0) {
      read_copy_cb = &callbacks[c];
      return true;
    }
  }
  error("can't set output type \"%s\" on pipe",name);
  return false;
}


Pipe::Pipe(int size) {
  func("Pipe::Pipe(%i)",size);
  pipesize = size;
  buffer = calloc(pipesize, 1);
  if(!buffer)
    error("FATAL: can't allocate %i bytes buffer for audio Pipe: %s",
	  pipesize, strerror(errno));
  bufferEnd=(char*)buffer+size;
  end=start=buffer;

  // set default types to simple bytes
  set_input_type("copy_byte");
  set_output_type("copy_byte");
  // set blocking timeout (ttl) defaults
  read_blocking = false;
  read_blocking_time = 2000;
  write_blocking = false;
  write_blocking_time = 2000;

  debug = false;

  _thread_init();
  //unlock();
  
}

Pipe::~Pipe() {
  func("Pipe::~Pipe : freeing %p",buffer);
  lock();
  free(buffer);
  unlock();
  //  _thread_destroy();
}

void Pipe::set_block(bool input, bool output) {
  lock();
  write_blocking = input;
  read_blocking = output;
  unlock();
}

void Pipe::set_block_timeout(int input, int output) {
  lock();
  write_blocking_time = input;
  read_blocking_time = output;
  unlock();
}
    
int Pipe::read(int length, void *data) {
  int worklen, origlen, truelen;
  int blk, len, buffered, buffered_bytes;
  int ttl = 0;
  int num;

  if(read_blocking) ttl = read_blocking_time;

  lock();

  _SIZE(buffered_bytes);
  buffered = buffered_bytes 
    / read_copy_cb->src_samplesize;
  truelen = length;


  while(buffered<length) {
    
  /* if less than desired is in, then 
     (blocking) waits
     (non blocking) returns what's available */
    if(read_blocking) {
      unlock();
      if(!ttl) return -1;
      jsleep(0,10000000); ttl -= 10;
      lock();
      _SIZE(buffered_bytes);
      buffered = buffered_bytes 
	/ read_copy_cb->src_samplesize;
    } else {
    // nothing in the pipe
      if(!buffered) {
	unlock();
	return 0;
      } else
	truelen = buffered;
      break;
    }
  }

  origlen = worklen = truelen * read_copy_cb->src_samplesize;

  while (worklen) {
				
    /* |buffer*****|end-----------|start********|bufferEnd
       |buffer-----|start*********|end----------|bufferEnd */
    
    len = MIN(worklen,buffered_bytes);
    
    blk = ((char*)bufferEnd - (char*)start);

    blk=MIN(blk,len);
    
    /* fill */
    (*read_copy_cb->callback)
      (data, start,
       blk / read_copy_cb->src_samplesize);
	/* blank just copied bytes */
	memset(start,0,blk / read_copy_cb->src_samplesize);
    
    start = (char *) start + blk;
    len -= blk;
    data = (char *) data + blk;
    worklen -= blk;
    if ((end!=buffer) && (start==bufferEnd))
      start = buffer;
    
    if (len) { /* short circuit */

      (*read_copy_cb->callback)
	(data, start,
	 len / read_copy_cb->src_samplesize);
      
	  /* blank just copied bytes */
	  memset(start,0,len / read_copy_cb->src_samplesize);
      data = (char *) data + len;
      start = (char *) start +len;
      worklen -= len;
      if ((end!=buffer) && (start==bufferEnd))
	start = buffer;
    }
  }
  
  unlock();

  num = (origlen-worklen)/read_copy_cb->src_samplesize;
  if(debug) func("read from %s pipe %u samples (%u byte per sample)",
		 read_copy_cb->name, num, read_copy_cb->src_samplesize);

  return ( num );
}

int Pipe::write(int length, void *data) {
  int worklen, origlen, space_samples;
  int space_bytes, len, truelen, blk;
  int ttl = 0;
  int num;

  if(write_blocking) ttl = write_blocking_time;

  lock();

  _SPACE(space_bytes);
  space_samples = (space_bytes / write_copy_cb->dst_samplesize);
  truelen = length;

  while(length > space_samples) {

    // timeout block mechanism
    if(write_blocking) {
      unlock();
      if(!ttl) return -1; // block timeout
      jsleep(0,10000000); ttl -= 10; // wait 10 milliseconds
      lock();
      // recalculate actual sizes
      _SPACE(space_bytes);
      space_samples = space_bytes
	/ write_copy_cb->dst_samplesize;

    } else { // non-block

      if(!space_bytes) {
	unlock();
	return 0; // nothing in the pipe
      } else
	// write what's available
	truelen = space_samples;
      break;
    }
  }
  
  origlen = worklen = truelen * write_copy_cb->dst_samplesize;

  while (worklen) {
    
    /* |buffer-----|end***********|start--------|bufferEnd
       |buffer*****|start---------|end**********|bufferEnd */
    len=MIN(worklen, space_bytes);
    
    blk = (char*)bufferEnd-(char*)end;
    blk = MIN(blk, len);
    
    /* fill */
    (*write_copy_cb->callback)
      (end, data,
       blk / write_copy_cb->dst_samplesize);

      end = (char *) end + blk;
      len -= blk;
      data = (char *) data + blk;
      worklen -= blk;
      if ((start!=buffer)
	  && (end==bufferEnd))
	end = buffer;
		
    if (len) { // short circuit		

      (*write_copy_cb->callback)
	(end, data,
	 len / write_copy_cb->dst_samplesize);

      data = (char *) data + len;
      end = (char *) end +len;
      worklen -= len;
      
      if ((start!=buffer)
	  && (end==bufferEnd))
	end = buffer;
    }
  }
  _SPACE(space_bytes);
  unlock();

  num = (origlen-worklen) / write_copy_cb->dst_samplesize;
  if(debug) func("write to %s pipe %u samples (%u byte per sample)",
		 write_copy_cb->name, num, write_copy_cb->dst_samplesize);

  return (num);

}

// |buffer******|end--------------|start**************|bufferEnd
// |buffer-----|start**************|end---------------|bufferEnd
int Pipe::size() {
  int res;
  /* size macro allocates the result variable by itself */
  lock();
  _SIZE(res);
  unlock();

  return res;
}

// |buffer------|end**************|start--------------|bufferEnd
// |buffer*****|start--------------|end***************|bufferEnd
int Pipe::space() {
  int res;
  lock();
  _SPACE(res);
  unlock();

  return res;
}

void Pipe::flush() {
  lock();
  bufferEnd=(char*)buffer+pipesize;
  end=start=buffer;
  unlock();
}

void Pipe::flush(int bytes) {
  lock();
  void *temp = malloc(bytes);
  read(bytes, temp);
  free(temp);
  unlock();
}
