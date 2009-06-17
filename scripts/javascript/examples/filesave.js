/// simple example script for saving to a file
/// do any action above here and then include the code below 



// you should connect the input manually using jack

// create a video encoder object
//    values 1-100         video quality  video bitrate  audio quality  audio_bitrate
encoder = new VideoEncoder(10,             120000,        5,             24000);

// create a jack audio input
have_audio = true;
try {
    //                     port name    buffer size  samlerate
    audio = new AudioJack("alsaplayer", 2048,        44100);
}

catch(e) {
	echo("audio not present: " + e);
	have_audio = false;
}

if(have_audio) {
    // add the audio channel in the video encoded
    encoder.add_audio(audio);
}


register_encoder(encoder);
encoder.start_filesave('/Users/xant/capture.ogg');
