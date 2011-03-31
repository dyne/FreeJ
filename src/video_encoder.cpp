/*  FreeJ
 *  (c) Copyright 2005      Silvano Galliani <kysucix@dyne.org>
 *                2007-2009 Denis Roio       <jaromil@dyne.org>
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
#include <iostream>

#include <config.h>

#include <string.h>
#include <context.h>

#include <video_encoder.h>


#include <convertvid.h>

/* function below taken from ccvt_misc.c
   CCVT: ColourConVerT: simple library for converting colourspaces
   Copyright (C) 2002 Nemosoft Unv. */
inline void ccvt_yuyv_420p(int width, int height,
			   const void *src, void *dsty,
			   void *dstu, void *dstv) {
  register int n, l, j;
  register unsigned char *dy, *du, *dv;
  register unsigned char *s1, *s2;
  
  dy = (unsigned char *)dsty;
  du = (unsigned char *)dstu;
  dv = (unsigned char *)dstv;
  s1 = (unsigned char *)src;
  s2 = s1; // keep pointer
  n = width * height;
  for (; n > 0; n--) {
    *dy = *s1;
    dy++;
    s1 += 2;
  }
  
  /* Two options here: average U/V values, or skip every second row */
  s1 = s2; // restore pointer
  s1++; // point to U
  for (l = 0; l < height; l += 2) {
    s2 = s1 + width * 2; // odd line
    for (j = 0; j < width; j += 2) {
      *du = (*s1 + *s2) / 2;
      du++;
      s1 += 2;
      s2 += 2;
      *dv = (*s1 + *s2) / 2;
      dv++;
      s1 += 2;
      s2 += 2;
    }
    s1 = s2;
  }
}



VideoEncoder::VideoEncoder()
  : Entry(), JSyncThread() {

  initialized = false;
  encbuf = NULL;
  use_audio = false;

  write_to_disk   = false;
  write_to_stream = false;

  filedump_fd = NULL;

  status = NULL;
  audio_kbps = 0;
  video_kbps = 0;
  bytes_encoded = 0;
  m_ElapsedTime = 0;
  m_Streamed = 0;
  enc_y = enc_u = enc_v = NULL;

  fps = new FPS();
  fps->init(25); // default FPS
  // initialize the encoded data pipe
  // TODO: set the size to width * height * 4 * nframes (3-4)
  ringbuffer = ringbuffer_create(1048*2096);

  shout_init();
  ice = shout_new();

  if( shout_set_protocol(ice,SHOUT_PROTOCOL_HTTP) )
    error("shout_set_protocol: %s", shout_get_error(ice));

  if( shout_set_format(ice,SHOUT_FORMAT_OGG) )
    error("shout_set_format: %s", shout_get_error(ice));

  if( shout_set_agent(ice,"FreeJ - freej.dyne.org") )
    error("shout_set_agent: %s", shout_get_error(ice));

  if( shout_set_public(ice,1) )
    error("shout_set_public: %s", shout_get_error(ice));

  func("init picture_yuv for colorspace conversion (avcodec)");  
  gettimeofday(&m_OldTime, NULL);
}

VideoEncoder::~VideoEncoder() {
  // flush all the ringbuffer to file and stream
  int encnum = 0;

  if (encbuf)
  {
    do {
      if (encnum = ringbuffer_read_space(ringbuffer))
	encnum = ringbuffer_read(ringbuffer, encbuf, encnum);
// 			     ((audio_kbps + video_kbps)*1024)/24);

      if(encnum <=0) break;

      if(write_to_disk && filedump_fd) {
	size_t nn;
	nn = fwrite(encbuf, 1, encnum, filedump_fd);
      }

      if(write_to_stream) {
	shout_sync(ice);
	shout_send(ice, (const unsigned char*)encbuf, encnum);
      }

      func("flushed %u bytes closing video encoder", encnum);

    } while(encnum > 0); 
    free(encbuf);
  }
  // close the filedump
  if(filedump_fd) fclose(filedump_fd);

  // now deallocate the ringbuffer
  ringbuffer_free(ringbuffer);

  shout_close(ice);
  //  shout_sync(ice);
  //  shout_free(ice);
  shout_shutdown();

  if(enc_y) free(enc_y);
  if(enc_u) free(enc_u);
  if(enc_v) free(enc_v);
  if(enc_yuyv) free(enc_yuyv);
  
  free(fps);
}

void VideoEncoder::thread_setup() {
  func("ok, encoder %s in rolling loop",name);
  func("VideoEncoder::run : begin thread %p",pthread_self());
 }

void VideoEncoder::thread_loop() {
  int encnum;
  int res;
  /* Convert picture from rgb to yuv420 planar 

     two steps here:
     
     1) rgb24a or bgr24a to yuv422 interlaced (yuyv)
     2) yuv422 to yuv420 planar (yuv420p)

     to fix endiannes issues try adding #define ARCH_PPC
     and using 
     mlt_convert_bgr24a_to_yuv422
     or
     mlt_convert_argb_to_yuv422
     (see mlt_frame.h in mltframework.org sourcecode)
     i can't tell as i don't have PPC, waiting for u mr.goil :)
  */
    
    uint8_t *surface = (uint8_t *)screen->get_surface();
  time_t *tm = (time_t *)malloc(sizeof(time_t));
  time (tm);
//   std::cerr << "-- ENC:" << asctime(localtime(tm));
    if (!surface) {
        fps->calc();
        fps->delay();
		//std::cout << "fps->start_tv.tv_sec :" << fps->start_tv.tv_sec << \
			" tv_usec :" << fps->start_tv.tv_usec << "   \r" << std::endl;
        return;
    }
    fps->calc(); 	//without this the thread_loop is called nearly two times more and
    fps->delay();	//stream speed is too slow

    screen->lock();

    switch(screen->get_pixel_format()) {
    case ViewPort::RGBA32:
      mlt_convert_rgb24a_to_yuv422(surface,
                   screen->geo.w, screen->geo.h,
                   screen->geo.w<<2, (uint8_t*)enc_yuyv, NULL);
      break;
      
    case ViewPort::BGRA32:
      mlt_convert_bgr24a_to_yuv422(surface,
                   screen->geo.w, screen->geo.h,
                   screen->geo.w<<2, (uint8_t*)enc_yuyv, NULL);
      break;

    case ViewPort::ARGB32:
      mlt_convert_argb_to_yuv422(surface,
                   screen->geo.w, screen->geo.h,
                   screen->geo.w<<2, (uint8_t*)enc_yuyv, NULL);
      break;
      
    default:
      error("Video Encoder %s doesn't supports Screen %s pixel format",
        name, screen->name);
    }

    screen->unlock();
    
    ccvt_yuyv_420p(screen->geo.w, screen->geo.h, enc_yuyv, enc_y, enc_u, enc_v);

    ////// got the YUV, do the encoding    
    res = encode_frame();

    /// proceed writing and streaming encoded data in encpipe
    
    encnum = 0;
    if(write_to_disk || write_to_stream) {
      if (encnum = ringbuffer_read_space(ringbuffer))
      {
	encbuf = (char *)realloc(encbuf, encnum);
// 	encbuf = (char *)realloc(encbuf, (((audio_kbps + video_kbps)*1024)/24)); //doesn't change anything for shifting problem
	encnum = ringbuffer_read(ringbuffer, encbuf, encnum);
// 	encnum = ringbuffer_read(ringbuffer, encbuf,
// 			       ((audio_kbps + video_kbps)*1024)/24);
      }
    }

    if(encnum > 0) {
      //      func("%s has encoded %i bytes", name, encnum);
      if(write_to_disk && filedump_fd) 
        fwrite(encbuf, 1, encnum, filedump_fd);
    
      if(write_to_stream) {
/*	int	wait_ms;
	wait_ms = shout_delay(ice);
	std::cerr << "---- shout delay :" << wait_ms << std::endl;*/
// 	shout_sync(ice);	//no sound when commented out !!
        if( shout_send(ice, (const unsigned char*)encbuf, encnum)
	      != SHOUTERR_SUCCESS) {
            error("shout_send: %s", shout_get_error(ice));
        }// else 
            //printf("%d %d\n", encnum, (int)shout_queuelen(ice));
      }
      gettimeofday(&m_ActualTime, NULL);
      if (m_ActualTime.tv_sec == m_OldTime.tv_sec)
	m_ElapsedTime += ((double)(m_ActualTime.tv_usec - m_OldTime.tv_usec))/1000000.0;
      else
	m_ElapsedTime += ((double)(m_ActualTime.tv_sec - m_OldTime.tv_sec)) + \
	  (((double)(m_ActualTime.tv_usec - m_OldTime.tv_usec))/1000000.0);
      m_OldTime.tv_sec = m_ActualTime.tv_sec;
      m_OldTime.tv_usec = m_ActualTime.tv_usec;
      m_Streamed += encnum;
      if (m_ElapsedTime >= 3.0)		//calculate stream rate every minimum 3 seconds
      {
	m_StreamRate = ((double)m_Streamed / m_ElapsedTime) / 1000.0;
	m_ElapsedTime = 0;
	m_Streamed = 0;
      }
    } 
}

double VideoEncoder::getStreamRate()
{
  return (m_StreamRate);
}

void VideoEncoder::thread_teardown() {
  func("VideoEncoder::run : end thread %p", pthread_self() );
}

bool VideoEncoder::set_filedump(const char *filename) {
  int filename_number=1;
  FILE *fp;

  if(write_to_disk) { // stop current filedump
    if(filedump_fd) {
      fclose(filedump_fd);
      filedump_fd = NULL;
    }
    act("Encoder %s stopped recording to file %s", name, filedump);
    write_to_disk = false;
  }

  // if filename is NULL, stop recording and that's it
  if(!filename) return false;

  // another filename is provided, start recording to it

  // store the filename
  strncpy(filedump,filename,512);


  // file already exists?
  fp = fopen(filedump, "r");
  while(fp) {

    // append a number to the filename
    char tmp[512];
    char *point;
    int lenght_without_extension;

    fclose(fp);
    
    // take point extension pointer ;P
    point = strrchr(filedump,'.');
    lenght_without_extension = (point - filedump);
    
    // copy the string before the  point
    strncpy (tmp, filedump, lenght_without_extension);
    
    // insert -n
    sprintf (tmp + lenght_without_extension, "-%d%s",
	      filename_number, filedump + lenght_without_extension);

    strncpy (filedump, tmp, 512);

    // increment number inside filename
    filename_number++;

    fp = fopen(filedump, "r");
  }

  filedump_fd = fopen(filedump,"w");
  if(!filedump_fd) {
    error("can't record to file %s: %s", filedump_fd, strerror(errno));
    return false;
  }

  act("Encoder %s recording to file %s", name, filedump);
  write_to_disk = true;

	  // if it's a number handle it as a file descriptor
	  // TODO QUAAA
	  //  if (isdigit (atoi (filename)))
	  //    return true;

  return true;
}

bool VideoEncoder::filedump_close()
{
  if (filedump_fd)
  {
    if (!fclose (filedump_fd))
    {
      write_to_disk = false;
      return (true);
    }
    else
    {
      std::cerr << "----- can't close :" << filedump << " " << errno << std::endl;
      return (false);
    }
  }
}
