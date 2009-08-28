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

#include <config.h>
#ifdef WITH_GOOM

#include <math.h>

#include <jutils.h>
#include <context.h>
#include <audio_collector.h>

#include <goom_layer.h>

#include <jsparser_data.h>


GoomLayer::GoomLayer()
  :Layer() {
  set_name("Goom");
  buffer = NULL;
  use_audio = true;
  jsclass = &goom_layer_class;
}

GoomLayer::~GoomLayer() {
  close();
}

bool GoomLayer::init(Context *freej) {

  int width  = freej->screen->w;
  int height = freej->screen->h;

  func("GoomLayer::init()");

  _init(width,height);
  
  goom = goom_init(geo.w, geo.h);

  buffer = malloc(geo.size);

  
  goom_set_screenbuffer(goom, buffer);

  opened = true;

  return(true);
}

bool GoomLayer::open(const char *file) {

  return true;
}

void GoomLayer::close() {
  if(buffer)
    free(buffer);

}

void *GoomLayer::feed() {
  int c;
  short int hc;
  // TODO: use FFT values and fill in incval and samples

  if(audio) {
    
    audio->GetFFT();

    /* find the max */
    float incvar = 0;
    for (c = 0; c < 16; c++) {
      hc = (int) floor(audio->GetHarmonic(c));
      if (incvar < hc) incvar = hc;
      goom->sound.samples[c] = hc;
    }
    
    if (incvar > goom->sound.allTimesMax)
      goom->sound.allTimesMax = incvar;
    
    /* volume sonore */
    goom->sound.volume = (float)incvar / (float)goom->sound.allTimesMax;

  }
  //  ringbuffer_peek(env->audio->input_pipe, (char*)audio, 1024);

  goom_update(goom, audiobuf, 0, -1, NULL, NULL);

  return buffer;
}

// bool GoomLayer::keypress(int key) {
//   return false;
// }

#endif
