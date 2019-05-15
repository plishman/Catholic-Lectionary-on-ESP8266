#pragma once

#ifndef _FONT_H
#define _FONT_H

#include "RCGlobals.h"

#include <Arduino.h>
#include <SD.h>
#define FS_NO_GLOBALS
#include <FS.h>
#include <math.h>

#ifdef EPAPER_GxGDEW027C44
	#include <GxEPD.h>
	#include <GxGDEW027C44/GxGDEW027C44.h>      // 2.7" b/w/r 176x264 GxGDEW027C44/GxGDEW027C44.cpp
#endif

#ifdef EPAPER_GxGDEW042Z15
	#include <GxEPD.h>
	#include <GxGDEW042Z15/GxGDEW042Z15.h>      // 4.2" b/w/r
#endif

#include <utf8string.h>
#include <ArabicLigaturizer.h>

#include "DebugPort.h"
#include "I18n.h"
#include "FCICache.h"
#include "FrameBuffer.h"

extern "C" {
#include "user_interface.h"
}
//extern const uint16_t colored;
//extern const int UNCOLORED;

//extern const int PANEL_SIZE_X;
//extern const int PANEL_SIZE_Y;

const unsigned long FONT_HEADER_OFFSET_START = 8;
//const unsigned long FONT_HEADER_OFFSET_CHARHEIGHT = 8; // word
//const unsigned long FONT_HEADER_OFFSET_STARTCHAR = 10; // dword
//const unsigned long FONT_HEADER_OFFSET_ENDCHAR = 14; // dword
//const unsigned long FONT_HEADER_OFFSET_NUMLOOKUPENTRIES = 18; // word
//const unsigned long FONT_HEADER_OFFSET_SPACECHARWIDTH = 20; // byte
//const unsigned long FONT_HEADER_OFFSET_DEPTH_BPP = 21; // byte
//const unsigned long FONT_HEADER_OFFSET_END = 22;
//const unsigned long BLOCKTABLE_OFFSET_BEGIN = FONT_HEADER_OFFSET_END;

typedef struct {
	uint32_t startchar;
	uint32_t endchar;
	uint32_t fileoffset_startchar;
} DiskFont_BlocktableEntry;

/*
typedef struct {	// 32 bytes
	uint16_t widthbits;
	uint16_t heightbits;
	uint32_t bitmapfileoffset;
	double advanceWidth;
	double advanceHeight;
	//uint32_t useCount;
	//uint32_t codepoint;
} DiskFont_FontCharInfo;

const int DiskFont_FontCharInfo_RecSize = 2+2+4+8+8; //bytes
*/

////#include "romfont.h"
#include "times10x4.h"
//#include "calibri10pt.h"
//#include "calibri20pt.h"



class DiskFont {
public:
	FCICache fciCache;
	
	//FONT_INFO* romfont = &calibri_10pt; // 19-12-2018 20pt 10pt
	FONT_INFO* romfont = &timesNewRoman_10pt; // 29-12-2018 20pt 10pt

	String _fontfilename;
	
	File _file_sd;
	fs::File _file_spiffs;
	
	double 	_font_tuning_percent = 50.0;
	bool	_font_use_fixed_spacing = false;
	bool	_font_use_fixed_spacecharwidth = false;
	int		_font_fixed_spacing = 1;
	int		_font_fixed_spacecharwidth = 2;

	bool available = false;

	struct {
		uint16_t charheight;
		uint32_t startchar;
		uint32_t endchar;
		uint16_t numlookupblocks;
		uint8_t spacecharwidth;
		double ascent;
		double descent;
		double linespacing;
		uint8_t antialias_level;
	} _FontHeader;
	
	DiskFont_BlocktableEntry* _Font_BlocktablePtr = NULL;
	
	#define CHARBUFF_SIZE 512	// was 32
	uint8_t _char_buffer[CHARBUFF_SIZE];
	
	DiskFont(FB_EPAPER ePaper);	
	FB_EPAPER_PTR _p_ePaper = NULL;

	~DiskFont();
	bool begin();
	bool begin(String fontfilename);
	bool begin(String fontfilename, double font_tuning_percent);
	bool begin(ConfigParams c);
	void end(void);
	void clear(); // does the same thing as end, but can be called as an alternative when diskfont.begin() has not been called (means the rom/default font is selected)

	bool OpenFontFile();
	bool CloseFontFile();
	bool Seek(uint64_t offset);
	size_t Position();
	size_t Size();
	bool Read(uint32_t* var);
	bool Read(uint16_t* var);
	bool Read(uint8_t* var);
	bool Read(double* var);
	bool Read(void* array, uint32_t bytecount);
	
	
	// for diskfont characters (will fail over to romfont rendering if diskfont is not available)
	int DrawCharAt(int x, int y, char ascii_char,    FB_EPAPER ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, String ch,          FB_EPAPER ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, uint32_t codepoint, FB_EPAPER ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	
	int DrawCharAt(int x, int y, char ascii_char, 	 double& advanceWidth, FB_EPAPER ePaper, uint16_t colored, uint16_t* blockToCheckFirst);	
	int DrawCharAt(int x, int y, String ch, 		 double& advanceWidth, FB_EPAPER ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, FB_EPAPER ePaper, uint16_t colored, uint16_t* blockToCheckFirst);

	int DrawCharAt(int x, int y, char ascii_char,    DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, String ch,          DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, uint32_t codepoint, DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);

	int DrawCharAt(int x, int y, char ascii_char, 	 double& advanceWidth, DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, String ch, 		 double& advanceWidth, DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);

	// for romfont characters
	int DrawCharAt(int x, int y, char ascii_char, double& advanceWidth, FONT_INFO* font, DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, FONT_INFO* font, DiskFont_FontCharInfo& fci, FB_EPAPER ePaper, uint16_t colored);

	void StripTags(String& text);
	
	void DrawStringAt(int x, int y, String text, FB_EPAPER ePaper, uint16_t color, bool right_to_left, bool reverse_string);
	void DrawStringAt(int x, int y, String text, FB_EPAPER ePaper, uint16_t color, bool right_to_left);

	void DrawStringAt(int x, int y, String text, FB_EPAPER ePaper, String colormap, bool right_to_left, bool reverse_string);
	void DrawStringAt(int x, int y, String text, FB_EPAPER ePaper, String colormap, bool right_to_left);

	void DrawStringAt(int x, int y, String text, FB_EPAPER ePaper, uint16_t color, String colormap, bool right_to_left, bool reverse_string);
		
	void GetTextWidth(String text, int& width, double& advanceWidth, int limit);
	void GetTextWidth(String text, int& width, double& advanceWidth);

	int GetTextWidth(const char* text, int limit);
	int GetTextWidth(String text, int limit);
	int GetTextWidth(const char* text);
	int GetTextWidth(String text);

	double GetAdvanceWidth(uint16_t bitmapwidth, double advanceWidth, uint32_t codepoint, uint16_t& char_width);
//	double GetAdvanceWidth(int bitmapwidth, double advanceWidth);
	
	void GetCharWidth(uint32_t codepoint, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo* &pfci, uint16_t& blockToCheckFirst);
	void GetCharWidth(uint32_t codepoint, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo* &pfci);
                                          
	void GetCharWidth(String ch, 		  uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo* &pfci, uint16_t& blockToCheckFirst);
	void GetCharWidth(String ch, 		  uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo* &pfci);

	double GetCharWidth(uint32_t codepoint, DiskFont_FontCharInfo* &fci, uint16_t& blockToCheckFirst);
	double GetCharWidth(uint32_t codepoint, DiskFont_FontCharInfo* &fci);

	double GetCharWidth(String ch, 			DiskFont_FontCharInfo* &fci, uint16_t& blockToCheckFirst);
	double GetCharWidth(String ch, 			DiskFont_FontCharInfo* &fci);
	
	double GetTextWidthA(String text);
	double GetTextWidthA(String text, bool shape_text);
	double GetTextWidthA(const char* text);

	double GetTextWidthA(String text, int limit);
	double GetTextWidthA(String text, bool shape_text, int limit);
	double GetTextWidthA(const char* text, int limit);
		
	bool getCharInfo(String ch, DiskFont_FontCharInfo* &pfci);
	bool getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* &pfci);
	bool readFontCharInfoEntry(DiskFont_FontCharInfo* &pfci, uint32_t codepoint);
	
	const FONT_CHAR_INFO* getCharInfo(int codepoint, uint16_t* blockToCheckFirst, FONT_INFO* font);

	int _displayPage = 0;
	void setDisplayPage(int displayPageNumber);
	bool IsInPage(int displayPageNumber, DiskFont_FontCharInfo& fci, int codepoint, int x0, int y0, int x1, int y1);
	void swap(int& a, int& b);
};

class FontBlockTable {
public:
	FontBlockTable();
	~FontBlockTable();
};

#endif
