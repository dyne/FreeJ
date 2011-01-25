// Audio input is taken via Jack from other applications running

//                    port name     buffer size  samlerate
audio = new AudioJack("metro:120_bpm", 1024,      48000);
// tweak the values below accordingly, see Jack documentation

// Create a Video Encoder object
// V=video A=audio         V quality  V bitrate  A quality  A bitrate
encoder = new VideoEncoder(10,        100000,     1,         48000);

// Add the audio channel in the video encoded
encoder.add_audio(audio);

//Configure the encoder to stream over an Icecast server
encoder.stream_host("localhost");
encoder.stream_port(8000);
encoder.stream_title("testing new freej");
encoder.stream_username("source");
encoder.stream_password("test!");
encoder.stream_mountpoint("freejcpp.ogg");


// Register the encoder on the running FreeJ engine
register_encoder(encoder);

// Start a network stream
encoder.start_stream();


// Record the stream into a local file
encoder.start_filesave('Video/js.ogg');
