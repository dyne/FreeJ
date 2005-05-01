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

#define ITERATOR_ENVELOPE_LINEAR 1
#define ITERATOR_ENVELOPE_SIN 2
#define ITERATOR_ENVELOPE_RAND 3
enum iterator_envelope_t { LINEAR, SIN, RAND };
enum iterator_mode_t { ONCE, LOOP, BOUNCE, PULSE };

class Iterator : public Entry {
 public:
  Iterator(int32_t *val);
  ~Iterator();

  int cafudda();

  void set_min(int32_t val);  
  void set_max(int32_t val);
  void set_step(int32_t val);
  void set_aim(int32_t val);
  void set_value(int32_t *val);
  void set_mode(iterator_mode_t m);
  void set_envelope(iterator_envelope_t e);
  
  bool direction;
  
 private:
  int32_t *value;
  int32_t saved_value;
  int32_t min;
  int32_t max;
  int32_t aim;
  int32_t step;

  iterator_envelope_t envelope;
  iterator_mode_t mode;
};


#endif
