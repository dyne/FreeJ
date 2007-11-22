/*  FreeJ - Frei0r wrapper
 *  (c) Copyright 2007 Denis Rojo <jaromil@dyne.org>
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
 * "$Id: $"
 *
 */

#ifndef __FREI0R_H__
#define __FREI0R_H__


#include <vector>
#include <map>
using namespace std;


#include <linklist.h>
#include <frei0r.h>

class Filter;


class Freior: public Entry {
  friend class Filter;
  friend class GenF0rLayer;
 public:

  Freior();
  virtual ~Freior();

  int open(char *file);

  void print_info();

  f0r_plugin_info_t info;

  bool opened;

  // parameter map
  vector<f0r_param_info_t> param_infos;
  void (*f0r_set_param_value)(f0r_instance_t instance, f0r_param_t param, int param_index);

 private:

  // dlopen handle
  void *handle;
  // full .so file path
  char filename[512];


 protected:
  // Interface function pointers.
  int (*f0r_init)();
  void (*f0r_get_plugin_info)(f0r_plugin_info_t* pluginInfo);
  void (*f0r_get_param_info)(f0r_param_info_t* info, int param_index);
  f0r_instance_t (*f0r_construct)(unsigned int width, unsigned int height);
  void (*f0r_destruct)(f0r_instance_t instance);

  void (*f0r_get_param_value)(f0r_instance_t instance, f0r_param_t param, int param_index);
  void (*f0r_update)(f0r_instance_t instance, double time,
		     const uint32_t* inframe, uint32_t* outframe);
  void (*f0r_update2)(f0r_instance_t instance, double time,
                      const uint32_t* inframe1, const uint32_t* inframe2,
		      const uint32_t* inframe3, uint32_t* outframe);

};


#endif
