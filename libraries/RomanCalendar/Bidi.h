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
	//Bidi();
	
	static bool ExpectRTL(String s, int* pos);
	static bool ExpectStr(String s, int* pos, String strtoexpect);
	static bool ExpectEmphasisTag(String s, int* pos, bool* Emphasis_On, bool* bLineBreak);
	static bool ExpectEmphasisTag(String s, int pos);
	
	static void GetString(String s, int* startstrpos, int* endstrpos, int* textwidth, DiskFont* diskfont,            bool* bEmphasis_On, bool* bLineBreak, bool* bRTL, bool* bDirectionChanged, bool* bNewLine, int fbwidth, int xpos, bool wrap_text);
	static bool RenderText(String s, int* xpos, int* ypos, Paint* paint_black, Paint* paint_red, DiskFont* diskfont, bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left, bool wrap_text);
};

#endif
