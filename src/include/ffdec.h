#include <stdint.h>  
/* prototypes of fn in ffdec.c */

typedef struct ffdec ffdec_t;

void init_ffmpeg();
void init_moviebuffer(ffdec_t *ffp, int width, int height, int render_fmt);
void limit_size(ffdec_t *ffp);
void free_moviebuffer(ffdec_t *ffp);
void free_ff(ffdec_t **ffpx);
void close_and_free_ff(ffdec_t **ffpx);
int  open_movie(ffdec_t **ffpx, char* file_url);
int  open_camera(ffdec_t ** ffpx, char* device);
int  open_ffmpeg(ffdec_t ** ffpx, char* file);
int  decode_frame(ffdec_t *ffp);
void close_movie(ffdec_t *ffp);

int      get_scaled_width(ffdec_t *ffp);
int      get_scaled_height(ffdec_t *ffp);
int      get_width(ffdec_t *ffp);
int      get_height(ffdec_t *ffp);
double   get_fps(ffdec_t *ffp);
uint8_t *get_bufptr(ffdec_t *ffp);

void init_letterbox(uint8_t *d, int w, int h);
void yuv_letterbox(void *dst, const void *src, int src_width, int src_height, int out_width, int out_height);
void calc_letterbox(int src_w, int src_h, int out_w, int out_h, int *sca_w, int *sca_h);
void select_sleep (int usec);


void yuv_memcpy(void *dst, const void *src, int src_width, int src_height, int out_width, int out_height, int xoff, int yoff);
void stride_memcpy(void *dst, const void *src, int width, int height, int dstStride, int srcStride);



int get_pt_status(ffdec_t *ffp);
int ffdec_thread(ffdec_t **ffpx, char *movie_url, int w, int h, int render_fmt);
