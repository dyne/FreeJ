/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author Jaromil
    @version $Id: $
*/

///////////////////////////////////////////////////
// GOOM LAYER

/** The Goom Layer constructor is user to create a new instance of Goom
    @class The GoomLayer is  drawing a trippy oscilloscope that reacts
    to  the sound  output of  an Audio  collector, the  result  can be
    controlled via various parameters.

   <div class="example">Example:
   // create the audio collector
   // args: jack_port, samplesize, samplerate
   audio = new AudioJack("xine:out_l", 2048, 44100);

   // create the Goom layer
   goom = new GoomLayer();

   // connect the audio output to the goom layer
   audio.add_output(goom);

   // as goom starts, it will call AudioCollector::fft() internally at
   // every new frame
   goom.start();

   // add the layer to the screen
   add_layer(goom);
   </div>

   This  layer needs  more documentation:  the methods  below  are not
   fully tested, if you use  it please contribute a better description
   for the behaviour of each method.

   @author Jean-Christophe Hoelt (ios-software), Jaromil
   @constructor
   @returns a new allocated Goom Layer
*/
function GoomLayer() { };

/**
   Change the Goom mode
   @param {int} modenum mode number */
function mode(modenum) { };
GoomLayer.prototype.mode = mode;

/**
   Set the middle point
   @param x horizontal coordinate
   @param y vertical coordinate
*/
function middle(x,y) { };
GoomLayer.prototype.middle = middle;

/**
   Reverse the Goom
   @param n reverse number
*/
function reverse(n) { };
GoomLayer.prototype.reverse = reverse;

/**
   Change Goom speed
   @param speed speed value
*/
function speed(speed) { };
GoomLayer.prototype.speed = speed;

/**
   Shift the Goom plane
   @param x horizontal coordinate
   @param y vertical coordinate
*/
function plane(x,y) { };
GoomLayer.prototype.plane = plane;

/**
   Set the Goom wave
   @param waveval value of wave height
*/
function wave(waveval) { };
GoomLayer.prototype.wave = wave;

/**
   Set the Goom hypercosine value
   @param hyperval value for the hypercosine transformations
*/
function hypercos(hyperval) { };
GoomLayer.prototype.hypercos = hypercos;

/**
   Set the level of noise in Goom
   @param noiselevel level of noise in Goom
*/
function noise(noiselevel) { };
GoomLayer.prototype.noise = noise;

