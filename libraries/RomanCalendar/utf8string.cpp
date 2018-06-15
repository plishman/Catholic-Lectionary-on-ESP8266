#include "stdafx.h"

#ifndef _WIN32
#include <utf8string.h>
#else
#include "utf8string.h"
#endif

int charLenBytesUTF8(char s) {
	byte ch = (byte)s;
	//Serial.println(String(ch)+ ";");

	byte b;

	b = (ch & 0xE0);  // 2-byte utf-8 characters start with 110xxxxx
	if (b == 0xC0) return 2;

	b = (ch & 0xF0);  // 3-byte utf-8 characters start with 1110xxxx
	if (b == 0xE0) return 3;

	b = (ch & 0xF8);  // 4-byte utf-8 characters start with 11110xxx
	if (b == 0xF0) return 4;

	b = (ch & 0xC0);  // bytes within multibyte utf-8 characters are 10xxxxxx
	if (b == 0x80) return 0; //somewhere in a multi-byte utf-8 character, so don't know the length. Return 0 so the scanner can keep looking

	return 1; // character must be 0x7F or below, so return 1 (it is an ascii character)
}

String utf8fromCodepoint(uint32_t c) {
	unsigned char char0;
	unsigned char char1;
	unsigned char char2;
	unsigned char char3;

	String ch;

	if (c < 0x80) {
		char0 = c & 0x7f;					// 1 byte
		char u[] = { char0, '\0' };
		return String(u);
	}

	if (c >= 0x80 && c < 0x800) {					// 2 bytes
		char1 = ((c & 0x3f) | 0x80);
		char0 = ((c & 0x7c0) >> 6 | 0xc0);
		char u[] = { char0, char1, '\0' };
		return String(u);
	}

	if (c >= 0x800 && c < 0x10000) {				// 3 bytes
		char2 = ((c & 0x3f) | 0x80);
		char1 = (((c & 0xfc0) >> 6) | 0x80);
		char0 = (((c & 0xf000)) >> 12 | 0xe0);
		char u[] = { char0, char1, char2, '\0' };
		return String(u);
	}

	if (c >= 0x10000 && c < 0x200000) {				// 4 bytes
		char3 = ((c & 0x3f) | 0x80);
		char2 = (((c & 0xfc0) >> 6) | 0x80);
		char1 = (((c & 0x3f000) >> 12) | 0x80);
		char0 = (((c & 0x1c0000)) >> 18 | 0xF0);
		char u[] = { char0, char1, char2, char3, '\0' };
		return String(u);
	}

	return String("");
}

uint32_t codepointUtf8(String c) {
	unsigned char b;
	int u = 0;

	const char* ch = c.c_str();

	int len = c.length();

	if (len == 0) return 0;

	b = (ch[0] & 0x80);
	if (b == 0 && len > 0) return (int)ch[0];	   // 1 byte character, u=0-0x7f

	b = (ch[0] & 0xE0);
	if (b == 0xC0 && len > 1) return (int)(((ch[0] & 0x1f) << 6) | (ch[1] & 0x3F)); // 2 byte character, u=0x80-0x7ff

	b = (ch[0] & 0xF0);
	if (b == 0xE0 && len > 2) return (int)(((ch[0] & 0x0f) << 12) | ((ch[1] & 0x3F) << 6) | (ch[2] & 0x3F)); // 3 byte character, u=0x800-0xFFFF

	b = (ch[0] & 0xF8);
	if (b == 0xF0 && len > 3) return (int)(((ch[0] & 0x07) << 18) | ((ch[1] & 0x3F) << 12) | ((ch[2] & 0x3F) << 6) | (ch[3] & 0x3F)); // 4 byte character, u=0x800-0xFFFF

	return 0;
}

String utf8CharAt(String s, unsigned int pos) {
	//Serial.println("String=" + s);

	if (pos >= s.length()) {
		//Serial.println("utf8CharAt string length is " + String(ps->length()) + " *ppos = " + String(*ppos));
		return String("");
	}

	int charLen = charLenBytesUTF8(s.charAt(pos));

	//Serial.println("char at pos " + String(*ppos) + " = " + String(ps->charAt(*ppos)) + "utf8 charLen = " + String(charLen));

	if (charLen == 0) {
		return String("");
	}
	else {
		//Serial.print("substring is" + s.substring(pos, pos+charLen));
		return s.substring(pos, pos + charLen);
	}
}

void SetUtf8CharByIndex(String& s, int index, String ch) {
	int charnum = 0;
	int strpos = 0;
	int charlen = charLenBytesUTF8(s.charAt(strpos));

	while (/*strpos < s.length() && */charnum < index) {
		if (strpos == s.length()) s += " "; // expand string to the position where the character is to be inserted

		strpos += charlen;
		charlen = charLenBytesUTF8(s.charAt(strpos));
		if (charlen == 0) charlen = 1; //skip over partial utf8 characters 1 byte at a time, count malformed chars as 1 utf8 char

		charnum++;
	}

	s = s.substring(0, strpos) + ch + s.substring(strpos + charlen);
}

String GetUtf8CharByIndex(String s, unsigned int index) {
	unsigned int charnum = 0;
	unsigned int strpos = 0;
	unsigned int charlen = charLenBytesUTF8(s.charAt(strpos));

	while (strpos < s.length() && charnum < index) {
		strpos += charlen;

		charlen = charLenBytesUTF8(s.charAt(strpos));
		if (charlen == 0) charlen = 1; // skip over partial or malformed utf8 chars 1 byte at a time, count as one utf-8 character

		charnum++;
	}

	return s.substring(strpos, strpos + charlen);
}

int Utf8CharCount(String s) {
	unsigned int utf8charcount = 0;
	unsigned int strpos = 0;
	unsigned int charlen = 0;

	while (strpos < s.length()) {
		charlen = charLenBytesUTF8(s.charAt(strpos));
		if (charlen == 0) charlen = 1; // skip over partial or malformed utf8 chars 1 byte at a time, count as one utf-8 character

		strpos += charlen;
		utf8charcount++;
	}

	return utf8charcount;
}

String Utf8substring(String s, unsigned int startutf8charindex, unsigned int endutf8charindex) {
	//	String res = "";
	//
	//	for(int i = startutf8charindex; i <= endutf8charindex; i++) {
	//		res += GetUtf8CharByIndex(s, i);
	//	}	
	//
	//	return res;	

	if (startutf8charindex > endutf8charindex) return "";

	unsigned int charnum = 0;
	unsigned int startstrpos = 0;
	unsigned int endstrpos = 0;
	unsigned int charlen = 0;

	// find strpos of first utf8 char
	while (startstrpos < s.length() && charnum < startutf8charindex) {
		charlen = charLenBytesUTF8(s.charAt(startstrpos));

		if (charlen == 0) charlen = 1; //count malformed or partial utf-8 chars as one utf8 character

		startstrpos += charlen;
		charnum++;
	}

	endstrpos = startstrpos;

	while (endstrpos < s.length() && charnum < endutf8charindex) {
		endstrpos += charLenBytesUTF8(s.charAt(endstrpos));
		charnum++;
	}

	return s.substring(startstrpos, endstrpos + charLenBytesUTF8(s.charAt(endstrpos)));
}

String Utf8ReverseString(String instr) {
	String outstr = "";
	String ch = "";
	String charblock = "";
	
	unsigned int charpos = 0;
	bool bLookingforchar = true;
	
	while (charpos < instr.length()) {
		if (bLookingforchar) {	// start by reading char from instr
			ch = utf8CharAt(instr, charpos);
		}
		
		if (charblock == "" || getBidiDirection(ch) == BIDI_NSM) {  // if have not started a block containing 1 normal char plus any number of NSM chars, add ch to charblock
			charblock+=ch;	// want all non-spacing marks (diacritics etc, which overprint other characters) to be in the same order as they are in the non-reversed string.
							// Eg. [ch1][ch2][nsm1][nsm2][ch3] becomes [ch3][ch2][nsm1][nsm2][ch1]
			charpos += ch.length();
			bLookingforchar = true; // next time round the while loop, the next character will be scanned from instr.
		} 					
		else {
			//Serial.printf("charblock = %s\n", charblock.c_str());
			// if in here, charblock has one or more utf8 characters in it, so add it to the *start* of outstr. By repeating this, the string will be reversed, but the charblocks won't be
			outstr = charblock + outstr;
			charblock = "";
			bLookingforchar = false; // if here, then a character has been scanned but not added to a new charblock, so preserve it next time round the loop (won't scan next char from instr)
									 // this path should happen when a second char in a row is scanned from instr that is not an NSM character
		}
	}

	outstr = charblock + outstr;	
	
	return outstr;
}

// https://stackoverflow.com/questions/4330951/how-to-detect-whether-a-character-belongs-to-a-right-to-left-language
bool IsRightToLeftChar(uint32_t c)
{
	//I2CSerial.printf("{%x}", c);

	uint8_t hasRandALCat = 0;
	if (c >= 0x5BE && c <= 0x10B7F)
	{
		if (c <= 0x85E)
		{
			if (c == 0x5BE) hasRandALCat = 1;
			else if (c == 0x5C0) hasRandALCat = 1;
			else if (c == 0x5C3) hasRandALCat = 1;
			else if (c == 0x5C6) hasRandALCat = 1;
			else if (0x5D0 <= c && c <= 0x5EA) hasRandALCat = 1;
			else if (0x5F0 <= c && c <= 0x5F4) hasRandALCat = 1;
			else if (c == 0x608) hasRandALCat = 1;
			else if (c == 0x60B) hasRandALCat = 1;
			else if (c == 0x60D) hasRandALCat = 1;
			else if (c == 0x61B) hasRandALCat = 1;
			else if (0x61E <= c && c <= 0x64A) hasRandALCat = 1;
			else if (0x66D <= c && c <= 0x66F) hasRandALCat = 1;
			else if (0x671 <= c && c <= 0x6D5) hasRandALCat = 1;
			else if (0x6E5 <= c && c <= 0x6E6) hasRandALCat = 1;
			else if (0x6EE <= c && c <= 0x6EF) hasRandALCat = 1;
			else if (0x6FA <= c && c <= 0x70D) hasRandALCat = 1;
			else if (c == 0x710) hasRandALCat = 1;
			else if (0x712 <= c && c <= 0x72F) hasRandALCat = 1;
			else if (0x74D <= c && c <= 0x7A5) hasRandALCat = 1;
			else if (c == 0x7B1) hasRandALCat = 1;
			else if (0x7C0 <= c && c <= 0x7EA) hasRandALCat = 1;
			else if (0x7F4 <= c && c <= 0x7F5) hasRandALCat = 1;
			else if (c == 0x7FA) hasRandALCat = 1;
			else if (0x800 <= c && c <= 0x815) hasRandALCat = 1;
			else if (c == 0x81A) hasRandALCat = 1;
			else if (c == 0x824) hasRandALCat = 1;
			else if (c == 0x828) hasRandALCat = 1;
			else if (0x830 <= c && c <= 0x83E) hasRandALCat = 1;
			else if (0x840 <= c && c <= 0x858) hasRandALCat = 1;
			else if (c == 0x85E) hasRandALCat = 1;
		}
		else if (c == 0x200F) hasRandALCat = 1;
		else if (c >= 0xFB1D)
		{
			if (c == 0xFB1D) hasRandALCat = 1;
			else if (0xFB1F <= c && c <= 0xFB28) hasRandALCat = 1;
			else if (0xFB2A <= c && c <= 0xFB36) hasRandALCat = 1;
			else if (0xFB38 <= c && c <= 0xFB3C) hasRandALCat = 1;
			else if (c == 0xFB3E) hasRandALCat = 1;
			else if (0xFB40 <= c && c <= 0xFB41) hasRandALCat = 1;
			else if (0xFB43 <= c && c <= 0xFB44) hasRandALCat = 1;
			else if (0xFB46 <= c && c <= 0xFBC1) hasRandALCat = 1;
			else if (0xFBD3 <= c && c <= 0xFD3D) hasRandALCat = 1;
			else if (0xFD50 <= c && c <= 0xFD8F) hasRandALCat = 1;
			else if (0xFD92 <= c && c <= 0xFDC7) hasRandALCat = 1;
			else if (0xFDF0 <= c && c <= 0xFDFC) hasRandALCat = 1;
			else if (0xFE70 <= c && c <= 0xFE74) hasRandALCat = 1;
			else if (0xFE76 <= c && c <= 0xFEFC) hasRandALCat = 1;
			else if (0x10800 <= c && c <= 0x10805) hasRandALCat = 1;
			else if (c == 0x10808) hasRandALCat = 1;
			else if (0x1080A <= c && c <= 0x10835) hasRandALCat = 1;
			else if (0x10837 <= c && c <= 0x10838) hasRandALCat = 1;
			else if (c == 0x1083C) hasRandALCat = 1;
			else if (0x1083F <= c && c <= 0x10855) hasRandALCat = 1;
			else if (0x10857 <= c && c <= 0x1085F) hasRandALCat = 1;
			else if (0x10900 <= c && c <= 0x1091B) hasRandALCat = 1;
			else if (0x10920 <= c && c <= 0x10939) hasRandALCat = 1;
			else if (c == 0x1093F) hasRandALCat = 1;
			else if (c == 0x10A00) hasRandALCat = 1;
			else if (0x10A10 <= c && c <= 0x10A13) hasRandALCat = 1;
			else if (0x10A15 <= c && c <= 0x10A17) hasRandALCat = 1;
			else if (0x10A19 <= c && c <= 0x10A33) hasRandALCat = 1;
			else if (0x10A40 <= c && c <= 0x10A47) hasRandALCat = 1;
			else if (0x10A50 <= c && c <= 0x10A58) hasRandALCat = 1;
			else if (0x10A60 <= c && c <= 0x10A7F) hasRandALCat = 1;
			else if (0x10B00 <= c && c <= 0x10B35) hasRandALCat = 1;
			else if (0x10B40 <= c && c <= 0x10B55) hasRandALCat = 1;
			else if (0x10B58 <= c && c <= 0x10B72) hasRandALCat = 1;
			else if (0x10B78 <= c && c <= 0x10B7F) hasRandALCat = 1;
		}
	}

	return hasRandALCat == 1 ? true : false;
}

bool IsRightToLeftChar(String ch) {
	uint32_t codepoint = codepointUtf8(ch);
	return IsRightToLeftChar(codepoint);
}

// The bidi types

/** Left-to-right*/
#define BIDI_L	0

/** Left-to-Right Embedding */
#define BIDI_LRE	1

/** Left-to-Right Override */
#define BIDI_LRO	2

/** Right-to-Left */
#define BIDI_R	3

/** Right-to-Left Arabic */
#define BIDI_AL	4

/** Right-to-Left Embedding */
#define BIDI_RLE	5

/** Right-to-Left Override */
#define BIDI_RLO	6

/** Pop Directional Format */
#define BIDI_PDF	7

/** European Number */
#define BIDI_EN	8

/** European Number Separator */
#define BIDI_ES	9

/** European Number Terminator */
#define BIDI_ET	10

/** Arabic Number */
#define BIDI_AN	11

/** Common Number Separator */
#define BIDI_CS	12

/** Non-Spacing Mark */
#define BIDI_NSM	13

/** Boundary Neutral */
#define BIDI_BN	14

/** Paragraph Separator */
#define BIDI_B	15

/** Segment Separator */
#define BIDI_S	16

/** Whitespace */
#define BIDI_WS	17

/** Other Neutrals */
#define BIDI_ON	18

/** Minimum bidi type value. */
#define BIDI_TYPE_MIN	0

/** Maximum bidi type value. */
#define BIDI_TYPE_MAX	18

#ifndef _WIN32
uint16_t baseTypes[1154] PROGMEM = {
#else
uint16_t baseTypes[1154] = {
#endif
	0,BIDI_BN,		9,BIDI_S,		10,BIDI_B,		11,BIDI_S,		12,BIDI_WS,		13,BIDI_B,
	14,BIDI_BN,		28,BIDI_B,		31,BIDI_S,		32,BIDI_WS,		33,BIDI_ON,		35,BIDI_ET,
	38,BIDI_ON,		43,BIDI_ET,		44,BIDI_CS,		45,BIDI_ET,		46,BIDI_CS,		47,BIDI_ES,
	48,BIDI_EN,		58,BIDI_CS,		59,BIDI_ON,		65,BIDI_L,		91,BIDI_ON,		97,BIDI_L,
	123,BIDI_ON,	127,BIDI_BN,	133,BIDI_B,		134,BIDI_BN,	160,BIDI_CS,
	161,BIDI_ON,	162,BIDI_ET,	166,BIDI_ON,	170,BIDI_L,		171,BIDI_ON,
	176,BIDI_ET,	178,BIDI_EN,	180,BIDI_ON,	181,BIDI_L,		182,BIDI_ON,
	185,BIDI_EN,	186,BIDI_L,		187,BIDI_ON,	192,BIDI_L,		215,BIDI_ON,
	216,BIDI_L,		247,BIDI_ON,	248,BIDI_L,		697,BIDI_ON,	699,BIDI_L,
	706,BIDI_ON,	720,BIDI_L,		722,BIDI_ON,	736,BIDI_L,		741,BIDI_ON,
	750,BIDI_L,		751,BIDI_ON,	768,BIDI_NSM,	856,BIDI_L,		861,BIDI_NSM,
	880,BIDI_L,		884,BIDI_ON,	886,BIDI_L,		894,BIDI_ON,	895,BIDI_L,
	900,BIDI_ON,	902,BIDI_L,		903,BIDI_ON,	904,BIDI_L,		1014,BIDI_ON,
	1015,BIDI_L,	1155,BIDI_NSM,	1159,BIDI_L,	1160,BIDI_NSM,
	1162,BIDI_L,	1418,BIDI_ON,	1419,BIDI_L,	1425,BIDI_NSM,
	1442,BIDI_L,	1443,BIDI_NSM,	1466,BIDI_L,	1467,BIDI_NSM,
	1470,BIDI_R,	1471,BIDI_NSM,	1472,BIDI_R,	1473,BIDI_NSM,
	1475,BIDI_R,	1476,BIDI_NSM,	1477,BIDI_L,	1488,BIDI_R,
	1515,BIDI_L,	1520,BIDI_R,	1525,BIDI_L,	1536,BIDI_AL,
	1540,BIDI_L,	1548,BIDI_CS,	1549,BIDI_AL,	1550,BIDI_ON,
	1552,BIDI_NSM,	1558,BIDI_L,	1563,BIDI_AL,	1564,BIDI_L,
	1567,BIDI_AL,	1568,BIDI_L,	1569,BIDI_AL,	1595,BIDI_L,
	1600,BIDI_AL,	1611,BIDI_NSM,	1625,BIDI_L,	1632,BIDI_AN,
	1642,BIDI_ET,	1643,BIDI_AN,	1645,BIDI_AL,	1648,BIDI_NSM,
	1649,BIDI_AL,	1750,BIDI_NSM,	1757,BIDI_AL,	1758,BIDI_NSM,
	1765,BIDI_AL,	1767,BIDI_NSM,	1769,BIDI_ON,	1770,BIDI_NSM,
	1774,BIDI_AL,	1776,BIDI_EN,	1786,BIDI_AL,	1806,BIDI_L,
	1807,BIDI_BN,	1808,BIDI_AL,	1809,BIDI_NSM,	1810,BIDI_AL,
	1840,BIDI_NSM,	1867,BIDI_L,	1869,BIDI_AL,	1872,BIDI_L,
	1920,BIDI_AL,	1958,BIDI_NSM,	1969,BIDI_AL,	1970,BIDI_L,
	2305,BIDI_NSM,	2307,BIDI_L,	2364,BIDI_NSM,	2365,BIDI_L,
	2369,BIDI_NSM,	2377,BIDI_L,	2381,BIDI_NSM,	2382,BIDI_L,
	2385,BIDI_NSM,	2389,BIDI_L,	2402,BIDI_NSM,	2404,BIDI_L,
	2433,BIDI_NSM,	2434,BIDI_L,	2492,BIDI_NSM,	2493,BIDI_L,
	2497,BIDI_NSM,	2501,BIDI_L,	2509,BIDI_NSM,	2510,BIDI_L,
	2530,BIDI_NSM,	2532,BIDI_L,	2546,BIDI_ET,	2548,BIDI_L,
	2561,BIDI_NSM,	2563,BIDI_L,	2620,BIDI_NSM,	2621,BIDI_L,
	2625,BIDI_NSM,	2627,BIDI_L,	2631,BIDI_NSM,	2633,BIDI_L,
	2635,BIDI_NSM,	2638,BIDI_L,	2672,BIDI_NSM,	2674,BIDI_L,
	2689,BIDI_NSM,	2691,BIDI_L,	2748,BIDI_NSM,	2749,BIDI_L,
	2753,BIDI_NSM,	2758,BIDI_L,	2759,BIDI_NSM,	2761,BIDI_L,
	2765,BIDI_NSM,	2766,BIDI_L,	2786,BIDI_NSM,	2788,BIDI_L,
	2801,BIDI_ET,	2802,BIDI_L,	2817,BIDI_NSM,	2818,BIDI_L,
	2876,BIDI_NSM,	2877,BIDI_L,	2879,BIDI_NSM,	2880,BIDI_L,
	2881,BIDI_NSM,	2884,BIDI_L,	2893,BIDI_NSM,	2894,BIDI_L,
	2902,BIDI_NSM,	2903,BIDI_L,	2946,BIDI_NSM,	2947,BIDI_L,
	3008,BIDI_NSM,	3009,BIDI_L,	3021,BIDI_NSM,	3022,BIDI_L,
	3059,BIDI_ON,	3065,BIDI_ET,	3066,BIDI_ON,	3067,BIDI_L,
	3134,BIDI_NSM,	3137,BIDI_L,	3142,BIDI_NSM,	3145,BIDI_L,
	3146,BIDI_NSM,	3150,BIDI_L,	3157,BIDI_NSM,	3159,BIDI_L,
	3260,BIDI_NSM,	3261,BIDI_L,	3276,BIDI_NSM,	3278,BIDI_L,
	3393,BIDI_NSM,	3396,BIDI_L,	3405,BIDI_NSM,	3406,BIDI_L,
	3530,BIDI_NSM,	3531,BIDI_L,	3538,BIDI_NSM,	3541,BIDI_L,
	3542,BIDI_NSM,	3543,BIDI_L,	3633,BIDI_NSM,	3634,BIDI_L,
	3636,BIDI_NSM,	3643,BIDI_L,	3647,BIDI_ET,	3648,BIDI_L,
	3655,BIDI_NSM,	3663,BIDI_L,	3761,BIDI_NSM,	3762,BIDI_L,
	3764,BIDI_NSM,	3770,BIDI_L,	3771,BIDI_NSM,	3773,BIDI_L,
	3784,BIDI_NSM,	3790,BIDI_L,	3864,BIDI_NSM,	3866,BIDI_L,
	3893,BIDI_NSM,	3894,BIDI_L,	3895,BIDI_NSM,	3896,BIDI_L,
	3897,BIDI_NSM,	3898,BIDI_ON,	3902,BIDI_L,	3953,BIDI_NSM,
	3967,BIDI_L,	3968,BIDI_NSM,	3973,BIDI_L,	3974,BIDI_NSM,
	3976,BIDI_L,	3984,BIDI_NSM,	3992,BIDI_L,	3993,BIDI_NSM,
	4029,BIDI_L,	4038,BIDI_NSM,	4039,BIDI_L,	4141,BIDI_NSM,
	4145,BIDI_L,	4146,BIDI_NSM,	4147,BIDI_L,	4150,BIDI_NSM,
	4152,BIDI_L,	4153,BIDI_NSM,	4154,BIDI_L,	4184,BIDI_NSM,
	4186,BIDI_L,	5760,BIDI_WS,	5761,BIDI_L,	5787,BIDI_ON,
	5789,BIDI_L,	5906,BIDI_NSM,	5909,BIDI_L,	5938,BIDI_NSM,
	5941,BIDI_L,	5970,BIDI_NSM,	5972,BIDI_L,	6002,BIDI_NSM,
	6004,BIDI_L,	6071,BIDI_NSM,	6078,BIDI_L,	6086,BIDI_NSM,
	6087,BIDI_L,	6089,BIDI_NSM,	6100,BIDI_L,	6107,BIDI_ET,
	6108,BIDI_L,	6109,BIDI_NSM,	6110,BIDI_L,	6128,BIDI_ON,
	6138,BIDI_L,	6144,BIDI_ON,	6155,BIDI_NSM,	6158,BIDI_WS,
	6159,BIDI_L,	6313,BIDI_NSM,	6314,BIDI_L,	6432,BIDI_NSM,
	6435,BIDI_L,	6439,BIDI_NSM,	6444,BIDI_L,	6450,BIDI_NSM,
	6451,BIDI_L,	6457,BIDI_NSM,	6460,BIDI_L,	6464,BIDI_ON,
	6465,BIDI_L,	6468,BIDI_ON,	6470,BIDI_L,	6624,BIDI_ON,
	6656,BIDI_L,	8125,BIDI_ON,	8126,BIDI_L,	8127,BIDI_ON,
	8130,BIDI_L,	8141,BIDI_ON,	8144,BIDI_L,	8157,BIDI_ON,
	8160,BIDI_L,	8173,BIDI_ON,	8176,BIDI_L,	8189,BIDI_ON,
	8191,BIDI_L,	8192,BIDI_WS,	8203,BIDI_BN,	8206,BIDI_L,
	8207,BIDI_R,	8208,BIDI_ON,	8232,BIDI_WS,	8233,BIDI_B,
	8234,BIDI_LRE,	8235,BIDI_RLE,	8236,BIDI_PDF,	8237,BIDI_LRO,
	8238,BIDI_RLO,	8239,BIDI_WS,	8240,BIDI_ET,	8245,BIDI_ON,
	8277,BIDI_L,	8279,BIDI_ON,	8280,BIDI_L,	8287,BIDI_WS,
	8288,BIDI_BN,	8292,BIDI_L,	8298,BIDI_BN,	8304,BIDI_EN,
	8305,BIDI_L,	8308,BIDI_EN,	8314,BIDI_ET,	8316,BIDI_ON,
	8319,BIDI_L,	8320,BIDI_EN,	8330,BIDI_ET,	8332,BIDI_ON,
	8335,BIDI_L,	8352,BIDI_ET,	8370,BIDI_L,	8400,BIDI_NSM,
	8427,BIDI_L,	8448,BIDI_ON,	8450,BIDI_L,	8451,BIDI_ON,
	8455,BIDI_L,	8456,BIDI_ON,	8458,BIDI_L,	8468,BIDI_ON,
	8469,BIDI_L,	8470,BIDI_ON,	8473,BIDI_L,	8478,BIDI_ON,
	8484,BIDI_L,	8485,BIDI_ON,	8486,BIDI_L,	8487,BIDI_ON,
	8488,BIDI_L,	8489,BIDI_ON,	8490,BIDI_L,	8494,BIDI_ET,
	8495,BIDI_L,	8498,BIDI_ON,	8499,BIDI_L,	8506,BIDI_ON,
	8508,BIDI_L,	8512,BIDI_ON,	8517,BIDI_L,	8522,BIDI_ON,
	8524,BIDI_L,	8531,BIDI_ON,	8544,BIDI_L,	8592,BIDI_ON,
	8722,BIDI_ET,	8724,BIDI_ON,	9014,BIDI_L,	9083,BIDI_ON,
	9109,BIDI_L,	9110,BIDI_ON,	9169,BIDI_L,	9216,BIDI_ON,
	9255,BIDI_L,	9280,BIDI_ON,	9291,BIDI_L,	9312,BIDI_EN,
	9372,BIDI_L,	9450,BIDI_EN,	9451,BIDI_ON,	9752,BIDI_L,
	9753,BIDI_ON,	9854,BIDI_L,	9856,BIDI_ON,	9874,BIDI_L,
	9888,BIDI_ON,	9890,BIDI_L,	9985,BIDI_ON,	9989,BIDI_L,
	9990,BIDI_ON,	9994,BIDI_L,	9996,BIDI_ON,	10024,BIDI_L,
	10025,BIDI_ON,	10060,BIDI_L,	10061,BIDI_ON,	10062,BIDI_L,
	10063,BIDI_ON,	10067,BIDI_L,	10070,BIDI_ON,	10071,BIDI_L,
	10072,BIDI_ON,	10079,BIDI_L,	10081,BIDI_ON,	10133,BIDI_L,
	10136,BIDI_ON,	10160,BIDI_L,	10161,BIDI_ON,	10175,BIDI_L,
	10192,BIDI_ON,	10220,BIDI_L,	10224,BIDI_ON,	11022,BIDI_L,
	11904,BIDI_ON,	11930,BIDI_L,	11931,BIDI_ON,	12020,BIDI_L,
	12032,BIDI_ON,	12246,BIDI_L,	12272,BIDI_ON,	12284,BIDI_L,
	12288,BIDI_WS,	12289,BIDI_ON,	12293,BIDI_L,	12296,BIDI_ON,
	12321,BIDI_L,	12330,BIDI_NSM,	12336,BIDI_ON,	12337,BIDI_L,
	12342,BIDI_ON,	12344,BIDI_L,	12349,BIDI_ON,	12352,BIDI_L,
	12441,BIDI_NSM,	12443,BIDI_ON,	12445,BIDI_L,	12448,BIDI_ON,
	12449,BIDI_L,	12539,BIDI_ON,	12540,BIDI_L,	12829,BIDI_ON,
	12831,BIDI_L,	12880,BIDI_ON,	12896,BIDI_L,	12924,BIDI_ON,
	12926,BIDI_L,	12977,BIDI_ON,	12992,BIDI_L,	13004,BIDI_ON,
	13008,BIDI_L,	13175,BIDI_ON,	13179,BIDI_L,	13278,BIDI_ON,
	13280,BIDI_L,	13311,BIDI_ON,	13312,BIDI_L,	19904,BIDI_ON,
	19968,BIDI_L,	42128,BIDI_ON,	42183,BIDI_L,	64285,BIDI_R,
	64286,BIDI_NSM,	64287,BIDI_R,	64297,BIDI_ET,	64298,BIDI_R,
	64311,BIDI_L,	64312,BIDI_R,	64317,BIDI_L,	64318,BIDI_R,
	64319,BIDI_L,	64320,BIDI_R,	64322,BIDI_L,	64323,BIDI_R,
	64325,BIDI_L,	64326,BIDI_R,	64336,BIDI_AL,	64434,BIDI_L,
	64467,BIDI_AL,	64830,BIDI_ON,	64832,BIDI_L,	64848,BIDI_AL,
	64912,BIDI_L,	64914,BIDI_AL,	64968,BIDI_L,	65008,BIDI_AL,
	65021,BIDI_ON,	65022,BIDI_L,	65024,BIDI_NSM,	65040,BIDI_L,
	65056,BIDI_NSM,	65060,BIDI_L,	65072,BIDI_ON,	65104,BIDI_CS,
	65105,BIDI_ON,	65106,BIDI_CS,	65107,BIDI_L,	65108,BIDI_ON,
	65109,BIDI_CS,	65110,BIDI_ON,	65119,BIDI_ET,	65120,BIDI_ON,
	65122,BIDI_ET,	65124,BIDI_ON,	65127,BIDI_L,	65128,BIDI_ON,
	65129,BIDI_ET,	65131,BIDI_ON,	65132,BIDI_L,	65136,BIDI_AL,
	65141,BIDI_L,	65142,BIDI_AL,	65277,BIDI_L,	65279,BIDI_BN,
	65280,BIDI_L,	65281,BIDI_ON,	65283,BIDI_ET,	65286,BIDI_ON,
	65291,BIDI_ET,	65292,BIDI_CS,	65293,BIDI_ET,	65294,BIDI_CS,
	65295,BIDI_ES,	65296,BIDI_EN,	65306,BIDI_CS,	65307,BIDI_ON,
	65313,BIDI_L,	65339,BIDI_ON,	65345,BIDI_L,	65371,BIDI_ON,
	65382,BIDI_L,	65504,BIDI_ET,	65506,BIDI_ON,	65509,BIDI_ET,
	65511,BIDI_L,	65512,BIDI_ON,	65519,BIDI_L,	65529,BIDI_BN,
	65532,BIDI_ON,	65534,BIDI_L,	65535,BIDI_L,	65535,BIDI_L
};

byte getBidiDirection(uint16_t uch) {
	const int biditable_length = 576;
	const int biditable_maxindex = biditable_length - 1;

	uint16_t base_start = 0;
	uint16_t base_end = 0;

	//return rtypes[c];	
	int l, r, m;
	l = 0;
	r = biditable_maxindex;
	while (l <= r) {	// binary chop search
		m = (l + r) / 2;

#ifndef _WIN32
		base_start = pgm_read_word(&baseTypes[m * 2]);
		base_end = pgm_read_word(&baseTypes[(m + 1) * 2]);
#else 
		base_start = baseTypes[m * 2];
		base_end = baseTypes[(m + 1) * 2];
#endif

		if (uch >= base_start && uch < base_end) {
#ifndef _WIN32
			return (byte)(pgm_read_word(&baseTypes[(m * 2) + 1]));
#else
			return (byte)baseTypes[(m * 2) + 1];
#endif
		}
		else if (uch < base_start) {
			r = m - 1;
		}
		else {
			l = m + 1;
		}
	}

	printf("\nDid't find entry, l=%d, r=%d, m=%d\t", l, r, m);

	return 0;	// error, shouldn't happen
}

byte getBidiDirection(String ch) {
	return getBidiDirection((uint16_t)codepointUtf8(ch));
}
