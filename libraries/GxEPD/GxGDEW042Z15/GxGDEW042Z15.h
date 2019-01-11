// class GxGDEW042Z15 : Display class for GDEW042Z15 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=515.html
// Controller: IL0398 : http://www.good-display.com/download_detail/downloadsId=537.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#ifndef _GxGDEW042Z15_H_
#define _GxGDEW042Z15_H_

#include "../GxEPD.h"

#define GxGDEW042Z15_WIDTH 400
#define GxGDEW042Z15_HEIGHT 300

#define GxGDEW042Z15_BUFFER_SIZE (uint32_t(GxGDEW042Z15_WIDTH) * uint32_t(GxGDEW042Z15_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW042Z15_HEIGHT
#define GxGDEW042Z15_PAGES 30

#define GxGDEW042Z15_PAGE_HEIGHT (GxGDEW042Z15_HEIGHT / GxGDEW042Z15_PAGES)
#define GxGDEW042Z15_PAGE_SIZE (GxGDEW042Z15_BUFFER_SIZE / GxGDEW042Z15_PAGES)

#define LUT_MODE_NORMAL 0x00						// set normal lut
#define LUT_MODE_COMPOSITE 0x01						// set lut for compositing (no pre-clear)
#define LUT_MODE_CLEAR 0x02							// set lut for clearing display
#define EPD_REFRESH_NUMBER_1BPP 1			//was 3		// maximum number of times to refresh the display to produce all supported shades (2bpp)
#define EPD_REFRESH_NUMBER_2BPP 3			//was 3		// maximum number of times to refresh the display to produce all supported shades (2bpp)
#define EPD_REFRESH_NUMBER_3BPP 7			//was 3		// maximum number of times to refresh the display to produce all supported shades (2bpp)
#define LUT_MODE_1BPP 1
#define LUT_MODE_2BPP 2
#define LUT_MODE_3BPP 3


// EPD4IN2 commands
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13
#define LUT_FOR_VCOM                                0x20 
#define LUT_WHITE_TO_WHITE                          0x21
#define LUT_BLACK_TO_WHITE                          0x22
#define LUT_WHITE_TO_BLACK                          0x23
#define LUT_BLACK_TO_BLACK                          0x24
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_COMMAND                  0x40
#define TEMPERATURE_SENSOR_SELECTION                0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define RESOLUTION_SETTING                          0x61
#define GSST_SETTING                                0x65
#define GET_STATUS                                  0x71
#define AUTO_MEASUREMENT_VCOM                       0x80
#define READ_VCOM_VALUE                             0x81
#define VCM_DC_SETTING                              0x82
#define PARTIAL_WINDOW                              0x90
#define PARTIAL_IN                                  0x91
#define PARTIAL_OUT                                 0x92
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAMMING                          0xA1
#define READ_OTP                                    0xA2
#define POWER_SAVING                                0xE3

#define CASCADE_SETTING								0xE0
#define FORCE_TEMPERATURE							0xE5

class GxGDEW042Z15 : public GxEPD
{
  public:
#if defined(ESP8266)
    //GxGDEW042Z15(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    GxGDEW042Z15(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEW042Z15(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(uint32_t serial_diag_bitrate = 0); // = 0 : disabled
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
	
	void drawPixel(int16_t x, int16_t y, uint16_t color, int saturation);	//PLL 06-01-2019
	void resetRefreshNumber(int bpp);										//
	int getRefreshNumber();													//
	bool decRefreshNumber();												//
	int getMaxRefreshNumber();												//
	
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, for example bitmaps
    void drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, general version
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode = bm_normal); // only bm_normal, bm_invert, bm_partial_update modes implemented
    void eraseDisplay(bool using_partial_update = false);
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    // terminate cleanly updateWindow or updateToWindow before removing power or long delays
    void powerDown();
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW042Z15_PAGES times
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
    void drawCornerTest(uint8_t em = 0);
  private:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    void _writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h);
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep(void);
    void _waitWhileBusy(const char* comment = 0);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

	void _writeLUT_Normal(void);			//PLL 06-01-2019
	void _writeLUT_Composite(void);			//
	void _writeLUT(void);					//
	void _writeCommand(uint8_t command);	//
	void _writeData(uint8_t data);			//
	void _writeRegisters(void);				//PLL 08-01-2019

  private:  
//#if defined(__AVR)
    uint8_t _black_buffer[GxGDEW042Z15_PAGE_SIZE];
    uint8_t _red_buffer[GxGDEW042Z15_PAGE_SIZE];
//#else
//    uint8_t _black_buffer[GxGDEW042Z15_BUFFER_SIZE];
//    uint8_t _red_buffer[GxGDEW042Z15_BUFFER_SIZE];
//#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    bool _diag_enabled;
    int8_t _rst;
    int8_t _busy;
	
	int8_t _refreshnumber;					//
	int8_t _epd_refresh_max;				//
	int8_t _lut_mode;						//PLL 06-01-2019	
    static const uint8_t lut_20_vcomDC[];	// standard waveshare luts
    static const uint8_t lut_21[];			//
    static const uint8_t lut_22_red[];		//
    static const uint8_t lut_23_white[];	//
    static const uint8_t lut_24_black[];	//
	
    static uint8_t lut_vcom_dc_comp[]; // luts for compositing (for grey/red levels) //PLL 06-01-2019
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
#define GxEPD_Class GxGDEW042Z15
#define GxEPD_WIDTH GxGDEW042Z15_WIDTH
#define GxEPD_HEIGHT GxGDEW042Z15_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW042Z15/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW042Z15/BitmapExamples.h"
#endif

#endif

