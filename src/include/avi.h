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

#ifndef __avi_h__
#define __avi_h__

#include <avifile/avifile.h>
#include <layer.h>

class AviLayer: public Layer {
 private:
  IAviReadFile *_avi;
  IAviReadStream *_stream;
  VideoRenderer *_rend;
  CImage *_img;
  const CodecInfo *_ci;

  unsigned int _samples_read;
  unsigned int _bytes_read;
  
  int _quality;

 public:
  AviLayer();
  ~AviLayer();
  
  bool init(Context *screen, int wdt, int hgt);
  bool open(char *file);
  void feed();
  void *get_buffer();
  void close();

  bool keypress(SDL_keysym *keysym);
};

#endif

  
