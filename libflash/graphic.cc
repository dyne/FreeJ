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

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

#define PRINT 0

// Public

GraphicDevice::GraphicDevice(FlashDisplay *fd)
{
	flashDisplay = fd;

	bgInitialized = 0;

	// Reset flash refresh flag
	flashDisplay->flash_refresh = 0;

        /* 16 bits, RGB565 */
	redMask = 0xF800;
	greenMask = 0x07E0;
	blueMask = 0x001F;

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
			newCmp[i] = old[i];
			newCmp[i].pixel = allocColor(old[i]);
		}
	}

	return newCmp;
}

long
GraphicDevice::getHeight()
{
	return targetHeight;
}

long
GraphicDevice::getWidth()
{
	return targetWidth;
}

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

Color
GraphicDevice::getBackgroundColor()
{
	return backgroundColor;
}

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

void
GraphicDevice::setMovieZoom(int z)
{
	z *= FRAC;
	if (z <= 0 || z > 100) return;
	zoom = z;
	setMovieDimension(movieWidth,movieHeight);
}

void
GraphicDevice::setMovieOffset(long x, long y)
{
	adjust->tx = -zoom*x;
	adjust->ty = -zoom*y;
}

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

// Virtual
void
GraphicDevice::clearCanvas()
{
}

long
GraphicDevice::allocColor(Color color)
{
	return 0;
}

void
GraphicDevice::fillLineBitmap(FillStyleDef *f, long y, long start, long end)
{
}

void
GraphicDevice::fillLineLG(Gradient *grad, long y, long start, long end)
{
}

void
GraphicDevice::fillLineRG(Gradient *grad, long y, long start, long end)
{
}

void
GraphicDevice::fillLine(FillStyleDef *f, long y, long start, long end)
{
}

void
GraphicDevice::fillLineAA(FillStyleDef *f, long y, long start, long end)
{
}

void
GraphicDevice::drawLine(long x1, long y1, long x2, long y2, long width)
{
}
