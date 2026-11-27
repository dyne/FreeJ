/*  FreeJ
 *  (c) Copyright 2005      Silvano Galliani <kysucix@dyne.org>
 *                2007-2009 Denis Roio       <jaromil@dyne.org>
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
#include <iostream>
#include <algorithm>

#include <config.h>

#include <string.h>
#include <context.h>

#include <video_encoder.h>


#include <convertvid.h>
#include "engine/frame_storage.h"

/* function below taken from ccvt_misc.c
   CCVT: ColourConVerT: simple library for converting colourspaces
   Copyright (C) 2002 Nemosoft Unv. */
inline void ccvt_yuyv_420p(int width, int height,
			   const void *src, void *dsty,
			   void *dstu, void *dstv) {
  register int n, l, j;
  register unsigned char *dy, *du, *dv;
  register unsigned char *s1, *s2;
  
  dy = (unsigned char *)dsty;
  du = (unsigned char *)dstu;
  dv = (unsigned char *)dstv;
  s1 = (unsigned char *)src;
  s2 = s1; // keep pointer
  n = width * height;
  for (; n > 0; n--) {
    *dy = *s1;
    dy++;
    s1 += 2;
  }
  
  /* Two options here: average U/V values, or skip every second row */
  s1 = s2; // restore pointer
  s1++; // point to U
  for (l = 0; l < height; l += 2) {
    s2 = s1 + width * 2; // odd line
    for (j = 0; j < width; j += 2) {
      *du = (*s1 + *s2) / 2;
      du++;
      s1 += 2;
      s2 += 2;
      *dv = (*s1 + *s2) / 2;
      dv++;
      s1 += 2;
      s2 += 2;
    }
    s1 = s2;
  }
}



VideoEncoder::VideoEncoder()
  : Entry(), JSyncThread() {

  initialized = false;
  use_audio = false;
  audio = NULL;
  audio_buf = NULL;
  screen = NULL;
  active = false;
  streaming = false;
  video_quality = audio_quality = 0;
  video_bitrate = audio_bitrate = 0;

  write_to_disk   = false;
  write_to_stream = false;

  filedump_fd = NULL;

  status = NULL;
  audio_kbps = 0;
  video_kbps = 0;
  bytes_encoded = 0;
  m_ElapsedTime = 0;
  m_StreamRate = 0;
  m_Streamed = 0;
  enc_y = enc_u = enc_v = enc_yuyv = NULL;

  fps = &static_cast<JSyncThread &>(*this).fps;
  fps->init(25); // default FPS
  // initialize the encoded data pipe
  // TODO: set the size to width * height * 4 * nframes (3-4)
  ringbuffer = ringbuffer_create(1048*2096);

  shout_init();
  if ((ice = shout_new()) != NULL) {

    if( shout_set_protocol(ice,SHOUT_PROTOCOL_HTTP) )
      error("shout_set_protocol: %s", shout_get_error(ice));

    if( shout_set_format(ice,SHOUT_FORMAT_OGG) )
      error("shout_set_format: %s", shout_get_error(ice));

    if( shout_set_agent(ice,"FreeJ - freej.dyne.org") )
      error("shout_set_agent: %s", shout_get_error(ice));

    if( shout_set_public(ice,1) )
      error("shout_set_public: %s", shout_get_error(ice));
  }

  func("init picture_yuv for colorspace conversion (avcodec)");  
  gettimeofday(&m_OldTime, NULL);
  
}

VideoEncoder::~VideoEncoder() {
  stop();
  drain_output();
  if (filedump_fd)
    fclose(filedump_fd);
  ringbuffer_free(ringbuffer);
  if (ice) {
    shout_close(ice);
    shout_free(ice);
  }
  shout_shutdown();
  free(enc_y);
  free(enc_u);
  free(enc_v);
  free(enc_yuyv);
}

bool VideoEncoder::configure_frame(unsigned width, unsigned height, unsigned coded_width,
                                   unsigned coded_height, unsigned offset_x, unsigned offset_y) {
  if (!width || !height || (coded_width & 1) || (coded_height & 1) ||
      offset_x + width > coded_width || offset_y + height > coded_height)
    return false;

  const size_t pixels = static_cast<size_t>(coded_width) * coded_height;
  using Buffer = std::unique_ptr<void, decltype(&free)>;
  Buffer y(calloc(pixels, 1), free);
  Buffer u(calloc(pixels / 4, 1), free);
  Buffer v(calloc(pixels / 4, 1), free);
  Buffer yuyv(calloc(pixels, 2), free);
  if (!y || !u || !v || !yuyv)
    return false;

  free(enc_y);
  free(enc_u);
  free(enc_v);
  free(enc_yuyv);
  enc_y = y.release();
  enc_u = u.release();
  enc_v = v.release();
  enc_yuyv = yuyv.release();
  width_ = width;
  height_ = height;
  coded_width_ = coded_width;
  coded_height_ = coded_height;
  offset_x_ = offset_x;
  offset_y_ = offset_y;
  return true;
}

bool VideoEncoder::convert_frame(const freej_engine::Frame &frame) {
  if (!coded_width_ || !frame.layout.width || !frame.layout.height)
    return false;

  const uint32_t *pixels = frame.pixels.data();
  if (frame.layout.width != coded_width_ || frame.layout.height != coded_height_ ||
      width_ != coded_width_ || height_ != coded_height_) {
    padded_pixels_.assign(static_cast<size_t>(coded_width_) * coded_height_, 0);
    for (unsigned row = 0; row < height_; ++row) {
      const auto *source = pixels +
          (static_cast<size_t>(row) * frame.layout.height / height_) * frame.layout.width;
      auto *target = padded_pixels_.data() + static_cast<size_t>(row + offset_y_) * coded_width_ + offset_x_;
      if (frame.layout.width == width_)
        std::copy_n(source, width_, target);
      else
        for (unsigned col = 0; col < width_; ++col)
          target[col] = source[static_cast<size_t>(col) * frame.layout.width / width_];
    }
    pixels = padded_pixels_.data();
  }

  // The legacy converters do not modify their input.
  auto *rgb = reinterpret_cast<uint8_t *>(const_cast<uint32_t *>(pixels));
  auto *packed = static_cast<uint8_t *>(enc_yuyv);
  const unsigned stride = coded_width_ * sizeof(uint32_t);
  switch (frame.layout.format) {
  case freej_engine::FrameFormat::rgba32:
    mlt_convert_rgb24a_to_yuv422(rgb, coded_width_, coded_height_, stride, packed, NULL);
    break;
  case freej_engine::FrameFormat::bgra32:
    mlt_convert_bgr24a_to_yuv422(rgb, coded_width_, coded_height_, stride, packed, NULL);
    break;
  case freej_engine::FrameFormat::argb32:
    mlt_convert_argb_to_yuv422(rgb, coded_width_, coded_height_, stride, packed, NULL);
    break;
  }
  ccvt_yuyv_420p(coded_width_, coded_height_, packed, enc_y, enc_u, enc_v);
  return true;
}

size_t VideoEncoder::drain_output() {
  if (!ringbuffer || (!write_to_disk && !write_to_stream))
    return 0;
  const size_t available = ringbuffer_read_space(ringbuffer);
  if (!available)
    return 0;
  encoded_bytes_.resize(available);
  const size_t count = ringbuffer_read(ringbuffer, encoded_bytes_.data(), available);
  if (write_to_disk && filedump_fd && fwrite(encoded_bytes_.data(), 1, count, filedump_fd) != count)
    error("could not write all encoded bytes to %s: %s", filedump, strerror(errno));
  if (write_to_stream && ice) {
    shout_sync(ice);
    if (shout_send(ice, reinterpret_cast<const unsigned char *>(encoded_bytes_.data()), count) != SHOUTERR_SUCCESS)
      error("shout_send: %s", shout_get_error(ice));
  }
  return count;
}

void VideoEncoder::thread_setup() {
  func("ok, encoder %s in rolling loop",name);
  func("VideoEncoder::run : begin thread %p",pthread_self());
 }

void VideoEncoder::thread_loop() {
  fps->calc();
  fps->delay();

  const auto frame = screen ? freej_engine::FrameAccess::viewport(*screen) : freej_engine::FrameLease{};
  if (!frame || !convert_frame(*frame))
    return;

  std::lock_guard<VideoEncoder> lock(*this);
  encode_frame();
  const size_t written = drain_output();
  if (!written)
    return;

  gettimeofday(&m_ActualTime, NULL);
  m_ElapsedTime += (m_ActualTime.tv_sec - m_OldTime.tv_sec) +
      (m_ActualTime.tv_usec - m_OldTime.tv_usec) / 1000000.0;
  m_OldTime = m_ActualTime;
  m_Streamed += written;
  if (m_ElapsedTime >= 3.0) {
    m_StreamRate = m_Streamed / m_ElapsedTime / 1000.0;
    m_ElapsedTime = 0;
    m_Streamed = 0;
  }
}

double VideoEncoder::getStreamRate() {
  std::lock_guard<VideoEncoder> lock(*this);
  return m_StreamRate;
}

void VideoEncoder::thread_teardown() {
  func("VideoEncoder::run : end thread %p", pthread_self() );
}

bool VideoEncoder::set_filedump(const char *filename) {
  std::lock_guard<VideoEncoder> lock(*this);
  if (filedump_fd)
    filedump_close();
  if (!filename)
    return false;

  const std::string original(filename);
  std::string path = original;
  const auto dot = original.find_last_of('.');
  const auto slash = original.find_last_of('/');
  const auto extension = dot != std::string::npos &&
      (slash == std::string::npos || dot > slash) ? dot : original.size();
  for (unsigned suffix = 1; access(path.c_str(), F_OK) == 0; ++suffix)
    path = original.substr(0, extension) + "-" + std::to_string(suffix) + original.substr(extension);
  if (path.size() >= sizeof(filedump)) {
    error("recording filename is too long");
    return false;
  }

  filedump_fd = fopen(path.c_str(), "wb");
  if (!filedump_fd) {
    error("can't record to file %s: %s", path.c_str(), strerror(errno));
    return false;
  }
  strcpy(filedump, path.c_str());
  write_to_disk = true;
  act("Encoder %s recording to file %s", name, filedump);
  return true;
}

bool VideoEncoder::filedump_close() {
  std::lock_guard<VideoEncoder> lock(*this);
  if (!filedump_fd)
    return false;
  flush_pending_output();
  drain_output();
  const int result = fclose(filedump_fd);
  filedump_fd = nullptr;
  write_to_disk = false;
  if (result)
    error("could not close recording %s: %s", filedump, strerror(errno));
  else
    act("Encoder %s stopped recording to file %s", name, filedump);
  return result == 0;
}
