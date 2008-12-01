/*  C++ Linked list class, threadsafe (boolean is atom)
 *
 *  (c) Copyright 2001-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 *
 * "$Id$"
 *
 -------------------------------------------------------------------------
   linked list container class

   NOTE: add and rem don't take care of deleting pointers
   that has to be done by the process that creates them and
   knows which inheriting class they are (delete is done in main)
*/

#include <stdlib.h>
#include <string.h>

#include <jutils.h>
#include <linklist.h>



Entry::Entry() {
  next = NULL;
  prev = NULL;
  list = NULL;
  data = NULL;
  select = false;
  name = (char*)calloc(256, sizeof(char));
}

Entry::~Entry() {
  rem();
  if(data) free(data);
  free(name);
}

void Entry::set_name(const char *nn) {
  strncpy(name,nn,255);
}

bool Entry::up() {
  if(!list) return(false);
  if(!prev) return(false);

#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tprev = prev,
    *tnext = next,
    *pp = prev->prev;

  if(!next)
    list->last = prev;

  if(tnext)
    tnext->prev = tprev;

  next = tprev;
  prev = pp;
  tprev->next = tnext;
  tprev->prev = this;

  if(pp)
    pp->next = this;

  if(!prev)
    list->first = this;

#ifdef THREADSAFE
  list->unlock();
#endif
  return(true);
}

bool Entry::down() {
  if(!list) return(false);
  if(!next) return(false);

#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tprev = prev,
    *tnext = next,
    *nn = next->next;

  if(!prev)
    list->first = next;

  if(tprev)
    tprev->next = tnext;

  prev = tnext;
  next = nn;
  tnext->prev = tprev;
  tnext->next = this;
  if(nn)
    nn->prev = this;

  if(!next)
    list->last = this;

#ifdef THREADSAFE
  list->unlock();
#endif
  return(true);
}

bool Entry::move(int pos) {
  func("Entry::move(%i) - NEW LINKLIST MOVE, TRYING IT...");
  if(!list) return(false);
#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tn, *tp;

  Entry *swapping = list->pick(pos);
  if(swapping == this) return(true);
  if(!swapping) return(false);

  tn = swapping->next;
  tp = swapping->prev;

  swapping->next = next;
  swapping->prev = prev;
  if(next) next->prev = swapping;
  else list->last = swapping;
  if(prev) prev->next = swapping;
  else list->first = swapping;

  next = tn;
  prev = tp;
  if(next) next->prev = this;
  else list->last = this;
  if(prev) prev->next = this;
  else list->first = this;

#ifdef THREADSAFE
  list->unlock();
#endif
  func("LINKLIST MOVE RETURNS SUCCESS");

  return(true);
}

void Entry::rem() {
  bool lastone = false;
  if(!list) return;
#ifdef THREADSAFE
  list->lock();
#endif

  if(next) { // if there is a next
    next->prev = prev; // link it to the previous
    next->select = select; // inherit selection
    list->selection = next;
  } else {
    list->last = prev; // else just make it the last
    lastone = true;
    list->selection = prev;
  }

  if(prev) { // if there is a previous
    prev->next = next; // link it to the next
    if(lastone) prev->select = select;
  } else list->first = next; // else just make it a first
  
  list->length--;
  prev = NULL;
  next = NULL;
#ifdef THREADSAFE
  list->unlock();
#endif
  list = NULL;
}

void Entry::sel(bool on) {
  if(!list) return;
  select = on;
  if(select)
    list->selection = this;
}
