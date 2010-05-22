/*  FreeJ - Freeframe wrapper
 *  (c) Copyright 2008 - 2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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

#ifndef __FREEFRAME_FREEJ_H__
#define __FREEFRAME_FREEJ_H__

#include <linklist.h>
#include <freeframe.h>
#include <filter.h>
#include <factory.h>

class Filter;


class Freeframe: public Filter {
  friend class GeneratorLayer;
#ifdef WITH_COCOA
  friend class CVF0rLayer;
#endif
 public:

  Freeframe();
  virtual ~Freeframe();

  int type();

  int open(char *file);

  const char *description();
  const char *author();

  void print_info();
  int  get_parameter_type(int i);
  char *get_parameter_description(int i);

  bool apply(Layer *lay, FilterInstance *instance);

  PlugInfoStruct *info;

  VideoInfoStruct vidinfo;

  bool opened;

 protected:
  void destruct(FilterInstance *inst);
  void update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe);
  
  // Interface function pointers.
  plugMainType *plugmain;

  private:
    // dlopen handle
    void *handle;
    // full .so file path
    char filename[512];
    FACTORY_ALLOWED
};


#endif
