#include "TextBuffer.h"

DisplayString::DisplayString() {
	
}

DisplayString::DisplayString(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont) {
	set(t, px, py, pcolor, rtl, reverse_str, diskfont);
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

void DisplayString::set(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont) {
	set(t, px, py, GxEPD_BLACK, rtl, reverse_str, diskfont);
}

void DisplayString::set(String t, int px, int py, DiskFont& diskfont) {
	set(t, px, py, GxEPD_BLACK, false, false, diskfont);
}

void DisplayString::set(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont) {
	set(t, px, py, pcolor, false, false, diskfont);
}

void DisplayString::set(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont) {
	text = t;
	x = px;
	y = py;
	color = pcolor;
	
	right_to_left = rtl;
	reverse_string = reverse_str;
	
	double advanceWidth = 0;
	int text_width = 0;
	diskfont.GetTextWidth(text, text_width, advanceWidth);
	
	w = text_width < (PANEL_SIZE_X - x) ? text_width : PANEL_SIZE_X - x;		
	h = diskfont._FontHeader.charheight;
	
	dump();
}

bool DisplayString::IsEmpty() {
	return text == "" && x == 0 && y == 0 && w == 0 && h == 0 && color == GxEPD_WHITE && right_to_left == false && reverse_string == false;
}

DisplayString::DisplayString(const DisplayString& d2) { // copy constructor
	x 				= d2.x;
	y 				= d2.y;
	w 				= d2.w;
	h	 			= d2.h;
	text 			= d2.text;
	right_to_left 	= d2.right_to_left;
	reverse_string 	= d2.reverse_string;
	color	 		= d2.color;
}

void DisplayString::dump() {
	DEBUG_PRT.println("Text: " + text);
	DEBUG_PRT.printf("x:%d, y:%d, w:%d, h:%d, color:%d, right_to_left:%d, reverse_string:%d\n\n", x, y, w, h, color, right_to_left, reverse_string);
	DEBUG_PRT.printf("IsEmpty: %s\n", IsEmpty() ? "true" : "false");
}

/*
int compare(DisplayString*& a, DisplayString*& b) {
return a->y - b->y;
}
*/

TextBuffer::TextBuffer() {
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
	DisplayString* d = new DisplayString(text, x, y, color, right_to_left, reverse_string, diskfont);
	displayStringsList.add(d);
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

bool TextBuffer::render(GxEPD_Class& ePaper, DiskFont& diskfont, int displayPage) {	
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
		diskfont.DrawStringAt(ds->x, ds->y, ds->text, ePaper, ds->color, ds->right_to_left, ds->reverse_string);
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
