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

#include <config.h>

#ifdef WITH_AVIFILE 

#include <iostream>
#include <string.h>

#include <avi_layer.h>
#include <avifile/except.h>
#include <avifile/renderer.h>
#include <avifile/avm_fourcc.h>
#include <avifile/creators.h>
#include <avifile/renderer.h>

#include <context.h>
#include <lubrify.h>
#include <jutils.h>

AviLayer::AviLayer() 
  :Layer() {
  _avi = NULL;
  _stream = NULL;
  setname("AVI");
}

AviLayer::~AviLayer() {
  close();
}

bool AviLayer::init(Context *scr) {
  func("AviLayer::init");
  _quality = 1;

  
  _ci = (CodecInfo *)CodecInfo::match(fccDIV3);
  //  CodecInfo::Get(_ci, avm::CodecInfo::Video, avm::CodecInfo::Decode, fcc);
  Creators::SetCodecAttr
    (*_ci, (const char*)"Quality", (const char*)_quality);

  if(bh.biBitCount!=32) {
    error("AVI layer does'nt supports color depths different from 32bpp");
    error("and cannot find a suitable colorspace conversion routine for this avi file");
    error("sorry, you can't use the selected AVI");
    close();
    return false;
  }

  if(scr) screen = scr;
  _init(screen,
	bh.biWidth,
	labs(bh.biHeight),
	bh.biBitCount);

  buffer = jalloc(buffer,geo.size);

  feed();
  notice("AviLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	 geo.w,geo.h,geo.bpp,geo.size);

  
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

  if(!_avi) {
    error("AviLayer::open(%s) - can't open file",file);
    return (false);
  }

  _stream = _avi->GetStream(0, AviStream::Video);
  if(!_stream) {
    /* check if here we got to free something */
    error("AviLayer::open(%s) - video stream not detected",file);
    return(false);
  }

  /* this crashes on .37 avifile
     verified with shezzan on 14 july 2003
     _stream->SetDirection(true); */

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

  try {


    /*
     * SetDestFmt() sets desired bit depth and color space of output picture. 
     * Returns zero on success, -1 if format is unsupported and 
     * +1 if there was no 'native' support for the depth and library
     * is going to perform internal conversion of format. Most decoders 
     * support depths 15, 16, 24 or 32. Many of them also allow to use
     * YUV formats. You can check if your particular decoder is able to decode
     * into the particular YUV format by calling GetCapabilities(),
     * which returns bitwise OR of supported formats.
     */


    if(_stream->StartStreaming()!=0) {
      /* check if here we got to free something */
      error("AviLayer::open(%s) - failed to initialize decoder object",file);
      return(false);
    }

    _stream->GetDecoder()->SetDestFmt(32);
    _stream->ReadFrame(false);
    bh = _stream->GetDecoder()->GetDestFmt();
     
  }
  catch(FatalError &e) {
    error("Avilib fatal error");
    e.Print();
    return(false);
  }

  set_filename(file);

  return(true);
}
  
void *AviLayer::feed() {
  if(paused) return buffer;  
    
  while(_stream->ReadFrame(true) <0)
    _stream->Seek(1);

  _img = _stream->GetFrame(false);

  memcpy(buffer,_img->Data(),geo.size);
  //  mmxcopy(_img->Data(),buf,geo.size);
  
  _img->Release();

  return buffer;
}

void AviLayer::close() {
  /* here close Aviclass
     TODO: CHECK CAREFULLY here */
  notice("Closing AVI layer");
  if(_stream)
    //    if(_stream->IsStreaming())
    _stream->StopStreaming();
  delete _avi;
  _avi = NULL;
  
  if(buffer) jfree(buffer);
}


/* now some actions */

void AviLayer::forward(framepos_t step) {
  framepos_t res = 0;
  lock_feed();
  if(step==1) res = _stream->SeekToNextKeyFrame();
  else {
    framepos_t p = _stream->GetPos();
    res = _stream->SeekToKeyFrame(p+step);
  }
  unlock_feed();
  show_osd("avi seeked to %u\% (K%u)",
       (res*100)/_stream->GetLength(),res);
}

void AviLayer::rewind(framepos_t step) {
  framepos_t res = 0;
  lock_feed();
  if(step==1) res = _stream->SeekToPrevKeyFrame();
  else {
    framepos_t p = _stream->GetPos();
    res = _stream->SeekToKeyFrame(p-step);
  }
  unlock_feed();
  notice("avi seeked to %u\% (K%u)",
       (res*100)/_stream->GetLength(),res);
  show_osd();
}

void AviLayer::pos(framepos_t p) {
  framepos_t res = 0;
  lock_feed();
  res = _stream->SeekToKeyFrame(p);
  unlock_feed();
  notice("avi seeked to %u\% (K%u)",
       (res*100)/_stream->GetLength(),res);
  show_osd();
}
  
void AviLayer::pause() {
  paused = !paused;
  func("avi pause : %s",(paused)?"on":"off");
}

bool AviLayer::keypress(SDL_keysym *keysym) {
  bool res = false;
  framepos_t steps = 1;
  switch(keysym->sym) {
  case SDLK_RIGHT:
    if(keysym->mod & KMOD_LCTRL) steps=5000;
    if(keysym->mod & KMOD_RCTRL) steps=1000;
    forward(steps);
    res = true; break;

  case SDLK_LEFT:
    if(keysym->mod & KMOD_LCTRL) steps=5000;
    if(keysym->mod & KMOD_RCTRL) steps=1000;
    rewind(steps);
    res = true; break;
    
  case SDLK_KP0: pause();
    res = true; break;

  default: break;
  }
  return res;
}

#endif
