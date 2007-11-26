/// simple example script for streaming
/// do any action above here and then include the code below to stream
// don't forget to edit it with your streaming authentication

// create a video encoder object
//    values 1-100         video quality  video bitrate  audio quality  audio_bitrate
encoder = new VideoEncoder(10,             64000,        0,             24000);

encoder.stream_host("giss.tv");
encoder.stream_port(8000);
encoder.stream_title("testing new freej");
encoder.stream_username("source");
encoder.stream_password("2t645");
encoder.stream_mountpoint("freej-test.ogg");

register_encoder(encoder);
encoder.start_stream();
//encoder.start_filesave('/mnt/hd1/3/video/luminescenza.ogg');
