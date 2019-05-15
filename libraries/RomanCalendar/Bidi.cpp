#include "Bidi.h"	

//#undef DEBUG_PRT
//#define DEBUG_PRT Serial
	
//Bidi::Bidi() {
//}

int Bidi::FindFirstSpacelikeCharacter(String s, int startpos) {
	uint32_t spacechars[14] = {0x0020, 0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x3000};
	
	int firstspacetypecharpos = s.length();
	bool bFoundSpacetypeChar = false;
	
	for (int i = 0; i < 14; i++) {
		String space_ch = utf8fromCodepoint(spacechars[i]);
		int pos = s.indexOf(space_ch, startpos);
		
		if (pos != -1 && pos < firstspacetypecharpos) {
			firstspacetypecharpos = pos;
			bFoundSpacetypeChar = true;
		}
	}
	
	if (bFoundSpacetypeChar) {
		return firstspacetypecharpos;
	}
	
	return -1;
}


bool Bidi::IsSpace(String& ch) {
	String spacechar = "";
	return IsSpace(ch, spacechar);
}

bool Bidi::IsSpace(String& ch, String& foundspacechar) {
	foundspacechar = "";
	
	uint32_t code = codepointUtf8(ch);
	
	switch(code)
	{
		case 0x0020:
		case 0x1680:
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2007:
		case 0x2008:
		case 0x2009:
		case 0x200A:
		case 0x3000:
			foundspacechar = ch;
			return true;
		
		default:
			return false;
	}
	
	return false;
}

bool Bidi::ExpectSpace(String& s, int* pos) {
	String ch = utf8CharAt(s, *pos);
	
	if (IsSpace(ch)) {
		*pos += ch.length();
		return true;
	}
	return false;
}
	
bool Bidi::ExpectRTL(String& s, int* pos) {
	String ch = utf8CharAt(s, *pos);
	
	if (ch == "") return false;
	
	uint32_t codepoint = codepointUtf8(ch);
	
	if (IsRightToLeftChar(codepoint)) {
		*pos += ch.length();
		return true;
	}
	
	return false;
}	

bool Bidi::ExpectStr(String& s, int* pos, String strtoexpect) {
	if (s.indexOf(strtoexpect, *pos) == *pos) {
		*pos += strtoexpect.length();
		return true;
	}
	return false;
}

bool Bidi::ExpectLineBreakTag(String& s, int* pos) {
// return true and advance the string index *pos when either <i> <b> </i> or </b> tags are found. *pos will be advanced past the last closing > in the tag
// *Emphasis_On will be left unchanged if no emphasis tag is found at the string starting in position *pos
// In this way, Emphasis_On can persist when the closing tag is found in a later call to the function.
	//DEBUG_PRT.printf("ExpectEmphasisTag: str at pos=%s\n", s.substring(*pos, *pos + 5).c_str());

	if (ExpectStr(s, pos, "<br>"))  return true; 
	if (ExpectStr(s, pos, "<br/>")) return true; 
	if (ExpectStr(s, pos, "<BR>"))  return true; 
	if (ExpectStr(s, pos, "<BR/>")) return true; 
	
	return false;
}

bool Bidi::ExpectEmphasisTag(String& s, int* pos, bool* bEmphasisOn, bool* bLineBreak) {
// return true and advance the string index *pos when either <i> <b> </i> or </b> tags are found. *pos will be advanced past the last closing > in the tag
// *Emphasis_On will be left unchanged if no emphasis tag is found at the string starting in position *pos
// In this way, Emphasis_On can persist when the closing tag is found in a later call to the function.
	//DEBUG_PRT.printf("ExpectEmphasisTag: str at pos=%s\n", s.substring(*pos, *pos + 5).c_str());

	if (s.indexOf("<i>",  *pos) == *pos) { *bEmphasisOn = true;  ExpectStr(s, pos, "<i>");  return true; } // ExpectStr will advance pos to the end of the tag 
	if (s.indexOf("<b>",  *pos) == *pos) { *bEmphasisOn = true;  ExpectStr(s, pos, "<b>");  return true; } // in string s
	if (s.indexOf("</i>", *pos) == *pos) { *bEmphasisOn = false; ExpectStr(s, pos, "</i>"); return true; } 
	if (s.indexOf("</b>", *pos) == *pos) { *bEmphasisOn = false; ExpectStr(s, pos, "</b>"); return true; }
	if (s.indexOf("<br>", *pos) == *pos) { *bLineBreak   = true;  ExpectStr(s, pos, "<br>"); return true; }
	if (s.indexOf("<br/>",*pos) == *pos) { *bLineBreak   = true;  ExpectStr(s, pos, "<br/>");return true; }
	
	return false;
}

bool Bidi::ExpectEmphasisTag(String& s, int pos) {
// nondestructive test for an emphasis tag - does not advance the string index, or change the state of the bEmphasisOn flag

	int p = pos;
	bool e = false;
	bool b = false;
	
	return ExpectEmphasisTag(s, &p, &e, &b);
}

bool Bidi::ExpectEmphasisTag(String& s, int* pos) {
// destructive test for an emphasis tag - advances the string index, but does not change the state of the bEmphasisOn flag

	bool e = false;
	bool b = false;
	
	return ExpectEmphasisTag(s, pos, &e, &b);
}

bool Bidi::ExpectEmphasisTagBefore(String& s, int* pos, int* skipped_count) {
// destructive test for an emphasis tag - decrements the string index
// this will jump over any tag in the reverse direction, not just supported ones.
	const char* const tags[6] = {"<i>", "<b>", "</i>", "</b>", "<br>", "</br>"};
	int tag_count = 6;

	String detected_tag = "";
	int detected_taglen = 0;
	
	if (s.charAt(*pos) == '>') { // if on a closing bracket
		DEBUG_PRT.print(F("\nfound > at pos="));
		DEBUG_PRT.println(*pos);

		int i = 0;
		String tag = "";
		bool bFound = false;
		int lastpos = *pos;
		
		while (i < tag_count && !bFound) {
			tag = String(tags[i]);
			DEBUG_PRT.print(F("Looking for tag "));
			DEBUG_PRT.print(tag);
			DEBUG_PRT.print(F("with length "));
			DEBUG_PRT.print(tag.length());
			DEBUG_PRT.println(F("..."));

			lastpos = s.lastIndexOf("<", *pos);
						
			detected_tag = s.substring(lastpos, *pos + 1);
			detected_taglen = detected_tag.length();

			//DEBUG_PRT.printf("s.lastIndexOf returned %d, *pos - lastpos = %d, substr is [%s], detected_tag=%s, detected_taglen=%d \n", 
			
			DEBUG_PRT.print(F("s.lastIndexOf returned "));
			DEBUG_PRT.print(lastpos);
			DEBUG_PRT.print(F(", *pos - lastpos = "));			
			DEBUG_PRT.print(*pos - lastpos);
			DEBUG_PRT.print(F(", substr is ["));
			DEBUG_PRT.print(s.substring(lastpos, *pos + 1));
			DEBUG_PRT.print(F("], detected_tag="));
			DEBUG_PRT.print(detected_tag);
			DEBUG_PRT.print(F(", detected_taglen="));
			DEBUG_PRT.println(detected_taglen);
			
			
			if (detected_taglen == tag.length() && tag == detected_tag) {
				//DEBUG_PRT.printf("found tag before = %s\n", tag.c_str());
				DEBUG_PRT.print(F("found tag before = "));
				DEBUG_PRT.println(tag);
				bFound = true;
			}
			i++;
		}

		if (bFound) {
			//DEBUG_PRT.printf("lastpos = %d, *pos = %d\n", lastpos, *pos);
			DEBUG_PRT.print(F("lastpos = "));
			DEBUG_PRT.print(lastpos);
			DEBUG_PRT.print(F(", *pos = "));
			DEBUG_PRT.println(*pos);
			
			*pos -= detected_taglen;// skip back to the character just before the tag.
			*skipped_count += detected_taglen; // skipped_count is not zeroed, so it can keep a running total, if more than one tag is found on subsequent calls
			return true;	// *pos will be set to -1 if the tag is at the very start of the string.
		}
	}
	return false;
}


/// !!! This function will have a crash bug - if the width of the string in pixels (before a breaking mark occurs) exceeds the full width of the screen!!!
/// If it is fixed by allowing such strings to wrap without a breaking space, then there will be the problem of rtl strings (in ltr text) or ltr strings (in rtl text)
/// which will need to be reversed for the purpose of calculating the width of the string before the wrap and then after, so that they wrap properly (so the broken
/// bits of the reversed string are in the right order, on the right lines.
/// This problem should only occur if very large fonts are used. To be fixed!

// GetString for disk font
void Bidi::GetString(String& s,
					 int* startstrpos, 
					 int* endstrpos, 
					 int* textwidth, 
					 DiskFont& diskfont, 
					 bool* bLineBreak,	 // this variable is set when either a <br> or <br/> tag is encountered, to tell the caller to skip a line before resuming text output
					 bool* bRTL,		 // this variable allows the state of the text block just scanned, right-to-left or left-to-right to be returned 
					 bool* bDirectionChanged, // this variable shows when the reading direction at the end of the text block just scanned has changed direction
					 bool* bNewLine,
					 bool bOneWordOnly,	// this flag if true will cause words to be processed one at a time, rather than by the number that will fit in remaining space on the line
					 bool bForceLineBreakAfterString, 
					 int forcedBreakPos,
					 int fbwidth, 
					 int xpos, 
					 bool wrap_text,
					 bool bLastLine,
					 int ellipsiswidth,
					 bool* bDisplayEllipsisNow,
					 bool bMoreText ) 
{
	//DEBUG_PRT.printf("GetString() xpos=%d, s.length=%d, startstrpos=%d, endstrpos=%d, textwidth=%d\n", xpos, s.length(), *startstrpos, *endstrpos, *textwidth);
	
	DEBUG_PRT.print(F("GetString() xpos="));
	DEBUG_PRT.print(xpos);
	DEBUG_PRT.print(F(", s.length="));
	DEBUG_PRT.print(s.length());
	DEBUG_PRT.print(F(", startstrpos="));
	DEBUG_PRT.print(*startstrpos);
	DEBUG_PRT.print(F(", endstrpos="));
	DEBUG_PRT.print(*endstrpos);
	DEBUG_PRT.print(F(", textwidth="));
	DEBUG_PRT.println(*textwidth);

	*bDisplayEllipsisNow = false;

	int pos = *startstrpos; // pos stores the postion in the string of the utf8 character currently being scanned
	
	int lastwordendstrpos = *startstrpos; // this variable stores the position of the last space (text word boundary) found 
	int lastwordendxwidth = 0; // this variable stores the width of the scanned string in pixels up to the last text word boundary 
	
	int utf8CharCount = 0;
	double dcurrwidth = 0.0;
	double dmaxwidth = (double)(fbwidth - xpos);

	*bNewLine = false;
	
	// check if an emphasis tag is present in the string starting at pos 
	if (ExpectLineBreakTag(s, startstrpos)) { // if so, skip the tag by changing the _start_ pos of the string to the first character after 
		DEBUG_PRT.println(F("found linebreak tag"));

		*bLineBreak = true;		   // signal a linebreak
		*endstrpos = *startstrpos; // the tag, and set the end pos of the scanned region to be the as the new start position (after the tag!).
		*textwidth = 0;			   // the textwidth of the scanned text must be 0, since no more text has been scanned, and the tag is skipped.
		
		return;
	}
	
	*bLineBreak = false;
	
	// scan string s up to the point where either 
	// i)   the string index pos reaches the end of the string
	// ii)  the width of the string in pixels exceeds the remaining space on the line
	// iii) an opening or closing emphasis tag <i>, </i>, <b>, or </b> is found
	// iv)  the string changes from left to right to right to left reading, or vice versa
	
	
	String ch = utf8CharAt(s, pos); // get character
	
	bool bLookingForRightToLeft = *bRTL; //IsRightToLeftChar(ch); // inherit rtl or ltr reading direction from previous call, unless previous call showed that the direction changed	
	
	if (*bDirectionChanged) {
		if (ch != "." && ch != " ") {
			bLookingForRightToLeft = IsRightToLeftChar(ch);
			*bRTL = bLookingForRightToLeft;
		}
		else {
			*bRTL = !(*bRTL);
			bLookingForRightToLeft = *bRTL;
		}
		*bDirectionChanged = false;
	}	

//	if(xpos == 0) {
//		if(ExpectStr(s, &pos, " ")) {
//			DEBUG_PRT.println("Skipped space at start of line");
//		}
//	}
	
	bool bCurrCharRightToLeft = bLookingForRightToLeft;
	//DEBUG_PRT.printf("\n*bCurrCharRightToLeft=%s, bLookingForRightToLeft=%s\n", bCurrCharRightToLeft?"true":"false", bLookingForRightToLeft?"true":"false");

	bool bEmphasisTagFound = false;
	
	int wordcount = 0;
	
//	DEBUG_PRT.printf("() bOneWordOnly = %s\n", bOneWordOnly ? "true" : "false");
//	
//	DEBUG_PRT.printf("GetString() pos < s.length = %s\n", pos < s.length() ? "true" : "false");
//	DEBUG_PRT.printf("GetString() dcurrwidth < dmaxwidth || !wrap_text = %s\n", (dcurrwidth < dmaxwidth || !wrap_text) ? "true" : "false");
//	DEBUG_PRT.printf("GetString() bCurrCharRightToLeft == bLookingForRightToLeft = %s\n", (bCurrCharRightToLeft == bLookingForRightToLeft) ? "true" : "false");
//	DEBUG_PRT.printf("GetString() !(bOneWordOnly && wordcount == 1) = %s\n", (!(bOneWordOnly && wordcount == 1)) ? "true" : "false");
	
	bool bAttachSpaceOrDotToNextRTLWord = false;
	String last_ch = "";

	int pos_start_spaceordot_start = -1;	// saves start and end pos of runs of dots and/or spaces, which attach to the word ahead or behind, depending on the reading order
	int pos_start_spaceordot_end = -1;
	
	bool bLineBreakTagFound = false;
	
	int prevwordendstrpos = pos;
	double prevwordendxwidth = (int)dcurrwidth;
	int prevwordcount = wordcount;
	
	// now determine the length of the right to left or left to right string from the start character ch found at position pos
	while (  (pos < s.length()) 	 			 				// keep doing the following while: not at end of string
		  && (dcurrwidth < dmaxwidth || !wrap_text)  		    // and the width of the string in pixels doesn't exceed the remaining space, (or if text is not being wrapped)
		  /*&& (!*bLineBreak)*/ 									// and an linebreak tag has not been fount
		  && !bLineBreakTagFound
		  && (bCurrCharRightToLeft == bLookingForRightToLeft)	// and the reading direction hasn't changed
		  && (!(bOneWordOnly && wordcount == 1)) )  // and fewer than 1 word has been processed if the bOneWordOnly flag is set
	{						
		int curpos = pos;
		if (ExpectLineBreakTag(s, &curpos) || pos == forcedBreakPos) { // if so, skip the tag by changing the _start_ pos of the string to the first character after 
			//*bLineBreak = true;
			bLineBreakTagFound = true;
			DEBUG_PRT.println(F("found linebreak tag-"));			
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos

			wordcount++;
			continue;
		}

		//DEBUG_PRT.printf("\n-bCurrCharRightToLeft=%s, bLookingForRightToLeft=%s\n", bCurrCharRightToLeft?"true":"false", bLookingForRightToLeft?"true":"false");

		last_ch = ch;
		ch = utf8CharAt(s, pos); // get next character

		if (ch != "." && ch != " ") { // . and " " inherit the reading direction of the text around them, so don't check if reading direction is changed for these
			bCurrCharRightToLeft = IsRightToLeftChar(ch);
		}
		
		utf8CharCount++;

		//DEBUG_PRT.printf("\n+bCurrCharRightToLeft=%s, bLookingForRightToLeft=%s\n", bCurrCharRightToLeft?"true":"false", bLookingForRightToLeft?"true":"false");

		DEBUG_PRT.print(bCurrCharRightToLeft ? F("<") : F(">"));
		if (bCurrCharRightToLeft != bLookingForRightToLeft) {
			DEBUG_PRT.println("GetString() text direction changed");
			*bDirectionChanged = true; //report that the direction of the next block to be scanned (on the next call) will be reversed
			continue;
		}

		//DEBUG_PRT.printf("last_ch=%s\n", last_ch.c_str());
		
		if (IsSpace(ch) || ch == ".") { // space char is special case, inherits the reading direction of the character preceding it. Also period (.), which sometimes (it appears) is used in RTL text				
		
			//DEBUG_PRT.print("[sp/.]");

			prevwordendstrpos = lastwordendstrpos;
			prevwordendxwidth = (int)dcurrwidth;
			prevwordcount = wordcount;
		
			pos += ch.length();
			dcurrwidth += diskfont.GetTextWidthA(ch);

			// in Chinese text, a Chinese space character (the size of the other Chinese glyphs) appears to the left and
			// right of their comma and to the left of their full stop characters. These characters cannot begin a line in 
			// Chinese typography, so need to handle that case.
			
			String ch_punctuation = utf8CharAt(s, pos);
			
			if ((ch_punctuation == "、") || (ch_punctuation == "。") || (ch_punctuation == "．")) { // these are Chinese comma and full stop characters, and they are usually surrounded by Chinese space characters (before and after)			
				double d_ch_p_width = diskfont.GetTextWidthA(ch_punctuation);
				
				DEBUG_PRT.print(F("[、。．]"));

				// if the comma or dot overflows the line, print it and the preceding ideogram on the next line
				if (dcurrwidth + d_ch_p_width > dmaxwidth) {	// if this line (including the punctuation) overflows
					//DEBUG_PRT.println("Punctuation (Chinese) overflows line - wrapping previous word so that punctuation is not at start of line.");
					*bNewLine = true;	// tell caller to insert a newline after the line to be printed
					break;				// and exit the loop
				}
				
				pos += ch_punctuation.length(); // punctuation character didn't overflow, so add it to dcurrwidth and bump string position pos
				dcurrwidth += d_ch_p_width;
			}
			
			int currxpos = ((int)dcurrwidth) + xpos;
			int lastxpos = lastwordendxwidth + xpos;

			//prevwordendstrpos = lastwordendstrpos;
			//prevwordendxwidth = (int)dcurrwidth;
			
			if (bLastLine) {
				if (lastxpos > (fbwidth - ellipsiswidth) && (currxpos > fbwidth)) { // don't print last two words if so (this word overflows the display and the previous word overflows the ellipsis area, and this is the last line)
					lastwordendstrpos = prevwordendstrpos;
					lastwordendxwidth = prevwordendxwidth;
					wordcount = prevwordcount;
					bLineBreakTagFound = true;
					*bDisplayEllipsisNow = true;
					DEBUG_PRT.println(F("Ellipsis case 1"));
					continue;
				}

				if (lastxpos <= (fbwidth - ellipsiswidth) && (currxpos > fbwidth)) { // don't print the last word if so (this word overflows the display, but the previous word does not overflow the ellipsis area)
					bLineBreakTagFound = true;
					*bDisplayEllipsisNow = true;
					DEBUG_PRT.println(F("Ellipsis case 2"));
					continue;
				}
				
				if (lastxpos > (fbwidth - ellipsiswidth) && (currxpos <= fbwidth) && (pos < s.length() || bMoreText) ) { // don't print last two words if so (both words encroach on the ellipsis area (though the last one is within the screen width), but there is more text to display, so need to leave room for the ellipsis)
					//DEBUG_PRT.printf("lastxpos=%d, fbwidth=%d, ellipsiswidth=%d, currxpos=%d, pos=%d, bMoreText=%d\n", lastxpos, fbwidth, ellipsiswidth, currxpos, pos, bMoreText);
					
					DEBUG_PRT.print(F("lastxpos="));
					DEBUG_PRT.print(lastxpos);
					DEBUG_PRT.print(F(", fbwidth="));
					DEBUG_PRT.print(fbwidth);
					DEBUG_PRT.print(F(", ellipsiswidth="));
					DEBUG_PRT.print(ellipsiswidth);
					DEBUG_PRT.print(F(", currxpos="));
					DEBUG_PRT.print(currxpos);
					DEBUG_PRT.print(F(", pos="));
					DEBUG_PRT.print(pos);
					DEBUG_PRT.print(F(", bMoreText="));
					DEBUG_PRT.println(bMoreText);

					//lastxpos=0, fbwidth=389, ellipsiswidth=11, currxpos=31, pos=160, bMoreText=1
					lastwordendstrpos = prevwordendstrpos;
					lastwordendxwidth = prevwordendxwidth;
					wordcount = prevwordcount;
					bLineBreakTagFound = true;
					*bDisplayEllipsisNow = true;
					DEBUG_PRT.println(F("Ellipsis case 3"));
					continue;
				}

				if (lastxpos <= (fbwidth - ellipsiswidth) && currxpos > (fbwidth - ellipsiswidth) && (pos < s.length() || bMoreText) ) { // don't print the last word if so (the previous word does not overflow the ellipsis area, but this word does (but is within the display), but there is more text to display (which will not be as there is no room, so need to leave room for the ellipsis)
					//DEBUG_PRT.printf("lastxpos=%d, fbwidth=%d, ellipsiswidth=%d, currxpos=%d, pos=%d, bMoreText=%d\n", lastxpos, fbwidth, ellipsiswidth, currxpos, pos, bMoreText);

					DEBUG_PRT.print(F("lastxpos="));
					DEBUG_PRT.print(lastxpos);
					DEBUG_PRT.print(F(", fbwidth="));
					DEBUG_PRT.print(fbwidth);
					DEBUG_PRT.print(F(", ellipsiswidth="));
					DEBUG_PRT.print(ellipsiswidth);
					DEBUG_PRT.print(F(", currxpos="));
					DEBUG_PRT.print(currxpos);
					DEBUG_PRT.print(F(", pos="));
					DEBUG_PRT.print(pos);
					DEBUG_PRT.print(F(", bMoreText="));
					DEBUG_PRT.println(bMoreText);	
				
					//lastxpos=0, fbwidth=389, ellipsiswidth=11, currxpos=31, pos=160, bMoreText=1
					bLineBreakTagFound = true;
					*bDisplayEllipsisNow = true;
					DEBUG_PRT.println(F("Ellipsis case 4"));
					continue;
				}

				DEBUG_PRT.println(F("Ellipsis case 0"));
			}
			
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos

			wordcount++;
			
			//DEBUG_PRT.printf("[wc=%d, lwepos=%d, lwewid=%d]\n", wordcount, lastwordendstrpos, lastwordendxwidth);
			continue;
		}							

		dcurrwidth += diskfont.GetTextWidthA(ch);
		pos += ch.length();
		DEBUG_PRT.print(ch);
	}

	if (dcurrwidth >= dmaxwidth || bForceLineBreakAfterString) { // if the next word after lastwordendstrpos overflowed the line, tell the caller to generate a cr/newline (after rendering the text between *startstrpos and *endstrpos).
		if (!ExpectLineBreakTag(s, &pos)) {
			DEBUG_PRT.print(F("GetString() Newline: dcurrwidth="));
			DEBUG_PRT.print((int)dcurrwidth);
			DEBUG_PRT.print(F(" dmaxwidth=")); 
			DEBUG_PRT.println((int)dmaxwidth);
			
			*bNewLine = true;		 
		}
		else {
			DEBUG_PRT.print(F("GetString() Newline at end of line being suppressed, as it is followed immediately by a linebreak tag:  dcurrwidth="));
			DEBUG_PRT.print((int)dcurrwidth);
			DEBUG_PRT.print(F(" dmaxwidth=")); 
			DEBUG_PRT.println((int)dmaxwidth);
		}
	}
	
	//DEBUG_PRT.printf("GetString() *startstrpos = %d, *endstrpos = %d, lastwordendstrpos = %d\n", *startstrpos, *endstrpos, lastwordendstrpos);
	DEBUG_PRT.print(F("GetString() *startstrpos = "));
	DEBUG_PRT.print(*startstrpos);
	DEBUG_PRT.print(F(", *endstrpos = "));
	DEBUG_PRT.print(*endstrpos);
	DEBUG_PRT.print(F(", lastwordendstrpos = "));
	DEBUG_PRT.println(lastwordendstrpos);
	
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

	//DEBUG_PRT.printf("GetString() %d words processed\n", wordcount);

	DEBUG_PRT.println();
}

String Bidi::strEllipsis = "";


void Bidi::SetEllipsisText(String ellipsis_text) {
	Bidi::strEllipsis = ellipsis_text;
	DEBUG_PRT.print(F("Bidi::Ellipsis text set to "));
	DEBUG_PRT.println(Bidi::strEllipsis);
}

void Bidi::StripTags(String& text) {
	text.replace("<b>", "");
	text.replace("</b>", "");
	text.replace("<i>", "");
	text.replace("</i>", "");
	text.replace("<br>", "");
	text.replace("<br/>", "");

	text.replace("<B>", "");
	text.replace("</B>", "");
	text.replace("<I>", "");
	text.replace("</I>", "");
	text.replace("<BR>", "");
	text.replace("<BR/>", "");	
	
	//return text;
}

void Bidi::StripEmphasisTagsOnly(String& text) { // strip only emphasis tags. Linebreak and all other tags should be left. The color map is generated before text
	text.replace("<b>", "");					  // processing by GetString, so this has already been dealt with by this time, but line breaks have not
	text.replace("</b>", "");
	text.replace("<i>", "");
	text.replace("</i>", "");

	text.replace("<B>", "");
	text.replace("</B>", "");
	text.replace("<I>", "");
	text.replace("</I>", "");
	
	//return text;
}

// This function checks if the next word is printable (ie, not html), and if so, if it is wider than a whole screen (such words will be wrapped without a
// breaking space). Returns true if an overflowing word was found and changed.

// I'm not going any further with this now. It renders emphasised text (red) incorrectly in edge cases, where embedded strings with reverse reading order get
// transposed so that they render intelligibly. The position of the text is correct, but when the text is longer than the width of the display, and contains tags
// which get transposed along with the text the text may be emphasised incorrectly, though the text position is correct.

bool Bidi::FixNextWordWiderThanDisplay(String& s, 
									   String& colormap, 
									   int& charsprocessed,
									   int& numcharslefttoprocess,
									   bool& bIsLastFragment,
									   bool& bMakeLineBreakBefore,
									   int* forcedBreakPos,
									   int startstrpos, 
									   int fbwidth, 
									   int xpos, 
									   bool render_right_to_left, 
									   DiskFont& diskfont)
{
	*forcedBreakPos = -1;
	
	if (ExpectEmphasisTag(s, startstrpos)) return false;	// return if it's an html tag
	
	//while (ExpectStr(s, &startstrpos, " ")); // skip over any leading spaces (adding spaces to create a break in earlier calls to this function will lengthen the string, so the first char on the next call may be the inserted space, perhaps followed by others which were already in the text).
	
	while (ExpectSpace(s, &startstrpos)); // skip over any leading unicode spaces (all types, not just 0x20)
	
	if (startstrpos == s.length()) return false; // return if got to the end of the string without finding the beginning of a new word
	
	//int endstrpos = s.indexOf(" ", startstrpos + 1); // find the first space character following the start of the word just found
	int endstrpos = FindFirstSpacelikeCharacter(s, startstrpos + utf8CharAt(s, startstrpos).length()); // start scanning at next character after character which starts the word
	
	if (endstrpos == -1) endstrpos = s.length();

	DEBUG_PRT.print(F("startstrpos = "));
	DEBUG_PRT.print(startstrpos);
	DEBUG_PRT.print(F(", endstrpos = "));
	DEBUG_PRT.println(endstrpos);
		
//	String word = s.substring(startstrpos, endstrpos); // get the next word up to the space char
//	if (word.length() == 0) {
//		DEBUG_PRT.println(F("word len is 0"));
//		return false;
//	}
	
	if(startstrpos == endstrpos) {
		DEBUG_PRT.println(F("word len is 0"));
		return false;
	}

	String word = s.substring(startstrpos, endstrpos); // get the next word up to the space char
	
	int last_numcharslefttoprocess = numcharslefttoprocess;
	numcharslefttoprocess = word.length();
	
	String colormap_word = colormap.substring(startstrpos, endstrpos);
	
	DEBUG_PRT.print(F("word is: ["));
	DEBUG_PRT.print(word);
	DEBUG_PRT.print(F("] "));
	
	int textwidthpx = diskfont.GetTextWidth(word);
	DEBUG_PRT.print(F("word textwidth (px) is: ["));
	DEBUG_PRT.print(String(textwidthpx));
	DEBUG_PRT.print(F("] "));
	
	bool bRTL = IsRightToLeftChar(utf8CharAt(word, 0)); // check if the next word starts with an RTL char. Will stop processing also if the reading direction changes.

	double dwidth_remaining = (double)(fbwidth - xpos);
	DEBUG_PRT.print(F("dwidth_remaining (px) is: ["));
	DEBUG_PRT.print(String((int)dwidth_remaining));
	DEBUG_PRT.println(F("]"));
	
	//bool bStringModified = false;
	bIsLastFragment = false;
	bMakeLineBreakBefore = false;
	
	if (textwidthpx <= fbwidth) {
		if (last_numcharslefttoprocess > 0) {  // deal with trailing fragment of string wider than display
			bIsLastFragment = true;
			numcharslefttoprocess = 0;
			return true;
		}
		
		if (textwidthpx > (fbwidth - xpos)) { // if textwidthpx <= fbwidth (ie word is less than one whole screen wide), 
			bMakeLineBreakBefore = true;	  // but still overruns the remaining space (taking into account xpos), then ask for a linebreak before
			return false;
		}

		return false;
	}
	
	if (xpos > 0 && textwidthpx > (int)dwidth_remaining && ((render_right_to_left && !bRTL) || (!render_right_to_left && bRTL))) {
		bMakeLineBreakBefore = true;
		dwidth_remaining = (double)(fbwidth);
	}
	
	if (textwidthpx > fbwidth) {
		DEBUG_PRT.println(F("Bidi::IsNextWordWiderThanDisplay() word is wider than display!"));
		int utf8strlen = Utf8CharCount(word);
		String utf8char = "";
		double dcurrwidth = 0.0;
		int substrwidth = 0;
		int lastsubstrwidth = 0;
		
		int i = 0;
		bool bDone = false;
		
		while (i < utf8strlen && !bDone) {
			while(ExpectEmphasisTag(word, &i)); // skip any tags
			utf8char = GetUtf8CharByIndex(word, i);
			lastsubstrwidth = substrwidth;
			substrwidth += utf8char.length();
			
			double dcharwidth = diskfont.GetTextWidthA(utf8char);
			dcurrwidth += dcharwidth;

			if (charsprocessed == 0 && dcharwidth > dwidth_remaining) { // if can fit less than one character, finish and don't modify string
				bDone = true;
				continue;
			}

			numcharslefttoprocess--;

			if (dcurrwidth >= dwidth_remaining) { 
				if (i > 0) { // if have processed at least one character
					//s =               s.substring(0, startstrpos + lastsubstrwidth) + " " +        s.substring(startstrpos + lastsubstrwidth);
					//colormap = colormap.substring(0, startstrpos + lastsubstrwidth) + " " + colormap.substring(startstrpos + lastsubstrwidth);

					////char c = colormap.charAt(startstrpos + lastsubstrwidth) | 0x20;
					////colormap.setCharAt(startstrpos + lastsubstrwidth, c);	// set bit 5 to indicate hard linebreak here
					
					*forcedBreakPos = startstrpos + lastsubstrwidth;
					
					//word = s.substring(startstrpos, startstrpos + lastsubstrwidth); // debugging
					//bStringModified = true;
				}
				bDone = true;
				continue; // finished, don't increment char count before dropping out of loop
			}
			else if (IsRightToLeftChar(utf8char) != bRTL) {
				DEBUG_PRT.println(F("Bidi::IsNextWordWiderThanDisplay() reading direction changed"));
				bDone = true; // stop if the reading direction changes
				continue; // finished, don't increment char count before dropping out of loop
			}

			charsprocessed++;
			i++;
		}			
		
		//if (bStringModified) {
		//	StripTags(word);
		//	DEBUG_PRT.printf("\nBidi::IsNextWordWiderThanDisplay(): %s\n", word.c_str());
		//}
		
		return true;//bStringModified;
	}

	return false;
}

void Bidi::getColorMap(String& s, bool& bEmphasisOn, String& colormap) {
	int pos = 0;
	int len = s.length();
	bool bLineBreak = false;
	colormap = "";
	String ch = "";
	
	DEBUG_PRT.print(F("\ngetColorMap() word is ["));
	DEBUG_PRT.print(s);
	DEBUG_PRT.println(F("]"));
	
	while(pos < len) {
		ExpectEmphasisTag(s, &pos, &bEmphasisOn, &bLineBreak);
		colormap += bEmphasisOn ? "R" : "B";
		pos++; // one colormap character *per byte* of string (not per utf8 character - simpler, but uses more memory)
	}
	
	DEBUG_PRT.print(F("\ngetColorMap() cmap is ["));
	DEBUG_PRT.print(colormap);
	DEBUG_PRT.println(F("]"));
}


bool Bidi::RenderText(String& s, 
				      int* xpos, int* ypos, 
					  TextBuffer& tb, 
					  DiskFont& diskfont, 
					  bool* bEmphasisOn, 
					  int fbwidth, int fbheight, 
					  bool render_right_to_left,
					  bool wrap_text)
{
	RenderText(s, xpos, ypos, tb, diskfont, bEmphasisOn, fbwidth, fbheight, render_right_to_left, wrap_text, false);
}

// render bidi text using disk font
bool Bidi::RenderText(String& s, 
				      int* xpos, int* ypos, 
					  TextBuffer& tb, 
					  DiskFont& diskfont, 
					  bool* bEmphasisOn, 
					  int fbwidth, int fbheight, 
					  bool render_right_to_left,
					  bool wrap_text,
					  bool bMoreText)	// PLL 26-12-2018 bMoreText set to true if this is not the last string in the block to be drawn
{
	//if (diskfont.available == false) return true; // if no diskfont is available, return true to stop caller from trying to output more text (is used to indicate screen full)
	
	bool bNewLine = false;
	int startstrpos = 0;
	int endstrpos = 0;
	int textwidth = 0;
	bool bLineBreak = false;

	String alshapedtext = "";
	alshapedtext.reserve(s.length());
	int level = ArabicLigaturizer::ar_nothing; //ArabicLigaturizer::ar_composedtashkeel | ArabicLigaturizer::ar_lig | ArabicLigaturizer::DIGITS_EN2AN;
	ArabicLigaturizer::Shape(s, alshapedtext, level);
	s = alshapedtext;
	
	bool bRTL = IsRightToLeftChar(utf8CharAt(s, 0)); // set initial state for RTL flag
	bool bRTLrender = bRTL; // bRTL must persist since it is modified and used in GetString. Hence a copy is used for rendering, inverted in value if the rendering direction is RTL 
							// (bRTLrender selects whether the text fragment being drawn is to be embedded reversed, eg. numbers in Arabic text).
	bool bDirectionChanged = true; // at the start of the string, want to force GetString to check which direction the text is reading, otherwise inherit it from the preceding text
	
	int font_ascent = (int)diskfont._FontHeader.ascent; //**** BUG - will not work with new diskfonts PLL 03-02-2019
	
	bool bLastLine = false;
	int currfbwidth = fbwidth;
	int ellipsiswidth = (int)diskfont.GetTextWidthA(Bidi::strEllipsis);
	bool bInsertEllipsis = false;
	bool bOverflowWordWrapped = false;
	int last_xpos = 0;
	int last_ypos = 0;
	int curr_xpos = 0;
	int curr_ypos = 0;

	String colormap = "";
	colormap.reserve(s.length());
	
	bool bEmphasisOnAtEnd = *bEmphasisOn;
	getColorMap(s, bEmphasisOnAtEnd, colormap);
	StripEmphasisTagsOnly(s);
	*bEmphasisOn = bEmphasisOnAtEnd;
	
	int charsprocessedbyoutsizestringhandler = 0;
	int numcharslefttoprocess = 0;
	bool bIsLastFragment = false;
	bool bMakeLineBreakBefore = false;
	
	DEBUG_PRT.print(F("\nBidi::RenderText(): str  =["));
	DEBUG_PRT.print(s);
	DEBUG_PRT.println(F("]"));
	
	DEBUG_PRT.print(F("Bidi::RenderText(): cmap =["));
	DEBUG_PRT.print(colormap);
	DEBUG_PRT.println(F("]"));

	String colormap_substr = "";
	String s_substr = "";
	
	colormap_substr.reserve(s.length());
	s_substr.reserve(s.length());

	bool bDisplayEllipsisNow = false;
		
	while ((*ypos + font_ascent) < fbheight && endstrpos < s.length() && !bDisplayEllipsisNow) {
		bLastLine = (wrap_text && (*ypos + font_ascent < fbheight && *ypos + (font_ascent * 2) >= fbheight)); //only worry about last line if wrapping text
		if (bLastLine) {
			DEBUG_PRT.println(F("bLastLine is true"));
			currfbwidth = fbwidth - ellipsiswidth;
		} 
		else {
			DEBUG_PRT.println(F("bLastLine is false"));
			currfbwidth = fbwidth;
		}
	
		bLineBreak = false;
		bIsLastFragment = false;
		bMakeLineBreakBefore = false;
		int forcedBreakPos = -1;
		
		// Fix case where string exceeds the whole screen width. If the next string is opposite reading 
		// direction, the *last* part of the string must be rendered on the upper line, and the first part on the lower line!
		// Easiest thing to do here is calculate how much of the end of the string should be rendered on the upper line, and insert a space after that point
		
		DEBUG_PRT.printf("FixNextWordWiderThanDisplay() /xpos=%d ypos=%d/\n", *xpos, *ypos);
		bOverflowWordWrapped = FixNextWordWiderThanDisplay(s, 
														   colormap,
														   charsprocessedbyoutsizestringhandler,
														   numcharslefttoprocess,
														   bIsLastFragment,
														   bMakeLineBreakBefore,
														   &forcedBreakPos,
														   startstrpos, 
														   fbwidth, 
														   *xpos, //last_xpos + textwidth, 
														   render_right_to_left, 
														   diskfont);
														   
		//DEBUG_PRT.printf("bOverflowWordWrapped = %s\n", bOverflowWordWrapped ? "true" : "false");			
		//DEBUG_PRT.println("String s is [" + s + "]");

		//DEBUG_PRT.printf("Bidi::RenderText() bMoreText=%s\n", bMoreText ? "true" : "false");
		DEBUG_PRT.print(F("Bidi::RenderText() bMoreText="));
		DEBUG_PRT.println(bMoreText ? F("true") : F("false"));
		
		//if(bOverflowWordWrapped) {
		//	ExpectStr(s, &startstrpos, " "); // skip over the leading space added by FixNextWordWiderThanDisplay()
		//}

		//DEBUG_PRT.printf("bMakeLineBreakBefore=%s\n", bMakeLineBreakBefore?"true":"false");
		if (bMakeLineBreakBefore) {
			*ypos += diskfont._FontHeader.charheight;
			*xpos = 0;
		}
		
		//DEBUG_PRT.printf("bOverflowWordWrapped=%s bIsLastFragment=%s bMakeLineBreakBefore=%s\n", bOverflowWordWrapped?"true":"false", bIsLastFragment?"true":"false", bMakeLineBreakBefore?"true":"false");
				
		GetString(s, &startstrpos, &endstrpos, &textwidth, diskfont, 
				  &bLineBreak, &bRTL, &bDirectionChanged, 
				  &bNewLine, 
				  false,	// bOneWordOnly - may be deprecated?
				  false,	// bForceLineBreakAfterString - may be deprecated?
				  forcedBreakPos,	// index of character where forced break must occur, or -1 if not needed (needed for words wider than the display)
				  currfbwidth, 
				  *xpos, 
				  wrap_text,
				  bLastLine,
				  ellipsiswidth,
				  &bDisplayEllipsisNow,
				  bMoreText); 
		  
		DEBUG_PRT.print(F("bRTL = "));
		DEBUG_PRT.println(bRTL ? F("<-") : F("->"));
		
		//DEBUG_PRT.printf("xpos=%d, s.length=%d, startstrpos=%d, endstrpos=%d, textwidth=%d\n", *xpos, s.length(), startstrpos, endstrpos, textwidth);
		DEBUG_PRT.print(F("xpos="));
		DEBUG_PRT.print(*xpos);
		DEBUG_PRT.print(F(", s.length="));
		DEBUG_PRT.print(s.length());
		DEBUG_PRT.print(F(", startstrpos="));
		DEBUG_PRT.print(startstrpos);
		DEBUG_PRT.print(F(", endstrpos="));
		DEBUG_PRT.print(endstrpos);
		DEBUG_PRT.print(F(", textwidth="));
		DEBUG_PRT.print(textwidth);
		
//		if ((textwidth > 0 && bLastLine && bNewLine && bMoreText && diskfont.GetTextWidthA(s, ellipsiswidth) > fbwidth - *xpos) // ellipsiswidth is limit to GetTextWidthA, so it stops processing when the calculated string width is greater than ellipsiswidth
//	     || (textwidth > 0 && bLastLine && bMoreText && endstrpos == s.length())) { 
			//if *some text was processed (ie, not html tags) and;
			//	 *this is the last line and;
			//	 *the line overflowed the width of the screen minus the width of the ellipsis and;
			//   *and *the text remaining to be printed would overflow the width of the screen were the ellipsis not printed*
			//		then need to insert the ellipsis.
			//DEBUG_PRT.printf("InsertEllipsis:\ntextwidth>0=%d\n, bLastLine=%d\n, bNewLine=%d\n, diskfont.GetTextWidthA(s, ellipsiswidth) > fbwidth - *xpos = %d\nbMoreText=%d\n, endstrpos == s.length()=%d\n", (textwidth > 0), bLastLine, bNewLine, (diskfont.GetTextWidthA(s, ellipsiswidth) > fbwidth - *xpos), bMoreText, (endstrpos == s.length()));
			
//			bInsertEllipsis = true;
//		}

		bInsertEllipsis = bDisplayEllipsisNow;
			
		if (textwidth > 0) {
			bRTLrender = bRTL;
			
			if (render_right_to_left) {
				bRTLrender = !bRTLrender;
			} 
			
			uint16_t color = *bEmphasisOn ? GxEPD_RED : GxEPD_BLACK;			
			
			colormap_substr = colormap.substring(startstrpos, endstrpos);
			s_substr = s.substring(startstrpos, endstrpos);
			StripTags(s_substr);
			
			DEBUG_PRT.print(F("Adding: ["));
			DEBUG_PRT.print(s_substr);
			DEBUG_PRT.println(F("]"));
			
			DEBUG_PRT.print(F("        ["));
			DEBUG_PRT.print(colormap_substr);
			DEBUG_PRT.println("]");
			
			DEBUG_PRT.print(F("render_right_to_left="));
			DEBUG_PRT.print(render_right_to_left?F("true"):F("false"));
			DEBUG_PRT.print(F(", bRTLrender="));
			DEBUG_PRT.println(bRTLrender?F("true"):F("false"));
			
			tb.add(*xpos, *ypos, s_substr, colormap_substr, render_right_to_left, bRTLrender, diskfont);
		
			*xpos += textwidth;

			if (bInsertEllipsis && Bidi::strEllipsis.length() > 0) {
				// insert the ellipsis at the current xpos, ypos
				tb.add(*xpos, *ypos, Bidi::strEllipsis, GxEPD_BLACK, render_right_to_left, bRTLrender, diskfont);		
				*xpos += ellipsiswidth;
			}
		}

		if (bLineBreak) {
			if (wrap_text) {
				if (!bLastLine) { 
					DEBUG_PRT.println("[linebreak]");
					*ypos += diskfont._FontHeader.charheight; // * 2;
					*xpos = 0;
				}
				else if (!bInsertEllipsis && Bidi::strEllipsis.length() > 0 && bMoreText){ // if bInsertEllipsis is true, then have already inserted the ellipsis, no need to to it twice
					tb.add(*xpos, *ypos, Bidi::strEllipsis, GxEPD_BLACK, render_right_to_left, bRTLrender, diskfont);		
					*ypos += diskfont._FontHeader.charheight; // * 2;		// this will ensure that the next line overflows the display, so ending output
					*xpos = 0;
				}
			}
			else {
				return true; // overflowed line
			}
		}

		if ((bNewLine && !bMakeLineBreakBefore) || (bNewLine && !bIsLastFragment)) { 
			if (wrap_text) {
				//DEBUG_PRT.println("[newline]");
				*ypos += diskfont._FontHeader.charheight;
				*xpos = 0;
			}
			else {
				return true; // overflowed line
			}
		}

		startstrpos = endstrpos;
		
		DEBUG_PRT.print(F("=\n\n"));
	}
	
	*bEmphasisOn = bEmphasisOnAtEnd;
	
	if (*ypos >= fbheight || (bLastLine && bMoreText)) return true; // will return true if the text overflows the screen
	
	return false;						// otherwise will return false if there is more space

}

