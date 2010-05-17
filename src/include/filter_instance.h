/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#ifndef __filter_instance_h__
#define __filter_instance_h__

#include <config.h>

#include <parameter.h>
#include <linklist.h>

class Filter;

class FilterInstance : public Entry {
  friend class Filter;

 public:
  FilterInstance(Filter *fr);
  ~FilterInstance();

  uint32_t *process(float fps, uint32_t *inframe);

  bool set_parameter(int idx); ///< apply the parameter value
  bool get_parameter(int idx); ///< get the parameter value
  bool apply(Layer *lay);

  uint32_t *outframe;

  Filter *proto;

  bool active;
  bool inuse;

  unsigned intcore;
  void *core;

};

#endif
