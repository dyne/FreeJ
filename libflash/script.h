#ifndef _SCRIPT_H_
#define _SCRIPT_H_

// SWF file parser.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Input script object definition.
//////////////////////////////////////////////////////////////////////

// An input script object.  This object represents a script created from 
// an external file that is meant to be inserted into an output script.
struct CInputScript  : public Dict
{
    int level;
    struct CInputScript *next;

    Program *program;	// Current parsed program

        // Memory fences
	int  outOfMemory;

	//Flash info
	long frameRate;
	long frameCount;
	Rect frameRect;

	// Pointer to file contents buffer.
	U8 *m_fileBuf;

	// File state information.
	U32 m_filePos;
	U32 m_fileSize;
	U32 m_actualSize;
	U32 m_fileStart;
	U16 m_fileVersion;

	int needHeader;

	// Bit Handling
	S32 m_bitPos;
	U32 m_bitBuf;

	// Tag parsing information.
	U32 m_tagStart;
	U32 m_tagEnd;
	U32 m_tagLen;

	// Parsing information.
	S32 m_nFillBits;
	S32 m_nLineBits;
	S32 m_nGlyphBits;
	S32 m_nAdvanceBits;

	// Set to true if we wish to dump all contents long form
	U32 m_dumpAll;

	// if set to true will dump image guts (i.e. jpeg, zlib, etc. data)
	U32 m_dumpGuts;

	// Handle to output file.
	FILE *m_outputFile;

	// Constructor/destructor.
	CInputScript(int level = 0);
	~CInputScript();

	// Tag scanning methods.
	U16 GetTag(void);
	U8 GetByte(void);
	U16 GetWord(void);
	U32 GetDWord(void);
	void GetRect(Rect *r);
	void GetMatrix(Matrix *matrix);

	void GetCxform(Cxform *cxform, BOOL hasAlpha);
	char *GetString(void);

	// Routines for reading arbitrary sized bit fields from the stream.
	// Always call start bits before gettings bits and do not intermix 
	// these calls with GetByte, etc...	
	void InitBits();
	S32 GetSBits(S32 n);
	U32 GetBits(S32 n);

	// Tag subcomponent parsing methods
	void ParseFillStyle(long getAlpha = 0);
	void ParseLineStyle(long getAlpha = 0);
	int  ParseShapeRecord(long getAlpha = 0);
	ButtonRecord *  ParseButtonRecord(long getCxform = 0);
	ActionRecord *  ParseActionRecord();
	TextRecord   *  ParseTextRecord(int hasAlpha = 0);
        void ParseShapeData(int getAlpha, int getStyles);

	// Parsing methods.
	void ParseEnd();				// 00: stagEnd
	void ParseShowFrame(U32 frame, U32 offset);	// 01: stagShowFrame
	void ParseDefineShape(int level);		// 02: stagDefineShape
	void ParseFreeCharacter();                     	// 03: stagFreeCharacter
	void ParsePlaceObject();			// 04: stagPlaceObject
	void ParseRemoveObject();                      	// 05: stagRemoveObject
	void ParseDefineBits();                        	// 06: stagDefineBits
	void ParseDefineButton();       		//x 07: stagDefineButton
	void ParseJPEGTables();		                // 08: stagJPEGTables
	void ParseSetBackgroundColor();                	// 09: stagSetBackgroundColor
	void ParseDefineFont();         		//x 10: stagDefineFont
	void ParseDefineText(int hasAplha);    		//x 11: stagDefineText    33: stagDefineText2
	void ParseDoAction();                          	// 12: stagDoAction	
	void ParseDefineFontInfo();     		//x 13: stagDefineFontInfo
	void ParseDefineSound();                       	// 14: stagDefineSound
	void ParseStartSound();                        	// 15: stagStartSound
	void ParseStopSound();                         	// 16: stagStopSound
	void ParseDefineButtonSound();                 	// 17: stagDefineButtonSound
	void ParseSoundStreamHead(); 	               	// 18: stagSoundStreamHead
	void ParseSoundStreamBlock();                  	// 19: stagSoundStreamBlock
	void ParseDefineBitsLossless(int level);       	// 20: stagDefineBitsLossless 36: stagDefineBitsLossless2
	void ParseDefineBitsJPEG2();                   	// 21: stagDefineBitsJPEG2
	void ParseDefineButtonCxform();	            	// 23: stagDefineButtonCxform
	void ParseProtect();           	            	// 24: stagProtect
	void ParsePlaceObject2();                      	// 26: stagPlaceObject2
	void ParseRemoveObject2();                     	// 28: stagRemoveObject2
	void ParseDefineButton2();      		//x 34: stagDefineButton2
	void ParseDefineBitsJPEG3();                   	// 35: stagDefineBitsJPEG3
	void ParseDefineMouseTarget();                 	// 38: stagDefineMouseTarget
	void ParseDefineSprite();       		//x 39: stagDefineSprite
	void ParseNameCharacter();                     	// 40: stagNameCharacter
	void ParseFrameLabel();                        	// 43: stagFrameLabel
	void ParseSoundStreamHead2(); 	               	// 45: stagSoundStreamHead2
	void ParseDefineMorphShape();   		//x 46: stagDefineMorphShape
	void ParseDefineFont2();        		//x 48: stagDefineFont2
	void ParseUnknown(long,long);

	void ParseTags(int *);
	int  ParseData(FlashMovie *movie, char * data, long size);
	void S_DumpImageGuts();

#ifdef DUMP
	long save(char *filenam);
#endif
};


#endif /* _SCRIPT_H_ */
