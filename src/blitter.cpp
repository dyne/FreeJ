/*  FreeJ - blitter layer component
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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


#include <layer.h>
#include <blitter.h>
#include <context.h>
#include <iterator.h>
#include <imagefilter.h>
#include <rotozoomer.h>

#include <sdl_screen.h>

#include <jutils.h>
#include <config.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static uint32_t rmask = 0x00ff0000;
static uint8_t rchan = 1;
static uint32_t gmask = 0x0000ff00;
static uint8_t gchan = 2;
static uint32_t bmask = 0x000000ff;
static uint8_t bchan = 3;
static uint32_t amask = 0xff000000;
static uint8_t achan = 0;
#else
static uint32_t rmask = 0x000000ff;
static uint8_t rchan = 2;
static uint32_t gmask = 0x0000ff00;
static uint8_t gchan = 1;
static uint32_t bmask = 0x00ff0000;
static uint8_t bchan = 0;
static uint32_t amask = 0xff000000;
static uint8_t achan = 3;
#endif

// blit functions prototype
#define BLIT static inline void


// Linear transparent blits

BLIT red_channel(void *src, void *dst, int bytes, void *value) {
  register int c;
  register uint8_t *s = (uint8_t*)src;
  register uint8_t *d = (uint8_t*)dst;
  for(c=bytes>>2;c>0;c--,s+=4,d+=4)
    *(d+rchan) = *(s+rchan);
}

BLIT green_channel(void *src, void *dst, int bytes, void *value) {
  register int c;
  register uint8_t *s = (uint8_t*)src;
  register uint8_t *d = (uint8_t*)dst;
  for(c=bytes>>2;c>0;c--,s+=4,d+=4)
    *(d+gchan) = *(s+gchan);
}

BLIT blue_channel(void *src, void *dst, int bytes, void *value) {
  register int c;
  register uint8_t *s = (uint8_t*)src;
  register uint8_t *d = (uint8_t*)dst;
  for(c=bytes>>2;c>0;c--,s+=4,d+=4)
    *(d+bchan) = *(s+bchan);
}

BLIT schiffler_add(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterAdd((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_sub(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterSub((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_mean(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterMean((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_absdiff(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterAbsDiff((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_mult(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterMult((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_multnor(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterMultNor((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_div(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterDiv((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_multdiv2(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterMultDivby2((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_multdiv4(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterMultDivby2((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_and(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterBitAnd((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_or(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterBitOr((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

/* ====== end of transparent blits
   all the following blits can be considered effects
   they completely overwrite the underlying image */

/// Linear non-transparent blits

BLIT schiffler_neg(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterBitNegation((unsigned char*)src,(unsigned char*)dst,bytes);
}

BLIT schiffler_addbyte(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterAddByte((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_addbytetohalf(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterAddByteToHalf((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_subbyte(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterSubByte((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_shl(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterShiftLeft((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_shlb(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterShiftLeftByte((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_shr(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterShiftRight((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_mulbyte(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterMultByByte((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

BLIT schiffler_binarize(void *src, void *dst, int bytes, void *value) {
  SDL_imageFilterBinarizeUsingThreshold
    ((unsigned char*)src,(unsigned char*)dst,bytes, *(unsigned char*)value);
}

/// Past-frame blits

BLIT past_add(void *src, void *past, void *dst, int bytes) {
  SDL_imageFilterAdd((unsigned char*)src,
		     (unsigned char*)past,
		     (unsigned char*)dst,bytes);
}

BLIT past_addneg(void *src, void *past, void *dst, int bytes) {
  SDL_imageFilterAdd((unsigned char*)src,
		     (unsigned char*)past,
		     (unsigned char*)dst,bytes);
  SDL_imageFilterBitNegation((unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT past_absdiff(void *src, void *past, void *dst, int bytes) {
  SDL_imageFilterAbsDiff((unsigned char*)src,
			 (unsigned char*)past,
			 (unsigned char*)dst,bytes);
}

///////////////////////////////////////////////////////////////////
// SDL BLITS
// TODO: more SDL blits playing with color masks

// temporary surface used in SDL blits
static SDL_Surface *sdl_surf;

BLIT sdl_rgb(void *src, SDL_Rect *src_rect,
	     SDL_Surface *dst, SDL_Rect *dst_rect,
	     ScreenGeometry *geo, void *value) {
  
  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->pitch, bmask, gmask, rmask, 0x0);
  
  SDL_BlitSurface( sdl_surf, src_rect, dst, dst_rect );
  
  SDL_FreeSurface( sdl_surf );

}

BLIT sdl_alpha(void *src, SDL_Rect *src_rect,
	       SDL_Surface *dst, SDL_Rect *dst_rect,
	       ScreenGeometry *geo, void *value) {

  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->pitch, bmask, gmask, rmask, 0x0);

  SDL_SetAlpha( sdl_surf, SDL_SRCALPHA|SDL_RLEACCEL, *(unsigned int*)value );  

  SDL_BlitSurface( sdl_surf, src_rect, dst, dst_rect );
  
  SDL_FreeSurface( sdl_surf );
  
}

BLIT sdl_srcalpha(void *src, SDL_Rect *src_rect,
		  SDL_Surface *dst, SDL_Rect *dst_rect,
		  ScreenGeometry *geo, void *value) {

  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->pitch, bmask, gmask, rmask, amask);

  //  SDL_SetAlpha( sdl_surf, SDL_SRCALPHA|SDL_RLEACCEL, *(unsigned int*)value );  

  SDL_BlitSurface( sdl_surf, src_rect, dst, dst_rect );
  
  SDL_FreeSurface( sdl_surf );
  
}

BLIT sdl_chromakey(void *src, SDL_Rect *src_rect,
		   SDL_Surface *dst, SDL_Rect *dst_rect,
		   ScreenGeometry *geo, void *value) {

  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->pitch, bmask, gmask, rmask, amask);

  SDL_SetColorKey( sdl_surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, *(uint32_t*)value);

//  SDL_SetAlpha(sdl_surf, SDL_RLEACCEL, 0);

  SDL_Surface *colorkey_surf=SDL_DisplayFormat(sdl_surf);

  SDL_BlitSurface( colorkey_surf, src_rect, dst, dst_rect );
  
  SDL_FreeSurface( sdl_surf );
  SDL_FreeSurface( colorkey_surf );

  sdl_surf = NULL;

}

Blit::Blit() :Entry() {
  sprintf(desc,"none");

  value = 0x0;
  has_value = false;

  memset(kernel,0,256);
  fun = NULL;
  type = 0x0;
  past_frame = NULL;

}

Blit::~Blit() { }

Blitter::Blitter() {
  current_blit = NULL;

  Blit *b;

  zoom_x = 1.0;
  zoom_y = 1.0;
  rotate = 0.0;
  zooming = false;
  rotating = false;
  rotozoom = NULL;
  antialias = false;

  old_x = 0;
  old_y = 0;
  old_w = 0;
  old_h = 0;


  /* fill up linklist of blits */

  // default blit is SDLCPY RGB
  b = new Blit(); b->set_name("RGB");
  sprintf(b->desc,"RGB blit (SDL)");
  b->type = SDL_BLIT;
  b->sdl_fun = sdl_rgb; blitlist.append(b);
  current_blit = b; b->sel(true);


  b = new Blit(); b->set_name("ADD");
  sprintf(b->desc,"bytewise addition");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_add; blitlist.append(b);
  
  b = new Blit(); b->set_name("SUB");
  sprintf(b->desc,"bytewise subtraction");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_sub; blitlist.append(b);
  
  b = new Blit(); b->set_name("MEAN");
  sprintf(b->desc,"bytewise mean");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_add; blitlist.append(b);

  b = new Blit(); b->set_name("ABSDIFF");
  sprintf(b->desc,"absolute difference");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_absdiff; blitlist.append(b);

  b = new Blit(); b->set_name("MULT");
  sprintf(b->desc,"multiplication");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_mult; blitlist.append(b);

  b = new Blit(); b->set_name("MULTNOR");
  sprintf(b->desc,"normalized multiplication");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_multnor; blitlist.append(b);

  b = new Blit(); b->set_name("DIV");
  sprintf(b->desc,"division");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_div; blitlist.append(b);

  b = new Blit(); b->set_name("MULTDIV2");
  sprintf(b->desc,"multiplication and division by 2");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_multdiv2; blitlist.append(b);

  b = new Blit(); b->set_name("MULTDIV4");
  sprintf(b->desc,"multiplication and division by 4");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_multdiv4; blitlist.append(b);

  b = new Blit(); b->set_name("AND");
  sprintf(b->desc,"bitwise and");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_and; blitlist.append(b);

  b = new Blit(); b->set_name("OR");
  sprintf(b->desc,"bitwise or");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_or; blitlist.append(b);

  b = new Blit(); b->set_name("RED");
  sprintf(b->desc,"red channel only blit");
  b->type = LINEAR_BLIT;
  b->fun = red_channel; blitlist.append(b);

  b = new Blit(); b->set_name("GREEN");
  sprintf(b->desc,"green channel only blit");
  b->type = LINEAR_BLIT;
  b->fun = green_channel; blitlist.append(b);

  b = new Blit(); b->set_name("BLUE");
  sprintf(b->desc,"blue channel only blit");
  b->type = LINEAR_BLIT;
  b->fun = blue_channel; blitlist.append(b);

  b = new Blit(); b->set_name("NEG");
  sprintf(b->desc,"bitwise negation");
  b->type = LINEAR_BLIT;
  b->fun = schiffler_neg; blitlist.append(b);

  b = new Blit(); b->set_name("ADDB");
  sprintf(b->desc,"add byte to bytes");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_addbyte; blitlist.append(b);

  b = new Blit(); b->set_name("ADDBH");
  sprintf(b->desc,"add byte to half");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_addbytetohalf; blitlist.append(b);
  
  b = new Blit(); b->set_name("SUBB");
  sprintf(b->desc,"subtract byte to bytes");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_subbyte; blitlist.append(b);

  b = new Blit(); b->set_name("SHL");
  sprintf(b->desc,"shift left bits");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_shl; blitlist.append(b);

  b = new Blit(); b->set_name("SHLB");
  sprintf(b->desc,"shift left byte");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_shlb; blitlist.append(b);

  b = new Blit(); b->set_name("SHR");
  sprintf(b->desc,"shift right bits");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_shr; blitlist.append(b);

  b = new Blit(); b->set_name("MULB");
  sprintf(b->desc,"multiply by byte");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_mulbyte; blitlist.append(b);

  b = new Blit(); b->set_name("BIN");
  sprintf(b->desc,"binarize using threshold");
  b->type = LINEAR_BLIT; b->has_value = true;
  b->fun = schiffler_binarize; blitlist.append(b);

  // SDL blits
  b = new Blit(); b->set_name("ALPHA");
  sprintf(b->desc,"alpha blit (SDL)");
  b->type = SDL_BLIT; b->has_value = true;
  b->sdl_fun = sdl_alpha; blitlist.append(b);

  b = new Blit(); b->set_name("SRCALPHA");
  sprintf(b->desc,"source alpha blit (SDL)");
  b->type = SDL_BLIT;
  b->sdl_fun = sdl_srcalpha; blitlist.append(b);
  
  b = new Blit(); b->set_name("CHROMAKEY");
  sprintf(b->desc,"chromakey blit (SDL)");
  b->type = SDL_BLIT; b->has_value = true;
  b->sdl_fun = sdl_chromakey; blitlist.append(b);

  // PAST blits
  b = new Blit(); b->set_name("PAST_ADD");
  sprintf(b->desc,"add to past frame");
  b->type = PAST_BLIT;
  b->past_fun = past_add; blitlist.append(b);

  b = new Blit(); b->set_name("PAST_ADDNEG");
  sprintf(b->desc,"add to past frame and negate");
  b->type = PAST_BLIT;
  b->past_fun = past_addneg; blitlist.append(b);

  b = new Blit(); b->set_name("PAST_ABSDIFF");
  sprintf(b->desc,"absolute difference on past frame");
  b->type = PAST_BLIT;
  b->past_fun = past_absdiff; blitlist.append(b);

}

Blitter::~Blitter() {
  Blit *tmp;
  Blit *b = (Blit*)blitlist.begin();
  while(b) {
    tmp = (Blit*)b->next;
    delete b;
    b = tmp;
  }
}

bool Blitter::init(Layer *lay) {
  layer = lay;
  func("blitter initialized for layer %s",lay->get_name());

  crop( lay->freej->screen );

  return true;
}

void Blitter::blit() {
  register int16_t c;
  void *offset;

  if(rotating || zooming) {
    // if we have to rotate or scale,
    // create a sdl surface from current pixel buffer
    sdl_surf = SDL_CreateRGBSurfaceFrom
      (layer->offset,
       layer->geo.w, layer->geo.h, layer->geo.bpp,
       layer->geo.pitch, bmask, gmask, rmask, amask);

    if(rotating) {

      rotozoom =
	schiffler_rotozoom(sdl_surf, rotate, zoom_x, (int)antialias);
      
    } else if(zooming) {
      
      rotozoom =
	schiffler_zoom(sdl_surf, zoom_x, zoom_y, (int)antialias);

    }

    offset = rotozoom->pixels;
    SDL_FreeSurface( sdl_surf );
    
  } else {

    offset = layer->offset;
    rotozoom = NULL;

  }



  crop( NULL );
  
  // executes LINEAR blit
  if( current_blit->type == LINEAR_BLIT ) {

    // setup crop variables
    pscr =
      (uint32_t*)layer->freej->screen->coords(0,0)
      + current_blit->scr_offset;
    play =
      (uint32_t*)offset
      + current_blit->lay_offset;

    // iterates the blit on each horizontal line
    for(c=current_blit->lay_height ; c>0 ; c--) {

      (*current_blit->fun)
	((void*)play, (void*)pscr,
	 current_blit->lay_bytepitch,// * layer->geo.bpp>>3,
	 (void*)&current_blit->value);

      // strides down to the next line
      pscr += current_blit->scr_stride
	+ current_blit->lay_pitch;
      play += current_blit->lay_stride
	+ current_blit->lay_pitch;

    }
    
    // executes SDL blit
  } else if (current_blit->type == SDL_BLIT) {
    
    (*current_blit->sdl_fun)
      (offset, &current_blit->sdl_rect,
       ((SdlScreen*)layer->freej->screen)->surface,
       NULL, geo, (void*)&current_blit->value);

  } else if (current_blit->type == PAST_BLIT) {
    // this is a linear blit which operates
    // line by line on the previous frame

    // setup crop variables
    pscr =
      (uint32_t*)layer->freej->screen->coords(0,0)
      + current_blit->scr_offset;
    play =
      (uint32_t*)offset
      + current_blit->lay_offset;
    ppast =
      (uint32_t*)current_blit->past_frame
      + current_blit->lay_offset;

    // iterates the blit on each horizontal line
    for(c = current_blit->lay_height; c>0; c--) {
      
      (*current_blit->past_fun)
	((void*)play, (void*)ppast, (void*)pscr,
	 current_blit->lay_bytepitch);
      
      // copy the present to the past
      jmemcpy(ppast, play, geo->pitch);
      
      // strides down to the next line
      pscr += current_blit->scr_stride
	+ current_blit->lay_pitch;
      play += current_blit->lay_stride
	+ current_blit->lay_pitch;
      ppast += current_blit->lay_stride
	+ current_blit->lay_pitch;

    }

  }

  if(rotozoom)
    SDL_FreeSurface(rotozoom);

}



bool Blitter::set_blit(char *name) {
  bool zeroing = false;
  if(name[0]=='0') /* if a 0 is in front of the name
		      we switch to 0 the value of the layer
		      before activating it */
    zeroing = true;

  Blit *b = (Blit*)blitlist.search(name+zeroing);

  if(!b) {
    error("blit %s not found",name);
    return false;
  }
  
  // found the matching name!

  if(b->type == PAST_BLIT) { // must fill previous frame
    if(b->past_frame) free(b->past_frame);
    b->past_frame = calloc(layer->geo.size,1);
  }

  if(zeroing) b->value = 0;

  current_blit = b; // start using
  crop(NULL);
  blitlist.sel(0);
  b->sel(true);


  func("blit %s selected for layer %s",
       b->get_name(),(layer)?layer->get_name():" ");
  return true;
}

void Blitter::set_value(int val) {
  current_blit->value = val;
}

bool Blitter::pulse_value(int step, int val) {
  Iterator *iter;
  iter = new Iterator(&current_blit->value);

  iter->set_mode(PULSE);
  iter->set_step(step);
  iter->set_aim(val);

  layer->iterators.add(iter);
  
  func("layer %s blit %s pulse to %i by step %i",
      layer->get_name(),current_blit->get_name(),val,step);

  return true;
}

bool Blitter::fade_value(int step, int val) {

  // if the layer is hidden then we don't bother fading
  // just set the value right away
  if(!layer->active) {
    set_value(val);
    return true;
  }

  Iterator *iter;

  /* setup an iterator to gradually change the value */
  iter = new Iterator(&current_blit->value);

  /** here we could setup the speed of the value change
      (fade_in/out speed and such), hardcoded for now */
  iter->set_mode(ONCE);
  iter->set_step(step);
  iter->set_aim(val);
  layer->iterators.add(iter);

  act("layer %s blit %s fade to %i by step %i",
      layer->get_name(),current_blit->get_name(),val,step);
  return true;
}

bool Blitter::set_colorkey(int colorkey_x,int colorkey_y) {

    Blit *b = (Blit*)blitlist.search("CHROMAKEY");
    if(!b) {
      error("can't find chromakey blit");
      return false;
    }

    uint8_t *colorkey=(uint8_t *)layer->offset + (colorkey_x<<2) + (colorkey_y * layer->geo.pitch);
	    
    uint8_t colorkey_r = *(colorkey + rchan);
    uint8_t colorkey_g = *(colorkey + gchan);
    uint8_t colorkey_b = *(colorkey + bchan);


    b->value = (colorkey_r)|((uint32_t)colorkey_g<<8)|((uint32_t)colorkey_b<<16);

      

    notice("Chromakey value: r%x g%x b%x #%x\n",
	   colorkey_r,colorkey_g,colorkey_b,b->value);
    return true;
}

bool Blitter::set_kernel(short *krn) {
  notice("Blitter::set_kernel : TODO convolution on blits");
  return false;
}

bool Blitter::set_zoom(double x, double y) {
  if(!x && !y) {
    zooming = false;
    x = y = 0;
    act("%s layer %s zoom deactivated",
	layer->get_name(), layer->get_filename());
  } else {
    zoom_x += x;
    zoom_y += y;
    zooming = true;
    act("%s layer %s zoom set to x%.2f y%.2f",	layer->get_name(),
	layer->get_filename(), zoom_x, zoom_y);
  }
  return zooming;
}

bool Blitter::set_rotate(double angle) {
  if(!angle) {
    rotating = false;
    angle = 0;
    act("%s layer %s rotation deactivated",
	layer->get_name(), layer->get_filename());
  } else {
    rotate += angle;
    rotating = true;
    act("%s layer %s rotation set to %.2f", layer->get_name(),
	layer->get_filename(), rotate);

  }
  return rotating;
}


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
     

void Blitter::crop(ViewPort *screen) {     

  Blit *b;

  if(!layer) return;

  // assign the right pointer to the *geo used in crop
  // we use the normal geometry if not roto|zoom
  // otherwise the layer::geo_rotozoom
  if(zooming | rotating) {

    if(!rotozoom) {
      error("internal error in blit rotozoomer, SDL_Surface layer::rotozoom is NULL");
      return;
    }
    geo = &geo_rotozoom;
    geo->w = rotozoom->w;
    geo->h = rotozoom->h;
    geo->x = layer->geo.x;
    geo->y = layer->geo.y;
    geo->bpp = 32;
    geo->pitch = 4*geo->w;

  } else geo = &layer->geo;

  /* compare old layer values
     crop the layer only if necessary */
  if( geo->x == old_x
      && geo->y == old_y
      && geo->w == old_w
      && geo->h == old_h )
    return;



  if(!current_blit) return;
  b = current_blit;

  func("crop on x%i y%i w%i h%i for blit %s",
       geo->x, geo->y, geo->w, geo->h, b->get_name());
  

  if(!screen) // return;
    screen = layer->freej->screen;

  // crop for the SDL blit
  if(b->type == SDL_BLIT) {
    b->sdl_rect.x = -(geo->x);
    b->sdl_rect.y = -(geo->y);
    b->sdl_rect.w = screen->w;
    b->sdl_rect.h = screen->h;

    // crop for the linear and past blit
  } else if(b->type == LINEAR_BLIT 
	    || b->type == PAST_BLIT) {

    b->lay_pitch = geo->w; // how many pixels to copy each row
    b->lay_height = geo->h; // how many rows we should copy
    
    b->scr_stride_up = 0; // rows to jump before starting to blit on screen
    b->scr_stride_sx = 0; // screen pixels stride on the left of each row
    b->scr_stride_dx = 0; // screen pixels stride on the right of each row
    
    b->lay_stride_up = 0; // rows to jump before starting to blit layer
    b->lay_stride_sx = 0; // how many pixels stride on the left of each row
    b->lay_stride_dx = 0; // how many pixels stride on the right of each row
    
    // BOTTOM
    if( geo->y+geo->h > screen->h ) {
      if(geo->y > screen->h) { // out of screen
	geo->y = screen->h+1; // don't go far
	layer->hidden = true;
	return;
      } else { // partially out
	b->lay_height -= (geo->y + geo->h) - screen->h;
      }
    }
    
    // LEFT
    if( geo->x < 0 ) {
      if( geo->x + geo->w < 0 ) { // out of screen
	geo->x = -( geo->w + 1 ); // don't go far
	layer->hidden = true;
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
	layer->hidden = true;
	return;
      } else { // partially out
	b->lay_stride_up += -geo->y;
	b->lay_height -= -geo->y;
      }
    } else { // inside
      b->scr_stride_up += geo->y;
    }
    
    // RIGHT
    if( geo->x + geo->w > screen->w ) {
      if( geo->x > screen->w ) { // out of screen
	geo->x = screen->w + 1; // don't go far
	layer->hidden = true;
	return;
      } else { // partially out
	b->lay_pitch -= ( geo->x + geo->w ) - screen->w;
	b->lay_stride_dx += ( geo->x + geo->w ) - screen->w;
      } 
    } else { // inside
      b->scr_stride_dx += screen->w - (geo->x + geo->w );
    }
    
    layer->hidden = false;
    
    b->lay_stride = b->lay_stride_dx + b->lay_stride_sx; // sum strides
    // precalculate upper left starting offset for layer
    b->lay_offset = (b->lay_stride_sx +
		     ( b->lay_stride_up * geo->w ));
    
    b->scr_stride = b->scr_stride_dx + b->scr_stride_sx; // sum strides
    // precalculate upper left starting offset for screen
    b->scr_offset = (b->scr_stride_sx +
		     ( b->scr_stride_up * screen->w ));
  }
  
  // calculate bytes per row
  b->lay_bytepitch = b->lay_pitch * (geo->bpp>>3);

  /* store values for further crop checking */
  old_x = geo->x;
  old_y = geo->y;
  old_w = geo->w;
  old_h = geo->h;
  
}  

