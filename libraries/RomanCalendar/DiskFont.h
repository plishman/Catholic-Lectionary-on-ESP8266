#pragma once

#ifndef _FONT_H
#define _FONT_H

#include "Arduino.h"
#include "SD.h"
#include <epd2in7b.h>
#include <epdpaint.h>
#include <utf8string.h>
#include "I2CSerialPort.h"
//#include "I18n.h"

extern "C" {
#include "user_interface.h"
}
extern const int COLORED;
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

class DiskFont {
public:
	//I18n* _I18n;
	String _fontfilename;
	File _file;
	bool available = false;
	
	struct {
		uint16_t charheight;
		uint32_t startchar;
		uint32_t endchar;
		uint16_t numlookupblocks;
		uint8_t spacecharwidth;
	} _FontHeader;
	
	DiskFont_BlocktableEntry* _Font_BlocktablePtr = NULL;
	
	#define CHARBUFF_SIZE 32
	uint8_t _char_buffer[CHARBUFF_SIZE];
	
	DiskFont();
	~DiskFont();
	
	bool begin(/*I18n* i18n,*/String fontfilename);
	void end(void);
	
	int DrawCharAt(int x, int y, char ascii_char, Paint* paint, int colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, uint32_t codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, double& advanceWidth, uint32_t codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst);	

	void DrawStringAt(int x, int y, const char* text, Paint* paint, int colored, bool right_to_left, bool reverse_string);
	void DrawStringAt(int x, int y, String text, Paint* paint, int colored, bool right_to_left, bool reverse_string);
	void DrawStringAt(int x, int y, String text, Paint* paint, int colored, bool right_to_left);
	
	void DrawStringAtA(int x, int y, String text, Paint* paint, int colored, bool right_to_left, bool reverse_string);
	
	//bool doRtlStrings(String* s, bool right_to_left); // will process input string so that substrings are in correct rtl or ltr order, depending on characterset
	
	void GetTextWidth(String text, int& width, double& advanceWidth);
	int GetTextWidth(const char* text);
	int GetTextWidth(String text);
	double GetTextWidth(String text);
	double GetTextWidth(const char* text);
	
//	String Utf8ReverseString(String instr); // function to reverse utf-8 string
//	int codepointUtf8(String c);
//	String utf8fromCodepoint(int c);
//	String utf8CharAt(String s, int pos);
//	int charLenBytesUTF8(char s);
//	//bool isRtlChar(String ch);
	
	bool getCharInfo(String ch, DiskFont_FontCharInfo* fci);
	bool getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* fci);
	bool readFontCharInfoEntry(DiskFont_FontCharInfo* fci);
	bool ReadUInt32LittleEndian(uint32_t* dword);
	bool ReadUInt16LittleEndian(uint16_t* word);
	bool ReadUInt8(uint8_t* byte);
	bool ReadDouble(double* d);
};

class FontBlockTable {
public:
	FontBlockTable();
	~FontBlockTable();
};

#endif
