/*  FreeJ
 *  (c) Copyright 2001-2003 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <impl_layers.h>

const char *layers_description =
#ifdef WITH_V4L
" .  - Video4Linux devices as of BTTV cards and webcams\n"
" .    you can specify the size  /dev/video0%160x120\n"
#endif
#ifdef WITH_AVIFILE
" .  - AVI,ASF,WMA,WMV movies as of codecs supported by avifile lib\n"
#endif
#ifdef WITH_AVCODEC
" .  - AVI,ASF,WMA,WMV,MPEG local and remote (http://localhost/file.mpg), dv1394 firewire devices\n"
#endif
#ifdef WITH_PNG
" .  - PNG images (also with transparency)\n"
#endif
#ifdef WITH_FT2
" .  - TXT files rendered with freetype2 library\n"
#endif
#ifdef WITH_XHACKS
" .  - xscreensaver screen hack. ex. /usr/X11R6/lib/xscreensaver/cynosure\n"
#endif
#ifdef WITH_FLASH
" .  - SWF flash v.3 layer for vectorial graphics animations\n"
#endif
" .  - particle generator ( try: 'freej layer_gen' on commandline)\n"
" .  - vertical text scroller (any other extension)\n"
"\n";


Layer *create_layer(char *file) {
  char *end_file_ptr,*file_ptr;
  FILE *tmp;
  Layer *nlayer = NULL;
  
  /* check that file exists */
  if(strncasecmp(file,"/dev/",5)!=0 
     && strncasecmp(file,"http://",7)!=0
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


  /* ==== Video4Linux */
  if(strncasecmp(file_ptr,"/dev/",5)==0 && ! IS_FIREWIRE_DEVICE(file_ptr)) {
#ifdef WITH_V4L
    unsigned int w=320, h=240;
    while(end_file_ptr!=file_ptr) {
      if(*end_file_ptr!='%') end_file_ptr--;
      else { /* size is specified */
	*end_file_ptr='\0'; end_file_ptr++;
	sscanf(end_file_ptr,"%ux%u",&w,&h);
	end_file_ptr = file_ptr; }
    }
    nlayer = new V4lGrabber();
    if(nlayer->open(file_ptr)) {
      ((V4lGrabber*)nlayer)->init_width = w;
      ((V4lGrabber*)nlayer)->init_heigth = h;
    } else {
      error("create_layer : V4L open failed");
      delete nlayer; nlayer = NULL;
    }
#else
    error("Video4Linux layer support not compiled");
    act("can't load %s",file_ptr);
#endif

  } else /* AVI LAYER */
    if( IS_VIDEO_EXTENSION(end_file_ptr)
	    | strncasecmp(end_file_ptr-4,".jpg",4)==0 
//	    | strncasecmp(end_file_ptr-4,".gif",4)==0  // it does not handle loops :''(
        | IS_FIREWIRE_DEVICE(file_ptr)) {
#ifdef WITH_AVCODEC 
      nlayer = new VideoLayer();
      if(!nlayer->open(file_ptr)) {
	error("create_layer : VIDEO open failed");
	delete nlayer; nlayer = NULL;
      }
#elif WITH_AVIFILE 
      if( strncasecmp(file_ptr,"/dev/ieee1394/",14)==0) 
	  nlayer=NULL;
      nlayer = new AviLayer();
      if(!nlayer->open(file_ptr)) {
	error("create_layer : AVI open failed");
	delete nlayer; nlayer = NULL;
      }
#else
      error("VIDEO and AVI layer support not compiled");
      act("can't load %s",file_ptr);
#endif
    } else /* PNG LAYER */
      if(strncasecmp((end_file_ptr-4),".png",4)==0) {
#ifdef WITH_PNG
	nlayer = new PngLayer();
	if(!nlayer->open(file_ptr)) {
	  error("create_layer : PNG open failed");
	  delete nlayer; nlayer = NULL;
	}
#else
	error("PNG layer support not compiled");
	act("can't load %s",file_ptr);
#endif
	
      } else /* TXT LAYER */
	if(strncasecmp((end_file_ptr-4),".txt",4)==0) {
#ifdef WITH_FT2
	  nlayer = new TxtLayer();
	  if(!nlayer->open(file_ptr)) {
	    error("create_layer : TXT open failed");
	    delete nlayer; nlayer = NULL;
	  }
#else
	  error("TXT layer support not compiled");
	  act("can't load %s",file_ptr);
	  return(NULL);
#endif
	} else
	  if(strstr(file_ptr,"xscreensaver")) { /* XHACKS_LAYER */
#ifdef WITH_XHACKS
	    nlayer = new XHacksLayer();
	    if (!nlayer->open(file_ptr)) {
	      error("create_layer : XHACK open failed");
	      delete nlayer; nlayer = NULL;
	    }
#else
	    error("no xhacks layer support");
	    act("can't load %s",file_ptr);
	    return(NULL);
#endif	  
	} else
	  if(strncasecmp(file_ptr,"layer_gen",9)==0) {
	    nlayer = new GenLayer();
	  } else
#ifdef WITH_FLASH
	    if(strncasecmp(end_file_ptr-4,".swf",4)==0) {
	      nlayer = new FlashLayer();
	      if(!nlayer->open(file_ptr)) {
		error("create_layer : SWF open failed");
		delete nlayer; nlayer = NULL;
	      }
#else
	      error("no flash layer support");
	      act("can't load %s",pp);
	      return(NULL);
#endif
	    } else {
	      nlayer = new ScrollLayer();
	      if(!nlayer->open(file_ptr)) {
		error("create_layer : SCROLL open failed");
		delete nlayer; nlayer = NULL;
	      }
	      
	    }

  if(!nlayer)
    error("can't create a layer with %s",file);
  else
    func("create_layer succesful, returns %p",nlayer);
  return nlayer;
}
