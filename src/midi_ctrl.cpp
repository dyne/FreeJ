/* seqdemo.c by Matthias Nagorni */
/* midi_ctrl.cpp by C. Rudorff */

#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <plugger.h>
#include <jutils.h>
#include <midi_ctrl.h>

#include <config.h>

MidiControl::MidiControl() {
	quit=false;
	env = NULL;
	seq_handle = NULL;
}

MidiControl::~MidiControl() {
}

void MidiControl::midi_action() {

  snd_seq_event_t *ev;
  do {
    snd_seq_event_input(seq_handle, &ev);

func ("midi action type/channel/param/value %5d \t %5d \t %5d \t %5d", 
		ev->type, 
		ev->data.control.channel,
		ev->data.control.param,
		ev->data.control.value);

switch (ev->type) {
      case SND_SEQ_EVENT_CONTROLLER: 
        notice("Control event on Channel\t%2d: %5d %5d (param/value)",
                ev->data.control.channel, ev->data.control.param, ev->data.control.value);
        break;
      case SND_SEQ_EVENT_PITCHBEND:
        notice("Pitchbender event on Channel\t%2d: %5d %5d   ", 
                ev->data.control.channel, ev->data.control.param, ev->data.control.value);
        break;
      case SND_SEQ_EVENT_NOTEON:
        notice("Note On event on Channel\t%2d: %5d %5d      ",
                ev->data.control.channel, ev->data.note.note, ev->data.note.velocity);
        break;        
      case SND_SEQ_EVENT_NOTEOFF: 
        notice("Note Off event on Channel\t%2d: %5d      ",         
                ev->data.control.channel, ev->data.note.note);           
        break;        
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
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
bool MidiControl::init(Context *context) {

  int portid;

  int result=snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
  if (result<0) {
    notice("Error opening ALSA sequencer: %s\n", snd_strerror(result));
    return(false);
  }
  snd_seq_set_client_name(seq_handle, CLIENT_NAME);
  if ((portid = snd_seq_create_simple_port(seq_handle, "MIDI IN",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    func("Error creating sequencer port.\n");
    return(false);
  }
  this->env = context;
  this->seq_handle = seq_handle;
  start();
  notice("opened ALSA MIDI sequencer port #%i", portid);
  return(true);
}

void MidiControl::run() {
  int npfd;
  struct pollfd *pfd;
	
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

  while (!quit) {
    if (poll(pfd, npfd, 100000) > 0) {
      midi_action();
    }  
  }
}


