/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author Jaromil
    @version 0.9
*/

///////////////////////////////////////////////////
/////// VIDEO ENCODER

/**
   This object compresses the video output of FreeJ (using the
   Ogg/Vorbis/Theora codec) and can be configured to save the
   compressed video in a file (playable later) or stream che
   compressed video on the internet for live playback (using icecast2)

   <div class="example">Example:

   // create a video encoder object
   //    values 1-100         video quality  video bitrate  audio quality  audio_bitrate
   encoder = new VideoEncoder(10,             64000,        5,             24000);
   encoder.stream_host("giss.tv");
   encoder.stream_port(8000);
   encoder.stream_title("testing new freej");
   encoder.stream_username("source");
   encoder.stream_password("2t645");
   encoder.stream_mountpoint("freej-test.ogg");

   register_encoder(encoder);
   encoder.start_stream();
   // encoder.start_filesave("prova.ogg");
   </div>

   @class The Video Encoder compresses video to save in a file or stream on the net
   @author Xiph.org, Kysucix, Jaromil
   @constructor
   @param {int} video_quality quality of the video compression, from 1 to 100.
   @param {int} video_bitrate desired target bitrate for the video compression (not exactly matched)
   @param {int} audio_quality quality of the audio compression, from 1 to 100. If 0 then uses fixed bitrate
   @param {int} audio_bitrate constant bitrate for the audio compression, when 0 quality is specified
   @returns a new allocated Video Encoder
*/
function VideoEncoder(video_quality, video_bitrate, audio_quality, audio_bitrate) { };

/**
   The Encoder object starts processing when this method is
   called. Every start function should be called only after having
   applied all configuration directives. start_filesave starts saving
   all compressed video into the specified .OGG file.
   @param {string} file_name full path to the file to be saved, it is recommended to include an .ogg or .ogm extension
*/
function start_filesave(file_name) { };
VideoEncoder.prototype.start_filesave = start_filesave;

/** Stop saving into the file. The Encoder object will stop processing
    if this method is called and both filesave and stream are stopped.
*/
function stop_filesave() { };
VideoEncoder.prototype.stop_filesave = stop_filesave;

/**
   The Encoder object starts processing when this method is
   called. Every start function should be called only after having
   applied all configuration directives. start_stream sending the 
   compressed video on the net to the configured Icecast server.
*/
function start_stream() { };
VideoEncoder.prototype.start_stream = start_stream;

/** Stop sending video on the net. The Encoder object will stop
    processing if this method is called and both filesave and stream
    are stopped.
*/
function stop_stream() { };
VideoEncoder.prototype.stop_stream = stop_stream;


/** Configure the network address for the icecast server we want to stream to.
    @param address can be an name or a numeric IP address, it will appear in the internet URL to be played.
*/
function stream_host(address) { };
VideoEncoder.prototype.stream_host = stream_host;


function stream_port(port) { };
VideoEncoder.prototype.stream_port = stream_port;

function stream_title(title) { };
VideoEncoder.prototype.stream_title = stream_title;

function stream_username(username) { };
VideoEncoder.prototype.stream_username = stream_username;

function stream_password(password) { };
VideoEncoder.prototype.stream_password = stream_password;

function stream_mountpoint(mountpoint) { };
VideoEncoder.prototype.stream_mountpouint = stream_mountpoint;

function stream_homepage(homepage) { };
VideoEncoder.prototype.stream_homepage = stream_homepage;
