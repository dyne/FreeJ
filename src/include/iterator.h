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

#ifndef __ITERATOR_H__
#define __ITERATOR_H__


#include <inttypes.h>
#include <linklist.h>

#define ITERATOR_MODE_ONCE 1
#define ITERATOR_MODE_LOOP 2
#define ITERATOR_MODE_PINGPONG 3

#define ITERATOR_ENVELOPE_LINEAR 1
#define ITERATOR_ENVELOPE_SIN 2
#define ITERATOR_ENVELOPE_RAND 3

class Iterator : public Entry {
 public:
  Iterator(char *ntype, void *val);
  ~Iterator();

  int cafudda();

  void set_min(void *val);  
  void set_max(void *val);
  void set_step(void *val);
  void set_aim(void *val);
  void set_value(void *val);

  int mode;
  int envelope;
  bool direction;
  
 private:
  void *buffer;
  void *min;
  void *max;
  void *aim;
  void *step;

  void *valloc(void *val);

};


#endif
