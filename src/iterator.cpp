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

/* here there was quite a lot of typecasting in order to
   support more iterators on different data types.
   now this support have been dropped and iterators only work
   on 16bit signed integer numbers. */

Iterator::Iterator(int16_t *val)
  : Entry() {

  /* buffer is not malloc'ed!
     it holds a pointer to the value to be changed */
  value = val;

  func("initialized iterator with value %i",*value);

  set_name("iterator");

  // we start aiming at the present value
  aim = *val;

  // minmax defaults
  max = aim+0xff;
  min = aim-0xff;  

  // default step
  step = 1;

  // default mode and envelope
  mode = ITERATOR_MODE_ONCE;
  envelope = ITERATOR_ENVELOPE_LINEAR;
}

Iterator::~Iterator() {
  func("Iterator::~Iterator destroy");
}

int Iterator::cafudda() {
  bool res;
  bool bound = false;
  //  func("Iterator::cafudda processing %i -> %i", *value, aim);

  // control if we are aiming to a different value:
  if(aim != *value) {
    
    
    // check which direction we wanna go
    direction = (aim>*value)?true:false;
    
    
    //    if( envelope == ITERATOR_ENVELOPE_LINEAR) {
    // we still have only one kind of envelope

    if(direction) { // we must increase
      *value += step;
      // control if we matched or passed over the aim
      if(*value > aim)
	*value = aim;
    } else { // we decrease
      *value -= step;
      // control if we matched or falled below the aim
      if(*value < aim)
	*value = aim;
    }
    
  } else { // goal is reached
    
    if(mode == ITERATOR_MODE_ONCE)
      /* stop once aim is reached */
      return -1;
    
  }
  
  return 1;
}

void Iterator::set_min(int16_t val) {
  min = val;
}
void Iterator::set_max(int16_t val) {
  max = val;
}
void Iterator::set_step(int16_t val) {
  step = val;
}
void Iterator::set_aim(int16_t val) {
  aim = val;
}
void Iterator::set_value(int16_t *val) {
  value = val;
}