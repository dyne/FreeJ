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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <SDL.h>

void set_debug(int lev);
int get_debug();
void set_osd(char *st);
void show_osd();
void notice(char *format, ...);
void func(char *format, ...);
void error(char *format, ...);
void act(char *format, ...);
void warning(char *format, ...);
void *jalloc(void *point,size_t size);
bool jfree(void *point);
Uint32 fastrand();
void fastsrand(Uint32 seed);
double dtime();
bool set_rtpriority(bool max);

#endif
