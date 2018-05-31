#ifndef _BIDI_H
#define _BIDI_H

#include <Arduino.h>
#include <DiskFont.h>
#include <epdpaint.h>

class Bidi {
public:
	int _fbwidth;
	int _fbheight;
	
	Bidi(int fbwidth, int fbheight);
	
	bool ExpectRTL(String s, int* pos);
	bool ExpectStr(String s, int* pos, String strtoexpect);
	bool ExpectEmphasisTag(String s, int* pos, bool* Emphasis_On);
	bool ExpectEmphasisTag(String s, int pos);
	
	void GetString(String s, int* startstrpos, int* endstrpos, int* textwidth, DiskFont* diskfont, bool* bEmphasis_On, bool* bRTL,int fbwidth, int xpos);
	
	bool IsRightToLeftChar(String ch);
	bool IsRightToLeftChar(uint32_t c);
	
	uint32_t codepointUtf8(String c);
	String utf8fromCodepoint(uint32_t c);
	String utf8CharAt(String s, int pos);
	int charLenBytesUTF8(char s);	
};

#endif
