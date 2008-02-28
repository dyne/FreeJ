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


typedef union ViMoData {
	struct {
		char h :8;	// header 0x03

		char i :2;	// inner wheel
		char o :4;	// outer whell
		char   :2;	// pad

		char k :7;	// keys
		char   :1;	// pad

		char c :8; 	// crc (?)
	} bits;
	char data[4];
	int w;
};
/* keys:
btn_rev, 1,
btn_rec, 2,
btn_fwd, 4,
btn_pause, 8,
btn_stop, 16,
btn_play, 32,
btn_esc,  64,
btn_player,   1|16,
btn_recorder, 4|16
*/

class ViMoController: public Controller {
	friend class Context;

	public:
		ViMoController();
		virtual ~ViMoController();

		bool init(JSContext *env, JSObject *obj);
		bool open();
		bool open(char *filename);
		void close();

		int poll();
		int dispatch();

		bool activate(bool);

	private:
		char *filename;
		int fd;
		ViMoData vmd, vmd_old;
//		int JSCall(char *funcname, ...);
};

#endif



