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
		font_fixed_spacechar_width = 2;		
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
	if (fontfilename == "builtin") return false;
	
	_fontfilename = fontfilename;
	
/*
	_file = SD.open(_fontfilename.c_str(), FILE_READ);

	if (!_file.available()) {
		I2CSerial.printf("unable to open diskfont %s\n", _fontfilename.c_str());
		return false;
	}
*/
	if (!OpenFontFile()) return false;

	/*
            const unsigned long FONT_HEADER_OFFSET_CHARHEIGHT = 8; // word
            const unsigned long FONT_HEADER_OFFSET_STARTCHAR = 10; // dword
            const unsigned long FONT_HEADER_OFFSET_ENDCHAR = 14; // dword
            const unsigned long FONT_HEADER_OFFSET_NUMLOOKUPENTRIES = 18; // word
            const unsigned long FONT_HEADER_OFFSET_SPACECHARWIDTH = 20; // byte
            const unsigned long FONT_HEADER_OFFSET_END = 21;
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
		&& Read(&_FontHeader.spacecharwidth))) 
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
int DiskFont::DrawCharAt(int x, int y, char ascii_char, Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), paint, colored, blockToCheckFirst);
}

int DiskFont::DrawCharAt(int x, int y, uint32_t codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst) {
	double advwidth = 0.0;
	return DrawCharAt(x, y, advwidth, codepoint, paint, colored, blockToCheckFirst);
}

 
int DiskFont::DrawCharAt(int x, int y, double& advanceWidth, uint32_t codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	//if (codepoint == 32) return _FontHeader.spacecharwidth; // space character

	if(!available) {
		Serial.printf("DrawCharAt() Diskfont is not available.\n");
		return 0;
	}
	
	DiskFont_FontCharInfo fci;
	if (!getCharInfo(codepoint, blockToCheckFirst, &fci)) {
		I2CSerial.printf("DrawCharAt() getCharInfo returned false\n");
		return 0;
	}
	//else {
		//refcolumn += (int)(pgm_read_byte(&(fci->widthBits))) + 1;
	//}

	if (codepoint == 32) {
		//Serial.printf("DrawCharAt: space char width = %d\n", (int)fci.advanceWidth);
		advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth); //fci.advanceWidth;
		return (int)advanceWidth; //fci.widthbits;
	}
	else {
		//Serial.printf("ch=%s, u+%x advanceWidth=%d\n", utf8fromCodepoint(codepoint).c_str(), codepoint, (int)fci.advanceWidth);
	}
	
	//int charIndex = codepoint - font->startChar;
	//I2CSerial.printf("char codepoint=%s, font->startChar = %d, char_index=%d\n", utf8fromCodepoint(codepoint).c_str(), font->startChar, charIndex);

	uint16_t char_height = _FontHeader.charheight;
	//I2CSerial.printf("char_height=%d\n", char_height);

	////uint8_t char_width = pgm_read_byte(&(font->charInfo[charIndex].widthBits));
	uint16_t char_width = fci.widthbits;
	//I2CSerial.printf("char_width=%d\n", char_width);

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

	advanceWidth = GetAdvanceWidth(fci.widthbits, fci.advanceWidth); //fci.advanceWidth;
	
	unsigned int bidi_info = getBidiDirection((uint16_t)codepoint);
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
}

/**
*  @brief: this displays a string on the frame buffer but not refresh [uses the dot factory proportional font]
*/
void DiskFont::DrawStringAt(int x, int y, const char* text, Paint* paint, int colored, bool right_to_left, bool reverse_string) {
	DrawStringAt(x, y, String(text), paint, colored, right_to_left, reverse_string);
}

void DiskFont::DrawStringAt(int x, int y, String text, Paint* paint, int colored, bool right_to_left, bool reverse_string) {
	Serial.printf("DrawStringAt: String is %s\n", text.c_str());

	if (!available) {
		Serial.printf("Diskfont is not available.\n");
		return;
	}
		
	int charIndex = 0;
	int len = Utf8CharCount(text); //text.length();
	
    int refcolumn = x;
	String ch = "";
	String ch_next = "";
	
	uint16_t blockToCheckFirst = 0;
	
	if (reverse_string) {
		text = Utf8ReverseString(text); // BUG: this is going to cause a bug when composing characters. The diacritic will appear left justified over the character to which it 
	}									// applies because it's code will be encountered first in the text string, before the width of the character to which it applies is known.
										// Will have to look ahead to the first non-NSM mark when a compositing diacritic is found, get the width, then use this for compositing.
	//printf("text=%s\n", text.c_str());
    
    /* Send the string character by character on EPD */	
	
	if (!right_to_left) {
		int charwidth = 0;
		while (charIndex != len) {
			//printf("refcolumn=%d\n", refcolumn);
			/* Display one character on EPD */
			ch = GetUtf8CharByIndex(text, charIndex); //utf8CharAt(text, charIndex);
			
			byte bidi_info = getBidiDirection(ch);
			
			if (bidi_info == BIDI_NSM) { // position at refcolumn + (charwidth / 2) to get the composited diacritic centred over the character to which it applies
				int overlaycharwidth = GetTextWidth(ch);
				
				refcolumn -= charwidth; // if string has been reversed in memory (eg. for ltr substrings in rtl text and vv), the composing diacritics will be
				//Serial.printf("overlaych = [%s], refcolumn = %d, charwidth = %d, overlaycharwidth = %d, midpoint=%d, offsetoverlaychar=%d\n", ch.c_str(), refcolumn, charwidth, overlaycharwidth, refcolumn + (charwidth / 2), refcolumn + ((charwidth - overlaycharwidth) / 2));
				int refcolumnoverlaychar =(int)(float)(refcolumn + ((charwidth - overlaycharwidth) / 2));

				DrawCharAt(refcolumnoverlaychar /*+ overlaycharwidth*//*refcolumn + ((charwidth-overlaycharwidth) / 2)*/, y, codepointUtf8(ch), paint, colored, &blockToCheckFirst); // first, so they will apply to the next non-nsm character to print, not the previous
				
				refcolumn += charwidth;									// one. In that case, do not need to subtract the last printed char width (ie do 
			}																					// backspace over it before overprinting the diacritic).
			else { 
				charwidth = DrawCharAt(refcolumn, y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
				refcolumn += charwidth;
			}
						
			//refcolumn += DrawCharAt(refcolumn, y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
			/* Point on the next character */
			charIndex++; // += ch.length();
		}
	}
	else {
		int charwidth = 0;
		while (charIndex != len) {
			//printf("refcolumn=%d\n", refcolumn);
			/* Display one character on EPD */
			ch = GetUtf8CharByIndex(text, charIndex); //utf8CharAt(text, charIndex);
			
			byte bidi_info = getBidiDirection(ch);
			
			if (bidi_info == BIDI_NSM) {
				int overlaycharwidth = GetTextWidth(ch);
				refcolumn -= charwidth; // position at refcolumn + (charwidth / 2) to get the diacritic centred over the character to which it applies
				//Serial.printf("overlaych = [%s], refcolumn = %d, charwidth = %d, overlaycharwidth = %d, midpoint=%d, offsetoverlaychar=%d\n", ch.c_str(), refcolumn, charwidth, overlaycharwidth, refcolumn + (charwidth / 2), refcolumn + ((charwidth - overlaycharwidth) / 2));
				int refcolumnoverlaychar =(int)(float)(refcolumn + ((charwidth - overlaycharwidth) / 2));

				DrawCharAt(PANEL_SIZE_X - (refcolumnoverlaychar + overlaycharwidth), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
				
				refcolumn += charwidth;
			}
			else {
				charwidth = DrawCharAt(PANEL_SIZE_X - (refcolumn + GetTextWidth(ch)), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
				refcolumn += charwidth;
			}
			
			//refcolumn += DrawCharAt(PANEL_SIZE_X - (refcolumn + GetTextWidth(ch)), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
			/* Point on the next character */
			charIndex++; //+= ch.length();
			wdt_reset();
		}		
	}
	
	//Serial.printf("----\n\n");
}

void DiskFont::DrawStringAt(int x, int y, String text, Paint* paint, int colored, bool right_to_left) {
	DrawStringAt(x, y, text, paint, colored, right_to_left, false);
}

void DiskFont::DrawStringAtA(int x, int y, String text, Paint* paint, int colored, bool right_to_left) {
	DrawStringAtA(x, y, text, paint, colored, right_to_left, false);
}

void DiskFont::DrawStringAtA(int x, int y, String text, Paint* paint, int colored, bool right_to_left, bool reverse_string) {
	Serial.printf("DrawStringAt: String is %s\n", text.c_str());

	if (!available) {
		Serial.printf("Diskfont is not available.\n");
		return;
	}
	
	int charIndex = 0;
	int len = Utf8CharCount(text); //text.length();
	
    int refcolumn = x;
	double drefcolumn = (double)x;
	double dcharwidth = 0.0;
	
	String ch = "";
	String ch_next = "";
	
	uint16_t blockToCheckFirst = 0;
	
	if (reverse_string) {
		text = Utf8ReverseString(text); // BUG: this is going to cause a bug when composing characters. The diacritic will appear left justified over the character to which it 
	}									// applies because it's code will be encountered first in the text string, before the width of the character to which it applies is known.
										// Will have to look ahead to the first non-NSM mark when a compositing diacritic is found, get the width, then use this for compositing.
	//printf("text=%s\n", text.c_str());
    
    /* Send the string character by character on EPD */	
	
	if (!right_to_left) {
		int charwidth = 0;
		while (charIndex != len) {
			//printf("refcolumn=%d\n", refcolumn);
			/* Display one character on EPD */
			ch = GetUtf8CharByIndex(text, charIndex); //utf8CharAt(text, charIndex);
			
			byte bidi_info = getBidiDirection(ch);
			
			if (bidi_info == BIDI_NSM) { // position at refcolumn + (charwidth / 2) to get the composited diacritic centred over the character to which it applies
				//int overlaycharwidth = GetTextWidth(ch);
				double doverlaycharwidth = GetTextWidthA(ch);
				double dnsmcolumn = drefcolumn - dcharwidth; //skip back over last char printed
				//refcolumn -= charwidth; // if string has been reversed in memory (eg. for ltr substrings in rtl text and vv), the composing diacritics will be
				//int refcolumnoverlaychar = (int)(drefcolumn - dcharwidth);
				//Serial.printf("overlaych = [%s], refcolumn = %d, charwidth = %d, overlaycharwidth = %d, midpoint=%d, offsetoverlaychar=%d\n", ch.c_str(), refcolumn, charwidth, overlaycharwidth, refcolumn + (charwidth / 2), refcolumn + ((charwidth - overlaycharwidth) / 2));
				dnsmcolumn = dnsmcolumn + ((dcharwidth - doverlaycharwidth) / 2.0);

				DrawCharAt((int)dnsmcolumn, y, codepointUtf8(ch), paint, colored, &blockToCheckFirst); // first, so they will apply to the next non-nsm character to print, not the previous
				
				//drefcolumn += dcharwidth;
				//refcolumn += charwidth;									// one. In that case, do not need to subtract the last printed char width (ie do 
			}																					// backspace over it before overprinting the diacritic).
			else { 
				charwidth = DrawCharAt((int)drefcolumn, y, dcharwidth, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
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
			charIndex++; // += ch.length();
		}
	}
	else {
		int char_bmwidth = 0;
		double charoffset = 0.0;
		double finalcharx = 0.0;
		
		while (charIndex != len) {
			//printf("refcolumn=%d\n", refcolumn);
			/* Display one character on EPD */
			ch = GetUtf8CharByIndex(text, charIndex); //utf8CharAt(text, charIndex);
			
			byte bidi_info = getBidiDirection(ch);
			
			if (bidi_info == BIDI_NSM) {
				//int overlaycharwidth = GetTextWidth(ch);
				double doverlaycharwidth = GetTextWidthA(ch);
				double dnsmcolumn = drefcolumn - dcharwidth; //+ charoffset; //skip back over last char printed
				//refcolumn -= charwidth; // position at refcolumn + (charwidth / 2) to get the diacritic centred over the character to which it applies
				//Serial.printf("overlaych = [%s], refcolumn = %d, charwidth = %d, overlaycharwidth = %d, midpoint=%d, offsetoverlaychar=%d\n", ch.c_str(), refcolumn, charwidth, overlaycharwidth, refcolumn + (charwidth / 2), refcolumn + ((charwidth - overlaycharwidth) / 2));
				//int refcolumnoverlaychar =(int)(float)(refcolumn + ((charwidth - overlaycharwidth) / 2));

				dnsmcolumn = /*dnsmcolumn + */(dcharwidth - doverlaycharwidth) / 2.0;
				DrawCharAt(PANEL_SIZE_X - ((int)finalcharx - dnsmcolumn), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);

//				dnsmcolumn = dnsmcolumn + (dcharwidth - doverlaycharwidth) / 2.0;
//				DrawCharAt(PANEL_SIZE_X - ((int)(dnsmcolumn + doverlaycharwidth)), y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);


				
				//refcolumn += charwidth;
			}
			else {
				GetTextWidth(ch, char_bmwidth, dcharwidth);
				charoffset = (dcharwidth - (double)char_bmwidth) / 2.0; //add, to center the character horizontally in its box
				
				finalcharx = drefcolumn + (dcharwidth - charoffset);
				
				char_bmwidth = DrawCharAt(PANEL_SIZE_X - ((int)finalcharx), y, dcharwidth, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
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
			charIndex++; //+= ch.length();
		}		
	}
	
	//Serial.printf("----\n\n");
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
		bresult = getCharInfo(codepointUtf8(ch), &blockToCheckFirst, &fci);

		//if (!bresult) Serial.printf("GetTextWidth: bresult is false, char=%x, ch=%s\n", codepointUtf8(ch), ch.c_str());
		
		if (bresult){
			if (ch == " ") {
				//refcolumn += _FontHeader.spacecharwidth;
				refcolumn += (int)fci.advanceWidth;
				advwidth += GetAdvanceWidth(_FontHeader.spacecharwidth, fci.advanceWidth);
			} 
			else {
				//charIndex = (int)(codepointUtf8(ch) - font->startChar);
				//refcolumn += (int)(pgm_read_byte(&(font->charInfo[charIndex].widthBits))) + 1;
				
				//I2CSerial.printf(">cp=%x, len(ch)=%d\n", codepointUtf8(ch), ch.length());
				bresult = getCharInfo(codepointUtf8(ch), &blockToCheckFirst, &fci);
				
				//I2CSerial.printf(bresult ? "getCharInfo returned true\n" : "getCharInfo returned false\n");
				
				//if (bresult) {
					refcolumn += fci.widthbits + 1;
					advwidth += GetAdvanceWidth(fci.widthbits, fci.advanceWidth); //fci.advanceWidth;
				//}
				//else {
				//	refcolumn += 0;
				//}
			}
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

double DiskFont::GetAdvanceWidth(int bitmapwidth, double advanceWidth) {
	//Font tuning value will be obtained from the config.csv file
	double dbitmapwidth = (double) bitmapwidth;

	double scalefactor = (dbitmapwidth / advanceWidth) * 100.0;
	
	//Serial.printf("dbitmapwidth = %d, advanceWidth = %d, sf = %d\n", (int)dbitmapwidth, (int)advanceWidth, (int)scalefactor);
	
	if (scalefactor < _font_tuning_percent) {
		//Serial.printf("dbitmapwidth < 50pc of advanceWidth: returning advanceWidth\n\n");
		return advanceWidth; // if bitmap width is less than 80% of advanceWidth (which comes from the font metrics), then return advancewidth, since it is likely a small character with a large footprint
	}
	
	//Serial.printf("returning dbitmapwidth\n\n");
	return dbitmapwidth; // otherwise, if the values are close, prefer the bitmap width (helps with Arabic ligaturization etc.)
}


int DiskFont::GetTextWidth(const char* text) {
	return GetTextWidth(String(text));
/*    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = 0;
	int charIndex;

	uint16_t blockToCheckFirst = 0;
	
    while (*p_text != 0) {
		if (*p_text == 32) {
			refcolumn += _FontHeader.spacecharwidth;
		} else {
			DiskFont_FontCharInfo fci;			
			if (getCharInfo((int)*p_text, &blockToCheckFirst, &fci)) {
				refcolumn += fci.widthbits + 1;
			}
			else {
				refcolumn += 0;
			}
		}

        p_text++;
        counter++;
    }
	return refcolumn;
*/
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
/*	
	
	//I2CSerial.printf("len:");

	int charIndex = 0;
	int i = 0;
	int len = text.length();
    int refcolumn = 0;
	String ch;

	uint16_t blockToCheckFirst = 0;

	bool bresult = false;
	DiskFont_FontCharInfo fci;
	
	while (i < len) {
		ch = utf8CharAt(text, i);
		if (ch == " ") {
			refcolumn += _FontHeader.spacecharwidth;
		} 
		else {
			//charIndex = (int)(codepointUtf8(ch) - font->startChar);
			//refcolumn += (int)(pgm_read_byte(&(font->charInfo[charIndex].widthBits))) + 1;
			
			//I2CSerial.printf(">cp=%x, len(ch)=%d\n", codepointUtf8(ch), ch.length());
			bresult = getCharInfo(codepointUtf8(ch), &blockToCheckFirst, &fci);
			
			//I2CSerial.printf(bresult ? "getCharInfo returned true\n" : "getCharInfo returned false\n");
			
			if (bresult) {
				refcolumn += fci.widthbits + 1;
			}
			else {
				refcolumn += 0;
			}
		}

		i += ch.length();
	}
	
	//I2CSerial.printf("refcolumn=%d\n", refcolumn);
	return refcolumn;		
*/
}

bool DiskFont::getCharInfo(String ch, DiskFont_FontCharInfo* fci) {
	Serial.printf("@");
	
	uint16_t blockToCheckFirst = 0;
	
	return getCharInfo(codepointUtf8(ch), &blockToCheckFirst, fci);
}

bool DiskFont::getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* fci) {
	if (!available) {
		Serial.printf("getCharInfo() diskfont _file is not available\n");
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
