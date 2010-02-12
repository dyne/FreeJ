size(200, 200);
smooth();
background(0);
strokeWeight(10);

for(int i = 0; i < width; i++) {
  float r = random(255);
  float x = random(0, width);
  stroke(r, 100);
  line(i, 0, x, height);
}
