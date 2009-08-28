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

/* @file plugger.h
   @brief Plugin Filter dispatcher header
*/

#ifndef __plugger_h__
#define __plugger_h__

#include <string.h>

#include <jutils.h>
#include <filter.h>

template <class T> class Linklist;

class Context;

/**
   This class implements the object storing all available filter
   instances and dispatching them for FreeJ operations.
   
   It reads thru paths ($(prefix)/lib/freej and ~/.freej) looking for
   valid plugins and creates instances of them which are ready to be
   returned upon request to the host application of FreeJ controllers.
*/
class Plugger {
 public:
  Plugger(); ///< Plugger onstructor
  ~Plugger(); ///< Plugger destructor

  /**
     Tell the Plugger to read again thru configured paths and updates
     the plugin table.
     @param env the context environment where to save found plugins
     @param type the type name of the plugins to look for (ie. "filters")
     @return number of valid plugins found
  */
  int refresh(Context *env);



 private:

  bool open(Context *env, char *file);

  /* checks if file/directory exist */
  void addsearchdir(const char *dir);
  void _setsearchpath(const char *path) {
    if(_searchpath) free(_searchpath); _searchpath = strdup(path); };
  char *_getsearchpath() { return(_searchpath); };

  char *_searchpath;

};  

#endif
