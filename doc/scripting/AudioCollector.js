/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author Jaromil
    @version 1.0
*/

///////////////////////////////////////////////////
/////// AUDIO COLLECTOR

/**
   <div class="desc">
   This      object      collects       audio      from      a      <a
   href="http://www.jackaudio.org">Jack</a> source, it  can be used to
   change parameter values based on the audio input.
   </div>

   <div class="desc">
   16 harmonics are provided using a fast fourier transform, returning
   values available for assignement to parameters.
   </div>

   <div class="example">Example:

   // create the audio collector
   // args: jack_port, samplesize, samplerate
   audio = new AudioJack("xine:out_l", 2048, 44100);

   // set an adequate resolution
   H=640;
   M=H/128;
   W=H/(H/80);
   set_resolution(W,H);

   // create a geometry layer to draw levels
   geo = new GeometryLayer();
   geo.start();
   add_layer(geo);
   
   // run the analisis on every frame and draw lines
   bang = new TriggerController();
   bang.frame = function() {

     // this function must be called to refresh harmonics
     audio.fft();

     for(c=0;c<16;c++) {
	hc = audio.get_harmonic(c);
	geo.vline( c*M, H, H-( hc ));
     }

   }
   </div>

   @class The Audio Jack class collects audio and analizes it for use in parameters
   @author Dave Griffiths, Jaromil
   @constructor
   @param {string} jack_port an input name like "alsa_pcm:input_1", see jack connections
   @param {int} sample_size size of the samples in bytes, best if same as jack configuration
   @param {int} sample_rate rate of the audio in input, best if same as jack configuration
   @returns a new allocated Audio Jack
*/
function AudioJack(jack_port, sample_size, sample_rate) { };


/**
   The Audio object  processes the current input with  an FFT (the cpu
   consuming part) to be  ready to provide up-to-date harmonics values
   on request of get_harmonics.
*/
function fft() { };
AudioJack.prototype.fft = fft;

/** 
    A call to get_harmonics returns  up to 16 values for each harmonic
    detected on  the audio,  as the last  time fft() was  called. This
    function provides values  that are "in sync to  the music" and can
    be used to set parameters.
    @param {int} harmonic number of the harmonic, from 1 to 16
*/
function get_harmonic(harmonic) { };
AudioJack.prototype.get_harmonic = get_harmonic;
