float x = 33;
float numFrames = 50;

void setup() 
{
  size(200, 200);
  smooth();
  noStroke();
}

void draw() 
{
  background(0);
  x += random(-2, 2);
  ellipse(x, 100, 80, 80);
  if (frameCount <= numFrames) {
    saveFrame("circles-####.tif");
  }
}
