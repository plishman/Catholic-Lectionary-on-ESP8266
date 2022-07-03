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

#include "GxGDEW042Z15.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

// partial update produces distortion on the right half of the screen in b/w/r mode (on my display)

// this workaround updates the whole screen
#define USE_PARTIAL_UPDATE_WORKAROUND

#define GxGDEW042Z15_BUSY_TIMEOUT 20000000

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

//lut_vcom0
const uint8_t GxGDEW042Z15::lut_20_vcomDC[] =
{
  0x00, 0x17, 0x00, 0x00, 0x00, 0x02,        
  0x00, 0x17, 0x17, 0x00, 0x00, 0x02,        
  0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,        
  0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,        
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//lut_ww
const uint8_t GxGDEW042Z15::lut_21[] = 
{
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//lut_bw
const uint8_t GxGDEW042Z15::lut_22_red[] = 
{
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    
};

//lut_wb
const uint8_t GxGDEW042Z15::lut_23_white[] =
{
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,          
};

//lut_bb
const uint8_t GxGDEW042Z15::lut_24_black[] =
{
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,           
};

//////////////////////////////////
/// composite (no invert/clear) 7 shades + white (3bpp)
//RED
#define LUT_VCOM_DC_RED_COUNT1 		7
#define LUT_VCOM_DC_RED_COUNT2 		8
#define LUT_VCOM_DC_RED_REPCOUNT 	11

#define LUT_RED_COUNT1 				7
#define LUT_RED_COUNT2 				8
#define LUT_RED_REPCOUNT 			11

#define LUT_RED_REPS_7 				0x0F // was 0x0F  PLL-07-07-2021	
#define LUT_RED_C1_7 				0x03  // was 0x03  PLL-07-07-2021	
#define LUT_RED_C2_7 				0x0E  // was 0x0E  PLL-07-07-2021	
		
#define LUT_RED_REPS_6 				0x01	
#define LUT_RED_C1_6 				0x01  // was 0x01  PLL-07-07-2021
#define LUT_RED_C2_6 				0x0C  // was 0x0C  PLL-07-07-2021
		
#define LUT_RED_REPS_5 				0x01	
#define LUT_RED_C1_5 				0x01  // was 0x01  PLL-07-07-2021
#define LUT_RED_C2_5 				0x0C  // was 0x0C  PLL-07-07-2021
		
#define LUT_RED_REPS_4 				0x01	
#define LUT_RED_C1_4 				0x01  // was 0x01  PLL-07-07-2021
#define LUT_RED_C2_4 				0x0C  // was 0x0C  PLL-07-07-2021
		
#define LUT_RED_REPS_3 				0x01	
#define LUT_RED_C1_3 				0x01  // was 0x01  PLL-07-07-2021
#define LUT_RED_C2_3 				0x0C  // was 0x0C  PLL-07-07-2021
		
#define LUT_RED_REPS_2 				0x01	
#define LUT_RED_C1_2 				0x01
#define LUT_RED_C2_2 				0x0C  // was 0x0C  PLL-07-07-2021
		
#define LUT_RED_REPS_1 				0x01	
#define LUT_RED_C1_1 				0x01
#define LUT_RED_C2_1 				0x0C  // was 0x0C  PLL-07-07-2021

//BLACK
#define LUT_VCOM_DC_BLACK_COUNT1 	1
#define LUT_VCOM_DC_BLACK_COUNT2 	2
#define LUT_VCOM_DC_BLACK_REPCOUNT 	5

#define LUT_BLACK_COUNT1 			1
#define LUT_BLACK_COUNT2 			2
#define LUT_BLACK_REPCOUNT 			5

#define LUT_BLACK_REPS_7 			0x08	//new pal was:0x0F	//was 8 8*10*2 = 160
#define LUT_BLACK_C1_7 				0x0A
#define LUT_BLACK_C2_7 				0x0A
		
#define LUT_BLACK_REPS_6 		  	0x04 //0x03	//8*3*2 = 48
#define LUT_BLACK_C1_6 				0x03 //0x03
#define LUT_BLACK_C2_6 				0x03 //0x03
		
#define LUT_BLACK_REPS_5 			0x03  //was 0x03	PLL-08-07-2021 //4*3*2 = 24
#define LUT_BLACK_C1_5 				0x02  //was 0x02  PLL-07-07-2021
#define LUT_BLACK_C2_5				0x02  //was 0x02  PLL-07-07-2021
		
#define LUT_BLACK_REPS_4 			0x02	//new pal 0x03	//2*3*2 = 12
#define LUT_BLACK_C1_4 				0x01	//new pal 0x02  //was 0x01  PLL-07-07-2021
#define LUT_BLACK_C2_4 				0x01	//new pal 0x02  //was 0x01  PLL-07-07-2021
		
#define LUT_BLACK_REPS_3 			0x02	//1*3*2 = 6
#define LUT_BLACK_C1_3 				0x01  //was 0x01  PLL-07-07-2021
#define LUT_BLACK_C2_3 				0x01  //was 0x01  PLL-07-07-2021
		
#define LUT_BLACK_REPS_2 			0x01	//was 0x01  PLL-07-07-2021 //1*2*2 = 4
#define LUT_BLACK_C1_2 				0x01  //was 0x01  PLL-07-07-2021
#define LUT_BLACK_C2_2 				0x01  //was 0x01  PLL-07-07-2021 
		
#define LUT_BLACK_REPS_1 			0x01	//1*1*2 = 2
#define LUT_BLACK_C1_1 				0x01
#define LUT_BLACK_C2_1 				0x01
									// 48+24+12+6+4+2 = 96

									

//////////////////////////////////
/// composite (no invert/clear) 3 shades + white (2bpp)
//RED
#define _2BPP_LUT_RED_REPS_2 		0x01	
#define _2BPP_LUT_RED_C1_2 			0x01
#define _2BPP_LUT_RED_C2_2 			0x0C
		
#define _2BPP_LUT_RED_REPS_1 		0x01	
#define _2BPP_LUT_RED_C1_1 			0x01
#define _2BPP_LUT_RED_C2_1 			0x0C

//BLACK	
#define _2BPP_LUT_BLACK_REPS_2 		0x03	
#define _2BPP_LUT_BLACK_C1_2 		0x02
#define _2BPP_LUT_BLACK_C2_2 		0x02
		
#define _2BPP_LUT_BLACK_REPS_1 		0x01	
#define _2BPP_LUT_BLACK_C1_1 		0x06
#define _2BPP_LUT_BLACK_C2_1		0x06

//command 0x01 (power setting) [01] 3a 00 2b 2b 11

//3a 00 2b 2b 11 1e 				what is 0x1e?



//LUT_FOR_VCOM (20h)
//R20H
uint8_t GxGDEW042Z15::lut_vcom_dc_comp[] =
{
	0x00 , 0x02 , 0x02 , 0x00 , 0x00 , 0x01 , 
	0x00 , 0x03 , 0x0e , 0x00 , 0x00 , 0x02 , 
	0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0x01 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 
};

//LUT_WHITE_TO_WHITE (21h)
//R21H
uint8_t GxGDEW042Z15::lut_ww_comp[] =
{
	0x40 , 0x02 , 0x02 , 0x00 , 0x00 , 0x01 ,
	0x00 , 0x03 , 0x0e , 0x00 , 0x00 , 0x02 ,
	0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0x01 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00   
};

//LUT_BLACK_TO_WHITE (22h)
//R22H  r
uint8_t GxGDEW042Z15::lut_bw_comp[] =
{
	0x80 , 0x02 , 0x02 , 0x00 , 0x00 , 0x01 , 
	0xb0 , 0x03 , 0x0e , 0x00 , 0x00 , 0x02 ,
	0xc0 , 0x01 , 0x00 , 0x00 , 0x00 , 0x01 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00   
};

//LUT_WHITE_TO_BLACK (23h)
//R23H  w
uint8_t GxGDEW042Z15::lut_bb_comp[] =
{
	0x50 , 0x02 , 0x02 , 0x00 , 0x00 , 0x01 ,  // 0x50 is VCOMH->VCOMH, was 0x40 (VCOMH -> VCM_DC)
	0x00 , 0x03 , 0x0e , 0x00 , 0x00 , 0x02 , 
	0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0x01 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00   
};

//LUT_BLACK_TO_BLACK (24h)
uint8_t GxGDEW042Z15::lut_wb_comp[] =
{
	0x20 , 0x02 , 0x02 , 0x00 , 0x00 , 0x01 ,
	0x00 , 0x03 , 0x0e , 0x00 , 0x00 , 0x02 ,
	0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0x01 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00   
};


GxGDEW042Z15::GxGDEW042Z15(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW042Z15_WIDTH, GxGDEW042Z15_HEIGHT), IO(io),
    _current_page(-1), _using_partial_mode(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEW042Z15::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW042Z15_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW042Z15_WIDTH - x - 1;
      y = GxGDEW042Z15_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW042Z15_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW042Z15_WIDTH / 8;
  if (_current_page < 0)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW042Z15_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW042Z15_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW042Z15_WIDTH / 8;
  }

  _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == GxEPD_WHITE) return;
  else if (color == GxEPD_BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  else if (color == GxEPD_RED) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2)) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}

void GxGDEW042Z15::init(uint32_t serial_diag_bitrate)
{
  if (serial_diag_bitrate > 0)
  {
    Serial.begin(serial_diag_bitrate);
    _diag_enabled = true;
  }
  IO.init();
  IO.setFrequency(4000000); // 4MHz
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
  _current_page = -1;
  _using_partial_mode = false;
}

void GxGDEW042Z15::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0x00;
  if (color == GxEPD_WHITE);
  else if (color == GxEPD_BLACK) black = 0xFF;
  else if (color == GxEPD_RED) red = 0xFF;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void GxGDEW042Z15::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10); // black
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    uint8_t data = i < sizeof(_black_buffer) ? _black_buffer[i] : 0x00;
    IO.writeDataTransaction(~data);
  }
  IO.writeCommandTransaction(0x13); // red
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    uint8_t data = i < sizeof(_red_buffer) ? _red_buffer[i] : 0x00;
    IO.writeDataTransaction(~data);
  }
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW042Z15::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW042Z15::drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size)
{
  drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_normal);
}

void GxGDEW042Z15::drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW042Z15_WIDTH - 1, GxGDEW042Z15_HEIGHT - 1);
    IO.writeCommandTransaction(0x10); // black
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // 0xFF is white
      if (i < black_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&black_bitmap[i]);
#else
        data = black_bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x13); // red
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // 0xFF is white
      if (i < red_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&red_bitmap[i]);
#else
        data = red_bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("drawPicture");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    IO.writeCommandTransaction(0x10); // black
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < black_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&black_bitmap[i]);
#else
        data = black_bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x13); // red
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < red_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&red_bitmap[i]);
#else
        data = red_bitmap[i];
#endif
        if (mode & bm_invert_red) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("drawPicture");
    _sleep();
  }
}

void GxGDEW042Z15::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_normal;
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW042Z15_WIDTH - 1, GxGDEW042Z15_HEIGHT - 1);
    IO.writeCommandTransaction(0x10); // black
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x13); // red
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF); // 0xFF is white
    }
    IO.writeCommandTransaction(0x92); // partial out
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawBitmap");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x10); // black
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x13); // red
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF); // 0xFF is white
    }
    IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    _sleep();
  }
}

void GxGDEW042Z15::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW042Z15_WIDTH - 1, GxGDEW042Z15_HEIGHT - 1);
    IO.writeCommandTransaction(0x10); // black
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF); // 0xFF is white
    }
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF); // 0xFF is white
    }
    IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x10); // black
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF); // 0xFF is white
    }
    IO.writeCommandTransaction(0x13); // red
    for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF); // 0xFF is white
    }
    IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEW042Z15::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEW042Z15_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEW042Z15_WIDTH - x - w - 1;
        y = GxGDEW042Z15_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEW042Z15_HEIGHT - y  - h - 1;
        break;
    }
  }
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  _writeToWindow(x, y, x, y, w, h);
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateWindow");
  IO.writeCommandTransaction(0x92); // partial out
}

void GxGDEW042Z15::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
  // the screen limits are the hard limits
  if (xs >= GxGDEW042Z15_WIDTH) return;
  if (ys >= GxGDEW042Z15_HEIGHT) return;
  if (xd >= GxGDEW042Z15_WIDTH) return;
  if (yd >= GxGDEW042Z15_HEIGHT) return;
  uint16_t xde = gx_uint16_min(GxGDEW042Z15_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GxGDEW042Z15_HEIGHT, yd + h) - 1;
  // soft limits, must send as many bytes as set by _setPartialRamArea
  uint16_t yse = ys + yde - yd; // inclusive
  uint16_t xss_d8 = xs / 8;
  uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde); // exclusive
  IO.writeCommandTransaction(0x10); // black
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
      IO.writeDataTransaction(~data);
    }
  }
  delay(2);
  //_setPartialRamArea(xd, yd, xde, yde);
  IO.writeCommandTransaction(0x13); // red
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
      IO.writeDataTransaction(~data);
    }
  }
#ifdef USE_PARTIAL_UPDATE_WORKAROUND
  _setPartialRamArea(0, 0, GxGDEW042Z15_WIDTH - 1, GxGDEW042Z15_HEIGHT - 1);
#endif
}

void GxGDEW042Z15::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEW042Z15_WIDTH - xs - w - 1;
        xd = GxGDEW042Z15_WIDTH - xd - w - 1;
        break;
      case 2:
        xs = GxGDEW042Z15_WIDTH - xs - w - 1;
        ys = GxGDEW042Z15_HEIGHT - ys - h - 1;
        xd = GxGDEW042Z15_WIDTH - xd - w - 1;
        yd = GxGDEW042Z15_HEIGHT - yd - h - 1;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEW042Z15_HEIGHT - ys  - h - 1;
        yd = GxGDEW042Z15_HEIGHT - yd  - h - 1;
        break;
    }
  }
  _wakeUp();
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  _writeToWindow(xs, ys, xd, yd, w, h);
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateToWindow");
  IO.writeCommandTransaction(0x92); // partial out
  delay(1000); // don't stress this display
}

void GxGDEW042Z15::powerDown()
{
  _sleep();
}

uint16_t GxGDEW042Z15::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe |= 0x0007; // byte boundary
  IO.writeCommandTransaction(0x90); // partial window
  IO.writeDataTransaction(x / 256);
  IO.writeDataTransaction(x % 256);
  IO.writeDataTransaction(xe / 256);
  IO.writeDataTransaction(xe % 256);
  IO.writeDataTransaction(y / 256);
  IO.writeDataTransaction(y % 256);
  IO.writeDataTransaction(ye / 256);
  IO.writeDataTransaction(ye % 256);
  //IO.writeDataTransaction(0x01); // distortion on full right half
  IO.writeDataTransaction(0x00); // distortion on right half
  uint16_t xb = x / 8; // first byte (containing first bit)
  uint16_t xeb = xe / 8; // last byte (containing last bit)
  return xeb - xb + 1; // number of bytes to transfer
}

void GxGDEW042Z15::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();

  #ifdef ESP8266
  unsigned long loopctr = 0; // PLL-03-07-2022 Call yield() every 1000 iterations of the loop to prevent watchdog crash
  #endif

  while (1)
  { //=0 BUSY
    if (digitalRead(_busy) == 1) break;
    delay(1);

    #ifdef ESP8266
    if (loopctr % 1000 == 0) { // PLL-03-07-2022 Call yield() every 1000 iterations of the loop to prevent watchdog crash
      yield();
    }
    #endif

    if (micros() - start > GxGDEW042Z15_BUSY_TIMEOUT)
    {
      if (_diag_enabled) Serial.println("Busy Timeout!");
      break;
    }

    #ifdef ESP8266
    loopctr++; // PLL-03-07-2022 Call yield() every 1000 iterations of the loop to prevent watchdog crash
    #endif

  }
  if (comment)
  {
#if !defined(DISABLE_DIAGNOSTIC_OUTPUT)
    if (_diag_enabled)
    {
      unsigned long elapsed = micros() - start;
      Serial.print(comment);
      Serial.print(" : ");
      Serial.println(elapsed);
    }
#endif
  }
  (void) start;
}

void GxGDEW042Z15::_wakeUp(void)
{
  if (_rst >= 0)
  {
    digitalWrite(_rst, 0);
    delay(1250);          // was 750 - lengthened due to slight instability when using grey/red level lookup tables 
    digitalWrite(_rst, 1);
    delay(850);           // was 500
  }
  IO.writeCommandTransaction(0x06); //boost
  IO.writeDataTransaction (0x17);
  IO.writeDataTransaction (0x17);
  IO.writeDataTransaction (0x17);
  IO.writeCommandTransaction(0x04);
  _waitWhileBusy("Power On");
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x0f); // LUT from OTP Pixel with B/W/R.

//3a 00 2b 2b 11
/*
    _writeCommand(POWER_SETTING);
    _writeData(0x3a);                  // VDS_EN, VDG_EN
    _writeData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    _writeData(0x2b);                  // VDH
    _writeData(0x2b);                  // VDL
    _writeData(0x11);                  // VDHR

    //_writeData(0x1e);                
*/						// 11v   10v    9v    8v   7v     6v    5v
	uint8_t voltages[8] = {0x2b, 0x26, 0x21, 0x1c, 0x17, 0x12, 0x0d};
	
	uint8_t v = voltages[_refreshnumber];
	
	
	_writeCommand(POWER_SETTING);
    _writeData(0x03);                  // VDS_EN, VDG_EN
    _writeData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    _writeData(0x2b);                  // VDH 0x2b = +11V 0x20 = +8.8V 0x08 = +4.0V
    _writeData(0x2b);                  // VDL 0x2b = -11V 0x20 = -8.8V 0x08 = -4.0V
    _writeData(0x2b);                  // VDHR 0x2b = +11V (red pixel)

    _writeCommand(BOOSTER_SOFT_START);
    _writeData(0x17);
    _writeData(0x17);
    _writeData(0x17);                  //07 0f 17 1f 27 2F 37 2f
    _writeCommand(POWER_ON);
	_waitWhileBusy("Power On");
//    _writeCommand(PANEL_SETTING);
   // _writeData(0xbf);    // KW-BF   KWR-AF  BWROTP 0f
  //  _writeData(0x0b);
//	_writeData(0x0F);  //300x400 Red mode, LUT from OTP
//	_writeData(0x1F);  //300x400 B/W mode, LUT from OTP
//	_writeData(0x3F); //300x400 B/W mode, LUT set by register
//	_writeData(0x2F); //300x400 Red mode, LUT set by register

    _writeCommand(PLL_CONTROL);
    _writeData(0x3A);        // 3A 100Hz   29 150Hz   39 200Hz    31 171Hz       3C 50Hz (default)    0B 10Hz
	//_writeData(0x0B);   //0B is 10Hz
    /* EPD hardware init end */


  
  //_writeLUT_Normal();	//PLL 06-01-2019
  _writeLUT();	//PLL 06-01-2019
  
}

//void GxGDEW042Z15::_wakeUp(void)
//{
//  if (_rst >= 0)
//  {
//    digitalWrite(_rst, 0);
//    delay(200);
//    digitalWrite(_rst, 1);		// was 10
//    delay(200);
//  }
///*
//  IO.writeCommandTransaction(0x06); //boost
//  IO.writeDataTransaction (0x17);
//  IO.writeDataTransaction (0x17);
//  IO.writeDataTransaction (0x17);
//  IO.writeCommandTransaction(0x04);
//  _waitWhileBusy("Power On");
//  IO.writeCommandTransaction(0x00);
//  IO.writeDataTransaction(0x0f); // LUT from OTP Pixel with B/W/R.
//*/
////3a 00 2b 2b 11
///*
//    _writeCommand(POWER_SETTING);
//    _writeData(0x3a);                  // VDS_EN, VDG_EN
//    _writeData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
//    _writeData(0x2b);                  // VDH
//    _writeData(0x2b);                  // VDL
//    _writeData(0x11);                  // VDHR
//
//    //_writeData(0x1e);                
//*/						// 11v   10v    9v    8v   7v     6v    5v
//	uint8_t voltages[8] = {0x2b, 0x26, 0x21, 0x1c, 0x17, 0x12, 0x0d};
//	
//	uint8_t v = voltages[_refreshnumber];
//	
//	
//	_writeCommand(POWER_SETTING);
//    _writeData(0x03);                  // VDS_EN, VDG_EN
//    _writeData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
//    _writeData(0x2b);                  // VDH 0x2b = +11V 0x20 = +8.8V 0x08 = +4.0V
//    _writeData(0x2b);                  // VDL 0x2b = -11V 0x20 = -8.8V 0x08 = -4.0V
//    _writeData(0x2b);                  // VDHR 0x2b = +11V (red pixel)
//
//    _writeCommand(BOOSTER_SOFT_START);
//    _writeData(0x17);
//    _writeData(0x17);
//    _writeData(0x17);                  //07 0f 17 1f 27 2F 37 2f
//    _writeCommand(POWER_ON);
//	_waitWhileBusy("Power On");
////    _writeCommand(PANEL_SETTING);
//   // _writeData(0xbf);    // KW-BF   KWR-AF  BWROTP 0f
//  //  _writeData(0x0b);
////	_writeData(0x0F);  //300x400 Red mode, LUT from OTP
////	_writeData(0x1F);  //300x400 B/W mode, LUT from OTP
////	_writeData(0x3F); //300x400 B/W mode, LUT set by register
////	_writeData(0x2F); //300x400 Red mode, LUT set by register
//
//    _writeCommand(PLL_CONTROL);
//    _writeData(0x3A);        // 3A 100Hz   29 150Hz   39 200Hz    31 171Hz       3C 50Hz (default)    0B 10Hz
//	//_writeData(0x0B);   //0B is 10Hz
//    /* EPD hardware init end */
//
//
//  
//  //_writeLUT_Normal();	//PLL 06-01-2019
//  _writeLUT();	//PLL 06-01-2019
//  
//}

void GxGDEW042Z15::_sleep(void)
{
  IO.writeCommandTransaction(0x50); // border floating
  IO.writeDataTransaction(0x17);
  IO.writeCommandTransaction(0x02); // power off
  _waitWhileBusy("Power Off");
  if (_rst >= 0)
  {
    IO.writeCommandTransaction(0x07); // deep sleep
    IO.writeDataTransaction(0xA5);
  }
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10); // black
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  IO.writeCommandTransaction(0x13); // red
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10); // black
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  IO.writeCommandTransaction(0x13); // red
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10); // black
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  IO.writeCommandTransaction(0x13); // red
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10); // black
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  IO.writeCommandTransaction(0x13); // red
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEW042Z15_WIDTH - x - w - 1;
      break;
    case 2:
      x = GxGDEW042Z15_WIDTH - x - w - 1;
      y = GxGDEW042Z15_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEW042Z15_HEIGHT - y - h - 1;
      break;
  }
}

void GxGDEW042Z15::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042Z15_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042Z15_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEW042Z15_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateToWindow");
  IO.writeCommandTransaction(0x92); // partial out
  _current_page = -1;
}

void GxGDEW042Z15::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042Z15_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042Z15_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEW042Z15_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateToWindow");
  IO.writeCommandTransaction(0x92); // partial out
  _current_page = -1;
}

void GxGDEW042Z15::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042Z15_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042Z15_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEW042Z15_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateToWindow");
  IO.writeCommandTransaction(0x92); // partial out
  _current_page = -1;
}

void GxGDEW042Z15::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042Z15_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042Z15_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEW042Z15_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateToWindow");
  IO.writeCommandTransaction(0x92); // partial out
  _current_page = -1;
}

void GxGDEW042Z15::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10); // black
  for (uint32_t y = 0; y < GxGDEW042Z15_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW042Z15_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW042Z15_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW042Z15_WIDTH / 8 - 4) && (y > GxGDEW042Z15_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW042Z15_HEIGHT - 33)) data = 0x00;
      IO.writeDataTransaction(data);
    }
  }
  IO.writeCommandTransaction(0x13); // red
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    IO.writeDataTransaction(0xFF); // 0xFF is white
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}

void GxGDEW042Z15::_writeLUT_Normal(void)
{
  //Serial.println("_writeLUT_Normal()");
  _writeCommand(0x00);
  _writeData(0x0f); // LUT from OTP Pixel with B/W/R.
/*
  unsigned int count;
  {
    _writeCommand(0x20);							//vcom
    for (count = 0; count < 44; count++)
    {
      _writeData(lut_20_vcomDC[count]);
    }

    _writeCommand(0x21);							//ww --
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_21[count]);
    }

    _writeCommand(0x22);							//bw r
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_22_red[count]);
    }

    _writeCommand(0x23);							//wb w
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_23_white[count]);
    }

    _writeCommand(0x24);							//bb b
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_24_black[count]);
    }
  }
  */
}

void GxGDEW042Z15::_writeRegisters(void)
{
//* default values for registers, starts with a5
//*PSR, PFS, BTST(3), TSE, CDI, TCON, TRES(2x2), GSST(2x2), CCSET, PWS, TSSET (19 bytes + 1 byte (enable=a5))
//0020  a5 0f 00 d7 d7 1e 00 77  22 01 90 01 2c 00 00 00 
//0030  00 00 00 00 ff ff ff ff  ff ff ff ff ff ff ff ff 
    _writeCommand(PANEL_SETTING);
	_writeData(0x2f); //300x400 Red mode, LUT set by register (0x0f for otp luts)
	
    _writeCommand(POWER_OFF_SEQUENCE_SETTING);
	_writeData(0x00);
	
	_writeCommand(BOOSTER_SOFT_START);
	_writeData(0xd7);
	_writeData(0xd7);
	_writeData(0x1e);
	
	_writeCommand(TEMPERATURE_SENSOR_SELECTION);
	_writeData(0x00);
	
	_writeCommand(VCOM_AND_DATA_INTERVAL_SETTING);
	_writeData(0x77);
	
	_writeCommand(TCON_SETTING);
	_writeData(0x22);

	_writeCommand(RESOLUTION_SETTING);
	_writeData(0x01);
	_writeData(0x90);
	_writeData(0x01);
	_writeData(0x2c);
	
	_writeCommand(GSST_SETTING);
	_writeData(0x00);
	_writeData(0x00);
	_writeData(0x00);
	_writeData(0x00);
	
	_writeCommand(CASCADE_SETTING);
	_writeData(0x00);
	
	_writeCommand(POWER_SAVING);
	_writeData(0x00);
	
	_writeCommand(FORCE_TEMPERATURE);
	_writeData(0x00);

}

void GxGDEW042Z15::_writeLUT_Composite(void)
{
  //Serial.println("_writeLUT_Composite()");
    _writeCommand(PLL_CONTROL);
    _writeData(0x29);        // 3A 100Hz   29 150Hz   39 200Hz    31 171Hz       3C 50Hz (default)    0B 10Hz

  //_writeRegisters();
    _writeCommand(PANEL_SETTING);
	// _writeData(0xbf);    // KW-BF   KWR-AF  BWROTP 0f
	//  _writeData(0x0b);
	//	_writeData(0x0F);  //300x400 Red mode, LUT from OTP
	//	_writeData(0x1F);  //300x400 B/W mode, LUT from OTP
	//	_writeData(0x3F); //300x400 B/W mode, LUT set by register
	_writeData(0x2F); //300x400 Red mode, LUT set by register

	unsigned int count;
  {
    _writeCommand(0x20);							//vcom
    for (count = 0; count < 42; count++)	// was < 44
    {
      _writeData(lut_vcom_dc_comp[count]);
    }
	_writeData(0x00);	// for safety, write these two control bytes individually, using flash-based code.
	_writeData(0x00);	// writing a value other than 0 to these bytes will destroy the display.

    _writeCommand(0x21);							//ww --
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_ww_comp[count]);
    }

    _writeCommand(0x22);							//bw r
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_bw_comp[count]);
    }

    _writeCommand(0x23);							//wb w
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_wb_comp[count]);
    }

    _writeCommand(0x24);							//bb b
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_bb_comp[count]);
    }
  }
}

void GxGDEW042Z15::_writeLUT(void)
{
	switch(_lut_mode) {
		case LUT_MODE_3BPP:
			//Serial.printf("_writeLUT() 3BPP: _refreshnumber=%d\n", _refreshnumber);
			switch(_refreshnumber) 
			{
				case 7:
					//BLACK
					_writeLUT_Normal();
					break;
				case 6:
					//BLACK
					
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_6;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_6;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_6;
					
					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_6;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_6;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_6;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_6;

					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_6;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_6;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_6;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_6;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_6;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_6;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_6;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_6;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_6;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_6;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_6;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_6;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_6;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_6;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_6;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_6;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_6;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_6;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_6;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_6;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_6;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_6;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_6;
					
					_writeLUT_Composite();		
					break;
				
				case 5:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_5;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_5;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_5;

					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_5;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_5;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_5;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_5;

					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_5;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_5;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_5;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_5;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_5;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_5;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_5;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_5;
					
					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_5;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_5;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_5;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_5;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_5;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_5;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_5;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_5;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_5;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_5;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_5;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_5;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_5;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_5;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_5;
					
					_writeLUT_Composite();		
					break;
				
				case 4:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_4;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_4;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_4;

					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_4;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_4;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_4;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_4;

					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_4;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_4;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_4;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_4;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_4;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_4;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_4;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_4;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_4;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_4;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_4;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_4;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_4;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_4;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_4;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_4;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_4;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_4;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_4;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_4;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_4;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_4;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_4;
					
					_writeLUT_Composite();		
					break;
				
				case 3:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_3;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_3;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_3;

					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_3;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_3;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_3;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_3;
																 
					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_3;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_3;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_3;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_3;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_3;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_3;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_3;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_3;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_3;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_3;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_3;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_3;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_3;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_3;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_3;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_3;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_3;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_3;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_3;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_3;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_3;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_3;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_3;
					
					_writeLUT_Composite();		
					break;
				
				case 2:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_2;

					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_2;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_2;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_2;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_2;
																 
					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_2;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_2;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_2;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_2;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_2;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_2;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_2;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_2;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_2;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_2;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_2;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_2;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_2;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_2;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_2;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_2;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_2;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_2;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_2;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_2;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_2;
					
					_writeLUT_Composite();		
					break;
				
				case 1:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_1;

					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_1;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_1;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_1;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_1;
																 
					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_1;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_1;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_1;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_1;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_1;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_1;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_1;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_1;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_1;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_1;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_1;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_1;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_1;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_1;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_1;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_1;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_1;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_1;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_1;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_1;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_1;
					
					_writeLUT_Composite();		
					break;
				
				default:
					_writeLUT_Normal();		
					break;
			}
			break;

		case LUT_MODE_2BPP:
			//Serial.printf("_writeLUT() 2BPP: _refreshnumber=%d\n", _refreshnumber);
			switch(_refreshnumber) 
			{
				case 3:
					_writeLUT_Normal();
					break;
					
				case 2:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_2;
					
					lut_ww_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_2;
					lut_bw_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_2;
					lut_wb_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_2;
					lut_bb_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_2;

					lut_ww_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_2;
					lut_bw_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_2;
					lut_wb_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_2;
					lut_bb_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_2;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_2;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_2;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_2;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_2;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = _2BPP_LUT_RED_C1_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = _2BPP_LUT_RED_C2_2;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_2;

					lut_ww_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_2;
					lut_bw_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_2;
					lut_wb_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_2;
					lut_bb_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_2;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_2;
					lut_bw_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_2;
					lut_wb_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_2;
					lut_bb_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_2;				

					lut_ww_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_2;
					lut_bw_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_2;
					lut_wb_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_2;
					lut_bb_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_2;
					
					_writeLUT_Composite();		
					break;
					
				case 1:
					
					//BLACK
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_1;
					
					lut_ww_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_1;
					lut_bw_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_1;
					lut_wb_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_1;
					lut_bb_comp[LUT_BLACK_COUNT1] = _2BPP_LUT_BLACK_C1_1;

					lut_ww_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_1;
					lut_bw_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_1;
					lut_wb_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_1;
					lut_bb_comp[LUT_BLACK_COUNT2] = _2BPP_LUT_BLACK_C2_1;

					lut_ww_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_1;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_1;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_1;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = _2BPP_LUT_BLACK_REPS_1;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = _2BPP_LUT_RED_C1_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = _2BPP_LUT_RED_C2_1;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_1;

					lut_ww_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_1;
					lut_bw_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_1;
					lut_wb_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_1;
					lut_bb_comp[LUT_RED_COUNT1] = _2BPP_LUT_RED_C1_1;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_1;
					lut_bw_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_1;
					lut_wb_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_1;
					lut_bb_comp[LUT_RED_COUNT2] = _2BPP_LUT_RED_C2_1;				

					lut_ww_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_1;
					lut_bw_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_1;
					lut_wb_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_1;
					lut_bb_comp[LUT_RED_REPCOUNT] = _2BPP_LUT_RED_REPS_1;
					
					_writeLUT_Composite();		
					break;
					
				default:
					_writeLUT_Normal();		
					break;
			}
			break;

		case LUT_MODE_1BPP:
			//Serial.printf("_writeLUT() 1BPP: _refreshnumber=%d\n", _refreshnumber);
			_writeLUT_Normal();
			break;
			
		default:
			//Serial.printf("_writeLUT() unknown BPP: _refreshnumber=%d\n", _refreshnumber);
			_writeLUT_Normal();
			break;
	}
}

void GxGDEW042Z15::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEW042Z15::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

//void GxGDEW042Z15::drawPixel(int16_t x, int16_t y, uint16_t color, int saturation)
//{
//  drawPixel(int16_t x, int16_t y, uint16_t color, int saturation, true);
//}

void GxGDEW042Z15::drawPixel(int16_t x, int16_t y, uint16_t color, int saturation, bool bCorrectRed)
{		
	uint8_t threshold = 0;

	const uint8_t weights_red_3bpp[8]             = {0, 7, 6, 4, 2, 1, 1, 1}; // add a bias to the left, to intensify lighter intensities (which are weak in its value scale)
	const uint8_t weights_red_uncorrected_3bpp[8] = {0, 7, 6, 5, 4, 3, 2, 1};
	const uint8_t weights_black_3bpp[8]           = {0, 7, 6, 5, 4, 3, 2, 1};
	//const uint8_t weights_black_3bpp[8]   = {0, 7, 6, 4, 2, 1, 1, 1}; // add a bias to the left, to intensify lighter intensities (which are weak in its value scale)
	//const uint8_t weights_black_3bpp[8] = {0, 7, 6, 5, 4, 2, 1, 1};

	const uint8_t weights_red_2bpp[4]   = {0, 3, 2, 1}; 
	const uint8_t weights_black_2bpp[4] = {0, 3, 2, 1};

	switch(_lut_mode) 
	{
		case LUT_MODE_3BPP:

			if (saturation > 15) saturation = 15;
			if (saturation < 0) saturation = 0;
			
			switch(color)
			{
				case GxEPD_BLACK:
					threshold = weights_black_3bpp[_refreshnumber];
					break;
				
				case GxEPD_RED:
					if (bCorrectRed) {
            threshold = weights_red_3bpp[_refreshnumber];
          }
          else {
            threshold = weights_red_uncorrected_3bpp[_refreshnumber];   // PLL-06-07-2021
          }
					break;
				
				case GxEPD_WHITE:
					threshold = weights_black_3bpp[_epd_refresh_max - _refreshnumber + 1];
					break;
					
				default:
					threshold = 0;
					break;
			}
			
			if ((saturation >> 1) >= threshold) {
				drawPixel(x, y, color);
			}

			break;
			
		case LUT_MODE_2BPP:
			if (saturation > 3) saturation = 3;
			if (saturation < 0) saturation = 0;
			
			switch(color)
			{
				case GxEPD_BLACK:
					threshold = weights_black_2bpp[_refreshnumber];
					break;
				
				case GxEPD_RED:
					threshold = weights_red_2bpp[_refreshnumber];
					break;
				
				case GxEPD_WHITE:
					threshold = weights_black_2bpp[_epd_refresh_max - _refreshnumber + 1];
					break;
					
				default:
					threshold = 0;
					break;
			}
			
			if (saturation >= threshold) {
				drawPixel(x, y, color);
			}

			break;
			
		case LUT_MODE_1BPP:
			drawPixel(x, y, color);
			break;

		default:
			drawPixel(x, y, color);
			break;			
	}
}

// refresh number - used for grey level compositing. Varies between 1 and 3 (2bpp) 3 grey/red levels plus white
void GxGDEW042Z15::resetRefreshNumber(int bpp = LUT_MODE_1BPP) 
{
	switch(bpp) 
	{
		case LUT_MODE_1BPP:
			_refreshnumber = EPD_REFRESH_NUMBER_1BPP;
			break;

		case LUT_MODE_2BPP:
			_refreshnumber = EPD_REFRESH_NUMBER_2BPP;
			break;

		case LUT_MODE_3BPP:
			_refreshnumber = EPD_REFRESH_NUMBER_3BPP;
			break;
		
		default:
			_refreshnumber = EPD_REFRESH_NUMBER_1BPP;
			break;
	}
	
	_epd_refresh_max = _refreshnumber;
	_lut_mode = bpp;
}

int GxGDEW042Z15::getRefreshNumber() 
{
	return _refreshnumber;
}

bool GxGDEW042Z15::decRefreshNumber() 
{	
	if (_refreshnumber > 0) {
		_refreshnumber--;
	}
	
	return (_refreshnumber > 0);
}

int GxGDEW042Z15::getMaxRefreshNumber()
{
	return _epd_refresh_max;
}

/*
//command 0x01 (power setting) [01] 3a 00 2b 2b 11

//3a 00 2b 2b 11 1e 				what is 0x1e?


//LUT_FOR_VCOM (20h)
//R20H
uint8_t GxGDEW042Z15::lut_vcom_dc_comp[] =
{
	0x00 , 0x37 , 0x37 , 0x37 , 0x08 , 0x01 , 
	0x00 , 0x0c , 0x0c , 0x01 , 0x00 , 0x07 , 
	0x84 , 0x11 , 0x01 , 0x11 , 0x01 , 0x08 ,  
	0x00 , 0x0c , 0x01 , 0x0c , 0x02 , 0x07 , 
	0x00 , 0x04 , 0x02 , 0x18 , 0x0c , 0x06 , 
	0x00 , 0x03 , 0x02 , 0x14 , 0x0c , 0x08 , 
	0x00 , 0x0b , 0x0b , 0x01 , 0x08 , 0x01 , 
	0x00 , 0x00 
};

//LUT_WHITE_TO_WHITE (21h)
//R21H
uint8_t GxGDEW042Z15::lut_ww_comp[] =
{
	0x90 , 0x37 , 0x37 , 0x37 , 0x08 , 0x01 , 
	0x40 , 0x0c , 0x0c , 0x01 , 0x00 , 0x07 ,
	0x48 , 0x11 , 0x01 , 0x11 , 0x01 , 0x08 ,
	0x80 , 0x0c , 0x01 , 0x0c , 0x02 , 0x07 ,
	0x00 , 0x04 , 0x02 , 0x18 , 0x0c , 0x06 , 
	0x00 , 0x03 , 0x02 , 0x14 , 0x0c , 0x08 ,
	0x00 , 0x0b , 0x0b , 0x01 , 0x08 , 0x01
};

//LUT_BLACK_TO_WHITE (22h)
//R22H  r
uint8_t GxGDEW042Z15::lut_bw_comp[] =
{
	0x88 , 0x37 , 0x37 , 0x37 , 0x08 , 0x01 ,
	0x80 , 0x0c , 0x0c , 0x01 , 0x00 , 0x07 , 
	0x48 , 0x11 , 0x01 , 0x11 , 0x01 , 0x08 ,
	0x06 , 0x0c , 0x01 , 0x0c , 0x02 , 0x07 ,
	0x8c , 0x04 , 0x02 , 0x18 , 0x0c , 0x06 ,
	0x8c , 0x03 , 0x02 , 0x14 , 0x0c , 0x08 ,
	0xf0 , 0x0b , 0x0b , 0x01 , 0x08 , 0x01
};

//LUT_WHITE_TO_BLACK (23h)
//R23H  w
uint8_t GxGDEW042Z15::lut_wb_comp[] =
{
	0x90 , 0x37 , 0x37 , 0x37 , 0x08 , 0x01 , 
	0x40 , 0x0c , 0x0c , 0x01 , 0x00 , 0x07 , 
	0x48 , 0x11 , 0x01 , 0x11 , 0x01 , 0x08 ,  
	0x80 , 0x0c , 0x01 , 0x0c , 0x02 , 0x07 , 
	0x00 , 0x04 , 0x02 , 0x18 , 0x0c , 0x06 , 
	0x00 , 0x03 , 0x02 , 0x14 , 0x0c , 0x08 , 
	0x00 , 0x0b , 0x0b , 0x01 , 0x08 , 0x01 
};

//LUT_BLACK_TO_BLACK (24h)
uint8_t GxGDEW042Z15::lut_bb_comp[] =
{
	0x92 , 0x37 , 0x37 , 0x37 , 0x08 , 0x01 ,
	0x20 , 0x0c , 0x0c , 0x01 , 0x00 , 0x07 ,
	0x48 , 0x11 , 0x01 , 0x11 , 0x01 , 0x08 ,
	0x04 , 0x0c , 0x01 , 0x0c , 0x02 , 0x07 , 
	0x00 , 0x04 , 0x02 , 0x18 , 0x0c , 0x06 ,
	0x00 , 0x03 , 0x02 , 0x14 , 0x0c , 0x08 ,
	0x01 , 0x0b , 0x0b , 0x01 , 0x08 , 0x01 
};
*/