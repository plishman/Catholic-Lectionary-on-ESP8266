#include "Bidi.h"	
	
//Bidi::Bidi() {
//}
	
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
					 int xpos, 
					 bool wrap_text) 
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
		I2CSerial.println(F("found tag"));

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
	while (pos < s.length() && (dcurrwidth < dmaxwidth || !wrap_text) && !bEmphasisTagFound && bCurrCharRightToLeft == bLookingForRightToLeft) {				
		bEmphasisTagFound = ExpectEmphasisTag(s, pos); // stop when an emphasis or line break tag is found, and break out of the while loop without changing pos
		if (bEmphasisTagFound) {
			I2CSerial.println(F("found tag-"));
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

		I2CSerial.print(bCurrCharRightToLeft ? F("<") : F(">"));
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
	
	if (wrap_text) {
		if (pos >= s.length() || bCurrCharRightToLeft != bLookingForRightToLeft) {
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos		
		}
		
		*endstrpos = lastwordendstrpos;	// make the end pos the index of the last character of the last whole word scanned that fitted on the line
		*textwidth = lastwordendxwidth;	// make the width of the text to be rendered the width of the whole line up to and including the last whole word scanned which fitted on the line
	}
	else {
		*endstrpos = pos;	// make the end pos the index of the last character of the last whole word scanned that fitted on the line
		*textwidth = (int)dcurrwidth;	// make the width of the text to be rendered the width of the whole line up to and including the last whole word scanned which fitted on the line		
	}

	if (dcurrwidth >= dmaxwidth) { // if the next word after lastwordendstrpos overflowed the line, tell the caller to generate a cr/newline (after rendering the text between
		*bNewLine = true;		 // *startstrpos and *endstrpos).
	}

	I2CSerial.println();
}

// render bidi text using disk font
bool Bidi::RenderText(String s, 
				      int* xpos, int* ypos, 
					  Paint* paint_black, Paint* paint_red, 
					  DiskFont* diskfont, 
					  bool* bEmphasisOn, 
					  int fbwidth, int fbheight, 
					  bool render_right_to_left,
					  bool wrap_text)
{
	//if (diskfont->available == false) return true; // if no diskfont is available, return true to stop caller from trying to output more text (is used to indicate screen full)
	
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

		GetString(s, &startstrpos, &endstrpos, &textwidth, diskfont, bEmphasisOn, &bLineBreak, &bRTL, &bDirectionChanged, &bNewLine, fbwidth, *xpos, wrap_text);
		I2CSerial.printf("bRTL = %s\n", bRTL ? "<-" : "->");
		I2CSerial.printf("xpos=%d, s.length=%d, startstrpos=%d, endstrpos=%d\n", *xpos, s.length(), startstrpos, endstrpos);

		if (textwidth > 0) {
			bRTLrender = bRTL;
			
			if (render_right_to_left) {
				bRTLrender = !bRTLrender;
			} 
			
			if (*bEmphasisOn == false) {
				diskfont->DrawStringAt(*xpos, *ypos, s.substring(startstrpos, endstrpos), paint_black, COLORED, render_right_to_left, bRTLrender);
			}
			else {
				diskfont->DrawStringAt(*xpos, *ypos, s.substring(startstrpos, endstrpos), paint_red, COLORED, render_right_to_left, bRTLrender);			
			}
			
			*xpos += textwidth;		
		}

		if (bLineBreak) {
			if (wrap_text) {
				*ypos += diskfont->_FontHeader.charheight * 2;
				*xpos = 0;
			}
			else {
				return true; // overflowed line
			}
		}

		if (bNewLine) {
			if (wrap_text) {
				*ypos += diskfont->_FontHeader.charheight;
				*xpos = 0;
			}
			else {
				return true; // overflowed line
			}
		}

		startstrpos = endstrpos;
		
		I2CSerial.printf("=\n\n");
	}
	
	if (*ypos >= fbheight) return true; // will return true if the text overflows the screen
	
	return false;						// otherwise will return false if there is more space

}

