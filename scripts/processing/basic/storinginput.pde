int num = 60;
float mx[] = new float[num];
float my[] = new float[num];

void setup() 
{
  size(200, 200);
  smooth();
  noStroke();
  fill(255, 153); 
}

void draw() 
{
  background(51); 
  
  // Reads throught the entire array
  // and shifts the values to the left
  for(int i=1; i<num; i++) {
    mx[i-1] = mx[i];
    my[i-1] = my[i];
  } 
  // Add the new values to the end of the array
  mx[num-1] = mouseX;
  my[num-1] = mouseY;
  
  for(int i=0; i<num; i++) {
    ellipse(mx[i], my[i], i/2, i/2);
  }
}
