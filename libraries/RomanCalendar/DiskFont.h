#pragma once

#ifndef _FONT_H
#define _FONT_H

#include "RCGlobals.h"

#include <Arduino.h>
#include <SD.h>
#define FS_NO_GLOBALS
#include <FS.h>
#include <math.h>
//#include <epd2in7b.h>
//#include <epdpaint.h>

#include <GxEPD.h>
#include <GxGDEW027C44/GxGDEW027C44.h>      // 7.5" b/w  640x384 GxGDEW075T8/GxGDEW075T8.cpp  // 2.7" color 176x264 GxGDEW027C44/GxGDEW027C44.cpp

#include <utf8string.h>
#include <ArabicLigaturizer.h>
#include "I2CSerialPort.h"
#include "I18n.h"

extern "C" {
#include "user_interface.h"
}
extern const uint16_t colored;
extern const int UNCOLORED;

extern const int PANEL_SIZE_X;
extern const int PANEL_SIZE_Y;

const unsigned long FONT_HEADER_OFFSET_START = 8;
const unsigned long FONT_HEADER_OFFSET_CHARHEIGHT = 8; // word
const unsigned long FONT_HEADER_OFFSET_STARTCHAR = 10; // dword
const unsigned long FONT_HEADER_OFFSET_ENDCHAR = 14; // dword
const unsigned long FONT_HEADER_OFFSET_NUMLOOKUPENTRIES = 18; // word
const unsigned long FONT_HEADER_OFFSET_SPACECHARWIDTH = 20; // byte
const unsigned long FONT_HEADER_OFFSET_END = 21;
const unsigned long BLOCKTABLE_OFFSET_BEGIN = FONT_HEADER_OFFSET_END;

typedef struct {
	uint32_t startchar;
	uint32_t endchar;
	uint32_t fileoffset_startchar;
} DiskFont_BlocktableEntry;

typedef struct {
	//uint8_t rtlflag;
	uint16_t widthbits;
	uint16_t heightbits;
	uint32_t bitmapfileoffset;
	double advanceWidth;
	double advanceHeight;
} DiskFont_FontCharInfo;

const int DiskFont_FontCharInfo_RecSize = 2+2+4+8+8; //bytes

//#include "romfont.h"
#include "calibri10pt.h"


class DiskFont {
public:
	FONT_INFO* romfont = &calibri_10pt;

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
	} _FontHeader;
	
	DiskFont_BlocktableEntry* _Font_BlocktablePtr = NULL;
	
	#define CHARBUFF_SIZE 32
	uint8_t _char_buffer[CHARBUFF_SIZE];
	
	DiskFont();
	~DiskFont();
	bool begin();
	bool begin(String fontfilename);
	bool begin(String fontfilename, double font_tuning_percent);
	bool begin(ConfigParams c);
	void end(void);

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
	int DrawCharAt(int x, int y, char ascii_char,    GxEPD_Class& ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, String ch,          GxEPD_Class& ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, uint32_t codepoint, GxEPD_Class& ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	
	int DrawCharAt(int x, int y, char ascii_char, 	 double& advanceWidth, GxEPD_Class& ePaper, uint16_t colored, uint16_t* blockToCheckFirst);	
	int DrawCharAt(int x, int y, String ch, 		 double& advanceWidth, GxEPD_Class& ePaper, uint16_t colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, GxEPD_Class& ePaper, uint16_t colored, uint16_t* blockToCheckFirst);

	int DrawCharAt(int x, int y, char ascii_char,    DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, String ch,          DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, uint32_t codepoint, DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);

	int DrawCharAt(int x, int y, char ascii_char, 	 double& advanceWidth, DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, String ch, 		 double& advanceWidth, DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);

	// for romfont characters
	int DrawCharAt(int x, int y, char ascii_char, double& advanceWidth, FONT_INFO* font, DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);
	int DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, FONT_INFO* font, DiskFont_FontCharInfo& fci, GxEPD_Class& ePaper, uint16_t colored);

	
	
	void DrawStringAt(int x, int y, String text, GxEPD_Class& ePaper, uint16_t colored, bool right_to_left, bool reverse_string);
	void DrawStringAt(int x, int y, String text, GxEPD_Class& ePaper, uint16_t colored, bool right_to_left);
	
	
	void GetTextWidth(String text, int& width, double& advanceWidth);
	int GetTextWidth(const char* text);
	int GetTextWidth(String text);

	double GetAdvanceWidth(uint16_t bitmapwidth, double advanceWidth, uint32_t codepoint, uint16_t& char_width);
//	double GetAdvanceWidth(int bitmapwidth, double advanceWidth);
	
	void GetCharWidth(uint32_t codepoint, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst);
	void GetCharWidth(uint32_t codepoint, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci);
                                          
	void GetCharWidth(String ch, 		  uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst);
	void GetCharWidth(String ch, 		  uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci);

	double GetCharWidth(uint32_t codepoint, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst);
	double GetCharWidth(uint32_t codepoint, DiskFont_FontCharInfo& fci);

	double GetCharWidth(String ch, 			DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst);
	double GetCharWidth(String ch, 			DiskFont_FontCharInfo& fci);
	
	double GetTextWidthA(String text);
	double GetTextWidthA(String text, bool shape_text);
	double GetTextWidthA(const char* text);
		
	bool getCharInfo(String ch, DiskFont_FontCharInfo* fci);
	bool getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* fci);
	bool readFontCharInfoEntry(DiskFont_FontCharInfo* fci);
	
	const FONT_CHAR_INFO* getCharInfo(int codepoint, uint16_t* blockToCheckFirst, FONT_INFO* font);
};

class FontBlockTable {
public:
	FontBlockTable();
	~FontBlockTable();
};

#endif
