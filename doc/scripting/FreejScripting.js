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
    @type Array
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



///////////////////////////
/// LAYER VIRTUAL PARENT

/**
        This class is pure virtual and this constructor is never used

        @class This class should never be used directly, it is the
	parent class for all Layer implementations and provides basic
	layer methods that are inherited by all other layers.
	@constructor
*/
function Layer() { };

//// Layer methods documentation

/** Make the layer active and visible */
function activate() { };
Layer.prototype.activate 	= activate;

/** Deactivate the layer: stop reading and feeding */
function deactivate() { };
Layer.prototype.deactivate 	= deactivate;

/** Raise the layer up one position in the chain of layers
    so that it comes in the front on top of others */
function up() { };
Layer.prototype.up = up;

/** Lower the layer down one position in the chain of layers
    so that it goes in the back under the others */
function down() { };
Layer.prototype.down = down;

/** Move the layer at the specified position in the chain of layers
    it will place the layer at the specified depth between others */
function move() { };
Layer.prototype.move = move;

/** Get the name of the layer
    @return layer name
    @type String */
function get_name() { };
Layer.prototype.get_name 	= get_name;

/** Get the full path to the file which is currently opened in the layer
    @return filename (full path)
    @type String */
function get_filename() { };
Layer.prototype.get_filename	= get_filename;

/** Select the BLIT mode for the layer
    @param {string} blit_name (in doublequotes) */
function set_blit(blit_name) { };
Layer.prototype.set_blit	= set_blit;

/** Get the BLIT mode for the layer
    @return blit name
    @type String */
function get_blit() { };
Layer.prototype.get_blit	= get_blit;

/** Set the BLIT value for the current blit
    this command will fade the value between 0 and 256
    it is useful for certain blits, for instance with ALPHA to set opacity
    @param {int} blit_value a number from 0 to 256 */
function set_blit_value() { };
Layer.prototype.set_blit_value  = set_blit_value;

/** Get the BLIT value for the current blit
    @return value from 0 to 256
    @type int */
function get_blit_value() { };
Layer.prototype.get_blit_value  = get_blit_value;

/** Set the position of the layer
    it switches the position to the desired x,y values
    chartesian axis with 0,0 at upper left corner
    @param {int} x horizontal position, from left to right
    @param {int} y vertical position, from up to down */
function set_position(x, y) { };
Layer.prototype.set_position	= set_position;

/** Slide the position of the layer
    it smoothly drags the layer until the new position is reached
    the path algorithm is simple linear for now
    @see #set_position
    @param {int} x horizontal position, from left to right
    @param {int} y vertical position, from up to down */
function slide_position(x, y) { };
Layer.prototype.slide_position  = slide_position;

/** Get the current X position of the layer
    @see #set_position
    @return x horizontal position, from left to right
    @type int */
function get_x_position() { };
Layer.prototype.get_x_position  = get_x_position;

/** Get the current Y position of the layer
    @see #set_position
    @return y horizontal position, from left to right
    @type int */
function get_y_position() { };
Layer.prototype.get_y_position  = get_y_position;

/** Get the current width of the layer
    @return width in pixels
    @type int */
function get_width() { };
Layer.prototype.get_width	= get_width;

/** Get the current height of the layer
    @return height in pixels
    @type int */
function get_height() { };
Layer.prototype.get_height	= get_height;

/** Add an effect to the layer
    it will append the effect to the chain applied to the layer
    the effect is specified by its name
    @param {string} effect_name (in doublequotes) */
function add_effect(effect_name) { };
Layer.prototype.add_effect	= add_effect;

/** Remove an effect to the layer
    it will remove the named effect from the chain of the layer
    the effect is specified by its name
    @param {string} effect_name (in doublequotes) */
function rem_effect(effect_name) { };
Layer.prototype.rem_effect	= rem_effect;

/** Rotate the layer at the selected degrees, counterclockwise
    @param {int} degrees for the rotation */
function rotate(degrees) { };
Layer.prototype.rotate		= rotate;

/** Zoom the layer by an x and y factor
    aspect ratio doesn't needs to be kept: the layer can be stretched
    @param {float} x horizontal zoom, value between 0.0 and 2.0 (1.0 = original size)
    @param {float} y vertical zoom, value between 0.0 and 2.0 (1.0 = original size) */
function zoom(x,y) { };
Layer.prototype.zoom		= zoom;

/** Spin the layer into a rotation
    it will keep rotating the layer at the spin factor
    @param {float} spin_factor value, advised boundary is -3 / +3 */
function spin(spin_factor) { };
Layer.prototype.spin		= spin;

/** List all the effects chain applied on the layer
    @return an array of strings
    @type Array */
function list_effects() { };
Layer.prototype.list_effects	= list_effects;




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





///////////////////////////////////////////////////
// GEOMETRY LAYER

/** The Geometry Layer constructor is used to create new instances of this layer
    @class The Geometry Layer lets you draw geometrical forms and
    shapes on its surface, as a vectorial drawing tool for programmers.
    This layer is well optimized for speed and good rendering.
    @author Andreas Schiffler (SDL_gfx), Jaromil
    @constructor
    @returns a new allocated Geometry Layer
*/
function GeometryLayer() { };
GeometryLayer.prototype		= new Layer();

/** Clears all the layer with the currently selected color */
function clear() { };
GeometryLayer.prototype.clear = clear;

/** Set the current color to a new value.
    This method accepts arguments in various ways:
    a single value is treated as a hexadecimal triplet (like in web pages) so you can use 0xFFEEDD;
    3 arguments are treated like an Red, Green and Blue triplet;
    4 arguments are treated like Red, Green, Blue and Alpha channel for opacity.
    @param {double} hex_color hexadecimal value in RGB format (or more arguments...) */
function color() { };
GeometryLayer.prototype.color = color;

/** Draw a pixel at x,y position with currently selected color.
    @param {int} x horizontal position, from left to right
    @param {int} y vertical position, from up to down */
function pixel(x, y) { };
GeometryLayer.prototype.pixel = pixel;

/** Draw an horizontal line from position x1,y tracing until x2 position.
    Lenght of the line will be then x2-x1.
    @param {int} x1 horizontal position at start of the line
    @param {int} x2 horizontal position at end of the line
    @param {int} y vertical position of the line */
function hline(x1,x2,y) { };
GeometryLayer.prototype.hline = hline;

/** Draw an vertical line from position x,y1 tracing until y2 position.
    Lenght of the line will be then y2-y1.
    @param {int} x horizontal position of the line
    @param {int} y1 vertical position at start of the line
    @param {int} y2 vertical position at end of the line */
function vline(x,y1,y2) { };
GeometryLayer.prototype.vline = vline;

/** Draw a rectangle.
    @param {int} x1 horizontal position of upper-left vertex
    @param {int} y1 vertical position of upper-left vertex
    @param {int} x2 horizontal position of lower-right vertex
    @param {int} y2 vertical position of lower-right vertex */
function rectangle(x1, y1, x2, y2) { };
GeometryLayer.prototype.rectangle = rectangle;

/** Draw a rectangle filled with currently selected color
    @param {int} x1 horizontal position of upper-left vertex
    @param {int} y1 vertical position of upper-left vertex
    @param {int} x2 horizontal position of lower-right vertex
    @param {int} y2 vertical position of lower-right vertex */
function rectangle_fill(x1, y1, x2, y2) { };
GeometryLayer.prototype.rectangle_fill = rectangle_fill;

/** Draw a line between two vertex.
    @param {int} x1 horizontal position of first vertex
    @param {int} y1 vertical position of first vertex
    @param {int} x2 horizontal position of second vertex
    @param {int} y2 vertical position of second vertex */
function line(x1, y1, x2, y2) { };
GeometryLayer.prototype.line = line;

/** Draw a smoothed line between two vertex.
    Antialias is used to blend the line contour
    @param {int} x1 horizontal position of first vertex
    @param {int} y1 vertical position of first vertex
    @param {int} x2 horizontal position of second vertex
    @param {int} y2 vertical position of second vertex */
function aaline(x1, y1, x2, y2) { };
GeometryLayer.prototype.aaline = aaline;

/** Draw a circle given the coordinate of it center and its radius.
    @param {int} x horizontal position of the center
    @param {int} y vertical position of the center
    @param {int} radius lenght in pixels */
function circle(x, y, radius) { };
GeometryLayer.prototype.circle = circle;

/** Draw a smoothed circle given the coordinate of it center and its radius.
    Antialias is used to blend the circle contour.
    @param {int} x horizontal position of the center
    @param {int} y vertical position of the center
    @param {int} radius lenght in pixels */
function aacircle(x, y, radius) { };
GeometryLayer.prototype.aacircle = aacircle;

/** Draw a filled circle given the coordinate of it center and its radius.
    Current color is used to fill.
    @param {int} x horizontal position of the center
    @param {int} y vertical position of the center
    @param {int} radius lenght in pixels */
function circle_fill(x, y, radius) { };
GeometryLayer.prototype.circle_fill = circle_fill;

/** Draw an ellipse.
    Given the coordinates of its center, of its horizontal and vertical radius, an ellipse is drawn.
    @param {int} x horizontal position of the center
    @param {int} y vertical position of the center
    @param {int} r_x lenght of horizontal radius
    @param {int} r_y lenght of vertical radius */
function ellipse(x, y, r_x, r_y) { };
GeometryLayer.prototype.ellipse = ellipse;

/** Draw an smoothed ellipse.  Given the coordinates of its center, of
    its horizontal and vertical radius, an antialiased ellipse is
    drawn.
    @param {int} x horizontal position of the center
    @param {int} y vertical position of the center
    @param {int} r_x lenght of horizontal radius
    @param {int} r_y lenght of vertical radius */
function aaellipse(x, y, r_x, r_y) { };
GeometryLayer.prototype.aaellipse = aaellipse;

/** Draw a filled ellipse.  Given the coordinates of its center, of
    its horizontal and vertical radius, an ellipse is drawn and filled
    with the current color.
    @param {int} x horizontal position of the center
    @param {int} y vertical position of the center
    @param {int} r_x lenght of horizontal radius
    @param {int} r_y lenght of vertical radius */
function ellipse_fill(x, y, r_x, r_y) { };
GeometryLayer.prototype.ellipse_fill = ellipse_fill;

/** Draw a pie. Given the coordinates of its center, of its radius, of
 * its start and end degrees, a pie is drawn (the uncompleted circle
 * with an angle left open, like the pacman character...)
 @param {int} x horizontal position of the center
 @param {int} y vertical position of the center
 @param {int} radius lenght of radius
 @param {int} start degree (0-360)
 @param {int} end degree (0-360) */
function pie(x, y, radius, start, end) { };
GeometryLayer.prototype.pie = pie;

/** Draw a filled pie. Given the coordinates of its center, of its
 * radius, of its start and end degrees, a pie is drawn and filled
 * with the current color.
 @param {int} x horizontal position of the center
 @param {int} y vertical position of the center
 @param {int} radius lenght of radius
 @param {int} start degree (0-360)
 @param {int} end degree (0-360) */
function pie_fill(x, y, radius, start, end) { };
GeometryLayer.prototype.pie_fill = pie_fill;

/** Draw a triangle. Given the coordinates of 3 vertices a triangle
    is drawn joining all of them.
    @param {int} x1 horizontal position of first vertex
    @param {int} y1 vertical position of first vertex
    @param {int} x2 horizontal position of second vertex
    @param {int} y2 vertical position of second vertex
    @param {int} x3 horizontal position of third vertex
    @param {int} y3 vertical position of third vertex */
function trigon() { };
GeometryLayer.prototype.trigon = trigon;

/** Draw a smoothed triangle. Given the coordinates of 3 vertices an
    antialiased triangle is drawn joining all of them.
    @param {int} x1 horizontal position of first vertex
    @param {int} y1 vertical position of first vertex
    @param {int} x2 horizontal position of second vertex
    @param {int} y2 vertical position of second vertex
    @param {int} x3 horizontal position of third vertex
    @param {int} y3 vertical position of third vertex */
function aatrigon() { };
GeometryLayer.prototype.aatrigon = aatrigon;

/** Draw a filled triangle. Given the coordinates of 3 vertices a
    triangle is drawn joining all of them and then filled with the
    current color.
    @param {int} x1 horizontal position of first vertex
    @param {int} y1 vertical position of first vertex
    @param {int} x2 horizontal position of second vertex
    @param {int} y2 vertical position of second vertex
    @param {int} x3 horizontal position of third vertex
    @param {int} y3 vertical position of third vertex */
function trigon_fill() { };
GeometryLayer.prototype.trigon_fill = trigon_fill;






///////////////////////////////////////////////////
// MOVIE LAYER

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


