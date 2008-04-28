/*  FreeJ
 *  (c) Copyright 2001-2006 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#ifndef __GOOM_LAYER_H__
#define __GOOM_LAYER_H__

extern "C" {
#include <goom.h>
}

#include <layer.h>

class GoomLayer: public Layer {

 public:
  GoomLayer();
  ~GoomLayer();

  bool init(Context *freej);
  bool init(Context *freej, int w, int h) { return init(freej); };

  bool open(char *file);
  void *feed();
  bool keypress(int key);
  void close();

  PluginInfo *goom;
  
 private:

  short int audiobuf[2][512];

  SDL_Surface *surf;

  
};

#endif
