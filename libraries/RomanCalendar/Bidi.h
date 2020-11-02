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
	static int FindFirstSpacelikeCharacter(String s, int startpos);

	static bool IsSpace(String& ch, bool bAlsoMatchTagOpeningBracket = true);
	static bool IsSpace(String& ch, String& foundspacechar, bool bAlsoMatchTagOpeningBracket = true);
	static bool ExpectSpace(String&	s, int* pos);
		
	static bool ExpectRTL(String& s, int* pos);
	static bool ExpectStr(String& s, int* pos, String strtoexpect, bool bCaseSensitive = false);

	static bool ExpectLineBreakTag(String& s, int* pos);
	static bool ExpectEmphasisTag(String& s, int* pos, bool* Emphasis_On, bool* bLineBreak);
	static bool ExpectEmphasisTag(String& s, int pos);
	static bool ExpectEmphasisTag(String& s, int* pos);
	static bool ExpectEmphasisTagBefore(String& s, int* pos, int* skipped_count);

	static String strEllipsis;
	static void SetEllipsisText(String ellipsis_text);
	static void StripEmphasisTagsOnly(String& text);
	static void StripTags(String& text);

	static bool FixNextWordWiderThanDisplay(String& s, /*String& colormap,*/ int& charsprocessed, int& numcharslefttoprocess, bool& bIsLastFragment, bool& bMakeLineBreakBefore, int* forcedBreakPos, int startstrpos, int fbwidth, int xpos, bool render_right_to_left, DiskFont& diskfont);
	static void getColorMap(String& s, bool& bEmphasisOn, String& colormap); // used for overflow words, to pre-calculate the colourmap (before text is repositioned)
	
	static void GetString(String& s, int* startstrpos, int* endstrpos, int* textwidth, DiskFont& diskfont, bool* bLineBreak, bool* bRTL, bool* bDirectionChanged, bool* bNewLine, bool bOneWordOnly, bool bForceLineBreakAfterString, int forcedBreakPos, int fbwidth, int xpos, bool wrap_text, bool bLastLine, int ellipsiswidth, bool* bDisplayEllipsisNow, bool bMoreText);
	static bool RenderText(String& s, int* xpos, int* ypos, TextBuffer& tb, DiskFont& diskfont, bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left, bool wrap_text);
	static bool RenderText(String& s, int* xpos, int* ypos, TextBuffer& tb, DiskFont& diskfont, bool* bEmphasisOn, int fbwidth, int fbheight, bool render_right_to_left, bool wrap_text, bool bMoreText);
	
	// for Latin Mass
	static bool RenderTextEx(String& s, int* xpos, int* ypos, TextBuffer& tb, DiskFont* pDiskfont, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi, bool& bBold, bool& bItalic, bool& bRed, int8_t& fontsize_rel, int8_t& line_number, int fbwidth, int fbheight, bool& bRTL, bool render_right_to_left, bool wrap_text, bool bMoreText, int16_t& last_line_height, uint8_t format_action = TB_FORMAT_NONE);
	static bool RenderTextEx(String& s, int* xpos, int* ypos, TextBuffer& tb, DiskFont* pDiskfont, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi, bool& bBold, bool& bItalic, bool& bRed, int8_t& fontsize_rel, int8_t& line_number, int fbwidth, int fbheight, bool& bRTL, bool render_right_to_left, bool wrap_text, bool bMoreText, uint8_t format_action = TB_FORMAT_NONE);

	//static String tageffectstack;
	
	#define TAGEFFECT_STACKSIZE 16
	static uint8_t tageffectstack[TAGEFFECT_STACKSIZE];
	static int16_t tageffectstackindex;

	static DiskFont* SelectDiskFont(bool bBold, bool bItalic, int8_t fontsize_rel, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi);
	static bool ParseTags(String& s, int* pos, bool& bBold, bool& bItalic, bool& bRed, /*bool& bLineBreak,*/ int8_t& fontsize_rel);

	static void dump_style(bool bBold, bool bItalic, bool bRed, int8_t fontsize_rel);
	static void dump_stack();

	static void tageffect_reset(bool& bBold, bool& bItalic, bool& bRed, int8_t& fontsize_rel);
	static bool tageffect_push(bool bBold, bool bItalic, bool bRed, int8_t fontsize_rel);
	static bool tageffect_pop(bool& bBold, bool& bItalic, bool& bRed, int8_t& fontsize_rel);

	static void tageffect_init();

	static bool tageffect_push(uint8_t val);
	static bool tageffect_pop(uint8_t& val);

	static bool tageffect_peek(uint8_t& val);
	static bool tageffect_peek(uint8_t& val, int16_t index);
	static void GetString2(String& s, int* startstrpos, int* endstrpos, int* textwidth, DiskFont& diskfont, bool* bLineBreak, bool* bRTL, bool* bDirectionChanged, bool* bNewLine, bool* bTagFound, bool bOneWordOnly, bool bForceLineBreakAfterString, int forcedBreakPos, int fbwidth, int xpos, bool wrap_text, bool bLastLine, int ellipsiswidth, bool* bDisplayEllipsisNow, bool bMoreText);

#if (false) // debugging/testing
	static void TestHarnessLa(TextBuffer& tb, int fbwidth, int fbheight,  DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi);
	static void TestHarnessLa2(TextBuffer& tb, int fbwidth, int fbheight, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi);
	static bool getHeaderRecord(File& file, IndexHeader& ih);
	static bool getIndexRecord(File& file, IndexRecord& ir);
	static uint8_t getClassIndex(String& cls);
	static bool getText(File& file, IndexRecord& indexrecord, String& s, bool& bFileOk, bool& bMoreText);
	static void DumpIndexHeader(IndexHeader& ih);
	static void DumpIndexRecord(IndexRecord& ir);
#endif
};

#endif
