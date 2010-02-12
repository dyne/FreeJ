size(200, 200);
background(0);

for(int i=10; i<width; i+=10) {
  // If 'i' divides by 20 with no remainder draw the first line
  // else draw the second line
  if(i%20 == 0) {
    stroke(153);
    line(i, 40, i, height/2);
  } else {
    stroke(102);
    line(i, 20, i, 180); 
  }
}
