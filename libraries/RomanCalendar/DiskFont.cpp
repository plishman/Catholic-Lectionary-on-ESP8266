#include <DiskFont.h>
extern "C" {
#include "user_interface.h"
}

bool DiskFont::OpenFontFile() {
	_file_sd = SD.open(_fontfilename.c_str(), FILE_READ);

	if (!_file_sd.available()) {
		Serial.printf("unable to open diskfont from SD card, trying spiffs%s\n", _fontfilename.c_str());
	}
	else {
		Serial.printf("Opened font file from SD Card\n");
		return true;
	}

	_file_spiffs = SPIFFS.open(_fontfilename.c_str(), "r");

	if (!_file_spiffs.available()) {
		Serial.printf("diskfont file not found%s\n", _fontfilename.c_str());
		return false;
	}
	else {
		Serial.printf("Opened font file from SPIFFS\n");
		return true;
	}
	
	return false;
}

bool DiskFont::CloseFontFile() {
	available = false;

	if (_file_sd.available()) {
		_file_sd.close();
	}	

	if (_file_spiffs.available()) {
		_file_spiffs.close();
	}	
}

bool DiskFont::Seek(uint64_t offset) {
	if (_file_sd) {
		return _file_sd.seek(offset);
	}

	if (_file_spiffs) {
		return _file_spiffs.seek(offset, fs::SeekSet);
	}
	
	return false;
}

size_t DiskFont::Position() {
	if (_file_sd) {
		return _file_sd.position();
	}

	if (_file_spiffs) {
		return _file_spiffs.position();
	}
	
	return 0;
}

size_t DiskFont::Size() {
	if (_file_sd) {
		return _file_sd.size();
	}

	if (_file_spiffs) {
		return _file_spiffs.size();
	}
	
	return 0;
}

bool DiskFont::Read(uint32_t* var) {
	if (_file_sd) {
		_file_sd.read(var, sizeof(*var)); // should be 4 bytes
		return true;
	} 
		
	if (_file_spiffs) {
		_file_spiffs.read((uint8_t*)var, sizeof(*var));		
		return true;
	} 
	
	return false;
	
}

bool DiskFont::Read(uint16_t* var) {
	if (_file_sd) {
		_file_sd.read(var, sizeof(*var)); // should be 2 bytes
		return true;
	} 
		
	if (_file_spiffs) {
		_file_spiffs.read((uint8_t*)var, sizeof(*var));
		return true;
	} 
	
	return false;
}

bool DiskFont::Read(uint8_t* var) {
	if (_file_sd) {
		_file_sd.read(var, sizeof(*var)); // should be 1 bytes
		return true;
	} 
		
	if (_file_spiffs) {
		_file_spiffs.read(var, sizeof(*var));		
		return true;
	} 
	
	return false;
}

bool DiskFont::Read(double* var) {
	if (_file_sd) {
		_file_sd.read(var, sizeof(*var)); // should be 8 bytes
		return true;
	} 
		
	if (_file_spiffs) {
		_file_spiffs.read((uint8_t*)var, sizeof(*var));
		return true;
	} 
	
	return false;
}

bool DiskFont::Read(void* array, uint32_t bytecount) { // no way of checking bounds, so careful with this function
	if (_file_sd) {
		_file_sd.read(array, bytecount);
		return true;
	} 
		
	if (_file_spiffs) {
		_file_spiffs.read((uint8_t*)array, bytecount);		
		return true;
	} 
	
	return false;
}




DiskFont::DiskFont() {
	available = false; // must have called SD.begin() and SPIFFS.begin() somewhere first
}

DiskFont::~DiskFont() {
	end();
}

bool DiskFont::begin(ConfigParams c) {
/*
		desc = "";
		lang = "";
		yml_filename = "";
		sanctorale_filename = "";
		bible_filename = "";
		font_filename = "builtin";
		transfer_to_sunday = true;
		celebrate_feast_of_christ_priest = true;
		have_config = false;
		right_to_left = false;
		font_tuning_percent = 50.0;
		font_use_fixed_spacing = false;
		font_use_fixed_spacecharwidth = false;
		font_fixed_spacing = 1;
		font_fixed_spacecharwidth = 2;		
*/
	if (c.font_tuning_percent >= 0.0 && c.font_tuning_percent <= 100.0) {
		_font_tuning_percent = c.font_tuning_percent;
	}

	_font_use_fixed_spacing 		= c.font_use_fixed_spacing;
	_font_use_fixed_spacecharwidth 	= c.font_use_fixed_spacecharwidth;
	_font_fixed_spacing 			= c.font_fixed_spacing;
	_font_fixed_spacecharwidth 		= c.font_fixed_spacecharwidth;
	
	begin(c.font_filename);
}


bool DiskFont::begin(String fontfilename, double font_tuning_percent) {
	if (font_tuning_percent >= 0.0 && font_tuning_percent <= 100.0) {
		_font_tuning_percent = font_tuning_percent;
	}
	
	begin(fontfilename);
}

bool DiskFont::begin(String fontfilename) {
	_fontfilename = fontfilename;

	if (_fontfilename == "builtin" || !OpenFontFile()) {
		I2CSerial.printf("Diskfont is not selected (fontfilename is 'builtin' or font file not found\n");

		_FontHeader.charheight = romfont->heightPages;
		_FontHeader.startchar = romfont->startChar;
		_FontHeader.endchar = romfont->endChar;
		_FontHeader.numlookupblocks = romfont->blockCount;
		_FontHeader.spacecharwidth = romfont->spacePixels;
		_FontHeader.ascent = romfont->ascent;
		_FontHeader.descent = romfont->descent;
		_FontHeader.linespacing = romfont->lineheight;
		
		return false;
	}
	
/*
	_file = SD.open(_fontfilename.c_str(), FILE_READ);

	if (!_file.available()) {
		I2CSerial.printf("unable to open diskfont %s\n", _fontfilename.c_str());
		return false;
	}
*/
	//if (!OpenFontFile()) return false;

	/*
            const unsigned long FONT_HEADER_OFFSET_CHARHEIGHT = 8; // word
            const unsigned long FONT_HEADER_OFFSET_STARTCHAR = 10; // dword
            const unsigned long FONT_HEADER_OFFSET_ENDCHAR = 14; // dword
            const unsigned long FONT_HEADER_OFFSET_NUMLOOKUPENTRIES = 18; // word
            const unsigned long FONT_HEADER_OFFSET_SPACECHARWIDTH = 20; // byte
			const double 		FONT_HEADER_OFFSET_ASCENT = 21;
            const double 		FONT_HEADER_OFFSET_DESCENT = 29;
            const unsigned long FONT_HEADER_OFFSET_END = 37;
            const unsigned long BLOCKTABLE_OFFSET_BEGIN = FONT_HEADER_OFFSET_END;
	*/
	
	_FontHeader = {0};
	
	//_file.seek(FONT_HEADER_OFFSET_START);
	if (!Seek(FONT_HEADER_OFFSET_START)) {
		Serial.printf("couldn't find diskfont header\n");
		CloseFontFile();
		return false;
	};
	
	if (!(Read(&_FontHeader.charheight)
		&& Read(&_FontHeader.startchar)
		&& Read(&_FontHeader.endchar)
		&& Read(&_FontHeader.numlookupblocks)
		&& Read(&_FontHeader.spacecharwidth)
		&& Read(&_FontHeader.ascent)
		&& Read(&_FontHeader.descent)
		&& Read(&_FontHeader.linespacing))) 
	{		
		Serial.printf("Problem reading diskfont header\n");
		CloseFontFile();
		return false;
	}
	
	Serial.printf("FontHeader\ncharheight=%d\nstartchar=%d\nendchar=%d\nnumlookupblocks=%d\nspacecharwidth=%d\n\n", _FontHeader.charheight, _FontHeader.startchar, _FontHeader.endchar, _FontHeader.numlookupblocks, _FontHeader.spacecharwidth);
	
	
	// need to read blocktable (1024/3*4 = 85 blocktable entries/kB)
	uint32_t font_blocktablesize = _FontHeader.numlookupblocks * sizeof(DiskFont_BlocktableEntry);
	
	Serial.printf("Diskfont blocktable count = %d, size = %d\n", _FontHeader.numlookupblocks, font_blocktablesize);
	
	if (font_blocktablesize > (system_get_free_heap_size() - 10240)) {
		//_I18n.closeFile(_file); // not enough memory for blocktable (leaves a minimum of 10k free)
		Serial.printf("Not enough room for diskfont blocktable\n");
		CloseFontFile();
		return false;
	}
	
	_Font_BlocktablePtr = new DiskFont_BlocktableEntry[_FontHeader.numlookupblocks];

	if (_Font_BlocktablePtr == NULL) {
		//_I18n.closeFile(_file);
		Serial.printf("Could not allocate memory for diskfont blocktable\n");
		CloseFontFile();	// 'new' failed (shouldn't happen, since we calculated the available memory beforehand, but still...)
		return false;
	}
	
	Read(_Font_BlocktablePtr, font_blocktablesize);
	
	available = true;
	
	Serial.printf("Diskfont is available\n");
	
	return true; // ready to access font
}

void DiskFont::end() {
	//_I18n.closeFile(_file);
	available = false;
	if (_Font_BlocktablePtr != NULL) delete _Font_BlocktablePtr;	
	CloseFontFile();
}


/**
 *  @brief: this draws a character on the frame buffer but not refresh [uses the dot factory proportional font]
 */
// No Pre-populated FontCharInfo structure (in), and no advanceWidth (out)
int DiskFont::DrawCharAt(int x, int y, char ascii_char,    Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), paint, colored, blockToCheckFirst);
}

int DiskFont::DrawCharAt(int x, int y, String ch,          Paint* paint, int colored, uint16_t* blockToCheckFirst) {
	return DrawCharAt(x, y, codepointUtf8(ch),                 paint, colored, blockToCheckFirst);
}

int DiskFont::DrawCharAt(int x, int y, uint32_t codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst) {
	double advwidth = 0.0;
	return DrawCharAt(x, y, codepoint, advwidth, paint, colored, blockToCheckFirst);
}



// No Pre-populated FontCharInfo structure (in), but advanceWidth (out)
int DiskFont::DrawCharAt(int x, int y, char ascii_char,    double& advanceWidth, Paint* paint, int colored, uint16_t* blockToCheckFirst) {
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), advanceWidth, paint, colored, blockToCheckFirst);
}

int DiskFont::DrawCharAt(int x, int y, String ch,          double& advanceWidth, Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	return DrawCharAt(x, y, codepointUtf8(ch),                 advanceWidth, paint, colored, blockToCheckFirst);
}

int DiskFont::DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	DiskFont_FontCharInfo fci;
	if (!getCharInfo(codepoint, blockToCheckFirst, &fci)) {
		I2CSerial.printf("DrawCharAt() getCharInfo returned false\n");
		return 0;
	}
	
	return DrawCharAt(x, y, codepoint, advanceWidth, fci, paint, colored);
}



// Pre-populated FontCharInfo structure (in), no advanceWidth (out)
int DiskFont::DrawCharAt(int x, int y, char ascii_char,    DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), fci, paint, colored);
}

int DiskFont::DrawCharAt(int x, int y, String ch,          DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	return DrawCharAt(x, y, codepointUtf8(ch),                 fci, paint, colored);
}

int DiskFont::DrawCharAt(int x, int y, uint32_t codepoint, DiskFont_FontCharInfo& fci, Paint* paint, int colored) {
	double advanceWidth = 0.0;
	return DrawCharAt(x, y, codepoint, advanceWidth, fci, paint, colored);
}


 
// Pre-populated FontCharInfo structure (in), and advanceWidth (out)
int DiskFont::DrawCharAt(int x, int y, char ascii_char,    double& advanceWidth, DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), advanceWidth, fci, paint, colored);
}

int DiskFont::DrawCharAt(int x, int y, String ch,          double& advanceWidth, DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	return DrawCharAt(x, y, codepointUtf8(ch),                 advanceWidth, fci, paint, colored);
} 
 
int DiskFont::DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	// allows pre-populated FontCharInfo structure to be passed in, for use where it would otherwise have to read from the disk more than once for each character

	//if (codepoint == 32) return _FontHeader.spacecharwidth; // space character

	if(!available) {
		//Serial.printf("DrawCharAt() Diskfont is not available.\n");
		return DrawCharAt(x, y, codepoint, advanceWidth, romfont, fci, paint, colored);
	}
	
//	DiskFont_FontCharInfo fci;
//	if (!getCharInfo(codepoint, blockToCheckFirst, &fci)) {
//		I2CSerial.printf("DrawCharAt() getCharInfo returned false\n");
//		return 0;
//	}

	uint16_t char_width = fci.widthbits;
	//I2CSerial.printf("char_width=%d\n", char_width);

	if (codepoint == 32) {
		//Serial.printf("DrawCharAt: space char width = %d\n", (int)fci.advanceWidth);
		advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth, codepoint, char_width); //fci.advanceWidth;
		return char_width; //fci.widthbits;
	}
	else {
		//Serial.printf("ch=%s, u+%x advanceWidth=%d\n", utf8fromCodepoint(codepoint).c_str(), codepoint, (int)fci.advanceWidth);
	}
	
	//int charIndex = codepoint - font->startChar;
	//I2CSerial.printf("char codepoint=%s, font->startChar = %d, char_index=%d\n", utf8fromCodepoint(codepoint).c_str(), font->startChar, charIndex);

	uint16_t char_height = _FontHeader.charheight;
	//I2CSerial.printf("char_height=%d\n", char_height);

	////uint32_t char_offset = pgm_read_dword(&(font->charInfo[charIndex].offset));
	uint32_t char_offset = fci.bitmapfileoffset;
	//printf("char_offset=%d\n", char_offset);
	
    int i, j;
    //unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    //const uint8_t* ptr = &font->data[char_offset];
	
	//int charbuffsize = 32;
	int buf_offset = 0;	
	int bytesremaining = 0;
	
	//I2CSerial.printf("-%x\t", fci.bitmapfileoffset);
	
	Seek(fci.bitmapfileoffset);
	
	// calculate the number of bytes needed to store the given number of bits (the character width in bits) in fci.widthbits
	if (fci.widthbits == 0) return 0; // zero-width character - return 0 for the character width.

	int numbytes = 0;
	int numbits = fci.widthbits;
	
	do {
			numbytes++;
			numbits-=8;
	} while (numbits > 0);
	
	bytesremaining = numbytes * fci.heightbits;
	
	//I2CSerial.printf("bytesremaining = %d\n", bytesremaining);
	
	for (j = 0; j < char_height; j++) {
		//printf("\n%d:\t|", j);
		for (i = 0; i < char_width; i++) {

			if (buf_offset == 0 && i % 8 == 0) {
				if ((Size() - Position()) > bytesremaining) {
					int bytestoread = CHARBUFF_SIZE;
					if (bytesremaining < CHARBUFF_SIZE) bytestoread = bytesremaining;
					Read(_char_buffer, bytestoread);
					bytesremaining -= bytestoread;
					/* 
					//debugging
					for (int o=0;o<CHARBUFF_SIZE;o++) {
						I2CSerial.printf("%x ", _char_buffer[o]);
					}
					I2CSerial.printf("\n");
					*/
				}
				else {
					// error - found unexpected end of file
					I2CSerial.printf("Diskfont Error (unexpected EOF)\n");
					return char_width + 1;
				}
			}

			if (_char_buffer[buf_offset] & (0x80 >> (i % 8))) {
				paint->DrawPixel(x + i, y + j, colored);
				//Serial.printf("#");
			} else {
				//Serial.printf(" ");
			}
										
			if (i % 8 == 7) {
				buf_offset++;
				//bytesremaining--;
				if (buf_offset == CHARBUFF_SIZE) buf_offset = 0; // will trigger a read of the next buffer full of character image data when buf_offset resets to 0
			}
		}
		if (char_width % 8 != 0) { // check if already incremented the buffer pointer last thing before we dropped out of the i loop - will have happened if
			buf_offset++;		   // width of char occurs on a byte boundary. If not, then increment the pointer to begin the next line of the character
			//bytesremaining--;
			if (buf_offset == CHARBUFF_SIZE) buf_offset = 0;
		}
	}
	//printf("char width=%d\n", char_width);
	unsigned int bidi_info = getBidiDirection((uint16_t)codepoint);

	advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth, codepoint, char_width); //fci.advanceWidth; bitmapwidth, advancewidth, isSpaceChar, bidi_info
	
	return char_width;
/*	
	switch(bidi_info) {
		case BIDI_AL:
			return char_width;
			break;
		
		case BIDI_NSM:	// non-spacing mark, composed character diacritics etc, don't move the cursor but overprint the character
			return 0;
			break;
			
		default:
			return char_width + 1;
	}
	
	return char_width + 1;
*/
}



/**
 *  @brief: this draws a character on the frame buffer but not refresh [uses the dot factory proportional font]
 */
int DiskFont::DrawCharAt(int x, int y, char ascii_char, double& advanceWidth, FONT_INFO* font, DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	return DrawCharAt(x, y, (uint32_t)codepointUtf8(String(ascii_char)), advanceWidth, font, fci, paint, colored);
}
 
int DiskFont::DrawCharAt(int x, int y, uint32_t codepoint, double& advanceWidth, FONT_INFO* font, DiskFont_FontCharInfo& fci, Paint* paint, int colored) {	
	uint16_t char_width = fci.widthbits;

	if (codepoint == 32) {
		//Serial.printf("DrawCharAt: space char width = %d\n", (int)fci.advanceWidth);
		advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth, codepoint, char_width); //fci.advanceWidth;
		return char_width; //fci.widthbits;
	}
	else {
		//Serial.printf("ch=%s, u+%x advanceWidth=%d\n", utf8fromCodepoint(codepoint).c_str(), codepoint, (int)fci.advanceWidth);
	}
	
	uint16_t char_height = (uint16_t)font->heightPages;
	//printf("char_height=%d\n", char_height);

	uint32_t char_offset = fci.bitmapfileoffset;
	//printf("char_offset=%d\n", char_offset);
	
    int i, j;
    //unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const uint8_t* ptr = &font->data[char_offset];
	
    for (j = 0; j < char_height; j++) {
		//printf("\n%d:\t|", j);
        for (i = 0; i < char_width; i++) {
            if (pgm_read_byte(ptr) & (0x80 >> (i % 8))) {
                paint->DrawPixel(x + i, y + j, colored);
				//Serial.printf("#");
            } else {
				//Serial.printf(" ");
			}
			
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (char_width % 8 != 0) {
            ptr++;
        }
    }
	
	//printf("char width=%d\n", char_width);
	
	unsigned int bidi_info = getBidiDirection((uint16_t)codepoint);

	advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth, codepoint, char_width); //fci.advanceWidth; bitmapwidth, advancewidth, isSpaceChar, bidi_info
	
	return char_width;

//	return char_width + 1;
}



/**
*  @brief: this displays a string on the frame buffer but not refresh [uses the dot factory proportional font]
*/
void DiskFont::DrawStringAt(int x, int y, String text, Paint* paint, int colored, bool right_to_left) {
	DrawStringAt(x, y, text, paint, colored, right_to_left, false);
}

void DiskFont::DrawStringAt(int x, int y, String text, Paint* paint, int colored, bool right_to_left, bool reverse_string) {
	Serial.printf("DrawStringAt: String is %s\n", text.c_str());

	//if (!available) {
	//	Serial.printf("Diskfont is not available.\n");
	//	return;
	//}
	
	int charIndex = 0;
	int len = text.length(); // Utf8CharCount(text); //text.length();
	
    int refcolumn = x;
	double drefcolumn = (double)x;
	double dcharwidth = 0.0;
	
	String ch = "";
	String ch_next = "";
	
	uint16_t blockToCheckFirst = 0;

	DiskFont_FontCharInfo fci;
	
	if (reverse_string) {
		text = Utf8ReverseString(text); // BUG: this is going to cause a bug when composing characters. The diacritic will appear left justified over the character to which it 
	}									// applies because it's code will be encountered first in the text string, before the width of the character to which it applies is known.
										// Will have to look ahead to the first non-NSM mark when a compositing diacritic is found, get the width, then use this for compositing.
	//printf("text=%s\n", text.c_str());
    
    /* Send the string character by character on EPD */	
	
	if (!right_to_left) {
		uint16_t char_bmwidth = 0;
		double charoffset = 0.0;
		double finalcharx = 0.0;

		while (charIndex != len) {
			//printf("refcolumn=%d\n", refcolumn);
			/* Display one character on EPD */
			ch = utf8CharAt(text, charIndex); //GetUtf8CharByIndex(text, charIndex); //utf8CharAt(text, charIndex);
			
			byte bidi_info = getBidiDirection(ch);
			
			if (bidi_info == BIDI_NSM) { // position at refcolumn + (charwidth / 2) to get the composited diacritic centred over the character to which it applies
				//int overlaycharwidth = GetTextWidth(ch);
				double doverlaycharwidth = GetCharWidth(ch, fci, blockToCheckFirst); //GetTextWidthA(ch);
				double dnsmcolumn = drefcolumn - dcharwidth; //skip back over last char printed (dcharwidth comes from the previous iteration of the while loop, from the else part of the if statement, that is, after a non-NSM character has been printed, if the NSM character is not the first character printed)
				//refcolumn -= charwidth; // if string has been reversed in memory (eg. for ltr substrings in rtl text and vv), the composing diacritics will be
				//int refcolumnoverlaychar = (int)(drefcolumn - dcharwidth);
				//Serial.printf("overlaych = [%s], refcolumn = %d, charwidth = %d, overlaycharwidth = %d, midpoint=%d, offsetoverlaychar=%d\n", ch.c_str(), refcolumn, charwidth, overlaycharwidth, refcolumn + (charwidth / 2), refcolumn + ((charwidth - overlaycharwidth) / 2));
				dnsmcolumn = dnsmcolumn + ((dcharwidth - doverlaycharwidth) / 2.0);

				DrawCharAt((int)dnsmcolumn, y, ch, fci, paint, colored); // first, so they will apply to the next non-nsm character to print, not the previous
				
				//drefcolumn += dcharwidth;
				//refcolumn += charwidth;									// one. In that case, do not need to subtract the last printed char width (ie do 
			}																					// backspace over it before overprinting the diacritic).
			else { 
				GetCharWidth(ch, char_bmwidth, dcharwidth, fci, blockToCheckFirst); //GetTextWidth(ch, char_bmwidth, dcharwidth);				
				charoffset = (dcharwidth - (double)char_bmwidth) / 2.0; //add, to center the character horizontally in its box
				finalcharx = drefcolumn + charoffset;

				DrawCharAt((int)finalcharx, y, ch, dcharwidth, fci, paint, colored);
				//refcolumn += charwidth;
				drefcolumn += dcharwidth;
				
				/*
				if (ch == " ") {
					Serial.printf("space char width is %d\n", (int)dcharwidth);
				}
				else {
					Serial.printf("ch=%s, dcharwidth=%d\n", ch.c_str(), (int)dcharwidth);
				}
				*/
			}
						
			//refcolumn += DrawCharAt(refcolumn, y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
			/* Point on the next character */
			 charIndex += ch.length(); //charIndex++; // += ch.length();
		}
	}
	else {
		uint16_t char_bmwidth = 0;
		double charoffset = 0.0;
		double finalcharx = 0.0;
		
		while (charIndex != len) {
			//printf("refcolumn=%d\n", refcolumn);
			/* Display one character on EPD */
			ch = utf8CharAt(text, charIndex); //GetUtf8CharByIndex(text, charIndex); //utf8CharAt(text, charIndex);
			
			byte bidi_info = getBidiDirection(ch);
			
			if (bidi_info == BIDI_NSM) {
				//int overlaycharwidth = GetTextWidth(ch);
				double doverlaycharwidth = GetCharWidth(ch, fci, blockToCheckFirst); //GetTextWidthA(ch);
				double dnsmcolumn = drefcolumn - dcharwidth; //+ charoffset; //skip back over last char printed
				//refcolumn -= charwidth; // position at refcolumn + (charwidth / 2) to get the diacritic centred over the character to which it applies
				//Serial.printf("overlaych = [%s], refcolumn = %d, charwidth = %d, overlaycharwidth = %d, midpoint=%d, offsetoverlaychar=%d\n", ch.c_str(), refcolumn, charwidth, overlaycharwidth, refcolumn + (charwidth / 2), refcolumn + ((charwidth - overlaycharwidth) / 2));
				//int refcolumnoverlaychar =(int)(float)(refcolumn + ((charwidth - overlaycharwidth) / 2));

				dnsmcolumn = (dcharwidth - doverlaycharwidth) / 2.0;
				DrawCharAt(PANEL_SIZE_X - (int)(finalcharx - dnsmcolumn), y, ch, fci, paint, colored);

//				dnsmcolumn = dnsmcolumn + (dcharwidth - doverlaycharwidth) / 2.0;
//				DrawCharAt(PANEL_SIZE_X - ((int)(dnsmcolumn + doverlaycharwidth)), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);


				
				//refcolumn += charwidth;
			}
			else {
				GetCharWidth(ch, char_bmwidth, dcharwidth, fci, blockToCheckFirst);//GetTextWidth(ch, char_bmwidth, dcharwidth); // populates fci structure with details of the character read from disk
				charoffset = (dcharwidth - (double)char_bmwidth) / 2.0; //add, to center the character horizontally in its box

				Serial.printf("ch=%s\tchar_bmwidth=%d\tdcharwidth=%s\tcharoffset=%s\n", ch.c_str(), char_bmwidth, String(dcharwidth).c_str(), String(charoffset,3).c_str());

				finalcharx = drefcolumn + (dcharwidth - charoffset);
				
				char_bmwidth = DrawCharAt(PANEL_SIZE_X - (int)finalcharx, y, ch, dcharwidth, fci, paint, colored);
				//refcolumn += charwidth;
				drefcolumn += dcharwidth;

				/*
				if (ch == " ") {
					Serial.printf("space char width is %d\n", (int)dcharwidth);
				}
				else {
					Serial.printf("ch=%s, dcharwidth=%d\n\n\n", ch.c_str(), (int)dcharwidth);
				}
				*/
			}
			
			//refcolumn += DrawCharAt(PANEL_SIZE_X - (refcolumn + GetTextWidth(ch)), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
			/* Point on the next character */
			charIndex += ch.length(); //charIndex++; //+= ch.length();
		}		
	}
	
	//Serial.printf("----\n\n");
}


double DiskFont::GetAdvanceWidth(uint16_t bitmapwidth, double advanceWidth, uint32_t codepoint, uint16_t& char_width) {
	//Serial.printf("ch = %s\t", utf8fromCodepoint(codepoint).c_str());
	
	// if using fixed (integer) pixel width space character, and character is a space
	//byte bidi_info = getBidiDirection(codepoint);
	//if (bidi_info == BIDI_NSM) {	// is non-spacing character, which is overprinted to create composed characters
	//	char_width = 0;
	//	return 0.0;
	//}

	if (codepoint == 32) {
		if (_font_use_fixed_spacecharwidth) {
			char_width = _font_fixed_spacecharwidth;
			return (double)_font_fixed_spacecharwidth;
		}
		else {
			char_width = (uint16_t)_FontHeader.spacecharwidth;
			return advanceWidth;			
		}
	}
	
	// if using fixed (integer) pixel width intercharacter spacing (may be useful for very small fonts)
	if (_font_use_fixed_spacing) {
		char_width = bitmapwidth + _font_fixed_spacing;
		return (double)char_width;
	}
	
	// otherwise, calculate the advance width according to the font and the tuning value
	
	//Font tuning value is obtained from the config.csv file
	double dbitmapwidth = (double)bitmapwidth;

	double scalefactor = (dbitmapwidth / advanceWidth) * 100.0;
	
	//Serial.printf("dbitmapwidth = %d, advanceWidth = %d, sf = %d\n", (int)dbitmapwidth, (int)advanceWidth, (int)scalefactor);

	char_width = bitmapwidth;
	
	if (scalefactor < _font_tuning_percent) {
		Serial.printf("dbitmapwidth < %spc of advanceWidth: returning advanceWidth\n\n", String(_font_tuning_percent).c_str());
		//char_width = (uint16_t)advanceWidth;
		return advanceWidth; // + 1.0; // if bitmap width is less than 80% of advanceWidth (which comes from the font metrics), then return advancewidth, since it is likely a small character with a large footprint
	}
	
	Serial.printf("returning dbitmapwidth\n\n");
	//char_width = (uint16_t)dbitmapwidth;
	return dbitmapwidth; // otherwise, if the values are close, prefer the bitmap width (helps with Arabic ligaturization etc.)
}


// Get char width and advance width (codepoint)
void DiskFont::GetCharWidth(uint32_t codepoint, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst) { 	//with blocktocheckfirst optimization
	if (getCharInfo(codepoint, &blockToCheckFirst, &fci)) {
		advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth, codepoint, width); 	// width is modified by GetAdvanceWidth()		
	}
}

void DiskFont::GetCharWidth(uint32_t codepoint, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci) { 								// without blocktocheckfirst optimization
	uint16_t blockToCheckFirst = 0;
	GetCharWidth(codepoint, width, advanceWidth, fci, blockToCheckFirst);
}


// Get char width and advance width (String)
void DiskFont::GetCharWidth(String ch, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst) { 			//with blocktocheckfirst optimization
	GetCharWidth(codepointUtf8(ch), width, advanceWidth, fci, blockToCheckFirst);
}

void DiskFont::GetCharWidth(String ch, uint16_t& width, double& advanceWidth, DiskFont_FontCharInfo& fci) {										 	// without blocktocheckfirst optimization
	uint16_t blockToCheckFirst = 0;
	GetCharWidth(codepointUtf8(ch), width, advanceWidth, fci, blockToCheckFirst);
}


// Get char advance width only (codepoint)
double DiskFont::GetCharWidth(uint32_t codepoint, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst) { 									//with blocktocheckfirst optimization
	double advanceWidth = 0.0;
	uint16_t width = 0;
	
	GetCharWidth(codepoint, width, advanceWidth, fci, blockToCheckFirst);
	
	return advanceWidth;
}

double DiskFont::GetCharWidth(uint32_t codepoint, DiskFont_FontCharInfo& fci) { 																// without blocktocheckfirst optimization
	uint16_t blockToCheckFirst = 0;
	return GetCharWidth(codepoint, fci, blockToCheckFirst);
}


// Get char advance width only (String)
double DiskFont::GetCharWidth(String ch, DiskFont_FontCharInfo& fci, uint16_t& blockToCheckFirst) { 											//with blocktocheckfirst optimization
	return GetCharWidth(codepointUtf8(ch), fci, blockToCheckFirst);
}

double DiskFont::GetCharWidth(String ch, DiskFont_FontCharInfo& fci) {																			// without blocktocheckfirst optimization
	uint16_t blockToCheckFirst = 0;
	return GetCharWidth(codepointUtf8(ch), fci, blockToCheckFirst);
}



void DiskFont::GetTextWidth(String text, int& width, double& advanceWidth) {
	//I2CSerial.printf("len:");

	int charIndex = 0;
	int i = 0;
	int len = text.length();
    int refcolumn = 0;
	String ch;

	double advwidth = 0.0;
	
	uint16_t blockToCheckFirst = 0;
	
	bool bresult = false;
	DiskFont_FontCharInfo fci;
	
	while (i < len) {
		ch = utf8CharAt(text, i);
		int codepoint = codepointUtf8(ch);
		bresult = getCharInfo(codepoint, &blockToCheckFirst, &fci);

		if (!bresult) Serial.printf("GetTextWidth: bresult is false, char=%x, ch=%s\n", codepointUtf8(ch), ch.c_str());
		
		if (bresult){
			uint16_t char_width = 0;
			advwidth += GetAdvanceWidth(fci.widthbits, fci.advanceWidth, codepoint, char_width); // char width is modified by GetAdvanceWidth()
			refcolumn += char_width;
		}
		else {
			//refcolumn += 0;
		}
		i += ch.length();
	}
	
	//I2CSerial.printf("refcolumn=%d\n", refcolumn);
	//return refcolumn;			
	width = refcolumn;
	advanceWidth = advwidth;
}


int DiskFont::GetTextWidth(const char* text) {
	return GetTextWidth(String(text));
}


double DiskFont::GetTextWidthA(const char* text) {
	return GetTextWidth(String(text));
}


double DiskFont::GetTextWidthA(String text) {
	int width = 0;
	double advwidth = 0.0;
	
	GetTextWidth(text, width, advwidth);
	return advwidth;	
}


int DiskFont::GetTextWidth(String text) {
	int width = 0;
	double advwidth = 0.0;
	
	GetTextWidth(text, width, advwidth);
	return width;
}


bool DiskFont::getCharInfo(String ch, DiskFont_FontCharInfo* fci) {
	uint16_t blockToCheckFirst = 0;
	
	return getCharInfo(codepointUtf8(ch), &blockToCheckFirst, fci);
}


bool DiskFont::getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* fci) {
	if (!available) {
		// will use rom font if diskfont is not available
		//Serial.printf("getCharInfo() diskfont _file is not available\n");
		const FONT_CHAR_INFO* f = getCharInfo(codepoint, blockToCheckFirst, romfont);

		if (f != NULL) {	
			fci->widthbits = (uint16_t)pgm_read_byte(&(f->widthBits));
			fci->heightbits = (uint16_t)pgm_read_byte(&(f->heightBits));
			fci->bitmapfileoffset = pgm_read_dword(&(f->offset));


			uint32_t* ptr = (uint32_t*)&(f->advanceWidth); // need to get a double out of flash, but there is no 'pgm_read_double'

			uint32_t double_h = pgm_read_dword(ptr);
			ptr+=1;
			uint32_t double_l = pgm_read_dword(ptr);
		  
			uint32_t p[2] = {double_h, double_l};
			  
			double d = *(double*)p;
	  
			fci->advanceWidth = d;

			fci->advanceHeight = 0; // not used in rom font

			//I2CSerial.printf("[%s] %s %d\n", utf8fromCodepoint(codepoint).c_str(), String(fci->advanceWidth, 3).c_str(), fci->widthbits);
			
			return true;
		}
		
		return false;
	}
		
	uint32_t foffset = 0;
	bool bresult = false;
	
	if (*blockToCheckFirst < _FontHeader.numlookupblocks) {
		if ((_Font_BlocktablePtr[*blockToCheckFirst].startchar <= codepoint) && (_Font_BlocktablePtr[*blockToCheckFirst].endchar >= codepoint)) {
			//I2CSerial.printf("default BlockIndex = %d\n", *blockToCheckFirst);
			//I2CSerial.printf("sc=%x, ec=%x, cp=%x\n", _Font_BlocktablePtr[*blockToCheckFirst].startchar, _Font_BlocktablePtr[*blockToCheckFirst].endchar, codepoint);
			foffset = _Font_BlocktablePtr[*blockToCheckFirst].fileoffset_startchar + ((codepoint - _Font_BlocktablePtr[*blockToCheckFirst].startchar) * DiskFont_FontCharInfo_RecSize); //sizeof(DiskFont_FontCharInfo));
			Seek(foffset);			
						
			//I2CSerial.printf("# foffset=%x\n", foffset);			
	
			bresult = readFontCharInfoEntry(fci);
			//I2CSerial.printf("getCharInfo() bresult=%s\n", bresult?"true":"false");
			//I2CSerial.printf("[%s] %s %d\n", utf8fromCodepoint(codepoint).c_str(), String(fci->advanceWidth, 3).c_str(), fci->widthbits);
			return bresult;
		}
		else {
			uint16_t blockIndex = 0;
			bool bFound = false;
			
			while (!bFound && blockIndex < _FontHeader.numlookupblocks) {
				if ((_Font_BlocktablePtr[blockIndex].startchar <= codepoint) && (_Font_BlocktablePtr[blockIndex].endchar >= codepoint)) {
					bFound = true;
				} else {
					blockIndex++;
				}
			}
			
			if (!bFound) {
				//I2CSerial.printf("block not found, _FontHeader.numlookupblocks = %d, codepoint=%x\n", _FontHeader.numlookupblocks, codepoint);
				return false;
			}
			
			*blockToCheckFirst = blockIndex;
			
			//I2CSerial.printf("BlockIndex = %d\n", blockIndex);
			
			foffset = _Font_BlocktablePtr[blockIndex].fileoffset_startchar + ((codepoint - _Font_BlocktablePtr[*blockToCheckFirst].startchar) * DiskFont_FontCharInfo_RecSize); //sizeof(DiskFont_FontCharInfo));
			Seek(foffset);
			
			//I2CSerial.printf("@ foffset=%x\n", foffset);			

			bresult = readFontCharInfoEntry(fci);
			//I2CSerial.printf("[%s] %s %d\n", utf8fromCodepoint(codepoint).c_str(), String(fci->advanceWidth, 3).c_str(), fci->widthbits);
			//I2CSerial.printf("getCharInfo() bresult=%s\n", bresult?"true":"false");
			return bresult;
		}
	}
		
	return false;
}

bool DiskFont::readFontCharInfoEntry(DiskFont_FontCharInfo* fci) {
	if (!available) return false;
	
	bool bresult = false;

	//I2CSerial.printf("pos:%x", _file.position());
	
	bresult = /*ReadUInt8(&(fci->rtlflag)) && */
			   Read(&(fci->widthbits))
			&& Read(&(fci->heightbits))
			&& Read(&(fci->bitmapfileoffset))
			&& Read(&(fci->advanceWidth))
			&& Read(&(fci->advanceHeight));

	//I2CSerial.printf(" fci: rtl:%x w:%x h:%x offset_char:%x bresult=%s\n", fci->rtlflag, fci->widthbits, fci->heightbits, fci->bitmapfileoffset, bresult?"true":"false");
	//I2CSerial.printf(" fci: w:%x h:%x offset_char:%x bresult=%s\n", fci->widthbits, fci->heightbits, fci->bitmapfileoffset, bresult?"true":"false");
	
	return bresult;
}

const FONT_CHAR_INFO* DiskFont::getCharInfo(int codepoint, uint16_t* blockToCheckFirst, FONT_INFO* font) {
	if (font->charInfo != NULL) {
		return &font->charInfo[codepoint - font->startChar];
	} 
	else {
		if (font->blockCount > 0 && font->fontcharinfoBlockLookup != NULL) {
			if (*blockToCheckFirst < font->blockCount) {
				if ((font->fontcharinfoBlockLookup[*blockToCheckFirst].startChar <= codepoint) && (font->fontcharinfoBlockLookup[*blockToCheckFirst].endChar >= codepoint)) {
					//Serial.printf("default BlockIndex = %d\n", *blockToCheckFirst);
					return &font->fontcharinfoBlockLookup[*blockToCheckFirst].fontcharinfoBlock[codepoint - font->fontcharinfoBlockLookup[*blockToCheckFirst].startChar];
				}
				else {
					uint16_t blockIndex = 0;
					bool bFound = false;
					
					while (!bFound && blockIndex < font->blockCount) {
						if ((font->fontcharinfoBlockLookup[blockIndex].startChar <= codepoint) && (font->fontcharinfoBlockLookup[blockIndex].endChar >= codepoint)) {
							bFound = true;
						} else {
							blockIndex++;
						}
					}
					
					if (!bFound) return NULL;
					
					*blockToCheckFirst = blockIndex;
					
					//Serial.printf("BlockIndex = %d\n", blockIndex);
					
					return &font->fontcharinfoBlockLookup[blockIndex].fontcharinfoBlock[codepoint - font->fontcharinfoBlockLookup[blockIndex].startChar];
				}
			}
		}
	}
	
	return NULL;
}