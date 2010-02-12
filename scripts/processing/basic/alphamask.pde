PImage img;
PImage maskImg;

void setup() 
{
  size(200,200);
  img = loadImage("data/test.jpg");
  maskImg = loadImage("data/mask.jpg");
  img.mask(maskImg);
}

void draw() 
{
  background((mouseX+mouseY)/1.5);
  image(img, 50, 50);
  image(img, mouseX-50, mouseY-50);
}
