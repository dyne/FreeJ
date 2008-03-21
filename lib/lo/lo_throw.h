/*
 *  Copyright (C) 2004 Steve Harris
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  $Id: lo_throw.h 2 2004-08-07 22:20:58Z theno23 $
 */

#ifndef LO_THROW_H
#define LO_THROW_H

#ifdef __cplusplus
extern "C" {
#endif

void lo_throw(lo_server s, int errnum, const char *message, const char *path);

#ifdef __cplusplus
}
#endif

#endif
