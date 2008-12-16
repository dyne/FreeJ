/*  FreeJ Midi Controller
 *  (c) Copyright 2006-2007 Christoph Rudorff <goil@dyne.org>
 *
 * code inspired by seqdemo.c by Matthias Nagorni
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
 */


#include <config.h>

#ifdef WITH_MIDI
#include <midi_ctrl.h>
//#include <unistd.h>
#include <alsa/asoundlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <context.h>
//#include <plugger.h>
#include <jutils.h>
#include <callbacks_js.h> // javascript
#include <jsparser_data.h>

JS(js_midi_ctrl_constructor);

DECLARE_CLASS_GC("MidiController", js_midi_ctrl_class, js_midi_ctrl_constructor,js_ctrl_gc);

JS(midi_connect);
JS(midi_connect_from);
JSFunctionSpec js_midi_ctrl_methods[] = { 
  {"connect_from", midi_connect_from, 3},
  //    {"connect", midi_connect, 4 },
  {0} 
};

JS(js_midi_ctrl_constructor) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    MidiController *midi = new MidiController();
    // assign instance into javascript object
    // initialize with javascript context
    if(! midi->init(cx, obj) ) {
        error("failed initializing midi controller");
        delete midi; return JS_FALSE;
    }
    if( ! JS_SetPrivate(cx, obj, (void*)midi) ) {
        error("failed assigning midi controller to javascript");
        delete midi; return JS_FALSE;
    }

    *rval = OBJECT_TO_JSVAL(obj);
    return JS_TRUE;
}

MidiController::MidiController():Controller() {
    set_name("Midi Controller");
    seq_handle = NULL;
    jsenv = NULL;
    jsobj = NULL;
}

MidiController::~MidiController() {
    notice("midi close client %u", seq_client_id);
    if (seq_handle)
        snd_seq_close(seq_handle);
}

JS(midi_connect_from) {
	func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
	JS_CHECK_ARGC(3);
	int res = 0;

	MidiController *midi = (MidiController *) JS_GetPrivate(cx,obj);
	if(!midi) {
		error("%u:%s:%s :: Midi core data is NULL",
		__LINE__,__FILE__,__FUNCTION__);
		return JS_FALSE;
	}

	// int snd_seq_connect_to(snd_seq_t * seq, int myport, int dest_client, int dest_port)
	JS_ARG_NUMBER(myport, 0);
	JS_ARG_NUMBER(dest_client, 1);
	JS_ARG_NUMBER(dest_port, 2);
	res = midi->connect_from(int(myport), int(dest_client), int(dest_port));

	return JS_NewNumberValue(cx, res, rval);
}

int MidiController::connect_from(int myport, int dest_client, int dest_port) {
    // Returns: 0 on success or negative error code
    int ret = snd_seq_connect_from(seq_handle, myport, dest_client, dest_port);
    if (ret != 0) {
        error("midi connect: %i %s", ret, snd_strerror(ret));
    }
    return ret;
}

int MidiController::dispatch() {
    snd_seq_event_t *ev;
    JSBool ret = JS_FALSE;

    if (jsenv == NULL) {
        error("Midi handle action: jsobj is null");
        return ret;
    }

    while (snd_seq_event_input(seq_handle, &ev) >=0) {
        func ("midi action type/channel/param/value/time/src:port/dest:port %5d/%5d/%5d/%5d/%5d/%u:%u/%u:%u", 
            ev->type, 
            ev->data.control.channel,
            ev->data.control.param,
            ev->data.control.value,
            ev->time.tick, // time
            ev->source.client, ev->source.port,
            ev->dest.client, ev->dest.port
        );

        JSBool ret;
        switch (ev->type) {
            case SND_SEQ_EVENT_CONTROLLER: 
            {
                func("midi Control event on Channel\t%2d: %5d %5d (param/value)",
                    ev->data.control.channel, ev->data.control.param, ev->data.control.value);
                jsval js_data[] = { 
                    ev->data.control.channel, ev->data.control.param, ev->data.control.value
                };
                JSCall("event_ctrl", 3, js_data, &ret);
            }
            break;

            case SND_SEQ_EVENT_PITCHBEND:
            {
                func("midi Pitchbender event on Channel\t%2d: %5d %5d   ", 
                    ev->data.control.channel, ev->data.control.param, ev->data.control.value);
                jsval js_data[] = {
                    ev->data.control.channel, ev->data.control.param, ev->data.control.value
                };
                JSCall("event_pitch", 3, js_data, &ret);
            }
            break;

            case SND_SEQ_EVENT_NOTEON:
            {
                func("midi Note On event on Channel\t%2d: %5d %5d      ",
                    ev->data.control.channel, ev->data.note.note, ev->data.note.velocity);
                jsval js_data[] = {
                    ev->data.control.channel, ev->data.note.note, ev->data.note.velocity
                };
                JSCall("event_noteon", 3, js_data, &ret);
            }
            break;

            case SND_SEQ_EVENT_NOTEOFF: 
            {
                func("midi Note Off event on Channel\t%2d: %5d      ",
                    ev->data.control.channel, ev->data.note.note);
                jsval js_data[] = {
                    ev->data.control.channel, ev->data.note.note, ev->data.note.velocity
                };
                JSCall("event_noteoff", 3, js_data, &ret);
            }
            break;
            
            case SND_SEQ_EVENT_PGMCHANGE:
            {
                func("midi PGM change event on Channel\t%2d: %5d %5d ",
                    ev->data.control.channel, ev->data.control.param, ev->data.control.value);
                jsval js_data[] = {
                    ev->data.control.channel, ev->data.control.param, ev->data.control.value
                };
                JSCall("event_pgmchange", 3, js_data, &ret);
            }
            break;
        } // switch
        snd_seq_free_event(ev);
    } // while event
    return ret;
}

/*
typedef struct snd_seq_event {
         snd_seq_event_type_t type;      
         unsigned char flags;            
         unsigned char tag;              
         unsigned char queue;            
         snd_seq_timestamp_t time;       
         snd_seq_addr_t source;          
         snd_seq_addr_t dest;            
         union {
                 snd_seq_ev_note_t note;         
                 snd_seq_ev_ctrl_t control;      
                 snd_seq_ev_raw8_t raw8;         
                 snd_seq_ev_raw32_t raw32;       
                 snd_seq_ev_ext_t ext;           
                 snd_seq_ev_queue_control_t queue; 
                 snd_seq_timestamp_t time;       
                 snd_seq_addr_t addr;            
                 snd_seq_connect_t connect;      
                 snd_seq_result_t result;        
                 snd_seq_ev_instr_begin_t instr_begin; 
                 snd_seq_ev_sample_control_t sample; 
         } data;                         
 } snd_seq_event_t;
*/

bool MidiController::init(JSContext* jsenv, JSObject *jsobj) {
    int portid;
    int result=snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
    if (result<0) {
        error("Error opening ALSA sequencer: %s\n", snd_strerror(result));
        return(false);
    }
    snd_seq_set_client_name(seq_handle, CLIENT_NAME);
    seq_client_id = snd_seq_client_id(seq_handle);
    // port name 16 chars
    if ((portid = snd_seq_create_simple_port(seq_handle, "MIDI IN",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
        error("Error creating sequencer port.\n");
        return(false);
    }
    notice("opened ALSA MIDI sequencer client-id:port #%i:%i", seq_client_id, portid);

    this->jsenv = jsenv;
    this->jsobj = jsobj;

    initialized = true;
    return(true);
}

int MidiController::poll() {
    return dispatch();
}

#endif



/*

obtain subscription information 
int snd_seq_get_port_subscription (snd_seq_t *handle, snd_seq_port_subscribe_t *sub)

subscribe a port connection 
int   snd_seq_subscribe_port (snd_seq_t *handle, snd_seq_port_subscribe_t *sub)
unsubscribe a connection between ports
int   snd_seq_unsubscribe_port (snd_seq_t *handle, snd_seq_port_subscribe_t *sub)

void capture_keyboard(snd_seq_t *seq)
{
        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;
        sender.client = 64;
        sender.port = 0;
        dest.client = 128;
        dest.port = 0;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_port_subscribe_set_queue(subs, 1);
        snd_seq_port_subscribe_set_time_update(subs, 1);
        snd_seq_port_subscribe_set_time_real(subs, 1);
        snd_seq_subscribe_port(seq, subs);
}

void coupling(snd_seq_t *seq)
{
        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;
        sender.client = 128;
        sender.port = 0;
        dest.client = 129;
        dest.port = 0;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_subscribe_port(seq, subs);
}

*/
