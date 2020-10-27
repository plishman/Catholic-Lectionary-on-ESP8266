#pragma once
#ifndef _TEXTBUFFER_H
#define _TEXTBUFFER_H

#include "RCGlobals.h"

#include "Arduino.h"
#include "DebugPort.h"
#include "LinkedList.h"

#include "Diskfont.h"
#include "FrameBuffer.h"

#ifdef EPAPER_GxGDEW027C44
	#include <GxEPD.h>
	#include <GxGDEW027C44/GxGDEW027C44.h>      // 2.7" b/w/r 176x264 GxGDEW027C44/GxGDEW027C44.cpp
#endif

#ifdef EPAPER_GxGDEW042Z15
	#include <GxEPD.h>
	#include <GxGDEW042Z15/GxGDEW042Z15.h>      // 4.2" b/w/r
#endif

class DisplayString
{
public:
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	double advance_width = 0.0;
	int8_t line_num = -1;
	double space_char_width = -1.0;
	bool b_use_default_spacechar_width = true;
	int wordcount = 0;
	String text = "";
	String colormap = "";
	
	bool right_to_left = false;
	bool reverse_string = false;
	uint16_t color = GxEPD_WHITE;

	DiskFont* pDiskFont = NULL;

	DisplayString();
	
	DisplayString(String t, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number = -1);
	DisplayString(String t, int px, int py, DiskFont& diskfont, int8_t line_number = -1);
	DisplayString(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number = -1);
	DisplayString(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont, int8_t line_number = -1);

	DisplayString(String t, String cmap, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number = -1);
	DisplayString(String t, String cmap, int px, int py, DiskFont& diskfont, int8_t line_number = -1);
	
	void set(String t, int px, int py, DiskFont& diskfont, int8_t line_number = -1);
	void set(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number = -1);
	void set(String t, int px, int py, uint16_t pcolor, DiskFont& diskfont, int8_t line_number = -1);
	
	void set(String t, String cmap, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number = -1);
	void set(String t, String cmap, int px, int py, DiskFont& diskfont, int8_t line_number = -1);

	void set(String t, String cmap, int px, int py, uint16_t pcolor, bool rtl, bool reverse_str, DiskFont& diskfont, int8_t line_number = -1); // if cmap == "", pcolor is used to colour whole string
	
	bool IsEmpty();
	
	DisplayString(const DisplayString& d2);
	
	void dump();
};

/*
int compare(DisplayString*& a, DisplayString*& b) {
	return a->y - b->y;
}
*/

#define TB_FORMAT_NONE 0
#define TB_FORMAT_CENTRE 1
#define TB_FORMAT_JUSTIFY 2
#define TB_FORMAT_LJUSTIFY 0 // default (does not do anything to text format)
#define TB_FORMAT_RJUSTIFY 3

class TextBuffer 
{
public:
	LinkedList<DisplayString*> displayStringsList = LinkedList<DisplayString*>();
	
	TextBuffer(FB_EPAPER ePaper);	
	~TextBuffer();
	
	FB_EPAPER_PTR _p_ePaper = NULL;
	
	void add(DisplayString* d);
	void add(int x, int y, String text, uint16_t color, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number = -1);
	void add(int x, int y, String text, String colormap, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number = -1);

	void add_buffered(int x, int y, String text, uint16_t color, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number);
	void add_buffered(int x, int y, String text, String colormap, bool right_to_left, bool reverse_string, DiskFont& diskfont, int8_t line_number);
	void format_buffer(int fbwidth, uint8_t format_action, int8_t line_number = -1);
	int16_t typeset(int8_t line_number, int fbwidth, uint8_t format_action, bool bFlushNow = true); // format_action is TB_FORMAT_NONE, TB_FORMAT_JUSTIFY or TB_FORMAT_CENTRE
	void flush();

	bool get(int displayListEntryNumber, DisplayString* displayString);

/*	
	void sort_on_y();
*/

	bool render(FB_EPAPER ePaper, DiskFont& diskfont, int displayPage);

	void dump();
};
#endif