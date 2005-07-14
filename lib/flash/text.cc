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

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

Text::Text(long id) : Character(TextType, id)
{
	textRecords = 0;
}

Text::~Text()
{
	TextRecord     *cur,*del;

	for(cur = textRecords; cur;)
	{
		del = cur;
		cur = cur->next;
		delete del;
	}
}

void
Text::setTextBoundary(Rect rect)
{
	boundary = rect;
}

void
Text::setTextMatrix(Matrix m)
{
	textMatrix = m;
}

void
Text::addTextRecord(TextRecord *tr)
{
	SwfFont *font = 0;
	long n;

	tr->next = 0;

	if (textRecords == 0) {
		textRecords = tr;
		font = tr->font;
	} else {
		TextRecord *current;
		long fontHeight = 0;

		for(current = textRecords; current->next; current = current->next) {
			if (current->flags & textHasFont) {
				font = current->font;
				fontHeight = current->fontHeight;
			}
		}

		current->next = tr;
		if (current->flags & textHasFont) {
			font = current->font;
			fontHeight = current->fontHeight;
		}

		if (tr->flags & textHasFont) {
			font = tr->font;
		} else {
			tr->font = font;
			tr->fontHeight = fontHeight;
		}
	}

	if (tr->nbGlyphs) {
		for(n=0; n < tr->nbGlyphs; n++) {
			tr->glyphs[n].code = font->getGlyphCode(tr->glyphs[n].index);
		}
	}
}

int
Text::execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform)
{
	return doText(gd, matrix, cxform, ShapeDraw, NULL, NULL);
}

void
Text::getRegion(GraphicDevice *gd, Matrix *matrix, 
                               void *id, ScanLineFunc scan_line_func)
{
	doText(gd, matrix, 0, ShapeGetRegion, id, scan_line_func);
}

void
Text::getBoundingBox(Rect *bb, DisplayListEntry *e)
{
    *bb =  boundary;
}

TextRecord *
Text::getTextRecords()
{
	return textRecords;
}

int
Text::doText(GraphicDevice *gd, Matrix *matrix, Cxform *cxform, ShapeAction action,
             void *id, ScanLineFunc scan_line_func)
{
	TextRecord	*tr;
	long		 x,y;		// Current position
	SwfFont		*font = 0;	// Current font
	long		 fontHeight;
	Matrix		 tmat,fmat;
	long		 g;

	x = y = 0;
	fontHeight = 0;

	// Compute final text matrix
	tmat = (*matrix) * textMatrix;

	for(tr = textRecords; tr; tr = tr ->next)
	{
		if (tr->flags & isTextControl) {
			if (tr->flags & textHasXOffset) {
				x = tr->xOffset;
			}
			if (tr->flags & textHasYOffset) {
				y = tr->yOffset;
			}
			if (tr->flags & textHasColor) {
				if (action == ShapeDraw) {
					if (cxform) {
						gd->setForegroundColor(cxform->getColor(tr->color));
					} else {
						gd->setForegroundColor(tr->color);
					}
				}
			}
		}

		font = tr->font;
		fontHeight = tr->fontHeight;
		// Update font matrix
		fmat.a = fontHeight/1000.0;
		fmat.d = fontHeight/1000.0;

		assert(font != 0);
		for (g = 0; g < tr->nbGlyphs; g++)
		{
			Shape *shape;
			Matrix cmat;

			shape = font->getGlyph( tr->glyphs[g].index );

#ifdef PRINT
			printf("%c", font->getGlyphCode(tr->glyphs[g].index));
#endif

			// Update font matrix
			fmat.tx = x;
			fmat.ty = y;

			// Compute Character matrix
			cmat = tmat * fmat;

			if (action == ShapeDraw) {
				shape->execute(gd, &cmat, cxform);
			} else {
				shape->getRegion(gd, &cmat, id, scan_line_func);
			}

			// Advance
			x += tr->glyphs[g].xAdvance;
		}
#ifdef PRINT
		printf("\n");
#endif
	}

	if (gd->showMore) {
		tmat = (*gd->adjust) * (*matrix);

		long x1,x2,y1,y2;

		x1 = boundary.xmin;
		y1 = boundary.ymin;
		x2 = boundary.xmax;
		y2 = boundary.ymax;
		gd->drawLine(tmat.getX(x1,y1),tmat.getY(x1,y1),tmat.getX(x2,y1),tmat.getY(x2,y1),FRAC);
		gd->drawLine(tmat.getX(x2,y1),tmat.getY(x2,y1),tmat.getX(x2,y2),tmat.getY(x2,y2),FRAC);
		gd->drawLine(tmat.getX(x2,y2),tmat.getY(x2,y2),tmat.getX(x1,y2),tmat.getY(x1,y2),FRAC);
		gd->drawLine(tmat.getX(x1,y2),tmat.getY(x1,y2),tmat.getX(x1,y1),tmat.getY(x1,y1),FRAC);
	}

	return 0;
}

////////// TextRecord Methods
TextRecord::TextRecord() {
	flags = (TextFlags)0;
	font = 0;
	fontHeight = 0;
	nbGlyphs = 0;
	glyphs = 0;
	xOffset = 0;
	yOffset = 0;
}

TextRecord::~TextRecord() {
	if (nbGlyphs) delete glyphs;
}

char *
TextRecord::getText() {
	static char text[256];
	long g;

	for(g=0; g < nbGlyphs; g++) {
		text[g] = glyphs[g].code;
	}
	text[g] = 0;

	return text;
}
