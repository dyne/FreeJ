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
#ifndef _SHAPE_H_
#define _SHAPE_H_

struct LineStyleDef {
    long		 width;
    Color		 color;
    FillStyleDef         fillstyle;
};

enum ShapeRecordType {
	shapeNonEdge,
	shapeCurve,
	shapeLine
};

enum ShapeFlags {
	flagsMoveTo	   = 0x01,
	flagsFill0	   = 0x02,
	flagsFill1	   = 0x04,
	flagsLine	   = 0x08,
	flagsNewStyles	   = 0x10,
	flagsEndShape 	   = 0x80
};

struct ShapeRecord {
	ShapeRecordType  type;

	// Non Edge
	ShapeFlags	 flags;
	long		 x,y;	// Moveto
	long		 fillStyle0;
	long		 fillStyle1;
	long		 lineStyle;
	FillStyleDef	*newFillStyles; // Array
	long		 nbNewFillStyles;
	LineStyleDef	*newLineStyles; // Array
	long		 nbNewLineStyles;

	// Curve Edge
	long		 ctrlX, ctrlY;
	long		 anchorX, anchorY;

	// Straight Line
	long		 dX,dY;

	struct ShapeRecord *next;

    ShapeRecord() {
        shaperecord_size += sizeof(ShapeRecord);
        shaperecord_nb++;
    }

};

enum ShapeAction {
	ShapeDraw,
	ShapeGetRegion
};

struct LineSegment {
    long x1,y1,x2,y2;
    char first;
    LineStyleDef *l;
    struct LineSegment *next;
};

struct Path {
    long lastX,lastY;
    int nb_edges;
    int nb_segments;
};

struct StyleList {
    FillStyleDef	*newFillStyles; // Array
    long		 nbNewFillStyles;
    LineStyleDef	*newLineStyles; // Array
    long		 nbNewLineStyles;
    
    StyleList *next;
};


/* fast bit parser */
struct BitParser {
    // Bit Handling
    S32 m_bitPos;
    U32 m_bitBuf;

    U8 *ptr;
};

class Shape;

/* state of the shape parser */
struct ShapeParser {
    Dict *dict;         /* XXX: should be put elsewhere */

    BitParser bit_parser;
    S32 m_nFillBits;
    S32 m_nLineBits;

    StyleList *style_list;
    Matrix *matrix;
    Path curPath;
    int reverse;

    /* line rasteriser */
    LineSegment *first_line,*last_line;
    GraphicDevice *gd;
    Cxform *cxform;
    Shape *shape;

    FillStyleDef *f0;
    FillStyleDef *f1;
    LineStyleDef *l;
};

class Shape : public Character {
 public:
	int		 defLevel; // 1,2 or 3
        

	Rect		 boundary;
	FillStyleDef	 defaultFillStyle;
	LineStyleDef	 defaultLineStyle;

	Matrix		 lastMat;
        /* parsing for the rendering stage (saves a lot of memory &
           may not reduce significantly the size). These variables
           should be in another structure (no state need to be
           maintained between two renderings) */
        int getAlpha, getStyles;
        unsigned char *file_ptr;
        Dict *dict;         /* XXX: should be put elsewhere */

protected:
	void	 drawLines(GraphicDevice *gd, Matrix *matrix, Cxform *cxform, long, long);
	void	 buildSegmentList(Segment **segs, int height, long &n, Matrix *matrix, int update, int reverse);
	Segment *progressSegments(Segment *, long);
	Segment *newSegments(Segment *, Segment *);

public:
	Shape(long id = 0 , int level = 1);
	~Shape();

	void	 setBoundingBox(Rect rect);
	int	 execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform);
	void	 getRegion(GraphicDevice *gd, Matrix *matrix, 
                           void *id, ScanLineFunc scan_line_func);

	void	 getBoundingBox(Rect *bb, DisplayListEntry *);

#ifdef DUMP
	void	 dump(BitStream *bs);
	void	 dumpShapeRecords(BitStream *bs, int alpha);
	void	 dumpFillStyles(BitStream *bs, FillStyleDef *defs, long n, int alpha);
	void	 dumpLineStyles(BitStream *bs, LineStyleDef *defs, long n, int alpha);
	void	 checkBitmaps(BitStream *bs);
#endif
};

#endif /* _SHAPE_H_ */
