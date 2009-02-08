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

#include <config.h>

#ifdef WITH_AVIFILE 

#include <string.h>

#include <avi_layer.h>
#include <avm_fourcc.h>
#include <renderer.h>

#include <ccvt.h>
#include <context.h>
#include <jutils.h>

AviLayer::AviLayer() 
  :Layer() {
  _avi = NULL;
  _stream = NULL;
  steps = 1;
  set_name("AVI");
  yuvcc = false;
}

AviLayer::~AviLayer() {
  close();
}

bool AviLayer::init(Context *scr) {
  func("AviLayer::init");
  avi_dirty = paused = false;
  marker_in = marker_out = slow_frame = slow_frame_s = 0;
  play_speed = 1;
  _quality = 1;

  
  _ci = (CodecInfo *)CodecInfo::match(fccDIV3);
  //  CodecInfo::Get(_ci, avm::CodecInfo::Video, avm::CodecInfo::Decode, fcc);
  //  Creators::SetCodecAttr
  //    (*_ci, (const char*)"Quality", (const char*)_quality);
  func("biBitCount %i biClrUsed %i biClrImportant %i",
       bh.biBitCount,bh.biClrUsed,bh.biClrImportant);
  if(bh.biBitCount!=32) {
    act("Movie file decoding produces %ibit colorspace images",bh.biBitCount);
    act("assuming YUV420 planar colorspace format, using software conversion");
    yuvcc = true;
  }

  _init(scr,labs(bh.biWidth),labs(bh.biHeight),32); //bh.biBitCount);

  if(yuvcc) buffer = malloc(geo.size*2);

  /* fill up first frame */
  while(_stream->ReadFrame(true)<0)
    _stream->Seek(1);

  _img = _stream->GetFrame(false);

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
  //catch(FatalError &e) {
  catch(void *p) {
    error("AviLayer::open(%s)",file);
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

    // _stream->GetDecoder()->SetDestFmt('YUY2');
    // _stream->GetDecoder()->SetDestFmt(12,'I420');
    _stream->GetDecoder()->SetDestFmt(32);
    _stream->ReadFrame(false);
    bh = _stream->GetDecoder()->GetDestFmt();
     
  }
  //catch(FatalError &e) {
  catch(void *p) {
    error("Avilib fatal error");
   // e.Print();
    return(false);
  }

  set_filename(file);

  return(true);
}
  
void *AviLayer::feed() {
  if(paused) 
    if(!avi_dirty) return buffer;  
  
  int32_t curr, curpos, curlen;
  curr = curpos = _stream->GetPos(); // uint_t
  curlen = _stream->GetLength();

  if(slow_frame>0) {
    slow_frame_s--;
    if (slow_frame_s>0) return buffer;
    else slow_frame_s=slow_frame;
  }
  if(slow_frame<0) {
    slow_frame_s++;
    if (slow_frame_s<0) return buffer;
    else {
      // unlogic? use -slowframe with -play_speed 8-)
      curr -= 2;
      slow_frame_s=slow_frame;
    }
  }
  
  _img->Release();
				
  if (play_speed != 1)  
    curr += play_speed - 1;
	
  if (curr < 0) 
    curr += curlen;
  if (curr >= curlen)
    curr -= curlen;
  
  if ( curr != curpos ) {
    _stream->Seek(curr);
    
  } else if (play_speed>0) { // we're going forward
    if (marker_out) // if there is a mark out
      if (marker_out < (uint32_t)curr) { // the mark out got passed:
	if(marker_in) { // if there is a mark in
	  curr = _stream->SeekToKeyFrame(marker_in); // loop to that
	}
      }
    
  } else if (play_speed<0) { // we're going backward
    if (marker_in) // if there is a mark in
      if (marker_in > (uint32_t)curr) { // the mark in passed:
	if (marker_out) // if there is a mark out
	  curr = _stream->SeekToKeyFrame(marker_out); // loop to that
	else // if there is no mark out
	  curr = _stream->SeekToKeyFrame(0); // goes back to the beginning
      }
 }

  while(_stream->ReadFrame(true) <0)
    _stream->Seek(curr);

  _img = _stream->GetFrame(false);

  avi_dirty=false;
  
  if(yuvcc) {
    if(bh.biBitCount == 12)
      ccvt_420p_rgb32(geo.w,geo.h,_img->Data(),buffer);
  } else buffer = _img->Data();

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
  
  if(yuvcc) free(buffer);
}


/* now some actions */

framepos_t AviLayer::forward(framepos_t step) {
  framepos_t res = 0;
  //  lock_feed();
  if(step==1) res = _stream->SeekToNextKeyFrame();
  else {
    framepos_t p = _stream->GetPos();
    res = _stream->SeekToKeyFrame(p+step);
  }
  //  unlock_feed();
  //  show_osd("avi seek to %u\% (K%u)",
  //       (res*100)/_stream->GetLength(),res);
  avi_dirty = true;
  return(res);
}

framepos_t AviLayer::rewind(framepos_t step) {
  framepos_t res = 0;
  //  lock_feed();
  if(step==1) res = _stream->SeekToPrevKeyFrame();
  else {
    framepos_t p = _stream->GetPos();
    res = _stream->SeekToKeyFrame(p-step);
  }
  //  unlock_feed();
  //  show_osd("avi seek to %u\% (K%u)",
  //       (res*100)/_stream->GetLength(),res);
  avi_dirty = true;
  return(res);
}

framepos_t AviLayer::setpos(framepos_t step) {
  framepos_t res = 0;
  //  lock_feed();
  res = _stream->SeekToKeyFrame(step);
  //  unlock_feed();
  //  notice("avi seek to %u\% (K%u)",
  //       (res*100)/_stream->GetLength(),res);
  show_osd();
  avi_dirty = true;
  return(res);
}
  
void AviLayer::pause() {
  paused = !paused;
  notice("avi pause : %s",(paused)?"on":"off");
  show_osd();
}

void AviLayer::set_play_speed(int speed) {
	play_speed += speed;
	show_osd("AviLayer::play speed is now %i", play_speed);
}

void AviLayer::set_slow_frame(int speed) {
	slow_frame += speed;
	show_osd("AviLayer::frame rate is now %i", slow_frame);
}

framepos_t AviLayer::getpos() {
  return _stream->GetPos();
}

framepos_t AviLayer::mark_in(framepos_t pos) {
  marker_in = pos;
  return marker_in;
}
framepos_t AviLayer::mark_in_now() {
  marker_in = _stream->GetPos();
  return marker_in;
}
framepos_t AviLayer::mark_out(framepos_t pos) {
  marker_out = pos;
  return marker_out;
}
framepos_t AviLayer::mark_out_now() {
  marker_out = _stream->GetPos();
  return marker_out;
}

bool AviLayer::keypress(int key) {
  bool res = true;
  framepos_t off;
  switch(key) {
  case 'k':
    off = forward(1);
    show_osd("AviLayer seek forward to %u\% (K%u)",
	     (off*100)/_stream->GetLength(),off);
    break;
  case 'j':
    off = rewind(1);
    show_osd("AviLayer rewind to %u\% (K%u)",
	     (off*100)/_stream->GetLength(),off);
    break;

    /*
Hi Jaromil,
I have been thinking about the looping problem, my feeling now is that
it does not comply with a natural usage. My advise is:

If you you click the IN-point, the OUT-point is cleared by default
If you then click the OUT-point, the clip starts looping

If you re-click the OUT point, the clip loops between the previous
IN-point and the new OUT-point                                                                                                       
In this case, people wont get confused.

Cheers,
Robert

That's ok, i make an addition to this:
if you re-click the OUT point, the OUT point gets deleted
at the next click, you mark another OUT point
(this way you can mark a new OUT position after the current one)
-jrml
    */

  case 'i':
    if(marker_in) {
      marker_in = 0;
      marker_out = 0;
    } else {
      mark_in_now(); // mark_in = _stream->GetPos();
      marker_out = 0;
    }
    act("AviLayer::mark IN[%u] - OUT[%u] (%s)",
	marker_in, marker_out, (marker_in)?"ON":"OFF");
    show_osd();
    break;

  case 'o':
    if(marker_out) { // there is allready a markout
      marker_out = 0;
    } else
      mark_out_now(); //      mark_out = _stream->GetPos();
    act("AviLayer::mark IN[%u] - OUT[%u] (%s)",
	marker_in, marker_out, (marker_in&&marker_out)?"ON":"OFF");
    show_osd();
    break;
    
  case 'l':
    marker_in = 0;
    marker_out = 0;
    act("AviLayer::marks removed IN[0] - OUT[0]");
    show_osd();
    break;
    /*    
  case 'n': 
    set_play_speed(-1);
    break;

  case 'm': 
    set_play_speed(+1);
    break;

  case 'v': 
    set_slow_frame(-1);
    break;

  case 'b': 
    set_slow_frame(+1);
    break;
    */	
  case 'p':
    pause();
    break;

  default:
    res = false;
    break;
  }
  return res;
}

#endif
