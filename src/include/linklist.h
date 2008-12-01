/*  C++ Linked list class, threadsafe (boolean is atom)
 *
 *  (c) Copyright 2001-2006 Denis Rojo <jaromil@dyne.org>
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

#ifndef __linklist_h__
#define __linklist_h__


// uncomment to have mutex locked operations
// can be slow on OSX and adds dependency to pthreads
#define THREADSAFE 1

#ifdef THREADSAFE
#include <pthread.h>
#endif

// maximum number of members returned by the completion
#define MAX_COMPLETION 512

template <class T>
class Linklist {
  friend class Entry;
 public:
  Linklist();
  virtual ~Linklist();

  T *begin() { return(first); };
  T *end() { return(last); };
  int len() { return(length); };
  
  void append(T *addr);
  void prepend(T *addr);
  void insert(T *addr, int pos);
  void insert_after(T *addr, T *pos);  
  void rem(int pos);
  void sel(int pos);
  void clear();
  bool moveup(int pos);
  bool movedown(int pos);
  bool moveto(int num, int pos);
  T *pick(int pos);  
  T *search(const char *name, int *idx);
  T **completion(char *needle);

  T *selected();

  T *operator[](int pos) { return pick(pos); };

  /* don't touch these directly */
  T *first;
  T *last;
  int length;

#ifdef THREADSAFE
  void lock() { pthread_mutex_lock(&mutex); };
  void unlock() { pthread_mutex_unlock(&mutex); };
#endif

 protected:
  T *selection;

 private:

#ifdef THREADSAFE
  pthread_mutex_t mutex;
  pthread_mutexattr_t mattr;
#endif
  
  T *compbuf[512]; // completion buffer
};

class Entry {
  //  friend class Linklist<Entry>;

 public:
  Entry();
  ~Entry();

  void set_name(const char *nn);
  
  Entry *next;
  Entry *prev;

  Linklist<Entry> *list;

  bool up();
  bool down();
  bool move(int pos);
  void rem();
  void sel(bool on);
  
  bool select;

  char *name;

  // generic data pointer, so far only used in console
  // and now also as JSObject -> jsval
  void *data; 
};

#endif
