/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
//  Author : Olivier Debon  <odebon@club-internet.fr>
//  

#include "swf.h"

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

static unsigned char *inputData;

// Class variables

int Bitmap::haveTables = 0;

struct jpeg_decompress_struct Bitmap::jpegObject;

struct jpeg_source_mgr Bitmap::jpegSourceManager;

MyErrorHandler Bitmap::jpegErrorMgr;

Bitmap::Bitmap(long id, int level) : Character(BitmapType, id )
{
    pixels = NULL;
    alpha_buf = NULL;
    colormap = NULL;
    nbColors = 0;
    defLevel = level;
}

Bitmap::~Bitmap()
{
	if (pixels) {
		delete[] pixels;
	}
        if (alpha_buf) {
            delete[] alpha_buf;
        }
	if (colormap)
	{
		delete colormap;
	}
	if (haveTables) {
		jpeg_destroy_decompress(&jpegObject);
		haveTables = 0;
	}
}

static void errorExit(j_common_ptr info)
{
	(*info->err->output_message) (info);
	longjmp(((MyErrorHandler *)info->err)->setjmp_buffer, 1);
}

// Methods for Source data manager
static void initSource(struct jpeg_decompress_struct *cInfo)
{
	cInfo->src->bytes_in_buffer = 0;
}

static boolean fillInputBuffer(struct jpeg_decompress_struct *cInfo)
{
	cInfo->src->next_input_byte = inputData;
	cInfo->src->bytes_in_buffer = 1;
	inputData++;

	return 1;
}

static void skipInputData(struct jpeg_decompress_struct *cInfo, long count)
{
	cInfo->src->bytes_in_buffer = 0;
	inputData += count;
}

static boolean resyncToRestart(struct jpeg_decompress_struct *cInfo, int desired)
{
	return jpeg_resync_to_restart(cInfo, desired);
}

static void termSource(struct jpeg_decompress_struct *cInfo)
{
}

long Bitmap::getWidth()
{
	return width;
}

long Bitmap::getHeight()
{
	return height;
}

Color *
Bitmap::getColormap(long *n) {
       if (n) *n = nbColors;
       return colormap;
}

unsigned char *
Bitmap::getPixels()
{
       return pixels;
}

// Read Tables and Compressed data to produce an image

static int
buildJpegAlpha(Bitmap *b, unsigned char *buffer)
{
    z_stream	stream;
    int		status;
    unsigned char  *data;

    data = new unsigned char[b->width*b->height];
    if (data == NULL) 
        return -1;

    stream.next_in = buffer;
    stream.avail_in = 1;
    stream.next_out = data;
    stream.avail_out = b->width*b->height;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
            
    status = inflateInit(&stream);

    while (1) {
        status = inflate(&stream, Z_SYNC_FLUSH) ;
        if (status == Z_STREAM_END) {
            break;
        }
        if (status != Z_OK) {
            printf("Zlib data error : %s\n", stream.msg);
	    delete data;
            return -1;
        }
        stream.avail_in = 1;
    }

    inflateEnd(&stream);
            
    b->alpha_buf = data;

    return 0;
}

int
Bitmap::buildFromJpegInterchangeData(unsigned char *stream, int read_alpha, long offset)
{
	struct jpeg_decompress_struct cInfo;
	struct jpeg_source_mgr mySrcMgr;
	MyErrorHandler errorMgr;
	JSAMPROW buffer[1];
	unsigned char *ptrPix;
	int stride;
	long n;

#if PRINT&1
        printf("flash: loading jpeg (interchange)\n");
#endif

	// Kludge to correct some corrupted files
	if (stream[1] == 0xd9 && stream[3] == 0xd8) {
		stream[3] = 0xd9;
		stream[1] = 0xd8;
	}

	// Setup error handler
	cInfo.err = jpeg_std_error(&errorMgr.pub);
	errorMgr.pub.error_exit = errorExit;

	if (setjmp(errorMgr.setjmp_buffer)) {
		// JPEG data Error
		jpeg_destroy_decompress(&cInfo);
		if (pixels) {
			delete[] pixels;
			pixels = NULL;
		}
		return -1;
	}

	// Set current stream pointer to stream
	inputData = stream;

	// Here it's Ok

	jpeg_create_decompress(&cInfo);

	// Setup source manager structure
	mySrcMgr.init_source = initSource;
	mySrcMgr.fill_input_buffer = fillInputBuffer;
	mySrcMgr.skip_input_data = skipInputData;
	mySrcMgr.resync_to_restart = resyncToRestart;
	mySrcMgr.term_source = termSource;

	// Set default source manager
	cInfo.src = &mySrcMgr;

	jpeg_read_header(&cInfo, FALSE);

	jpeg_read_header(&cInfo, TRUE);
	cInfo.quantize_colors = TRUE;	// Create colormapped image
	jpeg_start_decompress(&cInfo);

	// Set objet dimensions
	height = cInfo.output_height;
	width = cInfo.output_width;
        bpl = width;
	pixels = new unsigned char [height*width];
	if (pixels == NULL) {
		jpeg_finish_decompress(&cInfo);
		jpeg_destroy_decompress(&cInfo);
		return -1;
	}
	ptrPix = pixels;

	stride = cInfo.output_width * cInfo.output_components;

	buffer[0] = (JSAMPROW)malloc(stride);

	while (cInfo.output_scanline < cInfo.output_height) {

		jpeg_read_scanlines(&cInfo, buffer, 1);

		memcpy(ptrPix,buffer[0],stride);

		ptrPix+= stride;
	}

        free(buffer[0]);

	colormap = new Color[cInfo.actual_number_of_colors];
	if (colormap == NULL) {
		delete pixels;
		jpeg_finish_decompress(&cInfo);
		jpeg_destroy_decompress(&cInfo);
		return -1;
	}
	nbColors = cInfo.actual_number_of_colors;

	for(n=0; n < nbColors; n++)
	{
		colormap[n].red = cInfo.colormap[0][n];
		colormap[n].green = cInfo.colormap[1][n];
		colormap[n].blue = cInfo.colormap[2][n];
	}

	jpeg_finish_decompress(&cInfo);
	jpeg_destroy_decompress(&cInfo);

        if (read_alpha) {
            if (buildJpegAlpha(this,  stream + offset) < 0) {
	    	return -1;
	    }
        }
	return 0;
}

// Read JPEG image using pre-loaded Tables

int
Bitmap::buildFromJpegAbbreviatedData(unsigned char *stream)
{
	JSAMPROW buffer[1];
	unsigned char *ptrPix;
	int stride;
	long n;
	int status;

#if PRINT&1
        printf("flash: loading jpeg (abbreviated)\n");
#endif

	// Set current stream pointer to stream
	inputData = stream;

	// Error handler
	if (setjmp(jpegErrorMgr.setjmp_buffer)) {
		// JPEG data Error
		//jpeg_destroy_decompress(&jpegObject);
		if (pixels) {
			delete[] pixels;
			pixels = NULL;
		}
		return -1;
	}

	// Here it's ok

	jpeg_read_header(&jpegObject, TRUE);
	jpegObject.quantize_colors = TRUE;	// Create colormapped image
	jpeg_start_decompress(&jpegObject);

	// Set objet dimensions
	height = jpegObject.output_height;
	width = jpegObject.output_width;
        bpl = width;
	pixels = new unsigned char [height*width];
	if (pixels == NULL) {
		jpeg_finish_decompress(&jpegObject);
		return -1;
	}
	ptrPix = pixels;

	stride = jpegObject.output_width * jpegObject.output_components;

	buffer[0] = (JSAMPROW)malloc(stride);

	while (jpegObject.output_scanline < jpegObject.output_height) {

		status = jpeg_read_scanlines(&jpegObject, buffer, 1);

		memcpy(ptrPix,buffer[0],stride);

		ptrPix+= stride;
	}
        
        free(buffer[0]);

	colormap = new Color[jpegObject.actual_number_of_colors];
	if (colormap == NULL) {
		jpeg_finish_decompress(&jpegObject);
		delete pixels;
		return -1;
	}
	nbColors = jpegObject.actual_number_of_colors;

	for(n=0; n < nbColors; n++)
	{
		colormap[n].red = jpegObject.colormap[0][n];
		colormap[n].green = jpegObject.colormap[1][n];
		colormap[n].blue = jpegObject.colormap[2][n];
	}

	status = jpeg_finish_decompress(&jpegObject);

	return 0;
}

// Just init JPEG object and read JPEG Tables

int
Bitmap::readJpegTables(unsigned char *stream)
{
	if (haveTables) {
		//Error, it has already been initialized
		return -1;
	}

	// Setup error handler
	jpegObject.err = jpeg_std_error(&jpegErrorMgr.pub);
	jpegErrorMgr.pub.error_exit = errorExit;

	if (setjmp(jpegErrorMgr.setjmp_buffer)) {
		// JPEG data Error
		jpeg_destroy_decompress(&jpegObject);
		return -1;
	}

	// Set current stream pointer to stream
	inputData = stream;

	// Here it's Ok

	jpeg_create_decompress(&jpegObject);

	// Setup source manager structure
	jpegSourceManager.init_source = initSource;
	jpegSourceManager.fill_input_buffer = fillInputBuffer;
	jpegSourceManager.skip_input_data = skipInputData;
	jpegSourceManager.resync_to_restart = resyncToRestart;
	jpegSourceManager.term_source = termSource;

	// Set default source manager
	jpegObject.src = &jpegSourceManager;

	jpeg_read_header(&jpegObject, FALSE);

	haveTables = 1;

	return 0;
}

int
Bitmap::buildFromZlibData(unsigned char *buffer, int width, int height, int format, int tableSize, int tableHasAlpha)
{
	z_stream	stream;
	int		status;
	unsigned char  *data;
	int		elementSize;

#if PRINT&1
        printf("flash: loading with zlib\n");
#endif

	this->width = width;
	this->height = height;
        this->bpl = width;

	if (tableHasAlpha) {
		elementSize = 4;	// Cmap is RGBA
	} else {
		elementSize = 3;	// Cmap is RGB
	}

	stream.next_in = buffer;
	stream.avail_in = 1;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;

	tableSize++;

	// Uncompress Color Table
	if (format == 3) {
		unsigned char *colorTable;
		long n;

		// Ajust width for 32 bit padding
		width = (width+3)/4*4;
		this->width = width;
		this->bpl = width;

		depth = 1;
		colorTable = new unsigned char[tableSize*elementSize];
		if (colorTable == NULL) {
			return -1;
		}

		stream.next_out = colorTable;
		stream.avail_out = tableSize*elementSize;

		inflateInit(&stream);

		while (1) {
			status = inflate(&stream, Z_SYNC_FLUSH);
			if (status == Z_STREAM_END) {
					break;
			}
			if (status != Z_OK) {
				printf("Zlib cmap error : %s\n", stream.msg);
				return -1;
			}
			stream.avail_in = 1;
			// Colormap if full
			if (stream.avail_out == 0) {
				break;
			}
		}

		nbColors = tableSize;

		colormap = new Color[nbColors];
		if (colormap == NULL) {
			delete colorTable;
			return -1;
		}

		for(n=0; n < nbColors; n++) {
			colormap[n].red = colorTable[n*elementSize+0];
			colormap[n].green = colorTable[n*elementSize+1];
			colormap[n].blue = colorTable[n*elementSize+2];
			if (tableHasAlpha) {
				colormap[n].alpha = colorTable[n*elementSize+3];
			}
		}

		delete colorTable;

	} else if (format == 4) {
		depth = 2;
		width = (width+1)/2*2;
		this->bpl = width;
	} else if (format == 5) {
		depth = 4;
	}

	data = new unsigned char[depth*width*height];
	if (data == NULL) {
		if (colormap) delete colormap;
		return -1;
	}

	stream.next_out = data;
	stream.avail_out = depth*width*height;

	if (format != 3) {
		status = inflateInit(&stream);
	}

	while (1) {
		status = inflate(&stream, Z_SYNC_FLUSH) ;
		if (status == Z_STREAM_END) {
				break;
		}
		if (status != Z_OK) {
			printf("Zlib data error : %s\n", stream.msg);
			delete data;
			return -1;
		}
		stream.avail_in = 1;
	}

	inflateEnd(&stream);

	pixels = new unsigned char [height*width];
	if (pixels == NULL) {
		if (colormap) delete colormap;
		delete data;
		return -1;
	}

	if (format != 3) {
		int n,c;
		unsigned char r,g,b,a;
		unsigned char *ptr;

                r = g = b = a = 0; /* to supress warnings */

		nbColors = 0;
		colormap = new Color[256];
		if (colormap == NULL) {
			delete data;
			delete pixels;
			return -1;
		}
                memset(colormap, 0, 256 * sizeof(Color));
		ptr = pixels;
		
		for(n=0; n < width*height*depth; n+=depth,ptr++) {
                    
			switch (format) {
				case 4:
					a = 1;
					r = (data[n] & 0x78)<<1;
					g = ((data[n] & 0x03)<<6) | (data[n+1] & 0xc0)>>2;
					b = (data[n+1] & 0x1e)<<3;
					break;
				case 5:
					a = data[n];
					// Reduce color dynamic range
					r = data[n+1]&0xe0;
					g = data[n+2]&0xe0;
					b = data[n+3]&0xe0;
					break;
			}
			for(c=0; c < nbColors; c++) {
				if (r == colormap[c].red
				&&  g == colormap[c].green
				&&  b == colormap[c].blue) {
					*ptr = c;
					break;
				}
			}
			if (c == nbColors) {
				if (nbColors == 256) continue;
				nbColors++;
				if (nbColors == 256) {
					//printf("Colormap entries exhausted. After %d scanned pixels\n", n/4);
				}
				colormap[c].alpha = a;
				colormap[c].red   = r;
				colormap[c].green = g;
				colormap[c].blue  = b;
				*ptr = c;
			}
		}
	} else {
		memcpy(pixels, data, width*height);
		if (tableHasAlpha) {
			int n;
			unsigned char *ptr, *alpha;

			alpha_buf = (unsigned char *)malloc(width*height);
			ptr = data;
			alpha = alpha_buf;
			for(n=0; n < width*height; n++, ptr++, alpha++) {
				*alpha = colormap[*ptr].alpha;
			}
		}
	}

	delete data;
	return 0;
}

