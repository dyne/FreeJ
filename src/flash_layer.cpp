/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
//  Author : Olivier Debon  <odebon@club-internet.fr>
//  Ported to FreeJ by Denis Rojo <jaromil@dyne.org>
//
// "$Id$"

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include <flash_layer.h>
#include <context.h>
#include <jutils.h>


// callbacks
static void showUrl(char *url, char *target, void *client_data) {
  act("Flash showURL : %s",url);
}
static int readfile(char *filename, char **buffer, long *size) {
  FILE *in;
  char *buf;
  long length;
  
  in = fopen(filename,"r");
  if (in == 0) {
    error("FlashLayer::readfile : error on %s",filename);
    return -1;
  }
  fseek(in,0,SEEK_END);
  length = ftell(in);
  rewind(in);
  buf = (char*)malloc(length);
  act("readFile allocated %u bytes",length);
  fread(buf,length,1,in);
  fclose(in);
  
  *size = length;
  *buffer = buf;
  
  return length;
}
static void getSwf(char *url, int level, void *client_data) {
  FlashHandle flashHandle;
  char *buffer;
  long size;

  func("FlashLayer : getSwf called on url %s level %i",url,level);
	
  flashHandle = (FlashHandle) client_data;

  if(readfile(url, &buffer, &size) > 0) {
    FlashParse(flashHandle, level, buffer, size);
  }

}

FlashLayer::FlashLayer() 
  : Layer() {
  filedesc = NULL;
  render = NULL;
  procbuf = NULL;

  fh = FlashNew();
  if(!fh) {
    error("can't create a new FlashHandle");
    return;
  }



  set_name("SWF");
}

FlashLayer::~FlashLayer() {
  close();
}

bool FlashLayer::init(int width, int height) {
  func("FlashLayer::init");
  long res;

  _init(width,height);

  if(render) free(render);
  render = calloc(geo.size,1);
  if(procbuf) free(procbuf);
  procbuf = calloc(geo.size,1);
  
  fd.pixels = procbuf;
  fd.width = geo.w;
  fd.height = geo.h;
  fd.bpl = geo.pitch;
  fd.depth = geo.bpp>>3; //DefaultDepth(dpy, DefaultScreen(dpy));
  fd.bpp = geo.bpp>>3; // bytes per pixel

  res = FlashGraphicInit(fh, &fd);
  if(!res) {
    error("FlashGraphicInit error");
    return false;
  }

  FlashSetGetUrlMethod(fh, showUrl, 0);
  
  FlashSetGetSwfMethod(fh, getSwf, (void*)fh);


  return true;
}

bool FlashLayer::open(char *file){
  char *buffer;
  long size;
  int status;

  if(!readfile(file,&buffer,&size))
    return false;
  
  // Load level 0 movie
  do {
    status = FlashParse(fh, 0, buffer, size);
  } while (status & FLASH_PARSE_NEED_DATA);
  
  free(buffer);
  
  FlashGetInfo(fh, &fi);
  
  FlashSettings(fh, PLAYER_LOOP);

  
  set_filename(file);

  return true;
}

void *FlashLayer::feed() {
  struct timeval wd;
  long cmd, wakeUp;
  cmd = FLASH_WAKEUP;
  //  fe.type = FeRefresh;
  
  wakeUp = FlashExec(fh, FLASH_WAKEUP, 0, &wd);
  //  if(fd.flash_refresh) {
    jmemcpy(render,procbuf,fd.height*fd.bpl);
    //    fd.flash_refresh = 0;
    //  }
  //    memcpy(render,buffer,fd.width*fd.height*(fd.bpp>>3));

    //  return render;
    return procbuf;
}

void FlashLayer::close() {
  FlashClose(fh);
  if(procbuf) free(procbuf);
  if(render) free(render);
}

bool FlashLayer::keypress(char key) {

  //    FlashEvent fe;
    // TODO parse SDL event here and fill in flash
  //    if (flag & FLASH_EVENT) {
      /* X to Flash event structure conversion
        switch (event->type) {
        case ButtonPress:
            fe.type = FeButtonPress;
            break;
        case ButtonRelease:
            fe.type = FeButtonRelease;
            break;
        case MotionNotify:
            fe.type = FeMouseMove;
            fe.x = event->xmotion.x;
            fe.y = event->xmotion.y;
            break;
        case Expose:
            fe.type = FeRefresh;
            break;
        default:
            fe.type = FeNone;
            break;
	    }*/
  //      fe.type = FeNone;
      //??      fe.type = FeRefresh;
  //    }
    
//    return FlashExec(fh,flag,&fe,wakeDate);
  return false;
}
