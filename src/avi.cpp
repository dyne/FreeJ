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

    /*
      if(strlen(file)>=4 &&
      strncasecmp(file+sizeof(file)-4,".avi",4)==0 ||
      strncasecmp(file+sizeof(file)-4,".asf",4)==0 ) */


#include <iostream>

#include <avi.h>
#include <avifile/except.h>
#include <avifile/renderer.h>
#include <jutils.h>

AviLayer::AviLayer() {
  _avi = NULL;
  _stream = NULL;
}

AviLayer::~AviLayer() { 
}

bool AviLayer::init(Context *screen, int wdt, int hgt) {
  _init(screen,wdt,hgt);
  return true;
}

bool AviLayer::open(char *file) {
  func("AviLayer::open(%s)",file);
  if(_avi) close();
  
  try {
    _avi = CreateIAviReadFile(file);
  }
  catch(FatalError &e) {
    error("AviLayer::open(%s)",file); // - %s",file,e.Print());
    return(false);
  }

  if(!_avi->IsOpened()) {
    /* check if here we got to free something */
    error("Avilayer::open(%s) - can't open file",file);
    return(false);
  }
  
  if(!_avi->IsValid()) {
    /* check if here we got to free something */
    error("AviLayer::open(%s) - invalid file",file);
    return(false);
  }

  notice("AVI Layer :: %s",file);
  
  if(_avi->IsRedirector())
    act("supplied url is a network redirector");
  
  /* setup auto quality
  IvideoDecoder *vd = _stream->GetDecoder();
  if(vd) {
    IVideoDecoder::DecodingMode mode = IVideoDecoder::DIRECT;
    if(_buffered) mode = IVideoDecoder::BUFFERED_QUALITY_AUTO;
    vb->SetDecodingMode(mode);
  }
  ------------------------- */

  //  _stream = _avi->GetStream(0, IAviReadStream::Video);
  _stream = _avi->GetStream(0, AviStream::Video);
  if(!_stream) {
    /* check if here we got to free something */
    error("AviLayer::open(%s) - video stream not detected",file);
    return(false);
  }

  BITMAPINFOHEADER bh;

  try {

    if(_stream->StartStreaming()!=0) {
      /* check if here we got to free something */
      error("AviLayer::open(%s) - failed to initialize decoder object",file);
      return(false);
    }
    _stream->GetDecoder()->SetDestFmt(24); // QUAAAAAA

    _stream->GetOutputFormat(&bh, sizeof(bh));

    geo.w = labs(bh.biWidth);
    geo.h = labs(bh.biHeight);
    geo.bpp = bh.biBitCount;
    geo.size = geo.w*geo.h*(geo.bpp/8);
    geo.pitch = geo.w*(geo.bpp/8);
  }
  catch(FatalError &e) {
    error("fatal error");//: %s",e.Print());
    return(false);
  }
  /*
  rgb_buffer = jalloc(rgb_buffer,geo.size);
  */
  feed();
  
  notice("AviLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	 geo.w,geo.h,geo.bpp,geo.size);
  return(true);
}
  
void *AviLayer::get_buffer() {
  func("AviLayer::get_buffer()");
  void *res;
  res = _img->Data();
  func("AviLayer::get_buffer() returned %p",res);
  return res;
}

void AviLayer::feed() {
  func("AVILAYER FEED");
  _stream->ReadFrame(true);
  _img = _stream->GetFrame(false);
  func("AVILAYER FEED OK");
}

void AviLayer::close() {
  /* here close Aviclass */
  if(_stream)
    if(_stream->IsStreaming())
      _stream->StopStreaming();
  _avi = NULL;
}

bool AviLayer::keypress(SDL_keysym *keysym) {
  return false;
}
