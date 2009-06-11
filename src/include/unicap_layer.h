/*  FreeJ
 *  (c) Copyright 2008 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifndef __FREEJ_UNICAP_H__
#define __FREEJ_UNICAP_H__

#include <unicap.h>
#include <unicap_status.h>

#include <sys/types.h>

#include <context.h>

#define UNICAP_SYSTEM_CAPTURE 1
#define UNICAP_USER_CAPTURE 2


class UnicapLayer: public Layer {

 public:
  UnicapLayer();
  ~UnicapLayer();

  bool open(const char *devfile);
  bool init(Context *freej);
  bool init(Context *freej, int width, int height);
  void *feed();
  void close();

  void *feed_ready;
  void *rgba[2];
  int swap;
  int detected;

  int capture_type;

 private:

  unicap_device_t m_device;
  unicap_device_t m_device_spec;

  unicap_handle_t m_handle;

  unicap_data_buffer_t m_buffer;

  unicap_format_t m_format;

  unicap_property_t m_property;
  unicap_property_t m_property_spec;
};

#endif
