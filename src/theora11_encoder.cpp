/*  FreeJ - Ogg/Vorbis/Theora 1.1 encoder
 *
 *  (c) Copyright 2009 Denis Roio <jaromil@dyne.org>
 *
 * ogg/vorbis/theora code learned from encoder_example in 1.1 beta2
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <config.h>

#ifdef WITH_OGGTHEORA

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <context.h>
#include <sdl_screen.h>
#include <audio_collector.h>

#include <theora11_encoder.cpp>

Theora11Encoder::Theora11Encoder()
  : VideoEncoder() {
  func("%s",__PRETTY_FUNCTION__);
  
  frame_w=0;
  frame_h=0;
  pic_w=0;
  pic_h=0;
  pic_x=0;
  pic_y=0;
  video_fps_n=-1;
  video_fps_d=-1;
  video_par_n=-1;
  video_par_d=-1;

}

Theora11Encoder::~Theora11Encoder() {

  if(use_audio){
    ogg_stream_clear(&vo);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
  }
  ogg_stream_clear(&to);
  th_encode_free(td);
  th_comment_clear(&tc);

  if(audio_buf) free(audio_buf);

}


Theora11Encoder::init(Context *_env) {

  if(initialized) return true;
  
  env = _env;

  screen = env->screen;

  srand (time (NULL));
  rand_serial = rand(); // random seed, will be incremented for each new stream
  
  /* yayness.  Set up ogg audio output stream */
  if(use_audio) {
    ogg_stream_init(&vo,rand_serial);
    rand_serial++;
  }
  
  // and video output stream
  ogg_stream_init(&to,rand_serial);
  rand_serial++;

  ////////////////////////
  // Set up Theora encoder
  
  /* Theora  has a  divisible-by-sixteen restriction  for  the encoded
     frame  size scale  the picture  size up  to the  nearest  /16 and
     calculate offsets */
  frame_w = screen->w + 15&~0xF;
  frame_h = screen->h + 15&~0xF;
  /* Force the offsets to be even  so that chroma samples line up like
     we expect.*/
  pic_x= frame_w - screen->w >>1&~1;
  pic_y= frame_h - screen->h >>1&~1;

  th_info_init(&ti);
  ti.frame_width=frame_w;
  ti.frame_height=frame_h;
  ti.pic_width=screen->w;
  ti.pic_height=screen->h;
  ti.pic_x=pic_x;
  ti.pic_y=pic_y;
  ti.fps_numerator=video_fps_n;
  ti.fps_denominator=video_fps_d;
  ti.aspect_numerator=video_par_n;
  ti.aspect_denominator=video_par_d;
  ti.colorspace=TH_CS_UNSPECIFIED;
  /*Account for the Ogg page overhead.
    This is 1 byte per 255 for lacing values, plus 26 bytes per 4096 bytes for
     the page header, plus approximately 1/2 byte per packet (not accounted for
     here).*/
  ti.target_bitrate=(int)(64870*(ogg_int64_t)video_r>>16);
  ti.quality=video_q;
  ti.keyframe_granule_shift=ilog(keyframe_frequency-1);
  ti.pixel_fmt = TH_PF_422;

  td=th_encode_alloc(&ti);
  th_info_clear(&ti);


  /* setting just the granule shift only allows power-of-two keyframe
     spacing.  Set the actual requested spacing. */
  ret = th_encode_ctl(td,TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE,&keyframe_frequency,
		      sizeof(keyframe_frequency-1));
  if(ret<0) {
    error("Could not set keyframe interval to %d",(int)keyframe_frequency);
  }

  if(vp3_compatible){
    ret=th_encode_ctl(td,TH_ENCCTL_SET_VP3_COMPATIBLE,&vp3_compatible,
     sizeof(vp3_compatible));
    if(ret<0||!vp3_compatible){
      error("Could not enable strict VP3 compatibility");
      if(ret>=0){
        error("Ensure your source format is supported by VP3");
        error("(4:2:0 pixel format, width and height multiples of 16)");
      }
    }
  } /* init theora done */


  /* initialize Vorbis too, assuming we have audio to compress. */
  if(use_audio){
    vorbis_info_init(&vi);
    if(audio_q>-99)
      /* Encoding using a VBR quality mode.  */
      ret = vorbis_encode_init_vbr(&vi,audio_ch,audio_hz,audio_q);
    else
      ret = vorbis_encode_init(&vi,audio_ch,audio_hz,-1,
       (int)(64870*(ogg_int64_t)audio_r>>16),-1);
    if(ret){
      error("Vorbis encoder setup failed with requested quality or bitrate");
      exit(1);
    }
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag (&vc, "ENCODER",PACKAGE);
    /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd,&vi);
    vorbis_block_init(&vd,&vb);
  }    /* audio init done */

  
  /* write the bitstream header packets with proper page interleave */
  
  th_comment_init(&tc);
  
  /* first packet will get its own page automatically */
  if(th_encode_flushheader(td,&tc,&op)<=0){
    error("internal Theora library error");
    return(false);
  }
  ogg_stream_packetin(&to,&op);
  if(ogg_stream_pageout(&to,&og)!=1){
    error("internal Ogg library error");
    return(false);
  }
  
  ogg_pipe_write("write theora header", ringbuffer, (char*)og.header, og.header_len);
  ogg_pipe_write("write theora body", ringbuffer, (char*)og.body, og.body_len);

  /* create the remaining theora headers */
  for(;;){
    ret=th_encode_flushheader(td,&tc,&op);
    if(ret<0){
      error("Internal Theora library error");
      return(false);
    }
    else if(!ret) break;
    ogg_stream_packetin(&to,&op);
  }

  if(use_audio) {
    
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;
    
    vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
    ogg_stream_packetin(&vo,&header); /* automatically  placed  in its
                                         own page */
    if(ogg_stream_pageout(&vo,&og)!=1){
      error("Internal Ogg library error");
      return(false);
    }
    
    ogg_pipe_write("write vorbis header", ringbuffer, (char*)og.header, og.header_len);
    ogg_pipe_write("write vorbis body", ringbuffer, (char*)og.body, og.body_len);
    
    /* remaining vorbis header packets */
    ogg_stream_packetin(&vo,&header_comm);
    ogg_stream_packetin(&vo,&header_code);
  }

  /* Flush the rest of our headers. This ensures
     the actual data in each stream will start
     on a new page, as per spec. */
  for(;;){
    int result = ogg_stream_flush(&to,&og);
    if(result<0){
      /* can't get here */
      error("Internal Ogg library error.");
      return(false);
    }
    if(result==0) break;
    ogg_pipe_write("write vorbis header", ringbuffer, (char*)og.header, og.header_len);
    ogg_pipe_write("write vorbis body", ringbuffer, (char*)og.body, og.body_len);
  }
  if(audio){
    for(;;){
      int result=ogg_stream_flush(&vo,&og);
      if(result<0){
        /* can't get here */
        error("Internal Ogg library error");
	return(false);
      }
      if(result==0)break;
      ogg_pipe_write("write vorbis header", ringbuffer, (char*)og.header, og.header_len);
      ogg_pipe_write("write vorbis body", ringbuffer, (char*)og.body, og.body_len);
    }
  }

  // init complete
  // ----

  act("initialization succesful");
  initialized = true;
  
  return true;
 
}

Theora11Encoder::encode_frame() {

  return bytes_encoded;

}
