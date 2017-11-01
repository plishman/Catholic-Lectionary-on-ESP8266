/**
 *  @filename   :   epdpaint.cpp
 *  @brief      :   Paint tools
 *  @author     :   Yehui from Waveshare
 *  
 *  Copyright (C) Waveshare     September 9 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <pgmspace.h>
#include <arduino.h>
#include "epdpaint.h"

Paint::Paint(unsigned char* image, int width, int height) {
    Serial.println("paint constructor");
	this->rotate = ROTATE_0;
    this->image = image;
    /* 1 byte = 8 pixels, so the width should be the multiple of 8 */
    this->width = width % 8 ? width + 8 - (width % 8) : width;
    this->height = height;
    Serial.println("done");
}

Paint::~Paint() {
}

/**
 *  @brief: clear the image
 */
void Paint::Clear(int colored) {
    for (int x = 0; x < this->width; x++) {
        for (int y = 0; y < this->height; y++) {
            DrawAbsolutePixel(x, y, colored);
        }
    }
}

/**
 *  @brief: this draws a pixel by absolute coordinates.
 *          this function won't be affected by the rotate parameter.
 */
void Paint::DrawAbsolutePixel(int x, int y, int colored) {
    if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
        return;
    }
    if (IF_INVERT_COLOR) {
        if (colored) {
            image[(x + y * this->width) / 8] |= 0x80 >> (x % 8);
        } else {
            image[(x + y * this->width) / 8] &= ~(0x80 >> (x % 8));
        }
    } else {
        if (colored) {
            image[(x + y * this->width) / 8] &= ~(0x80 >> (x % 8));
        } else {
            image[(x + y * this->width) / 8] |= 0x80 >> (x % 8);
        }
    }
}

/**
 *  @brief: Getters and Setters
 */
unsigned char* Paint::GetImage(void) {
    return this->image;
}

int Paint::GetWidth(void) {
    return this->width;
}

void Paint::SetWidth(int width) {
    this->width = width % 8 ? width + 8 - (width % 8) : width;
}

int Paint::GetHeight(void) {
    return this->height;
}

void Paint::SetHeight(int height) {
    this->height = height;
}

int Paint::GetRotate(void) {
    return this->rotate;
}

void Paint::SetRotate(int rotate){
    this->rotate = rotate;
}

/**
 *  @brief: this draws a pixel by the coordinates
 */
void Paint::DrawPixel(int x, int y, int colored) {
    int point_temp;
    if (this->rotate == ROTATE_0) {
        if(x < 0 || x >= this->width || y < 0 || y >= this->height) {
            return;
        }
        DrawAbsolutePixel(x, y, colored);
    } else if (this->rotate == ROTATE_90) {
        if(x < 0 || x >= this->height || y < 0 || y >= this->width) {
          return;
        }
        point_temp = x;
        x = this->width - y;
        y = point_temp;
        DrawAbsolutePixel(x, y, colored);
    } else if (this->rotate == ROTATE_180) {
        if(x < 0 || x >= this->width || y < 0 || y >= this->height) {
          return;
        }
        x = this->width - x;
        y = this->height - y;
        DrawAbsolutePixel(x, y, colored);
    } else if (this->rotate == ROTATE_270) {
        if(x < 0 || x >= this->height || y < 0 || y >= this->width) {
          return;
        }
        point_temp = x;
        x = y;
        y = this->height - point_temp;
        DrawAbsolutePixel(x, y, colored);
    }
}

/**
 *  @brief: this draws a charactor on the frame buffer but not refresh
 */
void Paint::DrawCharAt(int x, int y, char ascii_char, sFONT* font, int colored) {
    int i, j;
    unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char* ptr = &font->table[char_offset];

    for (j = 0; j < font->Height; j++) {
        for (i = 0; i < font->Width; i++) {
            if (pgm_read_byte(ptr) & (0x80 >> (i % 8))) {
                DrawPixel(x + i, y + j, colored);
            }
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (font->Width % 8 != 0) {
            ptr++;
        }
    }
}

/**
*  @brief: this displays a string on the frame buffer but not refresh
*/
void Paint::DrawStringAt(int x, int y, const char* text, sFONT* font, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;
    
    /* Send the string character by character on EPD */
    while (*p_text != 0) {
        /* Display one character on EPD */
        DrawCharAt(refcolumn, y, *p_text, font, colored);
        /* Decrement the column position by 16 */
        refcolumn += font->Width;
        /* Point on the next character */
        p_text++;
        counter++;
    }
}

/**
 *  @brief: this draws a character on the frame buffer but not refresh [uses the dot factory proportional font]
 */
int Paint::DrawCharAt(int x, int y, char ascii_char, const FONT_INFO* font, int colored) {	
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), font, colored);
}
 
int Paint::DrawCharAt(int x, int y, int codepoint, const FONT_INFO* font, int colored) {	
	if (codepoint == 32) return font->spacePixels; // space character

	int charIndex = codepoint - font->startChar;
	printf("char codepoint=%s, font->startChar = %d, char_index=%d\n", utf8fromCodepoint(codepoint).c_str(), font->startChar, charIndex);

	uint8_t char_height = font->heightPages;
	printf("char_height=%d\n", char_height);

	uint8_t char_width = pgm_read_byte(&(font->charInfo[charIndex].widthBits));
	printf("char_width=%d\n", char_width);

	uint32_t char_offset = pgm_read_dword(&(font->charInfo[charIndex].offset));
	printf("char_offset=%d\n", char_offset);
	
    int i, j;
    //unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const uint8_t* ptr = &font->data[char_offset];
	
    for (j = 0; j < char_height; j++) {
		printf("\n%d:\t|", j);
        for (i = 0; i < char_width; i++) {
            if (pgm_read_byte(ptr) & (0x80 >> (i % 8))) {
                DrawPixel(x + i, y + j, colored);
				printf("#");
            } else {
				printf(" ");
			}
			
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (char_width % 8 != 0) {
            ptr++;
        }
    }
	
	printf("char width=%d\n", char_width);
	
	return char_width + 1;
}

/**
*  @brief: this displays a string on the frame buffer but not refresh [uses the dot factory proportional font]
*/
void Paint::DrawStringAt(int x, int y, const char* text, const FONT_INFO* font, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;

	printf("text=%s\n", text);
    
    /* Send the string character by character on EPD */
    while (*p_text != 0) {
		printf("counter=%d, refcolumn=%d\n", counter, refcolumn);
        /* Display one character on EPD */
        refcolumn += DrawCharAt(refcolumn, y, *p_text, font, colored);
        /* Decrement the column position by 16 */
        //refcolumn += font->Width;
        /* Point on the next character */
        p_text++;
        counter++;
    }
}

void Paint::DrawStringAt(int x, int y, String text, const FONT_INFO* font, int colored) {
	int charIndex = 0;
	int len = text.length();
	
    int refcolumn = x;
	String ch;
	
	printf("text=%s\n", text.c_str());
    
    /* Send the string character by character on EPD */
    while (charIndex != len) {
		printf("refcolumn=%d\n", refcolumn);
        /* Display one character on EPD */
		ch = utf8CharAt(text, charIndex);
        refcolumn += DrawCharAt(refcolumn, y, codepointUtf8(ch), font, colored);
        /* Decrement the column position by 16 */
        //refcolumn += font->Width;
        /* Point on the next character */
        charIndex += ch.length();
    }
}

int Paint::GetTextWidth(const char* text, const FONT_INFO* font) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = 0;
	int charIndex;

    while (*p_text != 0) {
		if (*p_text == 32) {
			refcolumn += font->spacePixels;
		} else {
			charIndex = (int)(*p_text - font->startChar);
			refcolumn += (int)(pgm_read_byte(&(font->charInfo[charIndex].widthBits))) + 1;
		}

        p_text++;
        counter++;
    }
	return refcolumn;
}

int Paint::GetTextWidth(String text, const FONT_INFO* font) {
	int charIndex = 0;
	int i = 0;
	int len = text.length();
    int refcolumn = 0;
	String ch;

    while (i < len) {
		ch = utf8CharAt(text, i);
		if (ch == " ") {
			refcolumn += font->spacePixels;
		} else {
			charIndex = (int)(codepointUtf8(ch) - font->startChar);
			refcolumn += (int)(pgm_read_byte(&(font->charInfo[charIndex].widthBits))) + 1;
		}

		i += ch.length();
	}
	return refcolumn;
}


int Paint::codepointUtf8(String c) {
	unsigned char b;
	int u = 0;

	const char* ch = c.c_str();

	int len = c.length();

	b = (ch[0] & 0x80);
	if (b == 0 && len > 0) return (int)ch[0];	   // 1 byte character, u=0-0x7f

	b = (ch[0] & 0xE0);
	if (b == 0xC0 && len > 1) return (int)(((ch[0] & 0x1f) << 6) | (ch[1] & 0x3F)); // 2 byte character, u=0x80-0x7ff

	b = (ch[0] & 0xF0);
	if (b == 0xE0 && len > 2) return (int)(((ch[0] & 0x0f) << 12) | ((ch[1] & 0x3F) << 6) | (ch[2] & 0x3F)); // 3 byte character, u=0x800-0xFFFF

	b = (ch[0] & 0xF8);
	if (b == 0xF0 && len > 3) return (int)(((ch[0] & 0x07) << 18) | ((ch[1] & 0x3F) << 12) | ((ch[2] & 0x3F) << 6) | (ch[3] & 0x3F)); // 4 byte character, u=0x800-0xFFFF

	return 0;
}

String Paint::utf8fromCodepoint(int c) {
	unsigned char char0;
	unsigned char char1;
	unsigned char char2;
	unsigned char char3;

	String ch;

	if (c < 0x80) {
		char0 = c & 0x7f;					// 1 byte
		char u[] = { char0, '\0' };
		return String(u);
	}

	if (c >= 0x80 && c < 0x800) {					// 2 bytes
		char1 = ((c & 0x3f) | 0x80);
		char0 = ((c & 0x7c0) >> 6 | 0xc0);
		char u[] = { char0, char1, '\0' };
		return String(u);
	}

	if (c >= 0x800 && c < 0x10000) {				// 3 bytes
		char2 = ((c & 0x3f) | 0x80);
		char1 = (((c & 0xfc0) >> 6) | 0x80);
		char0 = (((c & 0xf000)) >> 12 | 0xe0);
		char u[] = { char0, char1, char2, '\0' };
		return String(u);
	}

	if (c >= 0x10000 && c < 0x200000) {				// 4 bytes
		char3 = ((c & 0x3f) | 0x80);
		char2 = (((c & 0xfc0) >> 6) | 0x80);
		char1 = (((c & 0x3f000) >> 12) | 0x80);
		char0 = (((c & 0x1c0000)) >> 18 | 0xF0);
		char u[] = { char0, char1, char2, char3, '\0' };
		return String(u);
	}
	
	return String("");
}

String Paint::utf8CharAt(String s, int pos) { 
  //Serial.println("String=" + s);
  
  if (pos >= s.length()) {
    //Serial.println("utf8CharAt string length is " + String(ps->length()) + " *ppos = " + String(*ppos));
    return String("");
  }
  
  int charLen = charLenBytesUTF8(s.charAt(pos));

  //Serial.println("char at pos " + String(*ppos) + " = " + String(ps->charAt(*ppos)) + "utf8 charLen = " + String(charLen));

  if (charLen == 0) {
    return String("");
  } 
  else {
    //Serial.print("substring is" + s.substring(pos, pos+charLen));
    return s.substring(pos, pos + charLen);
  }
}

int Paint::charLenBytesUTF8(char s) {
  byte ch = (byte) s;
  //Serial.println(String(ch)+ ";");

  byte b;
 
  b = (ch & 0xE0);  // 2-byte utf-8 characters start with 110xxxxx
  if (b == 0xC0) return 2;

  b = (ch & 0xF0);  // 3-byte utf-8 characters start with 1110xxxx
  if (b == 0xE0) return 3;

  b = (ch & 0xF8);  // 4-byte utf-8 characters start with 11110xxx
  if (b == 0xF0) return 4;

  b = (ch & 0xC0);  // bytes within multibyte utf-8 characters are 10xxxxxx
  if (b == 0x80) return 0; //somewhere in a multi-byte utf-8 character, so don't know the length. Return 0 so the scanner can keep looking

  return 1; // character must be 0x7F or below, so return 1 (it is an ascii character)
}

/**
*  @brief: this draws a line on the frame buffer
*/
void Paint::DrawLine(int x0, int y0, int x1, int y1, int colored) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int D = 2*dy - dx;
  int y = y0;

  for (int x = x0; x < x1; x++) { 
    DrawPixel(x, y, colored);
    if (D > 0) {
       y = y + 1;
       D = D - 2 * dx;
    }
    D = D + 2*dy;
  }
/*
plotLine(x0,y0, x1,y1) (from Wikipedia)
  dx = x1 - x0
  dy = y1 - y0
  D = 2*dy - dx
  y = y0

  for x from x0 to x1
    plot(x,y)
    if D > 0
       y = y + 1
       D = D - 2*dx
    end if
    D = D + 2*dy
*/
}


/**
*  @brief: this draws a line on the frame buffer
*/
//void Paint::DrawLine(int x0, int y0, int x1, int y1, int colored) {
    /* Bresenham algorithm */
/*
    int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while((x0 != x1) && (y0 != y1)) {
        DrawPixel(x0, y0 , colored);
        if (2 * err >= dy) {     
            err += dy;
            x0 += sx;
        }
        if (2 * err <= dx) {
            err += dx; 
            y0 += sy;
        }
    }
}
*/
/**
*  @brief: this draws a horizontal line on the frame buffer
*/
void Paint::DrawHorizontalLine(int x, int y, int line_width, int colored) {
    int i;
    for (i = x; i < x + line_width; i++) {
        DrawPixel(i, y, colored);
    }
}

/**
*  @brief: this draws a vertical line on the frame buffer
*/
void Paint::DrawVerticalLine(int x, int y, int line_height, int colored) {
    int i;
    for (i = y; i < y + line_height; i++) {
        DrawPixel(x, i, colored);
    }
}

/**
*  @brief: this draws a rectangle
*/
void Paint::DrawRectangle(int x0, int y0, int x1, int y1, int colored) {
    int min_x, min_y, max_x, max_y;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    DrawHorizontalLine(min_x, min_y, max_x - min_x + 1, colored);
    DrawHorizontalLine(min_x, max_y, max_x - min_x + 1, colored);
    DrawVerticalLine(min_x, min_y, max_y - min_y + 1, colored);
    DrawVerticalLine(max_x, min_y, max_y - min_y + 1, colored);
}

/**
*  @brief: this draws a filled rectangle
*/
void Paint::DrawFilledRectangle(int x0, int y0, int x1, int y1, int colored) {
    int min_x, min_y, max_x, max_y;
    int i;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    for (i = min_x; i <= max_x; i++) {
      DrawVerticalLine(i, min_y, max_y - min_y + 1, colored);
    }
}

/**
*  @brief: this draws a circle
*/
void Paint::DrawCircle(int x, int y, int radius, int colored) {
    /* Bresenham algorithm */
    int x_pos = -radius;
    int y_pos = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        DrawPixel(x - x_pos, y + y_pos, colored);
        DrawPixel(x + x_pos, y + y_pos, colored);
        DrawPixel(x + x_pos, y - y_pos, colored);
        DrawPixel(x - x_pos, y - y_pos, colored);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
              e2 = 0;
            }
        }
        if (e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while (x_pos <= 0);
}

/**
*  @brief: this draws a filled circle
*/
void Paint::DrawFilledCircle(int x, int y, int radius, int colored) {
    /* Bresenham algorithm */
    int x_pos = -radius;
    int y_pos = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        DrawPixel(x - x_pos, y + y_pos, colored);
        DrawPixel(x + x_pos, y + y_pos, colored);
        DrawPixel(x + x_pos, y - y_pos, colored);
        DrawPixel(x - x_pos, y - y_pos, colored);
        DrawHorizontalLine(x + x_pos, y + y_pos, 2 * (-x_pos) + 1, colored);
        DrawHorizontalLine(x + x_pos, y - y_pos, 2 * (-x_pos) + 1, colored);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
                e2 = 0;
            }
        }
        if(e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while(x_pos <= 0);
}

/* END OF FILE */























