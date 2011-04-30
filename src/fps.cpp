/*  FreeJ
 *  (c) Copyright 2001-2009 Denis Roio <jaromil@dyne.org>
 *                2008-2009 Christoph Rudorff <goil@dyne.org>
 *                     2009 Andrea Guzzo <xant@xant.net>
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

#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <config.h>

#include <time.h>
#include <sys/time.h>

#include <fps.h>
#include <jutils.h>


FPS::FPS() {
  _fps = 0;
  _period = 0;
  fpsd.sum = 0;
  fpsd.i = 0;
  fpsd.n = 30;
  _passes = 0.0;
  _ratio = 1.0;
  m_OrgSets = false;
  fpsd.data = new float[fpsd.n];
  gettimeofday(&start_tv,NULL);
  m_OldTime.tv_sec = start_tv.tv_sec;
  m_OldTime.tv_usec = start_tv.tv_usec;
  
  wake_ts.tv_sec = wake_ts.tv_nsec = 0;

}

FPS::~FPS() {
  free(fpsd.data);

}
void FPS::init(double rate) {


  this->set(rate);

  for (int i=0; i<30; i++) {
    fpsd.data[i] = 0;
  }

}

void FPS::calc() {

  timeval done, now_tv;
  float curr_fps;  

  gettimeofday(&now_tv, NULL);

  if(now_tv.tv_usec == start_tv.tv_usec && now_tv.tv_sec == start_tv.tv_sec) {
    // tight loop, take a minimum breath
    wake_ts.tv_sec  = 0;
    wake_ts.tv_nsec = 1000000; // set the delay
    return;
  }

  timersub(&now_tv, &start_tv, &done);
  _period = (1000000 / _fps) / _ratio;

  if ( (done.tv_sec > 0)
       || (done.tv_usec >= _period) ) {
	 start_tv.tv_sec = now_tv.tv_sec;
	 start_tv.tv_usec = now_tv.tv_usec;
	 // FIXME: statistics here, too ?!
	 return;
  }

  wake_ts.tv_sec  = 0;
  wake_ts.tv_nsec = (_period - done.tv_usec)*1000; // set the delay
  if (wake_ts.tv_nsec >= 1000000000){
    wake_ts.tv_sec = wake_ts.tv_nsec / 1000000000;
    wake_ts.tv_nsec = wake_ts.tv_nsec - (wake_ts.tv_sec * 1000000000);
  }

  // statistic only
/*  if (done.tv_usec)
      curr_fps = 1000000 /  done.tv_usec;
  else
      curr_fps = 0;

  fpsd.sum = fpsd.sum - fpsd.data[fpsd.i] + curr_fps;
  fpsd.data[fpsd.i] = curr_fps;
  if (++fpsd.i >= fpsd.n) fpsd.i = 0;*/
  
}

float FPS::get() {
  return (_fps ? fpsd.sum / fpsd.n : 0 );
}

double FPS::set(double rate) {
  func("FPS set to %f",rate);
  if (rate < 0) // invalid
    return fps_old;
  
  if (rate != _fps)
    fps_old = _fps;
  _passes = 0.0;
  fps = rate; // public
  _period = 1000000 / rate;	//_period in us
  _fps = rate;
  _ratio = 1.0;
  m_OrgSets = false;	//tells to the next delay call to catch the new starting time

  return fps_old;
}

#if 1 // use nanosleep (otherwise select_sleep)
void FPS::delay() {
  struct timespec remaining = { 0, 0 };
  timeval did;
    if (wake_ts.tv_nsec > 100000) {	//if > to 100 us
      wake_ts.tv_nsec = wake_ts.tv_nsec - 100000;	// - 100 us
    }
    do {
      if (nanosleep(&wake_ts, &remaining) == -1) {
	if (errno == EINTR) {
        // we've been interrupted use remaining and then reset it
	  wake_ts.tv_nsec = remaining.tv_nsec;
	  remaining.tv_sec = remaining.tv_nsec = 0;
	} else {
	  error("nanosleep 2 returned an error, not performing delay!, remains :%lds %ldus: %s" \
	      , remaining.tv_sec, remaining.tv_nsec, strerror (errno));
	  wake_ts.tv_sec  = wake_ts.tv_nsec = 0;
	}
      } else {
	// nanosleep successful, reset wake_ts
	wake_ts.tv_sec  = wake_ts.tv_nsec = 0;
      }
    } while (wake_ts.tv_nsec > 0);
    gettimeofday(&start_tv,NULL);
    if (!m_OrgSets) {
      m_OrgSets = true;
      m_OrgTime.tv_sec = start_tv.tv_sec;
      m_OrgTime.tv_usec = start_tv.tv_usec;
    }
    else {
      _passes++;
      _ratio = ((((start_tv.tv_sec - m_OrgTime.tv_sec) * 1000000) + (start_tv.tv_usec - m_OrgTime.tv_usec)) \
      / _passes) / _period;
      if (_ratio > 1.02)
	_ratio = 1.02;
      else if (_ratio < 0.98)
	_ratio = 0.98;
    }
    timersub(&start_tv, &m_OldTime, &did);
  m_OldTime.tv_sec = start_tv.tv_sec;
  m_OldTime.tv_usec = start_tv.tv_usec;
}
#else 

void FPS::delay() {
  select_sleep(wake_ts.tv_sec * 1000000 + wake_ts.tv_nsec/1000);
}
#endif

void FPS::select_sleep (long usec) {
       fd_set fd;
       int max_fd=0;
       struct timeval tv;
       FD_ZERO(&fd);
       tv.tv_sec = 0; tv.tv_usec = usec;
       select(max_fd, &fd, NULL, NULL, &tv);
}
