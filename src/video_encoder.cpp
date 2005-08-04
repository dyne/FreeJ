/*  FreeJ
 *  (c) Copyright 2005 Silvano Galliani aka kysucix <kysucix@dyne.org>
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


#include <config.h>

#include <string.h>
#include <context.h>

#include <video_encoder.h>
#include <portaudio.h>

#define USE_PORTAUDIO_V19 // TODO XXX

#ifdef USE_PORTAUDIO_V19
int audio_callback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          const PaStreamCallbackFlags statusFlags, void *userData );
#else
int audio_callback(void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
			   PaTimestamp outTime, void *userData);
#endif


VideoEncoder::VideoEncoder(char *output_filename) {
	coda            = NULL;
	started         = false;
	stream		= true;
	env             = NULL;
	audio_started   = false;
	sample_rate	= 44100;
	write_to_disk   = true;
	set_output_name (output_filename);

	/* create pipe */
	coda = new Pipe();
	coda -> set_output_type("copy_byte");
	// blocking input and output
	coda -> set_block(true,true);
	coda -> set_block_timeout(500,500);
}

VideoEncoder::~VideoEncoder() {
	PaError err;

	err = Pa_CloseStream (pa_audioStream);
	if (err != paNoError) 
		error ("Error closing audio stream");

	Pa_Terminate ();
}
bool VideoEncoder::_init(Context *_env) {
	if(_env == NULL)
		return false;
	env=_env;


	if(!init_audio()) {
	    error("Error initing audio");
	    use_audio = false;
	}

	return true;
}
bool VideoEncoder::init_audio() {
	PaError err;

	err = Pa_Initialize();
	if( err != paNoError ) {
		error ("Could not neither init portaudio: %s!",Pa_GetErrorText( err ));
		Pa_Terminate();
		return false;
	}

#ifdef USE_PORTAUDIO_V19
	PaStreamParameters *captureParameters = NULL; 

	captureParameters = new PaStreamParameters;

	const PaDeviceInfo *captureDeviceInfo;

	captureParameters -> device = Pa_GetDefaultInputDevice ();



	const PaDeviceInfo *tmp;
	for( int i = 0; i < Pa_GetDeviceCount(); i++) {

		tmp = Pa_GetDeviceInfo( i);

		func("Device information:");
		func("\t name: %s" ,tmp -> name);
		func("\t maxInputChannels: %d", tmp -> maxInputChannels);
		func("\t maxOutputChannels: %d", tmp -> maxOutputChannels);
		func("\t native sample rate: %f", tmp -> defaultSampleRate);
	}

	/*
	 * use alsa as default audio output
	 */
	func ("I have found %d sound implementations: ", Pa_GetHostApiCount ());
	for (int i = 0; i< Pa_GetHostApiCount() ; i++) {
		const PaHostApiInfo *pai = Pa_GetHostApiInfo(i);
		func ("Found hostApi %s", pai -> name);
		if (pai -> type == paALSA ) { // look for alsa
		    func ("Ok I'm using alsa");
		    captureParameters -> device = pai -> defaultInputDevice;
		    //			func("\t native sample rate: %f", tmp -> defaultSampleRate);
		    captureDeviceInfo = Pa_GetDeviceInfo (captureParameters -> device );

		    if( captureDeviceInfo == NULL ) {
			error("captureDeviceInfo nullo");
			return false;
		    }
		    sample_rate = captureDeviceInfo -> defaultSampleRate;

		    break;
		}
	}

	captureParameters -> sampleFormat = paInt16;

	captureParameters -> hostApiSpecificStreamInfo = NULL;

	captureParameters -> channelCount = 1; // captureDeviceInfo -> maxInputChannels;

	captureParameters -> suggestedLatency = captureDeviceInfo -> defaultLowInputLatency;

	err = Pa_OpenStream( &pa_audioStream,
			captureParameters, 
			NULL,
			sample_rate, 
			paFramesPerBufferUnspecified,

			/** In a stream opened with paFramesPerBufferUnspecified, indicates that
			  input data is all silence (zeros) because no real data is available. In a
			  stream opened without paFramesPerBufferUnspecified, it indicates that one or
			  more zero samples have been inserted into the input buffer to compensate
			  for an input underflow.
			  */
			paInputUnderflow | 

			/** In a stream opened with paFramesPerBufferUnspecified, indicates that data
			  prior to the first sample of the input buffer was discarded due to an
			  overflow, possibly because the stream callback is using too much CPU time.
			  Otherwise indicates that data prior to one or more samples in the
			  input buffer was discarded.
			  */
			paInputOverflow,
//			paNoFlag,
			audio_callback, 
			coda );

	delete captureParameters;

#else	
	err = Pa_OpenStream( 
			&pa_audioStream,                // PortAudioStream** stream
			Pa_GetDefaultInputDeviceID (),  // PaDeviceID inputDevice
			1,                              // int numInputChannels,
			paInt16,                        // PaSampleFormat inputSampleFormat,
			NULL,                           // void *inputDriverInfo,
			paNoDevice,                     // PaDeviceID outputDevice,
			0,                              // int numOutputChannels,
			paInt16,                        // PaSampleFormat outputSampleFormat,
			NULL,                           // void *outputDriverInfo,
			(double) 44100,                 // double sampleRate,
			128,                            // unsigned long framesPerBuffer,
			1,                              // unsigned long numberOfBuffers,
			paClipOff|paDitherOff,          // PaStreamFlags streamFlags,
			//			paDitherOff,          // PaStreamFlags streamFlags,
			audio_callback,                 // PortAudioCallback *callback,
			coda );                         // void *userData 

#endif
	if( err != paNoError ) {
		error ("Could not open stream PortAudioClient: %s",Pa_GetErrorText( err ));
		Pa_Terminate();
		return false;
	}

	if(start_audio_stream ())
		audio_started = true;
	else
		audio_started = false;

	func ("Audio Inited correctly");
	return true;

}

bool VideoEncoder::set_output_name(char *output_filename) {
	int filename_number=1;

	if(!output_filename)
	{
		error("FFmpegEncoder:init::Invalid filename");
	}

	// save filename in the object
	filename = strdup (output_filename);

	// if it's a number handle it as a file descriptor
	if (isdigit (atoi (filename)))
		return true;

	/*
	 * Test if file exists, and append a number.
	 */
	char nuova[512];
	FILE *fp;
	
	// file already exists!
	while ( (fp = fopen(filename,"r")) != NULL) { 
		// take point extension pointer ;P
		char *point = strrchr(output_filename,'.');

		int lenght_without_extension = (point - output_filename);

		// copy the string before the  point
		strncpy (nuova,output_filename, lenght_without_extension);

		// insert -n
		sprintf (nuova + lenght_without_extension, "-%d%s", filename_number,output_filename + lenght_without_extension);
		jfree(filename);
		filename = strdup(nuova);

		fclose(fp);
		// increment number inside filename
		filename_number++;
	}

	func ("VideoEncoder:_init::filename %s saved",filename);
	return true;
}
#ifdef USE_PORTAUDIO_V19
int audio_callback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          const PaStreamCallbackFlags statusFlags, void *userData )
#else
int audio_callback(void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
			   PaTimestamp outTime, void *userData)
#endif
{
	func("audio_callback: input time %f", timeInfo -> inputBufferAdcTime);
	func("audio_callback: input time %f", timeInfo -> currentTime);

	Pipe *p = (Pipe *) userData;

	void *audio_in = (void * )inputBuffer;
	p -> write (framesPerBuffer * 2, audio_in); // sizeof(uint16_t) * number_of_channels

	return 0; // ARGH! annoying portaudio!
}
bool VideoEncoder::is_audio_inited() {
    /*
    notice("IS AUDIO INITED?");
    if(audio_started)
	notice("  YES!");
    else
	notice("  NO!");
	*/
    return audio_started;
}

bool VideoEncoder::start_audio_stream() {
    /*
    if(!use_audio)
    notice("NON USO L'AUDIO!!!!");
	if (!use_audio)
		return true;
		*/
	PaError err;

	//test if the stream has already started 
	/*
#ifdef USE_PORTAUDIO_V19
	err = Pa_IsStreamActive (pa_audioStream );
#else
	err = Pa_StreamActive (pa_audioStream );
#endif
	if (err == 1) {
//		notice("audio already active!");
		return true;
	}
	*/
	func ("Starting audio stream");

	err = Pa_StartStream (pa_audioStream );
	if( err != paNoError ) {
		error ("Could not start audio stream : %s",Pa_GetErrorText (err ));
		use_audio = false;
		audio_started = true;
		Pa_Terminate();
		return false;
	}
	return true;
}
bool VideoEncoder::stop_audio_stream() {
	if (! use_audio) 
	    return true;

	PaError err;

	func ("Stopping audio stream");

	err = Pa_StopStream( pa_audioStream );
	if( err != paNoError ) {
		error ("Could not stop audio stream : %s",Pa_GetErrorText( err ));
		Pa_Terminate();
		return false;
	}
	audio_started = false;
	return true;
}

bool VideoEncoder::set_sdl_surface(SDL_Surface *surface) {
	if(surface == NULL)
		return false;
	this->surface = surface;
	return true;
}

void VideoEncoder::handle_audio(bool audio) {
	use_audio = audio;
}

void VideoEncoder::stream_it(bool s) {
	stream = s;
}

bool VideoEncoder::is_stream() {
	return stream;
}

char *VideoEncoder::get_filename() {
	return filename;
}
