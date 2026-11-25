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
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <closure.h>
#include <fps.h>

namespace freej_engine { struct WorkerAccess; }

class JSyncThread {
 private:
  friend struct freej_engine::WorkerAccess;
  using CreateWorker = int (*)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
  int start_with(CreateWorker create);
  
  pthread_t _thread;

  std::recursive_mutex _mutex;
  
  static void* _run(void *arg);


  std::atomic<bool> _running{false};
  std::mutex _lifecycle;
  std::condition_variable _stopped;
  bool _joinable = false;
  bool _joining = false;

 protected:

  ClosureQueue *deferred_calls;

 public:
  
  JSyncThread();
  virtual ~JSyncThread();

  int start();
  void stop();
  virtual void thread_setup() {};
  virtual void thread_loop() {};
  virtual void thread_teardown() {};

  FPS fps;

  void lock() { _mutex.lock(); };
  void unlock() { _mutex.unlock(); };

  bool is_running() { return _running.load(); };

};

#endif
