// class GxGDEW027C44 : Display class for GDEW027C44 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=515.html
// Controller: IL91874 : http://www.good-display.com/download_detail/downloadsId=539.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#ifndef _GxGDEW027C44_H_
#define _GxGDEW027C44_H_

#include "../GxEPD.h"

#define GxGDEW027C44_WIDTH 176
#define GxGDEW027C44_HEIGHT 264

#define GxGDEW027C44_BUFFER_SIZE (uint32_t(GxGDEW027C44_WIDTH) * uint32_t(GxGDEW027C44_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW027C44_HEIGHT
#define GxGDEW027C44_PAGES 12

#define GxGDEW027C44_PAGE_HEIGHT (GxGDEW027C44_HEIGHT / GxGDEW027C44_PAGES)
#define GxGDEW027C44_PAGE_SIZE (GxGDEW027C44_BUFFER_SIZE / GxGDEW027C44_PAGES)

#define LUT_MODE_NORMAL 0x00						// set normal lut
#define LUT_MODE_COMPOSITE 0x01						// set lut for compositing (no pre-clear)
#define LUT_MODE_CLEAR 0x02							// set lut for clearing display
#define EPD_REFRESH_NUMBER_1BPP 1			//was 3		// maximum number of times to refresh the display to produce all supported shades (2bpp)
#define EPD_REFRESH_NUMBER_2BPP 3			//was 3		// maximum number of times to refresh the display to produce all supported shades (2bpp)
#define EPD_REFRESH_NUMBER_3BPP 7			//was 3		// maximum number of times to refresh the display to produce all supported shades (2bpp)
#define LUT_MODE_1BPP 1
#define LUT_MODE_2BPP 2
#define LUT_MODE_3BPP 3
class GxGDEW027C44 : public GxEPD
{
  public:
#if defined(ESP8266)
    //GxGDEW027C44(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    GxGDEW027C44(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEW027C44(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    
	void drawPixel(int16_t x, int16_t y, uint16_t color, int saturation); // saturation 3=100% 2=66% 1=33% 0=0% (2bpp)
	void resetRefreshNumber(int bpp);					  // resets the refresh counter (3 refreshes are needed for 3 shades of grey/red)
	int getRefreshNumber();								  // when 0, can stop refreshing
	bool decRefreshNumber();							  // decrements the refresh number, returns false when refresh number = 0
	int getMaxRefreshNumber();							  // returns the maximum number of refreshes for the selected number of bits per pixel
	
    void init(uint32_t serial_diag_bitrate = 0); // = 0 : disabled
	//void setMode(int mode); // 0 = LUT_MODE_NORMAL (clear before refresh) 1 = LUT_MODE_COMPOSITE (no clear before refresh) 2 = LUT_MODE_CLEAR (used for clearing display)
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, for example bitmaps
    void drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, general version
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode = bm_normal); // only bm_normal, bm_invert modes implemented
    void eraseDisplay(bool using_partial_update = false); // parameter ignored
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    // terminate cleanly, not needed as all screen drawing methods of this class do power down
    void powerDown();
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW027C44_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawPaged(void (*drawCallback)(uint32_t), uint32_t);
    void drawPaged(void (*drawCallback)(const void*), const void*);
    void drawPaged(void (*drawCallback)(const void*, const void*), const void*, const void*);
    // paged drawing to screen rectangle at (x,y) using partial update
    void drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t);
    void drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void*);
    void drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void*, const void*);
    void drawCornerTest(uint8_t em = 0x01);
  private:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    void _writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h);
    void _setPartialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeLUT_Normal();
	void _writeLUT_Composite(void);		//PLL 18-12-2018
    void _writeLUT(void);				//PLL 29-12-2018
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment = 0);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  private:
//#if defined(__AVR)
    uint8_t _black_buffer[GxGDEW027C44_PAGE_SIZE];
    uint8_t _red_buffer[GxGDEW027C44_PAGE_SIZE];
//#else
//    uint8_t _black_buffer[GxGDEW027C44_BUFFER_SIZE];
//    uint8_t _red_buffer[GxGDEW027C44_BUFFER_SIZE];
//#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    bool _diag_enabled;
    int8_t _rst;
    int8_t _busy;

	int8_t _refreshnumber;		//PLL 18-12-2018
	int8_t _epd_refresh_max;	//PLL 29-12-2018
	int8_t _lut_mode;			//PLL 18-12-2018

    static const uint8_t lut_20_vcomDC[];	// standard waveshare luts
    static const uint8_t lut_21[];
    static const uint8_t lut_22_red[];
    static const uint8_t lut_23_white[];
    static const uint8_t lut_24_black[];
    	
    static uint8_t lut_vcom_dc_comp[]; // luts for compositing (for grey/red levels) //PLL 18-12-2018
    static uint8_t lut_ww_comp[];
    static uint8_t lut_bw_comp[];
    static uint8_t lut_wb_comp[];
    static uint8_t lut_bb_comp[];


#if defined(ESP8266) || defined(ESP32)
  public:
    // the compiler of these packages has a problem with signature matching to base classes
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
    {
      Adafruit_GFX::drawBitmap(x, y, bitmap, w, h, color);
    };
#endif
};

#ifndef GxEPD_Class
#define GxEPD_Class GxGDEW027C44
#define GxEPD_WIDTH GxGDEW027C44_WIDTH
#define GxEPD_HEIGHT GxGDEW027C44_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW027C44/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW027C44/BitmapExamples.h"
#endif

#endif

