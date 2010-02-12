size(200, 200);
PImage a;  // Declare variable "a" of type PImage
a = loadImage("data/arch.jpg", null, function(){  // Load the images into the program
  image(a, 0, 0); // Displays the image from point (0,0) when image data has been loaded
  image(a, width/2, 0, a.width/2, a.height/2);
});
