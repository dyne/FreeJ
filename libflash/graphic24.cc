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

#include "graphic24.h"

extern unsigned char SQRT[];

#define FULL_AA

#define PRINT 0

typedef unsigned char TYPE;
#define BPP 3

GraphicDevice24::GraphicDevice24(FlashDisplay *fd) : GraphicDevice(fd)
{
}

long
GraphicDevice24::allocColor(Color color)
{
	return 0;
}

void
GraphicDevice24::clearCanvas()
{
    TYPE *point,*p;
    long                 h, w,n;

    if (!bgInitialized) return;

    point = (TYPE *)(canvasBuffer + clip_rect.ymin * bpl) + clip_rect.xmin*BPP;
    w = clip_rect.xmax - clip_rect.xmin;
    h = clip_rect.ymax - clip_rect.ymin;

    while (h--) {
        p = point;
        n = w;
        while (n--) {
            *p++ = backgroundColor.blue;
            *p++ = backgroundColor.green;
            *p++ = backgroundColor.red;
        }

        point = (TYPE *)((char *)point + bpl);
    }

    flashDisplay->flash_refresh = 1;
    flashDisplay->clip_x = clip_rect.xmin;
    flashDisplay->clip_y = clip_rect.ymin;
    flashDisplay->clip_width  = clip_rect.xmax-clip_rect.xmin;
    flashDisplay->clip_height = clip_rect.ymax-clip_rect.ymin;
}

/* alpha = 0 : select c1, alpha = 255 select c2 */
static inline void mix_alpha(TYPE *c1, Color c2, int alpha)
{
	*c1 = (((c2.blue- (*c1))*alpha + (*c1) * 256) >> 8);
	c1++;
	*c1 = (((c2.green- (*c1))*alpha + (*c1) * 256) >> 8);
	c1++;
	*c1 = (((c2.red- (*c1))*alpha + (*c1) * 256) >> 8);
}

void
GraphicDevice24::fillLineAA(FillStyleDef *f, long y, long start, long end)
{
    register long   n;
    TYPE *line;
    TYPE *point;
    Color pixel;
    unsigned int alpha, start_alpha,end_alpha;
    
    if (clip(y,start,end)) return;
    
    line = (TYPE *)(canvasBuffer + bpl*y);
    
    alpha = f->color.alpha;
    pixel = f->color;
    
    if (alpha == ALPHA_OPAQUE) {

        start_alpha = 255 - ((start & (FRAC-1)) << (8-FRAC_BITS));
        end_alpha = (end & (FRAC-1)) << (8-FRAC_BITS);
        
        start >>= FRAC_BITS;
        end >>= FRAC_BITS;
        
        point = &line[start*BPP];

        if (start == end) {
            mix_alpha(point, pixel, start_alpha + end_alpha - 255);
        } else {
            n = end-start;
            if (start_alpha < 255) {
                mix_alpha(point, pixel, start_alpha);
                point += BPP;
                n--;
            }
            while (n > 0) {
                *point++ = pixel.blue;
                *point++ = pixel.green;
                *point++ = pixel.red;
                n--;
            }
            if (end_alpha > 0) {
                mix_alpha(point, pixel, end_alpha);
            }
        }
    } else {

        start_alpha = 255 - ((start & (FRAC-1)) << (8-FRAC_BITS));
        end_alpha = (end & (FRAC-1)) << (8-FRAC_BITS);

        start >>= FRAC_BITS;
        end >>= FRAC_BITS;
        
        point = &line[start*BPP];
        
        if (start == end) {
            mix_alpha(point, pixel, ((start_alpha + end_alpha - 255) * alpha) >> 8);
        } else {
            n = end-start;
            if (start_alpha < 255) {
                mix_alpha(point, pixel, (start_alpha * alpha) >> 8);
                point+=BPP;
                n--;
            }
            while (n > 0) {
                mix_alpha(point, pixel, alpha);
                point+=BPP;
                n--;
            }
            if (end_alpha > 0) {
                mix_alpha(point, pixel, (end_alpha * alpha) >> 8);
            }
        }
    }
}

void
GraphicDevice24::fillLine(FillStyleDef *f, long y, long start, long end)
{
	register long   n;
        TYPE *line,*point;
        Color pixel;
        unsigned int alpha;

	if (clip(y,start,end)) return;

        start >>= FRAC_BITS;
        end >>= FRAC_BITS;

	line = (TYPE *)(canvasBuffer + bpl*y);
	point = &line[start*BPP];			
	n = end-start;				
        alpha = f->color.alpha;
        pixel = f->color;
        if (alpha == ALPHA_OPAQUE) {
            while (n--) { 
		*point++ = pixel.blue;
		*point++ = pixel.green;
		*point++ = pixel.red;
            }
        } else {
            while (n--) { 
		mix_alpha(point, pixel, alpha);
		point+=BPP;			
            }
        }
}

void
GraphicDevice24::fillLineBitmap(FillStyleDef *f, long y, long start, long end)
{
    int n;
    long x1,y1,dx,dy;
    Matrix *m = &f->bitmap_matrix;
    Bitmap *b = f->bitmap;
    unsigned char *pixels;
    TYPE *p;
    Color *cmap;
    long pixbpl;
    Color pixel;
    int offset;
    unsigned char *alpha_table;

    /* safety test) */
    if (!b) return;

    if (clip(y,start,end)) return;
    
    start /= FRAC;
    end /= FRAC;
    n = end - start;
    p = (TYPE *) (canvasBuffer + bpl*y + start*BPP);
    
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
                
                pixel = cmap[pixels[(y1 >> 16) * pixbpl + (x1 >> 16)]];
                *p++ = pixel.blue;
                *p++ = pixel.green;
                *p++ = pixel.red;
            } else {
                p+=BPP;
	    }
            x1 += dx;
            y1 += dy;
            n--;
        }
    } else if (f->alpha_table) {
        alpha_table = f->alpha_table;
        while (n) {
            if (x1 >= 0 && y1 >= 0 && 
                (x1 >> 16) < b->width && (y1 >> 16) < b->height) {
                
                offset = (y1 >> 16) * pixbpl + (x1 >> 16);
                mix_alpha(p, cmap[pixels[offset]], alpha_table[b->alpha_buf[offset]]);
	    }
            p+=BPP;
            x1 += dx;
            y1 += dy;
            n--;
        }
    } else {
        while (n) {
            if (x1 >= 0 && y1 >= 0 && 
                (x1 >> 16) < b->width && (y1 >> 16) < b->height) {
                
                offset = (y1 >> 16) * pixbpl + (x1 >> 16);
                mix_alpha(p, cmap[pixels[offset]], b->alpha_buf[offset]);
            }
            p+=BPP;
            x1 += dx;
            y1 += dy;
            n--;
        }
    }
}

void
GraphicDevice24::fillLineLG(Gradient *grad, long y, long start, long end)
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
	point = &line[start*BPP];

        r2 = r + n * dr;
        if ( ((r | r2) & ~255) == 0 ) {
            if (!grad->has_alpha) {
#ifdef FULL_AA
		if (start_alpha < 255) {
                    v = r>>16;
                    mix_alpha(point, ramp[v], start_alpha);
                    point+=BPP;
                    r += dr;
		    n--;
		}
#endif /* FULL_AA */
                while (n>0) {
                    v = r>>16;
                    *point++ = ramp[v].blue;
                    *point++ = ramp[v].green;
                    *point++ = ramp[v].red;
                    r += dr;				
		    n--;
                }
#ifdef FULL_AA
		if (end_alpha > 0) {
                    v = r>>16;
                    mix_alpha(point, ramp[v], end_alpha);
		}
#endif /* FULL_AA */
            } else {
                while (n--) {
                    v = r>>16;
                    cp = &ramp[v];
                    mix_alpha(point, *cp, cp->alpha);
                    point+=BPP;
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
                    mix_alpha(point, ramp[v], start_alpha);
                    point+=BPP;
                    r += dr;
		    n--;
		}
#endif /* FULL_AA */
                while (n>0) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    *point++ = ramp[v].blue;
                    *point++ = ramp[v].green;
                    *point++ = ramp[v].red;
                    r += dr;				
		    n--;
                }
#ifdef FULL_AA
		if (end_alpha > 0) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    mix_alpha(point, ramp[v], end_alpha);
		}
#endif /* FULL_AA */
            } else {
                while (n--) {
                    v = r>>16;
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    cp = &ramp[v];
                    mix_alpha(point, *cp, cp->alpha);
                    point+=BPP;
                    r += dr;
                }
            }
        }
}

void
GraphicDevice24::fillLineRG(Gradient *grad, long y, long start, long end)
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
	point = &line[start*BPP];
			     
        if (!grad->has_alpha) {
#ifdef FULL_AA
		if (start == end) {
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r= SQRT[dist2];	
			}
			mix_alpha(point, ramp[r], start_alpha + end_alpha - 255);
		} else {
		    if (start_alpha < 255) {
			dist2 = ((X>>16)*(X>>16))+((Y>>16)*(Y>>16));
			if ((unsigned long)dist2 >= 65536) {
			    r = 255;					
			} else {						
			    r= SQRT[dist2];	
			}
			mix_alpha(point, ramp[r], start_alpha);
			point+=BPP;
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
			*point++ = ramp[r].blue;
			*point++ = ramp[r].green;
			*point++ = ramp[r].red;
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
			mix_alpha(point, ramp[r], end_alpha);
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
		mix_alpha(point, *cp, cp->alpha);
		point+=BPP;
		X += dx;						
		Y += dy;						
            }		
        }
}

void
GraphicDevice24::drawLine(long x1, long y1, long x2, long y2, long width)
{
    int n,adr,dx,dy,sx;
    Color color;
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

    color = foregroundColor;
    alpha = foregroundColor.alpha;

    if (alpha == ALPHA_OPAQUE) {

#define PUTPIXEL() 				\
  {						\
      *pp++=color.red;		                \
      *pp++=color.green;	                \
      *pp++=color.blue;		                \
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
      mix_alpha(pp,color,alpha);	        \
  }

#define DRAWLINE(dx,dy,inc_1,inc_2) \
    n=dx;\
    a=2*dy-dx;\
    dy=2*dy;\
    dx=2*dx-dy;\
	 do {\
      PUTPIXEL();\
		if (a>0) { pp+=(inc_1*BPP); a-=dx; }\
		else { pp+=(inc_2*BPP); a+=dy; }\
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
