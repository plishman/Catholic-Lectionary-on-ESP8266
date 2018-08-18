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
extern int image_size;

class DisplayString
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	String text = "";

	bool bRed = false;
	int colored = UNCOLORED;
	
	bool IsEmpty() {
		return text == "" && x == 0 && y == 0 && width == 0 && height == 0 && bRed == false && colored == UNCOLORED;
	}
	
	DisplayString(const DisplayString& d2) { // copy constructor
		 x = d2.x;
		 y = d2.y;
		 width = d2.width;
		 height = d2.height;
		 text = d2.text;
	}
}

int compare_y(DisplayString& a, DisplayString& b) {
  return a.y - b.y;
}

class TextBuffer 
{
	LinkedList<DisplayString*> displayStringsList = LinkedList<DisplayString*>();
	
	TextBuffer();
	
	~TextBuffer() {
		displayStringsList.clear();
	}
	
	void add(DisplayString d) {
		displayStringsList.add(d);
	}
	
	bool get(int displayListEntryNumber, DisplayString& displayString) {
		displayString = displayStringsList.get(displayListEntryNumber);
		
		return displayString.IsEmpty();
	}
	
	void sort_on_y() {
		displayStringsList.sort(compare_y);
	}
		
	bool epd_init(Epd& epd) {
		I2CSerial.printf("Initializing panel...");
		if (epd.Init() != 0) {
			I2CSerial.printf("e-Paper init failed\n");
			return false;
		}
		epd.ClearFrame();
		I2CSerial.printf("done\n");
		return true;
	}

	bool get(int& displayListEntryNumber, DisplayString& displayString, int& offset_x, int& offset_y, int x0, int y0, int x1, int y1) {
		displayString = displayStringsList.get(displayListEntryNumber);
		
		if (displayString.IsEmpty()) return false;
		
		int dx0 = displayString.x;
		int dy0 = displayString.y;
		int dx1 = displayString.x + displayString.w;
		int dy1 = displayString.y + displayString.h;
		
		
		if ((dx0 <= x0 && dy0 <= y0 && dx1 <= x1  && dy1 <= y1)  ||
		    (dx0 >= x0 && dy0 <= y0 &&  x1 <= dx1 && dy1 <= y1)  ||
		    (dx0 <= x0 && dy0 >= y0 && dx1 <= x1  &&  y1 <= dy1) ||
		    (dx0 >= x0 && dy0 >= y0 &&  x1 <= dx1 &&  y1 <= dy1) ||
		    (dx0 >= x0 && dy0 >= y0 && dx1 <= x1  && dy1 <= y1)) {
				
				offset_x = dx0 - x0;
				offset_y = dy0 - y0;
				return true;
			}
	}

	bool render(Epd& epd, diskfont& DiskFont, int bufsize, int bufwidthpx, int bufheightpx) {
		if (!epd_init(epd)) return false;
		
		sort_on_y();
		
		int numListEntries = displayStringsList.size();

		int w = bufwidthpx;
		int h = bufheightpx;

		Paint paint(image, h, w); //Y, X: 5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 // image is extern
		paint.SetRotate(ROTATE_90);
		
		for (int x = 0; x <= PANEL_SIZE_X; x+=w) {
			for (int y = 0; x <= PANEL_SIZE_Y; x+=h) {
			
				int offset_x = 0;
				int offset_y = 0;
				int displayListEntryNumber = 0;
				DisplayString ds;
				
				bool finished = false;

				paint.Clear(UNCOLORED);

				while (!finished) {
					if (get(displayListEntryNumber, ds, offset_x, offset_y, x, x+w, y, y+w)) {
						diskfont.DrawStringAt(0, 0, ds.text, &paint, COLORED, false, false);
					}
					displayListEntryNumber++;
				}			
			}
		}
	}	
};