// freej script for a simple text scroller
// it makes smart use of arrays as FIFO pipes
// and the file_to_strings(filename) function
// to read a text file into an array of strings

// (C)2005 Denis Jaromil Rojo - GNU GPL 

W = 640;
H = 480;

wordspacing = 5;

// setup the keyboard quit
running = true;
kbd = new KeyboardController();
kbd.released_esc = function() { running = false; }
register_controller( kbd );

// read a text file into an array
words = file_to_strings("../README");
idx = 0;
// words.length property says how big it is
// access words with words[0] words[1] and so on


// make a black background
/*
background = new GeometryLayer();
background.color(0x00000000);
background.set_blit("alpha");
background.set_blit_value(0.2);
background.rectangle(0, 0, background.w(), background.h() );
add_layer( background );
*/

// setup the array of rendered words
// each one is a TextLayer object
scroll = new Array();

/*
  scroll[] is an Array object
  we will use it as a FIFO pipe, thru the push() and shift()
  methods of javascript arrays:

   t(n): scroll.push( txt ) pushes in text from the right (here below)
   array scroll  [ 0 ] [ 1 ] [ 2 ] [ 3 ] [ 4 ] [ 5 ]     _/
   screen -> | <- bla - bla - bla - bla - bla - bla - | <- screen
   
   t(n+1):
       [ 0 ] | <- [ 1 ] - [ 2 ] - [ 3 ] ...
       scroll.shift() returns scroll[0] (which we delete)
       
  so let's go on...
*/

function render_word(wrd) {

    lay = new TextLayer();

    lay.size( 50 ); // set the size
    
    add_layer( lay ); // add it to the screen

    lay.print( wrd ); // print the string in the layer

    lay.set_position( W, 200 ); // start from the right of the screen

    lay.slide_position( 0 - lay.w() , 200, 2); // slide to the right

    return lay;
}
  
// MAIN

// create a new TextLayer with the word
txt = render_word( words[idx] );

// append it as last element of the scroll[] array
scroll.push( txt );

// advance the index of words
idx++;

while(running) {


    // check if the leftmost is out of screen
    if( scroll[0].x() + scroll[0].w()  <   0 ) {
	// pull it out from the array
	txt = scroll.shift();
	// remove it from the screen
	rem_layer( txt );
	// delete it
	delete txt;
    }


    // check if the rightmost all entered the screen
    if( scroll[ scroll.length-1 ].x()
	+ scroll[ scroll.length-1 ].w()
	<
	W - wordspacing ) {
	// then we need a new one on the left
	
	// create a new TextLayer with the word
	txt = render_word( words[idx] );
	
	// append it as last element of the scroll[] array
	scroll.push( txt );
	
	// advance the index
	idx++;
    }

    
    run(0.5);
    
    // if the words are finished, start over
    if(idx > words.length) idx = 0;

    if(!running) break;
    
}

quit();
