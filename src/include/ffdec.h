#include <stdint.h>  
/* prototypes of fn in ffdec.c */

struct ffdec;

void init_ffmpeg();
void init_moviebuffer(void *ffp, int width, int height, int render_fmt);
void limit_size(void *ffp);
void free_moviebuffer(void *ffp);
void free_ff(void **ffpx);
void close_and_free_ff(void *ffpx);
int  open_movie(void **ffpx, char* file_url);
int  open_camera(void ** ffpx, char* device);
int  open_ffmpeg(void ** ffpx, char* file);
int  decode_frame(void *ffp);
void close_movie(void *ffp);

int      get_scaled_width(void *ffp);
int      get_scaled_height(void *ffp);
int      get_width(void *ffp);
int      get_height(void *ffp);
double   get_fps(void *ffp);
uint8_t *get_bufptr(void *ffp);

void init_letterbox(uint8_t *d, int w, int h);
void yuv_letterbox(void *dst, const void *src, int src_width, int src_height, int out_width, int out_height);
void calc_letterbox(int src_w, int src_h, int out_w, int out_h, int *sca_w, int *sca_h);
void select_sleep (int usec);


void yuv_memcpy(void *dst, const void *src, int src_width, int src_height, int out_width, int out_height, int xoff, int yoff);
void stride_memcpy(void *dst, const void *src, int width, int height, int dstStride, int srcStride);



int get_pt_status(void *ffp);
int ffdec_thread(void **ffpx, char *movie_url, int w, int h, int render_fmt);
