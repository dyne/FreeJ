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

#include "graphic16.h"

extern unsigned char SQRT[];

#define FULL_AA

#define PRINT 0

typedef unsigned short TYPE;

GraphicDevice16::GraphicDevice16(FlashDisplay *fd) : GraphicDevice(fd)
{
}

long
GraphicDevice16::allocColor(Color color)
{
	return (color.red >> 3)<<11 | (color.green>>2)<<5 | (color.blue>>3);
}

void
GraphicDevice16::clearCanvas()
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
GraphicDevice16::fillLineAA(FillStyleDef *f, long y, long start, long end)
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
GraphicDevice16::fillLine(FillStyleDef *f, long y, long start, long end)
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

void
GraphicDevice16::fillLineBitmap(FillStyleDef *f, long y, long start, long end)
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
GraphicDevice16::fillLineLG(Gradient *grad, long y, long start, long end)
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

void
GraphicDevice16::fillLineRG(Gradient *grad, long y, long start, long end)
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
			    r = SQRT[dist2];	
			}
			*point = mix_alpha(*point, (TYPE)ramp[r].pixel, start_alpha + end_alpha - 255);
		} else {
		    if (start_alpha < 255) {
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r = SQRT[dist2];	
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
GraphicDevice16::drawLine(long x1, long y1, long x2, long y2, long width)
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
