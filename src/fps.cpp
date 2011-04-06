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

#include <stdlib.h>
#include <errno.h>

#include <config.h>

#include <time.h>
#include <sys/time.h>

#include <fps.h>
#include <jutils.h>


FPS::FPS() {
  _fps = 0;

  fpsd.sum = 0;
  fpsd.i = 0;
  fpsd.n = 30;  
  fpsd.data = new float[fpsd.n];
  gettimeofday(&start_tv,NULL);

  wake_ts.tv_sec = wake_ts.tv_nsec = 0;

  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread feed mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread feed condtition"); 

}

FPS::~FPS() {
  free(fpsd.data);

  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread feed mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread feed attribute");
  
}
void FPS::init(float rate) {


  this->set(25);

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
  int rate = 1000000 / _fps;

  if ( (done.tv_sec > 0)
       || (done.tv_usec >= rate) ) {
	 start_tv.tv_sec = now_tv.tv_sec;
	 start_tv.tv_usec = now_tv.tv_usec;

	 // FIXME: statistics here, too ?!
	 return;
  }

  wake_ts.tv_sec  = 0;
  wake_ts.tv_nsec = (rate - done.tv_usec)*1000; // set the delay

  // statistic only
  if (done.tv_usec)
      curr_fps = 1000000 /  done.tv_usec;
  else
      curr_fps = 0;

  fpsd.sum = fpsd.sum - fpsd.data[fpsd.i] + curr_fps;
  fpsd.data[fpsd.i] = curr_fps;
  if (++fpsd.i >= fpsd.n) fpsd.i = 0;
  
}

float FPS::get() {
  return (_fps ? fpsd.sum / fpsd.n : 0 );
}

float FPS::set(float rate) {
  func("FPS set to %f",rate);
  if (rate < 0) // invalid
    return fps_old;
  
  if (rate != _fps)
    fps_old = _fps;
  
  fps = rate; // public
  _fps = rate;
  if (_fps > 0)
    _delay = 1/_fps;
  return fps_old;
}

#if 1 // use nanosleep (otherwise select_sleep)
void FPS::delay() {
  struct timespec remaining = { 0, 0 };

  do {
    if (nanosleep(&wake_ts, &remaining) == -1) {
      if (errno == EINTR) {
        // we've been interrupted use remaining and then reset it
        wake_ts.tv_nsec = remaining.tv_nsec;
        remaining.tv_sec = remaining.tv_nsec = 0;
      } else {
        error("nanosleep returned an error, not performing delay!");
        wake_ts.tv_sec  = wake_ts.tv_nsec = 0;
      }
    } else {
      // nanosleep successful, reset wake_ts
      wake_ts.tv_sec  = wake_ts.tv_nsec = 0;
    }
  } while (wake_ts.tv_nsec > 0);
  // update lo start time
  gettimeofday(&start_tv,NULL);
  
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
