#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


//--------------------------------------------
// Manage video file (encoding)
//--------------------------------------------

FILE *outputfile = NULL;
int outbuf_size = 0;
uint8_t *outbuf;
AVCodecContext    *outCodecCtx;

int        movie_width;
int        movie_height;
int        render_fmt = PIX_FMT_YUV420P; // PIX_FMT_ARGB;

int open_output_file(const char *file_name, int inW, int inH, int inFramerate) {
  outbuf_size = 1024*1024;
  int bitrate = 512 * 1024;
  movie_width = inW;
  movie_height= inH;

  AVCodec *codec;

//codec = avcodec_find_encoder(CODEC_ID_MPEG4);
  codec = avcodec_find_encoder(CODEC_ID_MPEG2VIDEO);
//codec = avcodec_find_encoder(CODEC_ID_XVID);
//codec = avcodec_find_encoder(CODEC_ID_DVVIDEO);
//codec = avcodec_find_encoder(CODEC_ID_XVID);
//codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);

  outCodecCtx= avcodec_alloc_context();

  outCodecCtx->bit_rate = bitrate;
  outCodecCtx->rc_buffer_size = outbuf_size;
#if 0
  outCodecCtx->bit_rate_tolerance = bitrate/2;
  outCodecCtx->rc_min_rate = bitrate/4;
  outCodecCtx->rc_max_rate = bitrate*4;
#else
  outCodecCtx->rc_min_rate = bitrate;
  outCodecCtx->rc_max_rate = bitrate;
#endif
  outCodecCtx->sample_aspect_ratio = (AVRational){1,1};
  outCodecCtx->width = movie_width;
  outCodecCtx->height = movie_height;
  outCodecCtx->time_base= (AVRational){1,inFramerate};
  outCodecCtx->gop_size = 30;
//outCodecCtx->max_b_frames=1;
  outCodecCtx->pix_fmt = render_fmt;
  outCodecCtx->flags |= CODEC_FLAG_QSCALE;
//outCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

  if (avcodec_open(outCodecCtx, codec) < 0) {
    fprintf(stderr, "could not initiate encoder codec.\n");
    return -1;
  }

  outputfile = fopen(file_name, "wb");
  if(outputfile == NULL) {
    fprintf(stderr, "Couldn't open outputfile: %s\n", file_name);
    return -1;
  }
  outbuf = (uint8_t *) malloc(outbuf_size);
  return 0;
}


void encode_frame (AVFrame *picture) {
  static int max_size=0;
  int out_size;
  if (outputfile == NULL) return;
  out_size = avcodec_encode_video(outCodecCtx, outbuf, outbuf_size, picture);
  fwrite(outbuf, 1, out_size, outputfile);
  if (out_size>max_size) {
    max_size=out_size;
    if (100.0*out_size/outbuf_size > 50)
      fprintf(stderr, "\nmax buf : %i/%i (%.1f%%)\n",out_size,outbuf_size,100.0*out_size/outbuf_size);
  }
}

#include <theora/theora.h> // we use theoras yuv_buffer & oggenc's argb_to_420 method
int yuv_copy__argb_to_420(void *b_rgb, size_t b_rgb_stride, size_t width, size_t height, size_t offset_x, size_t offset_y, yuv_buffer *dst);
AVFrame* picture = NULL;

void encode_buffer(uint8_t *buffer) { 

  if (!picture) {

    picture= avcodec_alloc_frame();

    int size = movie_width * movie_height;
    uint8_t* picture_buf = malloc((size * 3) / 2); /* size for YUV 420 */

    picture->data[0] = picture_buf;
    picture->data[1] = picture->data[0] + size;
    picture->data[2] = picture->data[1] + size / 4;
    picture->linesize[0] = movie_width;
    picture->linesize[1] = movie_width / 2;
    picture->linesize[2] = movie_width / 2;
  }
  
  yuv_buffer myYUV;
  myYUV.y= picture->data[0];
  myYUV.u= picture->data[1];
  myYUV.v= picture->data[2];
  yuv_copy__argb_to_420(buffer, movie_width*4, movie_width, movie_height, 0, 0, & myYUV);

  //memcpy(picture->data[0], buffer, (movie_width * movie_height *3) /2);

  encode_frame(picture);
}

void close_output_file(void) {
  if (outputfile == NULL) return;
  int out_size;
  do {
    out_size = avcodec_encode_video(outCodecCtx, outbuf, outbuf_size, NULL);
    fwrite(outbuf, 1, out_size, outputfile);
  } while(out_size);

#if 0 // MPEG container only
  outbuf[0] = 0x00;
  outbuf[1] = 0x00;
  outbuf[2] = 0x01;
  outbuf[3] = 0xb7;
#endif

  fwrite(outbuf, 1, 4, outputfile);
  free(outbuf);
  fclose(outputfile);
  if (picture) { 
    free(picture->data[0]);
    free(picture); // av_free_frame();
  }
  picture=NULL;
}
