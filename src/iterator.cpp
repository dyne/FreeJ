/*  FreeJ - Iterator class
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#include <stdlib.h>
#include <strings.h>
#include <iterator.h>
#include <jutils.h>
#include <config.h>

/* there is quite a lot of typecasting here in order to
   support more iterators on different data types,
   so here we go with some defines to simplify the code */

/* just a shortcut for: target (op) val */
#define OP(target,op,value) \
  if( get_name()[0] == 'f' ) { \
    *(float*)target op *(float*)value; \
  } else if( get_name()[0] == 'i' ) { \
    *(int32_t*)target op *(int32_t*)value; \
  } else if( get_name()[0] == 'u' ) { \
    *(uint32_t*)target op *(uint32_t*)value; \
  } else error("Value type %s not recognized",get_name())

/* the following compares the value between aim and buffer
   puts the result into res (supposed to be a boolean
   the comparison order is: aim (cmp) buffer */
#define CMP(target,cmp,value,res) \
  if( get_name()[0] == 'f' ) \
    if( *(float*)target cmp *(float*)value ) \
      res = true; \
  else if( get_name()[0] == 'i' ) \
    if( *(int32_t*)target cmp *(int32_t*)value ) \
      res = true; \
  else if( get_name()[0] == 'u' ) \
    if( *(uint32_t*)target cmp *(uint32_t*)value ) \
      res = true; \
  else error("Value type %s not recognized",get_name())

Iterator::Iterator(char *ntype, void *val)
  : Entry() {
  
  mode = ITERATOR_MODE_ONCE;
  envelope = ITERATOR_ENVELOPE_LINEAR;
  direction = true;

  buffer = val;
  set_name(ntype);

  // we start aiming at the present value
  aim = valloc(val);

  // minmax defaults
  int def = 0xff;
  max = valloc(val); OP(max,+=,&def);
  min = valloc(val); OP(max,-=,&def);  

}

Iterator::~Iterator() {
  free(min);
  free(max);
  free(aim);
}

void Iterator::cafudda() {
  bool res;
  bool bound = false;
  func("Iterator::cafudda processing %s",get_name());

  // control if we are aiming to a different value
  CMP(aim,==,buffer,res);
  // if the value is allready reached we have nothing to do
  
  CMP(aim,>,buffer,direction);
  
  switch(envelope) {
  case ITERATOR_ENVELOPE_LINEAR:
    if(direction) { // we must increase
      OP(buffer,+=,&step);
      // control if we matched or passed over the aim
      CMP(buffer,>=,aim,res); 
      if(res) {
	OP(buffer,=,aim);
	bound = true;
      }
    } else { 
      OP(buffer,-=,&step);
      // control if we matched or falled below the aim
      CMP(buffer,<=,aim,res);
      if(res) {
	OP(buffer,=,aim);
	bound = true;
      }
    }
    break;
  default: break;
  }

  if(bound)
    switch(mode) {
    case ITERATOR_MODE_ONCE:
      /* stop once aim is reached */
      break;
      
    default: break;
    }
  
}

void Iterator::set_min(void *val) {
  OP(min,=,val);
}
void Iterator::set_max(void *val) {
  OP(max,=,val);
}
void Iterator::set_step(void *val) {
  OP(step,=,val);
}
void Iterator::set_aim(void *val) {
  OP(aim,=,val);
}
void Iterator::set_value(void *val) {
  OP(buffer,=,val);
}

void *Iterator::valloc(void *val) {
  void *buf = NULL;

  if( strcasecmp("float",get_name()) ==0) {
    buf = calloc(1,sizeof(float));
    *(float*)buf = *(float*)val;
  } else if( strcasecmp("int",get_name()) ==0) {
    buf = calloc(1,sizeof(int32_t));
    *(int32_t*)buf = *(int32_t*)val;
  } else if( strcasecmp("uint",get_name()) ==0) {
    buf = calloc(1,sizeof(uint32_t));
    *(uint32_t*)buf = *(uint32_t*)val;
  } else
    error("Iterator::valloc : type %s not recognized",get_name());
  
  if(!buf)
    error("Iterator Value not initialized");

  func("Iterator Value initialized to %d",buf);

  return buf;
}
