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

#define HBOUND 30
#define VBOUND 18

#define VBP 16 /* vertical bound proportion */
#define HBP 13 /* horizontal bound proportion */
#define TOPLIST 6 /* distance down from vbound where they start the vertical lists */

typedef enum { black, white, green, red, blue, yellow } colors;
typedef void (write_routine)(char*, int, int, int, int);

//class Context;

class Osd {
 private:
  int _hbound, _vbound;
  int _layersel, _filtersel;
  uint32_t newline;

  void _show_fps();
  uint32_t *fps_offset;

  void _selection();
  uint32_t *selection_offset;

  void _filterlist();
  uint32_t *filter_offset;

  void _layerlist();
  uint32_t *layer_offset;

  void _print_credits();
  uint32_t *hicredits_offset;
  uint32_t *locredits_offset;
  uint32_t *hilogo_offset;

  void _print_status();
  uint32_t *status_offset;
  void _set_color(colors col);

  uint64_t *topclean_offset;
  uint64_t *downclean_offset;

  uint32_t _color32;

  char title[64];
  bool _active;
  bool _calibrate;
  bool _credits;
  bool _fps;

  Layer *ipernaut;
  Filter *osd_water;
  Filter *osd_vertigo;

 public:
  Osd();
  ~Osd();

  void init(Context *screen);
  void print();
  void splash_screen();
  void statusmsg(char *format,...);
  bool active();
  bool calibrate();
  bool credits();
  bool fps();
  void clean();

  Context *env;

  uint32_t *print(char *text, uint32_t* pos, int hsize, int vsize);

  char status_msg[50];

};

#endif
