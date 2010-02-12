size(200, 200);
background(102);

// Load the font. Fonts are located within the 
// main Processing directory/folder and they
// must be placed within the data directory
// of your sketch for them to load
PFont fontA = loadFont("Arial");

// Set the font and its size (in units of pixels)
textFont(fontA, 32);

int x = 30;

// Use fill() to change the value or color of the text
fill(0);
text("ichi", x, 60);
fill(51);
text("ni", x, 95);
fill(204);
text("san", x, 130);
fill(255);
text("shi", x, 165);
