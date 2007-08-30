/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author Jaromil
    @version 0.8
*/

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

