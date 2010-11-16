// Audio input is taken via Jack from other applications running

//                    port name     buffer size  samlerate
audio = new AudioJack("metro:120_bpm", 4096,      48000);
// tweak the values below accordingly, see Jack documentation

// Create a Video Encoder object
// V=video A=audio         V quality  V bitrate  A quality  A bitrate
encoder = new VideoEncoder(12,        300000,     6,         48000);

// Add the audio channel in the video encoded
encoder.add_audio(audio);

// Register the encoder on the running FreeJ engine
register_encoder(encoder);

// Record the stream into a local file
encoder.start_filesave('Video/freej-test.ogg');
