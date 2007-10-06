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

#ifndef __filter_h__
#define __filter_h__


#include <linklist.h>
#include <freej.h>

class Layer;
class Freior;
class FilterInstance;

#include <string>
#include <map>
using namespace std;


/* Parameter type for boolean values */
#define PARAM_BOOL      0
/* Parameter type for doubles */
#define PARAM_DOUBLE    1
/* Parameter type for color */
#define PARAM_COLOR     2
/* Parameter type for position */
#define PARAM_POSITION  3
/* Parameter type for string */
#define PARAM_STRING  4


class Filter : public Entry {
  friend class FilterInstance;
 public:
  Filter(Freior *f);
  ~Filter();

  FilterInstance *apply(Layer *lay);
  
  char *description();

  int get_parameter_type(int i);

  char *get_parameter_description(int i);

  bool set_parameter_value(FilterInstance *inst, double *value, int idx);

  bool initialized;
  bool active;
  bool inuse;

  Freior *freior;

  map<string, int> parameters;

 protected:
  void Filter::destruct(FilterInstance *inst);
  void Filter::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe);


};


class FilterInstance : public Entry {
 public:
  FilterInstance(Filter *fr);
  ~FilterInstance();

  uint32_t *process(float fps, uint32_t *inframe);

  void *core;

  uint32_t *outframe;

  Filter *proto;

  bool active;
  bool inuse;

};

// tuple of filter proto/instance
// to be saved in every javascript Filter class
class FilterDuo {
public:
  FilterDuo() {
    proto = NULL;
    instance = NULL;
  }
  ~FilterDuo() {
    if(instance) delete instance;
  }
  Filter *proto;
  FilterInstance *instance;
};

#endif
