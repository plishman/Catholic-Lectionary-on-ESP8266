#include "TextBuffer.h"

//#undef DEBUG_PRT
//#define DEBUG_PRT Serial

DisplayString::DisplayString() {
	
}

DisplayString::DisplayString(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number) {
	set(t, "", px, py, pcolor, rtl, reverse_str, diskfont, line_number);
}

DisplayString::DisplayString(String t, int px, int py, DiskFont& diskfont, int8_t line_number) {
	DisplayString(t, px, py, GxEPD_BLACK, false, false, diskfont, line_number);
}

DisplayString::DisplayString(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number) {
	DisplayString(t, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont, line_number);
}

DisplayString::DisplayString(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont, int8_t line_number) {
	DisplayString(t, px, py, pcolor, false, false, diskfont, line_number);
}

DisplayString::DisplayString(String t, String cmap, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number) {
	set(t, cmap, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont, line_number);	
}

DisplayString::DisplayString(String t, String cmap, int px, int py, DiskFont& diskfont, int8_t line_number) {
	DisplayString(t, cmap, px, py, false, false, diskfont, line_number);
}



void DisplayString::set(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number) {
	set(t, "", px, py, GxEPD_BLACK, rtl, reverse_str, diskfont, line_number);
}

void DisplayString::set(String t, int px, int py, DiskFont& diskfont, int8_t line_number) {
	set(t, "", px, py, GxEPD_BLACK, false, false, diskfont, line_number);
}

void DisplayString::set(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont, int8_t line_number) {
	set(t, "", px, py, pcolor, false, false, diskfont, line_number);
}

void DisplayString::set(String t, String cmap, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number) {
	set(t, cmap, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont, line_number);
}

void DisplayString::set(String t, String cmap, int px, int py, DiskFont& diskfont, int8_t line_number) {
	set(t, cmap, px, py, GxEPD_BLACK, false, false, diskfont, line_number);
}


void DisplayString::set(String t, String cmap, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number) {
	// if cmap == "", pcolor is used to colour whole string
	text = t;
	x = px;
	y = py;
	color = pcolor;
	colormap = cmap;
	line_num = line_number;
	pDiskFont = &diskfont;

	right_to_left = rtl;
	reverse_string = reverse_str;
	
	diskfont.GetTextWidth(text, w, advance_width);

	b_use_default_spacechar_width = true;

	//int s;
	//diskfont.GetTextWidth(" ", s, space_char_width);

	//w = text_width; //< (PANEL_SIZE_X - x) ? text_width : PANEL_SIZE_X - x;		
	h = diskfont._FontHeader.charheight;

	int p = -1;
	wordcount = 0;
	do {
		p = text.indexOf(' ', p + 1);
		wordcount++; // at least one word
	} while (p != -1);

	if (text.endsWith(" ")) {	// if there's a trailing space on the string, wordcount will be 1 too many
		wordcount--;
	}

	dump();
}

bool DisplayString::IsEmpty() {
	return text == "" && colormap == "" && x == 0 && y == 0 && w == 0 && h == 0 && color == GxEPD_WHITE && right_to_left == false && reverse_string == false;
}

DisplayString::DisplayString(const DisplayString& d2) { // copy constructor
	x 				 	   			= d2.x;
	y 				 	   			= d2.y;
	w 				 	   			= d2.w;
	h	 			 	   			= d2.h;
	space_char_width 	   			= d2.space_char_width;
	b_use_default_spacechar_width 	= d2.b_use_default_spacechar_width;
	wordcount 		 	   			= d2.wordcount;
	line_num		 	   			= d2.line_num;
	text 			 	   			= d2.text;
	colormap		 	   			= d2.colormap;
	right_to_left 	 	   			= d2.right_to_left;
	reverse_string 	 	   			= d2.reverse_string;
	color	 		 	   			= d2.color;
	pDiskFont						= d2.pDiskFont;
}

void DisplayString::dump() {
	DEBUG_PRT.print(F("Text: "));
	DEBUG_PRT.println(text);
	DEBUG_PRT.print(F("Colormap: "));
	DEBUG_PRT.println(colormap);
	
	//DEBUG_PRT.printf("x:%d, y:%d, w:%d, h:%d, color:%d, right_to_left:%d, reverse_string:%d\n\n", x, y, w, h, color, right_to_left, reverse_string);
	DEBUG_PRT.print(F("x:"));
	DEBUG_PRT.print(x);
	DEBUG_PRT.print(F(", y:"));
	DEBUG_PRT.print(y);
	DEBUG_PRT.print(F(", w:"));
	DEBUG_PRT.print(w);
	DEBUG_PRT.print(F(", h:"));
	DEBUG_PRT.print(h);
	DEBUG_PRT.print(F(", line_num:"));
	DEBUG_PRT.print(line_num);
	DEBUG_PRT.print(F(", color:"));
	DEBUG_PRT.print(color);
	DEBUG_PRT.print(F(", right_to_left:"));
	DEBUG_PRT.print(right_to_left);
	DEBUG_PRT.print(F(", reverse_string:"));
	DEBUG_PRT.println(reverse_string);
	
	//DEBUG_PRT.printf("IsEmpty: %s\n", IsEmpty() ? "true" : "false");
	DEBUG_PRT.print(F("IsEmpty: "));
	DEBUG_PRT.println(IsEmpty() ? F("true") : F("false"));

}

/*
int compare(DisplayString*& a, DisplayString*& b) {
return a->y - b->y;
}
*/

TextBuffer::TextBuffer(FB_EPAPER ePaper) {
	_p_ePaper = &ePaper;
	displayStringsList.clear();
}

TextBuffer::~TextBuffer() {
	for (int i = 0; i < displayStringsList.size(); i++) { // delete displaystring objects stored in linked list, before deleting list itself
		DisplayString* pDisplayString = displayStringsList.get(i);
		delete pDisplayString;
	}
	
	displayStringsList.clear();
}

void TextBuffer::add(DisplayString* d) {
	//DEBUG_PRT.println(F("add:"));
	//d->dump();
	displayStringsList.add(d);
	//DEBUG_PRT.println(F("-"));
}

void TextBuffer::add(int x, int y, String text, uint16_t color, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number) {
#ifndef USE_SPI_RAM_FRAMEBUFFER
	DisplayString* d = new DisplayString(text, x, y, color, right_to_left, reverse_string, diskfont);
	displayStringsList.add(d);
#else
	String colormap = "";
	diskfont.DrawStringAt(x, y, text, *_p_ePaper, color, colormap, right_to_left, reverse_string);
#endif
}

void TextBuffer::add(int x, int y, String text, String colormap, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number) {
#ifndef USE_SPI_RAM_FRAMEBUFFER
	DisplayString* d = new DisplayString(text, colormap, x, y, right_to_left, reverse_string, diskfont);
	displayStringsList.add(d);
#else
	uint16_t color = GxEPD_BLACK;
	diskfont.DrawStringAt(x, y, text, *_p_ePaper, color, colormap, right_to_left, reverse_string);
#endif
}

void TextBuffer::add_buffered(int x, int y, String text, uint16_t color, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number){
	DEBUG_PRT.print(F("add_buffered() adding ["));
	DEBUG_PRT.print(text);
	DEBUG_PRT.print(F("], font = "));
	DEBUG_PRT.print(diskfont._FontHeader.charheight);
	DEBUG_PRT.print(F(", colour = "));
	DEBUG_PRT.println(color == GxEPD_RED ? F("Red"):F("Black"));
	
	DisplayString* d = new DisplayString(text, x, y, color, right_to_left, reverse_string, diskfont, line_number);
	displayStringsList.add(d);
}

void TextBuffer::add_buffered(int x, int y, String text, String colormap, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number){
	DisplayString* d = new DisplayString(text, colormap, x, y, right_to_left, reverse_string, diskfont, line_number);
	displayStringsList.add(d);
}



//// action is either TB_FORMAT_JUSTIFY, TB_FORMAT_CENTRE or TB_FORMAT_RJUSTIFY
//void TextBuffer::format_buffer(int fbwidth, uint8_t format_action, int8_t line_number) {
//	DEBUG_PRT.print(F("TextBuffer::format_buffer: format_action is "));
//	DEBUG_PRT.print(format_action);
//	DEBUG_PRT.print(F(", line number is "));
//	DEBUG_PRT.print(line_number == -1 ? "All" : String(line_number));
//
//	bool bJustify = (format_action == TB_FORMAT_JUSTIFY);
//	bool bCentre = (format_action == TB_FORMAT_CENTRE);
//	bool bRJustify = (format_action == TB_FORMAT_RJUSTIFY);
//
//	DEBUG_PRT.print(F("format action is "));
//	if (bJustify) { 
//		DEBUG_PRT.print("Justify");
//	}
//	else if (bRJustify) {
//		DEBUG_PRT.print("Right Justify");
//	}
//	else if (bCentre) {
//		DEBUG_PRT.print("Centre");
//	}
//	else {
//		DEBUG_PRT.print("None");
//	}
//
//	int numListEntries = displayStringsList.size();
//	double space_width = 0.0;
//	int w = 0;
//	int wordcount = 0;
//
//	//diskfont.ClearSpaceCharCustomWidth();
//	//diskfont.GetTextWidth(" ", w, space_width);
//
//	DisplayString* ds;
//
//	// need to calculate the width of the space character needed to justify each line of text stored in the displayStringsList list
//	// if using display buffer sram, will need to call DrawStringAt for all the entries on the line
//	double txtwidth = 0.0;
//	double whitespacewidth = 0.0;
//
//	int8_t curr_line_number = -1;
//	int8_t last_line_number = -1;
//	int fragments_on_line = 0;
//	int ds_start_i = 0;
//
//	DEBUG_PRT.print(F("\ndisplayStringsList.size() = "));
//	DEBUG_PRT.println(displayStringsList.size());
//
//	for (int i = 0; i < displayStringsList.size(); i++) {
//		ds = displayStringsList.get(i);
//		
//		if (i == 0) {
//			last_line_number = ds->line_number;
//			curr_line_number = last_line_number;
//		}
//		else {
//			last_line_number = curr_line_number;
//			curr_line_number = ds->line_num;
//		}
//
//		DEBUG_PRT.print(F("1 "));
//
//		if (bJustify || bRJustify || bCentre) {
//			DEBUG_PRT.print(F("2 "));
//			if ((last_line_number == curr_line_number) { 
//				txtwidth += ds->advance_width;
//				wordcount += ds->wordcount;
//				ds->pDiskFont->GetTextWidth(" ", w, space_width);
//				whitespacewidth += ds->wordcount * space_width; // restart calculating whitespace width for displaystrings on this line
//			}
//			else｛			
//				if ((line_number != -1 && curr_line_number == line_number) || (line_number == -1)) {
//					if (bJustify) {
//						DEBUG_PRT.print(F("J "));
//						DEBUG_PRT.print(F("Justifying line number "));
//						DEBUG_PRT.print(line_number);
//						//double space_char_width = (fbwidth - (txtwidth - (space_width * (wordcount - 1)))) / wordcount; // wordcount - 1 because there is no space char at the end of the line to resize, eg, if you have 4 words you have 3 spaces
//						double space_char_width = (fbwidth - whitespacewidth) / (wordcount - 1); // wordcount - 1 because there is no space char at the end of the line to resize, eg, if you have 4 words you have 3 spaces
//						DisplayString* dss;
//						for (int j = ds_start_i; j < i; j++) {	// set the calculated space char width for justifying the text on this line, for all fragments on the line
//							dss = displayStringsList.get(j);
//							dss->space_char_width = space_char_width;
//							dss->b_use_default_spacechar_width = false;
//							DEBUG_PRT.print(F("."));
//						}
//
//						if (i == displayStringsList.size() - 1) { // if it's the last entry in the list
//							DEBUG_PRT.print(F("*"));
//							dss = displayStringsList.get(i);
//							dss->space_char_width = space_char_width;
//							dss->b_use_default_spacechar_width = false;
//						}
//						DEBUG_PRT.println(F("done."));
//					}
//					
//					if (bCentre || bRJustify) {
//						DEBUG_PRT.print(F("C/RJ "));
//						int ds_offset_x = (fbwidth - (int)txtwidth);	// right justify
//
//						if (bCentre) {
//							DEBUG_PRT.print(F("Centring line number "));
//							ds_offset_x = ds_offset_x / 2;				// centre
//						}
//						else {
//							DEBUG_PRT.print(F("Justifying line number "));
//						}
//						DEBUG_PRT.print(line_number);
//						DEBUG_PRT.print(F(", ds_offset_x = "));
//						DEBUG_PRT.print(ds_offset_x);
//
//						DisplayString* dss;
//						for (int j = ds_start_i; j < i; j++) {	// set the calculated space char width for justifying the text on this line, for all fragments on the line
//							DEBUG_PRT.print(F("."));
//							dss = displayStringsList.get(j);
//							dss->x += ds_offset_x;
//						}
//						
//						if (i == displayStringsList.size() - 1) { // if it's the last entry in the list
//							DEBUG_PRT.print(F("*"));
//							dss = displayStringsList.get(i);
//							dss->x += ds_offset_x;
//						}
//					}
//
//					if (line_number != -1) {	// -1 = do all lines, if not, then only doing one line, so break early
//						DEBUG_PRT.print(F("(formatting only one line - break"));
//						break;
//					}
//				}
//
//				ds_start_i = i;
//				curr_line_number = ds->line_num;
//				txtwidth = ds->advance_width;
//				wordcount = ds->wordcount;
//				ds->pDiskFont->GetTextWidth(" ", w, space_width);
//				whitespacewidth = ds->wordcount * space_width; // restart calculating whitespace width for displaystrings on this line
//
//			}
//			else {
//				txtwidth += ds->advance_width;
//				wordcount += ds->wordcount;
//				ds->pDiskFont->GetTextWidth(" ", w, space_width);
//				whitespacewidth += ds->wordcount * space_width; // calculate whitespace width for this displaystring
//			}
//		}
//		else {
//			if (line_number == -1) {						// -1 = do all lines, if not, then only doing one line
//				ds->b_use_default_spacechar_width = true;	// probably shouldn't touch these values unless going to process all of them.
//			}												// (checking line_number (to process) is -1 means doing all of them, so touch these)
//		}
//	}
//
//	DEBUG_PRT.println(F("TextBuffer::format_buffer complete"));
//}


void TextBuffer::format_buffer(int fbwidth, uint8_t format_action,  int8_t line_number) {
	DEBUG_PRT.print(F("TextBuffer::format_buffer: format_action is "));
	DEBUG_PRT.print(format_action);
	DEBUG_PRT.print(F(", line number is "));
	DEBUG_PRT.print(line_number == -1 ? "All" : String(line_number));

	bool bJustify = (format_action == TB_FORMAT_JUSTIFY);
	bool bCentre = (format_action == TB_FORMAT_CENTRE);
	bool bRJustify = (format_action == TB_FORMAT_RJUSTIFY);

	DEBUG_PRT.print(F("format action is "));
	if (bJustify) { 
		DEBUG_PRT.print("Justify");
	}
	else if (bRJustify) {
		DEBUG_PRT.print("Right Justify");
	}
	else if (bCentre) {
		DEBUG_PRT.print("Centre");
	}
	else {
		DEBUG_PRT.print("None");
	}

	int numListEntries = displayStringsList.size();
	double space_width = 0.0;
	int w = 0;
	int wordcount = 0;

	DisplayString* ds;

	// need to calculate the width of the space character needed to justify each line of text stored in the displayStringsList list
	// if using display buffer sram, will need to call DrawStringAt for all the entries on the line
	double txtwidth = 0.0;
	double whitespacewidth = 0.0;

	int8_t curr_line_number = -1;
	int fragments_on_line = 0;
	int ds_start_i = 0;

	int i = 0;

	while (i < displayStringsList.size()) {
		ds = displayStringsList.get(i);

		ds_start_i = i;
		curr_line_number = ds->line_num;
		txtwidth = 0.0;
		wordcount = 0;
		whitespacewidth = 0.0; // restart calculating whitespace width for displaystrings on this line
		ds->pDiskFont->GetTextWidth(" ", w, space_width);

		int8_t this_line = curr_line_number;
		while (this_line == curr_line_number) {
			txtwidth += ds->advance_width;
			wordcount += ds->wordcount;
			whitespacewidth += ds->wordcount * space_width; // restart calculating whitespace width for displaystrings on this line
			i++;
			if (i == displayStringsList.size()) break;
			ds = displayStringsList.get(i);
			this_line = ds->line_num;
		}

		if (line_number == -1 || line_number == curr_line_number) {
			if (bJustify) {
				DEBUG_PRT.print(F("J "));
				DEBUG_PRT.print(F("Justifying line number "));
				DEBUG_PRT.print(line_number);
				//double space_char_width = (fbwidth - (txtwidth - (space_width * (wordcount - 1)))) / wordcount; // wordcount - 1 because there is no space char at the end of the line to resize, eg, if you have 4 words you have 3 spaces
				double space_char_width = (fbwidth - whitespacewidth) / (wordcount - 1); // wordcount - 1 because there is no space char at the end of the line to resize, eg, if you have 4 words you have 3 spaces
				DisplayString* dss;
				for (int j = ds_start_i; j < i; j++) {	// set the calculated space char width for justifying the text on this line, for all fragments on the line
					dss = displayStringsList.get(j);
					dss->space_char_width = space_char_width;
					dss->b_use_default_spacechar_width = false;
					DEBUG_PRT.print(F("."));
				}

				if (i == displayStringsList.size() - 1) { // if it's the last entry in the list
					DEBUG_PRT.print(F("*"));
					dss = displayStringsList.get(i);
					dss->space_char_width = space_char_width;
					dss->b_use_default_spacechar_width = false;
				}
				DEBUG_PRT.println(F("done."));
			}
			
			if (bCentre || bRJustify) {
				DEBUG_PRT.print(F("C/RJ "));
				int ds_offset_x = (fbwidth - (int)txtwidth);	// right justify

				if (bCentre) {
					DEBUG_PRT.print(F("Centring line number "));
					ds_offset_x = ds_offset_x / 2;				// centre
				}
				else {
					DEBUG_PRT.print(F("Right Justifying line number "));
				}
				DEBUG_PRT.print(line_number);
				DEBUG_PRT.print(F(", ds_offset_x = "));
				DEBUG_PRT.print(ds_offset_x);

				DisplayString* dss;
				for (int j = ds_start_i; j < i; j++) {	// set the calculated space char width for justifying the text on this line, for all fragments on the line
					DEBUG_PRT.print(F("."));
					dss = displayStringsList.get(j);
					dss->x += ds_offset_x;
				}
			}
		}

		if (line_number != -1 && line_number == curr_line_number) break; // if doing one line only
	}
}








int16_t TextBuffer::typeset(int8_t line_number, int fbwidth, uint8_t format_action, bool bFlushNow) {
	int numListEntries = displayStringsList.size();
	double space_width = 0.0;
	int w = 0;
	int wordcount = 0;
	int16_t maxcharheight = 0;

	DisplayString* pDs;
	DiskFont* pDf = NULL;

	// need to calculate the width of the space character needed to justify each line of text stored in the displayStringsList list
	// if using display buffer sram, will need to call DrawStringAt for all the entries on the line
	double txtwidth = 0.0;

	int fragments_on_line = 0;
	int ds_start_i = 0;

	// get the font height of the tallest font with characters on the same line
	DEBUG_PRT.print(F("Typeset() number of display strings: "));
	DEBUG_PRT.println(displayStringsList.size());
	DEBUG_PRT.print(F("Typeset() line_number=: "));
	DEBUG_PRT.println(line_number);

	for (int i = 0; i < displayStringsList.size(); i++) {
		DEBUG_PRT.print(i);
		DEBUG_PRT.print(F(":"));
		pDs = displayStringsList.get(i);

		if (pDs->line_num == line_number) {
			if (pDs->line_num != -1) {
				pDf = pDs->pDiskFont;

				if (pDf == NULL) {
					DEBUG_PRT.println(F("(pDf is null)"));
				}

				if (pDf != NULL && pDf->_FontHeader.charheight > maxcharheight) {
					maxcharheight = pDf->_FontHeader.charheight;
					DEBUG_PRT.print(F("pDf->_FontHeader.charheight = "));
					DEBUG_PRT.println(pDf->_FontHeader.charheight);
				}
			}
		}
		DEBUG_PRT.print(F(" "));
	}
	
	DEBUG_PRT.print(F("\nTypeset() Max char height is "));
	DEBUG_PRT.println(maxcharheight);

	// now have maximum height of the different fonts on the line, so will be able to calculate how much to offset the shorter font height strings on the same line
	for (int i = 0; i < displayStringsList.size(); i++) {
		pDs = displayStringsList.get(i);

		if (pDs->line_num == line_number) {
			if (pDs->line_num != -1) {
				pDf = pDs->pDiskFont;

				int16_t currcharheight = maxcharheight; // initialize with the value we know
				int line_yoffset = 0;

				if (pDf != NULL) {
					currcharheight = pDf->_FontHeader.charheight;
					line_yoffset = maxcharheight - currcharheight;
					pDs->y = pDs->y + (int)line_yoffset; // align the bottoms of all the characters

					DEBUG_PRT.print(F("maxcharheight, currcharheight, line_yoffset, pDs->y = "));
					DEBUG_PRT.print(maxcharheight);
					DEBUG_PRT.print(F(","));
					DEBUG_PRT.print(currcharheight);
					DEBUG_PRT.print(F(","));
					DEBUG_PRT.print(line_yoffset);
					DEBUG_PRT.print(F(","));
					DEBUG_PRT.print(pDs->y);

					//String colormap = "";
					//pDf->DrawStringAt(pDs->x, pDs->y, pDs->text, *_p_ePaper, pDs->color, pDs->colormap, pDs->right_to_left, pDs->reverse_string);
				}
				else {
					DEBUG_PRT.println("pDf is null");
				}
			}
		}
	}

	if (format_action != TB_FORMAT_NONE) {
		DEBUG_PRT.print(F(", calling TextBuffer::format_buffer(), format action = "));
		DEBUG_PRT.print(format_action);
		DEBUG_PRT.print(F("..."));
		
		format_buffer(fbwidth, format_action, line_number);
		
		DEBUG_PRT.println(F("Returned from TextBuffer::format_buffer()"));
	}

	if (bFlushNow) flush();

	return maxcharheight;
	// now need to call flush to output the text (if not already called)
}

void TextBuffer::flush() { // write any textbuffer lines to the display then empty the textbuffer
	#ifdef USE_SPI_RAM_FRAMEBUFFER
	for (int i = 0; i < displayStringsList.size(); i++) { // delete displaystring objects stored in linked list, before deleting list itself
		DisplayString* pDs = displayStringsList.get(i);
		DiskFont* pDf = pDs->pDiskFont;

		if (pDs->b_use_default_spacechar_width) {
			pDf->ClearSpaceCharCustomWidth();
		}
		else {
			pDf->SetSpaceCharCustomWidth(pDs->space_char_width);
		}
		
		uint16_t color = GxEPD_BLACK;
		DEBUG_PRT.print(F("flush() Drawing string ["));
		DEBUG_PRT.print(pDs->text);
		DEBUG_PRT.print(F("].."));
		pDf->DrawStringAt(pDs->x, pDs->y, pDs->text, *_p_ePaper, pDs->color, pDs->colormap, pDs->right_to_left, pDs->reverse_string);
		DEBUG_PRT.println(F("OK"));
	}

	DEBUG_PRT.print(F("Clearing displaystrings list.."));
	for (int i = 0; i < displayStringsList.size(); i++) { // delete displaystring objects stored in linked list, before deleting list itself
		DisplayString* pDisplayString = displayStringsList.get(i);
		delete pDisplayString;
	}
	
	displayStringsList.clear();
	DEBUG_PRT.println(F("OK"));

	#endif
}

bool TextBuffer::get(int displayListEntryNumber, DisplayString* displayString) {
	if (displayStringsList.size() >= displayListEntryNumber) return false;
	
	displayString = displayStringsList.get(displayListEntryNumber);
	
	return displayString->IsEmpty();
}

/*	
void sort_on_y() {
	displayStringsList.sort(compare);
}
*/

bool TextBuffer::render(FB_EPAPER ePaper, DiskFont& diskfont, int displayPage) {	
	diskfont.setDisplayPage(displayPage);
	
	//DEBUG_PRT.println(F("render()..."));
	int numListEntries = displayStringsList.size();

	//ePaper.setRotation(1); //90 degrees				
	//ePaper.eraseDisplay();

	DisplayString* ds;
	
	for (int i = 0; i < displayStringsList.size(); i++) {
		ds = displayStringsList.get(i);
		//DEBUG_PRT.print(F("["));
		//DEBUG_PRT.print("{" + ds->text + "}");
		if (ds->b_use_default_spacechar_width) {
			//diskfont.SetSpaceCharCustomWidth(ds->space_char_width);
			diskfont.ClearSpaceCharCustomWidth();
		}
		else {
			//diskfont.ClearSpaceCharCustomWidth();
			diskfont.SetSpaceCharCustomWidth(ds->space_char_width);
		}

		diskfont.DrawStringAt(ds->x, ds->y, ds->text, ePaper, ds->color, ds->colormap, ds->right_to_left, ds->reverse_string);
		//DEBUG_PRT.print(F("]"));
		//DEBUG_PRT.print(F("render():"));
		//ds->dump();
	}
	//DEBUG_PRT.println(F("ok"));
}

void TextBuffer::dump() {
	return;
	
	DisplayString* ds;

	DEBUG_PRT.println(F("TextBuffer Dump:"));
	for (int i = 0; i < displayStringsList.size(); i++) {
		ds = displayStringsList.get(i);
		DEBUG_PRT.print(F("List entry "));
		DEBUG_PRT.println(String(i));
		ds->dump();
		DEBUG_PRT.println(F("-"));
	}
	DEBUG_PRT.println(F("-complete-"));
}
