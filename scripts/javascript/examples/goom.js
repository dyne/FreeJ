// simple use of goom and audio
// -jrml

audio = new AudioJack("xine:out_l", 2048, 44100);

goom = new GoomLayer();
audio.add_output(goom);
goom.start();
add_layer(goom);



