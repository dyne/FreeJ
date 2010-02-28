#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>  
#include <sys/types.h>
#include <sys/param.h>

#include <shout/shout.h>
#include <theora/theora.h>

#define MNL "" 

#define OBJC

#ifdef OBJC
// ObjC includes to use pixelBuffers
#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>

CVPixelBufferRef sourcePixelBuffer ;
#endif

extern int want_quiet;


ogg_stream_state myGlob_to; 
theora_state     myGlob_td;
ogg_page         myGlob_og; 
yuv_buffer       myGlob_yuv;
shout_t *myShout = NULL;

int video_x=0;
int video_y=0;
int frame_x=0;
int frame_y=0;
int frame_x_offset=0;
int frame_y_offset=0;

// private 
int myOggfwd_process(ogg_page inputVideoPage) {
  if (myShout == NULL) return -1;
  if (inputVideoPage.header_len == 0 || inputVideoPage.body_len == 0) {
    fprintf(stderr, MNL "!!!!!!!! oggfwd - buffer empty\n") ;
    return -1;
  }
  if (shout_send(myShout, inputVideoPage.header, inputVideoPage.header_len) != SHOUTERR_SUCCESS) {
    fprintf(stderr, MNL "OGGFWD - PH (error : %s) \n", shout_get_error(myShout)) ;
    return -1;
  }
  if (shout_send(myShout, inputVideoPage.body, inputVideoPage.body_len) != SHOUTERR_SUCCESS) {
    fprintf(stderr, MNL "OGGFWD - PB (error : %s) \n", shout_get_error(myShout)) ;
    return -1;
  }

  if(shout_queuelen(myShout) > 0) fprintf(stderr, MNL "OGGFWD - queue length: %d\n", shout_queuelen(myShout));

  shout_sync(myShout); // Sleep until the server will be ready for more data.
  return 0;
}

#define OGGERR(error) { fprintf(stderr, MNL "OGGFWD - Error - %s (%s)\n", error, shout_get_error(myShout)); return -1;}
int myOggfwd_init( const char* outIceIp, int outIcePort, const char* outIcePass, const char* outIceMount, 
	           const char *description, const char *genre, const char *name, const char *url ) {
	
  unsigned short  port = outIcePort ;
	
  if ((myShout = shout_new()) == NULL) { 
    printf(MNL "OGGFWD - Error - allocate failed.\n") ;
    return 0;
  }
  if (shout_set_host(myShout, outIceIp) != SHOUTERR_SUCCESS) OGGERR("set host pb");
  if (shout_set_port(myShout, port) != SHOUTERR_SUCCESS) OGGERR("invalid portnumber");
  if (shout_set_password(myShout, outIcePass) != SHOUTERR_SUCCESS) OGGERR("invalid password");
  if (shout_set_mount(myShout, outIceMount) != SHOUTERR_SUCCESS) OGGERR("invalid mountpoint");

  shout_set_format(myShout, SHOUT_FORMAT_OGG);
  shout_set_public(myShout, 1);
	
  if (description)  shout_set_description(myShout, description);
  if (genre)        shout_set_genre(myShout, genre);
  if (name)         shout_set_name(myShout, name);
  if (url)          shout_set_url(myShout, url);
	
  if (shout_open(myShout) == SHOUTERR_SUCCESS) {
    if (!want_quiet)
      fprintf(stderr, "OGGFWD - Connected to server (%s)\n", outIceIp) ;
    return 1;
  }
  if (!want_quiet)
    fprintf(stderr, "OGGFWD - Error connecting to server (%s)\n", outIceIp) ;
  return 0;
}

void encoder_init(int inW, int inH, int inFramerate, int in_video_r, int in_video_q) {
  printf("THEORA - Encoder Video Size: %dx%d\n", inW, inH) ;
  frame_x = inW ;
  frame_y = inH ;
		
  ogg_stream_init(&myGlob_to, rand());
	
  /* Theora has a divisible-by-sixteen restriction for the encoded video size
     scale the frame size up to the nearest /16 and calculate offsets */
  video_x=((frame_x + 15) >>4)<<4;
  video_y=((frame_y + 15) >>4)<<4;
  frame_x_offset=((video_x-frame_x)/2)&~1;
  frame_y_offset=((video_y-frame_y)/2)&~1;
 
  myGlob_yuv.y_width = video_x;
  myGlob_yuv.y_height= video_y;
  myGlob_yuv.y_stride= video_x;

  myGlob_yuv.uv_width=video_x/2;
  myGlob_yuv.uv_height=video_y/2;
  myGlob_yuv.uv_stride=video_x/2;

  printf("THEORA - video: %dx%d +%d+%d\n", video_x, video_y, frame_x_offset, frame_y_offset); 

  theora_info      myGlob_ti;
  theora_comment   myGlob_tc;
  ogg_packet       myGlob_op;

  theora_info_init(&myGlob_ti);
  myGlob_ti.width= video_x;
  myGlob_ti.height= video_y;
  myGlob_ti.frame_width= frame_x;
  myGlob_ti.frame_height= frame_y;
  myGlob_ti.offset_x= frame_x_offset;
  myGlob_ti.offset_y= frame_y_offset;
  myGlob_ti.fps_numerator= inFramerate ; //video_hzn;
  myGlob_ti.fps_denominator= 1 ; 
  myGlob_ti.aspect_numerator= 1 ; 
  myGlob_ti.aspect_denominator= 1 ; 
  myGlob_ti.colorspace=OC_CS_UNSPECIFIED;
  myGlob_ti.colorspace=OC_CS_ITU_REC_470BG;
  myGlob_ti.pixelformat=OC_PF_420;
  myGlob_ti.target_bitrate= in_video_r ;
  myGlob_ti.quality= in_video_q ;
  
  myGlob_ti.dropframes_p=0;
//myGlob_ti.quick_p=1;
  myGlob_ti.keyframe_auto_p=1;
  myGlob_ti.keyframe_frequency=30;
  myGlob_ti.keyframe_frequency_force=myGlob_ti.keyframe_frequency;
  myGlob_ti.keyframe_data_target_bitrate= myGlob_ti.target_bitrate * 4;
  myGlob_ti.keyframe_auto_threshold=80;
  myGlob_ti.keyframe_mindistance=8;
  myGlob_ti.noise_sensitivity=1 ;
  myGlob_ti.sharpness=0; /* range 0-2, 0 sharp, 2 less sharp, less bandwidth */
  
  theora_encode_init(&myGlob_td, &myGlob_ti);
  theora_info_clear(&myGlob_ti);

  //////////////////////////////////////////////////////////
  /* write the bitstream header packets with proper page interleave */
  
  /* first packet will get its own page automatically */
  theora_encode_header(&myGlob_td, &myGlob_op);
  ogg_stream_packetin(&myGlob_to, &myGlob_op);

  if(ogg_stream_pageout(&myGlob_to, &myGlob_og)!=1){
    fprintf(stderr, "Internal Ogg library error.\n");
    exit(1); // XXX
  }
  
  /* send header to icecas t*/
  myOggfwd_process(myGlob_og) ;
	
  /* create the remaining theora headers */
  theora_comment_init(&myGlob_tc);
  theora_encode_comment(&myGlob_tc, &myGlob_op);
  ogg_stream_packetin(&myGlob_to, &myGlob_op);
  /*theora_encode_comment() doesn't take a theora_state parameter, so it has to
    allocate its own buffer to pass back the packet data.
    If we don't free it here, we'll leak.
    libogg2 makes this much cleaner: the stream owns the buffer after you call
    packetin in libogg2, but this is not true in libogg1.*/
  free(myGlob_op.packet);
  theora_encode_tables(&myGlob_td, &myGlob_op);
  ogg_stream_packetin(&myGlob_to, &myGlob_op);
	
  /* Flush the rest of our headers. This ensures
     the actual data in each stream will start
     on a new page, as per spec. */
  while(1){
    int result = ogg_stream_flush(&myGlob_to, &myGlob_og);
    if(result<0){
      fprintf(stderr, MNL "THEORA - OGG stream flush error.\n");
      exit(1);
    }
    if(result==0) break ;
    myOggfwd_process(myGlob_og) ;
  }
  fprintf(stderr, "THEORA - encoder initialized. (framerate=%d,bitrate=%d,quality=%d)\n",inFramerate, in_video_r, in_video_q);
  /* setup complete.  Raw processing loop */
}

// private
void encode_video(uint8_t *theBuffer, int eos) {
  ogg_packet          op;
#if 0
  myGlob_yuv.y= theBuffer;
  myGlob_yuv.u= myGlob_yuv.y + video_x*video_y;
  myGlob_yuv.v= myGlob_yuv.u + video_x*video_y/4;
 #endif 
  theora_encode_YUVin(&myGlob_td, &myGlob_yuv);
  while(theora_encode_packetout (&myGlob_td, eos, &op))
    ogg_stream_packetin(&myGlob_to, &op);
}


int encoder_loop(uint8_t *imBuffRef) {
  encode_video(imBuffRef, 0);

  while(ogg_stream_pageout(&myGlob_to, &myGlob_og)>0) {
    theora_granule_time(&myGlob_td, ogg_page_granulepos(&myGlob_og));
    if (myOggfwd_process(myGlob_og)<0) return -1;
  }
  return 0;
}

void myOggfwd_close() {
  if (myShout == NULL) return;
  shout_close(myShout);
  myShout = NULL;
}

int encoder_close() {
  ogg_stream_clear(&myGlob_to);
  theora_clear(&myGlob_td);
  return(0);
}



// WRAPPER OSX 

#ifdef OBJC
int yuv_copy__argb_to_420(void *b_rgb, SInt32 b_rgb_stride, size_t width, size_t height, size_t offset_x, size_t offset_y, yuv_buffer *dst)
{
	// TODO: offset ! & strides
	
#define _CR ((float)((bptr[(4*i)+1])&0xff))
#define _CG ((float)((bptr[(4*i)+2])&0xff))
#define _CB ((float)((bptr[(4*i)+3])&0xff))
	
#define _CRX ((float)( ( ((bptr[(4*i)+1])&0xff) + ((bptr[(4*(i+1))+1])&0xff) + ((bptr[(4*(i+1+width))+1])&0xff) + ((bptr[(4*(i+1+width))+1])&0xff) )>>2))
#define _CGX ((float)( ( ((bptr[(4*i)+2])&0xff) + ((bptr[(4*(i+1))+2])&0xff) + ((bptr[(4*(i+1+width))+2])&0xff) + ((bptr[(4*(i+1+width))+2])&0xff) )>>2))
#define _CBX ((float)( ( ((bptr[(4*i)+3])&0xff) + ((bptr[(4*(i+1))+3])&0xff) + ((bptr[(4*(i+1+width))+3])&0xff) + ((bptr[(4*(i+1+width))+3])&0xff) )>>2))
	
	uint8_t *bptr = (uint8_t*) b_rgb;
	int i; int c=0;
	for (i=0;i<width*height;i++) {
		double Y  = (0.299 * _CR) + (0.587 * _CG) + (0.114 * _CB);
		if (Y<0) dst->y[i]=0;
		else if (Y>255) dst->y[i]=255;
		else dst->y[i]=(uint8_t) floor(Y+.5);
#if 1
		if (i%2==0 && ((i/width)%2)==0 && i < (width-1)*height) { 
            double V =  (0.500 * _CRX) - (0.419 * _CGX) - (0.081 * _CBX) + 128;
            double U = -(0.169 * _CRX) - (0.331 * _CGX) + (0.500 * _CBX) + 128;
			
            if (U<0) dst->u[c]=0;
            else if (U>255) dst->u[c]=255;
            else dst->u[c]=(uint8_t) floor(U+.5);
			
            if (V<0) dst->v[c]=0;
            else if (V>255) dst->v[c]=255;
            else dst->v[c]=(uint8_t) floor(V+.5);
            c++;
		}
#endif
    }
    return 1;
}

void encoder_example_init(int inW, int inH, int inFramerate, int in_video_r, int in_video_q) {
    encoder_init(inW, inH, inFramerate, in_video_r, in_video_q);
    myGlob_yuv.y= (uint8_t*) malloc(inW*inH*sizeof(uint8_t)*3/2);
    myGlob_yuv.u= myGlob_yuv.y + video_x*video_y;
    myGlob_yuv.v= myGlob_yuv.u + video_x*video_y/4;
}

int encoder_example_loop( CVPixelBufferRef theBuffer ) {
    size_t pxwidth = CVPixelBufferGetWidth(theBuffer) ;
    size_t pxheight = CVPixelBufferGetHeight(theBuffer) ;
    CVPixelBufferLockBaseAddress(theBuffer, 0);
    int err = yuv_copy__argb_to_420(
	CVPixelBufferGetBaseAddress(theBuffer),
	CVPixelBufferGetBytesPerRow(theBuffer),
	pxwidth, pxheight,
	frame_x_offset, frame_y_offset,
	&myGlob_yuv);

    CVPixelBufferUnlockBaseAddress( theBuffer, 0 );
    encoder_loop(myGlob_yuv.y);
}

int encoder_example_end() {
    encoder_close();
    if (myGlob_yuv.y) free(myGlob_yuv.y);
}
#endif

