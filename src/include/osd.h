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

#include <context.h>
#include <inttypes.h>


typedef enum { black, white, green, red, blue, yellow } colors;
typedef void (write_routine)(char*, int, int, int, int);

//class Context;

class Osd {
 private:
  int _hbound, _vbound;
  int _layersel, _filtersel;
  uint32_t newline;

  uint32_t *fps_offset;

  void _selection();
  uint32_t *selection_offset;

  void _filterlist();
  uint32_t *filter_offset;

  void _layerlist();
  uint32_t *layer_offset;

  void draw_credits();
  uint32_t *hicredits_offset;
  uint32_t *locredits_offset1;
  uint32_t *locredits_offset2;
  uint32_t *hilogo_offset;

  void _print_status();
  uint32_t *status_offset;
  void _set_color(colors col);

  uint64_t *topclean_offset;
  uint64_t *downclean_offset;

  uint32_t _color32;

  char title[64];

  bool _calibrate;
  bool _credits;
  bool _fps;

  Layer *ipernaut;
  Filter *osd_water;
  Filter *osd_vertigo;

  /* used by the font renderer */
  int y,x,i,len,f,v,ch,cv;
  uint32_t *ptr;


 public:
  Osd();
  ~Osd();

  void init(Context *screen);
  void print();
  //  void statusmsg(char *format,...);
  bool calibrate();
  bool credits(bool s);
  bool credits();
  bool fps();
  void clean();

  void _print_credits();
  void _show_fps();

  Context *env;

  uint32_t *print(char *text, uint32_t* pos, int hsize, int vsize);

  bool active;
  char status_msg[50];

};

#endif
