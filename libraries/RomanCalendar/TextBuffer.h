#pragma once
#ifndef _TEXTBUFFER_H
#define _TEXTBUFFER_H

#include "RCGlobals.h"

#include "Arduino.h"
#include "I2CSerialPort.h"
#include "LinkedList.h"

//#include <epd2in7b.h>
//#include <epdpaint.h>
#include <Diskfont.h>

#include <GxEPD.h>
#include <GxGDEW027C44/GxGDEW027C44.h>      // 7.5" b/w  640x384 GxGDEW075T8/GxGDEW075T8.cpp  // 2.7" color 176x264 GxGDEW027C44/GxGDEW027C44.cpp

class DisplayString
{
public:
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	String text = "";

	bool right_to_left = false;
	bool reverse_string = false;
	uint16_t color = GxEPD_WHITE;

	DisplayString();
	
	DisplayString(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont);
	DisplayString(String t, int px, int py, DiskFont& diskfont);
	DisplayString(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont);
	DisplayString(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont);

	void set(String t, int px, int py, DiskFont& diskfont);
	void set(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont);
	void set(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont);
	void set(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont);
	
	bool IsEmpty();
	
	DisplayString(const DisplayString& d2);
	
	void dump();
};

/*
int compare(DisplayString*& a, DisplayString*& b) {
	return a->y - b->y;
}
*/

class TextBuffer 
{
public:
	LinkedList<DisplayString*> displayStringsList = LinkedList<DisplayString*>();
	
	TextBuffer();	
	~TextBuffer();
	
	void add(DisplayString* d);
	void add(int x, int y, String text, uint16_t color, bool right_to_left, bool reverse_string, DiskFont& diskfont);
	
	bool get(int displayListEntryNumber, DisplayString* displayString);

/*	
	void sort_on_y();
*/

	bool render(GxEPD_Class& ePaper, DiskFont& diskfont, int displayPage);

	void dump();
};
#endif