#ifndef INOUTENCODER_H
#define INOUTENCODER_H

int myOggfwd_init( const char* outIceIp, int outIcePort, const char* outPassword, const char* outIceMount,
		   const char *description, const char *genre, const char *name, const char *url );
void myOggfwd_close() ;
void theora_enc_init(int inW, int inH, int inFramerate, int in_video_r, int in_video_q) ;
int theora_enc_loop( CVPixelBufferRef imBuffRef ) ;
int theora_enc_end() ;

#endif

