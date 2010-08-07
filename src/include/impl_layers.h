/*  FreeJ
 *  (c) Copyright 2001-2009 Denis Roio <jaromil@dyne.org>
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

#ifndef __IMPL_LAYERS_H__
#define __IMPL_LAYERS_H__

#include <config.h>

// controllers
#include <kbd_ctrl.h>

/* software layers which don't need special loaders */
#include <generator_layer.h>
#include <geo_layer.h>
#include <image_layer.h> // statically included sdl_image
#include <flash_layer.h> // statically included flash layer
#include <goom_layer.h>

#ifdef WITH_V4L
#include <v4l2_layer.h>
#endif

#ifdef WITH_AVIFILE
#include <avi_layer.h>
#endif

#ifdef WITH_FFMPEG
#include <video_layer.h>
#endif

#if defined WITH_TEXTLAYER
#include <text_layer.h>
#endif

#ifdef WITH_XSCREENSAVER
#include <xscreensaver_layer.h>
#endif

#ifdef WITH_UNICAP
#include <unicap_layer.h>
#endif

#ifdef WITH_OPENCV
#include <opencv_cam_layer.h>
#endif

#define IS_VIDEO_EXTENSION(end_file_ptr)                \
  ( strncasecmp((end_file_ptr-4),".avi",4)==0 )       	\
  | ( strncasecmp((end_file_ptr-4),".asf",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".asx",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".wma",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".mov",4)==0	)	\
  | ( strncasecmp((end_file_ptr-5),".mpeg",5)==0 )      \
  | ( strncasecmp((end_file_ptr-4),".mpg",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".mp4",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".ogg",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".ogv",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".ogm",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".gif",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".3gp",4)==0	)	\
  | ( strncasecmp((end_file_ptr-4),".flv",4)==0 )


//	    | strncasecmp(end_file_ptr-4,".gif",4)==0  // it does not handle loops :''(
//	    | strncasecmp(end_file_ptr-4,".jpg",4)==0

#define IS_FIREWIRE_DEVICE(file_ptr) \
    strncasecmp(file_ptr,"/dev/ieee1394/",14)==0

#define IS_IMAGE_EXTENSION(end_file_ptr) 		\
  ( strncasecmp((end_file_ptr-4),".bmp",4)==0 )		\
  | ( strncasecmp((end_file_ptr-4),".pnm",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".png",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".xpm",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".xcf",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".pcx",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".jpg",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-5),".jpeg",5)==0 )  	\
  | ( strncasecmp((end_file_ptr-4),".tif",4)==0 ) 	\
  | ( strncasecmp((end_file_ptr-4),".lbm",4)==0 )

#endif
