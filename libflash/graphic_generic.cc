////////////////////////////////////////////////////////////
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

extern unsigned char SQRT[];

#define FULL_AA

#define PRINT 0

typedef unsigned short TYPE;

static char cmp8[256];	// 8bit colormap

long
allocColor15(Color color)
{
	return (color.red >> 3)<<10 | (color.green>>3)<<5 | (color.blue>>3);
}

#if 0
long
allocColor16_646(Color color)
{
	return (color.red >> 2)<<10 | (color.green>>4)<<6 | (color.blue>>2);
}
#endif

long
allocColor16_565(Color color)
{
	return (color.red >> 3)<<11 | (color.green>>2)<<5 | (color.blue>>3);
}

long
allocColor24_32(Color color)
{
	return (color.red)<<16 | (color.green)<<8 | color.blue;
}

long
allocColor8(Color color)
{
	return cmp8[(color.red>>6)<<4 | (color.green>>6)<<2 | (color.blue>>6)];
}

// Public

GraphicDevice::GraphicDevice(FlashDisplay *fd)
{
	int depth;

	flashDisplay = fd;

	bgInitialized = 0;

	// Reset flash refresh flag
	flashDisplay->flash_refresh = 0;

        /* 16 bits, RGB565 */
	redMask = 0xF800;
	greenMask = 0x07E0;
	blueMask = 0x001F;
        bpp = 2;
        depth = 16;

        /* should be the actual window size */
	targetWidth = fd->width;
	targetHeight = fd->height;
        bpl = fd->bpl;

#if PRINT
	printf("Target Width  = %d\n", targetWidth);
	printf("Target Height = %d\n", targetHeight);
#endif

	zoom = FRAC;
	movieWidth = targetWidth;
	movieHeight = targetHeight;

	viewPort.xmin = 0;
	viewPort.xmax = targetWidth-1;
	viewPort.ymin = 0;
	viewPort.ymax = targetHeight-1;

	switch (bpp) {
		case 1:
			allocColor = allocColor8;
			redMask = 0xe0;
			greenMask = 0x18;
			blueMask = 0x07;
			break;
		case 2:
			if (depth == 16) {
				allocColor = allocColor16_565;
			} else
			if (depth == 15) {
				allocColor = allocColor15;
			}
			break;
		case 3:
		case 4:
			allocColor = allocColor24_32;
			break;
	}

	canvasBuffer = (unsigned char *) fd->pixels;

	adjust = new Matrix;
	foregroundColor.red = 0;
	foregroundColor.green = 0;
	foregroundColor.blue = 0;
	foregroundColor.alpha = ALPHA_OPAQUE;

	backgroundColor.red = 0;
	backgroundColor.green = 0;
	backgroundColor.blue = 0;
	backgroundColor.alpha = ALPHA_OPAQUE;

	showMore = 0;

	setClipping(0);	// Reset
	setClipping(1);
 
        /* polygon rasterizer : handle memory errors ! */

        height = targetHeight;
        segs = (Segment **)malloc(height * sizeof(Segment *));
        memset(segs, 0, height * sizeof(Segment *));
        ymin = height;
        ymax = -1;

        seg_pool = (Segment *)malloc(NB_SEGMENT_MAX * sizeof(Segment));
        seg_pool_cur = seg_pool;
}

GraphicDevice::~GraphicDevice()
{
    free(segs);
    free(seg_pool);
    
    if (adjust) {
        delete adjust;
    }
}

///////////// PLATFORM INDEPENDENT
Color *
GraphicDevice::getColormap(Color *old, long n, Cxform *cxform)
{
	Color *newCmp;

	newCmp = new Color[n];
	if (newCmp == NULL) return NULL;

	if (cxform) {
		for(long i = 0; i < n; i++)
		{
			newCmp[i] = cxform->getColor(old[i]);
			newCmp[i].pixel = allocColor(newCmp[i]);
		}
	} else {
		for(long i = 0; i < n; i++)
		{
			newCmp[i].pixel = allocColor(old[i]);
		}
	}

	return newCmp;
}

///////////// PLATFORM INDEPENDENT
long
GraphicDevice::getHeight()
{
	return targetHeight;
}

///////////// PLATFORM INDEPENDENT
long
GraphicDevice::getWidth()
{
	return targetWidth;
}

///////////// PLATFORM INDEPENDENT
Color
GraphicDevice::getForegroundColor()
{
	return foregroundColor;
}

void
GraphicDevice::setForegroundColor(Color color)
{
	foregroundColor = color;
}

///////////// PLATFORM INDEPENDENT
Color
GraphicDevice::getBackgroundColor()
{
	return backgroundColor;
}

///////////// PLATFORM INDEPENDENT
int
GraphicDevice::setBackgroundColor(Color color)
{
	if (bgInitialized == 0) {
		backgroundColor = color;
		clearCanvas();
		bgInitialized = 1;
		return 1;
	}
	return 0;
}

///////////// PLATFORM INDEPENDENT
void
GraphicDevice::setMovieDimension(long width, long height)
{
	float xAdjust, yAdjust;

	movieWidth = width;
	movieHeight = height;

	xAdjust = (float)targetWidth*zoom/(float)width;
	yAdjust = (float)targetHeight*zoom/(float)height;

	if (xAdjust < yAdjust) {
		adjust->a = xAdjust;
		adjust->d = xAdjust;
                adjust->ty = ((targetHeight*zoom) - (long)(height * xAdjust))/2;
		viewPort.ymin = adjust->ty/zoom;
		viewPort.ymax = targetHeight-viewPort.ymin-1;
	} else {
		adjust->a = yAdjust;
		adjust->d = yAdjust;
                adjust->tx = ((targetWidth*zoom) - (long)(width * yAdjust))/2;
		viewPort.xmin = adjust->tx/zoom;
		viewPort.xmax = targetWidth-viewPort.xmin-1;
	}

	if (viewPort.xmin < 0) viewPort.xmin = 0;
	if (viewPort.ymin < 0) viewPort.ymin = 0;
	if (viewPort.xmax >= targetWidth) viewPort.xmax = targetWidth-1;
	if (viewPort.ymax >= targetHeight) viewPort.ymax = targetHeight-1;
}

///////////// PLATFORM INDEPENDENT
void
GraphicDevice::setMovieZoom(int z)
{
	z *= FRAC;
	if (z <= 0 || z > 100) return;
	zoom = z;
	setMovieDimension(movieWidth,movieHeight);
}

///////////// PLATFORM INDEPENDENT
void
GraphicDevice::setMovieOffset(long x, long y)
{
	adjust->tx = -zoom*x;
	adjust->ty = -zoom*y;
}

///////////// PLATFORM INDEPENDENT
void
GraphicDevice::clearCanvas()
{
    TYPE  pixel;
    TYPE *point,*p;
    long                 h, w,n;

    if (!bgInitialized) return;

    pixel = allocColor(backgroundColor);

    point = (TYPE *)(canvasBuffer + clip_rect.ymin * bpl) + clip_rect.xmin;
    w = clip_rect.xmax - clip_rect.xmin;
    h = clip_rect.ymax - clip_rect.ymin;

    while (h--) {
        p = point;
        n = w;
        while (n--) {
            *p++ = pixel;
        }

        point = (TYPE *)((char *)point + bpl);
    }

    flashDisplay->flash_refresh = 1;
    flashDisplay->clip_x = clip_rect.xmin;
    flashDisplay->clip_y = clip_rect.ymin;
    flashDisplay->clip_width  = clip_rect.xmax-clip_rect.xmin;
    flashDisplay->clip_height = clip_rect.ymax-clip_rect.ymin;
}

///////////// PLATFORM INDEPENDENT
long
GraphicDevice::clip(long &y, long &start, long &end)
{
    long xmin,xend;

    if (y < clip_rect.ymin ||
        y >= clip_rect.ymax) return 1;
    if (end <= start)
        return 1;
    xmin = clip_rect.xmin * FRAC;
    xend = clip_rect.xmax * FRAC;

    if (end <= xmin || start >= xend) return 1;

    if (start < xmin) start = xmin;
    if (end > xend) end = xend;

    return 0;
}

#define RED_MASK   0xF800
#define GREEN_MASK 0x07E0
#define BLUE_MASK  0x001F

/* alpha = 0 : select c1, alpha = 255 select c2 */
static inline unsigned long
mix_alpha(unsigned long c1, 
                                      unsigned long c2, int alpha)
{
	long r1,r2,r;
	long g1,g2,g;
	long b1,b2,b;

	r1 = c1 & RED_MASK;
	r2 = c2 & RED_MASK;
	r = (((r2-r1)*alpha + r1 * 256) >> 8) & RED_MASK;

	g1 = c1 & GREEN_MASK;
	g2 = c2 & GREEN_MASK;
	g = (((g2-g1)*alpha + g1 * 256) >> 8) & GREEN_MASK;

	b1 = c1 & BLUE_MASK;
	b2 = c2 & BLUE_MASK;
	b = (((b2-b1)*alpha + b1 * 256) >> 8) & BLUE_MASK;

	return (r|g|b);
}

void
GraphicDevice::fillLineAA(FillStyleDef *f, long y, long start, long end)
{
    register long   n;
    TYPE *line;
    TYPE *point,pixel;
    unsigned int alpha, start_alpha,end_alpha;
    
    if (clip(y,start,end)) return;
    
    line = (TYPE *)(canvasBuffer + bpl*y);
    
    alpha = f->color.alpha;
    pixel = f->color.pixel;
    
    if (alpha == ALPHA_OPAQUE) {

        start_alpha = 255 - ((start & (FRAC-1)) << (8-FRAC_BITS));
        end_alpha = (end & (FRAC-1)) << (8-FRAC_BITS);
        
        start >>= FRAC_BITS;
        end >>= FRAC_BITS;
        
        point = &line[start];

        if (start == end) {
            *point = mix_alpha(*point, pixel, start_alpha + end_alpha - 255);
        } else {
            n = end-start;
            if (start_alpha < 255) {
                *point = mix_alpha(*point, pixel, start_alpha);
                point++;
                n--;
            }
            while (n > 0) {
                *point = pixel;
                point++;
                n--;
            }
            if (end_alpha > 0) {
                *point = mix_alpha(*point, pixel, end_alpha);
            }
        }
    } else {

        start_alpha = 255 - ((start & (FRAC-1)) << (8-FRAC_BITS));
        end_alpha = (end & (FRAC-1)) << (8-FRAC_BITS);

        start >>= FRAC_BITS;
        end >>= FRAC_BITS;
        
        point = &line[start];
        
        if (start == end) {
            *point = mix_alpha(*point, pixel, 
                               ((start_alpha + end_alpha - 255) * alpha) >> 8);
        } else {
            n = end-start;
            if (start_alpha < 255) {
                *point = mix_alpha(*point, pixel, (start_alpha * alpha) >> 8);
                point++;
                n--;
            }
            while (n > 0) {
                *point = mix_alpha(*point, pixel, alpha);
                point++;
                n--;
            }
            if (end_alpha > 0) {
                *point = mix_alpha(*point, pixel, (end_alpha * alpha) >> 8);
            }
        }
    }
}

void
GraphicDevice::fillLine(FillStyleDef *f, long y, long start, long end)
{
	register long   n;
        TYPE *line,*point;
        TYPE pixel;
        unsigned int alpha;

	if (clip(y,start,end)) return;

        start >>= FRAC_BITS;
        end >>= FRAC_BITS;

	line = (TYPE *)(canvasBuffer + bpl*y);
	point = &line[start];			
	n = end-start;				
        pixel = f->color.pixel;
        alpha = f->color.alpha;
        if (alpha == ALPHA_OPAQUE) {
            while (n--) { 
		*point = pixel;
		point++;			
            }
        } else {
            while (n--) { 
		*point = mix_alpha(*point, pixel, alpha);
		point++;			
            }
        }
}

/* 16 bit assumed... easy to change */
void
GraphicDevice::fillLineBitmap(FillStyleDef *f, long y, long start, long end)
{
    int n;
    long x1,y1,dx,dy;
    Matrix *m = &f->bitmap_matrix;
    Bitmap *b = f->bitmap;
    unsigned char *pixels;
    TYPE *p;
    Color *cmap;
    long pixbpl;
    TYPE pixel;
    int offset;
    unsigned char *alpha_table;

    /* safety test) */
    if (!b) return;

    if (clip(y,start,end)) return;
    
    start /= FRAC;
    end /= FRAC;
    n = end - start;
    p = (TYPE *) (this->canvasBuffer + this->bpl*y + start * 2);
    
    /* the coordinates in the image are normalized to 16 bits */
    x1 = (long) (m->a * start + m->b * y + m->tx);
    y1 = (long) (m->c * start + m->d * y + m->ty);
    dx = (long) (m->a);
    dy = (long) (m->c);
    
    pixels = b->pixels;
    pixbpl = b->bpl;
    cmap = f->cmap;

    if (b->alpha_buf == NULL) {
        while (n) {
            if (x1 >= 0 && y1 >= 0 && 
                (x1 >> 16) < b->width && (y1 >> 16) < b->height) {
                
                pixel = cmap[pixels[(y1 >> 16) * pixbpl + (x1 >> 16)]].pixel;
                *p = pixel;
            }
            x1 += dx;
            y1 += dy;
            p++;
            n--;
        }
    } else if (f->alpha_table) {
        alpha_table = f->alpha_table;
        while (n) {
            if (x1 >= 0 && y1 >= 0 && 
                (x1 >> 16) < b->width && (y1 >> 16) < b->height) {
                
                offset = (y1 >> 16) * pixbpl + (x1 >> 16);
                pixel = cmap[pixels[offset]].pixel;
                *p = mix_alpha(*p, pixel, alpha_table[b->alpha_buf[offset]]);
            }
            x1 += dx;
            y1 += dy;
            p++;
            n--;
        }
    } else {
        while (n) {
            if (x1 >= 0 && y1 >= 0 && 
                (x1 >> 16) < b->width && (y1 >> 16) < b->height) {
                
                offset = (y1 >> 16) * pixbpl + (x1 >> 16);
                pixel = cmap[pixels[offset]].pixel;
                *p = mix_alpha(*p, pixel, b->alpha_buf[offset]);
            }
            x1 += dx;
            y1 += dy;
            p++;
            n--;
        }
    }
}

void
GraphicDevice::fillLineLG(Gradient *grad, long y, long start, long end)
{
	long dr,r,v,r2;
	register long n;
	TYPE *line;
	TYPE *point;
        Color *cp,*ramp;
        Matrix *m = &grad->imat;
        unsigned int start_alpha,end_alpha;

	if (clip(y,start,end)) return;

        start_alpha = 255 - ((start & (FRAC-1)) << (8-FRAC_BITS));
        end_alpha = (end & (FRAC-1)) << (8-FRAC_BITS);
        
	start /= FRAC;
	end /= FRAC;

	n = end-start;

        r = (long) (m->a * start + m->b * y + m->tx);
        dr = (long) (m->a);

        ramp = grad->ramp;

        line = (TYPE *)(canvasBuffer + bpl*y);
	point = &line[start];	

        r2 = r + n * dr;
        if ( ((r | r2) & ~255) == 0 ) {
            if (!grad->has_alpha) {
#ifdef FULL_AA
		if (start_alpha < 255) {
                    v = r>>16;
                    *point = mix_alpha(*point, (TYPE)ramp[v].pixel, start_alpha);
                    point++;
                    r += dr;
		    n--;
		}
#endif /* FULL_AA */
                while (n>0) {
                    v = r>>16;
                    *point = (TYPE)ramp[v].pixel;	
                    point++;				
                    r += dr;				
		    n--;
                }
#ifdef FULL_AA
		if (end_alpha > 0) {
                    v = r>>16;
                    *point = mix_alpha(*point, (TYPE)ramp[v].pixel, end_alpha);
		}
#endif /* FULL_AA */
            } else {
                while (n--) {
                    v = r>>16;
                    cp = &ramp[v];
                    *point = mix_alpha(*point, cp->pixel, cp->alpha);
                    point++;
                    r += dr;
                }
            }
        } else {
            if (!grad->has_alpha) {
#ifdef FULL_AA
		if (start_alpha < 255) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    *point = mix_alpha(*point, (TYPE)ramp[v].pixel, start_alpha);
                    point++;
                    r += dr;
		    n--;
		}
#endif /* FULL_AA */
                while (n>0) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    *point = (TYPE)ramp[v].pixel;	
                    point++;				
                    r += dr;				
		    n--;
                }
#ifdef FULL_AA
		if (end_alpha > 0) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    *point = mix_alpha(*point, (TYPE)ramp[v].pixel, end_alpha);
		}
#endif /* FULL_AA */
            } else {
                while (n--) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    cp = &ramp[v];
                    *point = mix_alpha(*point, cp->pixel, cp->alpha);
                    point++;
                    r += dr;
                }
            }
        }
}

///////////// PLATFORM INDEPENDENT
void
GraphicDevice::fillLineRG(Gradient *grad, long y, long start, long end)
{
	long X,dx,r,Y,dy;
	long dist2;
	register long   n;
        Color *cp,*ramp;
	TYPE *line;							
	TYPE *point;							
        Matrix *m = &grad->imat;
        unsigned int start_alpha,end_alpha;

	if (clip(y,start,end)) return;

        start_alpha = 255 - ((start & (FRAC-1)) << (8-FRAC_BITS));
        end_alpha = (end & (FRAC-1)) << (8-FRAC_BITS);
        
	start /= FRAC;
	end /= FRAC;

	n = end-start;
        
        X = (long) (m->a * start + m->b * y + m->tx);
        Y = (long) (m->c * start + m->d * y + m->ty);
        dx = (long) (m->a);
        dy = (long) (m->c);

        ramp = grad->ramp;
									
	line = (TYPE *)(canvasBuffer + bpl*y);
	point = &line[start];
			     
        if (!grad->has_alpha) {
#ifdef FULL_AA
		if (start == end) {
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r= SQRT[dist2];	
			}
			*point = mix_alpha(*point, (TYPE)ramp[r].pixel, start_alpha + end_alpha - 255);
		} else {
		    if (start_alpha < 255) {
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r= SQRT[dist2];	
			}
			*point = mix_alpha(*point, (TYPE)ramp[r].pixel, start_alpha);
			point++;
			X += dx;						
			Y += dy;						
			n--;
		    }
#endif /* FULL_AA */
		    while (n>0) {					
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r= SQRT[dist2];	
			}
			*point = (TYPE)ramp[r].pixel;
			point++;				     
			X += dx;						
			Y += dy;						
			n--;
		    }		
#ifdef FULL_AA
		    if (end_alpha > 0) {
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r= SQRT[dist2];	
			}
			*point = mix_alpha(*point, (TYPE)ramp[r].pixel, end_alpha);
		    }
		}
#endif /* FULL_AA */

        } else {
            while (n--) {					
		dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
		if ((unsigned long)dist2 >= 65536) {
                    r = 255;					
		} else {						
                    r= SQRT[dist2];	
		}
                cp = &ramp[r];
		*point = mix_alpha(*point, cp->pixel, cp->alpha);
		point++;
		X += dx;						
		Y += dy;						
            }		
        }
}

void
GraphicDevice::drawBox(long x1, long y1, long x2, long y2)
{
    int i;

    for(i=0;i<FRAC*2;i++) {
        drawLine(x1+i, y1+i, x2-i, y1+i, 0);
        drawLine(x1+i, y2-i, x2-i, y2-i, 0);

        drawLine(x1+i, y1+i+1, x1+i, y2-i-1, 0);
        drawLine(x2-i, y1+i+1, x2-i, y2-i-1, 0);
    }
}

void
GraphicDevice::drawLine(long x1, long y1, long x2, long y2, long width)
{
    int n,adr,dx,dy,sx,color;
    register int a;
    register TYPE *pp;
    int alpha;

    x1 = (x1) >> FRAC_BITS;
    y1 = (y1) >> FRAC_BITS;
    x2 = (x2) >> FRAC_BITS;
    y2 = (y2) >> FRAC_BITS;
    
    if (y1 > y2 || (y1 == y2 && x1 > x2)) {
        long tmp;

        tmp=x1;
        x1=x2;
        x2=tmp;

        tmp=y1;
        y1=y2;
        y2=tmp;
    }

    if (y1 == y2 && (y1 < clip_rect.ymin || y1 > clip_rect.ymax)) return;
    if (x1 == x2 && (x1 < clip_rect.xmin || x1 > clip_rect.xmax)) return;
    if (x1 == x2 && y1 == y2) return;	// Bad !!!

    if (y1 < clip_rect.ymin && y1 != y2) {
	x1 += (x2-x1)*(clip_rect.ymin-y1)/(y2-y1);
	y1 = clip_rect.ymin;
    }

    if (y2 > clip_rect.ymax && y1 != y2) {
	x2 -= (x2-x1)*(y2-clip_rect.ymax)/(y2-y1);
	y2 = clip_rect.ymax;
    }

    if (x1 < x2) {
	    if (x1 < clip_rect.xmin && x1 != x2) {
		y1 += (y2-y1)*(clip_rect.xmin-x1)/(x2-x1);
		x1 = clip_rect.xmin;
	    }

	    if (x2 > clip_rect.xmax && x1 != x2) {
		y2 -= (y2-y1)*(x2-clip_rect.xmax)/(x2-x1);
		x2 = clip_rect.xmax;
	    }
    }

    if (x1 > x2) {
	    if (x2 < clip_rect.xmin && x2 != x1) {
		y2 -= (y2-y1)*(clip_rect.xmin-x2)/(x1-x2);
		x2 = clip_rect.xmin;
	    }

	    if (x1 > clip_rect.xmax && x2 != x1) {
		y1 += (y2-y1)*(x1-clip_rect.xmax)/(x1-x2);
		x1 = clip_rect.xmax;
	    }
    }

    // Check again
    if (x1 == x2 && y1 == y2) return;
    if (x1 < clip_rect.xmin || x2 < clip_rect.xmin) return;
    if (y1 < clip_rect.ymin || y2 < clip_rect.ymin) return;
    if (x1 > clip_rect.xmax || x2 > clip_rect.xmax) return;
    if (y1 > clip_rect.ymax || y2 > clip_rect.ymax) return;

    sx=bpl >> 1;
    adr=(y1 * sx + x1);
    pp = (TYPE *)canvasBuffer + adr;
    
    dx = x2 - x1;
    dy = y2 - y1;

    color = allocColor(foregroundColor);
    alpha = foregroundColor.alpha;

    if (alpha == ALPHA_OPAQUE) {

#define PUTPIXEL() 				\
  {						\
      *pp=color;		                \
  }

#define DRAWLINE(dx,dy,inc_1,inc_2) \
    n=dx;\
    a=2*dy-dx;\
    dy=2*dy;\
    dx=2*dx-dy;\
	 do {\
      PUTPIXEL();\
			if (a>0) { pp+=(inc_1); a-=dx; }\
			else { pp+=(inc_2); a+=dy; }\
	 } while (--n >= 0);

/* fin macro */

  if (dx == 0 && dy == 0) {
    PUTPIXEL();
  } else if (dx > 0) {
    if (dx >= dy) {
      DRAWLINE(dx, dy, sx + 1, 1);
    } else {
      DRAWLINE(dy, dx, sx + 1, sx);
    }
  } else {
    dx = -dx;
    if (dx >= dy) {
      DRAWLINE(dx, dy, sx - 1, -1);
    } else {
      DRAWLINE(dy, dx, sx - 1, sx);
    }
  }


#undef DRAWLINE
#undef PUTPIXEL
    } else {
#define PUTPIXEL() 				\
  {						\
      *pp=mix_alpha(*pp,color,alpha);	        \
  }

#define DRAWLINE(dx,dy,inc_1,inc_2) \
    n=dx;\
    a=2*dy-dx;\
    dy=2*dy;\
    dx=2*dx-dy;\
	 do {\
      PUTPIXEL();\
			if (a>0) { pp+=(inc_1); a-=dx; }\
			else { pp+=(inc_2); a+=dy; }\
	 } while (--n >= 0);

/* fin macro */

  if (dx == 0 && dy == 0) {
    PUTPIXEL();
  } else if (dx > 0) {
    if (dx >= dy) {
      DRAWLINE(dx, dy, sx + 1, 1);
    } else {
      DRAWLINE(dy, dx, sx + 1, sx);
    }
  } else {
    dx = -dx;
    if (dx >= dy) {
      DRAWLINE(dx, dy, sx - 1, -1);
    } else {
      DRAWLINE(dy, dx, sx - 1, sx);
    }
  }


#undef DRAWLINE
#undef PUTPIXEL
    }
}

/************************************************************************/

/* polygon rasteriser */

inline Segment *
GraphicDevice::allocSeg()
{
    Segment *seg;

    if ( (seg_pool_cur - seg_pool) >= NB_SEGMENT_MAX )
        return NULL;
    seg = seg_pool_cur++;

    return seg;
}

/* add a segment to the current path */
void
GraphicDevice::addSegment(long x1, long y1, long x2, long y2,
                          FillStyleDef *f0,
                          FillStyleDef *f1,
                          int aa)
{
    Segment *seg,**segs;
    long dX, X, Y, ymin, ymax, tmp;
    FillStyleDef *ff;

    if ( y1 == y2 ) {
        return;
    }

    if (y1 < y2) {
        ymin = y1;
        ymax = y2;
        ff = f0;
        f0 = f1;
        f1 = ff;
    } else {
        ymin = y2;
        ymax = y1;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    if (ymax>>FRAC_BITS < clip_rect.ymin) {
    	return;
    }
    if (ymin>>FRAC_BITS > clip_rect.ymax) {
    	return;
    }

    X = x1 << SEGFRAC;
    dX = ((x2 - x1)<<SEGFRAC)/(ymax-ymin);

    if (ymin < 0) {
        X += dX * (-ymin);
        ymin = 0;
    }

    Y = (ymin + (FRAC-1)) & ~(FRAC-1);
    if (Y > ymax) {
        //printf("Elimine @ y = %d   ymin = %d, ymax = %d\n", Y, ymin, seg->ymax);
        return;
    }
    X += dX * (Y-ymin);

    Y >>= FRAC_BITS;
    if (Y >= clip_rect.ymax) {
        return;
    }

    seg = allocSeg();
    if (seg == NULL) {
        return;
    }

    seg->next = 0;
    seg->nextValid = 0;
    seg->aa = aa;
    seg->ymax = ymax;
    seg->x1 = x1;
    seg->x2 = x2;
    seg->X = X;
    seg->dX = dX;
    seg->fs[0] = f0;
    seg->fs[1] = f1;

    if (Y < this->ymin) this->ymin = Y;
    ymax = (seg->ymax + FRAC - 1) >> FRAC_BITS;
    if (ymax >= this->height) ymax = this->height-1;
    if (ymax > this->ymax) this->ymax = ymax;

    segs = this->segs;

    if (segs[Y] == 0) {
        segs[Y] = seg;
    } else {
        Segment *s,*prev;

        prev = 0;
        for(s = segs[Y]; s; prev = s, s = s->next) {
            if (s->X > seg->X) {
                if (prev) {
                    prev->next = seg;
                    seg->next = s;
                } else {
                    seg->next = segs[Y];
                    segs[Y] = seg;
                }
                break;
            }
        }
        if (s == 0) {
            prev->next = seg;
            seg->next = s;
        }
    }
}

inline Segment *
GraphicDevice::progressSegments(Segment * curSegs, long y)
{
    Segment *seg,*prev;

    // Update current segments
    seg = curSegs;
    prev = 0;
    while(seg)
    {
        if ((y*FRAC) > seg->ymax) {
            // Remove this segment, no more valid
            if (prev) {
                prev->nextValid = seg->nextValid;
            } else {
                curSegs = seg->nextValid;
            }
            seg = seg->nextValid;
        } else {
            seg->X += seg->dX * FRAC;
            prev = seg;
            seg = seg->nextValid;
        }
    }
    return curSegs;
}

inline Segment *
GraphicDevice::newSegments(Segment *curSegs, Segment *newSegs)
{
    Segment *s,*seg,*prev;

    s = curSegs;
    prev = 0;

    // Check for new segments
    for (seg = newSegs; seg; seg=seg->next)
    {
        // Place it at the correct position according to X
        if (curSegs == 0) {
            curSegs = seg;
            seg->nextValid = 0;
        } else {
            for(; s; prev = s, s = s->nextValid)
            {
                if ( s->X > seg->X ||
                     ( (s->X == seg->X) && 
                       ( (seg->x1 == s->x1 && seg->dX < s->dX) ||
                         (seg->x2 == s->x2 && seg->dX > s->dX)
                         ))) {
                    // Insert before s
                    if (prev) {
                        seg->nextValid = s;
                        prev->nextValid = seg;
                    } else {
                        seg->nextValid = curSegs;
                        curSegs = seg;
                    }
                    break;
                }
            }
            // Append at the end
            if (s == 0) {
                prev->nextValid = seg;
                seg->nextValid = 0;
            }
        }

        s = seg;
    }

    return curSegs;
}

#if 0
static void
printSeg(Segment *seg)
{
    /*
    printf("Seg %08x : X = %5d, Ft = %d, Cl = %2x/%2x/%2x, Cr = %2x/%2x/%2x, x1=%5d, x2=%5d, ymin=%5d, ymax=%5d\n", seg,
        seg->X>>SEGFRAC,
        seg->right ? seg->right->type: -1,
        seg->left ? seg->left->color.red : -1,
        seg->left ? seg->left->color.green : -1,
        seg->left ? seg->left->color.blue : -1,
        seg->right ? seg->right->color.red : -1,
        seg->right ? seg->right->color.green : -1,
        seg->right ? seg->right->color.blue : -1,
        seg->x1, seg->x2, seg->ymin, seg->ymax);
    */
}
#endif

inline void
GraphicDevice::renderScanLine(long y, Segment *curSegs)
{
    Segment *seg;
    long width;
    int fi = 1;
    FillStyleDef *f;

    width = targetWidth * FRAC;

    if (curSegs && curSegs->fs[0] && curSegs->fs[1] == 0) {
        fi = 0;
    }
    for(seg = curSegs; seg && seg->nextValid; seg = seg->nextValid)
    {
        if (seg->nextValid->X <0) continue;
        if ((seg->X>>SEGFRAC) > width) break;
        f = seg->fs[fi];
        if (f) {
            switch (f->type) {
                case f_Solid:
                    if (seg->aa) {
                        fillLineAA(f, y, seg->X>>SEGFRAC, seg->nextValid->X>>SEGFRAC);
                    } else  {
                        fillLine(f, y, seg->X>>SEGFRAC, seg->nextValid->X>>SEGFRAC);
                    }
                    break;
                case f_TiledBitmap:
                case f_clippedBitmap:
                    fillLineBitmap(f, y, seg->X>>SEGFRAC, seg->nextValid->X>>SEGFRAC);
                    break;
                case f_LinearGradient:
                    fillLineLG(&f->gradient, y, seg->X>>SEGFRAC, seg->nextValid->X>>SEGFRAC);
                    break;
                case f_RadialGradient:
                    fillLineRG(&f->gradient, y, seg->X>>SEGFRAC, seg->nextValid->X>>SEGFRAC);
                    break;
	        case f_None:
		    break;
            }
        }
    }
}

/* draw the current path */
void
GraphicDevice::drawPolygon(void)
{
    long y;
    Segment *curSegs,*seg;

    // no segments ? 
    if (this->ymax == -1)
        return;

    // Foreach scanline
    curSegs = 0;
    for(y=this->ymin; y <= this->ymax; y++) {
        
        // Make X values progess and remove unuseful segments
        curSegs = progressSegments(curSegs, y);
        
        // Add the new segment starting at the y position.
        curSegs = newSegments(curSegs, this->segs[y]);
        
        // Render the scanline
        if (this->scan_line_func == NULL) {
            renderScanLine(y, curSegs);
        } else {
            for(seg = curSegs; seg && seg->nextValid; seg = seg->nextValid) {
                if (seg->nextValid->X >= seg->X) {
                    scan_line_func(this->scan_line_func_id, y, seg->X>>SEGFRAC, seg->nextValid->X>>SEGFRAC);
                }
            }
        }
    }

    /* free the segments */
    memset(this->segs + this->ymin, 0, 
           (this->ymax - this->ymin + 1) * sizeof(Segment *)); 
        
    this->ymax = -1;
    this->ymin = this->height;

    this->seg_pool_cur = this->seg_pool;
}

void
GraphicDevice::updateClippingRegion(Rect *rect)
{
	if (!clipping) return;

	transformBoundingBox(&clip_rect, adjust, rect, 1);
	clip_rect.xmin >>= FRAC_BITS;
	clip_rect.xmax >>= FRAC_BITS;
	clip_rect.ymin >>= FRAC_BITS;
	clip_rect.ymax >>= FRAC_BITS;

	clip_rect.xmin-=2;
	clip_rect.ymin-=2;
	clip_rect.xmax+=2;
	clip_rect.ymax+=2;

	if (clip_rect.xmin < viewPort.xmin) clip_rect.xmin = viewPort.xmin;
	if (clip_rect.xmax < viewPort.xmin) clip_rect.xmax = viewPort.xmin;
	if (clip_rect.ymin < viewPort.ymin) clip_rect.ymin = viewPort.ymin;
	if (clip_rect.ymax < viewPort.ymin) clip_rect.ymax = viewPort.ymin;

	if (clip_rect.xmax > viewPort.xmax) clip_rect.xmax = viewPort.xmax;
	if (clip_rect.ymax > viewPort.ymax) clip_rect.ymax = viewPort.ymax;
	if (clip_rect.xmin > viewPort.xmax) clip_rect.xmin = viewPort.xmax;
	if (clip_rect.ymin > viewPort.ymax) clip_rect.ymin = viewPort.ymax;
}

void
GraphicDevice::setClipping(int value)
{
	clipping = value;
	if (clipping == 0) {
		// Reset region
		clip_rect.xmin = viewPort.xmin;
		clip_rect.xmax = viewPort.xmax;
		clip_rect.ymin = viewPort.ymin;
		clip_rect.ymax = viewPort.ymax;
	}
}
