/*  FreeJ Movie Layer (based on MLT framework)
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
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
 * "$Id: $"
 *
 */

#include <math.h>

#include <config.h>

#include <context.h>
#include <jutils.h>

#include <movie_layer.h>



MovieLayer::MovieLayer()
  :Layer() {

  set_name("MOV");

  buffer = NULL;
  
  real_producer = NULL;
  properties = NULL;
  service = NULL;

}

MovieLayer::~MovieLayer() {

  //  if(producer) mlt_producer_close( producer );
  if(real_producer)
    mlt_producer_close( real_producer);

  if(buffer) free(buffer);
}

bool MovieLayer::init(Context *freej) {

  env = freej;

  set_name("MOV");

  return(true);
}


bool MovieLayer::open(char *file) {

  real_producer = mlt_factory_producer( "avformat", file ); // def: "fezzik"
  if(!file) {
    error("Movie layer fails opening file %s",file);
    return(false);
  }

  // Get the properties of this producer
  properties = MLT_PRODUCER_PROPERTIES( real_producer );

  _init( mlt_properties_get_int( properties, "width"),
	 mlt_properties_get_int( properties, "height") );
  
  buffer = (uint8_t*)jalloc(geo.size); // RGBA buffer


  service = mlt_producer_service( real_producer );
  //  mlt_properties_inc_ref( MLT_SERVICE_PROPERTIES( service ) );

  set_filename(file);

  act("opened movie %s",file);

  opened = true;

  return(true);
}

void *MovieLayer::feed() {

  mlt_image_format format = mlt_image_yuv422;
  //  mlt_frame frame = mlt_frame_init();
  mlt_frame frame;
  uint8_t *image;
  int width = geo.w;
  int height = geo.h;
  int err;

  // Get the frame
  err = mlt_service_get_frame( service, &frame, 1 );

  //  (real_producer->get_frame)(real_producer, &frame, 0);


  err = mlt_frame_get_image( frame, &image, &format, &width, &height, 0 );
  if( err != 0 ) {
    error("frame_get_image died");
    return NULL;
  }
  
  //  mlt_properties_set_int( MLT_FRAME_PROPERTIES( frame ), "format", format );
  
  //  memcpy(buffer, image, size );

  mlt_convert_yuv422_to_rgb24a( image, (uint8_t*)buffer, width * height);
  mlt_properties_set_int( MLT_FRAME_PROPERTIES( frame ), "rendered", 1 );

  mlt_frame_close( frame );

  return (void*)buffer;
}

void MovieLayer::close() {

  if ( real_producer ) {
    mlt_producer_close( real_producer );
    real_producer = NULL;
  }
  
}


bool MovieLayer::keypress(int k) {
  /* neither this */
  return(true);
}

