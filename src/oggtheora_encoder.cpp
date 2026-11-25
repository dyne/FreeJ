/*  FreeJ - Ogg/Vorbis/Theora encoder
 *
 *  (c) Copyright 2005 Silvano Galliani <kysucix@dyne.org>
 *                2007 Denis Rojo       <jaromil@dyne.org>
 *
 * ogg/vorbis/theora code learned from encoder_example and ffmpeg2theora
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */

#include <config.h>

#ifdef WITH_OGGTHEORA
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <context.h>
#include <screen.h>
#include <audio_collector.h>

#include <oggtheora_encoder.h>
#include <iomanip>
using namespace std;
using std::setiosflags;

OggTheoraEncoder::OggTheoraEncoder()
  : VideoEncoder(), oggmux{} {
  func("OggTheoraEncoder object created");
  
  video_quality  = 16;  // it's ok for streaming
  audio_quality =  10; // it's ok for streaming

  //  picture_rgb = NULL;
  //  enc_rgb24 = NULL;

  use_audio = false;
  audio = NULL;
  audio_buf = NULL;
  init_info(&oggmux);
  theora_comment_init(&oggmux.tc);


  set_name("encoder/theora");

}

OggTheoraEncoder::~OggTheoraEncoder() {
  func("OggTheoraEncoder:::~OggTheoraEncoder");
  stop();
  if (initialized) {
    if (use_audio) {
      drain_audio();
      encode_audio(1, 0);
    }
    oggmux_flush(&oggmux, 1);
    oggmux_close(&oggmux);
    theora_info_clear(&oggmux.ti);
  } else {
    theora_comment_clear(&oggmux.tc);
  }

  if (recording_) recording_->rate.store(0);
}

void OggTheoraEncoder::flush_pending_output() {
  if (initialized) {
    if (use_audio)
      drain_audio();
    oggmux_flush(&oggmux, 1);
  }
}

bool OggTheoraEncoder::init (ViewPort *scr) {

  if(initialized) return true;
  if (!ringbuffer)
    return false;

  const int w = scr->geo.w;
  const int h = scr->geo.h;
  video_x = ((w + 15) >> 4) << 4;
  video_y = ((h + 15) >> 4) << 4;
  // Theora's coded size is padded; chroma offsets must be even.
  frame_x_offset = ((video_x - w) / 2) & ~1;
  frame_y_offset = ((video_y - h) / 2) & ~1;
  if (!configure_frame(w, h, video_x, video_y, frame_x_offset, frame_y_offset))
    return false;

  screen = scr;
  oggmux.ringbuffer = ringbuffer;
  oggmux.bytes_encoded = 0;
  oggmux.aveVorBitRate = 0;

  oggmux.audio_only = 0;

  if(use_audio && audio) {
    recording_ = audio->IsConnected() ? audio->Jack->BeginRecording() : nullptr;
    if (!recording_) return false;

    m_MixVal = 1.0;	//sound mix level coef.
    oggmux.video_only = 0;
    oggmux.sample_rate = recording_->rate.load();
    oggmux.channels = recording_->channels.load();
    if ((audio_quality >= 0) && (audio_quality <= 100))
      oggmux.vorbis_quality = (double)audio_quality*0.011-0.1;	/*audio_quality as to become from -0.1 to 1.0 (lo to hi)*/

//     oggmux.vorbis_bitrate = audio_bitrate;	//not used
    
    mono_.resize(4096);
    mixed_.resize(mono_.size() * oggmux.channels);
  
  } else {

    oggmux.video_only = 1;
    use_audio = false;

  }


  /* Set up Theora encoder */

  int theora_quality = (int) ( (video_quality * 63) / 100);
  func("VideoEncoder: encoding theora to quality %u", theora_quality);


  /* video settings here */
  theora_info_init (&oggmux.ti);

  oggmux.ti.width                        = video_x;
  oggmux.ti.height                       = video_y;
  oggmux.ti.frame_width                  = screen->geo.w;
  oggmux.ti.frame_height                 = screen->geo.h;
  oggmux.ti.offset_x                     = frame_x_offset;
  oggmux.ti.offset_y                     = frame_y_offset;
  oggmux.ti.fps_numerator                = 25; // env->fps.fps;
  oggmux.ti.fps_denominator              = 1;
  oggmux.ti.aspect_numerator             = 0;
  oggmux.ti.aspect_denominator           = 0;
  oggmux.ti.colorspace                   = OC_CS_ITU_REC_470BG;
  //	oggmux.ti.colorspace                   = OC_CS_UNSPECIFIED;
  // #ifndef HAVE_64BIT
  oggmux.ti.pixelformat                  = OC_PF_420; // was OC_PF_420 with ccvt
  // #endif
  oggmux.ti.target_bitrate               = video_bitrate;
  oggmux.ti.quality                      = theora_quality;
  
  oggmux.ti.dropframes_p                 = 0; // was 0
  oggmux.ti.quick_p                      = 1;
  oggmux.ti.keyframe_auto_p              = 1;
  oggmux.ti.keyframe_frequency           = 64;
  oggmux.ti.keyframe_frequency_force     = 64;
  oggmux.ti.keyframe_data_target_bitrate = (unsigned int) (video_bitrate * 1.5);
  oggmux.ti.keyframe_auto_threshold      = 80;
  oggmux.ti.keyframe_mindistance         = 8;
  oggmux.ti.noise_sensitivity            = 1;
  oggmux.ti.sharpness                    = 1;

  oggmux_init(&oggmux);
  
  act("initialization successful");
  initialized = true;
 	
  return true;
}

//gets the average audio bit rate sets by the oggmux_init function
long OggTheoraEncoder::getAvBitRate()
{
  return (oggmux.aveVorBitRate);
}

void OggTheoraEncoder::setMixCoef(float val)
{
  m_MixVal = val;
}

int OggTheoraEncoder::Mux(int nfr)
{
  const size_t channels = oggmux.channels;
  const size_t frames = std::min(static_cast<size_t>(nfr), mono_.size());
  auto *input = recording_->input.get();
  auto *mix = recording_->mix.get();
  const size_t mono_frames = std::min(frames, ringbuffer_read_space(input) / sizeof(float));
  const size_t frame_bytes = channels * sizeof(float);
  const size_t clip_frames = std::min(frames, ringbuffer_read_space(mix) / frame_bytes);
  const size_t count = std::max(mono_frames, clip_frames);
  if (!count) return 0;
  ringbuffer_read(input, reinterpret_cast<char *>(mono_.data()), mono_frames * sizeof(float));
  ringbuffer_read(mix, reinterpret_cast<char *>(mixed_.data()), clip_frames * frame_bytes);
  std::fill(mixed_.begin() + clip_frames * channels, mixed_.begin() + count * channels, 0.f);
  const float gain = m_MixVal.load();
  for (size_t frame = 0; frame < mono_frames; ++frame)
    for (size_t channel = 0; channel < channels; ++channel)
      mixed_[frame * channels + channel] += mono_[frame] * gain;
  return count * frame_bytes;
}

void OggTheoraEncoder::drain_audio() {
  if (!recording_) return;
  const size_t frame_bytes = oggmux.channels * sizeof(float);
  size_t remaining = std::max(ringbuffer_read_space(recording_->input.get()) / sizeof(float),
                              ringbuffer_read_space(recording_->mix.get()) / frame_bytes);
  while (remaining) {
    const int bytes = Mux(std::min(remaining, mono_.size()));
    if (!bytes) break;
    encode_audio(0, bytes);
    remaining -= bytes / frame_bytes;
  }
}

int OggTheoraEncoder::encode_frame() {
  encode_video(0);
  if (use_audio)
    drain_audio();

  oggmux_flush(&oggmux, 0);

  bytes_encoded = oggmux.video_bytesout + oggmux.audio_bytesout;	//total from the beginning

  audio_kbps = oggmux.akbps;
  video_kbps = oggmux.vkbps;

  // just pass the reference for the status
  status = &oggmux.status[0];
  return bytes_encoded;
}



int OggTheoraEncoder::encode_video( int end_of_stream) {
  yuv_buffer          yuv;
  
  /* take picture and convert it to yuv420 */

  // picture was feeded in the right format by feed_video
  
  /* Theora is a one-frame-in,one-frame-out system; submit a frame
     for compression and pull out the packet */
  yuv.y_width   = video_x;
  yuv.y_height  = video_y;
  //  yuv.y_stride  = picture_yuv->linesize [0];
  yuv.y_stride = video_x;
  
  yuv.uv_width  = video_x >> 1;
  yuv.uv_height = video_y >> 1;
  //  yuv.uv_stride = picture_yuv->linesize [1];
  yuv.uv_stride = video_x >> 1;

   yuv.y = (uint8_t *) enc_y;
   yuv.u = (uint8_t *) enc_u;
   yuv.v = (uint8_t *) enc_v;
  
  /* encode image */
  oggmux_add_video (&oggmux, &yuv, end_of_stream);
  return 1;
}

int OggTheoraEncoder::encode_audio( int end_of_stream, size_t sizeInBuff) {

#ifdef WITH_AUDIO
  //  num = env->audio->framesperbuffer*env->audio->channels*sizeof(int16_t);
//   func("going to encode %u bytes of audio", audio->buffersize);
  ///// QUAAAA
  //  oggmux_add_audio (oggmux_info *info, int16_t * readbuffer, int bytesread, int samplesread,int e_o_s);
  //  oggmux_add_audio(&oggmux, env->audio->input,
  //		   read,
  //		   read / env->audio->channels /2,
  //		   end_of_stream );
  //  audio->get_audio(audio_buf);

    oggmux_add_audio(&oggmux, mixed_.data(),
  		   sizeInBuff,
  		   sizeInBuff/(sizeof(float)*oggmux.channels), //read / oggmux.channels,
  		   end_of_stream );
#endif

  return 1;
}




#endif
