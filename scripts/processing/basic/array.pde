size(200, 200);

float[] coswave = new float[width];

for(int i=0; i<width; i++) {
  float ratio = (float)i/(float)width;
  coswave[i] = abs( cos(ratio*PI) );
}

for(int i=0; i<width; i++) {
  stroke(coswave[i]*255);
  line(i, 0, i, width/3);
}

for(int i=0; i<width; i++) {
  stroke(coswave[i]*255/4);
  line(i, width/3, i, width/3*2);
}

for(int i=0; i<width; i++) {
  stroke(255-coswave[i]*255);
  line(i, width/3*2, i, height);
}
