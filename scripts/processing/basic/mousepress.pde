void setup() {
  size(200, 200);
  fill(126);
  background(102);
}

void draw() {
  if(mousePressed) {
    stroke(255);
  } else {
    stroke(0);
  }
  line(mouseX-66, mouseY, mouseX+66, mouseY);
  line(mouseX, mouseY-66, mouseX, mouseY+66); 
}
