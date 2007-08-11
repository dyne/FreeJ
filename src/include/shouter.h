/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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

#ifndef __SHOUTER_H__
#define __SHOUTER_H__

#include <shout/shout.h>

#include <linklist.h>
#include <jutils.h>

#include <string.h>
#include <jsync.h>

#define ERRORMSG 128
#define RETRY_DELAY 60 /* time to retry connect on broken icecasts, in seconds */
#define MAX_VALUE_SIZE 512

  /**
     Macro declaration of parameters 
  */
#define CHAR_SET(func,var) \
char var[MAX_VALUE_SIZE]; \
void func(char *in) { \
if(strncmp(var,in,MAX_VALUE_SIZE)==0) return; \
else strncpy(var,in,MAX_VALUE_SIZE); \
} \
char *func() { return var; };

#define INT_SET(func,var) \
int var; \
void func(int in) { \
if(var==in) return; \
else var=in; \
} \
int func() { return var; };

#define FLOAT_SET(func,var) \
float var; \
void func(float in) { \
if(var==in) return; \
else var=in; \
} \
float func() { return var; };

class Shouter : public Entry {
 private:
  shout_t *ice;

  int errors;
  void run(); ///< Main thread loop
  
 public:
  Shouter();
  ~Shouter();

  /* ======= GUI SETTINGS HERE
     the following macros declare two functions for each variable:
     set_variable(value); assign value to variable
     get_variable(); returns value of variable */
  CHAR_SET(ip,_ip);
  CHAR_SET(host,_host);
  char streamurl[MAX_VALUE_SIZE];
  INT_SET(port,_port);
  CHAR_SET(user,_user);
  CHAR_SET(pass,_pass);
  CHAR_SET(mount,_mount);
  
  /* setted by the encoder */
  CHAR_SET(bps,_bps);
  CHAR_SET(freq,_freq);
  CHAR_SET(channels,_channels);

  INT_SET(login,_login);
  CHAR_SET(name,_name);
  CHAR_SET(url,_url);
  CHAR_SET(desc,_desc);
  int format;
  
  bool start();
  bool stop();
  int send (unsigned char *buf, unsigned int enc);
  
  bool running;
  time_t retry;

  bool apply_profile();
  bool profile_changed;

};


#endif
