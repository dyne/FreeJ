/*  FreeJ
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#ifndef __SCROLL_LAYER_H__
#define __SCROLL_LAYER_H__

#include <layer.h>

struct txtline {
  void *buf;
  char *txt;
  int y;
  int len;
  int size;
  bool rendered;
  struct txtline *next;
  struct txtline *prev;
};

class ScrollLayer: public Layer {

 public:
  ScrollLayer();  
  ~ScrollLayer();

  bool init(Context *screen);
  bool open(char *file);
  void *feed();
  bool keypress(char key);
  void close();

  void append(char *txt);

  int line_space;
  int kerning;
  int step;
  

 private:

  void render(struct txtline *l);
  int streol(char *line);
  bool _open(char *file);
  void *procbuf;
  

  struct txtline *first;
  struct txtline *last;
  int length;
  int wmax;
  int border;

};

#endif
