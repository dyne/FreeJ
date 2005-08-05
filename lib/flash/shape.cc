/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998,1999 Olivier Debon
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
#include <math.h>

#ifdef RCSID
static char *rcsid = "$Id: shape.cc,v 1.1.1.1 2004/06/04 21:17:07 tgc Exp $";
#endif

#define PRINT 0

#define ABS(v) ((v) < 0 ? -(v) : (v))

static void prepareStyles(GraphicDevice *gd, Matrix *matrix, Cxform *cxform, FillStyleDef *f, long n);

static void clearStyles(GraphicDevice *gd, FillStyleDef *f, long n);

static void drawShape(GraphicDevice *gd, Matrix *matrix1, Cxform *cxform, Shape *shape,
                      ShapeAction shapeAction, void *id,ScanLineFunc scan_line_func);

// Constructor

Shape::Shape(long id, int level) : Character(ShapeType, id)
{
    defLevel = level;

    defaultFillStyle.type = f_Solid;
    defaultFillStyle.color.red = 0;
    defaultFillStyle.color.green = 0;
    defaultFillStyle.color.blue = 0;
    defaultFillStyle.color.alpha = ALPHA_OPAQUE;

    defaultLineStyle.width = 0;

    // This is to force a first update
    lastMat.a = 0;
    lastMat.d = 0;
    shape_size += sizeof(Shape);
    shape_nb ++;

    file_ptr = NULL;
    getStyles = 0;
    getAlpha = 0;
}

Shape::~Shape()
{
	if (file_ptr) {
		free(file_ptr);
	}
}

void
Shape::setBoundingBox(Rect rect)
{
    boundary = rect;
}

void
Shape::getBoundingBox(Rect *bb, DisplayListEntry *e)
{
    *bb =  boundary;
}

int
Shape::execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform)
{
    //printf("TagId = %d\n", getTagId());
    //if (getTagId() != 220) return 0;

    if (cxform) {
        defaultFillStyle.color = cxform->getColor(gd->getForegroundColor());
    } else {
        defaultFillStyle.color = gd->getForegroundColor();
    }
    defaultFillStyle.color.pixel = gd->allocColor(defaultFillStyle.color);

    drawShape(gd, matrix, cxform, this, ShapeDraw, NULL, 0);
    return 0;
}

void
Shape::getRegion(GraphicDevice *gd, Matrix *matrix, void *id, ScanLineFunc scan_line_func)
{
    gd->setClipping(0);
    drawShape(gd,matrix,0,this,ShapeGetRegion,id,scan_line_func);
    gd->setClipping(1);
}

/************************************************************************/

/* create a new path */

static void newPath(ShapeParser *shape,
                    long x, long y)
{
    Path *p;
    long x1,y1;

    p=&shape->curPath;
    
    x1 = shape->matrix->getX(x, y);
    y1 = shape->matrix->getY(x, y);

    p->lastX = x1;
    p->lastY = y1;

    p->nb_edges = 0;
    p->nb_segments = 0;
}


static void addSegment1(ShapeParser *shape,
                        long x, long y,
                        FillStyleDef *f0,
                        FillStyleDef *f1,
                        LineStyleDef *l)
{
    Path *p;
    p=&shape->curPath;

    if (l) {
        /* a line is defined ... it will be drawn later */
        LineSegment *ls;

        ls = new LineSegment;
	if (ls != NULL) {
		ls->l = l;
		ls->x1 = p->lastX;
		ls->y1 = p->lastY;
		ls->x2 = x;
		ls->y2 = y;
		ls->first = (p->nb_segments == 0);
		ls->next = NULL;
		if (shape->last_line == NULL) {
		    shape->first_line = ls;
		} else {
		    shape->last_line->next = ls;
		}
		shape->last_line = ls;
	}
    }

    /* anti antialiasing not needed if line */
    if (!shape->reverse) {
        shape->gd->addSegment(p->lastX,p->lastY,x,y,f0,f1,l ? 0 : 1);
    } else {
        shape->gd->addSegment(p->lastX,p->lastY,x,y,f1,f0,l ? 0 : 1);
    }

    p->lastX = x;
    p->lastY = y;

    p->nb_segments++;
}


static void addLine(ShapeParser *shape, long x, long y,
                    FillStyleDef *f0,
                    FillStyleDef *f1,
                    LineStyleDef *l)
{
    long x1,y1;
    Path *p;

    p=&shape->curPath;

    x1 = shape->matrix->getX(x, y);
    y1 = shape->matrix->getY(x, y);
    
    addSegment1(shape,x1,y1,f0,f1,l);

    p->nb_edges++;
}


// This is based on Divide and Conquer algorithm.

#define BFRAC_BITS  0
#define BFRAC       (1 << BFRAC_BITS)

static void
bezierBuildPoints (ShapeParser *s,
                   int subdivisions,
                   long a1X, long a1Y,
                   long cX, long cY,
                   long a2X, long a2Y)
{
    long c1X,c1Y;
    long c2X,c2Y;
    long X,Y;
    long xmin,ymin,xmax,ymax;

    if (subdivisions != 0) {

        /* find the bounding box */

        if (a1X < cX) {
            xmin = a1X;
            xmax = cX;
        } else {
            xmin = cX;
            xmax = a1X;
        }
        if (a2X < xmin) xmin = a2X;
        if (a2X > xmax) xmax = a2X;
        
        if (a1Y < cY) {
            ymin = a1Y;
            ymax = cY;
        } else {
            ymin = cY;
            ymax = a1Y;
        }
        if (a2Y < ymin) ymin = a2Y;
        if (a2Y > ymax) ymax = a2Y;
    
        if (((xmax - xmin) + (ymax - ymin)) >= (BFRAC*FRAC*2)) {
            // Control point 1
            c1X = (a1X+cX) >> 1;
            c1Y = (a1Y+cY) >> 1;
            
            // Control point 2
            c2X = (a2X+cX) >> 1;
            c2Y = (a2Y+cY) >> 1;
            
            // New point
            X = (c1X+c2X) >> 1;
            Y = (c1Y+c2Y) >> 1;
            
            subdivisions--;

            bezierBuildPoints(s, subdivisions, 
                              a1X, a1Y, c1X, c1Y, X, Y);
            bezierBuildPoints(s, subdivisions, 
                              X, Y, c2X, c2Y, a2X, a2Y);
            
            return;
        }
    }
            
    addSegment1(s, (a2X+(BFRAC/2)) >> BFRAC_BITS, 
                (a2Y+(BFRAC/2)) >> BFRAC_BITS, s->f0, s->f1, s->l);
}

/* this code is broken, but useful to get something */
static void flushPaths(ShapeParser *s)
{
    LineSegment *ls;
    LineStyleDef *l;
    long nx,ny,nn,w;
    GraphicDevice *gd = s->gd;

    /* draw the filled polygon */
    gd->drawPolygon();
    
    /* draw the lines */
    ls = s->first_line;
    if (ls != NULL) {
        do {
            l = ls->l;

#if 0
            printf("line %d %d %d %d width=%d\n",
                   ls->x1, ls->y1, ls->x2, ls->y2, l->width);
#endif

            /* XXX: this width is false, but it is difficult (and expensive)
               to have the correct one */
            w = ABS((long)(s->matrix->a * l->width));

            if (w <= ((3*FRAC)/2)) {
	    	w = FRAC;
	    }
#ifdef THIN_LINES
            if (w <= ((3*FRAC)/2)) {
                // draw the thin lines only in shapeAction == shapeDraw
                if (gd->scan_line_func == NULL) {
                    gd->setForegroundColor(l->fillstyle.color);
                    gd->drawLine(ls->x1, ls->y1, ls->x2, ls->y2, w);
                }
            } else {
#else
	    {
#endif
                /* compute the normal vector */
                
                nx = -(ls->y2 - ls->y1);
                ny = (ls->x2 - ls->x1);
                
                /* normalize & width */
                nn = 2 * (long) sqrt(nx * nx + ny * ny);
                
#define UL ls->x1 + nx -ny, ls->y1 + ny +nx
#define UR ls->x2 + nx +ny, ls->y2 + ny -nx
#define LL ls->x1 - nx -ny, ls->y1 - ny +nx
#define LR ls->x2 - nx +ny, ls->y2 - ny -nx

                if (nn > 0) {
                    nx = (nx * w) / nn;
                    ny = (ny * w) / nn;
                    
                    /* top segment */
                    gd->addSegment(UL, UR, NULL, &l->fillstyle, 1);
                    
                    /* bottom segment */
                    gd->addSegment(LL, LR, &l->fillstyle, NULL, 1);
                
                    /* right segment */
                    gd->addSegment(UR, LR, &l->fillstyle, NULL, 1);
                
                    /* left segment */
                    gd->addSegment(UL, LL, NULL, &l->fillstyle, 1);
                
                    /* draw the line polygon */
                    gd->drawPolygon();
                }
            }
                
            ls = ls->next;
        } while (ls != NULL);
        
        /* delete the line structures */

        ls = s->first_line;
        while (ls != NULL) {
            LineSegment *ls1;
            ls1 = ls->next;
            delete ls;
            ls = ls1;
        }

        /* reset the line pointers */
        s->first_line = NULL;
        s->last_line = NULL;
    }
}


static void addBezier(ShapeParser *shape, 
                      long ctrlX1, long ctrlY1,
                      long newX1, long newY1,
                      FillStyleDef *f0,
                      FillStyleDef *f1,
                      LineStyleDef *l)
{
    long newX,newY,ctrlX,ctrlY;
    Path *p;

    p=&shape->curPath;

    /* note: we do the matrix multiplication before calculating the
       bezier points (faster !) */

    ctrlX = shape->matrix->getX(ctrlX1, ctrlY1);
    ctrlY = shape->matrix->getY(ctrlX1, ctrlY1);
    newX = shape->matrix->getX(newX1, newY1);
    newY = shape->matrix->getY(newX1, newY1);

    shape->f0 = f0;
    shape->f1 = f1;
    shape->l = l;

    bezierBuildPoints(shape, 3,
                      p->lastX<<BFRAC_BITS,p->lastY<<BFRAC_BITS,
                      ctrlX<<BFRAC_BITS,ctrlY<<BFRAC_BITS,
                      newX<<BFRAC_BITS,newY<<BFRAC_BITS);

    p->nb_edges++;
}

/***********************************************************************/


/* bit parser */

static void InitBitParser(struct BitParser *b,U8 *buf) 
{
    b->ptr = buf;
}

static void InitBits(struct BitParser *b)
{
    // Reset the bit position and buffer.
    b->m_bitPos = 0;
    b->m_bitBuf = 0;
}



static inline U8 GetByte(struct BitParser *b)
{
    U8 v;
    v = *b->ptr++;
    return v;
}

static inline U16 GetWord(struct BitParser *b)
{
    U8 *s;
    U16 v;
    s = b->ptr;
    v = s[0] | ((U16) s[1] << 8);
    b->ptr = s + 2;
    return v;
}

static inline U32 GetDWord(struct BitParser *b)
{
    U32 v;
    U8 * s = b->ptr;
    v = (U32) s[0] | ((U32) s[1] << 8) | 
        ((U32) s[2] << 16) | ((U32) s [3] << 24);
    b->ptr = s + 4;
    return v;
}

static inline U32 GetBit (struct BitParser *b)
{
    U32 v;
    S32 m_bitPos = b->m_bitPos;
    U32 m_bitBuf = b->m_bitBuf;
    
    if (m_bitPos == 0) {
        m_bitBuf = (U32)(*b->ptr++) << 24;
        m_bitPos = 8;
    }

    v = (m_bitBuf >> 31);

    m_bitPos--;
    m_bitBuf <<= 1;

    b->m_bitPos = m_bitPos;
    b->m_bitBuf = m_bitBuf;

    return v;
}

static inline U32 GetBits (struct BitParser *b, int n)
{
    U32 v;
    S32 m_bitPos = b->m_bitPos;
    U32 m_bitBuf = b->m_bitBuf;

    if (n == 0) 
        return 0;

    while (m_bitPos < n) {
        m_bitBuf |= (U32)(*b->ptr++) << (24 - m_bitPos);
        m_bitPos += 8;
    }

    v = m_bitBuf >> (32 - n);
    m_bitBuf <<= n;
    m_bitPos -= n;

    b->m_bitPos = m_bitPos;
    b->m_bitBuf = m_bitBuf;
    return v;
}

// Get n bits from the string with sign extension.
static inline S32 GetSBits (struct BitParser *b,S32 n)
{
    // Get the number as an unsigned value.
    S32 v = (S32) GetBits(b,n);

    // Is the number negative?
    if (v & (1L << (n - 1)))
    {
        // Yes. Extend the sign.
        v |= -1L << n;
    }

    return v;
}



/************************************************************************/

static void GetMatrix(BitParser *b, Matrix* mat)
{
    InitBits(b);

    // Scale terms
    if (GetBit(b))
    {
        int nBits = (int) GetBits(b,5);
        mat->a = (float)(GetSBits(b,nBits))/(float)0x10000;
        mat->d = (float)(GetSBits(b,nBits))/(float)0x10000;
    }
    else
    {
     	mat->a = mat->d = 1.0;
    }

    // Rotate/skew terms
    if (GetBit(b))
    {
        int nBits = (int)GetBits(b,5);
        mat->c = (float)(GetSBits(b,nBits))/(float)0x10000;
        mat->b = (float)(GetSBits(b,nBits))/(float)0x10000;
    }
    else
    {
     	mat->b = mat->c = 0.0;
    }

    // Translate terms
    int nBits = (int) GetBits(b,5);
    mat->tx = GetSBits(b,nBits);
    mat->ty = GetSBits(b,nBits);
}

static FillStyleDef * ParseFillStyle(ShapeParser *shape, long *n, long getAlpha)
{
    BitParser *b = &shape->bit_parser;
	FillStyleDef *defs;
	U16 i = 0;

	// Get the number of fills.
	U16 nFills = GetByte(b);

	// Do we have a larger number?
	if (nFills == 255)
	{
		// Get the larger number.
		nFills = GetWord(b);
	}

	*n = nFills;
	defs = new FillStyleDef[ nFills ];
	if (defs == NULL) return NULL;

	// Get each of the fill style.
	for (i = 0; i < nFills; i++)
	{
		U16 fillStyle = GetByte(b);

		defs[i].type = (FillType) fillStyle;

		if (fillStyle & 0x10)
		{
			defs[i].type = (FillType) (fillStyle & 0x12);

			// Get the gradient matrix.
			GetMatrix(b,&(defs[i].matrix));

			// Get the number of colors.
			defs[i].gradient.nbGradients = GetByte(b);

			// Get each of the colors.
			for (U16 j = 0; j < defs[i].gradient.nbGradients; j++)
			{
				defs[i].gradient.ratio[j] = GetByte(b);
				defs[i].gradient.color[j].red = GetByte(b);
				defs[i].gradient.color[j].green = GetByte(b);
				defs[i].gradient.color[j].blue = GetByte(b);
				if (getAlpha) {
                                    defs[i].gradient.color[j].alpha = GetByte(b);
				} else {
					defs[i].gradient.color[j].alpha = ALPHA_OPAQUE;
                                }
			}
		}
		else if (fillStyle & 0x40)
		{
			defs[i].type = (FillType) (fillStyle & 0x41);

			// Get the bitmapId
			defs[i].bitmap = (Bitmap *)shape->dict->getCharacter(GetWord(b));
			// Get the bitmap matrix.
			GetMatrix(b,&(defs[i].matrix));
		}
		else
		{
			defs[i].type = (FillType) 0;

			// A solid color
			defs[i].color.red = GetByte(b);
			defs[i].color.green = GetByte(b);
			defs[i].color.blue = GetByte(b);
			if (getAlpha) {
				defs[i].color.alpha = GetByte(b);
			} else {
				defs[i].color.alpha = ALPHA_OPAQUE;
                        }
		}
	}
	
	return defs;
}

static LineStyleDef * ParseLineStyle(ShapeParser *shape, long *n, long getAlpha)
{
    BitParser *b = &shape->bit_parser;
    LineStyleDef *defs,*def;
    FillStyleDef *f;
    long i;

	// Get the number of lines.
	U16 nLines = GetByte(b);

	// Do we have a larger number?
	if (nLines == 255)
	{
		// Get the larger number.
		nLines = GetWord(b);
	}

	*n = nLines;
	defs = new LineStyleDef[ nLines ];
	if (defs == NULL) return NULL;

	// Get each of the line styles.
	for (i = 0; i < nLines; i++)
	{
            def=&defs[i];
            def->width = GetWord(b);
            def->color.red = GetByte(b);
            def->color.green = GetByte(b);
            def->color.blue = GetByte(b);
            if (getAlpha) {
                def->color.alpha = GetByte(b);
            } else {
                def->color.alpha = ALPHA_OPAQUE;
            }
            
            f=&def->fillstyle;
            f->type = f_Solid;
            f->color = def->color;
            if (shape->cxform) {
                f->color = shape->cxform->getColor(f->color);
            }
            f->color.pixel = shape->gd->allocColor(f->color);
	}

	return defs;
}

/* 0 = end of shape */
static int ParseShapeRecord(ShapeParser *shape, ShapeRecord *sr, long getAlpha)
{
    BitParser *b = &shape->bit_parser;

	// Determine if this is an edge.
	BOOL isEdge = (BOOL) GetBit(b);

	if (!isEdge)
	{
		// Handle a state change
		U16 flags = (U16) GetBits(b,5);

		// Are we at the end?
		if (flags == 0)
		{
			// End of shape
			return 0;
		}

		sr->type = shapeNonEdge;
		sr->flags = (ShapeFlags)flags;

		// Process a move to.
		if (flags & flagsMoveTo)
		{
			U16 nBits = (U16) GetBits(b,5);
			sr->x = GetSBits(b,nBits);
			sr->y = GetSBits(b,nBits);
		}

		// Get new fill info.
		if (flags & flagsFill0)
		{
			sr->fillStyle0 = GetBits(b,shape->m_nFillBits);
		}
		if (flags & flagsFill1)
		{
			sr->fillStyle1 = GetBits(b,shape->m_nFillBits);
		}

		// Get new line info
		if (flags & flagsLine)
		{
			sr->lineStyle = GetBits(b,shape->m_nLineBits);
		}

		// Check to get a new set of styles for a new shape layer.
		if (flags & flagsNewStyles)
		{
			FillStyleDef *fillDefs;
			LineStyleDef *lineDefs;
			long n;

			// Parse the style.
			fillDefs = ParseFillStyle(shape, &n, getAlpha);
			if (fillDefs == NULL)  return 0;

			sr->newFillStyles = fillDefs;
			sr->nbNewFillStyles = n;

			lineDefs = ParseLineStyle(shape, &n, getAlpha);
			if (lineDefs == NULL) return 0;

			sr->newLineStyles = lineDefs;
			sr->nbNewLineStyles = n;

			InitBits(b);	// Bug !

			// Reset.
			shape->m_nFillBits = (U16) GetBits(b,4);
			shape->m_nLineBits = (U16) GetBits(b,4);
		}

		//if (flags & flagsEndShape)
			//printf("\tEnd of shape.\n\n");
  
		return flags & flagsEndShape ? 0 : 1;
	}
	else
	{
		if (GetBit(b))
		{
			sr->type = shapeLine;

			// Handle a line
			U16 nBits = (U16) GetBits(b,4) + 2;	// nBits is biased by 2

			// Save the deltas
			if (GetBit(b))
			{
				// Handle a general line.
				sr->dX = GetSBits(b,nBits);
				sr->dY = GetSBits(b,nBits);
			}
			else
			{
				// Handle a vert or horiz line.
				if (GetBit(b))
				{
					// Vertical line
					sr->dY = GetSBits(b,nBits);
					sr->dX = 0;
				}
				else
				{
					// Horizontal line
					sr->dX = GetSBits(b,nBits);
					sr->dY = 0;
				}
			}
		}
		else
		{
			sr->type = shapeCurve;

		 	// Handle a curve
			U16 nBits = (U16) GetBits(b,4) + 2;	// nBits is biased by 2

			// Get the control
			sr->ctrlX = GetSBits(b,nBits);
			sr->ctrlY = GetSBits(b,nBits);

			// Get the anchor
			sr->anchorX = GetSBits(b,nBits);
			sr->anchorY = GetSBits(b,nBits);
		}

		return 1;
	}
}

static void drawShape(GraphicDevice *gd, Matrix *matrix1, Cxform *cxform, Shape *shape,
                      ShapeAction shapeAction, void *id,ScanLineFunc scan_line_func)
{
    LineStyleDef *l;
    FillStyleDef *f0;
    FillStyleDef *f1;
    ShapeRecord sr1,*sr = &sr1;
    int firstPoint;
    long lastX,lastY;
    LineStyleDef *curLineStyle;
    long curNbLineStyles;
    FillStyleDef *curFillStyle;
    long curNbFillStyles;
    StyleList *sl;
    ShapeParser sp1,*sp=&sp1;
    BitParser *b;
    Matrix     mat,*matrix;

    mat = (*gd->adjust) * (*matrix1);
    matrix = &mat;
    
    sp->reverse = (mat.a * mat.d) < 0;

    curLineStyle = NULL;
    curNbLineStyles = 0;
    curFillStyle = NULL;
    curNbFillStyles = 0;
    sp->style_list = NULL;

    sp->shape = shape;
    sp->gd = gd;
    sp->matrix = matrix;
    sp->cxform = cxform;
    sp->dict = shape->dict;

    if (shapeAction == ShapeGetRegion) {
        gd->scan_line_func = scan_line_func;
        gd->scan_line_func_id = id;
    } else {
        gd->scan_line_func = NULL;
    }

    b = &sp->bit_parser;
    InitBitParser(b,shape->file_ptr);

    if (shape->getStyles) {
        // ShapeWithStyle
        curFillStyle = ParseFillStyle(sp, &curNbFillStyles, shape->getAlpha);
	if (curFillStyle == NULL) return;

        curLineStyle = ParseLineStyle(sp, &curNbLineStyles, shape->getAlpha);
	if (curLineStyle == NULL) return;

        sl = new StyleList;
	if (sl == NULL) return;

        sl->next = NULL;
        sl->newFillStyles = curFillStyle;
        sl->nbNewFillStyles = curNbFillStyles;
        sl->newLineStyles = curLineStyle;
        sl->nbNewLineStyles = curNbLineStyles;

        sp->style_list = sl;

        if (shapeAction == ShapeDraw) {
            prepareStyles(gd, matrix, cxform, curFillStyle, curNbFillStyles);
        }
    }
        
    InitBits(b);
    sp->m_nFillBits = (U16) GetBits(b,4);
    sp->m_nLineBits = (U16) GetBits(b,4);

    l = 0;
    f0 = 0;
    f1 = 0;
    firstPoint = 1;
    lastX = 0;
    lastY = 0;
    sp->curPath.nb_edges = 0;
    sp->first_line = NULL;
    sp->last_line = NULL;

    for(;;) {
        if (ParseShapeRecord(sp, sr, shape->getAlpha) == 0) break;

        switch (sr->type)
        {
            case shapeNonEdge:
                if (sr->flags & flagsNewStyles) {

                    curFillStyle = sr->newFillStyles;
                    curNbFillStyles = sr->nbNewFillStyles;
                    curLineStyle = sr->newLineStyles;
                    curNbLineStyles = sr->nbNewLineStyles;
                    
                    sl = new StyleList;
                    sl->next = sp->style_list;
                    sl->newFillStyles = sr->newFillStyles;
                    sl->nbNewFillStyles = sr->nbNewFillStyles;
                    sl->newLineStyles = sr->newLineStyles;
                    sl->nbNewLineStyles = sr->nbNewLineStyles;

                    sp->style_list = sl;

                    if (shapeAction == ShapeDraw) {
                        prepareStyles(gd, matrix, cxform, curFillStyle, curNbFillStyles);
                    }
                }
                if (sr->flags & flagsFill0) {
                    if (sr->fillStyle0) {
                        if (curFillStyle) {
                            f0 = &curFillStyle[sr->fillStyle0-1];
                        } else {
                            f0 = &shape->defaultFillStyle;
                        }
                    } else {
                        f0 = 0;
                    }
                }
                if (sr->flags & flagsFill1) {
                    if (sr->fillStyle1) {
                        if (curFillStyle) {
                            f1 = &curFillStyle[sr->fillStyle1-1];
                        } else {
                            f1 = &shape->defaultFillStyle;
                        }
                    } else {
                        f1 = 0;
                    }
                }
                if (sr->flags & flagsLine) {
                    if (sr->lineStyle) {
                        l = &curLineStyle[sr->lineStyle-1];
                    } else {
                        l = 0;
                    }
                }
                if (sr->flags & flagsMoveTo) {
                    if (sp->curPath.nb_edges == 0) {
                        /* if no edges, draw the polygon, then the lines */
                        flushPaths(sp);
                    }

                    newPath(sp, sr->x, sr->y);
                    firstPoint = 0;

                    lastX = sr->x;
                    lastY = sr->y;

#if PRINT
                    printf("---------\nX,Y    = %4d,%4d\n", sr->x/20, sr->y/20);
#endif
                }
                break;
            case shapeCurve:
                // Handle Bezier Curves !!!
                if (firstPoint) {
                    newPath(sp, 0, 0);
                    firstPoint = 0;
                }
                {
                    long newX,newY,ctrlX,ctrlY;
                    
                    ctrlX = lastX+sr->ctrlX;
                    ctrlY = lastY+sr->ctrlY;
                    newX = ctrlX+sr->anchorX;
                    newY = ctrlY+sr->anchorY;

#if 1
                    addBezier(sp, ctrlX, ctrlY, newX, newY, f0 , f1, l);
#else
                    addLine(sp, newX, newY, f0, f1, l);
#endif
                    
                    lastX = newX;
                    lastY = newY;
                }
                break;
            case shapeLine:
                if (firstPoint) {
                    newPath(sp, 0, 0);
                    firstPoint = 0;
                }

                lastX += sr->dX;
                lastY += sr->dY;

                addLine(sp, lastX, lastY, f0, f1, l);
#if PRINT
                printf(" X, Y  = %4d,%4d\n", lastX/20, lastY/20);
#endif
                break;
        }
    }

    /* XXX: should test if there is something to draw */
    flushPaths(sp);

    /* free the styles */
    while (sp->style_list) {
        StyleList *sl;
        
        sl=sp->style_list;
        sp->style_list = sl->next;
        
        if (shapeAction == ShapeDraw) {
            clearStyles(gd, sl->newFillStyles, sl->nbNewFillStyles);
        }

        delete[] sl->newFillStyles;
        delete[] sl->newLineStyles;
        
        delete sl;
    }
}

static void
prepareStyles(GraphicDevice *gd, Matrix *matrix, Cxform *cxform, 
              FillStyleDef *ftab, long n)
{
    long fs;
    FillStyleDef *f;

    for(fs = 0; fs < n; fs++)
    {
        f = ftab + fs;
        switch (f->type)
        {
            case f_None:
	    	break;
            case f_Solid:
                if (cxform) {
                    f->color = cxform->getColor(f->color);
                }
                f->color.pixel = gd->allocColor(f->color);
                break;
            case f_LinearGradient:
            case f_RadialGradient:
                {
                    Matrix mat;
                    int  n,r,l;
                    long red, green, blue, alpha;
                    long dRed, dGreen, dBlue, dAlpha;
                    long min,max;
                    Matrix *m;

                    mat = *(matrix) * f->matrix;
                    // Compute inverted matrix
                    f->gradient.imat = mat.invert();

                    /* renormalize the matrix */
                    m=&f->gradient.imat;
                    if (f->type == f_LinearGradient) {
                        m->a = m->a * FRAC * (1/128.0) * 65536.0;
                        m->b = m->b * FRAC * (1/128.0) * 65536.0;
                        m->tx = (long) ((m->tx + 16384) * (1/128.0) * 65536.0);
                    } else {
                        m->a = m->a * FRAC * (1/64.0) * 65536.0;
                        m->b = m->b * FRAC * (1/64.0) * 65536.0;
                        m->c = m->c * FRAC * (1/64.0) * 65536.0;
                        m->d = m->d * FRAC * (1/64.0) * 65536.0;
                        m->tx = (long) (m->tx * (1/64.0) * 65536.0);
                        m->ty = (long) (m->ty * (1/64.0) * 65536.0);
                    }

                    // Reset translation in inverted matrix
                    f->gradient.has_alpha = 0;

                    // Build a 256 color ramp
                    f->gradient.ramp = new Color[256];
		    if (f->gradient.ramp == NULL) {
		    	// Invalidate fill style
			f->type = f_None;
			continue;
		    }

                    // Store min and max
                    min = f->gradient.ratio[0];
                    max = f->gradient.ratio[f->gradient.nbGradients-1];
                    for(r=0; r < f->gradient.nbGradients-1; r++)
                    {
                        Color start,end;

                        l = f->gradient.ratio[r+1]-f->gradient.ratio[r];
                        if (l == 0) continue;

                        if (cxform) {
                            start = cxform->getColor(f->gradient.color[r]);
                            end   = cxform->getColor(f->gradient.color[r+1]);
                        } else {
                            start = f->gradient.color[r];
                            end   = f->gradient.color[r+1];
                        }
                        
                        if (start.alpha != ALPHA_OPAQUE || 
                            end.alpha != ALPHA_OPAQUE) {
                            f->gradient.has_alpha = 1;
                        }

                        dRed   = end.red - start.red;
                        dGreen = end.green - start.green;
                        dBlue  = end.blue - start.blue;
                        dAlpha = end.alpha - start.alpha;
                        
                        dRed   = (dRed<<16)/l;
                        dGreen = (dGreen<<16)/l;
                        dBlue  = (dBlue<<16)/l;
                        dAlpha  = (dAlpha<<16)/l;

                        red   = start.red <<16;
                        green = start.green <<16;
                        blue  = start.blue <<16;
                        alpha  = start.alpha <<16;

                        for (n=f->gradient.ratio[r]; n<=f->gradient.ratio[r+1]; n++) {
                            f->gradient.ramp[n].red = red>>16;
                            f->gradient.ramp[n].green = green>>16;
                            f->gradient.ramp[n].blue = blue>>16;
                            f->gradient.ramp[n].alpha = alpha>>16;

                            f->gradient.ramp[n].pixel = gd->allocColor(f->gradient.ramp[n]);
                            red += dRed;
                            green += dGreen;
                            blue += dBlue;
                            alpha += dAlpha;
                        }
                    }
                    for(n=0; n<min; n++) {
                        f->gradient.ramp[n] = f->gradient.ramp[min];
                    }
                    for(n=max; n<256; n++) {
                        f->gradient.ramp[n] = f->gradient.ramp[max];
                    }
                }
                break;
            case f_TiledBitmap:
            case f_clippedBitmap:
                if (f->bitmap) {
                    Matrix *m;

                    f->cmap = gd->getColormap(f->bitmap->colormap, 
                                              f->bitmap->nbColors, cxform);
		    if (f->cmap == NULL) {
			/* Get the normal cmap anyway */
		    	f->cmap = f->bitmap->colormap;
		    }

                    f->bitmap_matrix = *(matrix) * f->matrix;

                    f->bitmap_matrix = f->bitmap_matrix.invert();

                    m=&f->bitmap_matrix;
                    m->a = m->a * FRAC * 65536.0;
                    m->b = m->b * FRAC * 65536.0;
                    m->c = m->c * FRAC * 65536.0;
                    m->d = m->d * FRAC * 65536.0;
                    m->tx = (long) (m->tx * 65536.0);
                    m->ty = (long) (m->ty * 65536.0);

                    f->alpha_table = NULL;

                    if (f->bitmap->alpha_buf && cxform) {
                        unsigned char *alpha_table;
                        int i;

                        alpha_table = (unsigned char *)malloc (256);
                        if (alpha_table != NULL) {
                            for(i=0;i<256;i++) {
                                alpha_table[i] = cxform->getAlpha(i);
                            }
                        }
                        f->alpha_table = alpha_table;
                    }
                }
                break;
        }
    }
}

static void
clearStyles(GraphicDevice *gd, FillStyleDef *ftab, long n)
{
    long fs;
    FillStyleDef *f;

    for(fs = 0; fs < n; fs++)
    {
        f = ftab + fs;
        switch (f->type)
        {
            case f_Solid:
                break;
            case f_LinearGradient:
            case f_RadialGradient:
                if (f->gradient.ramp) {
                    delete f->gradient.ramp;
                }
                break;
            case f_TiledBitmap:
            case f_clippedBitmap:
                if (f->bitmap) {
                    if (f->cmap && f->cmap != f->bitmap->colormap) delete f->cmap;
                    if (f->alpha_table) free(f->alpha_table);
                }
                break;
	    case f_None:
	    	break;
        }
    }
}

