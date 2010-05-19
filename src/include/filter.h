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

#include <config.h>

#include <parameter.h>
#include <filter_instance.h>

class Layer;
#ifdef WITH_FREI0R
class Freior;
#endif
#ifdef WITH_COCOA
class CVFilter;
#endif
class Freeframe;

template <class T> class Linklist;

class Filter : public Entry {
  friend class FilterInstance;
 public:
  // supported filter types
  enum Type {
#ifdef WITH_FREI0R
    FREIOR=0,
#endif
#ifdef WITH_COCOA
    COREIMAGE=1,
#endif
    FREEFRAME=2
  };
  Filter();
  virtual ~Filter();
    
  virtual FilterInstance *new_instance();
    
  virtual FilterInstance *apply(Layer *lay);
  virtual bool apply(Layer *lay, FilterInstance *instance);

  virtual const char *description();

  virtual int get_parameter_type(int i);

  virtual char *get_parameter_description(int i);

  virtual int type()=0;
    
  bool initialized;
  bool active;
  bool inuse;

    /*
#ifdef WITH_FREI0R
  Freior *freior;
#endif
#ifdef WITH_COCOA
  CVFilter *cvfilter;
#endif
  Freeframe *freeframe;
  */
  Linklist<Parameter> parameters;

 protected:
  virtual void destruct(FilterInstance *inst);
  virtual void update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe);

  int bytesize;

};

#endif
