/*  FreeJ
 *  (c) Copyright 2001-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <layer.h>
#include <jutils.h>
#include <config.h>

#ifdef WITH_GTK2
#include <gtk/gtk.h>
#endif

#include <impl_layers.h>
#include <context.h>

const char *layers_description =
" .  - ImageLayer for image files (png, jpeg etc.)\n"
" .  - GeometryLayer for scripted vectorial primitives\n"
#ifdef WITH_V4L
" .  - Video4Linux devices as of BTTV cards and webcams\n"
" .    you can specify the size  /dev/video0%160x120\n"
#endif
#ifdef WITH_FFMPEG
" .  - MovieLayer for movie files, urls and firewire devices\n"
#endif
#if defined WITH_FT2 && defined WITH_FC
" .  - TextLayer for text rendered with freetype2 library\n"
#endif
#ifdef WITH_FLASH
" .  - FlashLayer for SWF flash v.3 animations\n"
#endif
#ifdef WITH_AUDIO
" .  - Audio Layer\n" // XXX
#endif
"\n";


Layer *create_layer(Context *env, char *file) {
  char *end_file_ptr,*file_ptr;
  FILE *tmp;
  Layer *nlayer = NULL;

  warning("create_layer is deprecated! use Context::open instead");

  /* check that file exists */
  if(strncasecmp(file,"/dev/",5)!=0
     && strncasecmp(file,"http://",7)!=0
     && strncasecmp(file,"ftp://",6)!=0
     && strncasecmp(file,"layer_",6)!=0) {
    tmp = fopen(file,"r");
    if(!tmp) {
      error("can't open %s to create a Layer: %s",
	    file,strerror(errno));
      return NULL;
    } else fclose(tmp);
  }
  /* check file type, add here new layer types */
  end_file_ptr = file_ptr = file;
  end_file_ptr += strlen(file);
//  while(*end_file_ptr!='\0' && *end_file_ptr!='\n') end_file_ptr++; *end_file_ptr='\0';

  /* ==== Unified caputure API (V4L & V4L2) */
  if( strncasecmp ( file_ptr,"/dev/video",10)==0) {
#ifdef WITH_UNICAP
    unsigned int w=env->screen->w, h=env->screen->h;
    while(end_file_ptr!=file_ptr) {
      if(*end_file_ptr!='%') end_file_ptr--;
      else { /* size is specified */
        *end_file_ptr='\0'; end_file_ptr++;
        sscanf(end_file_ptr,"%ux%u",&w,&h);
        end_file_ptr = file_ptr; 
      }
    }
    nlayer = new UnicapLayer();
    if(! ((UnicapLayer*)nlayer)->init( env, (int)w, (int)h) ) {
      error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
      delete nlayer; return NULL;
    }
    if(nlayer->open(file_ptr)) {
      notice("video camera source opened");
    //  ((V4lGrabber*)nlayer)->init_width = w;
    //  ((V4lGrabber*)nlayer)->init_heigth = h;
    } else {
      error("create_layer : V4L open failed");
      delete nlayer; nlayer = NULL;
    }
#else
    error("Video4Linux layer support not compiled");
    act("can't load %s",file_ptr);
#endif

  } else /* VIDEO LAYER */

    if( ((IS_VIDEO_EXTENSION(end_file_ptr)) | (IS_FIREWIRE_DEVICE(file_ptr))) ) {
      func("is a movie layer");

      // // MLT experiments
      //       nlayer = new MovieLayer();
      //       func("MovieLayer instantiated");
      //       if(!nlayer->init(env)) {
      //  	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
      //  	delete nlayer; return NULL;
      //       }
      //       func("MovieLayer initialized");
      //       if(!nlayer->open(file_ptr)) {
      //  	error("create_layer : VIDEO open failed");
      //  	delete nlayer; nlayer = NULL;
      //       }


#ifdef WITH_FFMPEG
       nlayer = new VideoLayer();
       if(!nlayer->init( env )) {
 	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
 	delete nlayer; return NULL;
       }
       if(!nlayer->open(file_ptr)) {
 	error("create_layer : VIDEO open failed");
 	delete nlayer; nlayer = NULL;
       }
// #elif WITH_AVIFILE
//       if( strncasecmp(file_ptr,"/dev/ieee1394/",14)==0)
// 	  nlayer=NULL;
//       nlayer = new AviLayer();
//       if(!nlayer->init( env )) {
// 	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
// 	delete nlayer; return NULL;
//       }
//       if(!nlayer->open(file_ptr)) {
// 	error("create_layer : AVI open failed");
// 	delete nlayer; nlayer = NULL;
//       }
 #else
      error("VIDEO and AVI layer support not compiled");
      act("can't load %s",file_ptr);
#endif
// } else /* Audio LAYER */
//     if(strncasecmp(file_ptr,"/tmp/test",9)==0) {
//       nlayer = new AudioLayer();
//       if(!nlayer->init( env )) {
// 	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
// 	delete nlayer; return NULL;
//       }
//       if(!nlayer->open(file_ptr)) {
// 	  error("create_layer : Audio open failed");
// 	  delete nlayer; nlayer = NULL;
//       }
  } else /* IMAGE LAYER */
      if( (IS_IMAGE_EXTENSION(end_file_ptr))) {
//		strncasecmp((end_file_ptr-4),".png",4)==0) 
	      nlayer = new ImageLayer();
              if(!nlayer->init( env )) {
                error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
                delete nlayer; return NULL;
              }
	      if(!nlayer->open(file_ptr)) {
		  error("create_layer : IMG open failed");
		  delete nlayer; nlayer = NULL;
	      }
  } else /* TXT LAYER */
    if(strncasecmp((end_file_ptr-4),".txt",4)==0) {
#if defined WITH_FT2 && defined WITH_FC
	  nlayer = new TextLayer();

      if(!nlayer->init( env )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }

	  if(!nlayer->open(file_ptr)) {
	    error("create_layer : TXT open failed");
	    delete nlayer; nlayer = NULL;
	  }
#else
	  error("TXT layer support not compiled");
	  act("can't load %s",file_ptr);
	  return(NULL);
#endif

  } else /* XHACKS LAYER */
    if(strstr(file_ptr,"xscreensaver")) {
#ifdef WITH_XHACKS
	    nlayer = new XHacksLayer();

      if(!nlayer->init( env )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }

	    if (!nlayer->open(file_ptr)) {
	      error("create_layer : XHACK open failed");
	      delete nlayer; nlayer = NULL;
	    }
#else
	    error("no xhacks layer support");
	    act("can't load %s",file_ptr);
	    return(NULL);
#endif
	  }  else if(strncasecmp(file_ptr,"layer_goom",10)==0) {

#ifdef WITH_GOOM
            nlayer = new GoomLayer();

      if(!nlayer->init( env )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }
#else
      error("goom layer not supported");
      return(NULL);
#endif


  }
#ifdef WITH_FLASH
  else if(strncasecmp(end_file_ptr-4,".swf",4)==0) {

	    nlayer = new FlashLayer();
      if(!nlayer->init( env )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }

	    if(!nlayer->open(file_ptr)) {
	      error("create_layer : SWF open failed");
	      delete nlayer; nlayer = NULL;
	    }

  }
#endif
//   else { /* FALLBACK TO SCROLL LAYER */

//     func("opening scroll layer on generic file type for %s",file_ptr);
//     nlayer = new ScrollLayer();
    
//     if(!nlayer->init( env )) {
//       error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
//       delete nlayer; return NULL;
//     }
       
//        if(!nlayer->open(file_ptr)) {
// 	 error("create_layer : SCROLL open failed");
// 	 delete nlayer; nlayer = NULL;
//        }
       
//   }

  if(!nlayer)
    error("can't create a layer with %s",file);
  else
    func("create_layer succesful, returns %p",nlayer);
  return nlayer;
}

