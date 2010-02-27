
#include <encoder_example.h>

// InOut Hi, i'm using globals, not good :

ogg_stream_state myGlob_to; /* take physical pages, weld into a logical stream of packets */
ogg_stream_state myGlob_vo; /* take physical pages, weld into a logical stream of packets */
ogg_page         myGlob_og; /* one Ogg bitstream page.  Vorbis packets are inside */
ogg_packet       myGlob_op; /* one raw packet of data for decode */

theora_state     myGlob_td;
theora_info      myGlob_ti;
theora_comment   myGlob_tc;

vorbis_info      myGlob_vi; /* struct that stores all the static vorbis bitstream settings */
vorbis_comment   myGlob_vc; /* struct that stores all the user comments */

vorbis_dsp_state myGlob_vd; /* central working state for the packet->PCM decoder */
vorbis_block     myGlob_vb; /* local working space for packet->PCM decode */

FILE* outfile ; /// InOut file where im writing for tests
FILE* testOutFile ;

size_t audio_bytesout=0;
size_t video_bytesout=0;
int audioflag=0;
int videoflag=0;
int akbps=0;
int vkbps=0;
	
int myGlob_state ; // For allocating memory at the beginning

/* You'll go to Hell for using globals. */

FILE *audio=NULL;
FILE *video=NULL;

int audio_ch=0;
int audio_hz=0;

float audio_q=.1;
int audio_r=-1;

int video_x=0;
int video_y=0;
int frame_x=0;
int frame_y=0;
int frame_x_offset=0;
int frame_y_offset=0;

int noise_sensitivity=1;
int sharpness=0;
int keyframe_frequency=1;

/*
int yuv_copy__422_to_420(void *b_2vuy, SInt32 b_2vuy_stride, size_t width, size_t height, size_t offset_x, size_t offset_y, yuv_buffer *dst)
{
    UInt8 *base = b_2vuy;
    size_t off_x = offset_x & ~0x01, off_y = offset_y & ~0x01;
    size_t off_x2 = offset_x >> 1, off_y2 = offset_y >> 1;
    UInt8 *y_base = dst->y + off_y * dst->y_stride + off_x;
    UInt8 *cb_base = dst->u + off_y2 * dst->uv_stride + off_x2;
    UInt8 *cr_base = dst->v + off_y2 * dst->uv_stride + off_x2;
    size_t x, y;
	
    for (y = 0; y < height; y += 2) {
        UInt8 *b_top = base;
        UInt8 *b_bot = base + b_2vuy_stride;
        UInt8 *y_top = y_base;
        UInt8 *y_bot = y_base + dst->y_stride;
        UInt8 *cb = cb_base;
        UInt8 *cr = cr_base;
        for (x = 0; x < width; x += 2) {
            *cb++ = (UInt8) (((UInt16) *b_top++ + (UInt16) *b_bot++) >> 1);
            *y_top++ = *b_top++;
            *y_bot++ = *b_bot++;
            *cr++ = (UInt8) (((UInt16) *b_top++ + (UInt16) *b_bot++) >> 1);
            *y_top++ = *b_top++;
            *y_bot++ = *b_bot++;
        }
        base += 2 * b_2vuy_stride;
        y_base += 2 * dst->y_stride;
        cb_base += dst->uv_stride;
        cr_base += dst->uv_stride;
    }
	
    return 1;
}

int yuv_copy__argb_to_420(void *b_rgb, SInt32 b_rgb_stride, size_t width, size_t height, size_t offset_x, size_t offset_y, yuv_buffer *dst)
{
    UInt8 *base = b_rgb;

    UInt8 *y_base = dst->y;
    UInt8 *cb_base = dst->u;
    UInt8 *cr_base = dst->v;
    size_t x, y;
	
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
	    int yoff  =(x+dst->y_stride*y);
	    int rgboff=b_rgb_stride*y + (x<<2); // x*4;

	    register int R = base[rgboff+1];
	    register int G = base[rgboff+2];
	    register int B = base[rgboff+3];

	   // TODO: clamp values to [0..255]
	    y_base[yoff]   = (0.299 * R) + (0.587 * G) + (0.114 * B);
	    if((x%2) ==0 && (y%2)==0) {
  		// TODO: average 2x2 pixels  RGB.
            	int uvoff =((x>>1)+dst->uv_stride*(y>>1));
		cb_base[uvoff] = -(0.169 * R) - (0.331 * G) + (0.500 * B) + 128;
		cr_base[uvoff] =  (0.500 * R) - (0.419 * G) - (0.081 * B) + 128;
            }
        }
    }
    return 1;
}
*/

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



int fetch_and_process_video(CVPixelBufferRef theBuffer,ogg_page *videopage,
                            ogg_stream_state *to,
                            theora_state *td,
                            int localVideoflag){
	
	//printf("fetch_and_process_video()\n") ;
	
	
	/* You'll go to Hell for using static variables */
	static unsigned char *myGlob_yuvframe;
	yuv_buffer          myGlob_yuv;
	ogg_packet          op;
	
	  if(myGlob_state==-1){
	        /* initialize the double frame buffer */
		//printf("INOUT - initialize the double frame buffer...\n") ;
		
	    //myGlob_yuvframe = new unsigned char[video_x*video_y*3/2];
		myGlob_yuvframe = malloc(video_x*video_y*3/2);
	    //myGlob_yuvframe[1]=malloc(video_x*video_y*3/2);
		
		//printf("INOUT - initialize the double frame buffer DONE...\n") ;
	
	        /* clear initial frame as it may be larger than actual video data */
			/* fill Y plane with 0x10 and UV planes with 0X80, for black data */
		
	    memset(myGlob_yuvframe,0x10,video_x*video_y);
	    memset(myGlob_yuvframe+video_x*video_y,0x80,video_x*video_y/2);
//	    memset(myGlob_yuvframe[1],0x10,video_x*video_y);
//	    memset(myGlob_yuvframe[1]+video_x*video_y,0x80,video_x*video_y/2);
			  
	    myGlob_state=10;
	  }
	
	//InOut //printf("init AA\n") ;
	
	/* is there a video page flushed?  If not, work until there is. */
	//printf("Working...\n") ;

	while(!localVideoflag) {
		
		//printf("%d\n",workI++) ;
		//("encoder_example - INSIDE FETCH VIDEO loop\n", video_x) ;
	
		if(ogg_stream_pageout(to,videopage)>0) return 1;
		if(ogg_stream_eos(to)) { printf("boumboumboum (stream EOS)\n") ;return 0;}
	
		{
		/* read and process more video */
		/* video strategy reads one frame ahead so we know when we're
		at end of stream and can mark last video frame as such
		(vorbis audio has to flush one frame past last video frame
         due to overlap and thus doesn't need this extra work */
		
		/* have two frame buffers full (if possible) before
		proceeding.  after first pass and until eos, one will
		always be full when we get here */
	
		if(myGlob_state<1){
			/* can't get here unless YUV4MPEG stream has no video */
			fprintf(stderr,"Video input contains no frames.\n");
			exit(1);
		}
	  
		/* Theora is a one-frame-in,one-frame-out system; submit a frame
		for compression and pull out the packet */

		      {
				//printf("- (juste pour voir) video_x : %d \n", video_x) ;
		        myGlob_yuv.y_width= video_x;
		        myGlob_yuv.y_height= video_y;
		        myGlob_yuv.y_stride= video_x;
		
		        myGlob_yuv.uv_width=video_x/2;
		        myGlob_yuv.uv_height=video_y/2;
		        myGlob_yuv.uv_stride=video_x/2;
	
		        myGlob_yuv.y= &myGlob_yuvframe[0];
		        myGlob_yuv.u= &myGlob_yuvframe[0] + video_x*video_y;
		        myGlob_yuv.v= &myGlob_yuvframe[0] + video_x*video_y*5/4 ;
		      }
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// InOut myencoding Test
				
		//if(theBuffer==NULL) printf("hum \n") ;
		
		size_t pxwidth = CVPixelBufferGetWidth(theBuffer) ;
		size_t pxheight = CVPixelBufferGetHeight(theBuffer) ;
		//printf("DIMENSIONS DU CVPIXELBUFFER :%d,%d\n" ,pxwidth,pxheight) ;
		
		CVPixelBufferLockBaseAddress(theBuffer, 0);
		
//		printf("going to yuv CONVERT %ix%i+%i+%i (%i)\n", pxwidth, pxheight,myGlob_ti.offset_x,myGlob_ti.offset_y, CVPixelBufferGetBytesPerRow(theBuffer)) ;
						
		// Transfer the source frame into glob->currentFrame, converting it from chunky YUV422 to planar YUV420.
		int err = yuv_copy__argb_to_420(CVPixelBufferGetBaseAddress(theBuffer),
									   CVPixelBufferGetBytesPerRow(theBuffer),
									   pxwidth, pxheight,
									   myGlob_ti.offset_x,myGlob_ti.offset_y,
									   &myGlob_yuv);
		
//		printf("yuv convert done\n") ;
		
		CVPixelBufferUnlockBaseAddress( theBuffer, 0 );
		
		//printf("cv pixel unlock base adress unlocked\n") ;
		
		///////////////// InOut debug tests yuv :
		
//		printf("YUV_BUFFER : ywidth:%d yheight:%d ystride:%d uvwidth:%d uvheight:%d uvstride:%d\n",myGlob_yuv.y_width,myGlob_yuv.y_height,myGlob_yuv.y_stride,myGlob_yuv.uv_width,myGlob_yuv.uv_height,myGlob_yuv.uv_stride) ;
//		//printf("Y : %s\n", myGlob_yuv.y) ;
//		printf("size of y : %i\n", strlen(myGlob_yuv.y)) ;
//		int outr = fwrite(myGlob_yuv.y,1,strlen(myGlob_yuv.y),testOutFile);
//		fclose(testOutFile) ;
//		printf("OK file test closed (%d)\n",outr) ;
//		//fwrite("\n\n\n",1,3,testOutFile);
//		break ;
		
		
		int result = theora_encode_YUVin(td, &myGlob_yuv);
		//printf("pb? encodeYUV O success : %d\n", result);
		//printf("theora encode OK\n") ;
		
		result = theora_encode_packetout(td, 0, &op);
		//printf("pb? encodePACKETout 1 sucess : %d\n", result);
		//printf("theora pachet OUT OK\n") ;
				
		ogg_stream_packetin(to,&op);
		
		{
//			unsigned char *temp=myGlob_yuvframe[0];
//			myGlob_yuvframe[0]=myGlob_yuvframe[1];
//			myGlob_yuvframe[1]=temp;
//			state--;
		}
		
		}
	}
	//printf("END of fetch and process VIDEO\n") ;
	return localVideoflag;
}

shout_t     *myShout;
//FILE		*outTestFile ;
int myOggfwd_init( const char* outIceIp, int outIcePort, const char* outPassword, const char* outIceMount,
	           const char *description, const char *genre, const char *name, const char *url ) {
	
	int res = 0 ;
	
	unsigned short	port = outIcePort ;
	
	if ((myShout = shout_new()) == NULL) printf("oggfwd - Error - allocate shouter\n") ;
	if (shout_set_host(myShout, outIceIp) != SHOUTERR_SUCCESS) printf("oggfwd - Error - set host pb...%s\n",shout_get_error(myShout)) ;
	if (shout_set_port(myShout, port) != SHOUTERR_SUCCESS) printf("oggfwd - Error - look at the code to know...\n") ;
	if (shout_set_password(myShout, outPassword) != SHOUTERR_SUCCESS) printf("oggfwd - Error - look at the code to know...\n") ;
	if (shout_set_mount(myShout, outIceMount) != SHOUTERR_SUCCESS) printf("oggfwd - Error - look at the code to know...\n") ;
	shout_set_format(myShout, SHOUT_FORMAT_VORBIS);
	shout_set_public(myShout, 1);
	
	if (description)	shout_set_description(myShout, description);
	if (genre)		shout_set_genre(myShout, genre);
	if (name)		shout_set_name(myShout, name);
	if (url)		shout_set_url(myShout, url);
	
	//printf("oggfwd - Trying to connect\n") ;
	
	if (shout_open(myShout) == SHOUTERR_SUCCESS) {
		printf("INOUT - oggfwd - Connected to server (%s)\n",outIceIp) ;
		res = 1 ;
	}
	else {
		printf("INOUT - oggfwd - Error connecting server (%s)\n",outIceIp) ;
	}
	
	//outTestFile = fopen("/OUT_oggFwdOutVideo.ogg", "wb") ;
	return res ;
}
void encoder_example_init(int inW, int inH, int inFramerate, int in_video_r, int in_video_q) // int argc,char *const *argv)
{	
	//printf("encoder_example INIT function\n");	
	//testOutFile = fopen("OUT_testOutFile.txt","wb") ;
	//outfile=fopen("OUT_outVideo.ogg","wb");
	
	printf("INOUT - Encoder Video Size: %dx%d\n",inW,inH) ;
	frame_x = inW ;
	frame_y = inH ;
	myGlob_state = -1 ;
		
#ifdef _WIN32 
# ifdef THEORA_PERF_DATA
    LARGE_INTEGER start_time;
    LARGE_INTEGER final_time;
	
    LONGLONG elapsed_ticks;
    LARGE_INTEGER ticks_per_second;
    
    LONGLONG elapsed_secs;
    LONGLONG elapsed_sec_mod;
    double elapsed_secs_dbl ;
# endif
	
	/* We need to set stdin/stdout to binary mode. Damn windows. */
	/* if we were reading/writing a file, it would also need to in
		binary mode, eg, fopen("file.wav","wb"); */
	/* Beware the evil ifdef. We avoid these where we can, but this one we
		cannot. Don't add any more, you'll probably go to hell if you do. */
	_setmode( _fileno( stdin ), _O_BINARY );
	_setmode( _fileno( stdout ), _O_BINARY );
#endif
	
#ifdef THEORA_PERF_DATA
# ifdef WIN32
    QueryPerformanceCounter(&start_time);
# endif
#endif
	
	
	/* yayness.  Set up Ogg output stream */
	srand(time(NULL));
	{
		/* need two inequal serial numbers */
		int serial1, serial2;
		serial1 = rand();
		serial2 = rand();
		if (serial1 == serial2) serial2++;
		ogg_stream_init(&myGlob_to,serial1);
		ogg_stream_init(&myGlob_vo,serial2);
	}
	
	
	/* Theora has a divisible-by-sixteen restriction for the encoded video size */
	/* scale the frame size up to the nearest /16 and calculate offsets */
	video_x=((frame_x + 15) >>4)<<4;
	video_y=((frame_y + 15) >>4)<<4;
	/* We force the offset to be even.
		This ensures that the chroma samples align properly with the luma
		samples. */
	frame_x_offset=((video_x-frame_x)/2)&~1;
	frame_y_offset=((video_y-frame_y)/2)&~1;
	
	theora_info_init(&myGlob_ti);
	myGlob_ti.width= video_x;
	myGlob_ti.height= video_y;
	myGlob_ti.frame_width= frame_x;
	myGlob_ti.frame_height= frame_y;
	myGlob_ti.offset_x= frame_x_offset;
	myGlob_ti.offset_y= frame_y_offset;
	myGlob_ti.fps_numerator= inFramerate ; //video_hzn;
	myGlob_ti.fps_denominator= 1 ; //video_hzd;
	myGlob_ti.aspect_numerator= 1 ; //video_an;
	myGlob_ti.aspect_denominator= 1 ; //video_ad;
	myGlob_ti.colorspace=OC_CS_UNSPECIFIED;
	myGlob_ti.pixelformat=OC_PF_420;
	myGlob_ti.target_bitrate= in_video_r ;
	myGlob_ti.quality= in_video_q ;
	
	myGlob_ti.dropframes_p=0;
	myGlob_ti.quick_p=1;
	myGlob_ti.keyframe_auto_p=1;
	myGlob_ti.keyframe_frequency=keyframe_frequency;
	myGlob_ti.keyframe_frequency_force=keyframe_frequency;
	myGlob_ti.keyframe_data_target_bitrate= myGlob_ti.target_bitrate *1.5;
	myGlob_ti.keyframe_auto_threshold=80;
	myGlob_ti.keyframe_mindistance=8;
	myGlob_ti.noise_sensitivity= noise_sensitivity ;
	myGlob_ti.sharpness= sharpness;
	
	theora_encode_init(&myGlob_td,&myGlob_ti);
	theora_info_clear(&myGlob_ti);

	
	//////////////////////////////////////////////////////////
	/* write the bitstream header packets with proper page interleave */
	
	/* first packet will get its own page automatically */
	theora_encode_header(&myGlob_td,&myGlob_op);
	ogg_stream_packetin(&myGlob_to,&myGlob_op);
	if(ogg_stream_pageout(&myGlob_to,&myGlob_og)!=1){
		fprintf(stderr,"Internal Ogg library error.\n");
		exit(1);
	}
	
	/////////////////////////////////////////////////////////////
	// InOut - writing header in outFile
//	int poaResult = fwrite(myGlob_og.header,1,myGlob_og.header_len,outfile);
//	poaResult += fwrite(myGlob_og.body,1,myGlob_og.body_len,outfile);
//	fprintf(stderr,"HEADER ///////InOut fwrite header length: %d\n",myGlob_og.header_len);
//	fprintf(stderr,"HEADER ///////InOut fwrite body length: %d\n",myGlob_og.body_len);
//	fprintf(stderr,"HEADER ///////InOut fwrite header: %s\n",myGlob_og.header);
//	fprintf(stderr,"HEADER ///////InOut fwrite body: %s\n",myGlob_og.body);
//	fprintf(stderr,"HEADER InOut fwrite header result: %d\n",poaResult);
	
	/////////////////////////////////////////////////////////////
	// S2 - InOut - sending header to icecast
	int fwdResH1 = myOggfwd_process(myGlob_og) ;
	//printf("HEADER /////// Forwarded to Icecast : success ? %d\n", fwdResH1) ;
	
	////////////////////////////////////////////////////////////
	/* create the remaining theora headers */
	theora_comment_init(&myGlob_tc);
	theora_encode_comment(&myGlob_tc,&myGlob_op);
	ogg_stream_packetin(&myGlob_to,&myGlob_op);
	/*theora_encode_comment() doesn't take a theora_state parameter, so it has to
		allocate its own buffer to pass back the packet data.
		If we don't free it here, we'll leak.
		libogg2 makes this much cleaner: the stream owns the buffer after you call
		packetin in libogg2, but this is not true in libogg1.*/
	free(myGlob_op.packet);
	theora_encode_tables(&myGlob_td,&myGlob_op);
	ogg_stream_packetin(&myGlob_to,&myGlob_op);
	
	/* Flush the rest of our headers. This ensures
		the actual data in each stream will start
		on a new page, as per spec. */
	while(1){
		int result = ogg_stream_flush(&myGlob_to,&myGlob_og);
		if(result<0){
			/* can't get here */
			fprintf(stderr,"Internal Ogg library error.\n");
			exit(1);
		}
		if(result==0) break ;
		
		/////////////////////////////////////////////////////////////
		// InOut - writing header in outFile
//		fwrite(myGlob_og.header,1,myGlob_og.header_len,outfile);
//		fwrite(myGlob_og.body,1,myGlob_og.body_len,outfile);
//		fprintf(stderr,"HEADER ////////InOut V lg header: %d\n",myGlob_og.header_len);
//		fprintf(stderr,"HEADER ////////InOut V lg body: %d\n",myGlob_og.body_len);
//		fprintf(stderr,"HEADER ////////InOut header: %s\n",myGlob_og.header);
//		fprintf(stderr,"HEADER ////////InOut body: %s\n",myGlob_og.body);
		
		/////////////////////////////////////////////////////////////
		// S3 - InOut - sending to icecast
		int fwdResH2 = myOggfwd_process(myGlob_og) ;
		//printf("HEADERs /////// Forwarded to Icecast : success ? %d\n", fwdResH2) ;
		
	}
	
	fprintf(stderr,"INOUT - ENCODER_EXAMPLE_INIT done ! (framerate=%d,bitrate=%d,quality=%d)\n",inFramerate,in_video_r,in_video_q);
	
	/* setup complete.  Raw processing loop */
	//InOut done outside from mycontroller...
}


int encoder_example_loop( CVPixelBufferRef imBuffRef ) {
	
	//fprintf(stderr,"encore F\n") ;
	//if(imBuffRef==NULL) printf("AVT copie - de toute facon ca va pas marcher \n") ;
	//if(sourcePixelBuffer!=NULL) sourcePixelBuffer=NULL ;
	//if(sourcePixelBuffer==NULL) printf("APRES COPIE - de toute facon ca va pas marcher \n") ;	
	//fprintf(stderr,"ENCODER_EXAMPLE_LOOP : got the buffer from controller, going to Compress....\n");
	//fprintf(stderr,"ENCODER_EXAMPLE_LOOP : video : %d audio : %d\n",video, audio);
	
    ogg_page videopage;
	double timebase;
	
    /* is there a video page flushed?  If not, fetch one if possible */
    videoflag=fetch_and_process_video(imBuffRef,&videopage,&myGlob_to,&myGlob_td,videoflag);
	
    /* no pages of either?  Must be end of stream. */
    if(!videoflag) {
		printf("ENCODER_LOOP - Must be end of stream because i don't have any frame !!\n") ;
		return 0;
	}
	
    {     
	double videotime=videoflag?theora_granule_time(&myGlob_td,ogg_page_granulepos(&videopage)):-1;

      if(videoflag){
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// InOut - flush page on disk			
//			//printf("ENCODER_LOOP - going to output frame result (videopage/header+body)\n") ;
//			//fprintf(stderr,"Flush Video Page.\n\n");
//			/* flush a video page */
//			//fprintf(stderr,"LOOP /////// videopage header (%d) : %s\n",videopage.header_len,videopage.header);
//			//fprintf(stderr,"LOOP /////// videopage body (%d) : %s\n",videopage.body_len,videopage.body);
//			video_bytesout+=fwrite(videopage.header,1,videopage.header_len,outfile);
//			video_bytesout+=fwrite(videopage.body,1,videopage.body_len,outfile);
//			fprintf(stderr,"LOOP /////// Total written on disk : videobytes : %d\n",video_bytesout);
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// S1 - InOut - forward page to myOggfwd
			int fwdRes = myOggfwd_process(videopage) ;
			//printf("LOOP /////// Forwarded to Icecast : success ? %d\n", fwdRes) ;
			////////////////////////////////////////////////////////////////////////////////////
			videoflag=0;
			timebase=videotime;
			
		}
    }
	return 1 ;
}


int myOggfwd_process(ogg_page inputVideoPage) {
	int ret ;
	////////////////////////////// TESTING SIMPLE WRITE ON FILE ... /////////////////////
	//	int video_bytesout = 0 ;
	//	video_bytesout+=fwrite(inputVideoPage.header,1,inputVideoPage.header_len,outTestFile);
	//	video_bytesout+=fwrite(inputVideoPage.body,1,inputVideoPage.body_len,outTestFile);
	//	printf("[TESTFILE out of oggfwd] Written %d bytes on disk \n", video_bytesout) ;
	////////////////////////////////////////////////////////////////////////////////////
	
	//printf("going to OGGFWD TO ICECAST\n") ;
	
	if (inputVideoPage.header_len > 0 && inputVideoPage.body_len > 0) {
		ret = shout_send(myShout, inputVideoPage.header, inputVideoPage.header_len);
		ret = shout_send(myShout, inputVideoPage.body, inputVideoPage.body_len);
		//printf("The oggfwd sent process return : %d\n", ret) ;
		if(ret != SHOUTERR_SUCCESS) {
			printf("!!!!!!! oggfwd - PB (error : %s) \n",shout_get_error(myShout)) ;
			return 0 ;
		}
		//if(shout_queuelen(myShout) > 0) printf("DEBUG: queue length: %d\n", shout_queuelen(myShout));
	} 
	else {
		printf("!!!!!!!! oggfwd - PB (buffer empty)\n") ;
		return 0 ;
	}
	
	shout_sync(myShout); // Sleep until the server will be ready for more data.
	return 1 ;
}
void myOggfwd_close() {
	
	//if(outTestFile) fclose(outTestFile) ;	
	shout_close(myShout);
	printf("INOUT - oggfwd - Shout closed - Done\n") ;
}

int encoder_example_end() {
	
	/* clear out state */
	
	if(audio){
		ogg_stream_clear(&myGlob_vo);
		vorbis_block_clear(&myGlob_vb);
		vorbis_dsp_clear(&myGlob_vd);
		vorbis_comment_clear(&myGlob_vc);
		vorbis_info_clear(&myGlob_vi);
	}
	if(video){
		fprintf(stderr,"Clearing video state\n");
		ogg_stream_clear(&myGlob_to);
		theora_clear(&myGlob_td);
	}
	
	if(outfile && outfile!=stdout) fclose(outfile);
	
	fprintf(stderr,"INOUT - encoder - Done.\n\n");
	
#ifdef THEORA_PERF_DATA
# ifdef WIN32
    QueryPerformanceCounter(&final_time);
    elapsed_ticks = final_time.QuadPart - start_time.QuadPart;
    ticks_per_second;
    QueryPerformanceFrequency(&ticks_per_second);
    elapsed_secs = elapsed_ticks / ticks_per_second.QuadPart;
    elapsed_sec_mod = elapsed_ticks % ticks_per_second.QuadPart;
    elapsed_secs_dbl = elapsed_secs;
    elapsed_secs_dbl += ((double)elapsed_sec_mod / (double)ticks_per_second.QuadPart);
    printf("Encode time = %lld ticks\n", elapsed_ticks);
    printf("~%lld and %lld / %lld seconds\n", elapsed_secs, elapsed_sec_mod, ticks_per_second.QuadPart);
    printf("~%Lf seconds\n", elapsed_secs_dbl);
# endif
	
#endif 
	
	return(0);
	
}
