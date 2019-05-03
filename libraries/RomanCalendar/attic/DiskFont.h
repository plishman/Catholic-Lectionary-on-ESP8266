#pragma once

#ifndef _FONT_H
#define _FONT_H

#include "Arduino.h"
#include <epd2in7b.h>
#include <epdpaint.h>
#include "DebugPort.h"
//#include "I18n.h"

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
	uint16_t widthbits;
	uint16_t heightbits;
	uint32_t bitmapfileoffset;
} DiskFont_FontCharInfo;

class DiskFont {
public:
	//I18n* _I18n;
	String _fontfilename;
	File _file;
	
	struct _FontHeader {
		uint16_t charheight;
		uint32_t startchar;
		uint32_t endchar;
		uint16_t numlookupblocks;
		uint8_t spacecharwidth;
	};
	
	DiskFont_BlocktableEntry* _Font_BlocktablePtr;
	
	#define CHARBUFF_SIZE 32;
	uint8_t _char_buffer[CHARBUFF_SIZE];
	
	DiskFont();
	~DiskFont();
	
	bool begin(/*I18n* i18n,*/String fontfilename);
	void end();
	
	int DrawCharAt(int x, int y, char ascii_char, Paint* paint, int colored, uint16_t* blockToCheckFirst);
	int DrawCharAt(int x, int y, int codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst);

	void DrawStringAt(int x, int y, const char* text, Paint* paint, int colored);
	void DrawStringAt(int x, int y, String text, Paint* paint, int colored);
	
	int GetTextWidth(const char* text);
	int GetTextWidth(String text);
	
	int codepointUtf8(String c);
	String utf8fromCodepoint(int c);
	String utf8CharAt(String s, int pos);
	int charLenBytesUTF8(char s);
	
	bool getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* fci);
	bool readFontCharInfoEntry(DiskFont_FontCharInfo* fci);
	bool ReadUInt32BigEndian(uint32_t* dword);
	bool ReadUInt16BigEndian(uint16_t* word);
	bool ReadUInt8(uint8_t* byte);
};

class FontBlockTable {
public:
	FontBlockTable();
	~FontBlockTable();
};

#endif
