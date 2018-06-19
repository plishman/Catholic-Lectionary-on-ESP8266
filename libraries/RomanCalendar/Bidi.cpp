#include "Bidi.h"	
	
Bidi::Bidi() {
}
	
bool Bidi::ExpectRTL(String s, int* pos) {
	String ch = utf8CharAt(s, *pos);
	
	if (ch == "") return false;
	
	uint32_t codepoint = codepointUtf8(ch);
	
	if (IsRightToLeftChar(codepoint)) {
		*pos += ch.length();
		return true;
	}
	
	return false;
}	

bool Bidi::ExpectStr(String s, int* pos, String strtoexpect) {
	if (s.indexOf(strtoexpect, *pos) == *pos) {
		*pos += strtoexpect.length();
		return true;
	}
	return false;
}

bool Bidi::ExpectEmphasisTag(String s, int* pos, bool* bEmphasis_On, bool* bLineBreak) {
// return true and advance the string index *pos when either <i> <b> </i> or </b> tags are found. *pos will be advanced past the last closing > in the tag
// *Emphasis_On will be left unchanged if no emphasis tag is found at the string starting in position *pos
// In this way, Emphasis_On can persist when the closing tag is found in a later call to the function.
	//I2CSerial.printf("ExpectEmphasisTag: str at pos=%s\n", s.substring(*pos, *pos + 5).c_str());

	if (s.indexOf("<i>",  *pos) == *pos) { *bEmphasis_On = true;  ExpectStr(s, pos, "<i>");  return true; } // ExpectStr will advance pos to the end of the tag 
	if (s.indexOf("<b>",  *pos) == *pos) { *bEmphasis_On = true;  ExpectStr(s, pos, "<b>");  return true; } // in string s
	if (s.indexOf("</i>", *pos) == *pos) { *bEmphasis_On = false; ExpectStr(s, pos, "</i>"); return true; } 
	if (s.indexOf("</b>", *pos) == *pos) { *bEmphasis_On = false; ExpectStr(s, pos, "</b>"); return true; }
	if (s.indexOf("<br>", *pos) == *pos) { *bLineBreak   = true;  ExpectStr(s, pos, "<br>"); return true; }
	if (s.indexOf("<br/>",*pos) == *pos) { *bLineBreak   = true;  ExpectStr(s, pos, "<br/>");return true; }
	
	return false;
}

bool Bidi::ExpectEmphasisTag(String s, int pos) {
// nondestructive test for an emphasis tag - does not advance the string index, or change the state of the bEmphasisOn flag

	int p = pos;
	bool e = false;
	bool b = false;
	
	return ExpectEmphasisTag(s, &p, &e, &b);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TODO: fix getstring so that a word without a space that is longer than the width of the display gets hyphenated, rather than crashing the renderer
//

// GetString for internal font
void Bidi::GetString(String s, 
					 int* startstrpos, 
					 int* endstrpos, 
					 int* textwidth, 
					 Paint* paint,
					 FONT_INFO* font,
					 bool* bEmphasis_On, // this variable stores the emphasis state, bold/italic (which will be shown in red text). It will remain unchanged if no emphasis tag <i>, </i>, <b>, </b> is found in the text
					 bool* bLineBreak,	 // this variable is set when either a <br> or <br/> tag is encountered, to tell the caller to skip a line before resuming text output
					 bool* bRTL,		 // this variable allows the state of the text block just scanned, right-to-left or left-to-right to be returned 
					 bool* bDirectionChanged, // this variable shows when the reading direction at the end of the text block just scanned has changed direction
					 bool* bNewLine,
					 int fbwidth, 
					 int xpos) 
{
	int pos = *startstrpos; // pos stores the postion in the string of the utf8 character currently being scanned
	
	int lastwordendstrpos = *startstrpos; // this variable stores the position of the last space (text word boundary) found 
	int lastwordendxwidth = 0; // this variable stores the width of the scanned string in pixels up to the last text word boundary 
	
	int maxwidth = fbwidth - xpos;
	
	int currwidth = 0;

	*bNewLine = false;
	
	// check if an emphasis tag is present in the string starting at pos 
	if (ExpectEmphasisTag(s, startstrpos, bEmphasis_On, bLineBreak)) { // if so, skip the tag by changing the _start_ pos of the string to the first character after 
		I2CSerial.printf("found tag\n");
		
		*endstrpos = *startstrpos; // the tag, and set the end pos of the scanned region to be the as the new start position (after the tag!).
		*textwidth = 0;			   // the textwidth of the scanned text must be 0, since no more text has been scanned, and the tag is skipped.
								   // The emphasis state will be changed or left unchanged by the call to ExpectEmphasisTag.
		return;
	}
	
	// scan string s up to the point where either 
	// i)   the string index pos reaches the end of the string
	// ii)  the width of the string in pixels exceeds the remaining space on the line
	// iii) an opening or closing emphasis tag <i>, </i>, <b>, or </b> is found
	// iv)  the string changes from left to right to right to left reading, or vice versa
	
	String ch = utf8CharAt(s, pos); // get character
	bool bLookingForRightToLeft = *bRTL; //IsRightToLeftChar(ch); // inherit rtl or ltr reading direction from previous call, unless previous call showed that the direction changed	
	
	if (*bDirectionChanged) {
		bLookingForRightToLeft = IsRightToLeftChar(ch);
		*bRTL = bLookingForRightToLeft;
		*bDirectionChanged = false;
	}
		
	bool bCurrCharRightToLeft = bLookingForRightToLeft;
	bool bEmphasisTagFound = false;
	
	// now determine the length of the right to left or left to right string from the start character ch found at position pos
	while (pos < s.length() && currwidth < maxwidth && !bEmphasisTagFound && bCurrCharRightToLeft == bLookingForRightToLeft) {
		bEmphasisTagFound = ExpectEmphasisTag(s, pos); // stop when an emphasis or line break tag is found, and break out of the while loop without changing pos
		if (bEmphasisTagFound) {
			I2CSerial.printf("found tag-\n");
			lastwordendstrpos = pos; // save this position as if it is a word boundary
			lastwordendxwidth = currwidth; // and save the width in pixels of the string scanned up to position pos
			continue; // The tag will be dealt with on the next call to this function.
		}

		ch = utf8CharAt(s, pos); // get next character

		if (ch == " " || ch == ".") { // space char is special case, inherits the reading direction of the character preceding it
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = currwidth; // and save the width in pixels of the string scanned up to position pos
		} 
		else {
			bCurrCharRightToLeft = IsRightToLeftChar(ch);
			//I2CSerial.printf("%s", bCurrCharRightToLeft ? "<" : ">");
		}

		I2CSerial.printf("%s", bCurrCharRightToLeft ? "<" : ">");
		if (bCurrCharRightToLeft != bLookingForRightToLeft) {
			*bDirectionChanged = true; //report that the direction of the next block to be scanned (on the next call) will be reversed
			continue; // don't bump string index pos if the reading direction of the text has just changed, so that the first character of the reversed text is not skipped
		}
		
		I2CSerial.printf("%s", ch.c_str());
		currwidth += paint->GetTextWidth(ch, font);
		pos += ch.length();
	}

	//if (bEmphasisTagFound) I2CSerial.printf("Emphasis tag found\n");
	//if (pos >= s.length()) I2CSerial.printf("pos >= s.length()\n");
	//if (currwidth >= maxwidth) I2CSerial.printf("currwidth >= maxwidth\n");
	//if (bCurrCharRightToLeft != bLookingForRightToLeft) I2CSerial.printf("bCurrCharRightToLeft != bLookingForRightToLeft\n");

	if (pos >= s.length() || bCurrCharRightToLeft != bLookingForRightToLeft) {
		lastwordendstrpos = pos; // save this position as it is a word boundary
		lastwordendxwidth = currwidth; // and save the width in pixels of the string scanned up to position pos		
	}
	
	*endstrpos = lastwordendstrpos;	// make the end pos the index of the last character of the last whole word scanned that fitted on the line
	*textwidth = lastwordendxwidth;	// make the width of the text to be rendered the width of the whole line up to and including the last whole word scanned which fitted on the line

	if (currwidth >= maxwidth) { // if the next word after lastwordendstrpos overflowed the line, tell the caller to generate a cr/newline (after rendering the text between
		*bNewLine = true;		 // *startstrpos and *endstrpos).
	}
	
	I2CSerial.printf("\n");
}


// GetString for disk font
void Bidi::GetString(String s, 
					 int* startstrpos, 
					 int* endstrpos, 
					 int* textwidth, 
					 DiskFont* diskfont, 
					 bool* bEmphasis_On, // this variable stores the emphasis state, bold/italic (which will be shown in red text). It will remain unchanged if no emphasis tag <i>, </i>, <b>, </b> is found in the text
					 bool* bLineBreak,	 // this variable is set when either a <br> or <br/> tag is encountered, to tell the caller to skip a line before resuming text output
					 bool* bRTL,		 // this variable allows the state of the text block just scanned, right-to-left or left-to-right to be returned 
					 bool* bDirectionChanged, // this variable shows when the reading direction at the end of the text block just scanned has changed direction
					 bool* bNewLine,
					 int fbwidth, 
					 int xpos) 
{
	int pos = *startstrpos; // pos stores the postion in the string of the utf8 character currently being scanned
	
	int lastwordendstrpos = *startstrpos; // this variable stores the position of the last space (text word boundary) found 
	int lastwordendxwidth = 0; // this variable stores the width of the scanned string in pixels up to the last text word boundary 
	
	//int maxwidth = fbwidth - xpos;
	
	//int currwidth = 0;
	double dcurrwidth = 0.0;
	double dmaxwidth = (double)(fbwidth - xpos);

	*bNewLine = false;
	
	// check if an emphasis tag is present in the string starting at pos 
	if (ExpectEmphasisTag(s, startstrpos, bEmphasis_On, bLineBreak)) { // if so, skip the tag by changing the _start_ pos of the string to the first character after 
		I2CSerial.printf("found tag\n");

		*endstrpos = *startstrpos; // the tag, and set the end pos of the scanned region to be the as the new start position (after the tag!).
		*textwidth = 0;			   // the textwidth of the scanned text must be 0, since no more text has been scanned, and the tag is skipped.
								   // The emphasis state will be changed or left unchanged by the call to ExpectEmphasisTag.
		return;
	}
	
	// scan string s up to the point where either 
	// i)   the string index pos reaches the end of the string
	// ii)  the width of the string in pixels exceeds the remaining space on the line
	// iii) an opening or closing emphasis tag <i>, </i>, <b>, or </b> is found
	// iv)  the string changes from left to right to right to left reading, or vice versa
	
	String ch = utf8CharAt(s, pos); // get character
	bool bLookingForRightToLeft = *bRTL; //IsRightToLeftChar(ch); // inherit rtl or ltr reading direction from previous call, unless previous call showed that the direction changed	
	
	if (*bDirectionChanged) {
		bLookingForRightToLeft = IsRightToLeftChar(ch);
		*bRTL = bLookingForRightToLeft;
		*bDirectionChanged = false;
	}	
	
	bool bCurrCharRightToLeft = bLookingForRightToLeft;

	bool bEmphasisTagFound = false;
	
	// now determine the length of the right to left or left to right string from the start character ch found at position pos
	while (pos < s.length() && dcurrwidth < dmaxwidth && !bEmphasisTagFound && bCurrCharRightToLeft == bLookingForRightToLeft) {				
		bEmphasisTagFound = ExpectEmphasisTag(s, pos); // stop when an emphasis or line break tag is found, and break out of the while loop without changing pos
		if (bEmphasisTagFound) {
			I2CSerial.printf("found tag-\n");
			lastwordendstrpos = pos; // save this position as if it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos
			continue; // The tag will be dealt with on the next call to this function.
		}

		ch = utf8CharAt(s, pos); // get next character
			
		if (ch == " " || ch == ".") { // space char is special case, inherits the reading direction of the character preceding it. Also period (.), which sometimes (it appears) is used in RTL text
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos
			//I2CSerial.printf("*");
		} 
		else {
			bCurrCharRightToLeft = IsRightToLeftChar(ch);
			//I2CSerial.printf("%s", bCurrCharRightToLeft ? "<" : ">");
		}		

		I2CSerial.printf("%s", bCurrCharRightToLeft ? "<" : ">");
		if (bCurrCharRightToLeft != bLookingForRightToLeft) {
			*bDirectionChanged = true; //report that the direction of the next block to be scanned (on the next call) will be reversed
			continue; // don't bump string index pos if the reading direction of the text has just changed, so that the first character of the reversed text is not skipped
		}
		
		I2CSerial.printf("%s", ch.c_str());		
		//currwidth += diskfont->GetTextWidth(ch);
		
		dcurrwidth += diskfont->GetTextWidthA(ch);
		
		pos += ch.length();
	}

	//if (bEmphasisTagFound) I2CSerial.printf("Emphasis tag found\n");
	//if (pos >= s.length()) I2CSerial.printf("pos >= s.length()\n");
	//if (currwidth >= maxwidth) I2CSerial.printf("currwidth >= maxwidth\n");
	//if (bCurrCharRightToLeft != bLookingForRightToLeft) I2CSerial.printf("bCurrCharRightToLeft != bLookingForRightToLeft\n");
	
	if (pos >= s.length() || bCurrCharRightToLeft != bLookingForRightToLeft) {
		lastwordendstrpos = pos; // save this position as it is a word boundary
		lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos		
	}
	
	*endstrpos = lastwordendstrpos;	// make the end pos the index of the last character of the last whole word scanned that fitted on the line
	*textwidth = lastwordendxwidth;	// make the width of the text to be rendered the width of the whole line up to and including the last whole word scanned which fitted on the line

	if (dcurrwidth >= dmaxwidth) { // if the next word after lastwordendstrpos overflowed the line, tell the caller to generate a cr/newline (after rendering the text between
		*bNewLine = true;		 // *startstrpos and *endstrpos).
	}

	I2CSerial.printf("\n");
}


// render bidi text using the internal (rom) font
bool Bidi::RenderText(String s, 
					  int* xpos, int* ypos, 
					  Paint* paint_black, Paint* paint_red, 
					  FONT_INFO* font, 
					  bool* bEmphasisOn,
					  int fbwidth, int fbheight,
					  bool render_right_to_left) 
{
	bool bNewLine = false;
	int startstrpos = 0;
	int endstrpos = 0;
	int textwidth = 0;
	bool bLineBreak = false;

	String alshapedtext = "";
	int level = ArabicLigaturizer::ar_nothing; //ArabicLigaturizer::ar_composedtashkeel | ArabicLigaturizer::ar_lig | ArabicLigaturizer::DIGITS_EN2AN;
	ArabicLigaturizer::Shape(s, alshapedtext, level);
	s = alshapedtext;
	
	bool bRTL = IsRightToLeftChar(utf8CharAt(s, 0)); // set initial state for RTL flag
	bool bRTLrender = bRTL; // bRTL must persist since it is modified and used in GetString. Hence a copy is used for rendering, inverted in value if the rendering direction is RTL 
							// (bRTLrender selects whether the text fragment being drawn is to be embedded reversed, eg. numbers in Arabic text).
	bool bDirectionChanged = true; // at the start of the string, want to force GetString to check which direction the text is reading, otherwise inherit it from the preceding text

	int font_ascent = (int)font->ascent;
	
	while ((*ypos + font_ascent) < fbheight && endstrpos < s.length()) {		
		bLineBreak = false;

		GetString(s, &startstrpos, &endstrpos, &textwidth, paint_black, font, bEmphasisOn, &bLineBreak, &bRTL, &bDirectionChanged, &bNewLine, fbwidth, *xpos);
		I2CSerial.printf("bRTL = %s\n", bRTL ? "<-" : "->");
		I2CSerial.printf("s.length=%d, startstrpos=%d, endstrpos=%d\n", s.length(), startstrpos, endstrpos);

		if (textwidth > 0) {
			bRTLrender = bRTL;
						
			if (render_right_to_left) {
				bRTLrender = !bRTLrender;
			} 
			
			if (*bEmphasisOn == false) {
				paint_black->DrawStringAt(*xpos, *ypos, s.substring(startstrpos, endstrpos), font, COLORED, render_right_to_left, bRTLrender);
			}
			else {
				paint_red->DrawStringAt(*xpos, *ypos, s.substring(startstrpos, endstrpos), font, COLORED, render_right_to_left, bRTLrender);			
			}
			
			*xpos += textwidth;		
		}
		
		if (bLineBreak) {			
			*ypos += font->heightPages * 2;
			*xpos = 0;
		}
		
		if (bNewLine) {
			*ypos += font->heightPages;
			*xpos = 0;
		}
		
		startstrpos = endstrpos;

		I2CSerial.printf("=\n\n");
	}

	if (*ypos >= fbheight) return true; // will return true if the text overflows the screen
	
	return false;						// oherwise will return false if there is more space
}

// render bidi text using disk font
bool Bidi::RenderText(String s, 
				      int* xpos, int* ypos, 
					  Paint* paint_black, Paint* paint_red, 
					  DiskFont* diskfont, 
					  bool* bEmphasisOn, 
					  int fbwidth, int fbheight, 
					  bool render_right_to_left) 
{
	if (diskfont->available == false) return true; // if no diskfont is available, return true to stop caller from trying to output more text (is used to indicate screen full)
	
	bool bNewLine = false;
	int startstrpos = 0;
	int endstrpos = 0;
	int textwidth = 0;
	bool bLineBreak = false;

	String alshapedtext = "";
	int level = ArabicLigaturizer::ar_nothing; //ArabicLigaturizer::ar_composedtashkeel | ArabicLigaturizer::ar_lig | ArabicLigaturizer::DIGITS_EN2AN;
	ArabicLigaturizer::Shape(s, alshapedtext, level);
	s = alshapedtext;
	
	bool bRTL = IsRightToLeftChar(utf8CharAt(s, 0)); // set initial state for RTL flag
	bool bRTLrender = bRTL; // bRTL must persist since it is modified and used in GetString. Hence a copy is used for rendering, inverted in value if the rendering direction is RTL 
							// (bRTLrender selects whether the text fragment being drawn is to be embedded reversed, eg. numbers in Arabic text).
	bool bDirectionChanged = true; // at the start of the string, want to force GetString to check which direction the text is reading, otherwise inherit it from the preceding text
	
	int font_ascent = (int)diskfont->_FontHeader.ascent;
	
	while ((*ypos + font_ascent) < fbheight && endstrpos < s.length()) {
		bLineBreak = false;

		GetString(s, &startstrpos, &endstrpos, &textwidth, diskfont, bEmphasisOn, &bLineBreak, &bRTL, &bDirectionChanged, &bNewLine, fbwidth, *xpos);
		I2CSerial.printf("bRTL = %s\n", bRTL ? "<-" : "->");
		I2CSerial.printf("s.length=%d, startstrpos=%d, endstrpos=%d\n", s.length(), startstrpos, endstrpos);

		if (textwidth > 0) {
			bRTLrender = bRTL;
			
			if (render_right_to_left) {
				bRTLrender = !bRTLrender;
			} 
			
			if (*bEmphasisOn == false) {
				diskfont->DrawStringAtA(*xpos, *ypos, s.substring(startstrpos, endstrpos), paint_black, COLORED, render_right_to_left, bRTLrender);
			}
			else {
				diskfont->DrawStringAtA(*xpos, *ypos, s.substring(startstrpos, endstrpos), paint_red, COLORED, render_right_to_left, bRTLrender);			
			}
			
			*xpos += textwidth;		
		}

		if (bLineBreak) {			
			*ypos += diskfont->_FontHeader.charheight * 2;
			*xpos = 0;
		}
		
		if (bNewLine) {
			*ypos += diskfont->_FontHeader.charheight;
			*xpos = 0;
		}

		startstrpos = endstrpos;
		
		I2CSerial.printf("=\n\n");
	}
	
	if (*ypos >= fbheight) return true; // will return true if the text overflows the screen
	
	return false;						// oherwise will return false if there is more space

}


/*
bool Bidi::IsRightToLeftChar(String ch) {
	uint32_t codepoint = codepointUtf8(ch);
	return IsRightToLeftChar(codepoint);
}

// https://stackoverflow.com/questions/4330951/how-to-detect-whether-a-character-belongs-to-a-right-to-left-language
bool Bidi::IsRightToLeftChar(uint32_t c)
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

uint32_t Bidi::codepointUtf8(String c) {
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

String Bidi::utf8fromCodepoint(uint32_t c) {
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

String Bidi::utf8CharAt(String s, int pos) { 
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

int Bidi::charLenBytesUTF8(char s) {
  byte ch = (byte) s;
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
*/
