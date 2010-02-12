PImage a, b;
float offset;

void setup() 
{
  size(200, 200);
  a = loadImage("data/construct.jpg");  // Load an image into the program 
  b = loadImage("data/wash.jpg");   // Load an image into the program 
  frameRate(60);
}

void draw() 
{ 
  image(a, 0, 0);
  float offsetTarget = map(mouseX, 0, width, -b.width/2 - width/2, 0);
  offset += (offsetTarget-offset)*0.05; 
  tint(255, 153);
  image(b, offset, 20);
}
