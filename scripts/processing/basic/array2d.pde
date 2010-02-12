float[][] distances;
float maxDistance;

size(200, 200);
background(0);
maxDistance = dist(width/2, height/2, width, height);
distances = new float[width][height];
for(int i=0; i<height; i++) {
  for(int j=0; j<width; j++) {
    float d = dist(width/2, height/2, j, i);
    distances[j][i] = d/maxDistance * 255; 
  }
}

for(int i=0; i<height; i+=2) {
  for(int j=0; j<width; j+=2) {
    stroke(distances[j][i]);
    point(j, i);
  }
}
