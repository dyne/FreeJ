/*
 * Copyright (C) 2009 - Luca Bigliardi
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <closure.h>
#include <jutils.h>


Closing::Closing() {
  if(pthread_mutex_init(&job_queue_mutex_, NULL) == -1)
    error("error initializing POSIX thread job queue mutex");
}

Closing::~Closing() {
  do_jobs(); // flush queue
  if(pthread_mutex_destroy(&job_queue_mutex_) == -1)
    error("error destroying POSIX thread job queue mutex");
}

void Closing::add_job(Closure *job) {
  pthread_mutex_lock(&job_queue_mutex_);
  job_queue_.push(job);
  pthread_mutex_unlock(&job_queue_mutex_);
}

Closure *Closing::get_job_() {
  Closure *job = NULL;
  pthread_mutex_lock(&job_queue_mutex_);
  if (!job_queue_.empty()) {
    job = job_queue_.front();
    job_queue_.pop();
  }
  pthread_mutex_unlock(&job_queue_mutex_);
  return job;
}

void Closing::do_jobs() {
  Closure *job;
  bool to_delete;
  // TODO(shammash): maybe we'll need a timed condition to exit the loop
  while ((job = get_job_()) != NULL) {
    // convention: synchronized jobs are deleted by caller
    to_delete = !job->is_synchronized();
    job->run();
    if (to_delete) delete job;
  }
}



ThreadedClosing::ThreadedClosing() {
  running_ = true;
  pthread_mutex_init(&loop_mutex_, NULL);
  pthread_create(&thread_, &attr_, &ThreadedClosing::jobs_loop_, this);
}

ThreadedClosing::~ThreadedClosing() {
  running_ = false;
  pthread_mutex_unlock(&loop_mutex_);
  pthread_join(thread_, NULL);
  pthread_mutex_destroy(&loop_mutex_);
}

void ThreadedClosing::add_job(Closure *job) {
  Closing::add_job(job);
  pthread_mutex_unlock(&loop_mutex_);
}

void *ThreadedClosing::jobs_loop_(void *arg) {
  struct timespec delay = { 0, 100000000 };
  ThreadedClosing *me = (ThreadedClosing *)arg;
  while (me->running_) {
    me->do_jobs();
    pthread_mutex_lock(&me->loop_mutex_); // we block with double locking
					  // this adds a few spare cycles but
					  // in this way we don't miss anything
    nanosleep(&delay, NULL);
  }
}

