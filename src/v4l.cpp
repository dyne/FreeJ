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
 */

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <v4l.h>
#include <tvfreq.h>

V4lGrabber::V4lGrabber() {
  dev = -1;
  buffer = NULL;
  rgb_surface = NULL;
  have_tuner=false;
}

V4lGrabber::~V4lGrabber() {
  _close();
}

void V4lGrabber::_close() {
  notice("Closing video4linux grabber layer");

  if(dev>0) {
    act("closing video4linux device %u",dev);
    close(dev);
  }
  
  if(buffer!=NULL) {
    act("unmapping address %p sized %u bytes",buffer,grab_map.size);
    munmap(buffer,grab_map.size);
  }

  jfree(rgb_surface);
}

bool V4lGrabber::detect(char *devfile) {
  int counter, res;
  char *capabilities[] = {
    "VID_TYPE_CAPTURE          can capture to memory",
    "VID_TYPE_TUNER            has a tuner of some form",
    "VID_TYPE_TELETEXT         has teletext capability",
    "VID_TYPE_OVERLAY          can overlay its image onto the frame buffer",
    "VID_TYPE_CHROAKEY        overlay is chromakeyed",
    "VID_TYPE_CLIPPING         overlay clipping supported",
    "VID_TYPE_FRAMERAM         overlay overwrites frame buffer memory",
    "VID_TYPE_SCALES           supports image scaling",
    "VID_TYPE_MONOCHROME       image capture is grey scale only",
    "VID_TYPE_SUBCAPTURE       capture can be of only part of the image"
  };

  func("V4lGrabber::detect()");

  if (-1 == (dev = open(devfile,O_RDWR))) {
    error("error in opening video capture device");
    return(false);
  }
  
  res = ioctl(dev,VIDIOCGCAP,&grab_cap);
  if(res<0) {
    error("error in VIDIOCGCAP ");
    return(false);
  }

  if(get_debug()>0) {
    
    notice("Device detected is %s",devfile);
    act("%s",grab_cap.name);
    act("%u channels detected",grab_cap.channels);
    act("max size w[%u] h[%u] - min size w[%u] h[%u]",grab_cap.maxwidth,grab_cap.maxheight,grab_cap.minwidth,grab_cap.minheight);
    act("Video capabilities:");
    for (counter=0;counter<11;counter++)
      if (grab_cap.type & (1 << counter)) act("%s",capabilities[counter]);

    if (-1 == ioctl(dev, VIDIOCGPICT, &grab_pic)) {
      perror("ioctl VIDIOCGPICT ");
      exit(1);
    }
    
    if (grab_pic.palette & VIDEO_PALETTE_GREY)
      act("VIDEO_PALETTE_GREY        device is able to grab greyscale frames");
  }    
  
  if(grab_cap.type & VID_TYPE_TUNER)
    /* if the device does'nt has any tuner, so we avoid some ioctl
       this should be a fix for many webcams, thanks to Ben Wilson */
    have_tuner = 1;
  
  /* set the minwidth and minheight */
  minw = grab_cap.minwidth;
  minh = grab_cap.minheight;
  
  if (ioctl (dev, VIDIOCGMBUF, &grab_map) == -1) {
    error("error in ioctl VIDIOCGMBUF");
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
  return(true);
}

bool V4lGrabber::init(Context *screen,int wdt, int hgt, int chan_input) {
  int i;
  func("V4lGrabber::init()");

  /* set image source and TV norm */
  if(chan_input>channels) chan_input = 0;
  grab_chan.channel = input = chan_input;
  
  if(have_tuner) { /* does this only if the device has a tuner */
    _band = 5; /* default band is europe west */
    _freq = 0;
    /* resets CHAN */
    if (-1 == ioctl(dev,VIDIOCGCHAN,&grab_chan)) {
      error("error in ioctl VIDIOCGCHAN ");
      return(false);
    }
    
    /* here sets STATIC PAL MODE
    grab_chan.norm = VIDEO_MODE_PAL;
    */
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

  /*  
      if (-1 == ioctl(dev, VIDIOCGPICT, &grab_pic)) {
      error("error in ioctl VIDIOCGPICT ");
      return(false);
      }
  */

  /* TODO: check with minwidth maxwidth */

  w = (wdt == 0) ? screen->w : wdt;
  h = (hgt == 0) ? screen->h : hgt;
  bpp = screen->bpp;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);
  fps = &screen->fps;

  palette = VIDEO_PALETTE_YUV422P;
  /* choose best yuv2rgb routine (detecting cpu)
     supported: C, ASM-MMX, ASM-MMX+SSE */
  yuv2rgb = yuv2rgb_init(bpp,0x1); /* arg2 is MODE_RGB */
  rgb_surface = jalloc(rgb_surface,size);

  u = (w*h);
  v = u+(u/2);
  
  for(i=0; i<grab_map.frames; i++) {
    grab_buf[i].format = palette;
    grab_buf[i].frame  = i;
    grab_buf[i].height = h;
    grab_buf[i].width = w;
  }
  
  /* mmap (POSIX.4) buffer for grabber device */
  buffer = (unsigned char *) mmap(0,grab_map.size,PROT_READ|PROT_WRITE,MAP_SHARED,dev,0);
  if(MAP_FAILED == buffer) {
    error("cannot allocate v4lgrabber buffer ");
    return(false);
  }

  cur_frame = ok_frame = 0;  
  for(;cur_frame<num_frame;cur_frame++) {
    if (-1 == ioctl(dev,VIDIOCMCAPTURE,&grab_buf[cur_frame])) {
      func("V4lGrabber::feed");
      error("error in ioctl VIDIOCMCAPTURE");
    }
  }
  cur_frame = 0;

  feeded = true;

  /* INIT from the LAYER CLASS */
  _init(screen);

  notice("V4L layer :: w[%u] h[%u] bpp[%u] size[%u] grab_mmap[%u]",w,h,bpp,size,size*num_frame);
  act("using input channel %s",grab_chan.name);
  return(true);
}

void V4lGrabber::set_chan(int ch) {

  grab_chan.channel = input = ch;

  if (-1 == ioctl(dev,VIDIOCGCHAN,&grab_chan))
    error("error in ioctl VIDIOCGCHAN ");

  grab_chan.norm = VIDEO_MODE_PAL;

  if (-1 == ioctl(dev,VIDIOCSCHAN,&grab_chan))
    error("error in ioctl VIDIOCSCHAN ");
  
  screen->osd->status("V4L: input chan %u %s",ch,grab_chan.name);
}

void V4lGrabber::set_band(int b) {
  _band = b;
  chanlist = chanlists[b].list;
  if(_freq>chanlists[b].count) _freq = chanlists[b].count;
  screen->osd->status("V4L: frequency table %u %s [%u]",b,chanlists[b].name,chanlists[b].count);
}

void V4lGrabber::set_freq(int f) {
  _freq = f;

  unsigned long frequency = chanlist[f].freq*16/1000;
  float ffreq = (float) frequency/16;

  func("V4L: set frequency %u %.3f",frequency,ffreq);

  lock_feed();
  if (-1 == ioctl(dev,VIDIOCSFREQ,&frequency))
    error("error in ioctl VIDIOCSFREQ ");
  unlock_feed();
  screen->osd->status("V4L: frequency %s %.3f Mhz (%s)",chanlist[f].name,ffreq,chanlists[_band].name);
}
  

/* here are defined the keys for this layer */
bool V4lGrabber::keypress(SDL_keysym *keysym) {
  switch(keysym->sym) {

  case SDLK_k:
    if(input<channels) {
      set_chan(input+1);
      return(true);
    } else return(false);

  case SDLK_m:
    if(input>0) {
      set_chan(input-1);
      return(true);
    } else return(false);
    
    if(have_tuner) {
    case SDLK_j:
      if(_band<bandcount) {
	set_band(_band+1);
	return(true);
      } else return(false);
      
    case SDLK_n:
      if(_band>0) {
	set_band(_band-1);
	return(true);
      } else return(false);
      
    case SDLK_h:
      if(_freq<chanlists[_band].count) {
	set_freq(_freq+1);
	return(true);
      } else {
	set_freq(0);
	return(true);
      }
      
    case SDLK_b:
      if(_freq>0) {
	set_freq(_freq-1);
	return(true);
      } else {
	set_freq(chanlists[_band].count);
	return(true);
      }
    }

  default:
    return(false);
  }
}

void *V4lGrabber::get_buffer() {
  /*  return(&buffer[grab_map.offsets[ok_frame]]); */
  (*yuv2rgb)((uint8_t *) rgb_surface,
	     (uint8_t *) &buffer[grab_map.offsets[ok_frame]],
	     (uint8_t *) &buffer[grab_map.offsets[ok_frame]+u],
	     (uint8_t *) &buffer[grab_map.offsets[ok_frame]+v],
	     w, h, pitch, w, w);
  return(rgb_surface);
}

void V4lGrabber::feed() {
  ok_frame = cur_frame;
  cur_frame = ((cur_frame+1)%num_frame);
  grab_buf[0].format = palette;

  if (-1 == ioctl(dev,VIDIOCSYNC,&grab_buf[cur_frame])) {
    func("V4lGrabber::feed");
    error("error in ioctl VIDIOCSYNC");
  }

  if (-1 == ioctl(dev,VIDIOCMCAPTURE,&grab_buf[cur_frame])) {
    func("V4lGrabber::feed");
    error("error in ioctl VIDIOCMCAPTURE");
  }
}
