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
#include <memory>


ClosureQueue::ClosureQueue() = default;

ClosureQueue::~ClosureQueue() {
  dispose_jobs(true);
}

void ClosureQueue::add_job(Closure *job) {
  std::unique_ptr<Closure> owned(job->is_synchronized() ? nullptr : job);
  {
    std::lock_guard<std::mutex> lock(job_queue_mutex_);
    if (!closed_) {
      job_queue_.push(job);
      owned.release();
      return;
    }
  }
  job->complete();
}

Closure *ClosureQueue::get_job_() {
  std::lock_guard<std::mutex> lock(job_queue_mutex_);
  if (job_queue_.empty())
    return nullptr;
  Closure *job = job_queue_.front();
  job_queue_.pop();
  return job;
}

void ClosureQueue::do_jobs() {
  while (Closure *job = get_job_()) {
    std::unique_ptr<Closure> owned(job->is_synchronized() ? nullptr : job);
    job->run();
  }
}

void ClosureQueue::cancel_jobs() {
  dispose_jobs(false);
}

void ClosureQueue::close() {
  dispose_jobs(true);
}

void ClosureQueue::reopen() {
  std::lock_guard<std::mutex> lock(job_queue_mutex_);
  closed_ = false;
}

void ClosureQueue::dispose_jobs(bool close) {
  std::queue<Closure *> pending;
  {
    std::lock_guard<std::mutex> lock(job_queue_mutex_);
    closed_ = closed_ || close;
    pending.swap(job_queue_);
  }
  while (!pending.empty()) {
    Closure *job = pending.front();
    pending.pop();
    if (job->is_synchronized())
      job->complete();
    else
      delete job;
  }
}


ThreadedClosureQueue::ThreadedClosureQueue() {
  int r;
  running_ = true;
  if ((r=pthread_mutex_init(&cond_mutex_, NULL)) != 0)
    throw Error("Initializing cond_mutex_", r);
  if ((r=pthread_cond_init(&cond_, NULL)) != 0)
    throw Error("Initializing cond_", r);
  if ((r=pthread_attr_init(&attr_)) != 0)
    throw Error("Initializing attr_", r);
  if ((r=pthread_attr_setdetachstate(&attr_, PTHREAD_CREATE_JOINABLE)) != 0)
    throw Error("Setting attr_ joinable", r);
  if ((r=pthread_create(&thread_, &attr_, &ThreadedClosureQueue::jobs_loop_, this)) != 0)
    throw Error("Creating jobs_loop_ thread", r);
}

ThreadedClosureQueue::~ThreadedClosureQueue() {
  int r;
  pthread_mutex_lock(&cond_mutex_);
  running_ = false;
  pthread_cond_broadcast(&cond_);
  pthread_mutex_unlock(&cond_mutex_);
  if ((r=pthread_join(thread_, NULL)) != 0)
    error("In %s, pthread_join() : %s",
          __PRETTY_FUNCTION__, strerror(r));
  do_jobs();
  if ((r=pthread_attr_destroy(&attr_)) != 0)
    error("In %s, pthread_attr_destroy() : %s",
          __PRETTY_FUNCTION__, strerror(r));
  if ((r=pthread_cond_destroy(&cond_)) != 0)
    error("In %s, pthread_cond_destroy() : %s",
          __PRETTY_FUNCTION__, strerror(r));
  if ((r=pthread_mutex_destroy(&cond_mutex_)) != 0)
    error("In %s, pthread_mutex_destroy() : %s",
          __PRETTY_FUNCTION__, strerror(r));
}

void ThreadedClosureQueue::signal_() {
  int r;
  if ((r=pthread_mutex_lock(&cond_mutex_)) != 0)
    throw Error("Pre-signal locking of cond_mutex_", r);
  if ((r=pthread_cond_broadcast(&cond_)) != 0)
    throw Error("Signaling cond_", r);
  if ((r=pthread_mutex_unlock(&cond_mutex_)) != 0)
    throw Error("Post-signal unlocking of cond_mutex_", r);
}

void ThreadedClosureQueue::add_job(Closure *job) {
  ClosureQueue::add_job(job);
  signal_();
}

void *ThreadedClosureQueue::jobs_loop_(void *arg) {
  int r;
  ThreadedClosureQueue *me = (ThreadedClosureQueue *)arg;
  if ((r=pthread_mutex_lock(&me->cond_mutex_)) != 0)
    throw ThreadError("First lock of cond_mutex_", r);
  while (me->running_) {
    me->do_jobs();
    if ((r=pthread_cond_wait(&me->cond_, &me->cond_mutex_)) != 0)
    throw ThreadError("Waiting cond_", r);
  }
  if ((r=pthread_mutex_unlock(&me->cond_mutex_)) != 0)
    throw ThreadError("Final unlock for cond_mutex_", r);
  return NULL;
}

// vim:tabstop=2:expandtab:shiftwidth=2
