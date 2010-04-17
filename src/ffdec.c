#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <ffdec.h>

int want_quiet =1;

struct ffdec {
  int               videoStream;
  AVFormatContext   *pFormatCtx;
  AVCodecContext    *pCodecCtx;
  AVFrame           *pFrame;
  AVFrame           *pFrameFMT;
  struct SwsContext *pSWSCtx;
  AVPacket 	    packet;
  int               fFirstTime;

  int               ff_width;
  int               ff_height;
  double            ff_fps;
  uint8_t           *buffer;
  int               sc_width;
  int               sc_height;

  int               pt_status; // status/mode  for pthread
} ffdec;

//--------------------------------------------
// Manage video file (reading)
//--------------------------------------------

void init_ffmpeg() {
  av_register_all();
  avcodec_init();
  avcodec_register_all();
//avdevice_register_all();
  if(want_quiet) av_log_set_level(AV_LOG_QUIET);
}

int get_width(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->ff_width;
}

int get_height(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->ff_height;
}

int get_scaled_width(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->sc_width;
}

int get_scaled_height(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->sc_height;
}

double get_fps(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->ff_fps;
}

uint8_t *get_bufptr(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->buffer;
}

int get_pt_status(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  return ff->pt_status;
}

void init_moviebuffer(ffdec_t *ffp, int width, int height, int render_fmt) {
  ffdec_t *ff = (ffdec_t *) ffp;
  int     numBytes;

  ff->pFrameFMT=avcodec_alloc_frame();
  if(ff->pFrameFMT==NULL) {
      fprintf( stderr, "Cannot allocate FMT buffer\n");
      av_free(ff->pFrame);
      exit( -1 );
  }

  if (!want_quiet)
    fprintf(stderr, "scaling %dx%d -> %dx%d\n", ff->pCodecCtx->width, ff->pCodecCtx->height, width, height);

  numBytes=avpicture_get_size(render_fmt, width, height);
  ff->buffer=(uint8_t *) malloc(numBytes);

  avpicture_fill((AVPicture *)ff->pFrameFMT, ff->buffer, render_fmt, width, height);
  ff->pSWSCtx = sws_getContext(ff->pCodecCtx->width, ff->pCodecCtx->height, ff->pCodecCtx->pix_fmt, width, height, render_fmt, SWS_BICUBIC, NULL, NULL, NULL);
  ff->sc_width  = width;
  ff->sc_height = height;
  ff->pt_status |= 1;
  ff->pt_status |= 2;
}

void free_moviebuffer(ffdec_t *ffp) {
  ffdec_t *ff = (ffdec_t *) ffp;
  if (ff->buffer) free(ff->buffer);
}

int open_movie(ffdec_t ** ffpx, char* movie_url) {
  int      i;
  AVCodec *pCodec;
  double   duration;
  long     frames;
  ffdec_t **ffp = (ffdec_t **) ffpx;
  if (! *ffp) *ffp   = (ffdec_t *) calloc(1,sizeof (ffdec_t));
  ffdec_t *ff   = (ffdec_t *) *ffp;

  ff->pt_status=8;
  ff->videoStream=-1;
  ff->fFirstTime=1;
  
  if(av_open_input_file(&ff->pFormatCtx, movie_url, NULL, 0, NULL)!=0) {
    fprintf( stderr, "Cannot open video file: '%s'\n", movie_url);
    ff->pt_status=0;
    return -1;
  }

  if(av_find_stream_info(ff->pFormatCtx)<0) {
    fprintf( stderr, "Cannot find stream information in file %s\n", movie_url);
    ff->pt_status=0;
    return -1;
  }

  if (!want_quiet) dump_format(ff->pFormatCtx, 0, movie_url, 0);

  for(i=0; i<ff->pFormatCtx->nb_streams; i++) {
    if(ff->pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      ff->videoStream=i;
      break;
    }
  }

  if(ff->videoStream==-1) {
    fprintf( stderr, "Cannot find a video stream in file %s\n", movie_url);
    ff->pt_status=0;
    return -1;
  }

  AVStream *av_stream = ff->pFormatCtx->streams[ff->videoStream];
  ff->ff_fps = (double) av_stream->r_frame_rate.num / av_stream->r_frame_rate.den;
  duration = (double) ( (int64_t) ff->pFormatCtx->duration / (int64_t) AV_TIME_BASE);
  frames = (long) (ff->ff_fps * duration);
  
  if (!want_quiet) {
    fprintf(stdout, "original frame rate: %g\n", ff->ff_fps);
    fprintf(stdout, "length in seconds: %g\n", duration);
    fprintf(stdout, "total frames: %ld\n", frames);
  }
  
  ff->pCodecCtx=ff->pFormatCtx->streams[ff->videoStream]->codec;
  
  ff->ff_width = ff->pCodecCtx->width;
  ff->ff_height = ff->pCodecCtx->height;

  ff->sc_width = ff->sc_height = 0;

  pCodec=avcodec_find_decoder(ff->pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf( stderr, "Cannot find a codec for %s\n", movie_url);
    ff->pt_status=0;
    return -1;
  }
  if(avcodec_open(ff->pCodecCtx, pCodec)<0) {
    fprintf( stderr, "Cannot open the codec for file %s\n", movie_url);
    ff->pt_status=0;
    return -1;
  }
  
  ff->pFrame=avcodec_alloc_frame();
  ff->pt_status=0;
  return 0;
}

int open_camera(ffdec_t ** ffpx, char* device) {
  ffdec_t **ffp = (ffdec_t **) ffpx;
  if (! *ffp) *ffp   = (ffdec_t *) calloc(1,sizeof (ffdec_t));
  ffdec_t *ff   = (ffdec_t *) *ffp;
  ff->videoStream=-1;
  ff->pt_status=0;
  ff->fFirstTime=1;

  AVCodec *pCodec;
  AVFormatParameters vp1, *vp = &vp1;
  memset(vp, 0, sizeof(*vp));
  vp->time_base= ( AVRational ) {1,30};
//vp->width  = 640;
//vp->height = 480;
  vp->width  = 320;
  vp->height = 240;
  
  const char *video_grab_format = "video4linux2";
  AVInputFormat *fmt1;
  fmt1 = av_find_input_format(video_grab_format);
  vp->channel = 0;
  vp->standard = NULL; 
  vp->pix_fmt = PIX_FMT_NONE;
  if (av_open_input_file(&ff->pFormatCtx, device, fmt1, 0, vp) < 0) {
    fprintf(stderr, "Could not find video grab device\n");
    return(-1);
  }

  /* If not enough info to get the stream parameters, we decode the first frames to get it. */
  if ((ff->pFormatCtx->ctx_flags & AVFMTCTX_NOHEADER) && av_find_stream_info(ff->pFormatCtx) < 0) {
    fprintf(stderr, "Could not find video grab parameters\n");
    return(-1);
  }
  /* by now video grab has one stream */
  ff->videoStream=0;
//ff->pFormatCtx->streams[0]->r_frame_rate.num = vp->time_base.den;
//ff->pFormatCtx->streams[0]->r_frame_rate.den = vp->time_base.num;
  if (!want_quiet) dump_format(ff->pFormatCtx, 0, device, 0);


//AVStream *av_stream = ff->pFormatCtx->streams[ff->videoStream];
//ff->ff_fps = (double) av_stream->r_frame_rate.num / av_stream->r_frame_rate.den;

  ff->pCodecCtx=ff->pFormatCtx->streams[ff->videoStream]->codec;
  ff->ff_width  = ff->pCodecCtx->width;
  ff->ff_height = ff->pCodecCtx->height;
  ff->sc_width = ff->sc_height = 0;

  pCodec=avcodec_find_decoder(ff->pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf( stderr, "Cannot find a codec for %s\n", device);
    return -1;
  }
  if(avcodec_open(ff->pCodecCtx, pCodec)<0) {
    fprintf( stderr, "Cannot open the codec for file %s\n", device);
    return -1;
  }
  
  ff->pFrame=avcodec_alloc_frame();
  return 0;
}

int open_ffmpeg(ffdec_t ** ffpx, char* file) {
  if(!strncmp(file, "/dev/", 5)) 
    return open_camera(ffpx, file);
  return open_movie(ffpx, file);
}

int decode_frame(ffdec_t *ffp) {
  ffdec_t *ff = ffp;
  int frameFinished=0;
 
  if(ff->fFirstTime) {
    ff->fFirstTime=0;
    ff->packet.data=NULL;
    avcodec_flush_buffers(ff->pCodecCtx);
  }

  do {  
    if(!frameFinished) {
      if(ff->packet.data) av_free_packet(&ff->packet);
      if(av_read_frame(ff->pFormatCtx, &ff->packet)<0) {
        fprintf(stderr,"End of file reached.\n");
        return 0;
      }
      if (av_dup_packet(&ff->packet) < 0) {
	fprintf(stderr,"Can not allocate packet!\n");
        continue;
      }
    }
    if(ff->packet.stream_index==ff->videoStream)
#if LIBAVCODEC_VERSION_MAJOR < 52 || ( LIBAVCODEC_VERSION_MAJOR == 52 && LIBAVCODEC_VERSION_MINOR < 21)
      avcodec_decode_video(ff->pCodecCtx, ff->pFrame, &frameFinished, ff->packet.data, ff->packet.size);
#else
      avcodec_decode_video2(ff->pCodecCtx, ff->pFrame, &frameFinished, &ff->packet);
#endif
  } while (!frameFinished);

  if(frameFinished) {
    sws_scale(ff->pSWSCtx, (uint8_t**) ff->pFrame->data, ff->pFrame->linesize, 0, ff->pCodecCtx->height, ff->pFrameFMT->data, ff->pFrameFMT->linesize);
  }
  return frameFinished;
}
  
void close_movie(ffdec_t *ffp) {
  ffdec_t *ff = ffp;

  if (ff->pSWSCtx) sws_freeContext(ff->pSWSCtx);
  if (ff->pFrameFMT) av_free(ff->pFrameFMT);
  if (ff->pFrame) av_free(ff->pFrame);
  if (ff->pCodecCtx) avcodec_close(ff->pCodecCtx);
  if (ff->pFormatCtx) av_close_input_file(ff->pFormatCtx);
}

void free_ff(ffdec_t **ffpx) {
  ffdec_t *ff = *ffpx;
  free(ff);
  *ffpx = NULL;
}

void close_and_free_ff(ffdec_t **ffpx) {
  ffdec_t *ff = *ffpx;
#if 0
  int timeout=250;
  while (--timeout && ff->pt_status&10) usleep(20000);// do not free while thread is active
#endif
  if (ff) {
    close_movie(ff);
    free_moviebuffer(ff);
  }
  if (ffpx) {
    free_ff(ffpx);
    ffpx = NULL;
  }
}

void limit_size(ffdec_t *ffp) {
#ifdef FIX_AMOEBA_SIZE
  ff_width=384; ff_height=288;
#else
#define MAX_X (1024)
#define MAX_Y (300)
  ffdec_t *ff = (ffdec_t *) ffp;
  while(ff->ff_width > MAX_X || ff->ff_height > MAX_Y)  {
    if (ff->ff_width > MAX_X)  {
      ff->ff_height = ff->ff_height * MAX_X/ff->ff_width;
      ff->ff_width = MAX_X;
    }

    if (ff->ff_height > MAX_Y)  {
      ff->ff_width = ff->ff_width * MAX_Y/ff->ff_height;
      ff->ff_height = MAX_Y;
    }
  }
#endif
  ff->ff_width=((ff->ff_width + 15) >>4)<<4;
  ff->ff_height=((ff->ff_height + 15) >>4)<<4;
}

// ---------------------------------------------------------------------------

void stride_memcpy(void *dst, const void *src, int width, int height, int dstStride, int srcStride) {
  register int i;
  if(dstStride == srcStride)
    memcpy(dst, src, srcStride*height);
  else for(i=0; i<height; i++) {
    memcpy(dst, src, width);
    src+= srcStride;
    dst+= dstStride;
  }
}


void yuv_memcpy(void *dst, const void *src, int src_width, int src_height, int out_width, int out_height, int xoff, int yoff) {
  int out_size = out_width*out_height;
  int src_size = src_width*src_height;
  stride_memcpy(dst+xoff+out_width*yoff, src, src_width, src_height, out_width, src_width);

  stride_memcpy(dst+ out_size                + (xoff>>1) + ((out_width*yoff)>>2),
                src+ src_size                , src_width>>1, src_height>>1, out_width>>1, src_width>>1);
  stride_memcpy(dst+ out_size + (out_size>>2)+ (xoff>>1) + ((out_width*yoff)>>2),
                src+ src_size + (src_size>>2), src_width>>1, src_height>>1, out_width>>1, src_width>>1);
}



void yuv_letterbox(void *dst, const void *src, int src_width, int src_height, int out_width, int out_height) {
  if (src_width == out_width && src_height == out_height) {
    memcpy(dst, src, src_width*src_height*3/2);
    return;
  }
  // assert(src_width  <= out_width);
  // assert(src_height <= out_height);

  int xoff=((out_width-src_width)>>1)& ~0x01;
  int yoff=((out_height-src_height)>>1)& ~0x01;
  int out_size = out_width*out_height;
  int src_size = src_width*src_height;
  stride_memcpy(dst+xoff+out_width*yoff, src, src_width, src_height, out_width, src_width);

  stride_memcpy(dst+ out_size                + (xoff>>1) + ((out_width*yoff)>>2),
                src+ src_size                , src_width>>1, src_height>>1, out_width>>1, src_width>>1);
  stride_memcpy(dst+ out_size + (out_size>>2)+ (xoff>>1) + ((out_width*yoff)>>2),
                src+ src_size + (src_size>>2), src_width>>1, src_height>>1, out_width>>1, src_width>>1);
}

void init_letterbox(uint8_t *d, int w, int h) {
  int i;
  for (i=0;i<w*h;i++) d[i]=0;
  for (i=0;i<w*h/2;i++) d[w*h+i] = 0x80;
}

void calc_letterbox(int src_w, int src_h, int out_w, int out_h, int *sca_w, int *sca_h) {
  if (src_w*out_h > src_h*out_w) {
    (*sca_w)=out_w;
    (*sca_h)=(int)round((float)out_w*(float)src_h/(float)src_w);
  } else {
    (*sca_h)=out_h;
    (*sca_w)=(int)round((float)out_h*(float)src_w/(float)src_h);
  }
  (*sca_w)=(((*sca_w) + 1) >>1)<<1;
  (*sca_h)=(((*sca_h) + 1) >>1)<<1;
}

/* With select() the timeout is an UPPER bound (it waits at most.. - and
 * uses a signal to wake up), while [nano|u]sleep the time is a LOWER bound
 * (it waits at least for the given time). */
void select_sleep (int usec) {
  struct timeval tv;
  if (usec<0) return;
  if (usec<1000000) {
    tv.tv_sec = 0; 
    tv.tv_usec = usec;
  } else {
    tv.tv_sec  = usec/1000000;
    tv.tv_usec = usec%1000000;
  }
  select(0, NULL, NULL, NULL, &tv);
}


#define THREADED

#ifdef THREADED
#include <pthread.h>

struct fftarg {
  void **ffpx;
  char *movie_url;
  int w,h,render_fmt;
};

void *ffdec_decode_thread(void *ffpx) {
	ffdec_t **ffp = (ffdec_t **) ffpx;
	ffdec_t *ff = (ffdec_t *) *ffp;
	
	if (!decode_frame(ff)) {
		fprintf(stderr,"decoding failed. closing!\n");
		ff->pt_status|=4;
	}
	
	ff->pt_status&=~2;
	return(0);
}

void *ffdec_open_thread(void *arg) {
  struct fftarg *a = (struct fftarg*) arg;
  ffdec_t **ffpx = (ffdec_t **) a->ffpx;
  ffdec_t *ff = *ffpx;
    
  if (open_movie(ffpx, a->movie_url)) {
    //fprintf(stderr,"movie open failed!\n");
    ff->pt_status|=4;
  } else {
    int xw,xh;
#if 0
    calc_letterbox(get_width(ff),get_height(ff),a->w,a->h,&xw,&xh);  // XXX
#else
    xw=a->w; xh=a->h;
#endif
    if (!want_quiet)
	    printf("letterbox: %dx%d\n",xw,xh);
    init_moviebuffer(ff, xw, xh, a->render_fmt);
//  fprintf(stderr,"movie opened: %s!\n",a->movie_url);
    ff->pt_status|=1;
#if 1
    ff->pt_status |= 2;
    //ffdec_decode_thread(ffp);
#endif
  }
  free(a);
  return(0);
}

int ffdec_thread(ffdec_t **ffpx, char *movie_url, int w, int h, int render_fmt) {
  ffdec_t **ffp = (ffdec_t **) ffpx;
  pthread_t thread_id_tt;
  pthread_attr_t thread_att;
  pthread_attr_init(&thread_att);
  pthread_attr_setdetachstate(&thread_att,PTHREAD_CREATE_DETACHED);

  if (*ffp) {
    ffdec_t *ff   = (ffdec_t *) *ffp;
    if ((ff->pt_status & 4 )) {
      close_and_free_ff(ffpx);
      return(-1);
    }
  }

  if (!*ffp && movie_url) {
    struct fftarg *args = malloc(sizeof(struct fftarg));
    args->ffpx=(void *)ffpx;
    args->movie_url=movie_url;
    args->w=w; args->h=h;
    args->render_fmt = render_fmt;
    if (pthread_create(&thread_id_tt, &thread_att, ffdec_open_thread, args)) {
      free(args);
    }
    pthread_attr_destroy(&thread_att);
    return 0;
  }

  if (*ffp) {
    ffdec_t *ff   = (ffdec_t *) *ffp;
    if ((ff->pt_status & 7 ) == 1) {
      ff->pt_status |= 2;
      if (pthread_create(&thread_id_tt, &thread_att, ffdec_decode_thread, ffp)) {
        fprintf(stderr, "thread creation failed.");
        ff->pt_status&=~2;
      }
      pthread_attr_destroy(&thread_att);
      return 0;
    }
    //printf("DEBUG0 %i %i\n", ff->pt_status, render_fmt); 
  }
 return 1;
}

#endif
