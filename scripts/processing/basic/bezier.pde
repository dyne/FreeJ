size(200, 200); 
background(0); 
stroke(255);
noFill();
smooth(); 

for(int i = 0; i < 100; i += 20) {
  bezier(90-(i/2.0), 20+i, 210, 10, 220, 150, 120-(i/8.0), 150+(i/4.0));
}
