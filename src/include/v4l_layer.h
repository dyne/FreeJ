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

#ifndef __v4l_h__
#define __v4l_h__

/* this to avoid g++ complaining about videodev.h */
typedef unsigned long int ulong;

extern "C" {
#include <yuv2rgb.h>
}

#include <linux/types.h>
#include <linux/videodev.h>
#include <SDL_thread.h>
#include <context.h>
#include <lubrify.h>

class V4lGrabber: public Layer {
 private:
  int dev;
  int input;
  int norm;
  int _band;
  int _freq;
  Uint32 palette;

  struct video_capability grab_cap;

  struct video_mbuf grab_map;
  struct video_mmap grab_buf[2];
  int cur_frame;
  int ok_frame;
  int num_frame;

  struct video_channel grab_chan;
  struct video_picture grab_pic;
  struct video_tuner grab_tuner;

  bool grab24;
  bool have_tuner;
  int minw, minh, maxw, maxh;
  int channels;

  void *get_buffer();

  /* yuv2rgb conversion routine pointer 
     this is returned by yuv2rgb_init */
  yuv2rgb_fun *yuv2rgb;
  void *rgb_surface;
  int u,v; /* uv offset */

 public:
  V4lGrabber();
  ~V4lGrabber();
  bool detect(char *devfile);
  bool init(Context *screen, int wdt, int hgt);
  void *feed();
  void close();

  void set_chan(int ch);
  void set_band(int b);
  void set_freq(int f);
  bool keypress(SDL_keysym *keysym);

  unsigned char *buffer;
};

#endif
