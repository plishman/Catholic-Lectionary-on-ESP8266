#pragma once
#ifndef _TEXTBUFFER_H
#define _TEXTBUFFER_H

#include "Arduino.h"
#include "I2CSerialPort.h"
#include "LinkedList.h"

#include <epd2in7b.h>
#include <epdpaint.h>
#include <Diskfont.h>

extern const int COLORED;
extern const int UNCOLORED;

extern const int PANEL_SIZE_X;
extern const int PANEL_SIZE_Y;

extern unsigned char image[];
extern unsigned char image_red[];
extern int image_size;

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
	
	bool bRed = false;
	int colored = UNCOLORED;
	
	DisplayString(String t, int px, int py, DiskFont& diskfont) {
		DisplayString(t, px, py, false, false, diskfont);
	}
	
	DisplayString(String t, int px, int py, bool rtl, bool reverse_str, DiskFont& diskfont) {
		text = t;
		x = px;
		y = py;
		
		right_to_left = rtl;
		reverse_string = reverse_str;
		
		double advanceWidth = 0;
		int text_width = 0;
		diskfont.GetTextWidth(text, text_width, advanceWidth);
		
		w = text_width < (PANEL_SIZE_X - x) ? text_width : PANEL_SIZE_X - x;		
		h = diskfont._FontHeader.charheight;
	}
	
	bool IsEmpty() {
		return text == "" && x == 0 && y == 0 && w == 0 && h == 0 && bRed == false && colored == UNCOLORED &&  right_to_left == false && reverse_string == false;
	}
	
	DisplayString(const DisplayString& d2) { // copy constructor
		x 				= d2.x;
		y 				= d2.y;
		w 				= d2.w;
		h	 			= d2.h;
		text 			= d2.text;
		right_to_left 	= d2.right_to_left;
		reverse_string 	= d2.reverse_string;
		bRed 			= d2.bRed;
		colored 		= d2.colored;
	}
};

int compare_y(DisplayString*& a, DisplayString*& b) {
  return a->y - b->y;
}

class TextBuffer 
{
public:
	LinkedList<DisplayString*> displayStringsList = LinkedList<DisplayString*>();
	
	TextBuffer() {
		
	}
	
	~TextBuffer() {
		displayStringsList.clear();
	}
	
	void add(DisplayString d) {
		displayStringsList.add(&d);
	}
	
	bool get(int displayListEntryNumber, DisplayString* displayString) {
		if (displayStringsList.size() >= displayListEntryNumber) return false;
		
		displayString = displayStringsList.get(displayListEntryNumber);
		
		return displayString->IsEmpty();
	}
	
	void sort_on_y() {
		displayStringsList.sort(compare_y);
	}
		
	bool epd_init(Epd& epd) {
		Serial.printf("Initializing panel...");
		if (epd.Init() != 0) {
			Serial.printf("e-Paper init failed\n");
			return false;
		}
		epd.ClearFrame();
		Serial.printf("done\n");
		return true;
	}

	bool get(int& displayListEntryNumber, DisplayString* displayString, int& offset_x, int& offset_y, int x0, int y0, int x1, int y1) {
		if (displayListEntryNumber >= displayStringsList.size()) return false;
		
		displayString = displayStringsList.get(displayListEntryNumber);
		
		if (displayString == NULL) return false;
		
		//if (displayString.IsEmpty()) return false;
		
		int dx0 = displayString->x;
		int dy0 = displayString->y;
		int dx1 = displayString->x + displayString->w;
		int dy1 = displayString->y + displayString->h;
		
		
		if ((dx0 <= x0 && dy0 <= y0 && dx1 <= x1  && dy1 <= y1)  ||
		    (dx0 >= x0 && dy0 <= y0 &&  x1 <= dx1 && dy1 <= y1)  ||
		    (dx0 <= x0 && dy0 >= y0 && dx1 <= x1  &&  y1 <= dy1) ||
		    (dx0 >= x0 && dy0 >= y0 &&  x1 <= dx1 &&  y1 <= dy1) ||
		    (dx0 >= x0 && dy0 >= y0 && dx1 <= x1  && dy1 <= y1)) {
				
			offset_x = dx0 - x0;
			offset_y = dy0 - y0;
			return true;
		}
		
		return false;
	}

	bool render(Epd& epd, DiskFont& diskfont, int bufsize, int bufwidthpx, int bufheightpx) {
		if (!epd_init(epd)) return false;
		
		int neededbuffsize = (bufwidthpx * bufheightpx) / 8;
		
		if (neededbuffsize > bufsize) {
			Serial.printf("Insufficient memory: needed buffer is %d bytes, available buffer is %d bytes\n", neededbuffsize, bufsize);
			return false;
		}
		
		sort_on_y();
		
		int numListEntries = displayStringsList.size();

		int w = bufwidthpx;
		int h = bufheightpx;

		Paint paint_black(image, h, w); //Y, X: 5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 // image is extern
		paint_black.SetRotate(ROTATE_90);

		Paint paint_red(image_red, h, w); //Y, X: 5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 // image_red is extern
		paint_red.SetRotate(ROTATE_90);
		
		for (int x = 0; x <= PANEL_SIZE_X; x+=w) {
			for (int y = 0; x <= PANEL_SIZE_Y; x+=h) {
			
				int offset_x = 0;
				int offset_y = 0;
				int displayListEntryNumber = 0;
				DisplayString* ds;
				
				bool started = false;
				bool finished = false;

				paint_black.Clear(UNCOLORED);
				paint_red.Clear(UNCOLORED);

				while (!finished) {
					if (get(displayListEntryNumber, ds, offset_x, offset_y, x, y, x+w, y+w)) {
						if (!ds->bRed) {
							diskfont.DrawStringAt(offset_x, offset_y, ds->text, &paint_black, ds->colored, ds->right_to_left, ds->reverse_string);
						}
						else {
							diskfont.DrawStringAt(offset_x, offset_y, ds->text, &paint_red, ds->colored, ds->right_to_left, ds->reverse_string);							
						}
						started = true;
					} else {
						if (started) {
							finished = true;
						}
					}
					displayListEntryNumber++;
				}
				
				if (started) {
					epd.TransmitPartialBlack(paint_black.GetImage(), x, y, paint_black.GetWidth(), paint_black.GetHeight());    
					epd.TransmitPartialRed(paint_red.GetImage(), x, y, paint_red.GetWidth(), paint_red.GetHeight());    
				}
			}
		}
		epd.DisplayFrame();
  		epd.Sleep();
	}	
};
#endif