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

#ifndef __xhl_h__
#define __xhl_h__

//#include <renderer.h>
#undef BIG_ENDIAN
#include <layer.h>
#define NIL (0)       // A name for the void pointer

class XHacksLayer: public Layer {
 private:
  //IAviReadFile *_avi;
  //IAviReadStream *_stream;
  
  /* AviReadStream.h
     interesting methods:
     //Size of stream and one frame
     virtual framepos_t GetLength() const;
     virtual double GetLengthTime() const;
     virtual int GetFrameFlags(int* flags) const;
     virtual double GetFrameTime() const;
     virtual framepos_t GetPos() const;
     virtual StreamInfo* GetStreamInfo() const;
     virtual double GetTime(framepos_t frame = ERR) const;
     //Positioning in stream
     virtual int Seek(framepos_t pos);
     virtual int SeekTime(double pos);
     virtual framepos_t SeekToKeyFrame(framepos_t pos);
     virtual double SeekTimeToKeyFrame(double pos);
     virtual int SkipFrame();
     virtual int SkipTo(double pos);
     //Reading decompressed data
     virtual int SetDirection(bool d) { return -1; }
     virtual int SetOutputFormat(void* bi, uint_t size);
     virtual int ReadFrame(bool render = true);
     virtual CImage* GetFrame(bool readframe = false);
     virtual uint_t GetFrameSize() const;
     int ReadFrames(void* buffer, uint_t bufsize, uint_t samples,uint_t& samples_read, uint_t& bytes_read);
     int ReadDirect(void* buffer, uint_t bufsize, uint_t samples,uint_t& samples_read, uint_t& bytes_read, int* flags = 0);
     virtual framepos_t GetNextKeyFrame(framepos_t frame = ERR) const;
     virtual framepos_t GetPrevKeyFrame(framepos_t frame = ERR) const;
     virtual framepos_t SeekToNextKeyFrame();
     virtual framepos_t SeekToPrevKeyFrame(); */
  /* -------------------- */
  
  //  VideoRenderer *_rend;

  int x_pid;
  char *_name;
  char *_author;
  char *_info;
  int _version;
  //  int _bpp;
  char *_path;

 public:
  XHacksLayer();
  ~XHacksLayer();
  
 int paused;
  //bool init(Context *screen=NULL);
  bool init(Context *screen);
  Context *screen;
  bool open(char *file);
  void *feed();
  void close();
  void pause(bool paused);
  
  bool keypress(SDL_keysym *keysym);
  //extern char *progclass;
 // void xhacks_handle_events(int);
};
#endif
