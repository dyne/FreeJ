
audio = new AudioJack("freej", 1024, 48000);

animation_file = "/home/rgareus/Desktop/TELESCOPERA_MPEG2.mpg";

anim = new MovieLayer(animation_file);
anim.set_fps(25);
add_layer(anim);

audio.set_layer(anim);

anim.activate(true);

