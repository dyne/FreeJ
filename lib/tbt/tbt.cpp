/*  Time Based Text - Recorder
 *
 *  (C) Copyright 2006 Denis Rojo <jaromil@dyne.org>
 *                     Joan & Dirk <jodi@jodi.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>

#include <tbt.h>
#include <jutils.h>




TBTEntry::TBTEntry() 
 : Entry() {
  key  = 0;
  sec  = 0;
  usec = 0;
  // just a hint
  set_name("char64/sec32/usec32");
}

TBTEntry::~TBTEntry() { }

bool TBTEntry::parse_uint64(void *buf) {

  uint32_t *p = (uint32_t*)buf;

  key  = *(uint64_t*)(p);
  sec  = *(p+2);
  usec = *(p+3);
  
  return true;
}


int TBTEntry::render_uint64(void *buf) {
  int len;
  uint32_t tmp[4];

  ((uint64_t*)tmp)[0] = key;
  tmp[2] = sec;
  tmp[3] = usec;

  len = 16;

  memcpy(buf, (void*)tmp, len);

  return len;
}

int TBTEntry::render_ascii(void *buf) {
  int len;
  char tmp[512];
  
  len = snprintf(tmp,511,"%c:%u,%u\n",(int)key, sec, usec);
  if(len<0) {
    error("error rendering in ascii: %s", strerror(errno));
    return 0;
  }

  memcpy(buf,tmp,len*sizeof(char));
  return len;
}

int TBTEntry::render_javascript(void *buf) {
  int len;
  char tmp[512];

  len = snprintf(tmp,511,"[%u,%u,%u]", (int)key, sec, usec);
  if(len<0) {
    error("error rendering in javascript array: %s", strerror(errno));
    return 0;
  }

  memcpy(buf, tmp, len*sizeof(char));
  return len;
}  

/* Macros for converting between `struct timeval' and `struct timespec'.
#define TIMEVAL_TO_TIMESPEC(tv, ts) {                   \
        (ts).tv_sec =  (tv).tv_sec;                     \
        (ts).tv_nsec = (tv).tv_usec * 1000;             \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {                   \
        (tv).tv_sec =  (ts).tv_sec;                     \
        (tv).tv_usec = (ts).tv_nsec / 1000;             \
}
they seem to be already included in sys/time.h
*/


/* Macro to find time delta between now and past timeval */
#define DELTA_TIMEVAL(n, p, d) {                        \
        (d).tv_sec  = (n).tv_sec  - (p).tv_sec;         \
        (d).tv_usec = (n).tv_usec - (p).tv_usec;        \
}  


TBT::TBT()  {
  // zeroing counters
  now.tv_sec    = 0;
  now.tv_sec    = 0;
  past.tv_sec   = 0;
  past.tv_usec  = 0;
  delta.tv_sec  = 0;
  delta.tv_usec = 0;

  position = 1;

  if( gettimeofday(&past, NULL) <0 ) {
    error("error getting time of day: %s", strerror(errno));
    exit(1);
  }
}

TBT::~TBT() {
  clear();
}

void TBT::clear() {
  // erase all entries and free the memory
  TBTEntry *ent;
  ent = (TBTEntry*)buffer.begin();

  // when deleting objects that's the trick with linklist
  // it scrolls up the next at the beginning
  while(ent) {
        delete ent;	
	ent = (TBTEntry*) buffer.begin();
  }
}

void TBT::append(uint64_t key) {
  
  if( gettimeofday(&now, NULL) <0 )
    error("error getting time: %s",strerror(errno));

  // compute time delay since last key
  DELTA_TIMEVAL(now, past, delta);

  // store current time in the past for next delta
  memcpy(&past, &now, sizeof(struct timeval));

  TBTEntry *ent;
  ent = new TBTEntry();
  ent->key  = (uint64_t)key;
  ent->sec  = (uint32_t)delta.tv_sec;
  ent->usec = (uint32_t)delta.tv_usec;

  buffer.append(ent);
}

uint64_t TBT::getkey() {

  uint32_t wait_sec;
  uint64_t wait_nsec;

  TBTEntry *ent;
  ent = (TBTEntry*) buffer[position];

  if(!ent) {
	  func("NULL entry at position %u", position);
	  return 0;
  }

  position++;

  /* get the time now and check how much time elapsed since last getkey
     then get the delay of the current entry and subtract the elapsed to it
     then wait the time that is left to wait */
  if( gettimeofday(&now, NULL) <0 )
    error("error getting time: %s",strerror(errno));

  // compute time delay since last key
  DELTA_TIMEVAL(past, now, delta);

  // store current time in the past for next delta
  memcpy(&past, &now, sizeof(struct timeval));

  /* see how much time elapsed in processing
     and calculate how much we should still wait */
  wait_sec  = ent->sec  - delta.tv_sec;
  // here convert micro to nano seconds
  wait_nsec = (ent->usec - delta.tv_usec)*1000;

  // perform the nanosleep
  jsleep( wait_sec, wait_nsec );

  return ent->key;
}

int TBT::load(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "r");
  if(!fd) {
    error("can't open file: %s", strerror(errno));
    return false;
  }

  clear();

  // max bytes for an entry here
  buf = malloc(64);

  TBTEntry *ent;

  c = 0;
  
  act("loading file %s", filename);

  while( ! feof(fd) ) {

    len = fread(buf, 4, 4, fd);

    if(len != 4) {
      warning("truncated entry, %u elements read", len);
      continue;
    }
    
    ent = new TBTEntry();

    if( ! ent->parse_uint64(buf) ) {
      error("error in TBTEntry::parse_uint64");
      continue;
    }

    buffer.append( ent );
    c++;

  }

  free(buf);

  position = 1;

  return c;
}


int TBT::save_bin(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;
  
  // max bytes for an entry here
  buf = malloc(512);


  TBTEntry *ent;
  ent = (TBTEntry*) buffer.begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {
 
    len = ent->render_uint64(buf);

    fwrite(buf, len, 1, fd);
    
    c++;
	
	  ent = (TBTEntry*) ent->next;
  }

  
  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}

int TBT::save_ascii(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;
  
  // max bytes for an entry here
  buf = malloc(512);

  TBTEntry *ent;
  ent = (TBTEntry*)buffer.begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {
 
    len = ent->render_ascii(buf);

    fwrite(buf, len, 1, fd);
    
    c++;

    ent = (TBTEntry*) ent->next;
  }
  
  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}

int TBT::save_javascript(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;
  
  // start array
  fwrite("var TimeBasedText=[",sizeof(char),19,fd);

  // max bytes for an entry here
  buf = malloc(512);

  TBTEntry *ent;
  ent = (TBTEntry*) buffer.begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {

    if(c>0) // put a comma
      fwrite(",",sizeof(char),1,fd);

    len = ent->render_javascript(buf);

    fwrite(buf, len, 1, fd);
    
    c++;
	
	  ent = (TBTEntry*) ent->next;
  }


  // close array
  fwrite("]\n",sizeof(char),2,fd);

  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}
