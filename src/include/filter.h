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
#include <plugin.h>

class Filter: public Plugin, public Entry {
 public:
  Filter() { initialized=false; active=true; inuse=false; };
  virtual ~Filter() { };

  //  virtual bool kbd_input(SDL_keysym *keysym) { return(false); };

  /* bool array for supported screendepth
     [0]       [1]       [2]       [3]
     8bit      16bit     24bit     32bit */
  bool supported[4];
  bool bpp_ok(Uint8 bpp) { return(supported[((bpp>>3)-1)]); };
  
  bool initialized;
  bool active;
  bool inuse;
};

#endif
