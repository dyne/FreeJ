PImage a;
boolean onetime = true;
int[] aPixels = new int[200*200];
int direction = 1;

float signal;

void setup() 
{
  size(33, 33);
  stroke(255);
  a = loadImage("data/cait.jpg", null, function(){
    for(int i=0; i<width*height; i++) {
      aPixels[i] = a.pixels[i];
    }
  });  
  frameRate(30);
}

void draw() 
{
  if (signal > width-1 || signal < 0) { 
    direction = direction * -1; 
  }

  if(mousePressed) {
    signal = abs(mouseY%height);
  } else {
    signal += (0.3*direction);  
  }
  
  
  if(keyPressed) {
    image(a, 0, 0);
    /*
    loadPixels();
    for (int i=0; i<width*height; i++) { 
      pixels[i] = aPixels[i];  
    }
    updatePixels();
    */
    line(0, signal, width, signal);
  } else {
    loadPixels();
    for (int i=0; i<width*height; i++) { 
      pixels[i] = aPixels[int((width*int(signal))+(i%width))];
    }
    updatePixels();
  }
  
}
