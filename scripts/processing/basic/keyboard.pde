int numChars = 26;
color[] colors = new color[numChars];
int keyIndex;
float keyScale;
int rectWidth;

    
void setup()
{
  size(200, 200);
  noStroke();
  background(0);
  keyScale = 200/numChars-1.0;
  rectWidth = width/4;
}

void draw()
{ 
  if(keyPressed) {
    if(key >= 'A' && key <= 'z') {
      if(key <= 'Z') {
        keyIndex = key-'A';
      } else {
        keyIndex = key-'a';
      }
      fill(millis()%255);
      float beginRect = rectWidth/2 + keyIndex*keyScale-rectWidth/2;
      rect(beginRect, 0.0, rectWidth, height);
    }
  }
}
