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
 -------------------------------------------------------------------------
   linked list container class

   NOTE: add and rem don't take care of deleting pointers
   that has to be done by the process that creates them and
   knows which inheriting class they are (delete is done in main)
*/

#include <iostream>

#include <jutils.h>
#include <linklist.h>
#include <config.h>

Linklist::Linklist() {
  length = 0;
  first = NULL;
  last = NULL;
  pthread_mutex_init(&mutex,NULL);
}

Linklist::~Linklist() {
  clear();
}

/* adds one element at the end of the list */
void Linklist::add(Entry *addr) {
  Entry *ptr = NULL;
  if(addr->list) addr->rem();
  lock();

  if(first==NULL) { /* that's the first entry */
    first = addr;
    first->next = NULL;
    first->prev = NULL;
    last = first;
  } else { /* add the entry to the end */
    ptr = first;
    while(ptr->next!=NULL) ptr = ptr->next;
    ptr->next = addr;
    addr->next = NULL;
    addr->prev = ptr;
    last = addr;
  }
  /* save the pointer to this list */
  addr->list = this;
  length++;
  unlock();
}

//void Linklist::prepend(Entry *addr) {
  
/* adds an element at the position specified
   if pos is out of bounds adds it at the beginning or the end
   the element occupying allready the position slides down 
   THIS FUNCTION IS NOT YET RELIABLE
*/
void Linklist::add(Entry *addr, int pos) {
  if(length<=pos) { /* adds it at the end */
    add(addr);
    return;
  }
  
  if(pos<=1) { /* adds it at the beginning */
    first->prev = addr;
    addr->next = first;
    first = addr;
  } else {
    Entry *occ = pick(pos);
    occ->prev->next = addr;
    addr->prev = occ->prev;
    occ->prev = addr;
    addr->next = occ;
  }
  length++;
}
  

/* clears the list
   i don't delete filters here because they have to be deleted
   from the procedure creating them. so this call simply discards
   the pointers stored into the linked list. OBJECTS ARE NOT FREED */
void Linklist::clear() {
  lock();
  sel(0);
  length = 0;
  first = NULL;
  last = NULL;
  unlock();
}

/* takes one element from the list
   === STARTING FROM 1 ===
   returns NULL if called with pos=0 or pos>length
   returns Entry pointer otherwise 
   this function is then overloading the operator[]
*/
Entry *Linklist::pick(int pos) {
  if((length<pos)||(pos<1)) return(NULL);
  if(pos==1) return(first);
  if(pos==length) return(last);

  Entry *ptr = first;
  int c;
  for(c=1;c<pos;c++) ptr = ptr->next;

  return(ptr);
}


/* this function is a wrapper around Entry::up()
   better to use that if you have a pointer to your Entry */
bool Linklist::moveup(int pos) {
  Entry *p = pick(pos);
  if(!p) return(false);
  return( p->up() );
}
bool Linklist::movedown(int pos) {
  Entry *p = pick(pos);
  if(!p) return(false);
  return( p->down() );
}
bool Linklist::moveto(int num, int pos) {
  Entry 
    *p = pick(num);
  if(!p) return(false);
  return( p->move(pos) );
}
/* removes one element from the list */
void Linklist::rem(int pos) {
  Entry *ptr = pick(pos);
  if(ptr==NULL) return;
  ptr->rem();
}
  
/* selects ONLY ONE, deselects the others
   use Entry::sel() if you want to do multiple selects */
void Linklist::sel(int pos) {
  int c;
  Entry *ptr = last;

  if(pos>length) return;

  for(c=length;c>0;c--) {
    if(c==pos) ptr->sel(true);
    else ptr->sel(false);
    ptr = ptr->prev;
  }
}

/* returns the last one selected
   this is supposed to be used with single selections */
Entry *Linklist::selected() {  
  int c;
  Entry *ptr = last;
  for(c=length;c>0;c--) {
    if(ptr->select) return ptr;
    ptr = ptr->prev;
  }
  return NULL;
}

Entry::Entry() {
  next = NULL;
  prev = NULL;
  list = NULL;
  select = false;
}

Entry::~Entry() {
  rem();
}


bool Entry::up() {
  if(!prev || !list) return(false);
  list->lock();

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

  list->unlock();
  return(true);
}

bool Entry::down() {
  if(!next || !list) return(false);
  list->lock();

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

  list->unlock();
  return(true);
}

bool Entry::move(int pos) {
  func("Entry::move(%i) - NEW LINKLIST MOVE");
  if(!list) return(false);
  list->lock();

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

  list->unlock();
  func("LINKLIST MOVE RETURNS SUCCESS");

  return(true);
}

void Entry::rem() {
  if(!list) return;
  list->lock();

  if(prev)
    prev->next = next;
  else list->first = next;
  
  if(next)
    next->prev = prev;
  else list->last = prev;

  list->length--;
  list->unlock();
  list = NULL;
}

void Entry::sel(bool on) {
  select = on;
}
