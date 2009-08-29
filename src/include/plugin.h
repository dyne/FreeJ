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

#ifndef __DSO_PLUGIN_H__
#define __DSO_PLUGIN_H__

#include <freej.h>
#include <linklist.h>
#include <jutils.h>

class Plugin {
  typedef int (t_init)(Geometry*);
  typedef int (t_clean)(void);
  typedef void* (t_process)(void*);
  typedef int (t_kbdin)(int);
 public:
  Plugin();
  ~Plugin();
  
  bool open(const char *path);
  void *operator[](const char *);

  int init(Geometry *sg) { 
//    func("plugin %s::init",getname());
    return (*__init)(sg); };

  int clean() {
//    func("plugin %s::clean", getname());
    return (*__clean)(); };

  void *process(void *buffo) { return (*__process)(buffo); };

  int kbd_input(int key) { return (*__kbd_input)(key); };

  char *getname() { return _name; };
  char *getauthor() { return _author; };
  char *getinfo() { return _info; };
  int getversion() { return _version; };

private:
  void _delete();
  
  t_init *__init;
  t_clean *__clean;
  t_process *__process;
  t_kbdin *__kbd_input;

  void *_handle;

  /* plugin informations */
  char *_name;
  char *_author;
  char *_info;
  int _version;
  //  int _bpp;
  char *_path;
};

#endif
