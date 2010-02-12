PImage a;

void setup()
{
  a = loadImage("data/eames.jpg");
  size(200,200);
  noStroke();
  background(255);
  smooth();
}


void draw()
{ 
  float pointillize = map(mouseX, 0, width, 2, 18);
  int x = int(random(a.width));
  int y = int(random(a.height));

  if( a.loaded ){
    color pix = a.get(x, y);
    fill(pix, 126);
    ellipse(x, y, pointillize, pointillize);
  }
}
