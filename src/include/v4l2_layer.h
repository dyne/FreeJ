/*  FreeJ
 *  (c) Copyright 2010 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __FREEJ_V4L2_CAM_H__
#define __FREEJ_V4L2_CAM_H__

#include <config.h>

#include <context.h>

class V4L2CamLayer: public Layer {
 public:
  V4L2CamLayer();
  ~V4L2CamLayer();

  bool open(const char *devfile);
  void *feed();
  void close();

 protected:
  bool _init();


 private:

  int fd;  ///< filedescriptor for video device
  int buftype;
  int renderhop; ///< renderhop is how many frames to guzzle before rendering
  int framenum; 
  void *frame;

  struct v4l2_capability capability;

  struct v4l2_input input; ///< info about current video input
  struct v4l2_standard standard;
  struct v4l2_format format; ///< Need to find out (request?) specific data format (sec 1.10.1)
  struct v4l2_requestbuffers reqbuf;
  struct v4l2_buffer buffer;
  struct bufs {
    void *start;
    size_t length;
  } *buffers;


  // allow to use Factory on this class
  FACTORY_ALLOWED;
};

#endif
