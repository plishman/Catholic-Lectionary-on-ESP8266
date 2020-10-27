#ifndef _ROMFONT_H
#define _ROMFONT_H

#include "RCGlobals.h"

#include <stdint.h>
//vv the dot factory font structs - Internal font
// This structure describes a single character's display information
typedef struct
{
	const uint8_t widthBits;				// width, in bits (or pixels), of the character
	const uint8_t heightBits;				// height, in bits (or pixels), of the character
	const uint32_t offset;					// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array
	//PLL-30-07-2020 changed these to floats from doubles - saves 3% flash
	//const double advanceWidth;				// the number of pixels to advance before placing the next character
	//const double advanceHeight;				// the number of pixels to advance vertically before placing the next character
	const float advanceWidth;				// the number of pixels to advance before placing the next character
	const float advanceHeight;				// the number of pixels to advance vertically before placing the next character
} FONT_CHAR_INFO;	

typedef struct 
{
	const uint32_t startChar;
	const uint32_t endChar;
	const FONT_CHAR_INFO* fontcharinfoBlock;
} FONT_CHAR_INFO_LOOKUP;


// Describes a single font
typedef struct
{
	const uint8_t heightPages;		// height, in pages (8 pixels), of the font's characters
	const uint32_t startChar;		// the first character in the font (e.g. in charInfo and data)
	const uint32_t endChar;		// the last character in the font
	const uint8_t spacePixels;		// number of pixels that a space character takes up
	const FONT_CHAR_INFO_LOOKUP* fontcharinfoBlockLookup;		// points to array of fontcharinfo lookup entries
	const FONT_CHAR_INFO* charInfo;		// pointer to array of char information (NULL if multiple blocks are used in the font, in which case FONT_CHAR_INFO_LOOKUP* will be used)
	const uint16_t blockCount;		// number of blocks in font (set to 0 if a single-block font)
	const uint8_t* data;		// pointer to generated array of character visual representation
	const double ascent;		//Font ascent height (px)
	const double descent;		//Font descent height (px)
	const double lineheight;		//Font lineheight (px)
	const uint8_t antialias_level;  // antialias level (0, 2 or 4) (up to 3bpp supported, lsbit ignored)
} FONT_INFO;

#endif