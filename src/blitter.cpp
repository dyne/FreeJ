/*  FreeJ - blitter layer component
 *
 *  (c) Copyright 2004-2009 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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


#include <layer.h>
#include <blitter.h>
#include <context.h>
#include <iterator.h>
#include <linklist.h>


#include <sdl_screen.h>

#include <jutils.h>
#include <config.h>





Blit::Blit() :Entry() {
  sprintf(desc,"none");

  value = 0.0;

  fun = NULL;
  type = NONE;
  past_frame = NULL;

}

Blit::~Blit() {
  // clean up all parameters
  Parameter *par;
  par = parameters.begin();
  while(par) {
    par->rem();
    delete par;
    par = parameters.begin();
  }
}

Blitter::Blitter() {

  screen = NULL;

  old_lay_x = 0;
  old_lay_y = 0;
  old_lay_w = 0;
  old_lay_h = 0;


}

Blitter::~Blitter() {

  Blit *b = blitlist.begin();
  while(b) {
    b->rem();
    delete(b);
    b = blitlist.begin();
  }

}



// char *Blitter::get_name() {
//   return name;
// }
		       


  // TODO layer 2 layer blit
// pass src->geo and dst->geo from two layers
// in case we want to blit one on another
// then the crop will prepare it's own layer to the other

/* ok, let's draw the crop geometries and be nice commenting ;)

   that's tricky stuff

   here the generic case of a layer on the screen, with variable names:
   

                                                         screen->w
  0,0___________________________________________________ -> 
   '                 ^                                  '
   | screen          |scr_stride_up                     |
   |                 |                                  |
   |       x,y_______V________________ w                |
   | scr_sx '                         '                 |
   | stride | layer                   |                 |
   |<------>|                         |                 |
   |        |                         |<-------------...|
   |...---->|                         | scr_stride_dx   |
   |        '-------------------------'                 |
   |       h                                            |
   |                                                    |
   '----------------------------------------------------'
   |
   V screen->h

   we have a couple of cases in which both x and y as well
   w and h of the layer can be out of the bounds of the screen.

   for instance, if the layer goes out of the left bound (x<0):

            0,0____________________
 (offset)    '                     '
  x,y________|_________            |  offset is the point of start
   '         |         '           |  scr_stride is added to screen every line
   |layer    |         |           |  lay_stride is added to layer every line
   |         |         | scr_stride|
   |         |         |<--------->|
   |         |         |           |
   '_________|_________'           |
   <-------->|                     |
   lay_stride'---------------------'

     so the algorithm of the crop will look like:
*/
     

void Blitter::crop(Layer *lay, ViewPort *scr) {

  Blit *b;

  if(!lay || !scr) {
    warning("crop called with lay[%p] scr[%p]", lay, scr);
    return;
  }

  if(!lay->current_blit) {
    warning("no blit selected for layer %s", lay->name);
    return;
  }

  b = lay->current_blit;

  func("crop on layer %s x%i y%i w%i h%i for blit %s",
       lay->name, lay->geo.x, lay->geo.y,
       lay->geo.w, lay->geo.h, b->name);

  // assign the right pointer to the *geo used in crop
  // we use the normal geometry if not roto|zoom
  // otherwise the layer::geo_rotozoom
  if(lay->rotating | lay->zooming) {

    geo = &lay->geo_rotozoom;

    // shift up/left to center rotation
    geo->x = lay->geo.x - (geo->w - lay->geo.w)/2;
    geo->y = lay->geo.y - (geo->h - lay->geo.h)/2;

  } else geo = &lay->geo;

  //////////////////////


  
  // crop for the SDL blit
  if(b->type == Blit::SDL) {

    b->sdl_rect.x = -(geo->x);
    b->sdl_rect.y = -(geo->y);
    b->sdl_rect.w = scr->geo.w;
    b->sdl_rect.h = scr->geo.h;

    // crop for the linear and past blit
  } else if(b->type == Blit::LINEAR 
	    || b->type == Blit::PAST) {

    b->lay_pitch =  geo->w; // how many pixels to copy each row
    b->lay_height = geo->h; // how many rows we should copy
    
    b->scr_stride_up = 0; // rows to jump before starting to blit on screen
    b->scr_stride_sx = 0; // screen pixels stride on the left of each row
    b->scr_stride_dx = 0; // screen pixels stride on the right of each row
    
    b->lay_stride_up = 0; // rows to jump before starting to blit layer
    b->lay_stride_sx = 0; // how many pixels stride on the left of each row
    b->lay_stride_dx = 0; // how many pixels stride on the right of each row
    
    // BOTTOM
    if( geo->y + geo->h > scr->geo.h ) {
      if( geo->y > scr->geo.h ) { // out of screen
	geo->y = scr->geo.h+1; // don't go far
	lay->hidden = true;
	return;
      } else { // partially out
	b->lay_height -= (geo->y + geo->h) - scr->geo.h;
      }
    }
    
    // LEFT
    if( geo->x < 0 ) {
      if( geo->x + geo->w < 0 ) { // out of screen
	geo->x = -( geo->w + 1 ); // don't go far
	lay->hidden = true;
	return;
      } else { // partially out
	b->lay_stride_sx += -geo->x;
	b->lay_pitch -= -geo->x;
      } 
    } else { // inside
      b->scr_stride_sx += geo->x;
    }
    
    // UP
    if(geo->y < 0) {
      if( geo->y + geo->h < 0) { // out of screen
	geo->y = -( geo->h + 1 ); // don't go far
	lay->hidden = true;
	return;
      } else { // partially out
	b->lay_stride_up += -geo->y;
	b->lay_height -= -geo->y;
      }
    } else { // inside
      b->scr_stride_up += geo->y;
    }
    
    // RIGHT
    if( geo->x + geo->w > scr->geo.w ) {
      if( geo->x > scr->geo.w ) { // out of screen
	geo->x = scr->geo.w + 1; // don't go far
	lay->hidden = true;
	return;
      } else { // partially out
	b->lay_pitch -= ( geo->x + geo->w ) - scr->geo.w;
	b->lay_stride_dx += ( geo->x + geo->w ) - scr->geo.w;
      } 
    } else { // inside
      b->scr_stride_dx += scr->geo.w - (geo->x + geo->w );
    }
    
    lay->hidden = false;
    
    b->lay_stride = b->lay_stride_dx + b->lay_stride_sx; // sum strides
    // precalculate upper left starting offset for layer
    b->lay_offset = (b->lay_stride_sx +
		     ( b->lay_stride_up * geo->w ));
    
    b->scr_stride = b->scr_stride_dx + b->scr_stride_sx; // sum strides
    // precalculate upper left starting offset for screen
    b->scr_offset = (b->scr_stride_sx +
		     ( b->scr_stride_up * scr->geo.w ));
  }
  
  // calculate bytes per row
  b->lay_bytepitch = b->lay_pitch * 4;

  lay->need_crop = false;
}  


//////////////////

///////// LEFTOVERS


//   /* compare old layer values
//      crop the layer only if necessary */
//   if(!force)
//     if( (lay->geo->x == old_lay_x)
// 	&& (lay->geo->y == old_lay_y)
// 	&& (lay->geo->w == old_lay_w)
// 	&& (lay->geo->h == old_lay_h)
// 	&& (scr->w == old_scr_w)
// 	&& (scr->h == old_scr_h)
// 	)
//       return;




//   // assign the right pointer to the *geo used in crop
//   // we use the normal geometry if not roto|zoom
//   // otherwise the layer::geo_rotozoom
//   if(rotozoom) {

//     geo = &geo_rotozoom;
//     // shift up/left to center rotation
//     geo->x = layer->geo.x - (rotozoom->w - layer->geo.w)/2;
//     geo->y = layer->geo.y - (rotozoom->h - layer->geo.h)/2;

//     geo->w = rotozoom->w;
//     geo->h = rotozoom->h;
//     geo->bpp = 32;
//     geo->pitch = 4*geo->w;

//   } else geo = &layer->geo;

//   // QUAAA

//   if(layer->slide_x != geo->x) geo->x = (int16_t)layer->slide_x;
//   if(layer->slide_y != geo->y) geo->y = (int16_t)layer->slide_y;

//   //////////////////////
