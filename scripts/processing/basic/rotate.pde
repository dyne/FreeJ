void setup()
{
  size(200,200);
  noStroke();
  fill(255);
  frameRate(30);
}

float angle;
float cosine;
float jitter;

void draw()
{
  background(102);
  
  if(second()%2 == 0){
    jitter = (random(-0.1, 0.1));
  }
  angle = angle + jitter;
  cosine = cos(angle);
  
  translate(width/2, height/2);
  rotate(cosine);
  rectMode(CENTER);
  rect(0, 0, 115, 115);   
}
