/// simple example script for streaming
/// do any action above here and then include the code below to stream
// don't forget to edit it with your streaming authentication

// create a jack audio input
//                    port name     buffer size  samlerate
//audio = new AudioJack("alsaplayer", 2048,        44100);
// in case the port named is not already present:
// you should connect the input manually using jack

// create a video encoder object
//    values 1-100         video quality  video bitrate  audio quality  audio_bitrate
encoder = new VideoEncoder(50,             64000,        0,             0);

kbd = new KeyboardController();
register_controller( kbd );


// add the audio channel in the video encoded
//encoder.add_audio(audio);

//encoder.start_stream();

kbd.pressed_s = function() {
 encoder.stop_filesave();
 delete encoder;
}

kbd.pressed_r = function() {
 register_encoder(encoder);
}
