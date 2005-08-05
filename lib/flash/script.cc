#include "swf.h"

////////////////////////////////////////////////////////////
//  This file is derived from the 'buggy' SWF parser provided
//  by Macromedia.
//
//  Modifications : Olivier Debon  <odebon@club-internet.fr>
//  

#ifdef RCSID
static char *rcsid = "$Id: script.cc,v 1.6 2004/12/22 18:32:44 tgc Exp $";
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

    // Initialize file decompression.
    m_zBuffer = NULL;
    m_zInitialized = false;
    m_lastSize = 0;

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
    needFileID = TRUE;
    program = 0;

    outOfMemory = 0;

    next = NULL;

    // Streaming sound
    streamID=3000;
    streamNew=0;

    m_bCompressed = FALSE;
    m_bExe = FALSE;

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

    // Shut down the decompresser if it is running.
    if (m_zInitialized)
    {
	inflateEnd(&m_zstream);
	m_zInitialized = false;
    }

    // Free the decompression buffer if it is there.
    if (m_zBuffer)
    {
	delete m_zBuffer;
	 m_zBuffer = NULL;
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


void CInputScript::ParseStartSound()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->character = getCharacter(GetWord());
    ctrl->type = ctrlStartSound;

    program->addControlInCurrentFrame(ctrl);

    if (!m_dumpAll)
	return;

    U32 code = GetByte();

    printf("code %-3u", code);

    if ( code & soundHasInPoint )
	printf(" inpoint %u ", GetDWord());
    if ( code & soundHasOutPoint )
	printf(" oupoint %u", GetDWord());
    if ( code & soundHasLoops )
	printf(" loops %u", GetWord());

    printf("\n");
    if ( code & soundHasEnvelope ) 
    {
	int points = GetByte();

	for ( int i = 0; i < points; i++ ) 
	{
	    printf("\n");
	    printf("mark44 %u", GetDWord());
	    printf(" left chanel %u", GetWord());
	    printf(" right chanel %u", GetWord());
	    printf("\n");
	}
    }
}


void CInputScript::ParseStopSound()
{
    Control *ctrl;

    ctrl = new Control;
    if (ctrl == NULL) {
	outOfMemory = 1;
    	return;
    }
    ctrl->type = ctrlStopSound;

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


void CInputScript::ParseDefineSound()
{
    Sound		*sound;
    U32 tagid = (U32) GetWord();
    long		 nbSamples,skipSize;
    long		 flags;
    char		*buffer;

    sound = new Sound(tagid);

    flags = GetByte();
    sound->setSoundFlags(flags);

    switch (sound->getFormat()) {
    	// Raw
    	case 0:
	case 3:
		nbSamples = GetDWord();
		buffer = sound->setNbSamples(nbSamples);
		if (buffer == NULL) {
			outOfMemory = 1;
			delete sound;
			return;
		}
		memcpy(buffer, &m_fileBuf[m_filePos], m_tagLen-5);
		break;
	// ADPCM
	case 1:
		nbSamples = GetDWord();
		buffer = sound->setNbSamples(nbSamples);
		if (buffer == NULL) {
			outOfMemory = 1;
			delete sound;
			return;
		}
		Adpcm		*adpcm;
		adpcm = new Adpcm( &m_fileBuf[m_filePos] , flags & soundIsStereo );
		adpcm->Decompress((short *)buffer, nbSamples);
		delete adpcm;
		break;
	// MP3
	case 2:
		nbSamples = GetDWord();
		buffer = sound->setBuffer(m_tagLen-4);
		if (buffer == NULL) {
			outOfMemory = 1;
			delete sound;
			return;
		}
		skipSize = GetWord();
		fprintf(stderr, "New block\n");
		sound->setSoundSize(m_tagLen-4);
		memcpy(buffer, &m_fileBuf[m_filePos], m_tagLen-4);
		break;
	
	// Nellymoser ASAO
	case 6:
		fprintf(stderr, "Unsupported sound compression: Nellymoser\n");
		break;
    }

    addCharacter(sound);
}


void CInputScript::ParseDefineButtonSound()
{
    U32 tagid = (U32) GetWord();
    Button	*button;

    tagid = tagid;

    printf("tagDefineButtonSound \ttagid %-5u\n", tagid);

    button = (Button *)getCharacter(tagid);

    if (button == 0) {
	printf("	Couldn't find Button id %d\n", tagid);
	return;
    }

    // step through for button states
    for (int i = 0; i < 4; i++)
    {
	Sound	*sound;
	U32 soundTag = GetWord();

	sound = (Sound *)getCharacter(soundTag);

	if (sound) {
	    button->setButtonSound(sound,i);
	} else if (soundTag) {
	    printf("	Couldn't find Sound id %d\n", soundTag);
	}

	switch (i)
	{
	case 0:         
	    printf("upState \ttagid %-5u\n", soundTag);
	    break;
	case 1:            
	    printf("overState \ttagid %-5u\n", soundTag);
	    break;
	case 2:            
	    printf("downState \ttagid %-5u\n", soundTag);
	    break;
	}

	if (soundTag)
	{
	    U32 code = GetByte();
	    printf("sound code %u", code);

	    if ( code & soundHasInPoint )
		printf(" inpoint %u", GetDWord());
	    if ( code & soundHasOutPoint )
		printf(" outpoint %u", GetDWord());
	    if ( code & soundHasLoops )
		printf(" loops %u", GetWord());

	    printf("\n");
	    if ( code & soundHasEnvelope ) 
	    {
		int points = GetByte();

		for ( int p = 0; p < points; p++ ) 
		{
		    printf("\n");
		    printf("mark44 %u", GetDWord());
		    printf(" left chanel %u", GetWord());
		    printf(" right chanel %u", GetWord());
		    printf("\n");
		}
	    }
	}
	if (m_filePos == m_tagEnd) break;
    }
}

void CInputScript::ParseSoundStreamHead()
{
    int tmp = GetByte();
    tmp = GetByte();
    if (tmp != 0) {
    	streamFlags = tmp;
	streamNew = 1;
	streamID++;
	fprintf(stderr, "new stream 1, id: %d\n", streamID);
    }

}

void CInputScript::ParseSoundStreamHead2()
{
    int tmp = GetByte();
    tmp = GetByte();
    if (tmp != 0) {
    	streamFlags = tmp;
	streamNew = 1;
	streamID++;
	fprintf(stderr, "new stream 2, id: %d\n", streamID);
    }

}

void CInputScript::ParseSoundStreamBlock()
{
    Sound		*sound;
    long		 skipSize, oldsize, this_frame, next_frame;
    char		*buffer;
    
    if (m_tagLen < 5) return;
    
    skipSize = GetWord();
    skipSize = GetWord();
    

    if (streamNew) {
    	fprintf(stderr, "samples: %d\n", skipSize);
 	sound = new Sound(streamID);

	sound->setSoundFlags(streamFlags);

	buffer = sound->setBuffer(m_tagLen-4);
	if (buffer == NULL) {
	  	outOfMemory = 1;
		delete sound;
		return;
	}
	sound->setSoundSize(m_tagLen-4);
    	memcpy(buffer, &m_fileBuf[m_filePos], m_tagLen-4);
	addCharacter(sound);

	Control *ctrl;
	streamNew=0;

	ctrl = new Control;
    	if (ctrl == NULL) {
		outOfMemory = 1;
    		return;
    	}
    	ctrl->character = getCharacter(streamID);
    	ctrl->type = ctrlStartSound;

    	program->addControlInCurrentFrame(ctrl);
    } else {
    	sound = (Sound*) getCharacter(streamID);
	oldsize = sound->getSoundSize();
	buffer = sound->resetBuffer(m_tagLen-4);
    	memcpy(buffer+oldsize, &m_fileBuf[m_filePos], m_tagLen-4);

	// If playback already has started, we must update 
	// the SoundList as well...
	if (sound->getPlaybackStarted()) {
		SoundList* sl = sound->getSound();
		this_frame =  (char*) sl->mp3Stream.this_frame - sl->originalMp3;
		next_frame =  (char*) sl->mp3Stream.next_frame - sl->originalMp3;
		
		sl->currentMp3 = sound->getSamples();
		sl->originalMp3 = sl->currentMp3;

		sl->remainingMp3 += m_tagLen-4;

		mad_stream_buffer (&sl->mp3Stream, (unsigned char*)sl->currentMp3, sl->remainingMp3);

		sl->mp3Stream.this_frame = sl->mp3Stream.this_frame + this_frame;
		sl->mp3Stream.next_frame = sl->mp3Stream.next_frame + next_frame;
		
	}
    }


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

	case stagStartSound:
	    ParseStartSound();
	    break;

	case stagStopSound:
	    ParseStopSound();
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

	case stagDefineSound:
	    ParseDefineSound();
	    break;

	case stagDefineButtonSound:
	    ParseDefineButtonSound();
	    break;

	case stagSoundStreamHead:
	    ParseSoundStreamHead();
	    break;

	case stagSoundStreamHead2:
	    ParseSoundStreamHead2();
	    break;

	case stagSoundStreamBlock:
	    ParseSoundStreamBlock();
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
    static long lLastSize = 0;
    int status = FLASH_PARSE_ERROR;
    int zstatus;

    m_fileBuf = (unsigned char *)data;
    m_actualSize = size;

    if (needHeader) {

	    // Do we have sufficient data to read the header ?
	    if (size < 21) {
		return FLASH_PARSE_NEED_DATA;	// No, need more data
	    }

	    U8 fileHdr[8];


	    // recognize TRUE pure SWF files
	    if( data[1] == 'W' && data[2] == 'S' && 0 == lLastSize )
	    {
		if(data[0] == 'C' || data[0] == 'F')
		{
		    needFileID = FALSE;
		    memcpy(fileHdr,data,8);
		    m_filePos = 8;
		}
	    }

	    if( needFileID )
	    {
		char * pos = data + lLastSize;
		char * end = data + size - 21;
		// try to see if we have an embeded SWF file (exe)
		for(  ; pos < end && (pos[0]!='F' || pos[1]!='W' || pos[2]!='S'); pos++ );

		if( pos < end )
		{
		    // if we are here then we've found a flash header
		    memcpy(fileHdr, pos, 8);
		    m_filePos = 8+(pos-data);
		    m_bExe = TRUE; // Add file format field info]
		    needFileID = FALSE;
		    lLastSize = pos-data; // remember beginning of SWF data in the file
		}
		else
		{
		    lLastSize = size; // remember where we were
		    return FLASH_PARSE_NEED_DATA;
		}
	    }

	    m_bCompressed = (fileHdr[0] == 'C')?TRUE:FALSE;

	    // Get the file version.
	    m_fileVersion = (U16) fileHdr[3];
    
    	    // Get the file size.
	    m_fileSize = (U32) fileHdr[4]
	              | ((U32) fileHdr[5] << 8)
		      | ((U32) fileHdr[6] << 16)
		      | ((U32) fileHdr[7] << 24);


	    // Verify the minimum length of a Flash file.
	    if (m_fileSize < 21)
	    {
		return FLASH_PARSE_ERROR;	// Error
	    }

	    if( m_bCompressed )
	    {
		fprintf(stdout, "decoding...\n");
		// The file is compressed. Create a buffer to hold the
		// uncompressed data.

		m_zBuffer = new U8 [m_fileSize];
		if(m_zBuffer == NULL)
		{
    		    return FLASH_PARSE_OOM;
		}

		// Initialize the ZLib decompressor. We skip the first eight
		// bytes, since we've already processed them.

		m_zstream.next_out = (Bytef *)&m_zBuffer[8];
		m_zstream.avail_out = (uInt)m_fileSize;
		m_zstream.next_in = (Bytef *)&data[8];
		m_zstream.avail_in = 0;
		m_zstream.zalloc = Z_NULL;
		m_zstream.zfree = Z_NULL;
		m_zstream.opaque = Z_NULL;

		zstatus = inflateInit(&m_zstream);
		if(zstatus != Z_OK)
		{
		    return FLASH_PARSE_OOM;
		}
		m_zInitialized = true;

		// We'll be decoding from our own decompressed data buffer.

		m_fileBuf = m_zBuffer;
		m_lastSize = 8;
	    }
	    else
	    {
		// The file is uncompressed, so we'll be decoding it from the
		// caller's buffer.

		m_fileBuf = (U8 *)data;
	    }
	}

	// If we're reading a compressed file, we need to decompress as much of
	// it as we have available into the decompressed data buffer.
	if( m_zBuffer )
	{
	    // Figure out how much new compressed data is available.

	    m_zstream.avail_in = (uInt)(size - m_lastSize);
	    m_lastSize = size;

	    // Decompress it.

	    zstatus = inflate(&m_zstream, Z_SYNC_FLUSH);
	    if(   (zstatus != Z_OK) && (zstatus != Z_STREAM_END) &&
					(zstatus != Z_BUF_ERROR))
	    {
		// There was an error in decompression.

		return FLASH_PARSE_ERROR;
	    }

	    // Figure out how much decompressed data is available.
	    m_actualSize = m_zstream.total_out + 8;
	}
	else
	{
	    // The file is uncompressed, so the amount of it available for
	    // processing is just what the caller thinks it is.
	    // except if we are embeded in another file
	    m_actualSize = size;// - lLastSize;
	}

	if( needHeader )
	{
	    // Do we have sufficient data to read the header ?
	    if( m_actualSize < 21 )
	    {
		return FLASH_PARSE_NEED_DATA; // No, need more data
	    }

	    // Get the frame information.
	    GetRect(&frameRect);
	    frameRate = GetWord() >> 8;
	    frameCount = GetWord();

	    program = new Program(movie, frameCount);

	    if (program == NULL || program->totalFrames == 0)
	    {
		return FLASH_PARSE_ERROR;
	    }

	    status |= FLASH_PARSE_START;
	    needHeader = FALSE;
	}

	ParseTags(&status);

	// If we've reached the end of a compressed movie, we need to shut down
	// the decompressor and dispose of the decompressed data.
	if( m_zBuffer && (status & FLASH_PARSE_EOM) )
	{
	    inflateEnd(&m_zstream);
	    m_zInitialized = false;

	    delete m_zBuffer;
	    m_zBuffer = NULL;
	}

	return status;
}


