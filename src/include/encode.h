
#ifndef __avcodec_h__
#define __avcodec_h__
#include <context.h>
static Context *env;
#ifdef WITH_AVCODEC
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>

#define STREAM_IS_RAW(avformat_context) \
    (avformat_context->oformat->flags & AVFMT_RAWPICTURE)

#define STREAM_IS_YUV420P(codec) \
    (codec->pix_fmt != PIX_FMT_YUV420P)

#define IS_KEYFRAME(codec) \
    (codec->coded_frame->key_frame)

//XXX
#define STREAM_NB_FRAMES 125 
#define MAX_FILE_LENGHT 125 

class Encode {
    private:
	AVStream *video_stream;
	AVFormatContext *afc;
	AVFrame *picture;
	AVOutputFormat *aof;
	AVCodecContext *codec;

	uint8_t *video_outbuf, *tmp_picture;
	int frame_count, video_outbuf_size;
	char *filename;

	void free_av_objects();
	void open();
	void convert_to_YUV420P();
	AVFrame *prepare_image(AVFrame *picture_ptr);
    public:
	Encode(Context *_env);
	~Encode();
	char *init(char *output_filename);
	void set_encoding_parameter();
	bool write_frame();
};

#endif

#endif
