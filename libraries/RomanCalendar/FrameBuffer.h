#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include "RCGlobals.h"
#include <Arduino.h>
#include <SPI.h>
#include <pins_arduino.h>
#include <Adafruit_GFX.h>

#ifdef EPAPER_GxGDEW027C44
	#include <GxEPD.h>
	#include <GxGDEW027C44/GxGDEW027C44.h>      // 2.7" b/w/r 176x264 GxGDEW027C44/GxGDEW027C44.cpp
#endif

#ifdef EPAPER_GxGDEW042Z15
	#include <GxEPD.h>
	#include <GxGDEW042Z15/GxGDEW042Z15.h>      // 4.2" b/w/r
#endif

class FrameBuffer : public Adafruit_GFX {
public:
	uint32_t _fb_addr = 0;
	uint32_t _fb_size_bytes = 0;
	bool _fb_is_initialized = false;
	uint8_t _cs_pin = 0;
	SPIClass& IOSPI;
	int _displayPage = 0;

	#define PXBUF_SIZE 1024
	uint8_t pxbuf[PXBUF_SIZE];
	
	FrameBuffer(SPIClass& spi, uint8_t cs_pin, uint32_t framebuffer_address, int16_t w, int16_t h); // create a 3+1 bpp framebuffer	(1bit=red/black, 3bit grey/red level, 2 pixels per byte)
	~FrameBuffer();
	
	void reset_23LC1024();
	void set_spiram_mode(uint8_t mode);
	uint8_t get_spiram_mode();
	
	uint8_t peek(uint32_t address);
	void poke(uint32_t address, uint8_t val);

	void cls();
	void drawPixel(int16_t x, int16_t y, uint16_t color);
	void drawPixel(int16_t x, int16_t y, uint16_t color, int saturation);
	uint8_t readPixel(int16_t x, int16_t y, bool bUseRotation = true);
	
	void render(GxEPD_Class& ePaper);
	
	static uint8_t encodePixel(uint16_t color);
	static uint16_t decodepixel(uint8_t pixel);
	
	uint16_t makeColor(uint8_t intensity, bool bRed);
	
	void setDisplayPage(int displayPageNumber);
};


#endif