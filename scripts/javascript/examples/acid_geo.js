// dinges met iets acid-achtigs, met vierkantjes en rondjes ofzo, geen idee nog precies, maar iig 'jaren 90-ish'
// buZz, Puik.

W=get_width();
H=get_height();

echo("w=" + W);
//set_resolution(W,H);

kbd = new KeyboardController();

srand();

acidpalet = new Array(6);

for (i = 0; i < acidpalet.length ; ++i) {

	acidpalet [i] = new Array(3);

}

//viez blauw
acidpalet [0][0] = "0";
acidpalet [0][1] = "159";
acidpalet [0][2] = "247";

//supergeel
acidpalet [1][0] = "255";
acidpalet [1][1] = "245";
acidpalet [1][2] = "0";

//ranziggroen
acidpalet [2][0] = "46";
acidpalet [2][1] = "255";
acidpalet [2][2] = "84";

//gayroze
acidpalet [3][0] = "254";
acidpalet [3][1] = "0";
acidpalet [3][2] = "124";

//gewoon wit, mja, vet acid weetje
acidpalet [4][0] = "255";
acidpalet [4][1] = "255";
acidpalet [4][2] = "255";

//mehoranje
acidpalet [5][0] = "255";
acidpalet [5][1] = "168";
acidpalet [5][2] = "0";

//encoder = new VideoEncoder(50,             64000,        0,             0);
//register_encoder(encoder);


//kbd.pressed_x = function() {
// encoder.stop_filesave();
//}

//kbd.pressed_z = function() {
// encoder.start_filesave("acidvid.ogm");
//}

function brand ( n )
{
  return ( Math.floor ( Math.random ( ) * n + 1 ) );
}

function drawRotatedtriangle(layer, centerx, centery, radius, rotation) {

//	rotation = (rotation/180)*Math.PI;

	ax = Math.cos((rotation/180)*Math.PI) * radius;
	ay = Math.sin((rotation/180)*Math.PI) * radius;
	bx = Math.cos(((rotation+120)/180)*Math.PI) * radius;
	by = Math.sin(((rotation+120)/180)*Math.PI) * radius;
	cx = Math.cos(((rotation+240)/180)*Math.PI) * radius;
	cy = Math.sin(((rotation+240)/180)*Math.PI) * radius;

	layer.trigon_fill(ax+centerx,ay+centery,bx+centerx,by+centery,cx+centerx,cy+centery);

}

function drawRotatedsquare(layer, centerx, centery, radius, rotation) {

//	rotation = (rotation/180)*Math.PI;

	ax = Math.cos((rotation/180)*Math.PI) * radius;
	ay = Math.sin((rotation/180)*Math.PI) * radius;
	bx = Math.cos(((rotation+90)/180)*Math.PI) * radius;
	by = Math.sin(((rotation+90)/180)*Math.PI) * radius;
	cx = Math.cos(((rotation+180)/180)*Math.PI) * radius;
	cy = Math.sin(((rotation+180)/180)*Math.PI) * radius;
	dx = Math.cos(((rotation+270)/180)*Math.PI) * radius;
	dy = Math.sin(((rotation+270)/180)*Math.PI) * radius;

	layer.trigon_fill(ax+centerx,ay+centery,bx+centerx,by+centery,cx+centerx,cy+centery);
	layer.trigon_fill(ax+centerx,ay+centery,dx+centerx,dy+centery,cx+centerx,cy+centery);

}

kbd.pressed_esc = function() { quit(); }
//kbd.released_q = function() { quit(); }
register_controller( kbd );

geo = new GeometryLayer(W,H);
//geo.color(255,255,255,255);
geo.set_blit("alpha");
//geo.set_blit_value(1);
geo.start();
add_layer(geo);

//drawStar(geo,30,1);

framec = 24;

xinc = 2;
yinc = 1;
zinc = 0;

forceshape = 0;
forcedshape = 1;

acidmode = 1;

kbd.pressed_w = function() { yinc--; }
kbd.pressed_s = function() { yinc++; }
kbd.pressed_a = function() { xinc--; }
kbd.pressed_d = function() { xinc++; }
kbd.pressed_q = function() { zinc--; }
kbd.pressed_e = function() { zinc++; }

kbd.pressed_c = function() { xinc=0; yinc=0; }

kbd.pressed_1 = function() { forceshape=1; forcedshape=1; }
kbd.pressed_2 = function() { forceshape=1; forcedshape=2; }
kbd.pressed_3 = function() { forceshape=1; forcedshape=3; }
kbd.pressed_4 = function() { forceshape=1; forcedshape=4; }
kbd.pressed_5 = function() { forceshape=1; forcedshape=5; }
kbd.pressed_6 = function() { forceshape=1; forcedshape=6; }
kbd.pressed_7 = function() { forceshape=1; forcedshape=7; }

kbd.released_1 = function() { forceshape=0; }
kbd.released_2 = function() { forceshape=0; }
kbd.released_3 = function() { forceshape=0; }
kbd.released_4 = function() { forceshape=0; }
kbd.released_5 = function() { forceshape=0; }
kbd.released_6 = function() { forceshape=0; }
kbd.released_7 = function() { forceshape=0; }


kbd.pressed_0 = function() { acidmode=-acidmode; }

objectnum = 35;
maxz = 20;


object = new Array(objectnum)

function randomizeObjects() {
	for (i = 0; i < object.length ; ++i) {
		// ieder object is 7 dingen : r g b x y z en type 
		object [i] = new Array(7);

		// kleur instellen

		object [i][0] = brand(256)-1; //r
		object [i][1] = brand(256)-1; //g
		object [i][2] = brand(256)-1; //b
		// alpha zou hier ook nog kunnen, maar meh

		// posities instellen

		object [i][3] = brand(W); //x
		object [i][4] = brand(H); //y
		object [i][5] = brand(maxz)-1; //z
	
		// type instellen

		// ehm sja, 1 is circle :D

		object [i][6] = brand(8); // hihi

		// acidkleur kiezen ;)

		object [i][7] = brand(acidpalet.length)-1;

	}
}

randomizeObjects();

kbd.released_r = function() { randomizeObjects(); }

bang = new TriggerController();
register_controller(bang);
bang.frame = function() {
    if(framec>0) {
        framec--;
    } else {
	// ehm, ja, hier is dus een frame ofzo?
	// ja val me niet lastig, ik copypaste er ook maar op los :D

//	geo.set_blit("alpha");
//	geo.set_blit_value(256);


	geo.color(0,0,0,0);
	geo.clear();

	for (i = 0; i < object.length ; ++i) {

		// oude object wissen?

		// object updaten

		object [i][3] = (object [i][3])+(xinc * ((maxz-(object [i][5]))/2));
		if (object [i][3]<0) object [i][3]=W;
		if (object [i][3]>W) object [i][3]=0;

		object [i][4] = (object [i][4])+(yinc * ((maxz-(object [i][5]))/2));
		if (object [i][4]<0) object [i][4]=H;
		if (object [i][4]>H) object [i][4]=0;

		// object tekenen

//		geo.set_blit("add");
//		geo.set_blit_value(255);

		if (acidmode<0) {
			geo.color(object [i][0],  object [i][1], object [i][2]);
		} else {
			die = object [i][7];
			geo.color(acidpalet [die][0], acidpalet [die][1], acidpalet [die][2]);
		}

		objectshape = object [i][6];

		if (forceshape) objectshape = forcedshape;

		switch (objectshape) {
			case 1:
				geo.circle_fill(object [i][3], object [i][4], 30 - object [i][5]);
				break;
			case 2:
				geo.pie_fill(object [i][3], object [i][4], 30 - object [i][5], (360/W)*object [i][3], (360/H)*object [i][4]);
				break;
			case 3:
				rotation = (((360/W)*object[i][3])+((360/H)*object[i][4]))/2;
				drawRotatedsquare(geo, object [i][3],  object [i][4], (50 - object [i][5]), rotation);
				break;
			case 4:
				rotation = 360-((((360/W)*object[i][3])+((360/H)*object[i][4]))/2);
				drawRotatedsquare(geo, object [i][3],  object [i][4], (50 - object [i][5]), rotation);
				break;
			case 5:
				geo.rectangle_fill(object [i][3] - (10 - object [i][5]), object [i][4] - (30 - object [i][5]), object [i][3] + (10 - object [i][5]), object [i][4] + (30 - object [i][5]));
				break;
			case 6:
				geo.rectangle_fill(object [i][3] - (30 - object [i][5]), object [i][4] - (10 - object [i][5]), object [i][3] + (30 - object [i][5]), object [i][4] + (10 - object [i][5]));
				break;
			case 7:
				rotation = (((360/W)*object[i][3])+((360/H)*object[i][4]))/2;
				drawRotatedtriangle(geo, object [i][3],  object [i][4], (50 - object [i][5]), rotation);
				break;
			case 8:
				rotation = 360-((((360/W)*object[i][3])+((360/H)*object[i][4]))/2);
				drawRotatedtriangle(geo, object [i][3],  object [i][4], (50 - object [i][5]), rotation);
				break;
			default:
				// huuuuuuuuuuuuuuuuuu
		}

	}

    }
}

