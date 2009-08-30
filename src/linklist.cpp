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
  // freeing data in entry provokes crashes in ~Layer
  // esp. when freeing Filter instances
  //  if(data) free(data);
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
    func("Entry::move(%i)", pos);
    if(!list) 
        return(false);
#ifdef THREADSAFE
    list->lock();
#endif
    Entry *displaced;

    // find our position
    Entry *search = list->first;
    int mypos = 1;
    while (search && search != this) {
        mypos++;
        search = search->next;
    }
    
    // no move is necessary
    if (mypos == pos) {
#ifdef THREADSAFE
        list->unlock();
#endif
        return(true);
    }
    displaced = list->_pick(pos);
    
    // detach ourselves from the list
    if (next) {
        next->prev = prev;
        if (prev)
            prev->next = next;
        else
            list->first = next;
    } else { 
        list->last = prev;
    }
    if (prev) {
        prev->next = next;
        if (next)
            next->prev = prev;
        else
            list->last = prev;
    
    } else {
        list->first = next;
    }
    prev = NULL;
    next = NULL;
    // now insert ourselves at the new position
    if (pos >= list->length) { // shortcut if we are going to be the last entry
        list->last->next = this;
        prev = list->last;
        list->last = this;
    } else if (pos == 1) { // shortcut if we are going to be the first entry
        list->first->prev = this;
        next = list->first;
        list->first = this;
    } else {
        if (mypos > pos) { 
            prev = displaced->prev;
            if (prev) 
                prev->next = this;
            else 
                list->first = this;
            next = displaced;
            displaced->prev = this;
        } else if (mypos < pos) {
            next = displaced->next;
            if (next)
                next->prev = this;
            else
                list->last = this;
            prev = displaced;
            displaced->next = this;
        } 
    }
#ifdef THREADSAFE
    list->unlock();
#endif
    return(true);
}

bool Entry::swap(int pos) {
  func("Entry::swap(%i) - NEW LINKLIST SWAP, TRYING IT...");
  if(!list) return(false);
#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tn, *tp;

  Entry *swapping = list->_pick(pos);

  if (!swapping) {
#ifdef THREADSAFE
    list->unlock();
#endif
      return(false);
  }
    
  if (swapping == this) {
#ifdef THREADSAFE
    list->unlock();
#endif
    return (true);
  }
    
  tn = swapping->next;
  tp = swapping->prev;

  swapping->next = (next == swapping)?this:next;
  next = (tn == this)?swapping:tn;
  swapping->prev = (prev == swapping)?this:prev;
  prev = (tp == this)?swapping:tp;

  // update head of the list if necessary
  if (!prev) {
      list->first = this;
  } else {
      prev->next = this;
      if (!swapping->prev)
          list->first = swapping;
      else
          swapping->prev->next = swapping;
  }
  // update the tail of the list if necessary
  if (!next) {
      list->last = this;
  } else {
      next->prev = this;
      if (!swapping->next)
          list->last = swapping;
      else
          swapping->next->prev = swapping;
  }  
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
    if(select) { // change selection if we are selected
      next->select = select; // inherit selection
      list->selection = next;
    }
  } else {
    list->last = prev; // else just make it the last
    lastone = true;
  }


  if(prev) { // if there is a previous
    prev->next = next; // link it to the next
    if(select) { // change selection if we are selected
      if(lastone) prev->select = select;
      list->selection = prev;
    }
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
