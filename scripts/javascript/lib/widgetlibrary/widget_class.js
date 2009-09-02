// Widget Class for the Geo layer
// buZz, Puik. started aug. 2009

// ment to draw & maintain the widgets for the OSC controller interface
// might also end up doing the OSC part

function Widget(name,x,y,w,h,type) {

	// name (for referencing?)

	this.name = name;

	// x,y = topleft
	// w,h = size

	this.x = x;
	this.y = y;
	this.w = w;
	this.h = h;

	// type 0 = do nothing, draw nothing
	// type 1 = bang
	// type 2 = toggle

	this.type = type;

	// state 0 = off (invisible)
	// state 1 = on (default)
	// state 2 = temporary disabled

	this.state = 1;

	this.data = 0;

}

Widget.prototype.draw = function(lay) {
	// draw a widget, make sure to draw it's state!
	// lay = target geolayer

	switch(this.type) {
		case 1:
			// most overkill and ugly button ever
			// also there is no animation of the pressing atm :S

			// bang
			lay.color(255,255,255,255);
			lay.rectangle(this.x,this.y,this.x+this.w,this.y+this.h);

			// topleft
			lay.line(this.x,this.y,this.x+2,this.y+2);

			// top and left lines
			lay.line(this.x+2,this.y+2,this.x+2,this.y+this.h-2);
			lay.line(this.x+2,this.y+2,this.x+this.w-2,this.y+2);
			lay.line(this.x+1,this.y+1,this.x+1,this.y+this.h-1);
			lay.line(this.x+1,this.y+1,this.x+this.w-1,this.y+1);

			// switch color on state (E_DOESNOTWORK)
			if (this.data>0) {
				lay.color(192,192,192,255);
			} else {
				lay.color(255,255,255,255);
			}

			// bottomleft and topright corners
			lay.line(this.x+this.w,this.y,this.x+this.w-2,this.y+2);
			lay.line(this.x,this.y+this.h,this.x+2,this.y+this.h-2);

			// right and bottom lines
			lay.line(this.x+this.w-2,this.y+2,this.x+this.w-2,this.y+this.h-2);
			lay.line(this.x+2,this.y+this.h-2,this.x+this.w-2,this.y+this.h-2);
			lay.line(this.x+this.w-1,this.y+1,this.x+this.w-1,this.y+this.h-1);
			lay.line(this.x+1,this.y+this.h-1,this.x+this.w-1,this.y+this.h-1);

			// bottomright corner
			lay.line(0,0,0,255);
			lay.line(this.x+this.w,this.y+this.h,this.x+this.w-2,this.y+this.h-2);

			// middle
			lay.color(160,160,160,255);
			lay.rectangle_fill(this.x+3,this.y+3,this.x+this.w-3,this.y+this.h-3);

			// delivery! one ugly button!

			break;
		case 2:
			// toggle
			lay.color(255,255,255,255);
			lay.rectangle(this.x,this.y,this.x+this.h,this.y+this.w);
			if (this.data > 0) {
				// draw cross
				lay.color(255,255,255,255);
			} else {
				lay.color(0,0,0,255);
			}
			// commented lines are for a 'cross' style checkbox, i like the square one better ..

//			lay.line(this.x+2,this.y+2,this.x+this.h-2,this.y+this.w-2);
//			lay.line(this.x+this.h-2,this.y+2,this.x+2,this.y+this.w-2);
			lay.rectangle_fill(this.x+2,this.y+2,this.x+this.h-2,this.y+this.w-2);

			break;
		case 0:
		default:
			// do nothing
	}

	return;
}

Widget.prototype.intersects = function(x,y) {
	// is x,y within this widget?

	if ((x>this.x) && (x<(this.x+this.h) && (y>this.y) && (y<(this.y+this.w) ) ) )
		return true;
	return false;

}

Widget.prototype.change = function(b,mousestate) {

	switch(this.type) {
		case 1:
			this.data = 1;
			echo ("BANG @ "+this.name);
			break;
		case 2:
			if (this.data>0) { this.data = 0; } else { this.data = 1; }
			echo ("TOGGLE newdata: "+this.data+" @ "+this.name);
			break;
		case 0:
		default:
			// do nothing
	}

}
