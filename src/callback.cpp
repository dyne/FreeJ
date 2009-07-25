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

#include <callback.h>
#include <jutils.h>

DumbCall::DumbCall() {
  pthread_mutexattr_init(&pendingattr_);
  pthread_mutexattr_settype(&pendingattr_, PTHREAD_MUTEX_NORMAL);
  pthread_mutex_init(&pending_, &pendingattr_);
  pthread_mutexattr_init(&refcount_mutexattr_);
  pthread_mutexattr_settype(&refcount_mutexattr_, PTHREAD_MUTEX_NORMAL);
  pthread_mutex_init(&refcount_mutex_, &refcount_mutexattr_);
  refcount_ = 0;
}

DumbCall::~DumbCall() {
  pthread_mutex_lock(&pending_); // this blocks as far as we're in queue
  pthread_mutex_unlock(&pending_);
  pthread_mutex_destroy(&pending_);
  pthread_mutexattr_destroy(&pendingattr_);
  pthread_mutex_destroy(&refcount_mutex_);
  pthread_mutexattr_destroy(&refcount_mutexattr_);
}

void DumbCall::notify() {
  callback();
  dequeue();
}

void DumbCall::enqueue() {
  pthread_mutex_lock(&refcount_mutex_);
  if (refcount_ == 0)
    pthread_mutex_lock(&pending_);
  refcount_++;
  pthread_mutex_unlock(&refcount_mutex_);
}

void DumbCall::dequeue() {
  pthread_mutex_lock(&refcount_mutex_);
  refcount_--;
  if (refcount_ == 0)
    pthread_mutex_unlock(&pending_);
  pthread_mutex_unlock(&refcount_mutex_);
}

DumbCallback::DumbCallback() {
  dispatcher_ = new ThreadedClosureQueue();
}

DumbCallback::~DumbCallback() {
  delete dispatcher_;
  calls_.clear();
}

bool DumbCallback::add_call(DumbCall *call) {
  if (get_call_(call)) {
    warning("%s, callback already present", __PRETTY_FUNCTION__);
    return false;
  }
  calls_.push_back(call);
  return true;
}

bool DumbCallback::rem_call(DumbCall *call) {
  if (!get_call_(call)) {
    warning("%s, callback not present", __PRETTY_FUNCTION__);
    return false;
  }
  calls_.remove(call);
  return true;
}

void DumbCallback::notify() {
  std::list<DumbCall *>::iterator i;
  for (i=calls_.begin() ; i!=calls_.end() ; i++) {
    (*i)->enqueue();
    dispatcher_->add_job(NewClosure(*i, &DumbCall::notify));
  }
}

DumbCall *DumbCallback::get_call_(DumbCall *call) {
  DumbCall *fun = NULL;
  std::list<DumbCall *>::iterator i;
  for (i=calls_.begin() ; i!=calls_.end() ; i++)
    if (*i == call) fun = call;
  return fun;
}

// vim:tabstop=2:expandtab:shiftwidth=2
