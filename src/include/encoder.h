/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <kysucix@dyne.org>
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
 */

#ifndef __ENCODER_h__
#define __ENCODER_h__

#include <SDL.h>
#include <config.h>
#include <linklist.h>
#include <jutils.h>
#include <jsync.h>

class Context;

/**
 * Class describing the general interface of an encoder
 * Method implemented are:
 *   - Encoder::set_output_name()
 *   - Encoder::set_sdl_surface()
 *
 *   Virtual method to be implemented in an implementation:
 *   - Encoder::init
 *   - Encoder::set_encoding_parameter
 *   - Encoder::write_frame
 *
 * @brief Layer parent abstract class
*/

class Encoder: public Entry,public JSyncThread{

 public:
  
  Encoder(char *output_filename);
  virtual ~Encoder();
  
  virtual void set_encoding_parameter()=0;
  virtual bool write_frame()=0;
  virtual bool has_finished_frame()=0;
  virtual bool isStarted()=0;
  virtual bool init(Context *_env)=0;
  
  bool set_output_name(char * output_filename);
  bool set_sdl_surface(SDL_Surface *surface);
  char *get_filename();
  
 protected:
  char *filename;
  bool started;

  SDL_Surface *surface;
  
  Context *env;

  bool _init(Context *_env);
};

#endif
