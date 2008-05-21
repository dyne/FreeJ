/*  FreeJ - Freeframe wrapper
 *  (c) Copyright 2008 Denis Roio <jaromil@dyne.org>
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

#ifndef __FREEFRAME_FREEJ_H__
#define __FREEFRAME_FREEJ_H__


#include <vector>
#include <map>
using namespace std;


#include <linklist.h>
#include <freeframe.h>

class Filter;


class Freeframe: public Entry {
  friend class Filter;
  friend class GenF0rLayer;
 public:

  Freeframe();
  virtual ~Freeframe();

  int open(char *file);

  void print_info();

  PlugInfoStruct *info;

  VideoInfoStruct vidinfo;
  /*	vidinfo.frameWidth = width;
	vidinfo.frameHeight = height;
	vidinfo.orientation = 1;
	vidinfo.bitDepth = FF_CAP_V_BITS_VIDEO;  */

  bool opened;


 private:

  // dlopen handle
  void *handle;
  // full .so file path
  char filename[512];


 protected:
  // Interface function pointers.
  plugMainType *main;

};


#endif
