void setup() 
{
  size(200, 200);
  background(51);
  noStroke();
  smooth();
  noLoop();
}

void draw() 
{
  draw_target(68, 34, 200, 10);
  draw_target(152, 16, 100, 3);
  draw_target(100, 144, 80, 5);
}

void draw_target(int xloc, int yloc, int size, int num) 
{
  float grayvalues = 255/num;
  float steps = size/num;
  for(int i=0; i<num; i++) {
    fill(i*grayvalues);
    ellipse(xloc, yloc, size-i*steps, size-i*steps);
  }
}
