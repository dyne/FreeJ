/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Rojo aka jaromil <jaromil@dyne.org>
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

#ifndef __MIDI_CTRL_H__
#define __MIDI_CTRL_H__

#include <config.h>
#ifdef WITH_MIDI

//#include <unistd.h>
#include <alsa/asoundlib.h>
#include <controller.h>

// #include <callbacks_js.h>
#define CLIENT_NAME "freej MidiController" 

// int _poll (pollfd * a, nfds_t b, int c) {return poll(a,b,c);};

class Context;

class MidiController: public Controller {
	public:
		MidiController();
		~MidiController();

        bool init(JSContext*, JSObject*);
        int poll();
        virtual int dispatch();
	virtual int event_ctrl(int channel, int param, int value);
	virtual int event_pitch(int channel, int param, int value);
	virtual int event_noteon(int channel, int note, int velocity);
	virtual int event_noteoff(int channel, int note, int velocity);
	virtual int event_pgmchange(int channel, int param, int value);
        int connect_from(int myport, int dest_client, int dest_port);

		//bool quit;

	private:

		//int midi_action();

		snd_seq_t *seq_handle;
        int seq_client_id;
		//int npfd;
		//struct pollfd *pfd;
};
#endif
#endif
