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

void TextBuffer::add_buffered(int x, int y, String text, String colormap, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number){
	DisplayString* d = new DisplayString(text, colormap, x, y, right_to_left, reverse_string, diskfont);
	displayStringsList.add(d);
}

void TextBuffer::justify_buffer(double spacecharwidth, int fbwidth, DiskFont& diskfont, bool bJustify) {
	int numListEntries = displayStringsList.size();
	double space_width = 0.0;
	int w = 0;
	int wordcount = 0;

	diskfont.ClearSpaceCharCustomWidth();
	diskfont.GetTextWidth(" ", w, space_width);

	DisplayString* ds;

	// need to calculate the width of the space character needed to justify each line of text stored in the displayStringsList list
	// if using display buffer sram, will need to call DrawStringAt for all the entries on the line
	double txtwidth = 0.0;

	int8_t line_number = -1;
	int fragments_on_line = 0;
	int ds_start_i = 0;

	for (int i = 0; i < displayStringsList.size(); i++) {
		ds = displayStringsList.get(i);

		if (bJustify) {
			if (ds->line_num != line_number || i == displayStringsList.size() - 1) { // i == displayStringsList.size() - 1 because last entry in displayStrings list will have nothing to follow it, so won't be able to compare line number with next item
				if (line_number != -1) {
					
					if (i == displayStringsList.size() - 1) { // if it's the last entry in the list
						txtwidth += ds->advance_width;
						wordcount += ds->wordcount;
					}

					double space_char_width = (fbwidth - (txtwidth - (space_width * (wordcount - 1)))) / wordcount; // wordcount - 1 because there is no space char at the end of the line to resize, eg, if you have 4 words you have 3 spaces
					
					DisplayString* dss;
					for (int j = ds_start_i; j < i; j++) {	// set the calculated space char width for justifying the text on this line, for all fragments on the line
						dss = displayStringsList.get(j);
						dss->space_char_width = space_char_width;
						dss->b_use_default_spacechar_width = false;
					}

					if (i == displayStringsList.size() - 1) { // if it's the last entry in the list
						dss = displayStringsList.get(i);
						dss->space_char_width = space_char_width;
						dss->b_use_default_spacechar_width = false;
					}
				}

				ds_start_i = i;
				line_number = ds->line_num;
				txtwidth = ds->advance_width;
				wordcount = ds->wordcount;

			}
			else {
				txtwidth += ds->advance_width;
				wordcount += ds->wordcount;
			}
		}
		else {
			ds->b_use_default_spacechar_width = true;
		}
	}
}

void TextBuffer::flush(DiskFont& diskfont) { // write any textbuffer lines to the display then empty the textbuffer
	#ifdef USE_SPI_RAM_FRAMEBUFFER
	for (int i = 0; i < displayStringsList.size(); i++) { // delete displaystring objects stored in linked list, before deleting list itself
		DisplayString* ds = displayStringsList.get(i);
				
		if (ds->b_use_default_spacechar_width) {
			diskfont.SetSpaceCharCustomWidth(ds->space_char_width);
		}
		else {
			diskfont.ClearSpaceCharCustomWidth();
		}
		
		uint16_t color = GxEPD_BLACK;
		diskfont.DrawStringAt(ds->x, ds->y, ds->text, *_p_ePaper, ds->color, ds->colormap, ds->right_to_left, ds->reverse_string);
	}

	for (int i = 0; i < displayStringsList.size(); i++) { // delete displaystring objects stored in linked list, before deleting list itself
		DisplayString* pDisplayString = displayStringsList.get(i);
		delete pDisplayString;
	}
	
	displayStringsList.clear();
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
