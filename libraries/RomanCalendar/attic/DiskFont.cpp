#include <DiskFont.h>

DiskFont::DiskFont() {
}

DiskFont::~DiskFont() {
	//_I18n.closeFile(_file);
	delete _Font_BlocktablePtr;
	_file.close();
}

bool DiskFont::begin(/*I18n* i18n,*/ String fontfilename) {
	//if (i18n == NULL) return false;
	
	//_I18n = i18n;
	_fontfilename = fontfilename;
	
	//_file = _I18n->openFile(_fontfilename, "r");
	_file = SD.open(_fontfilename.c_str(), "r");

	if (!_file.available()) return false;
	/*
            const unsigned long FONT_HEADER_OFFSET_CHARHEIGHT = 8; // word
            const unsigned long FONT_HEADER_OFFSET_STARTCHAR = 10; // dword
            const unsigned long FONT_HEADER_OFFSET_ENDCHAR = 14; // dword
            const unsigned long FONT_HEADER_OFFSET_NUMLOOKUPENTRIES = 18; // word
            const unsigned long FONT_HEADER_OFFSET_SPACECHARWIDTH = 20; // byte
            const unsigned long FONT_HEADER_OFFSET_END = 21;
            const unsigned long BLOCKTABLE_OFFSET_BEGIN = FONT_HEADER_OFFSET_END;
	*/
	
	bool bResult = false;
	_FontHeader = {0};
	
	_file.seek(FONT_HEADER_OFFSET_START);
	bResult = ReadUInt16BigEndian(&_FontHeader.charheight);
	bResult = ReadUInt32BigEndian(&_FontHeader.startchar);
	bResult = ReadUInt32BigEndian(&_FontHeader.endchar);
	bResult = ReadUInt16BigEndian(&_FontHeader.numlookupblocks);
	bResult = ReadUInt8(&_FontHeader.spacecharwidth);

	// need to read blocktable (1024/3*4 = 85 blocktable entries/kB)
	uint32_t font_blocktablesize = _FontHeader.numlookupblocks * sizeof(DiskFont_BlocktableEntry);
	
	if (font_blocktablesize > (system_get_free_heap_size() - 10240)) {
		//_I18n.closeFile(_file); // not enough memory for blocktable (leaves a minimum of 10k free)
		_file.close();
		return false;
	}
	
	_Font_BlocktablePtr = new DiskFont_BlocktableEntry[_FontHeader.numlookupblocks];

	if (_Font_BlocktablePtr == NULL) {
		//_I18n.closeFile(_file);
		_file.close();	// 'new' failed (shouldn't happen, since we calculated the available memory beforehand, but still...)
		return false;
	}
	
	_file.read(_Font_BlocktablePtr, font_blocktablesize);
	
	return true; // ready to access font
}

DiskFont::end() {
	//_I18n.closeFile(_file);
	delete _Font_BlocktablePtr;	
	_file.close();
}

DiskFont::getCharWidthAndOffset(*char_width, *char_offset) {
	
}


/**
 *  @brief: this draws a character on the frame buffer but not refresh [uses the dot factory proportional font]
 */
int DiskFont::DrawCharAt(int x, int y, char ascii_char, Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	return DrawCharAt(x, y, codepointUtf8(String(ascii_char)), paint, colored, blockToCheckFirst);
}
 
int DiskFont::DrawCharAt(int x, int y, int codepoint, Paint* paint, int colored, uint16_t* blockToCheckFirst) {	
	if (codepoint == 32) return _FontHeader.spacecharwidth; // space character

	DiskFont_FontCharInfo fci;
	if (getCharInfo(codepoint, &blockToCheckFirst, &fci) {
		//Serial.printf("DrawCharAt fci is NULL\n");
		return 0;
	}
	//else {
		//refcolumn += (int)(pgm_read_byte(&(fci->widthBits))) + 1;
	//}



	////int charIndex = codepoint - font->startChar;
	//printf("char codepoint=%s, font->startChar = %d, char_index=%d\n", utf8fromCodepoint(codepoint).c_str(), font->startChar, charIndex);

	uint16_t char_height = _FontHeader.charheight;
	//printf("char_height=%d\n", char_height);

	////uint8_t char_width = pgm_read_byte(&(font->charInfo[charIndex].widthBits));
	uint16_t char_width = fci.widthbits;
	//printf("char_width=%d\n", char_width);

	////uint32_t char_offset = pgm_read_dword(&(font->charInfo[charIndex].offset));
	uint32_t char_offset = fci.bitmapfileoffset;
	//printf("char_offset=%d\n", char_offset);
	
    int i, j;
    //unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    //const uint8_t* ptr = &font->data[char_offset];
	
	//int charbuffsize = 32;
	int buf_offset = 0;	
	int bytesremaining = 0;
	
	if (fci.widthbits > 0) {
		bytesremaining = ((fci.widthbits / 8) + 1) * fci.heightbits;
	else {
		return 0;
	}
		
	for (j = 0; j < char_height; j++) {
		//printf("\n%d:\t|", j);
		for (i = 0; i < char_width; i++) {

			if (buf_offset == 0) {
				if (_file.size() - _file.position()) > bytesremaining) {
					_file.read(_char_buffer[0], bytesremaining < CHARBUFF_SIZE ? bytesremaining : CHARBUFF_SIZE);
				}
				else {
					// error - found unexpected end of file
					return char_width + 1;
				}
			}

			if (_char_buffer[buf_offset] & (0x80 >> (i % 8))) {
				paint->DrawPixel(x + i, y + j, colored);
				//Serial.printf("#");
			} else {
				//Serial.printf(" ");
			}
			
			bytesremaining--;
							
			if (i % 8 == 7) {
				buf_offset++;
				if (buf_offset == CHARBUFF_SIZE) buf_offset = 0; // will trigger a read of the next buffer full of character image data when buf_offset resets to 0
			}
		}
		if (char_width % 8 != 0) { // check if already incremented the buffer pointer last thing before we dropped out of the i loop - will have happened if
			buf_offset++;		   // width of char occurs on a byte boundary. If not, then increment the pointer to begin the next line of the character
			if (buf_offset == CHARBUFF_SIZE) buf_offset = 0;
		}
	}
	//printf("char width=%d\n", char_width);
	
	return char_width + 1;
}

/**
*  @brief: this displays a string on the frame buffer but not refresh [uses the dot factory proportional font]
*/
void DiskFont::DrawStringAt(int x, int y, const char* text, Paint* paint, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;

	uint16_t blockToCheckFirst = 0;

	//printf("text=%s\n", text);
    
    /* Send the string character by character on EPD */
    while (*p_text != 0) {
		//printf("counter=%d, refcolumn=%d\n", counter, refcolumn);
        /* Display one character on EPD */
        refcolumn += DrawCharAt(refcolumn, y, *p_text, paint, colored, &blockToCheckFirst);
        /* Decrement the column position by 16 */
        //refcolumn += font->Width;
        /* Point on the next character */
        p_text++;
        counter++;
    }
}

void DiskFont::DrawStringAt(int x, int y, String text, Paint* paint, int colored) {
	int charIndex = 0;
	int len = text.length();
	
    int refcolumn = x;
	String ch;
	
	uint16_t blockToCheckFirst = 0;
	
	//printf("text=%s\n", text.c_str());
    
    /* Send the string character by character on EPD */
    while (charIndex != len) {
		//printf("refcolumn=%d\n", refcolumn);
        /* Display one character on EPD */
		ch = utf8CharAt(text, charIndex);
        refcolumn += DrawCharAt(refcolumn, y, codepointUtf8(ch), paint, colored, &blockToCheckFirst);
        /* Decrement the column position by 16 */
        //refcolumn += font->Width;
        /* Point on the next character */
        charIndex += ch.length();
    }
}

int DiskFont::GetTextWidth(const char* text) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = 0;
	int charIndex;

	uint16_t blockToCheckFirst = 0;
	
    while (*p_text != 0) {
		if (*p_text == 32) {
			refcolumn += _FontHeader.spacecharwidth;
		} else {
			DiskFont_FontCharInfo fci;			
			if (getCharInfo((int)*p_text, &blockToCheckFirst, &fci) {
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
}

int DiskFont::GetTextWidth(String text) {

	int charIndex = 0;
	int i = 0;
	int len = text.length();
    int refcolumn = 0;
	String ch;

	uint16_t blockToCheckFirst = 0;
	
	while (i < len) {
		ch = utf8CharAt(text, i);
		if (ch == " ") {
			refcolumn += _FontHeader.spacecharwidth;
		} else {
			//charIndex = (int)(codepointUtf8(ch) - font->startChar);
			//refcolumn += (int)(pgm_read_byte(&(font->charInfo[charIndex].widthBits))) + 1;
			DiskFont_FontCharInfo fci;
			if (getCharInfo(codepointUtf8(ch), &blockToCheckFirst, &fci) {
				refcolumn += fci.widthbits + 1;
			}
			else {
				refcolumn += 0;
			}
		}

		i += ch.length();
	}
	return refcolumn;		
}

bool DiskFont::getCharInfo(int codepoint, uint16_t* blockToCheckFirst, DiskFont_FontCharInfo* fci) {
	if (!_file) return false;
	
	if (*blockToCheckFirst < _FontHeader.numlookupblocks) {
		if ((_Font_BlocktablePtr[*blockToCheckFirst].startchar <= codepoint) && (_Font_BlocktablePtr[*blockToCheckFirst].endchar >= codepoint)) {
			//Serial.printf("default BlockIndex = %d\n", *blockToCheckFirst);
			_file.seek(_Font_BlocktablePtr[*blockToCheckFirst].fileoffset_startchar + ((codepoint - _Font_BlocktablePtr[*blockToCheckFirst].startchar) * sizeof(DiskFont_FontCharInfo));			
			
			return readFontCharInfoEntry(fci);
		}
		else {
			uint16_t blockIndex = 0;
			bool bFound = false;
			
			while (!bFound && blockIndex < _FontHeader.numlookupblocks) {
				if ((_Font_BlocktablePtr[*blockToCheckFirst].startchar <= codepoint) && (_Font_BlocktablePtr[*blockToCheckFirst].endchar >= codepoint)) {
					bFound = true;
				} else {
					blockIndex++;
				}
			}
			
			if (!bFound) return false;
			
			*blockToCheckFirst = blockIndex;
			
			//Serial.printf("BlockIndex = %d\n", blockIndex);
			
			_file.seek(_Font_BlocktablePtr[blockIndex].fileoffset_startchar + ((codepoint - _Font_BlocktablePtr[*blockToCheckFirst].startchar) * sizeof(DiskFont_FontCharInfo)));
			
			return readFontCharInfo(fci);
		}
	}
		
	return false;
}

bool DiskFont::readFontCharInfoEntry(DiskFont_FontCharInfo* fci) {
	if (!_file) return false;
	
	return ReadUInt16BigEndian(&(fci->widthbits)) && ReadUInt16BigEndian(&(fci->heightbits)) && ReadUInt32BigEndian(&(fci->bitmapfileoffset));
}

int DiskFont::codepointUtf8(String c) {
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

String DiskFont::utf8fromCodepoint(int c) {
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

String DiskFont::utf8CharAt(String s, int pos) { 
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

int DiskFont::charLenBytesUTF8(char s) {
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

bool DiskFont::ReadUInt32BigEndian(uint32_t* dword){
	if (!_file.available()) return false;
	
	_file.read(dword, 4);	
	return true;
}

bool DiskFont::ReadUInt16BigEndian(uint16_t* word){
	if (!_file.available()) return false;
	
	_file.read(word, 2);	
	return true;
}

bool DiskFont::ReadUInt8(uint8_t* byte) {
	if (!_file.available()) return false;

	_file.read(byte, 1);
	return true;
}