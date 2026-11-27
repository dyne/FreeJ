
/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <kysucix@dyne.org>
 *
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

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <string>

#include <screen.h>
#include <video_layer.h>
#ifdef WITH_JAVASCRIPT
#include <jsparser_data.h>
#endif

#include "media/movie_decoder.h"
#include "media/movie_registration.h"

namespace {
constexpr std::int64_t no_mark = -1;
constexpr double microseconds_per_second = 1000000;

void *make_video_layer() { return new VideoLayer; }
}

int VideoLayer::isRegistered = 0;

void freej_media::register_movie_layer() {
  static const int registered = Factory<Layer>::register_instantiator(
      "MovieLayer", "ffmpeg", make_video_layer);
  if (registered)
    Factory<Layer>::set_default_classtype("MovieLayer", "ffmpeg");
}

struct VideoLayer::Playback {
  std::mutex mutex;
  std::unique_ptr<freej_media::MovieDecoder> decoder;
  freej_media::MovieDecoder::Frame frame;
  std::vector<std::uint32_t> pixels;
  std::string path;
  std::int64_t position = 0;
  std::int64_t mark_in = no_mark;
  std::int64_t mark_out = no_mark;
  bool paused = false;
  bool ended = false;
  bool rewind = false;
};

VideoLayer::VideoLayer()
    : to_seek(-1), audio_channels(0), audio_samplerate(0), playback_(new Playback) {
  set_name("VID");
  type = VIDEOLAYER;
  use_audio = true;
#ifdef WITH_JAVASCRIPT
  jsclass = &video_layer_class;
#endif
}

VideoLayer::~VideoLayer() {
  close();
}

bool VideoLayer::_init() { return true; }

bool VideoLayer::open(const char *file) {
  std::unique_ptr<freej_media::MovieDecoder> decoder(new freej_media::MovieDecoder);
  if (!decoder->open(file)) {
    error("VideoLayer: cannot open '%s': %s", file ? file : "(null)",
          decoder->last_error().c_str());
    return false;
  }

  const bool restart = is_running();
  stop();

  {
    auto &p = *playback_;
    std::lock_guard<std::mutex> playback_lock(p.mutex);
    std::lock_guard<Layer> layer_lock(*this);
#ifdef WITH_AUDIO
    if (screen)
      screen->flush_audio(*this);
#endif
    clear_frame();
    geo.init(decoder->width(), decoder->height(), 32);
    need_crop = true;
    audio_channels = decoder->channels();
    audio_samplerate = decoder->sample_rate();
    frame_rate = decoder->frame_rate();
    fps.set(frame_rate);
    set_filename(file);

    p.decoder = std::move(decoder);
    p.path = file;
    p.pixels.clear();
    p.mark_in = p.mark_out = no_mark;
    p.position = 0;
    p.paused = p.ended = p.rewind = false;
    to_seek = -1;
    opened = true;
  }
  return !restart || start() == 0;
}

void VideoLayer::close() {
  stop();
  auto &p = *playback_;
  std::lock_guard<std::mutex> playback_lock(p.mutex);
  std::lock_guard<Layer> layer_lock(*this);
#ifdef WITH_AUDIO
  if (screen)
    screen->flush_audio(*this);
#endif
  clear_frame();
  p.decoder.reset();
  p.frame = {};
  p.pixels.clear();
  audio_channels = audio_samplerate = 0;
  opened = false;
}

bool VideoLayer::audio_format(int &sample_rate, int &channels) const {
  auto &p = *playback_;
  std::lock_guard<std::mutex> lock(p.mutex);
  sample_rate = audio_samplerate;
  channels = audio_channels;
  return sample_rate > 0 && channels > 0;
}

void *VideoLayer::feed() {
  using Result = freej_media::MovieDecoder::Result;
  auto &p = *playback_;
  std::unique_lock<std::mutex> lock(p.mutex);
  if (!p.decoder)
    return nullptr;
  if (p.paused)
    return p.pixels.empty() ? nullptr : p.pixels.data();

  auto target = to_seek.exchange(-1);
  const bool loop_mark = p.mark_in >= 0 && p.mark_out > p.mark_in;
  const auto beginning = loop_mark ? p.mark_in : 0;
  const bool manual_seek = target >= 0;
  const bool rewind = p.rewind;
  if (!manual_seek && (rewind || (loop_mark && p.position >= p.mark_out)))
    target = beginning;
  if (target >= 0) {
    if (loop_mark)
      target = std::max(p.mark_in, std::min(target, p.mark_out));
    if (p.decoder->seek(target) || (target == 0 && p.decoder->open(p.path.c_str()))) {
      p.position = target;
      p.ended = false;
#ifdef WITH_AUDIO
      if (manual_seek) {
        std::lock_guard<Layer> layer_lock(*this);
        if (screen)
          screen->flush_audio(*this);
      }
#endif
    } else {
      warning("VideoLayer: cannot seek '%s' to %lld us: %s", p.path.c_str(),
              static_cast<long long>(target), p.decoder->last_error().c_str());
    }
    p.rewind = false;
  }

  if (p.ended)
    return p.pixels.empty() ? nullptr : p.pixels.data();

#ifdef WITH_AUDIO
  {
    std::lock_guard<Layer> layer_lock(*this);
    if (screen && use_audio)
      p.decoder->set_output_sample_rate(screen->audio_rate(*this));
  }
#endif

  // Keep the most recent picture until there is a new one. Bound the number
  // of decoded audio frames handed off before the worker yields.
  for (int work = 0; work < 32; ++work) {
    const auto result = p.decoder->next(p.frame);
    if (result == Result::video) {
      p.position = p.frame.timestamp;
      p.pixels.swap(p.frame.pixels);
      break;
    }
    if (result == Result::audio) {
#ifdef WITH_AUDIO
      std::lock_guard<Layer> layer_lock(*this);
      if (screen && use_audio)
        screen->write_audio(*this, p.frame.samples.data(), p.frame.samples.size(),
                            audio_channels, p.decoder->output_sample_rate());
#endif
      continue;
    }
    if (result == Result::end) {
      p.ended = p.rewind = true;
      auto *pixels = p.pixels.empty() ? nullptr : p.pixels.data();
      lock.unlock();
      eos.notify();
      return pixels;
    }
    if (result == Result::error) {
      p.ended = true;
      warning("VideoLayer: decode failed for '%s' at %lld us: %s", p.path.c_str(),
              static_cast<long long>(p.position), p.decoder->last_error().c_str());
    }
    break;
  }
  return p.pixels.empty() ? nullptr : p.pixels.data();
}

bool VideoLayer::relative_seek(double increment) {
  auto &p = *playback_;
  std::lock_guard<std::mutex> lock(p.mutex);
  if (!p.decoder || !std::isfinite(increment))
    return false;
  const long double requested = p.position + increment * microseconds_per_second;
  const auto length = p.decoder->duration();
  if (requested > 0 && length > 0 && requested > length)
    to_seek = static_cast<std::int64_t>(std::fmod(requested, length));
  else if (requested > std::numeric_limits<std::int64_t>::max())
    return false;
  else
    to_seek = static_cast<std::int64_t>(std::max(0.0L, requested));
  return true;
}

bool VideoLayer::set_mark_in() {
  std::lock_guard<std::mutex> lock(playback_->mutex);
  playback_->mark_in = playback_->mark_in == no_mark ? playback_->position : no_mark;
  return true;
}

bool VideoLayer::set_mark_out() {
  std::lock_guard<std::mutex> lock(playback_->mutex);
  playback_->mark_out = playback_->mark_out == no_mark ? playback_->position : no_mark;
  return true;
}

void VideoLayer::pause() {
  std::lock_guard<std::mutex> lock(playback_->mutex);
  playback_->paused = !playback_->paused;
}
