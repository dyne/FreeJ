// this script grabs the desktop
// discover your desktop id number using the command xwininfo from terminal
// then click on your desktop background and read the information

grab = new XGrabLayer();
grab.open(0x85);
grab.set_fps(25);
grab.activate(true);
grab.start();

add_layer(grab);

