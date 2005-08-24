/*
This is an example script by Mr.Goil
how to to Cynosure
$Id: $
*/

curColor    = 0;
curBase     = curColor;
shadowWidth = 2 ; //get_integer_resource ("shadowWidth", "Integer");
elevation   = 10; // get_integer_resource ("elevation", "Integer");
sway        = 30; //get_integer_resource ("sway", "Integer");
tweak       = 20; //get_integer_resource ("tweak", "Integer");
gridSize    = 10; //get_integer_resource ("gridSize", "Integer");
timeLeft    = 0;
ncolors     = 8; //get_integer_resource ("colors", "Colors");
iterations  = 100; //get_integer_resource ("iterations", "Iterations");
MINCELLSIZE = 50;
MINRECTSIZE = 16;
THRESHOLD = 100;

colors = new Array(0xa03000ff, 0x006000ff, 0x336000ff, 0x00ff00ff, 0xc20000ff, 0x0000c0ff, 0xc0c0c0ff, 0x30f0f0ff, 0x00ff90ff);
// should come from freej - how about resize event?
width=400; height=300;

geo = new GeometryLayer();
add_layer(geo);
Draw();

function integer(i) {
	if (i>0) 
		return Math.floor(i)
	else
		return Math.ceil(i)
}

function Draw() {
//debug("draw color3=" + colors[3]);
	var i = 0;
	while (1) {
		if (iterations > 0 && ++i >= iterations) {
			i = 0; // clearScreen
			geo.rectangle_fill(0,0,width,height,0);
//debug("clear\n");
		 }
	paint();
//debug("loop" + i + "col2:" + colors[2]);
	run(0.01);
	}
}

function paint() {
    var cellsWide, cellsHigh, cellWidth, cellHeight;
//debug ("paint");
    /* How many cells wide the grid is (equal to gridSize +/- (gridSize / 2))
     */
    cellsWide  = c_tweak(gridSize, gridSize / 2);
    /* How many cells high the grid is (equal to gridSize +/- (gridSize / 2))
     */
    cellsHigh  = c_tweak(gridSize, gridSize / 2);
    /* How wide each cell in the grid is */
    cellWidth  = width  / cellsWide;
    /* How tall each cell in the grid is */
    cellHeight = height / cellsHigh;

    /* Ensure that each cell is above a certain minimum size */

    if (cellWidth < MINCELLSIZE) {
      cellWidth  = MINCELLSIZE;
      cellsWide  = width / cellWidth;
    }

    if (cellHeight < MINCELLSIZE) {
      cellHeight = MINCELLSIZE;
      cellsHigh  = width / cellWidth;
    }
    /* fill the grid with randomly-generated cells */
    for(var i = 0; i < cellsHigh; i++) {
      /* Each row is a different color, randomly generated (but constrained) */
	var c = genNewColor();
	var fg_gc = colors[c];
//debug("pick color #" + c + " = " + fg_gc);
	  //XSetForeground(dpy, fg_gc, colors[c].pixel);

	for(var j = 0; j < cellsWide; j++) {
		var curWidth, curHeight, curX, curY;

		/* Generate a random height for a rectangle and make sure that */
		/* it's above a certain minimum size */
		curHeight = random() % (cellHeight - shadowWidth);
		if (curHeight < MINRECTSIZE)
		  curHeight = MINRECTSIZE;
		/* Generate a random width for a rectangle and make sure that
		   it's above a certain minimum size */
		curWidth  = random() % (cellWidth  - shadowWidth);
		if (curWidth < MINRECTSIZE)
		  curWidth = MINRECTSIZE;
		/* Figure out a random place to locate the rectangle within the
		   cell */
		curY = (i * cellHeight) + (random() % ((cellHeight - curHeight) -
							    shadowWidth));
		curX = (j * cellWidth) +  (random() % ((cellWidth  - curWidth) -
							    shadowWidth));

		/* Draw the shadow */
		if (elevation > 0)
		  geo.rectangle_fill(curX + elevation, curY + elevation,
			  curX + elevation + curWidth, 
			  curY + elevation + curHeight,
			  0x00000060);

		      /* Draw the edge */
		if (shadowWidth > 0)
		  geo.rectangle_fill(curX + shadowWidth, curY + shadowWidth,
		  curX + shadowWidth + curWidth, 
		  curY + shadowWidth + curHeight,
		  0x000000ff);

		//XFillRectangle(dpy, window, fg_gc, curX, curY, curWidth, curHeight);
		//geo.rectangle_fill(curX, curY, curX+curWidth, curY+curHeight, fg_gc);
		geo.rectangle_fill(curX, curY, curX+curWidth, curY+curHeight, fg_gc);

		/* Draw a 1-pixel black border around the rectangle */
		//XeDrawRectangle(dpy, window, bg_gc, curX, curY, curWidth, curHeight);
		geo.rectangle(curX, curY, curX+curWidth, curY+curHeight, 0x00000000);
	//debug("j=" + j + " color = " + fg_gc);
    }
//debug("i=" + i);
  }
}

function random() {
	return integer(Math.random() * 0xffffffff);
}


function genConstrainedColor(base, tweak) {
    var i = integer(1 + (random() % tweak));
    if (random() & 1)
      i = -i;
    i = (base + i) % ncolors;
    while (i < 0)
      i += ncolors;
    return integer(i);
}

function c_tweak(base, tweak) {
    var ranTweak = integer(random() % (2 * tweak));
    var n = (base + (ranTweak - tweak));
    if (n < 0) n = -n;
    return integer(n < 255 ? n : 255);
}

function genNewColor() {
    /* These lines handle "sway", or the gradual random changing of */
    /* colors. After genNewColor() has been called a given number of */
    /* times (specified by a random permutation of the tweak variable), */
    /* take whatever color has been most recently randomly generated and */
    /* make it the new base color. */
    if (timeLeft == 0) {
      timeLeft = c_tweak(sway, sway / 3);
      curColor = curBase;
    } else {
      timeLeft--;
    }
     
    /* If a randomly generated number is less than the threshold value,
       produce a "sport" color value that is completely unrelated to the 
       current palette. */
    if (0 == (random() % THRESHOLD)) {
      return integer(random() % ncolors);
    } else {
      curBase = genConstrainedColor(curColor, tweak);
      return integer(curBase);
    }

}


quit();
/*
REGISTER_CLASS("Layer",
REGISTER_CLASS("ParticleLayer",
REGISTER_CLASS("GeometryLayer",
REGISTER_CLASS("VScrollLayer",
REGISTER_CLASS("ImageLayer",
REGISTER_CLASS("CamLayer",
REGISTER_CLASS("MovieLayer",
REGISTER_CLASS("MovieLayer",
REGISTER_CLASS("TextLayer",
REGISTER_CLASS("Effect",

JS(cafudda) {
JS(pause) {
JS(quit) {
JS(rem_layer) {
JS(add_layer) {
JS(list_layers) {
JS(layer_list_effects) {
JS(fullscreen) {
JS(set_resolution) {
JS(stream_start) {
JS(stream_stop) {
JS(freej_scandir) {
JS(freej_echo) {
JS(freej_strstr) {
JS(debug) {
JS(rand) {
JS(srand) {
JS(layer_constructor) {
JS(effect_constructor) {
JS(entry_down) {
JS(entry_up) {
JS(entry_move) {
JS(layer_set_blit) {
JS(layer_get_blit) {
JS(layer_get_name) { 
JS(layer_get_filename) {
JS(layer_set_position) {
JS(layer_get_x_position) {
JS(layer_get_y_position) {
JS(layer_set_blit_value) {
JS(layer_get_blit_value) {
JS(layer_activate) {
JS(layer_deactivate) {
JS(layer_add_effect) {
JS(layer_rem_effect) {
JS(layer_rotate) {
JS(layer_zoom) {
JS(layer_spin) {
JS_CONSTRUCTOR("GeometryLayer",geometry_layer_constructor,GeoLayer);  
JS(geometry_layer_clear) {
JS(geometry_layer_pixel) {
JS(geometry_layer_hline) {
JS(geometry_layer_vline) {
JS(geometry_layer_rectangle) {
JS(geometry_layer_rectangle_fill) {
JS(geometry_layer_line) {
JS(geometry_layer_aaline) {
JS(geometry_layer_circle) {
JS(geometry_layer_aacircle) {
JS(geometry_layer_circle_fill) {
JS(geometry_layer_ellipse) {
JS(geometry_layer_aaellipse) {
JS(geometry_layer_ellipse_fill) {
JS(geometry_layer_pie) {
JS(geometry_layer_pie_fill) {
JS(geometry_layer_trigon) {
JS(geometry_layer_aatrigon) {
JS(geometry_layer_trigon_fill) {
JS_CONSTRUCTOR("ParticleLayer",particle_layer_constructor,GenLayer);
JS(particle_layer_blossom) {
JS_CONSTRUCTOR("ImageLayer",image_layer_constructor,ImageLayer);
JS(image_layer_open) {
JS_CONSTRUCTOR("VScrollLayer",vscroll_layer_constructor,ScrollLayer);
JS(vscroll_layer_append) {
JS(vscroll_layer_speed) {
JS(vscroll_layer_linespace) {
JS(vscroll_layer_kerning) {
JS_CONSTRUCTOR("V4lLayer",v4l_layer_constructor,V4lGrabber);
JS(v4l_layer_chan) {
JS(v4l_layer_freq) {
JS(v4l_layer_band) {
JS_CONSTRUCTOR("AviLayer",avi_layer_constructor,AviLayer);
JS(avi_layer_forward) {
JS(avi_layer_rewind) {
JS(avi_layer_mark_in) {
JS(avi_layer_mark_in_now) {
JS(avi_layer_mark_out) {
JS(avi_layer_mark_out_now) {
JS(avi_layer_getpos) {
JS(avi_layer_setpos) {
JS(avi_layer_pause) {
JS_CONSTRUCTOR("TxtLayer",txt_layer_constructor,TxtLayer);
JS(txt_layer_print) {
JS(txt_layer_size) {
JS(txt_layer_font) {
JS(txt_layer_advance) {
JS(txt_layer_blink) {
JS(txt_layer_blink_on) {
JS(txt_layer_blink_off) {
JS_CONSTRUCTOR("VideoLayer",video_layer_constructor,VideoLayer);
JS(video_layer_seek) {
JS(video_layer_forward) {
JS(video_layer_rewind) {
JS(video_layer_mark_in) {
JS(video_layer_mark_out) {
JS(video_layer_pause) {

*/
