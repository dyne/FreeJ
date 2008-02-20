/*  FreeJ
 *  (c) Copyright 2001-2006 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#include <jutils.h>
#include <context.h>
#include <audio_collector.h>

#include <goom_layer.h>

#include <config.h>


GoomLayer::GoomLayer()
  :Layer() {
  set_name("Goom");
  buffer = NULL;
  use_audio = true;
}

GoomLayer::~GoomLayer() {
  close();
}

bool GoomLayer::init(Context *freej) {

  int width  = freej->screen->w;
  int height = freej->screen->h;

  func("GoomLayer::init()");

  if(!freej->audio) {
    error("can't create Goom Layer: audio is not connected");
    return false;
  }

  _init(width,height);
  
  goom = goom_init(geo.w, geo.h);

  buffer = jalloc(geo.size);

  
  goom_set_screenbuffer(goom, buffer);


  return(true);
}

bool GoomLayer::open(char *file) {

  return true;
}

void GoomLayer::close() {
  if(buffer)
    free(buffer);

}

void *GoomLayer::feed() {
  int c;
  //  int num, found, samples;

  //  samples = env->audio->framesperbuffer * env->audio->channels;

  //  num = samples * sizeof(int16_t);
  
  
  /*  num = env->audio->input->read(num,audiotmp);

  if(num<=0) {
  func("no audio for goom");
  return buffer;
  }

  if(num!=samples*sizeof(int16_t)*env->audio->channels) {
    warning("goom audio buffer underrun");
    samples = num / sizeof(int16_t) / env->audio->channels;
    } */

  
//   if(env->audio->channels == 2) {
//     for( c = 0; c<512 ; c++ ) {
//       audio[0][c] = (int16_t) (env->audio->input[c*2]);
//       audio[1][c] = (int16_t) (env->audio->input[(c*2)+1]);
//     }
//   } else {
//     for( c = 0; c<512 ; c++ ) {
//       audio[0][c] = (int16_t) (env->audio->input[c]);
//       audio[1][c] = (int16_t) (env->audio->input[c]);
//     }
//   }
  float *buffer = env->audio->GetAudioBuffer();

  for( c = 0; c<512 ; c++) {
    audio[0][c] = (int16_t) buffer[c];
    audio[1][c] = (int16_t) buffer[c];
  }
  //  ringbuffer_peek(env->audio->input_pipe, (char*)audio, 1024);

  goom_update(goom, audio, 0, -1, NULL, NULL);

  return buffer;
}

bool GoomLayer::keypress(int key) {
  return false;
}
