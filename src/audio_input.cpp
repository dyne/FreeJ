/*  FreeJ
 *
 *  audio pipes using Portaudio v19
 *
 *  (c) Copyright 2005 Silvano Galliani <kysucix@dyne.org>
 *                2007 Denis Rojo <jaromil@dyne.org>
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

#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <audio_input.h>
#include <jutils.h>


int audio_callback(const void *inputBuffer, void *outputBuffer,
		   unsigned long framesPerBuffer,
		   const PaStreamCallbackTimeInfo *timeInfo,
		   const PaStreamCallbackFlags statusFlags, void *userData ) {

  int num;

  //  func("audio_callback returns %u frames at input time %f",
  //       framesPerBuffer, timeInfo -> inputBufferAdcTime);

  AudioInput *a = (AudioInput *) userData;
  a->frames = framesPerBuffer;
  a->bytesize = a->frames * sizeof(int16_t) * a->channels;

  num = pipe_write("audio_callback", a->input_pipe, (char*) inputBuffer, a->bytesize);
  //  if(num != a->bytesize)
  //    func("audio input pipe full, written %i instead of %u bytes", num, a->bytesize);

  //  a->output_pipe -> read  (a->bytesize , (void*)outputBuffer);

  return paContinue;
}

int pipe_read(char *name, ringbuffer_t *rb, char *dest, size_t cnt) {
  // wait until we have enough bytes to read
  while( ringbuffer_read_space(rb) < cnt ) {
    warning("%s pipe read not ready", name);
    jsleep(0,10);    
  }
  return ringbuffer_read(rb, dest, cnt);
}
int pipe_write(char *name, ringbuffer_t *rb, char *src, size_t cnt) {
  while( ringbuffer_write_space(rb) < cnt ) {
    warning("%s pipe write not ready", name);
    jsleep(0,10);
  }
  return ringbuffer_write(rb, src, cnt);
}
int pipe_peek(char *name, ringbuffer_t *rb, char *dest, size_t cnt) {
  while( ringbuffer_read_space(rb) < cnt ) {
    warning("%s pipe peek not ready", name);
    jsleep(0,10);
  }
  return ringbuffer_peek(rb, dest, cnt);
}



AudioInput::AudioInput() {
  func("creating audio input");

  input = NULL;

  input_pipe = ringbuffer_create(1024*16);

  outputParameters = NULL;
  captureParameters = NULL;
  captureDeviceInfo = NULL;
  pa_audioStream = NULL;
  
  err = paNoError;

  //  set_format(1, 44100); // default mono 44khz

  initialized = false;
  started = false;
}

AudioInput::~AudioInput() {

  Pa_Terminate();

  if(outputParameters) delete outputParameters;
  if(captureParameters) delete captureParameters;

  free(input);
  //  free(output);

  ringbuffer_free(input_pipe);
  //  delete output_pipe;

}

bool AudioInput::init() {
  act("opening audio device");
  
  const PaDeviceInfo *tmp;

  notice("Initializing audio input device");

  err = Pa_Initialize();
  if( err != paNoError ) {
    error ("Could not neither init portaudio: %s!",Pa_GetErrorText( err ));
    Pa_Terminate();
    return false;
  }
  
  // allocate parameters structure
  if(outputParameters) delete outputParameters;
  outputParameters = new PaStreamParameters;

  if(captureParameters) delete captureParameters;
  captureParameters = new PaStreamParameters;  

  
  
  //  captureParameters -> device = Pa_GetDefaultInputDevice ();
  
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
  int found = Pa_GetHostApiCount();
  if(found>1)
    act("I have found %d sound implementations", found);

  for (int i = 0; i< found ; i++) {

    pai = (PaHostApiInfo*)Pa_GetHostApiInfo(i);
    func ("Found hostApi %s", pai -> name);

    if (pai -> type == paALSA ) { // look for alsa
      act("using Advanced Linux Sound Architecture (ALSA)");
      captureParameters -> device = pai -> defaultInputDevice;
      outputParameters -> device = pai -> defaultOutputDevice;

      //			func("\t native sample rate: %f", tmp -> defaultSampleRate);
      captureDeviceInfo = (PaDeviceInfo*)Pa_GetDeviceInfo (captureParameters -> device );
      outputDeviceInfo  = (PaDeviceInfo*)Pa_GetDeviceInfo (outputParameters -> device );
      

      set_format( 1, (int) captureDeviceInfo -> defaultSampleRate);
		 //sample_rate = captureDeviceInfo -> defaultSampleRate;
      
      break;
      //		    break;
    }
  }


  if( ! outputDeviceInfo )
    warning("no audio output device found");
  else {
    // OUTPUT PARAMETERS
    outputParameters -> hostApiSpecificStreamInfo = NULL;
    outputParameters -> channelCount = channels;
    outputParameters -> sampleFormat = paInt16;
    outputParameters -> suggestedLatency = outputDeviceInfo -> defaultLowInputLatency;
  }
  
  if( ! captureDeviceInfo )
    warning("no audio input device found");
  else {
    // CAPTURE PARAMETERS
    captureParameters -> hostApiSpecificStreamInfo = NULL;
    captureParameters -> channelCount = channels; // captureDeviceInfo -> maxInputChannels;
    captureParameters -> sampleFormat = paInt16;
    captureParameters -> suggestedLatency = captureDeviceInfo -> defaultLowInputLatency;
  }

  err = Pa_OpenStream( &pa_audioStream,
		       captureParameters, 
		       NULL, //		       outputParameters,
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
		       //		       paNoFlag,
		       audio_callback, 
		       this );
  
  //  delete captureParameters;

  if( err != paNoError ) {
    error ("Could not open stream PortAudioClient: %s",Pa_GetErrorText( err ));
    Pa_Terminate();
    return false;
  }

  act("audio initialization succesful");
  initialized = true;

  return true;
  
}

  
bool AudioInput::start() {

  if(!initialized) {
    
    if(! init() ) {
      error("can't initialize any audio device");
      return false;
    }

  }

  if(!started) {
    act("start recording from audio device");
    
    err = Pa_StartStream (pa_audioStream );
    
    if ( err != paNoError ) {
      error ("Could not start audio stream : %s",Pa_GetErrorText (err ));
      Pa_Terminate();
      return false;
    }
    started = true;
  }

  return true;
}

void AudioInput::stop() {

  if(!initialized) return;

  if(started) {
    act("stop recording from audio device");
    
    err = Pa_CloseStream (pa_audioStream);
    if (err != paNoError)
      error ("Error closing audio stream");

    started = false;
  }
}

int AudioInput::cafudda() {
  int num;

  if(started) {

    num = framesperbuffer*sizeof(int16_t)*channels;

    inputframes = pipe_read("AudioInput::cafudda", input_pipe, (char*)input, num);

  }
  //  output_pipe->write(num, (void*)input);

  return(inputframes);
}

#define FPS 25
void AudioInput::set_format(int chans, int srate) {

  // TODO: check values for validity
  channels = chans;
  sample_rate = srate;

  framesperbuffer = sample_rate / FPS;

  // allocate input public buffer
  if(input) free(input);
  input = (int16_t*)calloc(framesperbuffer, sizeof(int16_t) * channels);
  if(!input)
    error("can't allocate memory for internal audio buffer: %s",strerror(errno));

  // allocate output public buffer
  //  output = (int16_t*)calloc(framesperbuffer, sizeof(int16_t) * channels);
  //  if(!output)
  //    error("can't allocate memory for public audio buffer: %s",strerror(errno));
  act("audio format %u Hz channels %u", sample_rate, channels);
  func("%u frames per buffer", framesperbuffer);

}
