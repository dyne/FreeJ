/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

	@author  Denis Jaromil Rojo
        @version 0.8.1
*/


/**
        This class is pure virtual and this constructor is never used

        @class This class should never be used directly, it is the parent class for all Layer implementations
               and provides basic layer methods that are inherited by all other layers.
	@constructor
*/
function Layer() { };
Layer.prototype.activate 	= activate;
Layer.prototype.deactivate 	= deactivate;
Layer.prototype.get_name 	= get_name;
Layer.prototype.get_filename	= get_filename;
Layer.prototype.set_blit	= set_blit;
Layer.prototype.get_blit	= get_blit;
Layer.prototype.set_blit_value  = set_blit_value;
Layer.prototype.get_blit_value  = get_blit_value;
Layer.prototype.set_position	= set_position;
Layer.prototype.slide_position  = slide_position;
Layer.prototype.get_x_position  = get_x_position;
Layer.prototype.get_y_position  = get_y_position;
Layer.prototype.get_width	= get_width;
Layer.prototype.get_height	= get_height;
Layer.prototype.add_effect	= add_effect;
Layer.prototype.rem_effect	= rem_effect;
Layer.prototype.rotate		= rotate;
Layer.prototype.zoom		= zoom;
Layer.prototype.spin		= spin;
Layer.prototype.list_effects	= list_effects;
////////////////////////////////////////////////////
//// Layer methods documentation
	/**
	    Make the layer active and visible
	 */
	function activate() { };
        /**
	    Deactivate the layer: stop reading and feeding
	 */
        function deactivate() { };

        /**
	    Get the name of the layer
	    @return the full name
	    @type String
	 */
        function get_name() { };
///////////////////////////////////////////////////


/**
    The Image Layer constructor is used to create new instances of this layer
        @class The Image Layer can load images of various formats: PNG, JPG, BMP, GIF and more
	@author Sam Lantinga (SDL_image)
	@constructor
	@returns a new allocated Image Layer
 */
function ImageLayer() { };

/**
    The Particle Generator constructor is used to create new instances of this layer
        @class The Particle Generator will compute and display shiny and moving particles
	@author Jaromil
	@constructor
	@returns a new allocated Particle Generator Layer
 */
function ParticleLayer() { };

/**
    ImageLayer is a subclass of Layer and inherits all its methods
*/
ImageLayer.prototype 		= new Layer();
ParticleLayer.prototype 	= new Layer();

