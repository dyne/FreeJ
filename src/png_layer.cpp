/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifdef WITH_PNG

#include <iostream>
#include <errno.h>
#include <png_layer.h>
#include <context.h>
#include <jutils.h>


#define PNG_BYTES_TO_CHECK 4

PngLayer::PngLayer()
  :Layer() {
  core = NULL;
  info = NULL;
  row_pointers = NULL;
  fp = NULL;
  subliminal=0;
  blinking=false;
  count=0;
  set_name("PNG");
}

PngLayer::~PngLayer() {
  close();
}

bool PngLayer::open(char *file) {
  func("PngLayer::open(%s)",file);

  fp = fopen(file,"rb");
  if(!fp) {
    error("Pnglayer::open(%s) - %s",file,strerror(errno));
    return (false); }

  fread(sig,1,PNG_BYTES_TO_CHECK,fp);
  if(png_sig_cmp(sig,(png_size_t)0,PNG_BYTES_TO_CHECK)) {
    error("Pnglayer::open(%s) - not a valid png file",file);
    fclose(fp); fp = NULL;
    return (false); }

  set_filename(file);

  return(true);
}

bool PngLayer::init(Context *scr) {

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;

  /** init image pointers */
  black_image=NULL;
  png_image=NULL;

  if(!fp) {
    error("no png file opened, layer skipped");
    return false;
  }

  /* create png structures */

  core = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
  if (!core) {
    error("can't create PNG core");
    return(false);
  }
  
  info = png_create_info_struct(core);
  if (!info) {
    error("can't gather PNG info");
    png_destroy_read_struct
      (&core, (png_infopp)NULL, (png_infopp)NULL);
    return (false);
  }

  /* initialize error message callback */

  if ( setjmp(core->jmpbuf) )
    error("error reading the PNG file.");

  /* start peeking into the file */

  png_init_io(core,fp);

  png_set_sig_bytes(core,PNG_BYTES_TO_CHECK);  

  png_read_info(core,info);

  png_get_IHDR(core, info, &width, &height, &bit_depth, &color_type,
	       &interlace_type, NULL, NULL);

  /* tell libpng to strip 16 bit/color files down to 8 bits/color */
  png_set_strip_16(core) ;

  /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
     byte into separate bytes (useful for paletted and grayscale images). */
  png_set_packing(core);

  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    func("PNG set palette to rgb");
    png_set_palette_to_rgb(core);
  }
  
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
    func("PNG set gray to 8bpp");
    png_set_gray_1_2_4_to_8(core);
  }

  png_set_filler(core, 0xff, PNG_FILLER_AFTER);
  
  png_set_tRNS_to_alpha(core);
  
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    func("PNG set gray to rgb");
    png_set_gray_to_rgb(core);
  }


  /* we don't want background to keep transparence
  png_color_16 bg;
  png_color_16p image_bg;
  if (png_get_bKGD(core, info, &image_bg)) {
    func("PNG set background on file gamma");
    png_set_background(core, image_bg, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
  } else {
    func("PNG set background on screen gamma");
    png_set_background(core, &bg, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
  } */


  if (color_type == PNG_COLOR_TYPE_RGB ||
      color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    png_set_bgr(core);
  
  png_set_interlace_handling(core);
  
  png_read_update_info(core,info);                 
  
  png_get_IHDR(core, info, &width, &height, &bit_depth, &color_type,
	       &interlace_type, NULL, NULL);
 
  if(scr) freej = scr;
  _init(freej, width, height, bit_depth*4);

  png_image = (png_bytep)jalloc(png_image,geo.size);

  /** allocate memory for the black image */
  black_image = jalloc(black_image,geo.size);

  black_image = memset(black_image,0,geo.size);

  /* allocate image memory */
  row_pointers = (png_bytep*)jalloc(row_pointers,geo.h*sizeof(png_bytep));
  for(int i=0;i<geo.h;i++) 
    row_pointers[i] = (png_bytep) png_image + i*geo.pitch;

  png_read_image(core,row_pointers);

  png_read_end(core,NULL);

  fclose(fp); fp = NULL;
  png_destroy_info_struct(core,&info);
  png_destroy_read_struct(&core,NULL,NULL);
  
  notice("PngLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	 geo.w,geo.h,geo.bpp,geo.size);

  return(true);
}

void *PngLayer::feed() {
  count++;
  // threads problems !! XXX TODO 
  if(count==freej->fps_speed)
    count=0;
  if(blinking && count%2!=0) {
    return black_image;
  }
  // subliminal mode
  if(subliminal!=0 && count>= subliminal)
    return black_image;
  
  return png_image;
}

void PngLayer::close() {
  func("PngLayer::close()");
  jfree(row_pointers);
  jfree(buffer);
}

bool PngLayer::keypress(char key) {
  bool res = true;

  switch(key) {
  case 'o':
    subliminal++;
    act("PngLayer::subliminal count %i",subliminal);
    break;
  case 'p':
    subliminal=0;
    act("PngLayer::subliminal reset");
    break;
  case 'b':
    if(blinking)
      blinking=false;
    else 
      blinking=true;
    act("PngLayer::blinking %s",(blinking)?"ON":"OFF");
    break;
    
  default:
    res = false;
    break;
  }
  return res;
}

#endif
