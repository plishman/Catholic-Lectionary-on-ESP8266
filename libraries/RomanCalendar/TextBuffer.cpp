#include "TextBuffer.h"

//#undef DEBUG_PRT
//#define DEBUG_PRT Serial

DisplayString::DisplayString() {
	
}

DisplayString::DisplayString(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont) {
	set(t, "", px, py, pcolor, rtl, reverse_str, diskfont);
}

DisplayString::DisplayString(String t, int px, int py, DiskFont& diskfont) {
	DisplayString(t, px, py, GxEPD_BLACK, false, false, diskfont);
}

DisplayString::DisplayString(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont) {
	DisplayString(t, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont);
}

DisplayString::DisplayString(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont) {
	DisplayString(t, px, py, pcolor, false, false, diskfont);
}

DisplayString::DisplayString(String t, String cmap, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont) {
	set(t, cmap, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont);	
}

DisplayString::DisplayString(String t, String cmap, int px, int py, DiskFont& diskfont) {
	DisplayString(t, cmap, px, py, false, false, diskfont);
}




void DisplayString::set(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont) {
	set(t, "", px, py, GxEPD_BLACK, rtl, reverse_str, diskfont);
}

void DisplayString::set(String t, int px, int py, DiskFont& diskfont) {
	set(t, "", px, py, GxEPD_BLACK, false, false, diskfont);
}

void DisplayString::set(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont) {
	set(t, "", px, py, pcolor, false, false, diskfont);
}

void DisplayString::set(String t, String cmap, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont) {
	set(t, cmap, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont);
}

void DisplayString::set(String t, String cmap, int px, int py, DiskFont& diskfont) {
	set(t, cmap, px, py, GxEPD_BLACK, false, false, diskfont);
}


void DisplayString::set(String t, String cmap, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont) {
	// if cmap == "", pcolor is used to colour whole string
	text = t;
	x = px;
	y = py;
	color = pcolor;
	colormap = cmap;
	
	right_to_left = rtl;
	reverse_string = reverse_str;
	
	double advanceWidth = 0.0;
	int text_width = 0;
	diskfont.GetTextWidth(text, text_width, advanceWidth);
	
	w = text_width; //< (PANEL_SIZE_X - x) ? text_width : PANEL_SIZE_X - x;		
	h = diskfont._FontHeader.charheight;
	
	dump();
}

bool DisplayString::IsEmpty() {
	return text == "" && colormap == "" && x == 0 && y == 0 && w == 0 && h == 0 && color == GxEPD_WHITE && right_to_left == false && reverse_string == false;
}

DisplayString::DisplayString(const DisplayString& d2) { // copy constructor
	x 				= d2.x;
	y 				= d2.y;
	w 				= d2.w;
	h	 			= d2.h;
	text 			= d2.text;
	colormap		= d2.colormap;
	right_to_left 	= d2.right_to_left;
	reverse_string 	= d2.reverse_string;
	color	 		= d2.color;
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

void TextBuffer::add(int x, int y, String text, uint16_t color, bool right_to_left, bool reverse_string, DiskFont& diskfont) {
#ifndef USE_SPI_RAM_FRAMEBUFFER
	DisplayString* d = new DisplayString(text, x, y, color, right_to_left, reverse_string, diskfont);
	displayStringsList.add(d);
#else
	String colormap = "";
	diskfont.DrawStringAt(x, y, text, *_p_ePaper, color, colormap, right_to_left, reverse_string);
#endif
}

void TextBuffer::add(int x, int y, String text, String colormap, bool right_to_left, bool reverse_string, DiskFont& diskfont) {
#ifndef USE_SPI_RAM_FRAMEBUFFER
	DisplayString* d = new DisplayString(text, colormap, x, y, right_to_left, reverse_string, diskfont);
	displayStringsList.add(d);
#else
	uint16_t color = GxEPD_BLACK;
	diskfont.DrawStringAt(x, y, text, *_p_ePaper, color, colormap, right_to_left, reverse_string);
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
