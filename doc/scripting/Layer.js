/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author Jaromil
    @version 0.8
*/


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

/** Add a frei0r effect filter to the layer
    it will append the filter to the chain applied to the layer
    the filter is specified by its name
    @param {string} filter_name (in doublequotes) */
function add_filter(filter_name) { };
Layer.prototype.add_filter	= add_filter;

/** Remove a filter from the layer
    it will remove the named filter from the chain of the layer
    the filter is specified by its name
    @param {string} filter_name (in doublequotes) */
function rem_filter(filter_name) { };
Layer.prototype.rem_filter = rem_filter;

/** Rotate the layer at the selected degrees, counterclockwise
    @param {int} degrees for the rotation */
function rotate(degrees) { };
Layer.prototype.rotate		= rotate;

/** Zoom the layer by an x and y factor
    aspect ratio doesn't needs to be kept: the layer can be stretched
    zoom(val) = zoom(val,val). A value of 1 disables the zoom processing.
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
function list_filters() { };
Layer.prototype.list_filters = list_filters;

