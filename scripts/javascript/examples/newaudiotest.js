
audio = new AudioJack("freej", 1024, 44100);

animation_file = "/home/jaromil/Movies/TheRevolutionWillNotBeTelevisedGilScottHeron.mp4";

anim = new MovieLayer(animation_file);

anim.activate(true);

add_layer(anim);

audio.set_layer(anim);




