#include "Bidi.h"	
#include <ctype.h>

#include "Tridentine.h"
#include "Yml.h"
#include "RCConfig.h"
#include "Csv.h"

//#undef DEBUG_PRT
//#define DEBUG_PRT Serial
	
//Bidi::Bidi() {
//}

int Bidi::FindFirstSpacelikeCharacter(String s, int startpos) {
	uint32_t spacechars[15] = {0x0020, /*0x003C is '<', for matching opening tags*/0x003C, 0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x3000};
	
	int firstspacetypecharpos = s.length();
	bool bFoundSpacetypeChar = false;
	
	for (int i = 0; i < 15; i++) {
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


bool Bidi::IsSpace(String& ch, bool bAlsoMatchTagOpeningBracket) {
	String spacechar = "";
	return IsSpace(ch, spacechar, bAlsoMatchTagOpeningBracket);
}

bool Bidi::IsSpace(String& ch, String& foundspacechar, bool bAlsoMatchTagOpeningBracket) {
	foundspacechar = "";
	
	uint32_t code = codepointUtf8(ch);
	bool bFoundSpaceChar = false;

	switch(code)
	{
		case 0x003C: // PLL 07-07-2020 this is '<', opening tag, which we'll treat as a word boundary
			if (bAlsoMatchTagOpeningBracket) {
				bFoundSpaceChar	= true;
			}
			break;

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
			bFoundSpaceChar = true;
			break;

		default:
			return false;
	}

	if (bFoundSpaceChar) {
		foundspacechar = ch;
	}
	return bFoundSpaceChar;
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

bool Bidi::ExpectStr(String& s, int* pos, String strtoexpect, bool bCaseSensitive) {
	// made case insensitive
	//DEBUG_PRT.print(F("ExpectStr() pos = "));
	//DEBUG_PRT.print(*pos);
	//DEBUG_PRT.print(F(" strtoexpect = "));
	//DEBUG_PRT.println(strtoexpect);

	bool bMatched = true;
	int stringindex = 0;
	int checkstrlen = strtoexpect.length();

	if (*pos + checkstrlen > s.length()) { // if too few chars remaining in string s for a match
		return false;
	}

	if (!bCaseSensitive) { // case insensitivity is basic - this is really for tags, containing only A-Za-z chars (not extended Latin chars)
		while(bMatched && stringindex < checkstrlen) {
			////DEBUG_PRT.print(F("."));
			//DEBUG_PRT.println(F("si = "));
			//DEBUG_PRT.print(stringindex);
			//DEBUG_PRT.print(F(" chars: "));
			//DEBUG_PRT.print(toupper(s.charAt(*pos + stringindex)));
			//DEBUG_PRT.print(F(" - "));
			//DEBUG_PRT.println(toupper(strtoexpect.charAt(stringindex)));
			if (toupper(s.charAt(*pos + stringindex)) != toupper(strtoexpect.charAt(stringindex))) {
				bMatched = false;
			}
			else {
				stringindex++;
			}
		}
	}
	else {
		while(bMatched && stringindex < checkstrlen) {
			DEBUG_PRT.print(F(","));
			if (s.charAt(*pos + stringindex) != strtoexpect.charAt(stringindex)) {
				bMatched = false;
			}
			else {
				stringindex++;
			}
		}
	}

	if (bMatched) {
		DEBUG_PRT.print(F("ExpectStr() matched, old *pos = "));
		DEBUG_PRT.print(*pos);
		DEBUG_PRT.print(F(" checkstrlen = "));
		DEBUG_PRT.print(checkstrlen);
		DEBUG_PRT.print(F(" strtoexpect = ["));
		DEBUG_PRT.print(strtoexpect);
		DEBUG_PRT.print(F("] new *pos = "));
		*pos += checkstrlen;
		DEBUG_PRT.println(*pos);
	}

	return bMatched;

	//if (s.indexOf(strtoexpect, *pos) == *pos) {
	//	*pos += strtoexpect.length();
	//	return true;
	//}
	//return false;
}

bool Bidi::ExpectLineBreakTag(String& s, int* pos) {
// return true and advance the string index *pos when either <i> <b> </i> or </b> tags are found. *pos will be advanced past the last closing > in the tag
// *Emphasis_On will be left unchanged if no emphasis tag is found at the string starting in position *pos
// In this way, Emphasis_On can persist when the closing tag is found in a later call to the function.
	//DEBUG_PRT.printf("ExpectEmphasisTag: str at pos=%s\n", s.substring(*pos, *pos + 5).c_str());

	if (ExpectStr(s, pos, "<br>"))  return true; 
	if (ExpectStr(s, pos, "<br/>")) return true; 
	//if (ExpectStr(s, pos, "<BR>"))  return true; 
	//if (ExpectStr(s, pos, "<BR/>")) return true; 
	
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
	
	// check if an emphasis tag is present in the string starting at pos 
	if (ExpectLineBreakTag(s, startstrpos)) { // if so, skip the tag by changing the _start_ pos of the string to the first character after 
		if (*bNewLine == false) {
			DEBUG_PRT.println(F("found linebreak tag"));

			*bLineBreak = true;		   // signal a linebreak
			*endstrpos = *startstrpos; // the tag, and set the end pos of the scanned region to be the as the new start position (after the tag!).
			*textwidth = 0;			   // the textwidth of the scanned text must be 0, since no more text has been scanned, and the tag is skipped.
			
			return;
		}
		else {
			// on entry here, *bNewLine should be still set to the value it was left at after the last call to this function by GetString()
			DEBUG_PRT.println(F("found linebreak tag immediately following generated newline, suppressing (to prevent blank line)"));

			*endstrpos = *startstrpos; // skip over the <br> tag
			*bNewLine = false;		   // reset NewLine flag
			*bLineBreak = false;	   // clear LineBreak tag (is usually set when a <br> is scanned in input string, but not if directly after a generated newline)
			*textwidth = 0;			   // set textwidth to 0, since a newline has already occurred if we're in here
			return;
		}
	}
	
	*bNewLine = false; // now *bNewLine is reset
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
	String next_ch = "";

	int pos_start_spaceordot_start = -1;	// saves start and end pos of runs of dots and/or spaces, which attach to the word ahead or behind, depending on the reading order
	int pos_start_spaceordot_end = -1;
	
	bool bLineBreakTagFound = false;
	
	int prevwordendstrpos = pos;
	double prevwordendxwidth = (int)dcurrwidth;
	int prevwordcount = wordcount;
	
	int currxpos = xpos;
	int lastxpos = xpos;
	
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
		int cl = ch.length();
		
		if ((pos + cl) > s.length()) {
			next_ch = "";
		}
		else {
			next_ch = utf8CharAt(s, pos + cl);
		}


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
		
		if (IsSpace(ch) || (ch == "." && (next_ch != "'" && next_ch != "\"")) || (last_ch == "." && (ch == "'" || ch == "\""))) { // space char is special case, inherits the reading direction of the character preceding it. Also period (.), which sometimes (it appears) is used in RTL text				
		
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
			
			currxpos = ((int)dcurrwidth) + xpos;
			lastxpos = lastwordendxwidth + xpos;
			
			if (bLastLine) {
				//debugging
				DEBUG_PRT.print(F("Ellipsis cases 1 & 2: lastxpos="));
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
				//debugging
				
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
					DEBUG_PRT.println(F(" - Ellipsis case 1"));
					continue;
				}

				if (lastxpos <= (fbwidth - ellipsiswidth) && currxpos > (fbwidth - ellipsiswidth) && (pos < s.length() || bMoreText) ) { 
					// don't print the last word if so (the previous word does not overflow the ellipsis area, but this word does (but is within the display), but there is more text to display (which will not be as there is no room, so need to leave room for the ellipsis)
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
					DEBUG_PRT.println(F(" - Ellipsis case 2"));
					continue;
				}
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

		if (bLastLine) {
			//debugging
			DEBUG_PRT.print(F("Ellipsis cases 3 - 6: lastxpos="));
			DEBUG_PRT.print(lastxpos);
			DEBUG_PRT.print(F(", fbwidth="));
			DEBUG_PRT.print(fbwidth);
			DEBUG_PRT.print(F(", dcurrwidth="));
			DEBUG_PRT.print((int)dcurrwidth);
			DEBUG_PRT.print(F(", ellipsiswidth="));
			DEBUG_PRT.print(ellipsiswidth);
			DEBUG_PRT.print(F(", currxpos="));
			DEBUG_PRT.print(currxpos);
			DEBUG_PRT.print(F(", pos="));
			DEBUG_PRT.print(pos);
			DEBUG_PRT.print(F(", bMoreText="));
			DEBUG_PRT.println(bMoreText);		
			//debugging

			if (lastxpos > (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= fbwidth)) { 
				// don't print last two words if so (this word overflows the display and the previous word overflows the ellipsis area, and this is the last line)
				lastwordendstrpos = prevwordendstrpos;
				lastwordendxwidth = prevwordendxwidth;
				wordcount = prevwordcount;
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 3"));
				continue;
			}

			if (lastxpos <= (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= fbwidth)) { 
				// don't print the last word if so (this word overflows the display, but the previous word does not overflow the ellipsis area)
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 4"));
				continue;
			}

			// ellipsis cases 3 and 4 may be bugged (fbwidth instead of dmaxwidth - dcurrwidth and dmaxwidth are relative to the line space remaining since the last call, not the entire line width)

			if (lastxpos > (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= dmaxwidth)) { 
				// don't print last two words if so (this word overflows the display and the previous word overflows the ellipsis area, and this is the last line)
				lastwordendstrpos = prevwordendstrpos;
				lastwordendxwidth = prevwordendxwidth;
				wordcount = prevwordcount;
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 5"));
				continue;
			}

			if (lastxpos <= (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= dmaxwidth)) { 
				// don't print the last word if so (this word overflows the display, but the previous word does not overflow the ellipsis area)
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 6"));
				continue;
			}
		}
	}

	if (dcurrwidth >= dmaxwidth || bForceLineBreakAfterString) { // if the next word after lastwordendstrpos overflowed the line, tell the caller to generate a cr/newline (after rendering the text between *startstrpos and *endstrpos).
		DEBUG_PRT.print(F("GetString() Newline: dcurrwidth="));
		DEBUG_PRT.print((int)dcurrwidth);
		DEBUG_PRT.print(F(" dmaxwidth=")); 
		DEBUG_PRT.println((int)dmaxwidth);
		
		*bNewLine = true;
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
									   /*String& colormap, */
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
	
	//if (ExpectEmphasisTag(s, startstrpos)) return false;	// return if it's an html tag
	if (s.charAt(startstrpos) == '<') return false;	// PLL 07-07-2020 return if it's an html tag
	
	//while (ExpectStr(s, &startstrpos, " ")); // skip over any leading spaces (adding spaces to create a break in earlier calls to this function will lengthen the string, so the first char on the next call may be the inserted space, perhaps followed by others which were already in the text).
	
	while (ExpectSpace(s, &startstrpos)); // skip over any leading unicode spaces (all types, not just 0x20)
	
	////TODO: PLL 07-07-2020 What if there's another tag (or series of tags) starting here?

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
	
	//String colormap_word = colormap.substring(startstrpos, endstrpos);
	
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
		} 
		else {
			DEBUG_PRT.println(F("bLastLine is false"));
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
														   /*colormap,*/
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
		// BUG 20-04-2020: 
		//      If the last verse output ends on the end of the last line of text that *can* be printed, and there is another verse to display, bMakeLineBreakBefore
		//      will be set by the call (above) to FixNextWordWiderThanDisplay, but no check is made when the following four lines of code implement a CR/LF. So the
		//      text of the following verse will be wrongly displayed in the very bottom line, which is reserved for the date and feast day.
		if (bMakeLineBreakBefore) {
			if (!bLastLine) {
				*ypos += diskfont._FontHeader.charheight;
				*xpos = 0;
			}
		}
		
		//DEBUG_PRT.printf("bOverflowWordWrapped=%s bIsLastFragment=%s bMakeLineBreakBefore=%s\n", bOverflowWordWrapped?"true":"false", bIsLastFragment?"true":"false", bMakeLineBreakBefore?"true":"false");
				
		GetString(s, &startstrpos, &endstrpos, &textwidth, diskfont, 
				  &bLineBreak, &bRTL, &bDirectionChanged, 
				  &bNewLine, 
				  false,	// bOneWordOnly - may be deprecated?
				  false,	// bForceLineBreakAfterString - may be deprecated?
				  forcedBreakPos,	// index of character where forced break must occur, or -1 if not needed (needed for words wider than the display)
				  fbwidth, 
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
	
	//if ((*ypos >= fbheight) || (bLastLine && bMoreText && *xpos >= fbwidth)) return true; // will return true if the text overflows the screen
	if ((*ypos >= fbheight) || (bLastLine && bMoreText && (bInsertEllipsis || *xpos >= fbwidth))) return true; // will return true if the text overflows the screen
	
	return false;						// otherwise will return false if there is more space

}



bool Bidi::RenderTextEx(String& s, 
				        int* xpos, int* ypos, 
					    TextBuffer& tb, 
					    DiskFont* pDiskfont,			// diskfont currently being used (persists over lines and multiple calls to this function)
					    DiskFont& diskfont_normal,		// diskfont for normal text
					    DiskFont& diskfont_i,			// normal text plus italic
					    DiskFont& diskfont_plus1_bi,	// +1 size bold italic text
					    DiskFont& diskfont_plus2_bi,	// +2 size bold italic text
					    bool& bBold,
					    bool& bItalic, 
					    bool& bRed, 
					    int8_t& fontsize_rel,			// supported 0, +1 or +2
					    int8_t& line_number, 
					    int fbwidth, int fbheight, 
					    bool render_right_to_left,
					    bool wrap_text,
					    bool bMoreText, 				// PLL 26-12-2018 bMoreText set to true if this is not the last string in the block to be drawn
						uint8_t format_action)			// either TB_FORMAT_NONE, TB_FORMAT_JUSTIFY, TB_FORMAT_CENTRE
{
	// RenderTextEx version which does not return the line height
	int16_t last_line_height = 0;

	return RenderTextEx(s, xpos, ypos, tb, pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
			 	        bBold, bItalic, bRed, fontsize_rel, line_number, fbwidth, fbheight, 
			            render_right_to_left, wrap_text, bMoreText, last_line_height, format_action);
}

// render bidi text using disk font
bool Bidi::RenderTextEx(String& s, 
				        int* xpos, int* ypos, 
					    TextBuffer& tb, 
					    DiskFont* pDiskfont,			// diskfont currently being used (persists over lines and multiple calls to this function)
					    DiskFont& diskfont_normal,		// diskfont for normal text
					    DiskFont& diskfont_i,			// normal text plus italic
					    DiskFont& diskfont_plus1_bi,	// +1 size bold italic text
					    DiskFont& diskfont_plus2_bi,	// +2 size bold italic text
					    bool& bBold,
					    bool& bItalic, 
					    bool& bRed, 
					    int8_t& fontsize_rel,			// supported 0, +1 or +2
					    int8_t& line_number, 
					    int fbwidth, int fbheight, 
					    bool render_right_to_left,
					    bool wrap_text,
					    bool bMoreText, 				// PLL 26-12-2018 bMoreText set to true if this is not the last string in the block to be drawn
					    int16_t& last_line_height, 			// will return the result from the last call to Textbuffer::typeset (the line height of the tallest line (in pixels))
						uint8_t format_action)			// either TB_FORMAT_NONE, TB_FORMAT_JUSTIFY, TB_FORMAT_CENTRE
{
	//if (diskfont.available == false) return true; // if no diskfont is available, return true to stop caller from trying to output more text (is used to indicate screen full)

	last_line_height = 0;
	
	bool bNewLine = false;
	int startstrpos = 0;
	int endstrpos = 0;
	int textwidth = 0;
	bool bLineBreak = false;

	bool bTagFound = false; //PLL 06-07-2020

	DEBUG_PRT.print(F("1.."));
	String alshapedtext = "";
	alshapedtext.reserve(s.length());
	int level = ArabicLigaturizer::ar_nothing; //ArabicLigaturizer::ar_composedtashkeel | ArabicLigaturizer::ar_lig | ArabicLigaturizer::DIGITS_EN2AN;
	ArabicLigaturizer::Shape(s, alshapedtext, level);
	s = alshapedtext;
	DEBUG_PRT.print(F("2.."));	
	bool bRTL = IsRightToLeftChar(utf8CharAt(s, 0)); // set initial state for RTL flag
	bool bRTLrender = bRTL; // bRTL must persist since it is modified and used in GetString. Hence a copy is used for rendering, inverted in value if the rendering direction is RTL 
							// (bRTLrender selects whether the text fragment being drawn is to be embedded reversed, eg. numbers in Arabic text).
	bool bDirectionChanged = true; // at the start of the string, want to force GetString to check which direction the text is reading, otherwise inherit it from the preceding text
	bool bLastLine = false;
	DEBUG_PRT.print(F("3.."));
	int ellipsiswidth = (int)pDiskfont->GetTextWidthA(Bidi::strEllipsis);
	DEBUG_PRT.print(F("4.."));
	
	bool bInsertEllipsis = false;
	bool bOverflowWordWrapped = false;
	int last_xpos = 0;
	int last_ypos = 0;
	int curr_xpos = 0;
	int curr_ypos = 0;

	//String colormap = "";
	//colormap.reserve(s.length());
	//
	//bool bEmphasisOnAtEnd = *bEmphasisOn;
	//getColorMap(s, bEmphasisOnAtEnd, colormap);
	//StripEmphasisTagsOnly(s);
	//*bEmphasisOn = bEmphasisOnAtEnd;
	
	int charsprocessedbyoutsizestringhandler = 0;
	int numcharslefttoprocess = 0;
	bool bIsLastFragment = false;
	bool bMakeLineBreakBefore = false;
	
	//DEBUG_PRT.print(F("\nBidi::RenderTextEx(): str  =["));
	//DEBUG_PRT.print(s);
	//DEBUG_PRT.println(F("]"));
	//
	//DEBUG_PRT.print(F("Bidi::RenderTextEx(): cmap =["));
	//DEBUG_PRT.print(colormap);
	//DEBUG_PRT.println(F("]"));

	//String colormap_substr = "";
	//String s_substr = "";
	//
	//colormap_substr.reserve(s.length());
	//s_substr.reserve(s.length());

	bool bDisplayEllipsisNow = false;

	DEBUG_PRT.print(F("5.."));
	if (ParseTags(s, &startstrpos, bBold, bItalic, bRed, /*bLineBreak,*/ fontsize_rel)) {
		DEBUG_PRT.print(F("Select Diskfont.."));
		pDiskfont = SelectDiskFont(bBold, bItalic, fontsize_rel, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
		ellipsiswidth = (int)pDiskfont->GetTextWidthA(Bidi::strEllipsis);
		DEBUG_PRT.print(F("Done.."));
	}
	DEBUG_PRT.print(F("6.."));
	int font_ascent = (int)pDiskfont->_FontHeader.charheight;
	DEBUG_PRT.println(F("7"));
	
	DEBUG_PRT.print(F("*ypos ="));
	DEBUG_PRT.print(*ypos);
	DEBUG_PRT.print(F(", font_ascent ="));
	DEBUG_PRT.print(font_ascent);
	DEBUG_PRT.print(F(", fbheight ="));
	DEBUG_PRT.print(fbheight);
	DEBUG_PRT.print(F(", (*ypos + font_ascent) < fbheight ="));
	DEBUG_PRT.println((*ypos + font_ascent) < fbheight);

	while ((*ypos + font_ascent) < fbheight && endstrpos < s.length() && !bDisplayEllipsisNow) {
		bLastLine = (wrap_text && (*ypos + font_ascent < fbheight && *ypos + (font_ascent * 2) >= fbheight)); //only worry about last line if wrapping text
		
		DEBUG_PRT.print(F("wrap_text="));
		DEBUG_PRT.print(wrap_text);
		DEBUG_PRT.print(F(" *ypos="));
		DEBUG_PRT.print(*ypos);
		DEBUG_PRT.print(F(" fbheight="));
		DEBUG_PRT.print(fbheight);
		DEBUG_PRT.print(F(" font_ascent="));
		DEBUG_PRT.print(font_ascent);
		DEBUG_PRT.print(F(" *ypos + font_ascent="));
		DEBUG_PRT.print(*ypos + font_ascent);
		DEBUG_PRT.print(F(" font_ascent * 2 + *ypos="));
		DEBUG_PRT.println(font_ascent * 2 + *ypos);
		
		if (bLastLine) {
			DEBUG_PRT.println(F("bLastLine is true"));
		} 
		else {
			DEBUG_PRT.println(F("bLastLine is false"));
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
														   /*colormap,*/
														   charsprocessedbyoutsizestringhandler,
														   numcharslefttoprocess,
														   bIsLastFragment,
														   bMakeLineBreakBefore,
														   &forcedBreakPos,
														   startstrpos, 
														   fbwidth, 
														   *xpos, //last_xpos + textwidth, 
														   render_right_to_left, 
														   *pDiskfont);
														   
		//DEBUG_PRT.printf("bOverflowWordWrapped = %s\n", bOverflowWordWrapped ? "true" : "false");			
		//DEBUG_PRT.println("String s is [" + s + "]");

		//DEBUG_PRT.printf("Bidi::RenderText() bMoreText=%s\n", bMoreText ? "true" : "false");
		DEBUG_PRT.print(F("Bidi::RenderTextEx() bMoreText="));
		DEBUG_PRT.println(bMoreText ? F("true") : F("false"));
		
		//if(bOverflowWordWrapped) {
		//	ExpectStr(s, &startstrpos, " "); // skip over the leading space added by FixNextWordWiderThanDisplay()
		//}

		//DEBUG_PRT.printf("bMakeLineBreakBefore=%s\n", bMakeLineBreakBefore?"true":"false");
		if (bMakeLineBreakBefore) {
			if (!bLastLine) {
				*ypos += pDiskfont->_FontHeader.charheight;
				*xpos = 0;
			}
		}
		
		DEBUG_PRT.print(F("bOverflowWordWrapped="));
		DEBUG_PRT.println(bOverflowWordWrapped?F("true"):F("false"));
		DEBUG_PRT.print(F("bIsLastFragment="));
		DEBUG_PRT.println(bIsLastFragment?F("true"):F("false"));
		DEBUG_PRT.print(F("bMakeLineBreakBefore="));
		DEBUG_PRT.println(bMakeLineBreakBefore?F("true"):F("false"));

		DEBUG_PRT.print(F("*xpos = "));
		DEBUG_PRT.print(*xpos);
		DEBUG_PRT.println(F(", calling GetString2().."));

		GetString2(s, &startstrpos, &endstrpos, &textwidth, *pDiskfont, 
				  &bLineBreak, &bRTL, &bDirectionChanged, 
				  &bNewLine, 
				  &bTagFound, // PLL 06-07-2020: Will be set by function if an opening '<' is found whilst scanning string s that is a tag other than <br>
				  false,	// bOneWordOnly - may be deprecated?
				  false,	// bForceLineBreakAfterString - may be deprecated?
				  forcedBreakPos,	// index of character where forced break must occur, or -1 if not needed (needed for words wider than the display)
				  fbwidth, 
				  *xpos, 
				  wrap_text,
				  bLastLine,
				  ellipsiswidth,
				  &bDisplayEllipsisNow,
				  bMoreText); 

		DEBUG_PRT.println(F("Returned from GetString2():"));

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

		DEBUG_PRT.print(F("bInsertEllipsis="));
		DEBUG_PRT.print(bInsertEllipsis);
		DEBUG_PRT.print(F(", textwidth="));
		DEBUG_PRT.println(textwidth);

		if (textwidth > 0) {
			bRTLrender = bRTL;
			
			if (render_right_to_left) {
				bRTLrender = !bRTLrender;
			} 
			
			uint16_t color = bRed ? GxEPD_RED : GxEPD_BLACK;			
			
			//colormap_substr = colormap.substring(startstrpos, endstrpos);
			//s_substr = s.substring(startstrpos, endstrpos);
			//StripTags(s_substr);
			
			DEBUG_PRT.print(F("Adding: ["));
			DEBUG_PRT.print(s.substring(startstrpos, endstrpos));
			DEBUG_PRT.println(F("]"));
			
			//DEBUG_PRT.print(F("        ["));
			//DEBUG_PRT.print(colormap_substr);
			//DEBUG_PRT.println("]");
			
			DEBUG_PRT.print(F("render_right_to_left="));
			DEBUG_PRT.print(render_right_to_left?F("true"):F("false"));
			DEBUG_PRT.print(F(", bRTLrender="));
			DEBUG_PRT.println(bRTLrender?F("true"):F("false"));
			
			tb.add_buffered(*xpos, *ypos, s.substring(startstrpos, endstrpos), color, render_right_to_left, bRTLrender, *pDiskfont, line_number);
		
			*xpos += textwidth;

			if (bInsertEllipsis && Bidi::strEllipsis.length() > 0) {
				DEBUG_PRT.println(F("Adding Ellipsis (1)"));
				// insert the ellipsis at the current xpos, ypos
				tb.add_buffered(*xpos, *ypos, Bidi::strEllipsis, GxEPD_BLACK, render_right_to_left, bRTLrender, *pDiskfont, line_number);		
				*xpos += ellipsiswidth;
			}

			if (!bMoreText && !wrap_text) {
				int16_t advanceheight = tb.typeset(line_number, fbwidth, format_action);
				last_line_height = (advanceheight > last_line_height) ? advanceheight : last_line_height;
			}
		}

		if (bLineBreak) {
			if (wrap_text) {
				if (!bLastLine) { 
					DEBUG_PRT.println("[linebreak]");
					
					int16_t advanceheight = tb.typeset(line_number, fbwidth, format_action);
					font_ascent = (int)advanceheight;
					last_line_height = (advanceheight > last_line_height) ? advanceheight : last_line_height;

					if (bMoreText && *ypos + font_ascent < fbheight && *ypos + (font_ascent * 2) >= fbheight) {
						// Last line if so. Last line on display in this case is a linebreak, so display ellipsis now before doing it if there is still more text available
						DEBUG_PRT.println(F("Adding Ellipsis (2)"));
						tb.add(*xpos, *ypos, Bidi::strEllipsis, GxEPD_BLACK, render_right_to_left, bRTLrender, *pDiskfont, line_number);		

						advanceheight = tb.typeset(line_number, fbwidth, format_action);
						return true; // the ellipsis is the last thing to be displayed
					}

					//*ypos += diskfont._FontHeader.charheight; // * 2;
					*ypos += advanceheight;
					*xpos = 0;
					line_number += 1;
				}
				else if (!bInsertEllipsis && Bidi::strEllipsis.length() > 0 && bMoreText){ // if bInsertEllipsis is true, then have already inserted the ellipsis, no need to to it twice
					DEBUG_PRT.println(F("Adding Ellipsis (3)"));
					tb.add(*xpos, *ypos, Bidi::strEllipsis, GxEPD_BLACK, render_right_to_left, bRTLrender, *pDiskfont, line_number);		
					
					int16_t advanceheight = tb.typeset(line_number, fbwidth, format_action);
					font_ascent = (int)advanceheight;
					last_line_height = (advanceheight > last_line_height) ? advanceheight : last_line_height;

					//*ypos += diskfont._FontHeader.charheight; // * 2;		// this will ensure that the next line overflows the display, so ending output
					*ypos += advanceheight;
					*xpos = 0;
					line_number += 1;
					return true; // nothing comes after the ellipsis, so return true/overflowed screen
				}
			}
			else {
				return true; // overflowed line
			}
		}

		if ((bNewLine && !bMakeLineBreakBefore) || (bNewLine && !bIsLastFragment)) { 
			if (wrap_text) {
				//DEBUG_PRT.println("[newline]");
				int16_t advanceheight = tb.typeset(line_number, fbwidth, format_action);
				font_ascent = (int)advanceheight;
				last_line_height = (advanceheight > last_line_height) ? advanceheight : last_line_height;

				*ypos += advanceheight;
				//*ypos += diskfont._FontHeader.charheight;
				*xpos = 0;
				line_number += 1;
			}
			else {
				return true; // overflowed line
			}
		}

		startstrpos = endstrpos;

		if (bTagFound) {
			if (ParseTags(s, &startstrpos, bBold, bItalic, bRed, /*bLineBreak,*/ fontsize_rel)) {
				pDiskfont = SelectDiskFont(bBold, bItalic, fontsize_rel, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
				ellipsiswidth = (int)pDiskfont->GetTextWidthA(Bidi::strEllipsis);// recalculate ellipsiswidth for this font, should it be needed
				endstrpos = startstrpos;	// ParseTags call will move startstrpos to skip over processed tags
			}
			font_ascent = (int)pDiskfont->_FontHeader.ascent;
		}
		
		DEBUG_PRT.print(F("=\n\n"));
	}
	
	if (!bMoreText && endstrpos == s.length()) {	// typeset last bit of text
		DEBUG_PRT.println(F("(Typesetting remaining text)"));
		int16_t advanceheight = tb.typeset(line_number, fbwidth, format_action);
		last_line_height = (advanceheight > last_line_height) ? advanceheight : last_line_height;
	}

	//*bEmphasisOn = bEmphasisOnAtEnd;
	
	//if ((*ypos >= fbheight) || (bLastLine && bMoreText && *xpos >= fbwidth)) return true; // will return true if the text overflows the screen
	
	DEBUG_PRT.print(F("RenderTextEx() Dropped through, fbheight="));
	DEBUG_PRT.print(fbheight);
	DEBUG_PRT.print(F(", *ypos="));
	DEBUG_PRT.print(*ypos);
	DEBUG_PRT.print(F(", bLastLine="));
	DEBUG_PRT.print(bLastLine);
	DEBUG_PRT.print(F(", bMoreText="));
	DEBUG_PRT.print(bMoreText);
	DEBUG_PRT.print(F(", bInsertEllipsis="));
	DEBUG_PRT.print(bInsertEllipsis);
	DEBUG_PRT.print(F(", fbwidth="));
	DEBUG_PRT.print(fbwidth);
	DEBUG_PRT.print(F(", *xpos="));
	DEBUG_PRT.println(*xpos);
	
	if ((*ypos >= fbheight) || (bLastLine && bMoreText && (bInsertEllipsis || *xpos >= fbwidth))) return true; // will return true if the text overflows the screen
	
	return false;						// otherwise will return false if there is more space

}

DiskFont* Bidi::SelectDiskFont(bool bBold, bool bItalic, int8_t fontsize_rel, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi) {
	DEBUG_PRT.print(F("SelectDiskFont(): bBold = "));
	DEBUG_PRT.print(bBold);
	DEBUG_PRT.print(F(" bItalic = "));
	DEBUG_PRT.print(bItalic);
	DEBUG_PRT.print(F(" fontsize_rel = "));
	DEBUG_PRT.print(fontsize_rel);
	DEBUG_PRT.print(F(", font selected = "));
	switch(fontsize_rel) {
		case 0:
			if (!bItalic) {
				DEBUG_PRT.println(F("normal"));
				return &diskfont_normal;
			}
			else {
				DEBUG_PRT.println(F("italic"));
				return &diskfont_i;
			}
			break;
		
		case 1:
			DEBUG_PRT.println(F("+1 bold italic"));
			return &diskfont_plus1_bi;
			break;

		case 2:
			DEBUG_PRT.println(F("+2 bold italic"));
			return &diskfont_plus2_bi;
			break;
		
		default:
			DEBUG_PRT.println(F("normal (default)"));
			return &diskfont_normal;
			break;
	}
	
	DEBUG_PRT.println(F("normal (fell through)"));
	return &diskfont_normal;
}

bool Bidi::ParseTags(String& s, int* pos, bool& bBold, bool& bItalic, bool& bRed, /*bool& bLineBreak,*/ int8_t& fontsize_rel) {
	uint16_t tag_count = 0;
	uint16_t last_tagcount = 0;

	// Handle all supported tags and closing tags. On entry, *pos should be the index of the opening '<' of a tag found in scanning string s.
	DEBUG_PRT.print(F("ParseTags():"));
	while (s.charAt(*pos) == '<') {
		last_tagcount = tag_count; // must process at least one tag per iteration
		
		int pos2 = *pos;
		if (ExpectLineBreakTag(s, &pos2)) {	// test for next tag = <br> or <br/> (case insensitive), without bumping *pos, so the caller can deal with
			break;							// the linebreak tag
		}

		//	DEBUG_PRT.println(F("(no '<')"));
		//	return false; // check quickly to see if a tag is present, return if not
		

		// closing tag (any)
		DEBUG_PRT.print(F("A"));
		if (ExpectStr(s, pos, "</")) { // Closing tag found - just assume they're in the right order, for speed don't check the closing tag name
			int cltag_end = s.indexOf(">", *pos);
			if (cltag_end != -1 && (cltag_end - *pos) < 10) {		// Sanity check: limit closing tag length to 10 characters plus the </ at the start.
				*pos = cltag_end + 1;
				tageffect_pop(bBold, bItalic, bRed, fontsize_rel);
				tag_count++;
			}
		}

		// won't search for linebreak tags here, just font styles/colour
		//bLineBreak = false;

		// Linebreak tag
		//if (ExpectStr(s, pos, "<BR>")) { 
		//	bLineBreak = true; 
		//	return true; 
		//}

		// Italic tag
		DEBUG_PRT.print(F("B"));
		if (ExpectStr(s, pos, "<I>")) {
			tageffect_push(bBold, true, bRed, fontsize_rel);
			bItalic = true;
			tag_count++;
		}

		// Bold tag
		DEBUG_PRT.print(F("C"));
		if (ExpectStr(s, pos, "<B>")) {
			tageffect_push(true, bItalic, bRed, fontsize_rel);
			bBold = true;
			tag_count++;
		}

		// Span tag, supports css <span style='color:red; font-size:1.25em'> only (for the Cross symbol).
		DEBUG_PRT.print(F("D"));
		if (ExpectStr(s, pos, "<span style='color:red; font-size:1.25em'>")) {
			fontsize_rel = fontsize_rel < 2 ? fontsize_rel + 1 : fontsize_rel; // make the font 1pt larger for 1.25em
			bRed = true;
			tageffect_push(bBold, bItalic, bRed, fontsize_rel);		
			tag_count++;
		}

		// Font tag, supports COLOR="red" and SIZE=+/-n
		DEBUG_PRT.print(F("E"));
		if (ExpectStr(s, pos, "<FONT")) {
			int tag_endpos = s.indexOf(">", *pos);
			
			String attrstr = "SIZE=";
			
			int si = s.indexOf(attrstr, *pos);
			if (si != -1 && si < tag_endpos) {
				// now look for a size, eg -1 or +2		
				si += attrstr.length();
				int ispace = s.indexOf(" ", si);
				int ibracket = s.indexOf(">", si);
				DEBUG_PRT.print(F(" *pos = "));
				DEBUG_PRT.print(*pos);
				DEBUG_PRT.print(F(" si = "));
				DEBUG_PRT.print(si);
				DEBUG_PRT.print(F(" ispace = "));
				DEBUG_PRT.print(ispace);
				DEBUG_PRT.print(F(" ibracket = "));
				DEBUG_PRT.print(ibracket);

				char fs_sign = s.charAt(si++);
				
				fontsize_rel = s.substring(si, ispace < ibracket ? ispace : ibracket).toInt();
				if (fs_sign == '-') {
					fontsize_rel = -fontsize_rel;
				}

				DEBUG_PRT.print(F("fontsize_rel = ["));
				DEBUG_PRT.print(fontsize_rel);
				DEBUG_PRT.println(F("] "));
				
				//char fs_sign = s.charAt(si);
				//int si_end = si + 1;
				//if (fs_sign == '-' || fs_sign == '+') {
				//	char signordigit = s.charAt(si_end);
				//	while(signordigit != '>' && signordigit != ' ' && isDigit(signordigit)) {
				//		si_end++;
				//		signordigit = s.charAt(si_end);
				//	}
				//}
				//fontsize_rel = (s.substring(si, si_end - 1)).toInt();
			}

			attrstr = "COLOR";

			si = s.indexOf(attrstr, *pos);
			if (si != -1 && si < tag_endpos) {
				int col_si = s.indexOf("\"red\"", si + attrstr.length());	// a bit of a bodge, it will detect the word "red" (in double quotes) any where between the attribute "COLOR"
				if (col_si < tag_endpos) {									// and the end of the tag.
					bRed = true;
				} 
				else {
					bRed = false;
				}
			}

			*pos = tag_endpos + 1;
			tageffect_push(bBold, bItalic, bRed, fontsize_rel);
			tag_count++;
		}

		if (tag_count == last_tagcount) {
			// there was a tag, but it wasn't supported
			*pos = s.indexOf(">", *pos); // skip to the end of the tag, whatever it was, and continue
			if (*pos == -1) {
				*pos = s.length();
				break;
			}
			tag_count++;
		}
	}
	
	if (tag_count > 0) {
		DEBUG_PRT.print(F("(Found "));
		DEBUG_PRT.print(tag_count);
		DEBUG_PRT.println(F(" tags)"));
		return true;
	}

	DEBUG_PRT.println(F("(No tags found)"));
	return false;
}

bool Bidi::tageffect_push(bool bBold, bool bItalic, bool bRed, int8_t fontsize_rel) {
	// store the state of the font effect variables in a byte on the tageffect stack
	uint8_t val = 0;
	tageffect_peek(val);

	val |= 0x80;
	if (bBold) val |= 4;
	if (bItalic) val |= 2;
	if (bRed) val |= 1;
	
	val |= ((fontsize_rel & 0x7) << 3); // use low three bits, shifted up to bits 5,4,3

	if (fontsize_rel < 0) {
		val |= (1 << 6);
	}

	dump_style(bBold, bItalic, bRed, fontsize_rel);
	return tageffect_push(val);
}

bool Bidi::tageffect_pop(bool& bBold, bool& bItalic, bool& bRed, int8_t& fontsize_rel) {
	// recover the state of the font effect variables from the byte on top of the tageffect stack
	uint8_t val = 0;
	bool bPopped = tageffect_pop(val);

	if (bPopped) { // after, need to peek the top item on the stack to get the previous state of the font effects (before the present state was pushed)
		if (tageffect_peek(val, Bidi::tageffectstackindex)) {
			bBold = (val & 0x04) != 0;
			bItalic = (val & 0x02) != 0;
			bRed = (val & 0x01) != 0;
			fontsize_rel = ((val & 0x38) >> 3);
			fontsize_rel = (val & 0x40) == 0 ? fontsize_rel : -fontsize_rel;
			dump_style(bBold, bItalic, bRed, fontsize_rel);			
		}
		else {
			tageffect_reset(bBold, bItalic, bRed, fontsize_rel); // stack is empty
		}
		return true;
	}
	else {
		tageffect_reset(bBold, bItalic, bRed, fontsize_rel);
	}

	return false;
}

void Bidi::tageffect_reset(bool& bBold, bool& bItalic, bool& bRed, int8_t& fontsize_rel) {
	DEBUG_PRT.print(F("(tageffect_reset())"));
	bBold = false;
	bItalic = false;
	bRed = false;
	fontsize_rel = 0;
	dump_style(bBold, bItalic, bRed, fontsize_rel);
}

void Bidi::dump_style(bool bBold, bool bItalic, bool bRed, int8_t fontsize_rel) {
	DEBUG_PRT.print(F("[style: Font "));
	if (fontsize_rel > -1) {
		DEBUG_PRT.print(F("+"));
	}
	else {
		DEBUG_PRT.print(F("-"));
	}
	DEBUG_PRT.print(fontsize_rel);
	DEBUG_PRT.print(bBold ? F(" Bold ") : F(""));
	DEBUG_PRT.print(bItalic ? F(" Italic ") : F(""));
	DEBUG_PRT.print(bRed ? F(" Red ") : F(""));
	DEBUG_PRT.println(F("]"));
}

void Bidi::dump_stack() {
	DEBUG_PRT.print(F(" Stack ["));

	for (int16_t i = 0; i <= Bidi::tageffectstackindex; i++) {
		DEBUG_PRT.print(F("["));
		//DEBUG_PRT.print((uint8_t)tageffectstack.charAt(i), BIN);
		DEBUG_PRT.print(tageffectstack[i], BIN);
		DEBUG_PRT.print(F("]"));
	}
	DEBUG_PRT.println(F("]"));
}


// simple stack to store tag effects (Bold, Italic, font size + 0, 1 or 2, black or red text)
//	bit:  7   6    5   4   3   2  1   0
// byte: [x][fs-][fs3 fs2 fs1][B][I][b/r] 
// (1       	= set to 1 (the string class used for the stack treats a byte of 0 as a string terminator, this prevents it)
//  fs_sign    	= relative fontsize sign 1 = -, 
//  fs3 fs2 fs1 = relative font size, eg +1 or -1 (3 bit)
//  B       	= bold flag, 
//  I       	= italic flag, 
//  b/r     	= black/red flag)

//String Bidi::tageffectstack = "";
int16_t Bidi::tageffectstackindex = -1; // tageffectstackindex of -1 means the stack is empty
uint8_t Bidi::tageffectstack[TAGEFFECT_STACKSIZE];

void Bidi::tageffect_init() {
	DEBUG_PRT.print(F("(tageffect_init())"));
	//Bidi::tageffectstack = "";
	Bidi::tageffectstackindex = -1;
	for (int i = 0; i < TAGEFFECT_STACKSIZE; i++) {
		tageffectstack[i] = 0;
	}
	//Bidi::tageffectstack.reserve(TAGEFFECT_STACKSIZE);
}

bool Bidi::tageffect_push(uint8_t val) {
	DEBUG_PRT.print(F("(tageffect_push() val="));
	DEBUG_PRT.print(val);
	
	if (Bidi::tageffectstackindex + 1 == TAGEFFECT_STACKSIZE) {
		DEBUG_PRT.println(F("[stack full])"));
		return false;
	}

	Bidi::tageffectstackindex++; // stack index is pre-incremented, so in first case goes from -1 to 0 before storing the byte in val

	//Bidi::tageffectstack.setCharAt((unsigned int)Bidi::tageffectstackindex, val);
	Bidi::tageffectstack[Bidi::tageffectstackindex] = val;
	DEBUG_PRT.print(F(" stackindex = "));
	DEBUG_PRT.print(Bidi::tageffectstackindex);
	dump_stack();
	DEBUG_PRT.println(F(")"));
	return true;
}

bool Bidi::tageffect_pop(uint8_t& val) {
	DEBUG_PRT.print(F("(tageffect_pop() val="));
	if (Bidi::tageffectstackindex > -1) {
		//val = Bidi::tageffectstack.charAt((unsigned int)Bidi::tageffectstackindex);
		val = Bidi::tageffectstack[Bidi::tageffectstackindex];
		Bidi::tageffectstackindex--;
		DEBUG_PRT.print(val);
		dump_stack();
		DEBUG_PRT.println(F(")"));		
		return true;
	}
	DEBUG_PRT.print(F("[stack empty])"));
	return false;
}

bool Bidi::tageffect_peek(uint8_t& val) {
	return tageffect_peek(val, Bidi::tageffectstackindex);
}

bool Bidi::tageffect_peek(uint8_t& val, int16_t index) {
	DEBUG_PRT.print(F("(tageffect_peek() index = "));
	DEBUG_PRT.print(index);
	DEBUG_PRT.print(F(" val = "));
	if (index > -1 && index <= Bidi::tageffectstackindex) {
		//val = Bidi::tageffectstack.charAt((unsigned int)index);
		val = Bidi::tageffectstack[index];
		DEBUG_PRT.print(val);
		DEBUG_PRT.println(F(")"));		
		return true;
	}
	DEBUG_PRT.print(F("[stack empty])"));
	return false;
}


////////////

void Bidi::GetString2(String& s,
					 int* startstrpos, 
					 int* endstrpos, 
					 int* textwidth, 
					 DiskFont& diskfont, 
					 bool* bLineBreak,	 // this variable is set when either a <br> or <br/> tag is encountered, to tell the caller to skip a line before resuming text output
					 bool* bRTL,		 // this variable allows the state of the text block just scanned, right-to-left or left-to-right to be returned 
					 bool* bDirectionChanged, // this variable shows when the reading direction at the end of the text block just scanned has changed direction
					 bool* bNewLine,
					 bool* bTagFound,
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
	
	DEBUG_PRT.print(F("GetString2() xpos="));
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
	



	////TODO: call the ParseTags function whenever an opening '<' is encountered while scanning the string. Need also to modify
	////TODO: this code so that it uses the correct font when a change of style is encountered (Bold, Italic, +1bi or +2bi) (For the Latin Mass
	////TODO: html fragments I'm using, larger font text will always be bold italic, and usually red)

	*bTagFound = false; // will be set if an opening '<' is found (assumed to be start of an html tag).

	// check if an emphasis tag is present in the string starting at pos 
	if (ExpectLineBreakTag(s, startstrpos)) { // if so, skip the tag by changing the _start_ pos of the string to the first character after 
		if (*bNewLine == false) {
			DEBUG_PRT.println(F("found linebreak tag"));

			*bLineBreak = true;		   // signal a linebreak
			*endstrpos = *startstrpos; // the tag, and set the end pos of the scanned region to be the as the new start position (after the tag!).
			*textwidth = 0;			   // the textwidth of the scanned text must be 0, since no more text has been scanned, and the tag is skipped.
			
			return;
		}
		else {
			// on entry here, *bNewLine should be still set to the value it was left at after the last call to this function by GetString()
			DEBUG_PRT.println(F("found linebreak tag immediately following generated newline, suppressing (to prevent blank line)"));

			*endstrpos = *startstrpos; // skip over the <br> tag
			*bNewLine = false;		   // reset NewLine flag
			*bLineBreak = false;	   // clear LineBreak tag (is usually set when a <br> is scanned in input string, but not if directly after a generated newline)
			*textwidth = 0;			   // set textwidth to 0, since a newline has already occurred if we're in here
			return;
		}
	}
	
	*bNewLine = false; // now *bNewLine is reset
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
	String next_ch = "";

	int pos_start_spaceordot_start = -1;	// saves start and end pos of runs of dots and/or spaces, which attach to the word ahead or behind, depending on the reading order
	int pos_start_spaceordot_end = -1;
	
	bool bLineBreakTagFound = false;
	
	int prevwordendstrpos = pos;
	double prevwordendxwidth = (int)dcurrwidth;
	int prevwordcount = wordcount;
	
	int currxpos = xpos;
	int lastxpos = xpos;
	
	bool bOtherTagStartFound = false; 		//PLL 06-07-2020

	// now determine the length of the right to left or left to right string from the start character ch found at position pos
	while (  (pos < s.length()) 	 			 				// keep doing the following while: not at end of string
		  && (dcurrwidth < dmaxwidth || !wrap_text)  		    // and the width of the string in pixels doesn't exceed the remaining space, (or if text is not being wrapped)
		  /*&& (!*bLineBreak)*/ 									// and an linebreak tag has not been fount
		  && !bLineBreakTagFound
		  && !bOtherTagStartFound 		//PLL 06-07-2020
		  && (bCurrCharRightToLeft == bLookingForRightToLeft)	// and the reading direction hasn't changed
		  && (!(bOneWordOnly && wordcount == 1)) )  // and fewer than 1 word has been processed if the bOneWordOnly flag is set
	{	

		DEBUG_PRT.print(F("|"));

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
		int cl = ch.length();
		
		if ((pos + cl) > s.length()) {
			next_ch = "";
		}
		else {
			next_ch = utf8CharAt(s, pos + cl);
		}

		//PLL 06-07-2020
		if (ch == "<") {
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos
			bOtherTagStartFound = true;
			*bTagFound = true;

			DEBUG_PRT.println("(Found tag start)");
			continue;
		}
		//PLL 06-07-2020


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
		
		if (IsSpace(ch) || (ch == "." && (next_ch != "'" && next_ch != "\"")) || (last_ch == "." && (ch == "'" || ch == "\""))) { // space char is special case, inherits the reading direction of the character preceding it. Also period (.), which sometimes (it appears) is used in RTL text				
		
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
			
			currxpos = ((int)dcurrwidth) + xpos;
			lastxpos = lastwordendxwidth + xpos;
			
			if (bLastLine) {
				//debugging
				DEBUG_PRT.print(F("Ellipsis cases 1 & 2: lastxpos="));
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
				//debugging
				
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
					DEBUG_PRT.println(F(" - Ellipsis case 1"));
					continue;
				}

				if (lastxpos <= (fbwidth - ellipsiswidth) && currxpos > (fbwidth - ellipsiswidth) && (pos < s.length() || bMoreText) ) { 
					// don't print the last word if so (the previous word does not overflow the ellipsis area, but this word does (but is within the display), but there is more text to display (which will not be as there is no room, so need to leave room for the ellipsis)
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
					DEBUG_PRT.println(F(" - Ellipsis case 2"));
					continue;
				}
			}
			
			lastwordendstrpos = pos; // save this position as it is a word boundary
			lastwordendxwidth = (int)dcurrwidth; // and save the width in pixels of the string scanned up to position pos

			wordcount++;
			DEBUG_PRT.print(F("^"));
			//DEBUG_PRT.printf("[wc=%d, lwepos=%d, lwewid=%d]\n", wordcount, lastwordendstrpos, lastwordendxwidth);
			continue;
		}							

		dcurrwidth += diskfont.GetTextWidthA(ch);
		pos += ch.length();
		DEBUG_PRT.print(ch);

		if (bLastLine) {
			//debugging
			DEBUG_PRT.print(F("Ellipsis cases 3 - 6: lastxpos="));
			DEBUG_PRT.print(lastxpos);
			DEBUG_PRT.print(F(", fbwidth="));
			DEBUG_PRT.print(fbwidth);
			DEBUG_PRT.print(F(", dcurrwidth="));
			DEBUG_PRT.print((int)dcurrwidth);
			DEBUG_PRT.print(F(", ellipsiswidth="));
			DEBUG_PRT.print(ellipsiswidth);
			DEBUG_PRT.print(F(", currxpos="));
			DEBUG_PRT.print(currxpos);
			DEBUG_PRT.print(F(", pos="));
			DEBUG_PRT.print(pos);
			DEBUG_PRT.print(F(", bMoreText="));
			DEBUG_PRT.println(bMoreText);		
			//debugging

			if (lastxpos > (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= fbwidth)) { 
				// don't print last two words if so (this word overflows the display and the previous word overflows the ellipsis area, and this is the last line)
				lastwordendstrpos = prevwordendstrpos;
				lastwordendxwidth = prevwordendxwidth;
				wordcount = prevwordcount;
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 3"));
				continue;
			}

			if (lastxpos <= (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= fbwidth)) { 
				// don't print the last word if so (this word overflows the display, but the previous word does not overflow the ellipsis area)
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 4"));
				continue;
			}

			// ellipsis cases 3 and 4 may be bugged (fbwidth instead of dmaxwidth - dcurrwidth and dmaxwidth are relative to the line space remaining since the last call, not the entire line width)

			if (lastxpos > (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= dmaxwidth)) { 
				// don't print last two words if so (this word overflows the display and the previous word overflows the ellipsis area, and this is the last line)
				lastwordendstrpos = prevwordendstrpos;
				lastwordendxwidth = prevwordendxwidth;
				wordcount = prevwordcount;
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 5"));
				continue;
			}

			if (lastxpos <= (fbwidth - ellipsiswidth) && ((int)dcurrwidth >= dmaxwidth)) { 
				// don't print the last word if so (this word overflows the display, but the previous word does not overflow the ellipsis area)
				bLineBreakTagFound = true;
				*bDisplayEllipsisNow = true;
				DEBUG_PRT.println(F(" - Ellipsis case 6"));
				continue;
			}
		}
	}

	if (dcurrwidth >= dmaxwidth || bForceLineBreakAfterString) { // if the next word after lastwordendstrpos overflowed the line, tell the caller to generate a cr/newline (after rendering the text between *startstrpos and *endstrpos).
		DEBUG_PRT.print(F("GetString() Newline: dcurrwidth="));
		DEBUG_PRT.print((int)dcurrwidth);
		DEBUG_PRT.print(F(" dmaxwidth=")); 
		DEBUG_PRT.println((int)dmaxwidth);
		
		*bNewLine = true;
	}
	
	//DEBUG_PRT.printf("GetString() *startstrpos = %d, *endstrpos = %d, lastwordendstrpos = %d\n", *startstrpos, *endstrpos, lastwordendstrpos);
	DEBUG_PRT.print(F("GetString() *startstrpos = "));
	DEBUG_PRT.print(*startstrpos);
	DEBUG_PRT.print(F(", *endstrpos = "));
	DEBUG_PRT.print(*endstrpos);
	DEBUG_PRT.print(F(", lastwordendstrpos = "));
	DEBUG_PRT.println(lastwordendstrpos);
	
	if (wrap_text) {
		if ((pos >= s.length() && dcurrwidth <= dmaxwidth) || bCurrCharRightToLeft != bLookingForRightToLeft) { // makes the word boundary the end of the string if the string was scanned to the very end, unless that would overflow the line. Alternatively, makes the word boundary at the point that RTL/LTR direction changed
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


#if (false) // Debugging/testing
void Bidi::TestHarnessLa(TextBuffer& tb, int fbwidth, int fbheight, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi) {
	DEBUG_PRT.print(F("Bidi::TestHarnessLa() string s.."));
	String s = "<FONT SIZE=+1 COLOR=\"red\"><B><I>Collect</I></B></FONT> <BR><FONT SIZE=+2 COLOR=\"red\"><B><I>L</I></B></FONT>et us pray.<BR>O God, You Who by the fruitful virginity of blessed Mary, have bestowed upon  mankind the rewards of eternal salvation, grant, we beseech You, that we may enjoy the intercession of her through whom we have been found worthy to receive among us the author of life, our Lord Jesus Christ Your Son. <BR><FONT SIZE=+1 COLOR=\"red\"><B><I>W</I></B></FONT>ho livest and reignest with God the Father, in the unity of the Holy Spirit, one God, world without end.<BR><FONT COLOR=\"red\"><I>R.</I></FONT> Amen<BR>";
	DEBUG_PRT.println(F("Done. Style variables.."));

	bool bBold = false;
	bool bItalic = false;
	bool bRed = false;
	int8_t fontsize_rel = 0;

	DEBUG_PRT.print(F("Done"));
/*  	
	DEBUG_PRT.println(String(system_get_free_heap_size()));
	DEBUG_PRT.print(F("Instantiating fonts.."));
	DiskFont diskfont_normal(ePaper);
	DEBUG_PRT.print(F("Normal.."));
	DEBUG_PRT.println(String(system_get_free_heap_size()));
	DiskFont diskfont_i(ePaper);
	DEBUG_PRT.print(F(" Italic.."));
	DEBUG_PRT.println(String(system_get_free_heap_size()));
	DiskFont diskfont_plus1_bi(ePaper); 
	DEBUG_PRT.print(F(" +1 bold italic.."));
	DEBUG_PRT.println(String(system_get_free_heap_size()));
	DiskFont diskfont_plus2_bi(ePaper);
	DEBUG_PRT.println(F(" +2 bold italic"));
	DEBUG_PRT.println(String(system_get_free_heap_size()));
*/
	DEBUG_PRT.print(F("Loading fonts.."));
	diskfont_normal.begin("/Fonts/droid11.lft");
	diskfont_i.begin("/Fonts/droid11i.lft");
	diskfont_plus1_bi.begin("/Fonts/droi12bi.lft");
	diskfont_plus2_bi.begin("/Fonts/droi13bi.lft");
	DEBUG_PRT.println(F("Done"));
	DiskFont* pDiskfont = &diskfont_normal;
	DEBUG_PRT.println(F("Assigned font to diskfont"));

	int xpos = 0;
	int ypos = pDiskfont->_FontHeader.charheight;

	int8_t line_number = 0;
	bool bRenderRtl = false;
	bool bWrapText = true;
	bool bMoreText = false;

	tageffect_init();
	tageffect_reset(bBold, bItalic, bRed, fontsize_rel);
	DEBUG_PRT.println(F("Set default tag effects"));
	pDiskfont = SelectDiskFont(bBold, bItalic, fontsize_rel, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
	DEBUG_PRT.print(F("Selected diskfont, running RenderTextEx.."));

	RenderTextEx(s, &xpos, &ypos, tb, 
				pDiskfont, 
				diskfont_normal, 
				diskfont_i, 
				diskfont_plus1_bi, 
				diskfont_plus2_bi, 
				bBold, bItalic, bRed, fontsize_rel, 
				line_number, fbwidth, fbheight, 
				bRenderRtl, bWrapText, bMoreText);

	DEBUG_PRT.println(F("Done. Now flushing textbuffer.."));

	tb.flush();
	DEBUG_PRT.println(F("Done flushing textbuffer. Leaving Bidi::TestHarnessLa()"));

}

void Bidi::TestHarnessLa2(TextBuffer& tb, int fbwidth, int fbheight, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi) {
	// setup fonts and variables for text output

	bool bBold = false;
	bool bItalic = false;
	bool bRed = false;
	int8_t fontsize_rel = 0;

	DEBUG_PRT.print(F("Done"));
	DEBUG_PRT.print(F("Loading fonts.."));
	diskfont_normal.begin("/Fonts/droid11.lft");
	diskfont_i.begin("/Fonts/droid11i.lft");
	diskfont_plus1_bi.begin("/Fonts/droi12bi.lft");
	diskfont_plus2_bi.begin("/Fonts/droi13bi.lft");
	DEBUG_PRT.println(F("Done"));
	DiskFont* pDiskfont = &diskfont_normal;
	DEBUG_PRT.println(F("Assigned font to diskfont"));

	int xpos = 0;
	int ypos = pDiskfont->_FontHeader.charheight;
    fbheight = fbheight - pDiskfont->_FontHeader.charheight;

	int8_t line_number = 0;
	bool bRenderRtl = false;
	bool bWrapText = true;
	bool bMoreText = false;

	tageffect_init();
	tageffect_reset(bBold, bItalic, bRed, fontsize_rel);
	DEBUG_PRT.println(F("Set default tag effects"));
	pDiskfont = SelectDiskFont(bBold, bItalic, fontsize_rel, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
	DEBUG_PRT.print(F("Selected diskfont, running RenderTextEx.."));



	// text output

	String mass_version = "1955";
	String lang = "en";

	Yml::SetConfig(lang, "en-1962.yml", "en-1962.txt");
	Tr_Calendar_Day td;
	time64_t date;
    Config::getLocalDateTime(&date);
    tmElements_t ts;
    breakTime(date, ts);
	//ts.Day = 23;
	//ts.Hour = 20;
	//date = makeTime(ts);

	Tridentine::get(date, td, true);

	DEBUG_PRT.printf("Datetime: %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
	DEBUG_PRT.println(td.DayofWeek);
	DEBUG_PRT.println(td.Cls);
	DEBUG_PRT.println(td.Colour);
	DEBUG_PRT.println(td.Mass);
	DEBUG_PRT.println(td.Commemoration);
	DEBUG_PRT.println(td.HolyDayOfObligation);
	DEBUG_PRT.println(td.FileDir_Season);
	DEBUG_PRT.println(td.FileDir_Saint);

	int8_t filenumber = (int8_t)(ts.Hour % 12);

	String fileroot = "/Lect/EF/" + mass_version + "/" + lang;

	String indexfilename = "index.txt";
	String propersfilename = "propers.txt";

	File ifile_saint;
	File pfile_saint;
	File ifile_season;
	File pfile_season;

	IndexHeader indexheader_season;
	IndexRecord indexrecord_season;
	IndexHeader indexheader_saint;
	IndexRecord indexrecord_saint;

	DEBUG_PRT.print(F("ifile_season = ["));
	DEBUG_PRT.print(fileroot + td.FileDir_Season + indexfilename);
	DEBUG_PRT.println(F("]"));
	DEBUG_PRT.print(F("pfile_season = ["));
	DEBUG_PRT.print(fileroot + td.FileDir_Season + propersfilename);
	DEBUG_PRT.println(F("]"));

	DEBUG_PRT.print(F("ifile_saint = ["));
	DEBUG_PRT.print(fileroot + td.FileDir_Saint + indexfilename);
	DEBUG_PRT.println(F("]"));
	DEBUG_PRT.print(F("pfile_saint = ["));
	DEBUG_PRT.print(fileroot + td.FileDir_Saint + propersfilename);
	DEBUG_PRT.println(F("]\n"));

	bool bFeastDayOnly = (td.FileDir_Season == td.FileDir_Saint); // will be true if there is no seasonal day
	bool bIsFeast = SD.exists(fileroot + td.FileDir_Saint + indexfilename); // if there is no feast day on this day, the directory/propers for this day will not exist

	if (bIsFeast) {
		ifile_saint = SD.open(fileroot + td.FileDir_Saint + indexfilename, FILE_READ);
		if (!ifile_saint.available()) {
			DEBUG_PRT.println(F("Couldn't open ifile_saint"));
		}
		pfile_saint = SD.open(fileroot + td.FileDir_Saint + propersfilename, FILE_READ);
		if (!pfile_saint.available()) {
			DEBUG_PRT.println(F("Couldn't open pfile_saint"));
		}

		DEBUG_PRT.print(F("Getting headerrecord for feast:"));
		getHeaderRecord(ifile_saint, indexheader_saint);

		DEBUG_PRT.print(F("Getting indexrecord for feast:"));
		while (getIndexRecord(ifile_saint, indexrecord_saint) && indexrecord_saint.filenumber < filenumber) {
			DEBUG_PRT.print(F("."));
		}

		pfile_saint.seek(indexrecord_saint.fileoffset_start);
	}
	
	ifile_season = SD.open(fileroot + td.FileDir_Season + indexfilename, FILE_READ);
	if (!ifile_season.available()) {
		DEBUG_PRT.println(F("Couldn't open ifile_season"));
	}

	pfile_season = SD.open(fileroot + td.FileDir_Season + propersfilename, FILE_READ);
	if (!pfile_season.available()) {
		DEBUG_PRT.println(F("Couldn't open pfile_season"));
	}

	DEBUG_PRT.print(F("\nGetting headerrecord for season"));
	getHeaderRecord(ifile_season, indexheader_season);
	DEBUG_PRT.print(F("\nGetting indexrecord for season"));
	while (getIndexRecord(ifile_season, indexrecord_season) && indexrecord_season.filenumber < filenumber) {
		DEBUG_PRT.print(F("."));
	}

	pfile_season.seek(indexrecord_season.fileoffset_start);
	// Now have the indexrecords for the season and saint (if also a feast), and the filepointers pointing to the start of the text.

		//if (getClassIndex(indexheader_saint.cls) > getClassIndex(indexheader_season.cls)) {
			// Feast day takes precendence
			/*
				In this case:
				0 Introitus 	- Comes from the feast
				1 Gloria 		- Omit if season requires
				2 Collect 		- Seasonal day becomes commemoration (after the Collect text for the Feast day:
							  	  "Commemoratio <indexheader_season.name> followed by the Collect text from the season.
				3, 4, 5 Lesson, Gradual, Gospel - Comes from the Feast
				6 Credo 		- Omit if season requires
				7 Offertorium 	- Comes from the Feast
				8 Secreta 		- Like collect (both)
				9 Prefatio 		- From the Season
				10 Communio 	- From the Feast
				11 Postcommunio - From both
			*/
		//}

	bool bFeastIsCommemoration = false;
	bool bFileOk = false;
	String s = "";
	bool bOverflowedScreen = true;

	if (bIsFeast) { // there is a feast on this day
		if (bFeastDayOnly) { // is a saint's feast only
			DEBUG_PRT.println(F("Feast day only"));
			pfile_saint.seek(indexrecord_saint.fileoffset_start);

			bOverflowedScreen = false;
			while (getText(pfile_saint, indexrecord_saint, s, bFileOk, bMoreText)) {
				if (bFileOk && !bOverflowedScreen) {
					bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
													pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
													bBold, bItalic, bRed, fontsize_rel, 
													line_number, fbwidth, fbheight, 
													bRenderRtl, bWrapText, bMoreText);
				}
			}
		}
		else { // there is both a feast and a seasonal day
			DEBUG_PRT.println(F("Feast day and seasonal day"));
			File* pFileDay = &pfile_saint;
			IndexRecord* pIndexRecordDay = &indexrecord_saint;
			IndexHeader* pIndexHeaderDay = &indexheader_saint;

			File* pFileComm = &pfile_season;
			IndexRecord* pIndexRecordComm = &indexrecord_season;
			IndexHeader* pIndexHeaderComm = &indexheader_season;

			// need to exclude commemoration on Sundays and Feasts of the Lord

			bool bSaintsDayTakesPrecedence = (getClassIndex(indexheader_saint.cls) > getClassIndex(indexheader_season.cls));
			if (!bSaintsDayTakesPrecedence) {
				pFileDay = &pfile_season;
				pIndexRecordDay = &indexrecord_season;
				pIndexHeaderDay = &indexheader_season;
			
				pFileComm = &pfile_season;
				pIndexRecordComm = &indexrecord_saint;
				pIndexHeaderComm = &indexheader_saint;
			}

			// Feast day takes precedence, seasonal day is commemoration
			switch(indexrecord_saint.filenumber) {
				case 0:	// Introitus (from the feast)
				case 3: // Lesson
				case 4:	// Gradual
				case 5: // Gospel
				case 7: // Offertorium
				case 10: // Communio
					pFileDay->seek(pIndexRecordDay->fileoffset_start);
					bOverflowedScreen = false;
					while (getText(*pFileDay, *pIndexRecordDay, s, bFileOk, bMoreText)) {
						if (bFileOk && !bOverflowedScreen) {
							bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
															pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
															bBold, bItalic, bRed, fontsize_rel, 
															line_number, fbwidth, fbheight, 
															bRenderRtl, bWrapText, bMoreText);
						}
					}
				break;

				case 1: // Gloria (from the season, omit if required)
				case 6: // Credo
					pFileComm->seek(pIndexRecordComm->fileoffset_start);
					bOverflowedScreen = false;
					while (getText(*pFileComm, *pIndexRecordComm, s, bFileOk, bMoreText)) {
						if (bFileOk && !bOverflowedScreen) {
							bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
															pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
															bBold, bItalic, bRed, fontsize_rel, 
															line_number, fbwidth, fbheight, 
															bRenderRtl, bWrapText, bMoreText);
						}
					}
				break;

				case 2: // Collect
				case 8: // Secreta
				case 11: // Postcommunio
					pFileDay->seek(pIndexRecordDay->fileoffset_start);
					bOverflowedScreen = false;
					while (getText(*pFileDay, *pIndexRecordDay, s, bFileOk, bMoreText)) {
						if (bFileOk && !bOverflowedScreen) {
							bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
															pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
															bBold, bItalic, bRed, fontsize_rel, 
															line_number, fbwidth, fbheight, 
															bRenderRtl, bWrapText, bMoreText);
						}
					}
					//// TODO: PLL-25-07-2020 Find out how this works! (can remove this part?)
					//s = "<BR><FONT COLOR=\"red\"><I>Commemorio " + pIndexHeaderComm->name + "</I></FONT>";
					//if (!bOverflowedScreen) {
					//	bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
					//									pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
					//									bBold, bItalic, bRed, fontsize_rel, 
					//									line_number, fbwidth, fbheight, 
					//									bRenderRtl, bWrapText, true);
					//}
//
					//pFileComm->seek(pIndexRecordComm->fileoffset_start);
					//getText(*pFileComm, *pIndexRecordComm, s, bFileOk, bMoreText); // eat a line (the heading), should then get a <BR> on its own line after the heading
					//while (getText(*pFileComm, *pIndexRecordComm, s, bFileOk, bMoreText)) {
					//	if (bFileOk && !bOverflowedScreen) {
					//		bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
					//										pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, 	diskfont_plus2_bi, 
					//										bBold, bItalic, bRed, fontsize_rel, 
					//										line_number, fbwidth, fbheight, 
					//										bRenderRtl, bWrapText, bMoreText);
					//	}
					//}
					//// PLL-25-07-2020
				break;

				default:
				break;
			}
		}
	}
	else { // is a seasonal day only
		DEBUG_PRT.println(F("Seasonal day only"));
		pfile_season.seek(indexrecord_season.fileoffset_start);
		bOverflowedScreen = false;
		while (getText(pfile_season, indexrecord_season, s, bFileOk, bMoreText)) {
			if (bFileOk && !bOverflowedScreen) {
				bOverflowedScreen = RenderTextEx(s, &xpos, &ypos, tb, 
												pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
												bBold, bItalic, bRed, fontsize_rel, 
												line_number, fbwidth, fbheight, 
												bRenderRtl, bWrapText, bMoreText);
			}
		}
	}


	DEBUG_PRT.println(F("Done. Now flushing remaining text from textbuffer.."));

	tb.flush();
	DEBUG_PRT.println(F("Done flushing textbuffer. Leaving Bidi::TestHarnessLa2()"));
}

bool Bidi::getHeaderRecord(File& file, IndexHeader& ih) {
	//ih = {0};
	
	if (file && file.available()) {
		//int filepos = file.position();

		file.seek(0); // header is at start of index file
		
		Csv csv;
		bool bFileOk = true;
		String headers = Yml::readLine(file, bFileOk);
		DEBUG_PRT.print(F("getHeaderRecord() ["));
		DEBUG_PRT.print(headers);
		DEBUG_PRT.println(F("]"));

		if (bFileOk) {
			int pos = 0;
			ih.heading = csv.getCsvField(headers, &pos);
			ih.name = csv.getCsvField(headers, &pos);
			ih.commemoration = csv.getCsvField(headers, &pos);
			ih.cls  = csv.getCsvField(headers, &pos);
			ih.colour  = csv.getCsvField(headers, &pos);

			DumpIndexHeader(ih);
			//file.seek(filepos); // restore original filepos (if needed?)

			return true;
		}
	}
	DEBUG_PRT.print(F("getHeaderRecord() File is not available, free mem="));
	DEBUG_PRT.println(String(system_get_free_heap_size()));
	return false;
}

bool Bidi::getIndexRecord(File& file, IndexRecord& ir) {	// gets the index record at file.position()
	//ir = {0};

	if (file && file.available()) {

		Csv csv;
		bool bFileOk = true;
		String indexrecord = Yml::readLine(file, bFileOk);
		int filepos = file.position();

		DEBUG_PRT.print(F("getIndexRecord() ["));
		DEBUG_PRT.print(indexrecord);
		DEBUG_PRT.println(F("]"));

		if (bFileOk) {
			int pos = 0;
			ir.heading = csv.getCsvField(indexrecord, &pos);
			ir.filenumber = (int8_t)csv.getCsvField(indexrecord, &pos).toInt();
			ir.filecount = (int8_t)csv.getCsvField(indexrecord, &pos).toInt();
			ir.partnumber  = (int8_t)csv.getCsvField(indexrecord, &pos).toInt();
			ir.partcount  = (int8_t)csv.getCsvField(indexrecord, &pos).toInt();
			ir.fileoffset_start = csv.getCsvField(indexrecord, &pos).toInt();
			ir.fileoffset_end = csv.getCsvField(indexrecord, &pos).toInt();

			DumpIndexRecord(ir);

//			// now need to read the next record and retrieve only the fileoffset__start value, which becomes the fileoffset_end value of the
//			// record we want.
//
//			if (file.available()) {
//				String nextindexrecord = Yml::readLine(file, bFileOk);
//				if (bFileOk) {
//					pos = 0;
//					for (int8_t i = 0; i < 5; i++) {
//						csv.getCsvField(indexrecord, &pos);	// skip over unwanted records in csv line, only want the last one
//					}
//					ir.fileoffset_end = csv.getCsvField(indexrecord, &pos).toInt();
//				}
//			}
//
//			file.seek(filepos); // restore original filepos which should be end of record n (ie start of record n+1, where we got the fileoffset_end value)

			return true;
		}
	}
	DEBUG_PRT.print("getIndexRecord() File is not available, free mem=");
	DEBUG_PRT.println(String(system_get_free_heap_size()));
	return false;
}

uint8_t Bidi::getClassIndex(String& cls) {
	const char* const tradtable[8] = {
        "none", "Simplex", "Semiduplex", "Duplex",
        "Duplex majus", "Duplex II. classis", "Duplex I. classis", "Duplex I. classis"
	};
    
	const char* const newtable[8] = {
        "none",
        "Commemoratio",
        "III. classis",
        "III. classis",
        "III. classis",
        "II. classis",
        "I. classis",
        "I. classis"
	};

	for (uint8_t i = 0; i < 8; i++) {
		if (cls == String(tradtable[i]) || cls == String(newtable[i])) return i;
	}
	
	return 0;
}

bool Bidi::getText(File& file, IndexRecord& indexrecord, String& s, bool& bFileOk, bool& bMoreText) {
	bMoreText = false;
	bool bTextAvailable = (file.position() < indexrecord.fileoffset_end);
	if (indexrecord.fileoffset_end == -1) {
		bTextAvailable = (indexrecord.fileoffset_end < file.size());
	}

	DEBUG_PRT.print(F("getText() file.position="));
	DEBUG_PRT.print(file.position());
	DEBUG_PRT.print(F(", indexrecord.fileoffset_start="));
	DEBUG_PRT.print(indexrecord.fileoffset_start);
	DEBUG_PRT.print(F(", indexrecord.fileoffset_end="));
	DEBUG_PRT.print(indexrecord.fileoffset_end);
	DEBUG_PRT.print(F(", bTextAvailable="));
	DEBUG_PRT.println(bTextAvailable);

	if (bTextAvailable) {
		s = Yml::readLine(file, bFileOk, indexrecord.fileoffset_end);
		
		bMoreText = (file.position() < indexrecord.fileoffset_end);
		if (indexrecord.fileoffset_end == -1) {
			bMoreText = (indexrecord.fileoffset_end < file.size());
		}

		DEBUG_PRT.print(F("Text is ["));
		DEBUG_PRT.print(s);
		DEBUG_PRT.print(F("]\nbMoreText = "));
		DEBUG_PRT.println(bMoreText);
	}

	return bTextAvailable;	// shows if this call returned some text
}

void Bidi::DumpIndexHeader(IndexHeader& ih) {
	DEBUG_PRT.println(ih.heading);
	DEBUG_PRT.println(ih.name);
	DEBUG_PRT.println(ih.commemoration);
	DEBUG_PRT.println(ih.cls);
	DEBUG_PRT.println(ih.colour);
}

void Bidi::DumpIndexRecord(IndexRecord& ir) {
	DEBUG_PRT.println(ir.heading);
	DEBUG_PRT.println(ir.filenumber);
	DEBUG_PRT.println(ir.filecount);
	DEBUG_PRT.println(ir.partnumber);
	DEBUG_PRT.println(ir.partcount);
	DEBUG_PRT.println(ir.fileoffset_start);
	DEBUG_PRT.println(ir.fileoffset_end);
}
#endif