/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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
 */

#ifndef FREEJ_VIDEO_LAYER_H
#define FREEJ_VIDEO_LAYER_H

#include <atomic>
#include <cstdint>
#include <memory>

#include <callback.h>
#include <factory.h>
#include <layer.h>

class VideoLayer : public Layer {
public:
  VideoLayer();
  ~VideoLayer() override;

  bool open(const char *file) override;
  void *feed() override;
  void close() override;
  bool audio_format(int &sample_rate, int &channels) const override;

  // Absolute position in microseconds. Negative means no pending seek.
  std::atomic<std::int64_t> to_seek;
  bool relative_seek(double increment);
  bool set_mark_in();
  bool set_mark_out();
  void pause();

  bool add_eos_call(DumbCall *call) { return eos.add_call(call); }
  bool rem_eos_call(DumbCall *call) { return eos.rem_call(call); }

  int audio_channels;
  int audio_samplerate;

protected:
  bool _init() override;

private:
  struct Playback;
  std::unique_ptr<Playback> playback_;
  DumbCallback eos;

  FACTORY_ALLOWED
};

#endif
