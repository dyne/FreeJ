#include "swf.h"

////////////////////////////////////////////////////////////
//  This file is derived from the 'buggy' SWF parser provided
//  by Macromedia.
//
//  Modifications : Olivier Debon  <odebon@club-internet.fr>
//  

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

#define printf(fmt,args...)

//////////////////////////////////////////////////////////////////////
// Inline input script object methods.
//////////////////////////////////////////////////////////////////////

//
// Inlines to parse a Flash file.
//
inline U8 CInputScript::GetByte(void) 
{
    return m_fileBuf[m_filePos++];
}

inline U16 CInputScript::GetWord(void)
{
    U8 * s = m_fileBuf + m_filePos;
    m_filePos += 2;
    return (U16) s[0] | ((U16) s[1] << 8);
}

inline U32 CInputScript::GetDWord(void)
{
    U8 * s = m_fileBuf + m_filePos;
    m_filePos += 4;
    return (U32) s[0] | ((U32) s[1] << 8) | ((U32) s[2] << 16) | ((U32) s [3] << 24);
}




//////////////////////////////////////////////////////////////////////
// Input script object methods.
//////////////////////////////////////////////////////////////////////

CInputScript::CInputScript(int level)
// Class constructor.
{
    this->level = level;

    // Initialize the input pointer.
    m_fileBuf = NULL;

    // Initialize the file information.
    m_filePos = 0;
    m_fileSize = 0;
    m_fileVersion = 0;

    // Initialize the bit position and buffer.
    m_bitPos = 0;
    m_bitBuf = 0;

    // Initialize the output file.
    m_outputFile = NULL;

    // Set to true if we wish to dump all contents long form
    m_dumpAll = false;

    // if set to true will dump image guts (i.e. jpeg, zlib, etc. data)
    m_dumpGuts = false;

    needHeader = 1;
    program = 0;

    outOfMemory = 0;

    next = NULL;

    return;
}


CInputScript::~CInputScript(void)
// Class destructor.
{
    // Free the buffer if it is there.
    if (m_fileBuf)
    {
	delete program;
        m_fileBuf = NULL;
        m_fileSize = 0;
    }
}


U16 CInputScript::GetTag(void)
{
    // Save the start of the tag.
    m_tagStart = m_filePos;
    
    if (m_actualSize-m_filePos < 2) return notEnoughData;

    // Get the combined code and length of the tag.
    U16 code = GetWord();

    // The length is encoded in the tag.
    U32 len = code & 0x3f;

    // Remove the length from the code.
    code = code >> 6;

    // Determine if another long word must be read to get the length.
    if (len == 0x3f) {
        if (m_actualSize-m_filePos < 4) return notEnoughData;
    	len = (U32) GetDWord();
    }

    // Determine the end position of the tag.
    m_tagEnd = m_filePos + (U32) len;
    m_tagLen = (U32) len;

    return code;
}


void CInputScript::GetRect (Rect * r)
{
    InitBits();
    int nBits = (int) GetBits(5);
    r->xmin = GetSBits(nBits);
    r->xmax = GetSBits(nBits);
    r->ymin = GetSBits(nBits);
    r->ymax = GetSBits(nBits);
}

void CInputScript::GetMatrix(Matrix* mat)
{
    InitBits();

    // Scale terms
    if (GetBits(1))
    {
        int nBits = (int) GetBits(5);
        mat->a = (float)(GetSBits(nBits))/(float)0x10000;
        mat->d = (float)(GetSBits(nBits))/(float)0x10000;
    }
    else
    {
     	mat->a = mat->d = 1.0;
    }

    // Rotate/skew terms
    if (GetBits(1))
    {
        int nBits = (int)GetBits(5);
        mat->c = (float)(GetSBits(nBits))/(float)0x10000;
        mat->b = (float)(GetSBits(nBits))/(float)0x10000;
    }
    else
    {
     	mat->b = mat->c = 0.0;
    }

    // Translate terms
    int nBits = (int) GetBits(5);
    mat->tx = GetSBits(nBits);
    mat->ty = GetSBits(nBits);
}


void CInputScript::GetCxform(Cxform* cx, BOOL hasAlpha)
{
    int flags;
    int nBits;
    float aa; long ab;
    float ra; long rb;
    float ga; long gb;
    float ba; long bb;

    InitBits();

    flags = (int) GetBits(2);
    nBits = (int) GetBits(4);
    aa = 1.0; ab = 0;
    if (flags & 1)
    {
        ra = (float) GetSBits(nBits)/256.0;
        ga = (float) GetSBits(nBits)/256.0;
        ba = (float) GetSBits(nBits)/256.0;
        if (hasAlpha) aa = (float) GetSBits(nBits)/256.0;
    }
    else
    {
        ra = ga = ba = 1.0;
    }
    if (flags & 2)
    {
        rb = (S32) GetSBits(nBits);
        gb = (S32) GetSBits(nBits);
        bb = (S32) GetSBits(nBits);
        if (hasAlpha) ab = (S32) GetSBits(nBits);
    }
    else
    {
        rb = gb = bb = 0;
    }
    if (cx) {
    	cx->aa = aa;
    	cx->ab = ab;
    	cx->ra = ra;
    	cx->rb = rb;
    	cx->ga = ga;
    	cx->gb = gb;
    	cx->ba = ba;
    	cx->bb = bb;
    }
}


/* XXX: should allocate string */
char *CInputScript::GetString(void)
{
    // Point to the string.
    char *str = (char *) &m_fileBuf[m_filePos];

    // Skip over the string.
    while (GetByte());

    return str;
}

void CInputScript::InitBits(void)
{
    // Reset the bit position and buffer.
    m_bitPos = 0;
    m_bitBuf = 0;
}


S32 CInputScript::GetSBits (S32 n)
// Get n bits from the string with sign extension.
{
    // Get the number as an unsigned value.
    S32 v = (S32) GetBits(n);

    // Is the number negative?
    if (v & (1L << (n - 1)))
    {
        // Yes. Extend the sign.
        v |= -1L << n;
    }

    return v;
}


U32 CInputScript::GetBits (S32 n)
// Get n bits from the stream.
{
    U32 v = 0;

    for (;;)
    {
        S32 s = n - m_bitPos;
        if (s > 0)
        {
            // Consume the entire buffer
            v |= m_bitBuf << s;
            n -= m_bitPos;

            // Get the next buffer
            m_bitBuf = GetByte();
            m_bitPos = 8;
        }
        else
        {
         	// Consume a portion of the buffer
            v |= m_bitBuf >> -s;
            m_bitPos -= n;
            m_bitBuf &= 0xff >> (8 - m_bitPos);	// mask off the consumed bits
            return v;
        }
    }
}

void CInputScript::ParseFreeCharacter()
{
    U32 tagid = (U32) GetWord();

    tagid = tagid;

    printf("tagFreeCharacter \ttagid %-5u\n", tagid);
}


void CInputScript::ParsePlaceObject()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlPlaceObject;
    ctrl->flags = (PlaceFlags)(placeHasMatrix | placeHasCharacter);

    ctrl->character = getCharacter(GetWord());
    ctrl->depth = GetWord();

    GetMatrix(&(ctrl->matrix));

    if ( m_filePos < m_tagEnd ) 
    {
	ctrl->flags = (PlaceFlags)(ctrl->flags | placeHasColorXform);

	GetCxform(&ctrl->cxform, false);
    }

    program->addControlInCurrentFrame(ctrl);
}


void CInputScript::ParsePlaceObject2()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlPlaceObject2;

    ctrl->flags = (PlaceFlags)GetByte();
    ctrl->depth = GetWord();

    // Get the tag if specified.
    if (ctrl->flags & placeHasCharacter)
    {
	ctrl->character = getCharacter(GetWord());
    }

    // Get the matrix if specified.
    if (ctrl->flags & placeHasMatrix)
    {
	GetMatrix(&(ctrl->matrix));
    }

    // Get the color transform if specified.
    if (ctrl->flags & placeHasColorXform) 
    {
	GetCxform(&ctrl->cxform, true);
    }        

    // Get the ratio if specified.
    if (ctrl->flags & placeHasRatio)
    {
	ctrl->ratio = GetWord();
    }        

    // Get the ratio if specified.
    if (ctrl->flags & placeHasName)
    {
	ctrl->name = strdup(GetString());
    }        

    // Get the clipdepth if specified.
    if (ctrl->flags & placeHasClip) 
    {
	ctrl->clipDepth = GetWord();
    }        

    program->addControlInCurrentFrame(ctrl);
}


void CInputScript::ParseRemoveObject()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlRemoveObject;
    ctrl->character = getCharacter(GetWord());
    ctrl->depth = GetWord();

    program->addControlInCurrentFrame(ctrl);
}


void CInputScript::ParseRemoveObject2()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlRemoveObject2;
    ctrl->depth = GetWord();

    program->addControlInCurrentFrame(ctrl);
}


void CInputScript::ParseSetBackgroundColor()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlBackgroundColor;
    ctrl->color.red = GetByte();
    ctrl->color.green = GetByte();
    ctrl->color.blue = GetByte();

    program->addControlInCurrentFrame(ctrl);
}


void CInputScript::ParseDoAction()
{
    Control *ctrl;
    ActionRecord *ar;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlDoAction;

    do {
	ar = ParseActionRecord();
	if (ar) {
	    ctrl->addActionRecord( ar );
	}
	if (outOfMemory) {
		return;
	}
    } while (ar);

    program->addControlInCurrentFrame(ctrl);

}


void CInputScript::ParseShapeData(int getAlpha, int getStyles)
{
    int shapeRecord = 0;

    if (getStyles) {
	// ShapeWithStyle
	ParseFillStyle(getAlpha);
	ParseLineStyle(getAlpha);
    }

    InitBits();
    m_nFillBits = (U16) GetBits(4);
    m_nLineBits = (U16) GetBits(4);

    do {
	shapeRecord = ParseShapeRecord(getAlpha);
    } while (shapeRecord);
}

int
CInputScript::ParseShapeRecord(long getAlpha)
{
    // Determine if this is an edge.
    BOOL isEdge = (BOOL) GetBits(1);

    if (!isEdge)
    {
	// Handle a state change
	U16 flags = (U16) GetBits(5);

	// Are we at the end?
	if (flags == 0)
	{
	    // End of shape
	    return 0;
	}

	// Process a move to.
	if (flags & flagsMoveTo)
	{
	    U16 nBits = (U16) GetBits(5);
	    GetSBits(nBits);
	    GetSBits(nBits);
	}

	// Get new fill info.
	if (flags & flagsFill0)
	{
	    GetBits(m_nFillBits);
	}
	if (flags & flagsFill1)
	{
	    GetBits(m_nFillBits);
	}

	// Get new line info
	if (flags & flagsLine)
	{
	    GetBits(m_nLineBits);
	}

	// Check to get a new set of styles for a new shape layer.
	if (flags & flagsNewStyles)
	{
	    // Parse the style.
	    ParseFillStyle(getAlpha);
	    ParseLineStyle(getAlpha);

	    InitBits();	// Bug !

	    // Reset.
	    m_nFillBits = (U16) GetBits(4);
	    m_nLineBits = (U16) GetBits(4);
	}

	return flags & flagsEndShape ? 0 : 1;
    }
    else
    {
	if (GetBits(1))
	{
	    // Handle a line
	    U16 nBits = (U16) GetBits(4) + 2;	// nBits is biased by 2

	    // Save the deltas
	    if (GetBits(1))
	    {
		// Handle a general line.
		GetSBits(nBits);
		GetSBits(nBits);
	    }
	    else
	    {
		// Handle a vert or horiz line.
		GetBits(1);
		GetSBits(nBits);
	    }
	}
	else
	{
	    // Handle a curve
	    U16 nBits = (U16) GetBits(4) + 2;	// nBits is biased by 2

	    // Get the control
	    GetSBits(nBits);
	    GetSBits(nBits);

	    // Get the anchor
	    GetSBits(nBits);
	    GetSBits(nBits);
	}

	return 1;
    }
}


void CInputScript::ParseFillStyle(long getAlpha)
	// 
{
    U16 i = 0;
    FillType type;
    Matrix matrix;

    // Get the number of fills.
    U16 nFills = GetByte();

    // Do we have a larger number?
    if (nFills == 255)
    {
	// Get the larger number.
	nFills = GetWord();
    }

    // Get each of the fill style.
    for (i = 0; i < nFills; i++)
    {
	U16 fillStyle = GetByte();

	type = (FillType) fillStyle;

	printf("fillstyle: type=%d\n",defs[i].type);
	if (fillStyle & 0x10)
	{
	    U16 nbGradients;

	    type = (FillType) (fillStyle & 0x12);

	    // Get the gradient matrix.
	    GetMatrix(&matrix);

	    // Get the number of colors.
	    nbGradients = GetByte();

	    // Get each of the colors.
	    for (U16 j = 0; j < nbGradients; j++)
	    {
		GetByte();
		GetByte();
		GetByte();
		GetByte();
		if (getAlpha) {
		    GetByte();
		}
	    }
	}
	else if (fillStyle & 0x40)
	{
	    type = (FillType) (fillStyle & 0x41);

	    // Get the bitmapId
	    GetWord();

	    // Get the bitmap matrix.
	    GetMatrix(&matrix);
	}
	else
	{
	    type = (FillType) 0;

	    // A solid color
	    GetByte();
	    GetByte();
	    GetByte();
	    if (getAlpha) {
		GetByte();
	    }

	    printf("fillstyle: %x %x %x %x\n",			
		   defs[i].color.red,
		   defs[i].color.green,
		   defs[i].color.blue,
		   defs[i].color.alpha);
	}
    }
}

void CInputScript::ParseLineStyle(long getAlpha)
{
    long i;

    // Get the number of lines.
    U16 nLines = GetByte();

    // Do we have a larger number?
    if (nLines == 255)
    {
	// Get the larger number.
	nLines = GetWord();
    }

    // Get each of the line styles.
    for (i = 0; i < nLines; i++)
    {
	GetWord();
	GetByte();
	GetByte();
	GetByte();
	if (getAlpha) {
	    GetByte();
	}
    }
}


void CInputScript::ParseDefineShape(int level)
{
    Shape *shape;
    Rect rect;
    U32 tagid;

    tagid = (U32) GetWord();
    shape = new Shape(tagid,level);
    if (shape == NULL) {
	outOfMemory = 1;
    	return;
    }
    shape->dict = this;

    // Get the frame information.
    GetRect(&rect);

    shape->setBoundingBox(rect);

    shape->file_ptr = (unsigned char*)malloc(m_tagEnd-m_filePos);
    if (shape->file_ptr == NULL) {
	outOfMemory = 1;
    	delete shape;
	return;
    }
    memcpy((void*)shape->file_ptr,(void*)&m_fileBuf[m_filePos], m_tagEnd-m_filePos);

    shape->getStyles = 1;
    shape->getAlpha = (level == 3);

    ParseShapeData(level == 3, 1);

    addCharacter(shape);
}

void CInputScript::S_DumpImageGuts()
{
#if 0
    U32 lfCount = 0;                
    printf("----- dumping image details -----");
    while (m_filePos < m_tagEnd)
    {
	if ((lfCount % 16) == 0)
	{
	    fprintf(stdout, "\n");
	}
	lfCount += 1;
	fprintf(stdout, "%02x ", GetByte());
    }
    fprintf(stdout, "\n");
#endif
}

void CInputScript::ParseDefineBits()
{
    Bitmap *bitmap;
    U32 tagid = (U32) GetWord();
    int status;

    bitmap = new Bitmap(tagid,1);
    if (bitmap == NULL) {
	outOfMemory = 1;
    	return;
    }

    status = bitmap->buildFromJpegAbbreviatedData(&m_fileBuf[m_filePos]);

    if (status < 0) {
	fprintf(stderr,"Unable to read JPEG data\n");
	delete bitmap;
	return;
    }

    addCharacter(bitmap);
}


void CInputScript::ParseDefineBitsJPEG2()
{
    Bitmap *bitmap;
    U32 tagid = (U32) GetWord();
    int status;

    bitmap = new Bitmap(tagid,2);
    if (bitmap == NULL) {
	outOfMemory = 1;
    	return;
    }

    status = bitmap->buildFromJpegInterchangeData(&m_fileBuf[m_filePos], 0, 0);

    if (status < 0) {
	fprintf(stderr,"Unable to read JPEG data\n");
	delete bitmap;
	return;
    }

    addCharacter(bitmap);
}

void CInputScript::ParseDefineBitsJPEG3()
{
    Bitmap *bitmap;
    U32 tagid = (U32) GetWord();
    int status;
    long offset;

    printf("tagDefineBitsJPEG3 \ttagid %-5u\n", tagid);

    bitmap = new Bitmap(tagid,3);
    if (bitmap == NULL) {
	outOfMemory = 1;
    	return;
    }

    offset = GetDWord();	// Not in the specs !!!!

    status = bitmap->buildFromJpegInterchangeData(&m_fileBuf[m_filePos], 1, offset);
    if (status < 0) {
	fprintf(stderr,"Unable to read JPEG data\n");
	delete bitmap;
	return;
    }

    addCharacter(bitmap);
}


void CInputScript::ParseDefineBitsLossless(int level)
{
    Bitmap *bitmap;
    U32 tagid = (U32) GetWord();
    int status;
    int tableSize;         

    bitmap = new Bitmap(tagid,0);
    if (bitmap == NULL) {
	outOfMemory = 1;
    	return;
    }

    int format = GetByte();
    int width  =  GetWord();
    int height = GetWord();

    tableSize = 0;

    if (format == 3) {
	tableSize = GetByte();
    }

    status = bitmap->buildFromZlibData(&m_fileBuf[m_filePos], width, height, format, tableSize, level == 2);

    if (status < 0) {
	fprintf(stderr,"Unable to read ZLIB data\n");
	delete bitmap;
	return;
    }

    addCharacter(bitmap);
}

void CInputScript::ParseJPEGTables()
{
    Bitmap::readJpegTables(&m_fileBuf[m_filePos]);
}


ButtonRecord * CInputScript::ParseButtonRecord(long getCxform)
{
    U16 state;
    ButtonRecord *br;
    long tagid;
    Matrix matrix;
    long layer;
    Cxform *cxform;

    state = (U16) GetByte();

    if (state == 0) return 0;

    br = new ButtonRecord;
    if (br == NULL) {
	outOfMemory = 1;
    	return 0;
    }

    tagid = GetWord();
    layer = GetWord();
    GetMatrix(&matrix);

    if (br) {
        br->state = (ButtonState) state;
        br->character = getCharacter(tagid);
        br->layer = layer;
        br->cxform = 0;
	br->buttonMatrix = matrix;
    }

    if (getCxform) {
	cxform = new Cxform;
	GetCxform(cxform, true);
	if (br) {
		br->cxform = cxform;
		if (cxform == NULL) {
			outOfMemory = 1;
		}
	}
    }

    return br;
}

ActionRecord * CInputScript::ParseActionRecord()
{
    U8 action;
    U16 length = 0;
    char *url, *target, *label;
    long frameIndex, skipCount;
    ActionRecord *ar;

    action = GetByte();
    if (action == 0) return 0;

    ar = new ActionRecord;
    if (ar == NULL) {
    	outOfMemory = 1;
	return 0;
    }

    ar->action = (Action)action;

    if (action & 0x80) {
	length = GetWord();
    }

    switch (action) {
    case ActionGotoFrame:
	frameIndex = GetWord();
	if (ar) {
		ar->frameIndex = frameIndex;
	}
	break;
    case ActionGetURL:
	url = GetString();
	target = GetString();
	if (ar) {
		ar->url = strdup(url);
		ar->target = strdup(target);
	}
	break;
    case ActionWaitForFrame:
	frameIndex = GetWord();
	skipCount = GetByte();
	if (ar) {
		ar->frameIndex = frameIndex;
		ar->skipCount = skipCount;
	}
	break;
    case ActionSetTarget:
	target = strdup(GetString());
	if (ar) {
		ar->target = target;
	}
	break;
    case ActionGoToLabel:
	label = GetString();
	if (ar) {
		ar->frameLabel = strdup(label);
	}
	break;
    default:
	while (length--) {
		GetByte();
	}
    	break;
    }

    return ar;
}

void CInputScript::ParseDefineButton()
{
    Button		*button;
    ButtonRecord	*buttonRecord;
    ActionRecord	*actionRecord;

    U32 tagid = (U32) GetWord();

    button = new Button(tagid);
    if (button == NULL) {
	outOfMemory = 1;
    	return;
    }

    do {
	buttonRecord = ParseButtonRecord();
	if (buttonRecord) {
	    button->addButtonRecord( buttonRecord );
	}
	if (outOfMemory) {
		return;
	}
    } while (buttonRecord);

    do {
	actionRecord = ParseActionRecord();
	if (actionRecord) {
	    button->addActionRecord( actionRecord );
	}
	if (outOfMemory) {
		return;
	}
    } while (actionRecord);

    addCharacter(button);
}


void CInputScript::ParseDefineButton2()
{
    Button		*button;
    ButtonRecord	*buttonRecord;
    ActionRecord	*actionRecord;
    U16		 transition;
    U16		 offset;
    U8		 menu;

    U32 tagid = (U32) GetWord();

    button = new Button(tagid);

    if (button == NULL) {
    	outOfMemory = 1;
	return;
    }

    menu = GetByte();

    offset = GetWord();

    do {
	buttonRecord = ParseButtonRecord(true);
	if (buttonRecord) {
	    button->addButtonRecord( buttonRecord );
	}
	if (outOfMemory) {
		return;
	}
    } while (buttonRecord);

    while (offset) {
	offset = GetWord();

	transition = GetWord();

	do {
	    actionRecord = ParseActionRecord();
	    if (actionRecord) {
		button->addActionRecord( actionRecord );
	    }
	    if (outOfMemory) {
		    return;
	    }
	} while (actionRecord);

	button->addCondition( transition );
    }

    addCharacter(button);
}


void CInputScript::ParseDefineFont()
{
    SwfFont	*font = 0;
    U32 tagid = (U32) GetWord();
    long	 start;
    long	 nb,n;
    long	 offset;
    long	*offsetTable = 0;
    Shape	*shapes = 0;

    font = new SwfFont(tagid);
    if (font == NULL) {
	outOfMemory = 1;
    	return;
    }
    start = m_filePos;

    offset = GetWord();
    nb = offset/2;
    offsetTable = new long[nb];
    if (offsetTable == NULL) {
	goto memory_error;
    }
    offsetTable[0] = offset;

    for(n=1; n<nb; n++)
    {
	offsetTable[n] = GetWord();
    }

    shapes = new Shape[nb];
    if (shapes == NULL) {
	goto memory_error;
    }

    for(n=0; n<nb; n++)
    {
	long here;

	m_filePos = offsetTable[n]+start;

	here = m_filePos;
	ParseShapeData(0, 0);

	// Keep data for later parsing
	shapes[n].file_ptr = (unsigned char*)malloc(m_filePos-here);
	if (shapes[n].file_ptr == NULL) {
		goto memory_error;
	}
	memcpy((void*)shapes[n].file_ptr,(void*)&m_fileBuf[here],m_filePos-here);
    }

    font->setFontShapeTable(shapes,nb);

    delete[] offsetTable;

    addCharacter(font);
    return;

memory_error:
    outOfMemory = 1;
    if (offsetTable) delete offsetTable;
    if (font) delete font;
    if (shapes) delete[] shapes;
}


void CInputScript::ParseDefineMorphShape()
{
    U32 tagid = (U32) GetWord();

    tagid = tagid;
    printf("tagDefineMorphShape \ttagid %-5u\n", tagid);
}

void CInputScript::ParseDefineFontInfo()
{
    SwfFont	*font;
    U32 tagid = (U32) GetWord();
    long	 nameLen;
    char	*name;
    long	 n,nb;
    FontFlags    flags;
    long	*lut;

    font = (SwfFont *)getCharacter(tagid);

    if (font == NULL) {
    	outOfMemory = 1;
	return;
    }

    nameLen = GetByte();
    name = new char[nameLen+1];
    if (name == NULL) {
    	outOfMemory = 1;
	return;
    }
    for(n=0; n < nameLen; n++)
    {
	name[n] = GetByte();
    }
    name[n]=0;

    font->setFontName(name);

    delete name;

    flags = (FontFlags)GetByte();

    font->setFontFlags(flags);

    nb = font->getNbGlyphs();

    lut = new long[nb];
    if (lut == NULL) {
    	outOfMemory = 1;
    	delete font;
	return;
    }

    for(n=0; n < nb; n++)
    {
	if (flags & fontWideCodes) {
	    lut[n] = GetWord();
	} else {
	    lut[n] = GetByte();
	}
    }

    font->setFontLookUpTable(lut);
}





void CInputScript::ParseDefineFont2()
{
    int n;
    U32 tagid = (U32) GetWord();
    FontFlags	 flags;
    char		*name;
    long		 nameLen;
    long		 fontGlyphCount;
    long		*offsetTable = NULL;
    Shape       	*shapes = NULL;
    long        	 start;
    SwfFont     	*font;
    long 		*lut = NULL;

    font = new SwfFont(tagid);
    if (font == NULL) {
    	goto memory_error;
    }

    flags = (FontFlags)GetWord();

    font->setFontFlags(flags);

    nameLen = GetByte();
    name = new char[nameLen+1];
    if (name == NULL) {
    	goto memory_error;
    }
    for(n=0; n < nameLen; n++)
    {
	name[n] = GetByte();
    }
    name[n]=0;

    font->setFontName(name);

    delete name;

    fontGlyphCount = GetWord();

    start = m_filePos;

    offsetTable = new long[fontGlyphCount];
    if (offsetTable == NULL) {
    	goto memory_error;
    }
    for (n=0; n<fontGlyphCount; n++) {
	if (flags & 8) {
	    offsetTable[n] = GetDWord();
	} else {
	    offsetTable[n] = GetWord();
	}
    }

    shapes = new Shape[fontGlyphCount];
    if (shapes == NULL) {
    	goto memory_error;
    }

    for (n=0; n<fontGlyphCount; n++) {
	long here;

	m_filePos = offsetTable[n]+start;

	here = m_filePos;
	ParseShapeData(0, 0);

	// Keep data for later parsing
	shapes[n].file_ptr = (unsigned char*)malloc(m_filePos-here);
	if (shapes[n].file_ptr == NULL) {
		goto memory_error;
	}
	memcpy((void*)shapes[n].file_ptr,(void*)&m_fileBuf[here],m_filePos-here);
    }

    font->setFontShapeTable(shapes,fontGlyphCount);

    lut = new long[fontGlyphCount];
    if (lut == NULL) {
    	goto memory_error;
    }

    for(n=0; n < fontGlyphCount; n++)
    {
	if (flags & 4) {
	    lut[n] = GetWord();
	} else {
	    lut[n] = GetByte();
	}
    }

    font->setFontLookUpTable(lut);

    delete offsetTable;

    addCharacter(font);

    // This is an incomplete parsing
    return;

memory_error:
    outOfMemory = 1;
    if (font) delete font;
    if (offsetTable) delete offsetTable;
    if (lut) delete lut;
    if (shapes) delete[] shapes;
}

TextRecord * CInputScript::ParseTextRecord(int hasAlpha)
{
    TextRecord *tr;
    TextFlags   flags;

    flags = (TextFlags) GetByte();
    if (flags == 0) return 0;

    tr = new TextRecord;
    if (tr == NULL) {
    	outOfMemory = 1;
	return 0;
    }

    tr->flags = flags;

    if (flags & isTextControl) {
	if (flags & textHasFont) {
	    long fontId;

	    fontId = GetWord();
	    tr->font = (SwfFont *)getCharacter(fontId);
	}
	if (flags & textHasColor) {
	    tr->color.red = GetByte();
	    tr->color.green = GetByte();
	    tr->color.blue = GetByte();
	    if (hasAlpha) {
		tr->color.alpha = GetByte();
	    } else {
		tr->color.alpha = ALPHA_OPAQUE;
	    }
	}
	if (flags & textHasXOffset) {
	    tr->xOffset = GetWord();
	}
	if (flags & textHasYOffset) {
	    tr->yOffset = GetWord();
	}
	if (flags & textHasFont) {
	    tr->fontHeight = GetWord();
	}
	tr->nbGlyphs = GetByte();
    } else {
	tr->flags = (TextFlags)0;
	tr->nbGlyphs = (long)flags;
    }

    tr->glyphs = new Glyph[ tr->nbGlyphs ];
    if (tr->glyphs == NULL) {
    	outOfMemory = 1;
	delete tr;
	return 0;
    }

    InitBits();
    for (int g = 0; g < tr->nbGlyphs; g++)
    {
	tr->glyphs[g].index = GetBits(m_nGlyphBits);
	tr->glyphs[g].xAdvance = GetBits(m_nAdvanceBits);
    }

    return tr;
}

void CInputScript::ParseDefineText(int hasAlpha)
{
    Text		*text;
    TextRecord	*textRecord;
    Matrix  	 m;
    Rect		 rect;
    U32 tagid = (U32) GetWord();

    text = new Text(tagid);
    if (text == NULL) {
    	outOfMemory = 1;
	return;
    }

    GetRect(&rect);
    text->setTextBoundary(rect);

    GetMatrix(&m);
    text->setTextMatrix(m);

    m_nGlyphBits = GetByte();
    m_nAdvanceBits = GetByte();

    do {
	textRecord = ParseTextRecord(hasAlpha);
	if (textRecord) {
	    text->addTextRecord( textRecord );
	}
	if (outOfMemory) {
		delete text;
		return;
	}
	if (m_filePos >= m_tagEnd) break;
    } while (textRecord);

    addCharacter(text);
}


void CInputScript::ParseDefineButtonCxform()
{
    ButtonRecord *br;
    Button	*button;
    U32 tagid = (U32) GetWord();

    button = (Button *)getCharacter(tagid);

    for (br = button->getButtonRecords(); br; br = br->next)
    {
	br->cxform = new Cxform;
	GetCxform(br->cxform, false);
    }
}

void CInputScript::ParseNameCharacter()
{
    U32 tagid = (U32) GetWord();
    char *label = strdup(GetString());

    nameCharacter(tagid, label);
}


void CInputScript::ParseFrameLabel()
{
    char *label = strdup(GetString());
    program->setCurrentFrameLabel(label);
}


void CInputScript::ParseDefineMouseTarget()
{
    printf("tagDefineMouseTarget\n");
}


void CInputScript::ParseDefineSprite()
{
    Sprite  *sprite;
    Program *prg;
    int status;

    U32 tagid = (U32) GetWord();
    U32 frameCount = (U32) GetWord();

    if (frameCount == 0) return;

    printf("tagDefineSprite \ttagid %-5u \tframe count %-5u\n", tagid, frameCount);

    sprite = new Sprite(program->movie, tagid, frameCount);
    if (sprite == NULL) {
    	outOfMemory = 1;
	return;
    }
    if (sprite->getProgram() == NULL) {
    	delete sprite;
    	outOfMemory = 1;
	return;
    }

    prg = sprite->getProgram();

    // Set current program
    program = prg;

    ParseTags(&status);

    if (outOfMemory) {
    	delete sprite;
	return;
    }

    addCharacter(sprite);
}


void CInputScript::ParseUnknown(long code, long len)
{
    printf("Unknown Tag : %d  - Length = %d\n", code, len);
}


void
CInputScript::ParseTags(int *status)
	// Parses the tags within the file.
{

    // Initialize the end of frame flag.
    BOOL atEnd = false;

    // Loop through each tag.
    while (!atEnd)
    {
	U32 here;

	// Get the current tag.
	U16 code = GetTag();

	if (code == notEnoughData) {
		m_filePos = m_tagStart;
		*status |= FLASH_PARSE_NEED_DATA;
		return;
	}

	//printf("Code %d, tagLen %8u \n", code, m_tagLen);

	here = m_filePos;

	// Get the tag ending position.
	U32 tagEnd = m_tagEnd;

	if (m_tagEnd > m_actualSize) {
		m_filePos = m_tagStart;
		*status |= FLASH_PARSE_NEED_DATA;
	    	return;
	}

	switch (code)
	{
	case stagProtect:
	    break;

	case stagEnd:

	    // We reached the end of the file.
	    atEnd = true;

	    printf("End of Movie\n");

	    break;

	case stagShowFrame:

	    // Validate frame
	    program->validateLoadingFrame();
	    *status |= FLASH_PARSE_WAKEUP;

	    break;

	case stagFreeCharacter:
	    ParseFreeCharacter();
	    break;

	case stagPlaceObject:
	    ParsePlaceObject();
	    break;

	case stagPlaceObject2:
	    ParsePlaceObject2();
	    break;

	case stagRemoveObject:
	    ParseRemoveObject();
	    break;

	case stagRemoveObject2:
	    ParseRemoveObject2();
	    break;

	case stagSetBackgroundColor:
	    ParseSetBackgroundColor();
	    break;

	case stagDoAction:
	    ParseDoAction();
	    break;

	case stagDefineShape: 
	    ParseDefineShape(1);
	    break;

	case stagDefineShape2:
	    ParseDefineShape(2);
	    break;

	case stagDefineShape3:
	    ParseDefineShape(3);
	    break;

	case stagDefineBits:
	    ParseDefineBits();
	    break;

	case stagDefineBitsJPEG2:
	    ParseDefineBitsJPEG2();
	    break;

	case stagDefineBitsJPEG3:
	    ParseDefineBitsJPEG3();
	    break;

	case stagDefineBitsLossless:
	    ParseDefineBitsLossless(1);
	    break;

	case stagDefineBitsLossless2:
	    ParseDefineBitsLossless(2);
	    break;

	case stagJPEGTables:
	    ParseJPEGTables();
	    break;

	case stagDefineButton:
	    ParseDefineButton();
	    break;

	case stagDefineButton2:
	    ParseDefineButton2();
	    break;

	case stagDefineFont:
	    ParseDefineFont();
	    break;

	case stagDefineMorphShape:
	    ParseDefineMorphShape();
	    break;

	case stagDefineFontInfo:
	    ParseDefineFontInfo();
	    break;

	case stagDefineText:
	    ParseDefineText(0);
	    break;

	case stagDefineText2:
	    ParseDefineText(1);
	    break;

	case stagDefineButtonCxform:
	    ParseDefineButtonCxform();
	    break;

	case stagDefineSprite:
	    Program *save;

	    save = program;
	    ParseDefineSprite();
	    program->rewindMovie();
	    program = save;
	    break;

	case stagNameCharacter:
	    ParseNameCharacter();
	    break;

	case stagFrameLabel:
	    ParseFrameLabel();
	    break;

	case stagDefineFont2:
	    ParseDefineFont2();
	    break;

	default:
	    ParseUnknown(code, m_tagLen);
	    break;
	}

	//printf("Bytes read = %d\n", m_filePos-here);

	// Increment the past the tag.
	m_filePos = tagEnd;

	if (outOfMemory) {
		fprintf(stderr,"Flash: Out of memory\n");
		*status |= FLASH_PARSE_OOM;
		return;
	}
    }

    program->validateLoadingFrame();
    *status |= FLASH_PARSE_EOM;
}

int
CInputScript::ParseData(FlashMovie *movie, char * data, long size)
{
    int status = FLASH_PARSE_ERROR;

    m_fileBuf = (unsigned char *)data;
    m_actualSize = size;

    if (needHeader) {

	    // Do we have sufficient data to read the header ?
	    if (size < 21) {
		return FLASH_PARSE_NEED_DATA;	// No, need more data
	    }

	    needHeader = 0;	// Yes

	    U8 fileHdr[8];

	    memcpy(fileHdr,data,8);

	    // Verify the header and get the file size.
	    if (fileHdr[0] != 'F' || fileHdr[1] != 'W' || fileHdr[2] != 'S' )
	    {
		//fprintf(stderr, "Not a Flash File.\n");
		return FLASH_PARSE_ERROR;	// Error
	    }
	    else
	    {
		// Get the file version.
		m_fileVersion = (U16) fileHdr[3];
	    }

	    // Get the file size.
	    m_fileSize = (U32) fileHdr[4]
	    	      | ((U32) fileHdr[5] << 8)
		      | ((U32) fileHdr[6] << 16)
		      | ((U32) fileHdr[7] << 24);

	    // Verify the minimum length of a Flash file.
	    if (m_fileSize < 21)
	    {
		//printf("ERROR: File size is too short\n");
		return FLASH_PARSE_ERROR;	// Error
	    }

	    // Set the file position past the header and size information.
	    m_filePos = 8;

	    // Get the frame information.
	    GetRect(&frameRect);

	    frameRate = GetWord() >> 8;

	    frameCount = GetWord();

	    program = new Program(movie, frameCount);

	    if (program == NULL || program->totalFrames == 0) {
	    	return FLASH_PARSE_ERROR;
	    }

	    status |= FLASH_PARSE_START;
    }

    ParseTags(&status);

    return status;
}


