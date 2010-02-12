float scale;

void setup()
{
  size(200, 200);
  noStroke();
  scale = width/10;
}

void draw()
{ 
  for(int i=0; i<scale; i++) {
    colorMode(RGB, (i+1) * scale * 10);
    fill(millis()%((i+1) * scale * 10) );
    rect(i*scale, 0, scale, height);
  }
}
