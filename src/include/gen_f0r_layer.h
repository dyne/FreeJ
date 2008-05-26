/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: gen_layer.h 926 2007-10-05 21:21:44Z jaromil $"
 *
 */

#ifndef __GEN_F0R_LAYER_H__
#define __GEN_F0R_LAYER_H__

#include <layer.h>

class GenF0rLayer: public Layer {
 public:
  GenF0rLayer();
  ~GenF0rLayer();
  
  bool init(Context *freej);
  bool init(Context *freej, int w, int h) { return init(freej); };

  bool open(const char *file);
  void *feed();
  bool keypress(int key);
  void close();

  FilterInstance *generator;

};

#endif
