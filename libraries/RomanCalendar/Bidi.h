#ifndef _BIDI_H
#define _BIDI_H

#include <Arduino.h>
#include <DiskFont.h>
#include <epdpaint.h>
#include <I2CSerialPort.h>
#include <utf8string.h>
#include <ArabicLigaturizer.h>

class Bidi {
public:
	Bidi();
	
	bool ExpectRTL(String s, int* pos);
	bool ExpectStr(String s, int* pos, String strtoexpect);
	bool ExpectEmphasisTag(String s, int* pos, bool* Emphasis_On, bool* bLineBreak);
	bool ExpectEmphasisTag(String s, int pos);
	
	void GetString(String s, int* startstrpos, int* endstrpos, int* textwidth, Paint* paint, FONT_INFO* font, bool* bEmphasis_On, bool* bLineBreak, bool* bRTL, bool* bDirectionChanged, bool* bNewLine, int fbwidth, int xpos);
	void GetString(String s, int* startstrpos, int* endstrpos, int* textwidth, DiskFont* diskfont,            bool* bEmphasis_On, bool* bLineBreak, bool* bRTL, bool* bDirectionChanged, bool* bNewLine, int fbwidth, int xpos);

	bool RenderText(String s, int* xpos, int* ypos, Paint* paint_black, Paint* paint_red, FONT_INFO* font,    bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left);
	bool RenderText(String s, int* xpos, int* ypos, Paint* paint_black, Paint* paint_red, DiskFont* diskfont, bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left);

/*
	bool IsRightToLeftChar(String ch);
	bool IsRightToLeftChar(uint32_t c);

	uint32_t codepointUtf8(String c);
	String utf8fromCodepoint(uint32_t c);
	String utf8CharAt(String s, int pos);
	int charLenBytesUTF8(char s);	
*/
};

#endif
