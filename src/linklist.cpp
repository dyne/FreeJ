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

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "linklist.h"

Linklist::Linklist() {
  length = 0;
  first = NULL;
  last = NULL;
}

Linklist::~Linklist() {
  clear();
}

/* adds one element at the end of the list */
void Linklist::add(Entry *addr) {
  Entry *ptr;

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
  length++;
}

/* adds an element at the position specified
   if pos is out of bounds adds it at the beginning or the end
   the element occupying allready the position slides down 
   THIS FUNCTION IS NOT YET RELIABLE
*/
void Linklist::add(Entry *addr, int pos) {
  if(length<pos) { /* adds it at the end */
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
  
/* removes one element from the list */
void Linklist::rem(int pos) {
  Entry *ptr = pick(pos);
  Entry *prev = ptr->prev, *next = ptr->next;
  if(ptr==NULL) return;
  /* we are on it, now take it out WITHOUT deallocating */
  if(prev!=NULL)
    prev->next = next;
  else first = next;

  if(next!=NULL)
    next->prev = prev;
  else last = prev;

  length--;
}

/* clears the list
   i don't delete filters here because they have to be deleted
   from the procedure creating them. so this call simply discards
   the pointers stored into the linked list. OBJECTS ARE NOT FREED */
void Linklist::clear() {
  length = 0;
  first = NULL;
  last = NULL;
}

/* takes one element from the list
   === STARTING FROM 1 ===
   returns NULL if called with pos=0 or pos>lenght
   returns Entry pointer otherwise */
Entry *Linklist::pick(int pos) {
  if((length<pos)||(pos==0)) return(NULL);
  if(pos==1) return(first);
  if(pos==length) return(last);

  Entry *ptr = first;
  int c;
  for(c=1;c<pos;c++) ptr = ptr->next;

  return(ptr);
}

bool Linklist::moveup(int pos) {
  if(pos<=1) return(false);
  Entry *p = pick(pos);
  Entry *prev = p->prev, *next = p->next, *pp = prev->prev;

  if(next!=NULL) next->prev = prev;
  p->next = prev;
  p->prev = pp;
  prev->next = next;
  prev->prev = p;
  if(pp!=NULL) pp->next = p;

  if(pos==2) first = p;
  if(pos==length) last = prev;
  return(true);
}

bool Linklist::movedown(int pos) {
  if(pos>=length) return(false);
  Entry *p = pick(pos);
  Entry *prev = p->prev, *next = p->next, *nn = next->next;

  if(prev!=NULL) prev->next = next;
  p->prev = next;
  p->next = nn;
  next->prev = prev;
  next->next = p;
  if(nn!=NULL) nn->prev = p;
  
  if(pos==length-1) last = p;
  if(pos==1) first = next;
  return(true);
}
