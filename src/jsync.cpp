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

#include <jsync.h>
#include <jutils.h>
#include <config.h>

JSyncThread::JSyncThread() {
  deferred_calls = new ClosureQueue();
}

JSyncThread::~JSyncThread() {

  // be sure we stop the thread before destroying
  deferred_calls->close();
  stop();

  delete deferred_calls;
}

int JSyncThread::start() {
  return start_with(pthread_create);
}

int JSyncThread::start_with(CreateWorker create) {
  std::lock_guard<std::mutex> lifecycle(_lifecycle);
  if (_joinable)
    return EBUSY;

  _running.store(true);
  const int result = create(&_thread, nullptr, &JSyncThread::_run, this);
  if (result == 0)
    _joinable = true;
  else
    _running.store(false);
  return result;
}

void JSyncThread::stop() {
  std::unique_lock<std::mutex> lifecycle(_lifecycle);
  _running.store(false);
  if (_joinable && pthread_equal(_thread, pthread_self()))
    return; // The next stop on the owner thread joins this worker.
  if (_joining) {
    _stopped.wait(lifecycle, [this] { return !_joining; });
    return;
  }
  if (!_joinable)
    return;

  _joining = true;
  const pthread_t worker = _thread;
  lifecycle.unlock();
  const int result = pthread_join(worker, NULL);
  lifecycle.lock();
  _joining = false;
  if (result == 0)
    _joinable = false;
  else
    error("error joining worker: %s", strerror(result));
  _stopped.notify_all();
}

void* JSyncThread::_run(void *arg) {
  JSyncThread *me = (JSyncThread *)arg;
  //me->_running = true;	// _running is set inside and outside the thread (see start & stop)
  /*
   * In addiction to 'looped' invocation below, we execute deferred calls before
   * setting up the thread because the setup phase might take into account some
   * informations arrived within a deferred call while the thread was not
   * running.
   */
  me->deferred_calls->do_jobs();
  me->thread_setup();
  while (me->_running.load()) {
    me->deferred_calls->do_jobs();
    if (me->_running.load())
      me->thread_loop();
  }
  me->thread_teardown();
  return NULL;
}

