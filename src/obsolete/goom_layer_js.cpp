/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: freej.cpp 654 2005-08-18 16:52:47Z jaromil $"
 *
 */

#include <config.h>
#ifdef WITH_GOOM

#include <callbacks_js.h>
#include <jsparser_data.h>

#include <goom_layer.h>

DECLARE_CLASS_GC("GoomLayer",goom_layer_class,goom_layer_constructor,js_layer_gc);

////////////////////////////////
// Goom Layer methods
JSFunctionSpec goom_layer_methods[] = {
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "mode",         goom_layer_mode,        1  },
  {     "middle",      goom_layer_middle,       2  },
  {     "reverse",     goom_layer_reverse,      1  },
  {     "speed",      goom_layer_speed,         1  },
  {     "plane",    goom_layer_plane,           2  },
  {     "wave",         goom_layer_wave,        1  },
  {     "hypercos",     goom_layer_hypercos,    1  },
  {     "noise",    goom_layer_noise,           1  },
  {0}
};

JS_CONSTRUCTOR("GoomLayer", goom_layer_constructor, GoomLayer);

JS(goom_layer_mode) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  GET_LAYER(GoomLayer);

  jsint mod = js_get_int(argv[0]);
  if(mod>9)
    mod = 9;
  else if(mod<0)
    mod = 0;

  lay->goom->update.zoomFilterData.mode = mod;

  return JS_TRUE;
}


JS(goom_layer_speed) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  GET_LAYER(GoomLayer);

  jsint spd = js_get_int(argv[0]);

  if(spd>256)
    spd = 256;
  else if(spd<0)
    spd = 0;

  lay->goom->update.zoomFilterData.vitesse = spd;

  return JS_TRUE;
}


JS(goom_layer_middle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(2);

  GET_LAYER(GoomLayer);

  jsint x = js_get_int(argv[0]);
  jsint y = js_get_int(argv[1]);

  lay->goom->update.zoomFilterData.middleX = x;
  lay->goom->update.zoomFilterData.middleY = y;

  return JS_TRUE;
}


JS(goom_layer_plane) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(2);

  GET_LAYER(GoomLayer);

  jsint h = js_get_int(argv[0]);
  jsint v = js_get_int(argv[1]);

  lay->goom->update.zoomFilterData.hPlaneEffect = h;
  lay->goom->update.zoomFilterData.vPlaneEffect = v;

  return JS_TRUE;
}


JS(goom_layer_reverse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  GET_LAYER(GoomLayer);

  jsint rev = js_get_int(argv[0]);

  lay->goom->update.zoomFilterData.reverse = (char)rev;

  return JS_TRUE;
}

JS(goom_layer_noise) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  GET_LAYER(GoomLayer);

  jsint noi = js_get_int(argv[0]);

  lay->goom->update.zoomFilterData.noisify = (char)noi;

  return JS_TRUE;
}

JS(goom_layer_hypercos) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  GET_LAYER(GoomLayer);

  jsdouble = js_get_double(argv[0]);

  lay->goom->update.zoomFilterData.hypercosEffect = hyp;
  // TODO

  return JS_TRUE;
}
JS(goom_layer_wave) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  GET_LAYER(GoomLayer);

  jsdouble = js_get_double(argv[0]);

  lay->goom->update.zoomFilterData.waveEffect = wav;
  // TODO

  return JS_TRUE;
}

#endif
