#ifndef __MIDI_CTRL_H__
#define __MIDI_CTRL_H__

#include <unistd.h>
#include <alsa/asoundlib.h>
#include <jsync.h>

#define CLIENT_NAME "freej control" 

class Context;

class MidiControl : public JSyncThread {
	public:
		MidiControl();
		~MidiControl();

		bool init(Context *context);
		void run();

		bool quit;

	private:
		void midi_action();
		void debug();		

		snd_seq_t *seq_handle;
		int npfd;
		struct pollfd *pfd;
		Context *env;
};
#endif
