/*  FreeJ
 *  (c) Copyright 2006 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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

 * serial VideoMouse Controller by MrGoil (c) 2008

 */


#ifndef __ViMoCONTROLLER_H__
#define __ViMoCONTROLLER_H__

#include <controller.h>
#include <linklist.h>

class Context;
class JSContext;
class JSObject;


union ViMoData;

class ViMoController: public Controller {
	friend class Context;

	public:
		ViMoController();
		virtual ~ViMoController();

		bool open();
		bool open(char *filename);
		void close();

		int poll();
		virtual int dispatch();
		virtual void button(unsigned int button, bool state, unsigned int mask, unsigned int old_mask);
		virtual void inner_wheel(int direction, unsigned int history);
		virtual void outer_wheel(int speed, int old_speed);

	private:
		char *filename;
		int fd;
		ViMoData *vmd, *vmd_old;
		int read_pos;
		unsigned int wi_hist; // wheel history
		int wi_dir; // inner wheel -1=left; 1=right
};

#endif

