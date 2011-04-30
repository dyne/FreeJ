/*  FreeJ
 *  (c) Copyright 2001-2009 Denis Roio <jaromil@dyne.org>
 *                2008-2009 Christoph Rudorff <goil@dyne.org>
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

#ifndef __FPS_H__
#define __FPS_H__

// #include <pthread.h>
#include <inttypes.h>

class FPS {
 public:
  FPS();
  ~FPS();

  void init(double rate);

  float get();
  double set(double rate);
  void calc();
  void delay();
  void select_sleep(long usec);

  double fps, fps_old;

 private:

  struct fps_data_t {
    int i;
    int n;
    float sum;
    float *data;
  } fpsd;

  struct timespec wake_ts;  
  double _fps, _passes, _ratio;
  struct timeval start_tv, m_OldTime, m_OrgTime;
  long int _period;
  bool m_OrgSets;
};

#endif

