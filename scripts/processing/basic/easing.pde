float x;
float y;
float targetX, targetY;
float easing = 0.05;

void setup() 
{
  size(200, 200); 
  smooth();
  noStroke();  
}

void draw() 
{ 
  background( 51 );
  
  targetX = mouseX;
  float dx = mouseX - x;
  if(abs(dx) > 1) {
    x += dx * easing;
  }
  
  targetY = mouseY;
  float dy = mouseY - y;
  if(abs(dy) > 1) {
    y += dy * easing;
  }
  
  ellipse(x, y, 33, 33);
}
