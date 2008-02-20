/*  FreeJ
 *  (c) Copyright 2001-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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

#ifdef WITH_V4L

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include <ccvt.h>
#include <v4l_layer.h>
#include <tvfreq.h>
#include <jutils.h>

#include <jsparser_data.h>

V4lGrabber::V4lGrabber() 
  :Layer() {
  dev = -1;
  rgb_surface = NULL;
  buffer = NULL;
  have_tuner=false;
  set_name("V4L");
  jsclass = &v4l_layer_class;
}

V4lGrabber::~V4lGrabber() {
  func("%s %s", __FILE__, __FUNCTION__);
  close();
}

void V4lGrabber::close() {
  func("%s %s", __FILE__, __FUNCTION__);
  if(dev>0) notice("Closing video4linux grabber layer");
 
  if(buffer!=NULL) {
    act("unmapping address %p sized %u bytes",buffer,grab_map.size);
    munmap(buffer,grab_map.size);
  }

  if(dev>0) {
    act("closing video4linux device %u",dev);
    ::close(dev);
  }
  
  if(rgb_surface) jfree(rgb_surface);
  
}

bool V4lGrabber::open(char *file) {
  int counter, res;
  char *capabilities[] = {
    "VID_TYPE_CAPTURE          can capture to memory",
    "VID_TYPE_TUNER            has a tuner of some form",
    "VID_TYPE_TELETEXT         has teletext capability",
    "VID_TYPE_OVERLAY          can overlay its image to video",
    "VID_TYPE_CHROMAKEY        overlay is chromakeyed",
    "VID_TYPE_CLIPPING         overlay clipping supported",
    "VID_TYPE_FRAMERAM         overlay overwrites video memory",
    "VID_TYPE_SCALES           supports image scaling",
    "VID_TYPE_MONOCHROME       image capture is grey scale only",
    "VID_TYPE_SUBCAPTURE       capture can be of only part of the image"
  };

  func("%s %s detect()", __FILE__, __FUNCTION__);

  if (-1 == (dev = ::open(file,O_RDWR|O_NONBLOCK))) {
    error("open capture device %s: %s",file,strerror(errno));
    return(false);
  } else {
    ::close(dev);
    dev = ::open(file,O_RDWR);
  }
  
  res = ioctl(dev,VIDIOCGCAP,&grab_cap);
  if(res<0) {
    error("error in VIDIOCGCAP ");
    return(false);
  }

  if(get_debug()>0) {
    
    notice("Device detected is %s",file);
    act("%s",grab_cap.name);
    act("%u channels detected",grab_cap.channels);
    act("max size w[%u] h[%u] - min size w[%u] h[%u]",grab_cap.maxwidth,grab_cap.maxheight,grab_cap.minwidth,grab_cap.minheight);
    act("Video capabilities:");
    for (counter=0;counter<11;counter++)
      if (grab_cap.type & (1 << counter)) act("%s",capabilities[counter]);
  }    
  
  if(grab_cap.type & VID_TYPE_TUNER)
    /* if the device does'nt has any tuner, so we avoid some ioctl
       this should be a fix for many webcams, thanks to Ben Wilson */
    have_tuner = 1;
  
  /* set and check the minwidth and minheight */
  if( (geo.w<grab_cap.minwidth) || (geo.w>grab_cap.maxwidth) || (geo.h<grab_cap.minheight) || (geo.h>grab_cap.maxheight) ) {
    error("your device doesn't supports grabbing size %ix%i", geo.w, geo.h);
    return(false);
  }

if (-1 == ioctl(dev, VIDIOCGPICT, &grab_pic)) {
  error("ioctl VIDIOCGPICT ");
  exit(1);
}
#define TRY_VIDEO_PALETTE(pal) \
  grab_pic.palette = pal; \
  res=ioctl(dev,VIDIOCSPICT,&grab_pic); \
  if ( res < 0 ) \
    func("v4l: palette     %s(0x%08x) not supported for grabbing, res: %i got instead: %u", ""#pal, pal, res, grab_pic.palette); \
  else { \
    palette = grab_pic.palette; \
    func("v4l: palette ok: %s(0x%08x) res: %i palette: %u bpp: %u", ""#pal, pal, res, palette, grab_pic.depth); \
    }

  palette = 0;
  func("v4l: probing color formats");
  TRY_VIDEO_PALETTE(VIDEO_PALETTE_RGB32)
  if(palette == 0) {
    TRY_VIDEO_PALETTE(VIDEO_PALETTE_RGB24) }
  if(palette == 0) {
    TRY_VIDEO_PALETTE(VIDEO_PALETTE_YUV422P) }
  if(palette == 0) {
    TRY_VIDEO_PALETTE(VIDEO_PALETTE_YUV420P) }
  if(palette == 0) {
    TRY_VIDEO_PALETTE(VIDEO_PALETTE_YUYV) }
//if(palette == 0) {
//  TRY_VIDEO_PALETTE(VIDEO_PALETTE_UYVY) }
  if(palette == 0) {
    error("device %s doesn't supports grabbing any desired palette", file);
    return(false);
  }
  func("v4l: probing for size");
  grab_buf[0].format = palette;
  grab_buf[0].frame  = 0;
  grab_buf[0].height = geo.h;
  grab_buf[0].width = geo.w;
  res=ioctl(dev,VIDIOCMCAPTURE,&grab_buf[0]);
  if ( res<0 ) {
    error("v4l: size %ix%i not supported res: %i", geo.w, geo.h, res);
    return false;
  }
  //res = ioctl(dev,VIDIOCSYNC,&grab_buf[0]);
  //func("SYNC: %i", res);
  //XX_TRY_VIDEO_PALETTE(palette);
errno=0;
  res = ioctl (dev, VIDIOCGMBUF, &grab_map);
func("v4l: memory map of %i frames: %u bytes",grab_map.frames,grab_map.size);
  if (res < 0) {
    error("error in ioctl VIDIOCGMBUF: (%i)", res);
    error("ERR %d %s dev: %i", errno, strerror(errno), dev);
    return(false);
  }
  /* print memory info */
  if(get_debug()>0) {
    act("memory map of %i frames: %i bytes",grab_map.frames,grab_map.size);
    for(counter=0;counter<grab_map.frames;counter++)
      act("Offset of frame %i: %i",counter,grab_map.offsets[counter]);
  }
  num_frame = grab_map.frames;
  channels = grab_cap.channels;
  set_filename(file);

  /* set image source and TV norm */
  grab_chan.channel = input = (channels>1) ? 1 : 0;
  
  if(have_tuner) { /* does this only if the device has a tuner */
    _band = 5; /* default band is europe west */
    _freq = 0;
    /* resets CHAN */
    if (-1 == ioctl(dev,VIDIOCGCHAN,&grab_chan)) {
      error("error in ioctl VIDIOCGCHAN ");
      return(false);
    }
    
    if (-1 == ioctl(dev,VIDIOCSCHAN,&grab_chan)) {
      error("error in ioctl VIDIOCSCHAN ");
      return(false);
    }

    /* get/set TUNER settings */
    if (-1 == ioctl(dev,VIDIOCGTUNER,&grab_tuner)) {
      error("error in ioctl VIDIOCGTUNER ");
      return(false);
    }
  }


  /* choose best yuv2rgb routine (detecting cpu)
     supported: C, ASM-MMX, ASM-MMX+SSE */
  /*  yuv2rgb = yuv2rgb_init(geo.bpp,0x1); arg2 is MODE_RGB */


  //  u = (geo.w*geo.h);
  //  v = u+(u/2);//u+(u/2);
  /* mmap (POSIX.4) buffer for grabber device */
  buffer = (unsigned char *) mmap(0,grab_map.size,PROT_READ|PROT_WRITE,MAP_SHARED,dev,0);
  if(MAP_FAILED == buffer) {
    error("cannot allocate v4lgrabber buffer %d %s dev: %i", errno, strerror(errno), dev);
    return(false);
  }


  /*
  //  palette = VIDEO_PALETTE_RGB32; // good is YUV422P;
  grab_buf[0].format = palette;
  grab_buf[0].frame  = 0;
  grab_buf[0].height = geo.h;
  grab_buf[0].width = geo.w;
  // feed up the mmapped frame
  if (-1 == ioctl(dev,VIDIOCMCAPTURE,&grab_buf[0])) {
    func("v4l::init : palette test on CMCAPTURE with YUV422P failed");
    func("try to grab format YUV420P");
    palette = VIDEO_PALETTE_YUV420P;
    grab_buf[0].format = palette;
    grab_buf[0].frame  = 0;
    grab_buf[0].height = geo.h;
    grab_buf[0].width = geo.w;
    if (-1 == ioctl(dev,VIDIOCMCAPTURE,&grab_buf[0])) {
      error("no valid palette supported by V4L device %s",get_filename());
      munmap(buffer,grab_map.size);
      return false;
    }
  }
  */

  /* initialize frames geometry */						      
  int i;
  for(i=0; i<grab_map.frames; i++) {

    grab_buf[i].format = palette;
    grab_buf[i].frame  = i;
    grab_buf[i].height = geo.h;
    grab_buf[i].width = geo.w;
  }

  rgb_surface = malloc(geo.size);

  cur_frame = ok_frame = 0;

  func("V4L layer :: w[%u] h[%u] bpp[%u] size[%u] grab_mmap[%u]",
	 geo.w,geo.h,geo.bpp,geo.size,geo.size*num_frame);
  if(grab_cap.channels>1)
    act("using input channel %s",grab_chan.name);

  return(true);
}

bool V4lGrabber::init(Context *env, int width, int height) {
    func("%s %s", __FILE__, __FUNCTION__);
    _init(width,height);
    return true;
}

bool V4lGrabber::init(Context *env) {
  func("%s %s", __FILE__, __FUNCTION__);
  return init(env, env->screen->w, env->screen->h);
}

void V4lGrabber::set_chan(int ch) {

  grab_chan.channel = input = ch;

  if (-1 == ioctl(dev,VIDIOCGCHAN,&grab_chan))
    error("error in ioctl VIDIOCGCHAN ");

  grab_chan.norm = VIDEO_MODE_PAL;

  if (-1 == ioctl(dev,VIDIOCSCHAN,&grab_chan))
    error("error in ioctl VIDIOCSCHAN ");
  
  act("V4L: input chan %u %s",ch,grab_chan.name);
  show_osd();
}

void V4lGrabber::set_band(int b) {
  _band = b;
  chanlist = chanlists[b].list;
  if(_freq>chanlists[b].count) _freq = chanlists[b].count;
  act("V4L: frequency table %u %s [%u]",b,chanlists[b].name,chanlists[b].count);
  show_osd();
}

void V4lGrabber::set_freq(int f) {
  _freq = f;

  unsigned long frequency = chanlist[f].freq*16/1000;
  float ffreq = (float) frequency/16;

  func("V4L: set frequency %u %.3f",frequency,ffreq);

  //  lock_feed();
  if (-1 == ioctl(dev,VIDIOCSFREQ,&frequency))
    error("error in ioctl VIDIOCSFREQ ");
  //  unlock_feed();
  act("V4L: frequency %s %.3f Mhz (%s)",chanlist[f].name,ffreq,chanlists[_band].name);
  show_osd();
}
  

/* here are defined the keys for this layer */
bool V4lGrabber::keypress(int key) {
  int res = 1;

  switch(key) {
  case 'k':
    if(input<channels)
      set_chan(input+1);
    break;
    
  case 'm':
    if(input>0)
      set_chan(input-1);
    break;
    
    if(have_tuner) {
    case 'j':
      if(_band<bandcount)
	set_band(_band+1);
      break;
      
    case 'n':
      if(_band>0)
	set_band(_band-1);
      break;
      
    case 'h':
      if(_freq<chanlists[_band].count)
	set_freq(_freq+1);
      else
	set_freq(0);
      break;

    case 'b':
      if(_freq>0)
	set_freq(_freq-1);
      else
	set_freq(chanlists[_band].count);
      break;
      
    } /* if (have_tuner) */
    
  default:
    res = 0;
  }
  return res;
}

void *V4lGrabber::get_buffer() {
  return(rgb_surface);
}

void *V4lGrabber::feed() {

  ok_frame = cur_frame;
  
  cur_frame = (cur_frame>=num_frame) ? 0 : cur_frame++;
  // cur_frame = ((cur_frame+1)%num_frame); 10x luka@ljudmila

  grab_buf[ok_frame].format = palette;
  if (-1 == ioctl(dev,VIDIOCSYNC,&grab_buf[ok_frame])) {
    func("V4lGrabber::feed");
    error("error in ioctl VIDIOCSYNC on buffer %i/%i (%p)",
	  ok_frame,num_frame,&grab_buf[ok_frame]);
    return(NULL);
  }

  grab_buf[cur_frame].format = palette;
  if (-1 == ioctl(dev,VIDIOCMCAPTURE,&grab_buf[cur_frame])) {
    func("V4lGrabber::feed");
    error("error in ioctl VIDIOCMCAPTURE on buffer %i/%i (%p)",
	  cur_frame,num_frame,&grab_buf[cur_frame]);
  }

  /*
  (*yuv2rgb)((uint8_t *) rgb_surface,
	     (uint8_t *) &buffer[grab_map.offsets[ok_frame]],
	     (uint8_t *) &buffer[grab_map.offsets[ok_frame]+u],
	     (uint8_t *) &buffer[grab_map.offsets[ok_frame]+v],
	     geo.w, geo.h, geo.pitch, geo.w, geo.w);  
  */
  //  if(palette == VIDEO_PALETTE_YUV422P)
#if defined HAVE_MMX && !defined HAVE_64BIT
  if(palette == VIDEO_PALETTE_YUV422P
     || palette == VIDEO_PALETTE_YUYV)
    ccvt_yuyv_rgb32(geo.w, geo.h, &buffer[grab_map.offsets[ok_frame]], rgb_surface);

  else
#endif
  if(palette == VIDEO_PALETTE_YUV420P) 
    ccvt_420p_rgb32(geo.w, geo.h, &buffer[grab_map.offsets[ok_frame]], rgb_surface);

  else if(palette == VIDEO_PALETTE_RGB32) 
    memcpy(rgb_surface,&buffer[grab_map.offsets[ok_frame]],geo.size);

  else if(palette == VIDEO_PALETTE_RGB24) 
    ccvt_rgb24_rgb32(geo.w, geo.h, &buffer[grab_map.offsets[ok_frame]], rgb_surface);

  else
    error("video palette %i for layer %s %s not supported",
	  palette, get_name(),get_filename());

  return rgb_surface;
}

#endif
