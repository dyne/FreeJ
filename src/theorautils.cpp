/* -*- tab-width:4;c-file-style:"cc-mode"; -*- */
/*
 * theorautils.c - Ogg Theora/Ogg Vorbis Abstraction and Muxing
 * Copyright (C) 2003-2008 <j@v2v.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>


#include <theora/theora.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#ifdef HAVE_OGGKATE
#include <kate/oggkate.h>
#endif

#include <theorautils.h>

#include <config.h>
#include <jutils.h>

static void write_audio_page(oggmux_info *);

int ogg_pipe_read(char *name, ringbuffer_t *rb, char *dest, size_t cnt) {
  // wait until we have enough bytes to read
  while( ringbuffer_read_space(rb) < cnt ) {
    warning("%s pipe read not ready", name);
    jsleep(0,10);    
  }
  return ringbuffer_read(rb, dest, cnt);
}
int ogg_pipe_write(const char *name, ringbuffer_t *rb, char *src, size_t cnt) {
  while( ringbuffer_write_space(rb) < cnt ) {
    std::cerr << name << " pipe write not ready" << std::endl;
    warning("%s pipe write not ready", name);
    jsleep(0,10);
  }
  return ringbuffer_write(rb, src, cnt);
}



static double rint(double x)
{
  if (x < 0.0)
    return (double)(int)(x - 0.5);
  else
    return (double)(int)(x + 0.5);
}

void init_info(oggmux_info *info) {
    info->audio_only = 0;
    info->with_skeleton = 1; /* skeleton is enabled by default    */ 
    info->frontend = NULL; /*frontend mode*/
    info->videotime =  0;
    info->audiotime = 0;
    info->audio_bytesout = 0;
    info->video_bytesout = 0;
    info->kate_bytesout = 0;

    info->videopage_valid = 0;
    info->audiopage_valid = 0;
    info->audiopage_buffer_length = 0;
    info->videopage_buffer_length = 0;
    info->audiopage = NULL;
    info->videopage = NULL;
    info->start_time = time(NULL);
    info->duration = -1;
    info->speed_level = -1;

    info->v_pkg=0;
    info->a_pkg=0;
    info->k_pkg=0;
#ifdef OGGMUX_DEBUG
    info->a_page=0;
    info->v_page=0;
    info->k_page=0;
#endif

    info->with_kate = 0;
    info->n_kate_streams = 0;
    info->kate_streams = NULL;

	memset(info->status, 0x0, 512); // zeroing stats

}

void oggmux_setup_kate_streams(oggmux_info *info, int n_kate_streams)
{
    int n;

    info->n_kate_streams = n_kate_streams;
    info->kate_streams = NULL;
    if (n_kate_streams == 0) return;
    info->kate_streams = (oggmux_kate_stream*)malloc(n_kate_streams*sizeof(oggmux_kate_stream));
    for (n=0; n<n_kate_streams; ++n) {
        oggmux_kate_stream *ks=info->kate_streams+n;
        ks->katepage_valid = 0;
        ks->katepage_buffer_length = 0;
        ks->katepage = NULL;
        ks->katetime = 0;
    }
}

static void write16le(unsigned char *ptr,ogg_uint16_t v)
{
  ptr[0]=v&0xff;
  ptr[1]=(v>>8)&0xff;
}

static void write32le(unsigned char *ptr,ogg_uint32_t v)
{
  ptr[0]=v&0xff;
  ptr[1]=(v>>8)&0xff;
  ptr[2]=(v>>16)&0xff;
  ptr[3]=(v>>24)&0xff;
}

static void write64le(unsigned char *ptr,ogg_int64_t v)
{
  ogg_uint32_t hi=v>>32;
  ptr[0]=v&0xff;
  ptr[1]=(v>>8)&0xff;
  ptr[2]=(v>>16)&0xff;
  ptr[3]=(v>>24)&0xff;
  ptr[4]=hi&0xff;
  ptr[5]=(hi>>8)&0xff;
  ptr[6]=(hi>>16)&0xff;
  ptr[7]=(hi>>24)&0xff;
}

void add_fishead_packet (oggmux_info *info) {
    ogg_packet op;

    memset (&op, 0, sizeof (op));

    op.packet = (unsigned char*)_ogg_calloc (64, sizeof(unsigned char));
    if (op.packet == NULL) return;

    memset (op.packet, 0, 64);
    memcpy (op.packet, FISHEAD_IDENTIFIER, 8); /* identifier */
    write16le(op.packet+8, SKELETON_VERSION_MAJOR); /* version major */
    write16le(op.packet+10, SKELETON_VERSION_MINOR); /* version minor */
    write64le(op.packet+12, (ogg_int64_t)0); /* presentationtime numerator */
    write64le(op.packet+20, (ogg_int64_t)1000); /* presentationtime denominator */
    write64le(op.packet+28, (ogg_int64_t)0); /* basetime numerator */
    write64le(op.packet+36, (ogg_int64_t)1000); /* basetime denominator */
    /* both the numerator are zero hence handled by the memset */
    write32le(op.packet+44, 0); /* UTC time, set to zero for now */

    op.b_o_s = 1; /* its the first packet of the stream */
    op.e_o_s = 0; /* its not the last packet of the stream */
    op.bytes = 64; /* length of the packet in bytes */

    ogg_stream_packetin (&info->so, &op); /* adding the packet to the skeleton stream */
    _ogg_free (op.packet);
}

/*
 * Adds the fishead packets in the skeleton output stream along with the e_o_s packet
 */
void add_fisbone_packet (oggmux_info *info) {
    ogg_packet op;

    if (!info->audio_only) {
        memset (&op, 0, sizeof (op));
        op.packet = (unsigned char*)_ogg_calloc (80, sizeof(unsigned char));
        if (op.packet == NULL) return;

        memset (op.packet, 0, 80);
        /* it will be the fisbone packet for the theora video */
        memcpy (op.packet, FISBONE_IDENTIFIER, 8); /* identifier */
        write32le(op.packet+8, FISBONE_MESSAGE_HEADER_OFFSET); /* offset of the message header fields */
        write32le(op.packet+12, info->to.serialno); /* serialno of the theora stream */
        write32le(op.packet+16, 3); /* number of header packets */
        /* granulerate, temporal resolution of the bitstream in samples/microsecond */
        write64le(op.packet+20, info->ti.fps_numerator); /* granulrate numerator */
        write64le(op.packet+28, info->ti.fps_denominator); /* granulrate denominator */
        write64le(op.packet+36, 0); /* start granule */
        write32le(op.packet+44, 0); /* preroll, for theora its 0 */
        *(op.packet+48) = theora_granule_shift (&info->ti); /* granule shift */
        memcpy(op.packet+FISBONE_SIZE, "Content-Type: video/theora\r\n", 28); /* message header field, Content-Type */

        op.b_o_s = 0;
        op.e_o_s = 0;
        op.bytes = 80; /* size of the packet in bytes */

        ogg_stream_packetin (&info->so, &op);
        _ogg_free (op.packet);
    }

    if (!info->video_only) {
        memset (&op, 0, sizeof (op));
        op.packet = (unsigned char*)_ogg_calloc (80, sizeof(unsigned char));
        if (op.packet == NULL) return;

        memset (op.packet, 0, 80);
        /* it will be the fisbone packet for the vorbis audio */
        memcpy (op.packet, FISBONE_IDENTIFIER, 8); /* identifier */
        write32le(op.packet+8, FISBONE_MESSAGE_HEADER_OFFSET); /* offset of the message header fields */
        write32le(op.packet+12, info->vo.serialno); /* serialno of the vorbis stream */
        write32le(op.packet+16, 3); /* number of header packet */
        /* granulerate, temporal resolution of the bitstream in Hz */
        write64le(op.packet+20, info->sample_rate); /* granulerate numerator */
        write64le(op.packet+28, (ogg_int64_t)1); /* granulerate denominator */
        write64le(op.packet+36, 0); /* start granule */
        write32le(op.packet+44, 2); /* preroll, for vorbis its 2 */
        *(op.packet+48) = 0; /* granule shift, always 0 for vorbis */
        memcpy (op.packet+FISBONE_SIZE, "Content-Type: audio/vorbis\r\n", 28);
        /* Important: Check the case of Content-Type for correctness */

        op.b_o_s = 0;
        op.e_o_s = 0;
        op.bytes = 80;

        ogg_stream_packetin (&info->so, &op);
        _ogg_free (op.packet);
    }

#ifdef HAVE_KATE
    int n, ret;
    if (info->with_kate) {
        for (n=0; n<info->n_kate_streams; ++n) {
            oggmux_kate_stream *ks=info->kate_streams+n;
	    memset (&op, 0, sizeof (op));
	    op.packet = _ogg_calloc (86, sizeof(unsigned char));
	    memset (op.packet, 0, 86);
            /* it will be the fisbone packet for the kate stream */
	    memcpy (op.packet, FISBONE_IDENTIFIER, 8); /* identifier */
            write32le(op.packet+8, FISBONE_MESSAGE_HEADER_OFFSET); /* offset of the message header fields */
	    write32le(op.packet+12, ks->ko.serialno); /* serialno of the vorbis stream */
            write32le(op.packet+16, ks->ki.num_headers); /* number of header packet */
	    /* granulerate, temporal resolution of the bitstream in Hz */
	    write64le(op.packet+20, ks->ki.gps_numerator); /* granulerate numerator */
            write64le(op.packet+28, ks->ki.gps_denominator); /* granulerate denominator */
	    write64le(op.packet+36, 0); /* start granule */
            write32le(op.packet+44, 0); /* preroll, for kate it's 0 */
	    *(op.packet+48) = ks->ki.granule_shift; /* granule shift */
            memcpy (op.packet+FISBONE_SIZE, "Content-Type: application/x-kate\r\n", 34);
	    /* Important: Check the case of Content-Type for correctness */
	
	    op.b_o_s = 0;
	    op.e_o_s = 0;
	    op.bytes = 86;
	
            ogg_stream_packetin (&info->so, &op);
	    _ogg_free (op.packet);
        }
    }
#endif
}

void *timer_start(void)
{
    time_t *start = (time_t *)malloc(sizeof(time_t));
    time(start);
    return (void *)start;
}

int	sampleError;
SRC_STATE *src_state;
SRC_DATA *src_data;
double ratio;
double *sampleOut;

void oggmux_init (oggmux_info *info){
    ogg_page og;
    ogg_packet op;
    TIMER *timer;
    src_state = NULL;
    src_data = NULL;
    ratio = 1.0;
    sampleOut = NULL;

    /* yayness.  Set up Ogg output stream */
    srand (time (NULL));

    if(!info->audio_only){
        ogg_stream_init (&info->to, rand ());    /* oops, add one ot the above */
        theora_encode_init (&info->td, &info->ti);
        if(info->speed_level >= 0) {
            int max_speed_level;
            theora_control(&info->td, TH_ENCCTL_GET_SPLEVEL_MAX, &max_speed_level, sizeof(int));
            if(info->speed_level > max_speed_level)
            info->speed_level = max_speed_level;
            theora_control(&info->td, TH_ENCCTL_SET_SPLEVEL, &info->speed_level, sizeof(int));
        }
    }
    /* init theora done */

    /* initialize Vorbis too, if we have audio. */
    if(!info->video_only){
        int ret;
	if (ogg_stream_init (&info->vo, rand ()) == -1)
	{
	    std::cerr << "-------- ogg_stream_init -- failed !!" << std::endl;
	    return;
	}
  if (!(src_state = src_new (0, info->channels, &sampleError))) {
    std::cerr << "--- error initialysing the SRC_STATE :" << src_strerror (sampleError) << std::endl;
  }
  else {
    src_data = (SRC_DATA *)malloc(sizeof(SRC_DATA));
  }
       vorbis_info_init (&info->vi);//
       if (!(info->vorbis_quality >= -0.1) || !(info->vorbis_quality <= 1.0))
	 info->vorbis_quality = 0.5;	//if quality has a wrong value, sets to 0.5
        if(vorbis_encode_setup_vbr(&info->vi, info->channels, info->sample_rate, info->vorbis_quality)){
            std::cerr << "Mode initialisation failed: invalid parameters for quality" << std::endl;
            vorbis_info_clear(&info->vi);
            return;
        }
        
        long bitrate;
        struct ovectl_ratemanage2_arg ai;
	ret = vorbis_encode_ctl(&info->vi, OV_ECTL_RATEMANAGE2_GET, &ai);
	if (ret && OV_EINVAL)
	{
            std::cerr << "Invalid argument, or an attempt to modify a setting after calling vorbis_encode_setup_init() 1" << std::endl;
	    //return;
	}
	else if (ret && OV_EIMPL)
	{
            std::cerr << "Unimplemented or unknown request" << std::endl;
	    //return;
	}
	{
	        vorbis_info vi2;
                vorbis_info_init(&vi2);
                vorbis_encode_setup_vbr(&vi2, info->channels, info->sample_rate, info->vorbis_quality);
                vorbis_encode_setup_init(&vi2);
                bitrate = vi2.bitrate_nominal;
                vorbis_info_clear(&vi2);
	}	
	ai.bitrate_average_kbps = bitrate/1000;
	info->aveVorBitRate = ai.bitrate_average_kbps;
        ai.bitrate_average_damping = 1.5;
        ai.bitrate_limit_reservoir_bits = bitrate * 2;
        ai.bitrate_limit_reservoir_bias = .1;

            /* And now the ones we actually wanted to set */
        ai.bitrate_limit_min_kbps=10;
        ai.bitrate_limit_max_kbps=256;
        ai.management_active=1;
	ret = vorbis_encode_ctl(&info->vi, OV_ECTL_RATEMANAGE2_SET, &ai);
	if (ret && OV_EINVAL)
	{
            std::cerr << "Invalid argument, or an attempt to modify a setting after calling vorbis_encode_setup_init() 2" << std::endl;
	}
	else if (ret && OV_EIMPL)
	{
            std::cerr << "Unimplemented or unknown request" << std::endl;
	}
	ret = vorbis_encode_setup_init(&info->vi);
	if (ret && OV_EINVAL)
	{
            std::cerr << "Attempt to use vorbis_encode_setup_init() without first calling \
		one of vorbis_encode_setup_managed() or vorbis_encode_setup_vbr() \
		to initialize the high-level encoding setup" << std::endl;
	}
	else if (ret && OV_EFAULT)
	{
            std::cerr << "Internal logic fault; indicates a bug or heap/stack corruption." << std::endl;
	}

//---------------------- vorbis_analysis_init renvoie 0 comme avec la commande oggenc !!

        if (!vorbis_analysis_init (&info->vd, &info->vi))
	{
	  std::cerr << "-------- vorbis_analysis_init failed, seems normal !!" << std::endl;
	}

        vorbis_comment_init (&info->vc);
        vorbis_comment_add_tag (&info->vc, "ENCODER",PACKAGE);
        vorbis_block_init (&info->vd, &info->vb);

    }
    /* audio init done */

    /* initialize kate if we have subtitles */
    if (info->with_kate) {
#ifdef HAVE_KATE
        int ret, n;
        for (n=0; n<info->n_kate_streams; ++n) {
            oggmux_kate_stream *ks=info->kate_streams+n;
            ogg_stream_init (&ks->ko, rand ());    /* oops, add one ot the above */
            ret = kate_encode_init (&ks->k, &ks->ki);
            if (ret<0) {
				error("kate_encode_init: %d",ret);
				return;
            }
            ret = kate_comment_init(&ks->kc);
            if (ret<0) {
				error("kate_comment_init: %d",ret);
				return;
            }
            kate_comment_add_tag (&ks->kc, "ENCODER",PACKAGE_STRING);
        }
#endif
    }
    /* kate init done */

    /* first packet should be skeleton fishead packet, if skeleton is used */

    if (info->with_skeleton) {
        ogg_stream_init (&info->so, rand());
        add_fishead_packet (info);
        if (ogg_stream_pageout (&info->so, &og) != 1){
            error("Internal Ogg library error.");
			return;
        }

		ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
		ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);
		
//         fwrite (og.header, 1, og.header_len, info->outfile);
//         fwrite (og.body, 1, og.body_len, info->outfile);
    }

    /* write the bitstream header packets with proper page interleave */

    /* first packet will get its own page automatically */
    if(!info->audio_only){
        if (theora_encode_header (&info->td, &op))
	{
	  std::cerr << "can't put the theora header in the packet" << std::endl;
	}
        ogg_stream_packetin (&info->to, &op);
        if (ogg_stream_pageout (&info->to, &og) != 1){
	    std::cerr << "--------Internal Ogg library error.  !!" << std::endl;
            error("Internal Ogg library error.");
	    return;
        }
	ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
	ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);
//         fwrite (og.header, 1, og.header_len, info->outfile);
//         fwrite (og.body, 1, og.body_len, info->outfile);

        /* create the remaining theora headers */
        /* theora_comment_init (&info->tc); is called in main() prior to parsing options */
        theora_comment_add_tag (&info->tc, (char*)"ENCODER",(char*)PACKAGE_STRING);
        theora_encode_comment (&info->tc, &op);
        ogg_stream_packetin (&info->to, &op);
//         _ogg_free (op.packet);

        theora_encode_tables (&info->td, &op);
        ogg_stream_packetin (&info->to, &op);
    }
    if(!info->video_only){
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;


        if (vorbis_analysis_headerout (&info->vd, &info->vc, &header,
                       &header_comm, &header_code))
	{
	  std::cerr << "-------- vorbis_analysis_headerout -- failed !!" << std::endl;
	  //return;
	}
	else
	{
	  if (ogg_stream_packetin (&info->vo, &header))    /* automatically placed in its own page */
	  {
	    std::cerr << "-------- ogg_stream_packetin header failed !!" << std::endl;
	    //return;
	  }
	  int result;
	  while((result = ogg_stream_flush(&info->vo, &og))){ //1 Vorbis  same as encode.c
            if(!result) break;
	    int ret = 0;
	    if (!(ret = ogg_pipe_write("write vorbis header", info->ringbuffer, (char*)og.header, og.header_len)))
	    {
	      std::cerr << "-------- ogg_pipe_write vorbis header failed !!" << std::endl;
	      //return;
	    }
	    
	    if (!(ret += ogg_pipe_write("write vorbis body", info->ringbuffer, (char*)og.body, og.body_len)))
	    {
	      std::cerr << "-------- ogg_pipe_write vorbis body failed !!" << std::endl;
	      //return;
	    }
            //ret = oe_write_page(&og, opt->out); belongs to daddy .... no encode.c :)
            if(ret != og.header_len + og.body_len)
            {
                std::cerr << "Failed writing header to output stream" << std::endl << std::flush;
                ret = 1;
		return;
//                 goto cleanup; /* Bail and try to clean up stuff */
            }
	  }	  

	  if (ogg_stream_packetin (&info->vo, &header_comm))    /* automatically placed in its own page */
	  {
	    std::cerr << "-------- ogg_stream_packetin header failed !!" << std::endl;
	    //return;
	  }
	  if (ogg_stream_packetin (&info->vo, &header_code))    /* automatically placed in its own page */
	  {
	    std::cerr << "-------- ogg_stream_packetin header failed !!" << std::endl;
	    //return;
	  }
	  while((result = ogg_stream_flush(&info->vo, &og))){	//2 Vorbis  same as encode.c
            if(!result) break;
	    int ret = 0;
	    if (!(ret = ogg_pipe_write("write vorbis header", info->ringbuffer, (char*)og.header, og.header_len)))
	    {
	      std::cerr << "-------- ogg_pipe_write vorbis header failed !!" << std::endl;
	      //return;
	    }
	    
	    if (!(ret += ogg_pipe_write("write vorbis body", info->ringbuffer, (char*)og.body, og.body_len)))
	    {
	      std::cerr << "-------- ogg_pipe_write vorbis body failed !!" << std::endl;
	      //return;
	    }
            //ret = oe_write_page(&og, opt->out); belongs to daddy .... no encode.c :)
            if(ret != og.header_len + og.body_len)
            {
                std::cerr << "Failed writing header to output stream" << std::endl;
                ret = 1;
		return;
//                 goto cleanup; /* Bail and try to clean up stuff */
            }
	  }
	}
    }

#ifdef HAVE_KATE
    if (info->with_kate) {
        int n;
        for (n=0; n<info->n_kate_streams; ++n) {
            oggmux_kate_stream *ks=info->kate_streams+n;
            int ret;
            while (1) {
                ret=kate_ogg_encode_headers(&ks->k,&ks->kc,&op);
                if (ret==0) {
                  ogg_stream_packetin(&ks->ko,&op);
                  ogg_packet_clear(&op);
                }
                if (ret<0) warning("kate_encode_headers: %d",ret);
                if (ret>0) break;
            }

            /* first header is on a separate page - libogg will do it automatically */
            ret=ogg_stream_pageout (&ks->ko, &og);
            if (ret!=1) {
                error("Internal Ogg library error.");
				return;
            }
			ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
			ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);

//             fwrite (og.header, 1, og.header_len, info->outfile);
//             fwrite (og.body, 1, og.body_len, info->outfile);
        }
    }
#endif

    /* output the appropriate fisbone packets */
    if (info->with_skeleton) {
        add_fisbone_packet (info);
        while (1) {
            int result = ogg_stream_flush (&info->so, &og);	//3 with_skeleton
			if (result < 0) {
                /* can't get here */
                error("Internal Ogg library error.");
				return;
			}
            if (result == 0)  break;
			ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
			ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);
			
        }
    }

    if (info->audio_only) {
      theora_info_clear(&info->ti);
    }

    /* Flush the rest of our headers. This ensures
     * the actual data in each stream will start
     * on a new page, as per spec. */
    while (1 && !info->audio_only){
        int result = ogg_stream_flush (&info->to, &og);	//4 theora
        if (result < 0){
            /* can't get here */
	    std::cerr << "--------  Internal Ogg library error !!" << std::endl << std::flush;
            error("Internal Ogg library error.");
	    return;
        }
        if (result == 0) break;
	ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
	ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);
    }
    if (info->with_kate) {
        int n;
        for (n=0; n<info->n_kate_streams; ++n) {
            oggmux_kate_stream *ks=info->kate_streams+n;
            while (1) {
                int result = ogg_stream_flush (&ks->ko, &og);	//6 kate
                if (result < 0){
                    /* can't get here */
                    error("Internal Ogg library error.");
					return;
                }
                if (result == 0) break;
				ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
				ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);
				
            }
        }
    }
	
    if (info->with_skeleton) {
        int result;

        /* build and add the e_o_s packet */
        memset (&op, 0, sizeof (op));
            op.b_o_s = 0;
        op.e_o_s = 1; /* its the e_o_s packet */
            op.granulepos = 0;
        op.bytes = 0; /* e_o_s packet is an empty packet */
            ogg_stream_packetin (&info->so, &op);

        result = ogg_stream_flush (&info->so, &og);	//7 with_skeleton
        if (result < 0){
            /* can't get here */
            error("Internal Ogg library error.");
			return;
        }
		ogg_pipe_write("write theora header", info->ringbuffer, (char*)og.header, og.header_len);
		ogg_pipe_write("write theora body", info->ringbuffer, (char*)og.body, og.body_len);
		
//         fwrite (og.header, 1, og.header_len,info->outfile);
//         fwrite (og.body, 1, og.body_len, info->outfile);
    }
}

/**
 * adds a video frame to the encoding sink
 * if e_o_s is 1 the end of the logical bitstream will be marked.
 * @param this ff2theora struct
 * @param info oggmux_info
 * @param yuv_buffer
 * @param e_o_s 1 indicates ond of stream
 */
void oggmux_add_video (oggmux_info *info, yuv_buffer *yuv, int e_o_s){
    ogg_packet op;
    theora_encode_YUVin (&info->td, yuv);
    while(theora_encode_packetout (&info->td, e_o_s, &op)) {
        ogg_stream_packetin (&info->to, &op);
        info->v_pkg++;
    }
}


/**
 * adds audio samples to encoding sink
 * @param buffer pointer to buffer
 * @param bytes bytes in buffer
 * @param samples samples in buffer
 * @param e_o_s 1 indicates end of stream.
 */
void oggmux_add_audio (oggmux_info *info, float * buffer, int bytes, int samples, int e_o_s){
    ogg_packet op;
    float *ptr = buffer;
    float *sampleOut = NULL;
    int i,j, c, count = 0;
    float **vorbis_buffer;

    if (bytes <= 0 && samples <= 0){
        /* end of audio stream */
        if(e_o_s)
            vorbis_analysis_wrote (&info->vd, 0);
    }
    else{	//resample code
      sampleOut = (float *)realloc(sampleOut, ((samples * info->channels * sizeof(float)) + 1000));
      memset(src_data, 0, sizeof(SRC_DATA));
      src_data->data_in = buffer;
      src_data->input_frames = (long)samples;
      src_data->data_out = sampleOut;
      src_data->output_frames = (long)((double)samples * 1.1);
      src_data->src_ratio = ratio; //48000.0 / 48500.0;
      src_data->end_of_input = 0;
      if (int processError = src_process (src_state, src_data)) {
	std::cerr << "--- resample error :" << src_strerror (processError) << std::endl;
      }
      
      vorbis_buffer = vorbis_analysis_buffer (&info->vd, src_data->output_frames_gen);	//samples = rv/(channels*sizeof(float))
      for (j=0; j < info->channels; j++)
      {
	for (i=0, c=0; i < src_data->output_frames_gen ; i++, c+=info->channels)
	{
	  vorbis_buffer[j][i] = sampleOut[c+j];
	}
      }
      vorbis_analysis_wrote (&info->vd, src_data->output_frames_gen);
    }
    int ret;
    while((ret = vorbis_analysis_blockout (&info->vd, &info->vb)) == 1){	//idem
        /* analysis, assume we want to use bitrate management */
        vorbis_analysis (&info->vb, NULL);				//idem
        vorbis_bitrate_addblock (&info->vb);				//idem

	int bet;
        /* weld packets into the bitstream */
	while ((bet = vorbis_bitrate_flushpacket (&info->vd, &op)) == 1){
            ogg_stream_packetin (&info->vo, &op);
	    info->a_pkg++;
        }
        if (bet && OV_EINVAL)
          std::cerr << std::endl << "vorbis_analysis_blockout :Invalid parameters." << std::endl << std::flush;
        else if (bet && OV_EFAULT)
          std::cerr << std::endl << "vorbis_analysis_blockout :Internal fault; \
              indicates a bug or memory corruption." << std::endl << std::flush;
        else if (bet && OV_EIMPL)
          std::cerr << std::endl << "vorbis_analysis_blockout : Unimplemented; \
              not supported by this version of the library." << std::endl << std::flush;
    }
    if (ret && OV_EINVAL)
      std::cerr << std::endl << "vorbis_analysis_blockout :Invalid parameters." << std::endl << std::flush;
    else if (ret && OV_EFAULT)
      std::cerr << std::endl << "vorbis_analysis_blockout :Internal fault; \
	    indicates a bug or memory corruption." << std::endl << std::flush;
    else if (ret && OV_EIMPL)
      std::cerr << std::endl << "vorbis_analysis_blockout : Unimplemented; \
	    not supported by this version of the library." << std::endl << std::flush;
}

/**    
 * adds a subtitles text to the encoding sink
 * if e_o_s is 1 the end of the logical bitstream will be marked.
 * @param info oggmux_info
 * @param idx which kate stream to output to
 * @param t0 the show time of the text
 * @param t1 the hide time of the text
 * @param text the utf-8 text
 * @param len the number of bytes in the text
 */
void oggmux_add_kate_text (oggmux_info *info, int idx, double t0, double t1, const char *text, size_t len){
#ifdef HAVE_KATE
    ogg_packet op;
    oggmux_kate_stream *ks=info->kate_streams+idx;
    int ret;
    ret = kate_ogg_encode_text(&ks->k, t0, t1, text, len, &op);
    if (ret>=0) {
        ogg_stream_packetin (&ks->ko, &op);
        ogg_packet_clear (&op);
        info->k_pkg++;
    }
    else {
        error("Failed to encode kate data packet (%f --> %f, [%s]): %d",
            t0, t1, text, ret);
    }
#endif
}
    
/**    
 * adds a kate end packet to the encoding sink
 * @param info oggmux_info
 * @param idx which kate stream to output to
 * @param t the time of the end packet
 */
void oggmux_add_kate_end_packet (oggmux_info *info, int idx, double t){
#ifdef HAVE_KATE
    ogg_packet op;
    oggmux_kate_stream *ks=info->kate_streams+idx;
    int ret;
    ret = kate_ogg_encode_finish(&ks->k, t, &op);
    if (ret>=0) {
        ogg_stream_packetin (&ks->ko, &op);
        ogg_packet_clear (&op);
        info->k_pkg++;
    }
    else {
        error("Failed to encode kate end packet: %d", ret);
    }
#endif
}
    
static double get_remaining(oggmux_info *info, double timebase) {
  double remaining = 0;
  double to_encode, time_so_far;

    if(info->duration != -1 && timebase > 0) {
        time_so_far = time(NULL) - info->start_time;
        to_encode = info->duration - timebase;
        if(to_encode > 0) {
            remaining = (time_so_far / timebase) * to_encode;
        }
    }
    return remaining;
}

static void print_stats(oggmux_info *info, double timebase) {
//    int hundredths = timebase * 100 - (long) timebase * 100;
    int seconds = (long) timebase % 60;
    int minutes = ((long) timebase / 60) % 60;
    int hours = (long) timebase / 3600;
 	double remaining = time(NULL) - info->start_time;
//    double remaining = get_remaining(info, timebase);
    int remaining_seconds = (long) remaining % 60;
    int remaining_minutes = ((long) remaining / 60) % 60;
    int remaining_hours = (long) remaining / 3600;
	
	// status
	snprintf (info->status, 511,
			  "%d:%02d:%02d audio: %dkbps video: %dkbps, time elapsed: %02d:%02d:%02d",
			  hours, minutes, seconds, info->akbps, info->vkbps,
			  remaining_hours, remaining_minutes, remaining_seconds );
}


static void write_audio_page(oggmux_info *info)
{
  int ret;

  ret = ogg_pipe_write("write vorbis audio page",
					   info->ringbuffer, (char*)info->audiopage, info->audiopage_len);
//  ret = fwrite(info->audiopage, 1, info->audiopage_len, info->outfile);

  if(ret < info->audiopage_len) {
	  error("error writing audio page");
	  exit (0);
  }
  else {
    info->audio_bytesout += ret;
  }
  info->audiopage_valid = 0;
  info->a_pkg -=ogg_page_packets((ogg_page *)&info->audiopage);
#ifdef OGGMUX_DEBUG
  info->a_page++;
  info->v_page=0;
  func("audio page %d (%d pkgs) | pkg remaining %d",
	   info->a_page,ogg_page_packets((ogg_page *)&info->audiopage),info->a_pkg);
#endif
  
  info->akbps = rint (info->audio_bytesout * 8. / info->audiotime * .001);
  if(info->akbps<0)
    info->akbps=0;
  print_stats(info, info->audiotime);
}

static void write_video_page(oggmux_info *info)
{
  int ret;
  
  ret = ogg_pipe_write("write theora video page",
					   info->ringbuffer, (char*)info->videopage, info->videopage_len);
//  ret = fwrite(info->videopage, 1, info->videopage_len, info->outfile);
  if(ret < info->videopage_len) {
	  error("error writing video page");
  }
  else {
    info->video_bytesout += ret;
  }
  info->videopage_valid = 0;
  info->v_pkg -= ogg_page_packets((ogg_page *)&info->videopage);
#ifdef OGGMUX_DEBUG
  info->v_page++;
  info->a_page=0;
  func("video page %d (%d pkgs) | pkg remaining %d",
	   info->v_page,ogg_page_packets((ogg_page *)&info->videopage),info->v_pkg);
#endif


  info->vkbps = rint (info->video_bytesout * 8. / info->videotime * .001);
  if(info->vkbps<0)
    info->vkbps=0;
  print_stats(info, info->videotime);
}

static void write_kate_page(oggmux_info *info, int idx)
{
  int ret;
  oggmux_kate_stream *ks=info->kate_streams+idx;

  ret = ogg_pipe_write("write kate page",
					   info->ringbuffer, (char*)ks->katepage, ks->katepage_len);
//  ret = fwrite(ks->katepage, 1, ks->katepage_len, info->outfile);
  if(ret < ks->katepage_len) {
	  error("error writing kate page");
  }
  else {
    info->kate_bytesout += ret;
  }
  ks->katepage_valid = 0;
  info->k_pkg -= ogg_page_packets((ogg_page *)&ks->katepage);
#ifdef OGGMUX_DEBUG
  ks->k_page++;
  func("kate page %d (%d pkgs) | pkg remaining %d",
	   ks->k_page,ogg_page_packets((ogg_page *)&info->katepage),info->k_pkg);
#endif


  /*
  info->kkbps = rint (info->kate_bytesout * 8. / info->katetime * .001);
  if(info->kkbps<0)
    info->kkbps=0;
  print_stats(info, info->katetime);
  */
}

static int find_best_valid_kate_page(oggmux_info *info)
{
  int n;
  double t=0.0;
  int best=-1;
  if (info->with_kate) for (n=0; n<info->n_kate_streams;++n) {
    oggmux_kate_stream *ks=info->kate_streams+n;
    if (ks->katepage_valid) {
      if (best==-1 || ks->katetime<t) {
        t=ks->katetime;
        best=n;
      }
    }
  }
  return best;
}

double _fabs (double val)
{
  if (val >= 0)
    return (val);
  else
    return (-val);
}

void oggmux_flush (oggmux_info *info, int e_o_s)
{
    int len;
//     ogg_page ogv, ogt;
    ogg_page og;
    int best;
    
    /* flush out the ogg pages to info->outfile */
    while(1) {
      /* Get pages for both streams, if not already present, and if available.*/
      if(!info->audio_only && !info->videopage_valid) {
        int v_next=0;
        if(ogg_stream_pageout(&info->to, &og) > 0) {			//2
          v_next=1;
        }
        if(v_next) {
          len = og.header_len + og.body_len;
          if(info->videopage_buffer_length < len) {
		  info->videopage = (unsigned char*)realloc(info->videopage, len);
            info->videopage_buffer_length = len;
          }
          info->videopage_len = len;
          memcpy(info->videopage, og.header, og.header_len);
          memcpy(info->videopage+og.header_len , og.body, og.body_len);

          info->videopage_valid = 1;
          if(ogg_page_granulepos(&og)>0) {					//3
            info->videotime = theora_granule_time (&info->td,
                  ogg_page_granulepos(&og));
	    if (info->videotime == -1)
	    {
	      std::cerr << "the given theora granulepos is invalid" << std::endl << std::flush;
	    }
          }
        }
      }
      if(!info->video_only && !info->audiopage_valid) {
        int a_next=0;
	if(ogg_stream_pageout(&info->vo, &og) > 0) {
	    a_next=1;
	}
	if(a_next) {
	  len = og.header_len + og.body_len;
	  if(info->audiopage_buffer_length < len) {
	      info->audiopage = (unsigned char*)realloc(info->audiopage, len);
	      info->audiopage_buffer_length = len;
	  }
	  info->audiopage_len = len;
	  memcpy(info->audiopage, og.header, og.header_len);
	  memcpy(info->audiopage+og.header_len , og.body, og.body_len);

	  info->audiopage_valid = 1;
	  if(ogg_page_granulepos(&og)>0) {
	      info->audiotime= vorbis_granule_time (&info->vd,
                  ogg_page_granulepos(&og));
	    if (info->audiotime == -1)
		std::cerr << "the given vorbis granulepos is invalid" << std::endl << std::flush;
	    else {
	      ratio = ((info->videotime - info->audiotime) / 10.0) + 1.0;
	      if (ratio > 1.05)
		ratio = 1.05;
	      else if (ratio <= 0.95)
		ratio = 0.95;
	    }
	  }
// 	  std::cerr << "Vorbis time :" << info->audiotime << std::endl << std::flush;
	}
      }

#ifdef HAVE_KATE
      int n;
      if (info->with_kate) for (n=0; n<info->n_kate_streams; ++n) {
        oggmux_kate_stream *ks=info->kate_streams+n;
        if (!ks->katepage_valid) {
          int k_next=0;
          /* always flush kate stream */
          if (ogg_stream_flush(&ks->ko, &og) > 0) {	//8 kate
            k_next = 1;
          }
          if (k_next) {
            len = og.header_len + og.body_len;
            if(ks->katepage_buffer_length < len) {
              ks->katepage = realloc(ks->katepage, len);
              ks->katepage_buffer_length = len;
            }
            ks->katepage_len = len;
            memcpy(ks->katepage, og.header, og.header_len);
            memcpy(ks->katepage+og.header_len , og.body, og.body_len);

            ks->katepage_valid = 1;
            if(ogg_page_granulepos(&og)>0) {
              ks->katetime= kate_granule_time (&ks->ki,
                    ogg_page_granulepos(&og));
            }
          }
        }
      }
#endif

#ifdef HAVE_KATE
#define CHECK_KATE_OUTPUT(which) \
        if (best>=0 && info->kate_streams[best].katetime/*-1.0*/<=info->which##time) { \
          write_kate_page(info, best); \
          continue; \
        }
#else
#define CHECK_KATE_OUTPUT(which) ((void)0)
#endif

      best=find_best_valid_kate_page(info);

      if(info->video_only && info->videopage_valid) {
//         CHECK_KATE_OUTPUT(video);
        write_video_page(info);
      }
      else if(info->audio_only && info->audiopage_valid) {
//         CHECK_KATE_OUTPUT(audio);
        write_audio_page(info);
      }
      /* We're using both. We can output only:
       *  a) If we have valid pages for both
       *  b) At EOS, for the remaining stream.
       */
      else if(info->videopage_valid /*&& info->audiopage_valid*/) {
        /* Make sure they're in the right order. */
//         if(info->videotime <= info->audiotime) {
          //CHECK_KATE_OUTPUT(video);
          write_video_page(info);
//         }
//         else {
          //CHECK_KATE_OUTPUT(audio);
//           write_audio_page(info);
//         }
      }
      else if(info->audiopage_valid) {
	write_audio_page(info);
      }
/*      else if(e_o_s && best>=0) {
          write_kate_page(info, best);
      }*/
      else if(e_o_s && info->videopage_valid) {
          write_video_page(info);
      }
      else if(e_o_s && info->audiopage_valid) {
          write_audio_page(info);
      }
      else {
        break; /* Nothing more writable at the moment */
      }
    }
}

void oggmux_close (oggmux_info *info){
    int n;

    if(!info->video_only)
    {
      ogg_stream_clear (&info->vo);	//segfault here !!
      vorbis_block_clear (&info->vb);
      vorbis_dsp_clear (&info->vd);
      vorbis_comment_clear (&info->vc);
      vorbis_info_clear (&info->vi);
    }

    if (!info->audio_only)
    {
      ogg_stream_clear (&info->to);
      theora_comment_clear (&info->tc);
      theora_clear (&info->td);
    }

#ifdef HAVE_KATE
    for (n=0; n<info->n_kate_streams; ++n) {
        ogg_stream_clear (&info->kate_streams[n].ko);
        kate_comment_clear (&info->kate_streams[n].kc);
        kate_info_clear (&info->kate_streams[n].ki);
        kate_clear (&info->kate_streams[n].k);
    }
    free(info->kate_streams);
#endif

    if (info->with_skeleton)
        ogg_stream_clear (&info->so);
//outfile doesn't seem to be used
//     if (info->outfile && info->outfile != stdout)
//         fclose (info->outfile);

    if(info->videopage)
        free(info->videopage);
    if(info->audiopage)
        free(info->audiopage);

    for (n=0; n<info->n_kate_streams; ++n) {
        if(info->kate_streams[n].katepage)
          free(info->kate_streams[n].katepage);
    }
}
