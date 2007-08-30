/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

	@author  Denis Jaromil Rojo
        @version 0.9
*/

///////////////////////
/// GLOBAL FUNCTIONS

/** Run the engine for the indicated amount of time
    @param {double} time seconds or a fraction of seconds
*/
function run(time) { };

/** Quit the running script and close all open layers */
function quit() { };


/** Print a string to standard output console
    @param {string} string text to be printed to console
*/
function echo(string) { };

/** Process instructions from another javascript file
    @param {string} filename full path to a FreeJ script
*/
function include(filename) { };

/** Execute an external program on the running system
    @param {string} program executable to be called (current PATH is searched)
    @param {string} arguments one or more arguments for the program can follow
*/
function exec(program, arguments) { };

/** Add a layer to the engine and start processing it
    @param {Layer} layer instance of the layer to be added
*/
function add_layer(layer) { };

/** Remove a layer from the engine and stop processing it
    @param {Layer} layer instance to be removed
*/
function rem_layer(layer) { };

/** List all layers currently registered and processed by the running engine
    @return array of Layer instances
    @type Array
 */
function list_layers() { };

/** Check if a "needle" string is contained inside an "haystack" string
    @param {string} haystack longer string in which we want to search for the needle
    @param {string} needle shorter string we are searching for
    @return 1 if needle is found, 0 otherwise
    @type int
*/
function strstr(haystack, needle) { };

/** List all files inside a directory
    @param {string} dir directory location to explore for files present in it
    @return array of filenames (strings) found
    @type string
*/
function scandir(dir) { };

/** Parse a textfile loading in memory all words contained in it
    @param {string} file text to be loaded in memory
    @return array of words found in the file
    @type Array
*/
function file_to_strings(file) { };

/** Register a controller for the running engine
    @param {Controller} controller instance of the controller to be registered
*/
function register_controller(controller) { };

///////////////////////////////////////////////////
// IMAGE LAYER

/**
   This constructor is used to create new instances of this layer,
   It will be then possible to load an image file using the {@link #open} method
   
   
   @class The Image Layer can load images of various formats: PNG, JPG, BMP, GIF and more
   @author Sam Lantinga (SDL_image)
   @constructor
   @returns a new allocated Image Layer
 */
function ImageLayer() { };
ImageLayer.prototype 		= new Layer();

/** Open an image file to be displayed in the ImageLayer
    @param {string} file_name full path to the image to be opened */
function open(file_name) { };
ImageLayer.prototype.open = open;



///////////////////////////////////////////////////
// PARTICLE GENERATOR LAYER

/**
    The Particle Generator constructor is used to create new instances of this layer
        @class The Particle Generator will compute and display shiny and moving particles
	@author Jaromil
	@constructor
	@returns a new allocated Particle Generator Layer
 */
function ParticleLayer() { };
ParticleLayer.prototype 	= new Layer();

/** Recalculate the particle generator flow. This algorithm is a
    combination of prime numbers and moderate randomness.  There are
    two directions which we can choose for the recalculation: more
    complex (cloud) or less complex (simple circle).

    @param {int} complexity is diminished by a value of 0 and augmented by 1 */
function blossom(complexity) { };
ParticleLayer.prototype.blossom = blossom;



///////////////////////
//////// TODO below:




///////////////////////////////////////////////////
// TEXT LAYER

/** The Text Layer constructor is used to create new instances of this layer
    @class The Text Layer renders letters, words or .txt files using true-type
    fonts, it can cycle (blinking) words in a long text.
    @author Sam Lantinga (SDL_ttf), Jaromil
    @constructor
    @returns a new allocated Text Layer
*/
function TextLayer() { };
TextLayer.prototype		= new Layer();

///////////////////////////////////////////////////
// MOVIE LAYER

/** The Movie Layer constructor is used to create new instances of this layer
    @class The Movie Layer can load video files or streams and play them back
    as a manipulable layer surface.
    @author FFMpeg, Kysucix
    @constructor
    @returns a new allocated Movie Layer
*/
function MovieLayer() { };
MovieLayer.prototype		= new Layer();


function FlashLayer() { };
FlashLayer.prototype		= new Layer();


function CamLayer() { };
CamLayer.prototype		= new Layer();

