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

#ifndef __OSD_H__ 
#define __OSD_H__

#include <SDL.h>

#define VBP 16 /* vertical bound proportion */
#define HBP 13 /* horizontal bound proportion */
#define TOPLIST 6 /* distance down from vbound where they start the vertical lists */

typedef enum { black, white, green, red, blue, yellow } colors;
typedef void (write_routine)(char*, int, int, int, int);

class Context;

class Osd {
 private:
  int _hbound, _vbound;
  int _layersel, _filtersel;
  void _show_fps();
  void _selection();
  void _filterlist();
  void _layerlist();
  void _print_credits();
  void _print_status();

  void _set_color(colors col);
  void _write16(char *text, int xpos, int ypos, int hsize, int vsize);
  void _write32(char *text, int xpos, int ypos, int hsize, int vsize);
  Uint16 _color16;
  Uint32 _color32;
  write_routine Osd::*write;

  bool _active;
  bool _calibrate;
  bool _credits;
  bool _fps;
  
 public:
  Osd();
  ~Osd();

  void init(Context *screen);
  void print();
  void splash_screen();
  bool active();
  bool calibrate();
  bool credits();
  bool fps();

  Context *screen;

  char status_msg[50];

};

#endif
