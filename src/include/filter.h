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

// supported filter types
#define FREIOR 1
#define FREEFRAME 2

class Layer;
class Freior;
class Freeframe;
class FilterInstance;
class Parameter;

/* Parameter type for boolean values */
#define PARAM_BOOL      0
/* Parameter type for doubles */
#define PARAM_NUMBER    1
/* Parameter type for color */
#define PARAM_COLOR     2
/* Parameter type for position */
#define PARAM_POSITION  3
/* Parameter type for string */
#define PARAM_STRING  4

typedef void (layer_param_f)(Layer *lay, Parameter *param, int idx);
typedef void (filter_param_f)(FilterInstance *filt, Parameter *param, int idx);

class Parameter : public Entry {
  friend class Iterator;
  // TODO: different iterator beahaviour for different parameter types
 public:
  Parameter(int param_type);
  ~Parameter();

  bool set(void *val);
  bool parse(char *p);

  int type;

  const char *description;

  void *value;

  layer_param_f *layer_func;
  filter_param_f *filter_func;
};

class Filter : public Entry {
  friend class FilterInstance;
 public:
  Filter(int type, void *filt);
  ~Filter();

  FilterInstance *apply(Layer *lay);
  
  char *description();

  int get_parameter_type(int i);

  char *get_parameter_description(int i);

  bool initialized;
  bool active;
  bool inuse;

  int backend;
  Freior *freior;
  Freeframe *freeframe;
  
  Linklist parameters;

 protected:
  void destruct(FilterInstance *inst);
  void update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe);

};


class FilterInstance : public Entry {
  friend class Filter;

 public:
  FilterInstance(Filter *fr);
  ~FilterInstance();

  uint32_t *process(float fps, uint32_t *inframe);

  bool set_parameter(int idx); ///< apply the parameter value
  
  uint32_t *outframe;

  Filter *proto;

  bool active;
  bool inuse;

  void *core;

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
