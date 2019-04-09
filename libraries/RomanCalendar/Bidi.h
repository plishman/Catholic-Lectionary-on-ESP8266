#ifndef _BIDI_H
#define _BIDI_H

#include "RCGlobals.h"

#include <Arduino.h>
#include <DiskFont.h>
//#include <epdpaint.h>
#include <I2CSerialPort.h>
#include <utf8string.h>
#include <ArabicLigaturizer.h>

#include <GxEPD.h>
#include <GxGDEW027C44/GxGDEW027C44.h>      // 7.5" b/w  640x384 GxGDEW075T8/GxGDEW075T8.cpp  // 2.7" color 176x264 GxGDEW027C44/GxGDEW027C44.cpp

#include <TextBuffer.h>

class Bidi {
public:
	//Bidi();
	static bool IsSpace(String ch);
	static bool ExpectRTL(String s, int* pos);
	static bool ExpectStr(String s, int* pos, String strtoexpect);

	static bool ExpectLineBreakTag(String s, int* pos);
	static bool ExpectEmphasisTag(String s, int* pos, bool* Emphasis_On, bool* bLineBreak);
	static bool ExpectEmphasisTag(String s, int pos);
	static bool ExpectEmphasisTag(String s, int* pos);
	static bool ExpectEmphasisTagBefore(String s, int* pos, int* skipped_count);

	static String strEllipsis;
	static void SetEllipsisText(String ellipsis_text);
	static String StripEmphasisTagsOnly(String text);
	static String StripTags(String text);

	static bool FixNextWordWiderThanDisplay(String &s, String& colormap, int& charsprocessed, int& numcharslefttoprocess, bool& bIsLastFragment, bool& bMakeLineBreakBefore, int startstrpos, int fbwidth, int xpos, bool render_right_to_left, DiskFont& diskfont);
	static void getColorMap(String word, bool& bEmphasisOn, String& colormap); // used for overflow words, to pre-calculate the colourmap (before text is repositioned)
	
	static void GetString(String s, int* startstrpos, int* endstrpos, int* textwidth, DiskFont& diskfont, bool* bLineBreak, bool* bRTL, bool* bDirectionChanged, bool* bNewLine, bool bOneWordOnly, bool bForceLineBreakAfterString, int fbwidth, int xpos, bool wrap_text);
	static bool RenderText(String s, int* xpos, int* ypos, TextBuffer& tb, DiskFont& diskfont, bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left, bool wrap_text);
	static bool RenderText(String s, int* xpos, int* ypos, TextBuffer& tb, DiskFont& diskfont, bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left, bool wrap_text, bool bMoreText);
};

#endif
