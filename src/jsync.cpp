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

  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");

  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_JOINABLE);

  running = false;
  quit = false;
}

JSyncThread::~JSyncThread() {

  // be sure we stop the thread before destroying
  stop();

  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");
}

int JSyncThread::start() {
  if (running) return EBUSY;
  quit = false;
  return pthread_create(&_thread, &_attr, &JSyncThread::_run, this);
}

void JSyncThread::stop() {
  if (running) {
    quit = true;
    pthread_join(_thread,NULL);
  }
}

void* JSyncThread::_run(void *arg) {
  JSyncThread *me = (JSyncThread *)arg;

  me->running = true;
  me->run();
  me->running = false;
}

