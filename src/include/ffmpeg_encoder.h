
#ifndef __ffmpeg_encoder_h__
#define __ffmpeg_encoder_h__


#include <config.h>
#include <linklist.h>
#include <video_encoder.h>

#ifdef WITH_AVCODEC

#ifdef HAVE_LIBAVCODEC_AVCODEC_H
#   include <libavcodec/avcodec.h>
#elif defined(HAVE_FFMPEG_AVCODEC_H)
#   include <ffmpeg/avcodec.h>
#else
#   include <avcodec.h>
#endif

#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
#   include <libavformat/avformat.h>
#elif defined(HAVE_FFMPEG_AVFORMAT_H)
#   include <ffmpeg/avformat.h>
#else
#   include <avformat.h>
#endif

#define STREAM_IS_RAW(avformat_context) \
    (avformat_context->oformat->flags & AVFMT_RAWPICTURE)

#define STREAM_IS_YUV420P(codec) \
    (codec->pix_fmt != PIX_FMT_YUV420P)

#define IS_KEYFRAME(codec) \
    (codec->coded_frame->key_frame)

//XXX
#define STREAM_NB_FRAMES 125 
#define MAX_FILE_LENGHT 125 

class Context;


class FFmpegEncoder: public VideoEncoder {

 public:
  
  FFmpegEncoder();
  ~FFmpegEncoder();
  
  bool init(Context *_env);
  void set_encoding_parameter();
  bool write_frame();
  
  
 private:
  AVStream *video_stream;
  AVFormatContext *afc;
  AVFrame *picture;
  AVOutputFormat *aof;
  AVCodecContext *codec;
  
  uint8_t *video_outbuf, *tmp_picture;
  int frame_count, video_outbuf_size;
  
  void free_av_objects();
  void open();
  void convert_to_YUV420P();
  AVFrame *prepare_image(AVFrame *picture_ptr);

};

#endif

#endif
