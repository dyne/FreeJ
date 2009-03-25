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

#ifndef __JSYNC_H__
#define __JSYNC_H__

#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <queue>

#include <closure.h>
#include <fps.h>

class JSyncThread {
 private:
  
  pthread_t _thread;
  pthread_attr_t _attr;

  pthread_mutex_t _mutex;
  //pthread_cond_t _cond;
  
  /* mutex and conditional for the feed */
  pthread_mutex_t _mutex_feed;
  pthread_cond_t _cond_feed;

  void _run();

  FPS fps;

 public:
  
  JSyncThread();
  virtual ~JSyncThread();

  int start();
  void stop();
  virtual void run() {};

  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };

  /* MUTEX MUST BE LOCKED AND UNLOCKED WHILE USING WAIT */
  //void wait() { pthread_cond_wait(&_cond,&_mutex); };
  //void signal() { pthread_cond_signal(&_cond); };

  void wait_feed();
  void signal_feed() { pthread_cond_signal(&_cond_feed); };
  int sleep_feed();

  bool running, quit;

 protected:

  static void* kickoff(void *arg) { ((JSyncThread *) arg)->_run(); return NULL; };

  void lock_feed() { pthread_mutex_lock(&_mutex_feed); };
  void unlock_feed() { pthread_mutex_unlock(&_mutex_feed); };
  int join() { return pthread_join(_thread,NULL); }
  
};

#endif
