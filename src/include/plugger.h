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
#include <linklist.h>
#include <filter.h>

#define MAX_PLUGINS 12*12

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
     Query the plugger to find a plugin filter of a certain name

     @param name filter name
     
     @return the pointer to the Filter class instance found, or NULL
  */
  Filter *pick(char *name);

  /**
     Tell the Plugger to read again thru configured paths and updates
     the plugin table.
     
     @return number of valid Filter plugins found
  */
  int refresh();
  
  /**
     Close the plugger and deinitialize all Filter instances, usually
     this doesn't needs to be called directly: FreeJ does it at quit.
  */
  void close() { _delete(); };
  
  /**
     Return a pointer to the Filter instance of a plugin, given its
     number in the Plugger table.
     
     @param num Filter plugin position number in the Plugger

     @return the pointer to the Filter class instance found, or NULL
  */
  Filter *operator[](const int num);
  

  Filter *plugs[MAX_PLUGINS];


 private:

  /* clears up the whole plugs list */
  int _delete();
  bool _add_plug(Filter *f);
  /* checks if file/directory exist */
  bool _filecheck(const char *file);
  bool _dircheck(const char *dir);
  void _addsearchdir(const char *dir);
  void _setsearchpath(const char *path) {
    if(_searchpath) jfree(_searchpath); _searchpath = strdup(path); };
  char *_getsearchpath() { return(_searchpath); };

  char *_searchpath;

};  

#endif
