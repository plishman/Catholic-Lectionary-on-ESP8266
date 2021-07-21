#include "FrameBuffer.h"
#include "DebugPort.h"

#define SPIRAM_READ		0x03	// Read data from memory array beginning at selected address 
#define SPIRAM_WRITE	0x02	// Write data to memory array beginning at selected address
#define SPIRAM_EDIO		0x3B	// Enter Dual I/O access (enter SDI bus mode)
#define SPIRAM_EQIO		0x38	// Enter Quad I/O access (enter SQI bus mode)
#define SPIRAM_RSTIO	0xFF	// Reset Dual and Quad I/O access (revert to SPI bus mode)
#define SPIRAM_RDMR		0x05	// Read Mode Register
#define SPIRAM_WRMR		0x01	// Write Mode Register

#define SPIRAM_MODE_BYTE		0x00
#define SPIRAM_MODE_PAGE		0x80
#define SPIRAM_MODE_SEQUENTIAL	0x40
#define SPIRAM_MODE_RESERVED	0xC0

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

FrameBuffer::FrameBuffer(SPIClass& spi, uint8_t cs_pin, uint32_t framebuffer_address, int16_t w, int16_t h) : Adafruit_GFX(w, h), IOSPI(spi) { 
// create a 3+1 bpp framebuffer (1bit=red/black, 3bit grey/red level, 2 pixels per byte)
	_cs_pin = cs_pin;
	
	pinMode(_cs_pin, OUTPUT);
	digitalWrite(_cs_pin, 1); // active low
	
	reset_23LC1024();
	set_spiram_mode(SPIRAM_MODE_SEQUENTIAL);
	
	_fb_addr = framebuffer_address;
	_fb_size_bytes = (w / 2) * h;	// 2 pixels/byte
	_fb_is_initialized = true;
}

FrameBuffer::~FrameBuffer() {
	_fb_is_initialized = false;
	_fb_size_bytes = 0;
	_fb_addr = 0;
}

void FrameBuffer::reset_23LC1024() {
	IOSPI.begin();

	digitalWrite(_cs_pin, 0); // active low
	IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
	IOSPI.transfer(SPIRAM_RSTIO);
	IOSPI.endTransaction();
	digitalWrite(_cs_pin, 1); // active low
	
	set_spiram_mode(SPIRAM_MODE_BYTE);
}

void FrameBuffer::set_spiram_mode(uint8_t mode) {
	digitalWrite(_cs_pin, 0); // active low
	IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
	IOSPI.transfer(SPIRAM_WRMR);
	IOSPI.transfer(mode);
	IOSPI.endTransaction();
	digitalWrite(_cs_pin, 1); // active low	
}

uint8_t FrameBuffer::get_spiram_mode() {
	uint8_t val = 0;
	
	digitalWrite(_cs_pin, 0); // active low
	IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
	IOSPI.transfer(SPIRAM_RDMR);
	val = IOSPI.transfer(0);
	IOSPI.endTransaction();
	digitalWrite(_cs_pin, 1); // active low	
	
	return val;
}

uint8_t FrameBuffer::peek(uint32_t address) {
	if (address == cached_address) { // one byte at one address holds 2 pixels (4bits each), so this should halve the number of reads
		return cached_byte;
	}
	else {
		if (!cache_flushed)
		{
			poke(cached_address, cached_byte, true); // flush the existing cached byte to the existing cached address
		}

		uint8_t val = 0;
		uint8_t buf[4];
		
		buf[0] = (uint8_t)SPIRAM_READ;					// read command
		buf[1] = (uint8_t)((address & 0xFFFFFF) >> 16);	// 24 bit address
		buf[2] = (uint8_t)((address & 0xFFFF) >> 8);
		buf[3] = (uint8_t)(address & 0xFF);
		
		digitalWrite(_cs_pin, 0); // active low
		IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
		for (uint8_t i = 0; i < 4; i++) {	// send read command and address
			IOSPI.transfer(buf[i]);	
		}
		val = IOSPI.transfer(0);			// read byte from memory at selected address
		IOSPI.endTransaction();					
		digitalWrite(_cs_pin, 1); // active low

		cached_address = address;
		cached_byte = val;
		cache_flushed = true;
		return val;
	}
}

void FrameBuffer::poke(uint32_t address, uint8_t val, bool flush = false) {
	if (address == cached_address && !flush) { // one byte at one address holds 2 pixels (4bits each), so this should halve the number of writes
		if (cached_byte != val) {
			cached_byte = val;
			cache_flushed = false;
		}
	}
	else {
		if (!cache_flushed){		
			uint8_t buf[5];
			
			buf[0] = (uint8_t)SPIRAM_WRITE;					// write command
			buf[1] = (uint8_t)((cached_address & 0xFFFFFF) >> 16);	// 24 bit address
			buf[2] = (uint8_t)((cached_address & 0xFFFF) >> 8);
			buf[3] = (uint8_t)(cached_address & 0xFF);
			buf[4] = cached_byte; //val;							// byte to write
			
			digitalWrite(_cs_pin, 0); // active low
			IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
			for (uint8_t i = 0; i < 5; i++) {
				IOSPI.transfer(buf[i]);	
			}
			IOSPI.endTransaction();
			digitalWrite(_cs_pin, 1); // active low
		}

		cached_byte = val;
		cached_address = address;
		cache_flushed = false;
	}
}

void FrameBuffer::cls() {
	DEBUG_PRT.print("Cls()..");
	
	cached_address = -1;
	cached_byte = 0;
	cache_flushed = true; // set 2 pixel (byte) cache to reset state

	if (!_fb_is_initialized) return;
	
	uint8_t spiram_mode = get_spiram_mode();
	set_spiram_mode(SPIRAM_MODE_SEQUENTIAL);

	uint8_t buf[4];
	
	buf[0] = (uint8_t)SPIRAM_WRITE;					// write command
	buf[1] = (uint8_t)((_fb_addr & 0xFFFFFF) >> 16);	// 24 bit address
	buf[2] = (uint8_t)((_fb_addr & 0xFFFF) >> 8);
	buf[3] = (uint8_t) (_fb_addr & 0xFF);
	
	digitalWrite(_cs_pin, 0); // active low
	IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
	for (uint8_t i = 0; i < 4; i++) {	// send read command and address
		IOSPI.transfer(buf[i]);	
	}
	
	for (uint32_t i = 0; i < _fb_size_bytes; i++) {
		IOSPI.transfer(0);			// write zeroes into the framebuffer in sequential mode
	}
	
	IOSPI.endTransaction();					
	digitalWrite(_cs_pin, 1); // active low

	set_spiram_mode(spiram_mode);

	DEBUG_PRT.println("done.");
}

void FrameBuffer::drawPixel(int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

	// check rotation, move pixel around if necessary

	switch (getRotation())
	{
	case 1:
		_swap_int16_t(x, y);
		x = WIDTH - x - 1;
		break;
	case 2:
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		break;
	case 3:
		_swap_int16_t(x, y);
		y = HEIGHT - y - 1;
		break;
	}

	uint32_t addr = (x / 2) + ((y * WIDTH) / 2); // 4bpp -> 8/4 == 2
	if (addr >= _fb_size_bytes) return; // if address is outside framebuffer

	uint8_t pixelvalue = encodePixel(color); // low 4 bits will contain the encoded pixel value

	addr += _fb_addr;

	if (x % 2 == 0) { // even pixel
		poke(addr, ((peek(addr) & 0x0F) | (pixelvalue << 4)));
	}
	else {
		poke(addr, ((peek(addr) & 0xF0) | pixelvalue));
	}
}

void FrameBuffer::drawPixel(int16_t x, int16_t y, uint16_t color, int saturation)
{		
	if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

	// check rotation, move pixel around if necessary

	switch (getRotation())
	{
	case 1:
		_swap_int16_t(x, y);
		x = WIDTH - x - 1;
		break;
	case 2:
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		break;
	case 3:
		_swap_int16_t(x, y);
		y = HEIGHT - y - 1;
		break;
	}

//	if ((x < 0) || (x >= PANEL_SIZE_X) || (y < 0) || (y >= PANEL_SIZE_Y)) return;

	bool bRed = false;
	saturation = (saturation & 0xF) >> 1;
	
	if (color == GxEPD_RED) {
		bRed = true;
	}
	
	if (color == GxEPD_WHITE) {
		saturation = (~saturation) & 0x7;
	}

	uint32_t addr = (x / 2) + ((y * WIDTH) / 2); // 4bpp -> 8/4 == 2
	
	if (addr >= _fb_size_bytes) return; // if address is outside framebuffer

	uint8_t pixelvalue = saturation; // low 4 bits will contain the encoded pixel value
	if (bRed) pixelvalue |= 0x8;	 // set the red bit (bit 4) if it is a red scale rather than a grey scale

	addr += _fb_addr;

	if (x % 2 == 0) { // even pixel
		poke(addr, ((peek(addr) & 0x0F) | (pixelvalue << 4)));
	}
	else {
		poke(addr, ((peek(addr) & 0xF0) | pixelvalue));
	}
}

uint8_t FrameBuffer::readPixel(int16_t x, int16_t y, bool bUseRotation) {
	if (bUseRotation) {
		if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return 0;

		// check rotation, move pixel around if necessary
		switch (getRotation())
		{
		case 1:
			_swap_int16_t(x, y);
			x = WIDTH - x - 1;
			break;
		case 2:
			x = WIDTH - x - 1;
			y = HEIGHT - y - 1;
			break;
		case 3:
			_swap_int16_t(x, y);
			y = HEIGHT - y - 1;
			break;
		}
	}
	else {
		if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT)) return 0xff;
	}
	
	uint32_t addr = x / 2 + y * WIDTH / 2; // 4bpp -> 8/4 == 2
	if (addr >= _fb_size_bytes) return 0; // if address is outside framebuffer

	uint8_t pixelvalue = 0; // low 4 bits will contain the encoded pixel value

	addr += _fb_addr;

	if (x % 2 == 0) { // even pixel
		pixelvalue = ((peek(addr) & 0xF0) >> 4);
	}
	else {
		pixelvalue = (peek(addr) & 0x0F);
	}
	
	return pixelvalue;	
}

void FrameBuffer::render(GxEPD_Class& ePaper) {
	#ifdef GAMMA_CORRECT_FONT
	bool bCorrectRed = true;
	#else
	bool bCorrectRed = false;
	#endif
	
	// flush the single byte 2 pixel cache
	poke(cached_address, cached_byte, true); // write out last byte if write is pending (will write if the cache_flushed flag is false)
	cached_address = -1;
	cached_byte = 0;
	cache_flushed = true; // set 2 pixel (byte) cache to reset state

	int16_t page_x0 = 0; 
	int16_t page_y0 = _displayPage * PAGE_HEIGHT;	
	int16_t page_x1 = PAGE_WIDTH;
	int16_t page_y1 = (page_y0 + PAGE_HEIGHT) - 1;
	
	uint16_t x = page_x0;
	uint16_t y = page_y0;

	uint32_t fb_start_addr = _fb_addr + ((page_x0 / 2) + ((page_y0 * PAGE_WIDTH) / 2));
	uint32_t fb_end_addr   = _fb_addr + ((page_x1 / 2) + ((page_y1 * PAGE_WIDTH) / 2));

	uint32_t bytesremaining = fb_end_addr - fb_start_addr;
	uint32_t fb_cur_addr = fb_start_addr; //_fb_addr + (x / 2) + ((y * PANEL_SIZE_X) / 2);

	uint32_t pxbuf_index = 0;
	uint32_t bytestoread = 0;

	uint8_t cur_rotation = getRotation();
	setRotation(0);

	uint8_t spiram_mode = get_spiram_mode();
	set_spiram_mode(SPIRAM_MODE_SEQUENTIAL);

	while (bytesremaining > 0) {
		// read lines between x0,y0 and x1,y1 into pxbuf, in PXBUF_SIZE chunks
		// fill the pxbuffer
		bytestoread = bytesremaining > PXBUF_SIZE ? PXBUF_SIZE : bytesremaining;
		bytesremaining -= bytestoread;

		uint8_t val = 0;
		uint8_t buf[4];
		
		buf[0] = (uint8_t)SPIRAM_READ;					// read command
		buf[1] = (uint8_t)((fb_cur_addr & 0xFFFFFF) >> 16);	// 24 bit address
		buf[2] = (uint8_t)((fb_cur_addr & 0xFFFF) >> 8);
		buf[3] = (uint8_t) (fb_cur_addr & 0xFF);
		
		digitalWrite(_cs_pin, 0); // active low
		IOSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
		for (uint8_t i = 0; i < 4; i++) {	// send read command and address
			IOSPI.transfer(buf[i]);	
		}
		
		for (uint32_t i = 0; i < bytestoread; i++) {
			pxbuf[i] = IOSPI.transfer(0);			// read bytes from memory at selected address (sequential mode)
		}
		
		IOSPI.endTransaction();					
		digitalWrite(_cs_pin, 1); // active low

		fb_cur_addr += bytestoread;
		pxbuf_index = 0;
				
		while (pxbuf_index < bytestoread) {
			uint8_t lpixel = (pxbuf[pxbuf_index] & 0xF0) >> 4;
			uint8_t rpixel = (pxbuf[pxbuf_index] & 0x0F);

			uint16_t lcolor = GxEPD_BLACK;
			if (lpixel & 0x08) { 
				lcolor = GxEPD_RED;	// if red bit is set
			}
			int lsaturation = (lpixel & 0x7) << 1; // saturation arg is 4 bit, but lsbit is thrown away

			if (lsaturation != 0) { // 0 = white, don't draw pixel if so (screen will have been cls'd on initialization)
				ePaper.drawPixel(x, y, lcolor, lsaturation, bCorrectRed);			
			}
			
			uint16_t rcolor = GxEPD_BLACK;
			if (rpixel & 0x08) { 
				rcolor = GxEPD_RED;	// if red bit is set
			}
			int rsaturation = (rpixel & 0x7) << 1; // saturation arg is 4 bit, but lsbit is thrown away
			
			if (rsaturation != 0) { // 0 = white, don't draw pixel if so (screen will have been cls'd on initialization)
				ePaper.drawPixel(x+1, y, rcolor, rsaturation, bCorrectRed); 
			}
					
			x+=2;	// 2 pixels per byte
			if (x >= page_x1) {
				x = page_x0;
				y++;
				//pxbuf_index += modulo;
			}
			//else {
			//	pxbuf_index += 1;
			//}
			pxbuf_index++;	// this code will assume that the page is the full width of the display (so no modulo to add on when x>=x1)
		}	
	}

	set_spiram_mode(spiram_mode);
	setRotation(cur_rotation);
}

uint8_t FrameBuffer::encodePixel(uint16_t color) { // returns a 4 bit value, b3 = r/b, b2, b1, b0 = intensity, from an rgb 565 16 bit value
	uint16_t r = (color >> 13) & 0x7; // get r as 3 ms bits (r is 5 bits) [r r r x x]
	uint16_t g = (color >>  8) & 0x7; // get g as 3 ms bits (g is 6 bits) [g g g x x x]
	uint16_t b = (color >>  2) & 0x7; // get b as 3 ms bits (b is 5 bits) [b b b x x]
	
	bool bRed = true;
	
	uint8_t pixelvalue = (~g) & 0x7; // if it's a red colour, r will always be set to full intensity, and g and b to lower or equal values, to give pinks, so use g as the intensity, since in both red and grey colours it will be set according to the brightness of the colour
	
	if (r > g && r > b) { // not a grey level
		pixelvalue |= 0x8; 	// set the red bit
	}

	return pixelvalue;
}

uint16_t FrameBuffer::decodepixel(uint8_t pixel) {	// decode pixel (lower 4 bits of byte)
	uint16_t color = 0;
	
	uint16_t pixel16 = pixel & 0x0F; //get rid of top 4 bits
	
	if (pixel16 & 0x8) { // is the red bit set
		color = ((0x7 << 13) | 0x1800) | ((pixel16 << 8) | 0x00E0) | ((pixel16 << 2) | 0x0003); // set the lowest 2 bits to 1 and use the 3 bit intensity value as the top 3 red bits
	}
	else {
		color = ((pixel16 << 13) | 0x1800) | ((pixel16 << 8) | 0x00E0) | ((pixel16 << 2) | 0x0003);
	}
	
	return color;
}

uint16_t FrameBuffer::makeColor(uint8_t intensity, bool bRed) {
	intensity &= 0x7;

	if (bRed) {
		return ((0x7 << 13) | 0x1800) | ((intensity << 8) | 0x00E0) | ((intensity << 2) | 0x0003); // rgb565 value, set red intensity only and g,b to intensity (for pinks)
	}
	else {
		return ((intensity << 13) | 0x1800) | ((intensity << 8) | 0x00E0) | ((intensity << 2) | 0x0003); // red, green and blue intensities the same
	}
	
	return 0;
	
}

void FrameBuffer::setDisplayPage(int displayPageNumber) {
	_displayPage = displayPageNumber;
}
