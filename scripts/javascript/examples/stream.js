/// simple example script for streaming
/// do any action above here and then include the code below to stream



// don't forget to edit it with your streaming authentication

// in case the port named is not already present:
// you should connect the input manually using jack

// create a video encoder object
//    values 1-100         video quality  video bitrate  audio quality  audio_bitrate
encoder = new VideoEncoder(10,             120000,        5,             24000);

// create a jack audio input
//                    port name     buffer size  samlerate
have_audio = true;
try {
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

// encoder.stream_host("giss.tv");
// encoder.stream_port(8000);
// encoder.stream_title("testing new freej");
// encoder.stream_username("source");
// encoder.stream_password("2t645");
// encoder.stream_mountpoint("freej-test.ogg");

encoder.stream_host("radio.dyne.org");
encoder.stream_port(8000);
encoder.stream_title("testing new freej");
encoder.stream_description("streaming from da livingroom");
encoder.stream_username("source");
encoder.stream_password("revolution");
encoder.stream_mountpoint("freej-test.ogg");

register_encoder(encoder);
encoder.start_stream();
//function record() {
// encoder.start_filesave('/mnt/hd1/2/capture.ogm');
//}
