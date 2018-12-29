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

#include "GxGDEW027C44.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

const uint8_t GxGDEW027C44::lut_20_vcomDC[] =
{
  0x00  , 0x00,
  0x00  , 0x1A  , 0x1A  , 0x00  , 0x00  , 0x01,
  0x00  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x00  , 0x0E  , 0x01  , 0x0E  , 0x01  , 0x10,
  0x00  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x00  , 0x04  , 0x10  , 0x00  , 0x00  , 0x05,
  0x00  , 0x03  , 0x0E  , 0x00  , 0x00  , 0x0A,
  0x00  , 0x23  , 0x00  , 0x00  , 0x00  , 0x01
};
//R21H
const uint8_t GxGDEW027C44::lut_21[] = {
  0x90  , 0x1A  , 0x1A  , 0x00  , 0x00  , 0x01,
  0x40  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x84  , 0x0E  , 0x01  , 0x0E  , 0x01  , 0x10,
  0x80  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x00  , 0x04  , 0x10  , 0x00  , 0x00  , 0x05,
  0x00  , 0x03  , 0x0E  , 0x00  , 0x00  , 0x0A,
  0x00  , 0x23  , 0x00  , 0x00  , 0x00  , 0x01
};
//R22H  r
const uint8_t GxGDEW027C44::lut_22_red[] = {
  0xA0  , 0x1A  , 0x1A  , 0x00  , 0x00  , 0x01,
  0x00  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x84  , 0x0E  , 0x01  , 0x0E  , 0x01  , 0x10,
  0x90  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0xB0  , 0x04  , 0x10  , 0x00  , 0x00  , 0x05,
  0xB0  , 0x03  , 0x0E  , 0x00  , 0x00  , 0x0A,
  0xC0  , 0x23  , 0x00  , 0x00  , 0x00  , 0x01
};
//R23H  w
const uint8_t GxGDEW027C44::lut_23_white[] = {
  0x90  , 0x1A  , 0x1A  , 0x00  , 0x00  , 0x01,
  0x40  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x84  , 0x0E  , 0x01  , 0x0E  , 0x01  , 0x10,
  0x80  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x00  , 0x04  , 0x10  , 0x00  , 0x00  , 0x05,
  0x00  , 0x03  , 0x0E  , 0x00  , 0x00  , 0x0A,
  0x00  , 0x23  , 0x00  , 0x00  , 0x00  , 0x01
};
//R24H  b
const uint8_t GxGDEW027C44::lut_24_black[] = {
  0x90  , 0x1A  , 0x1A  , 0x00  , 0x00  , 0x01,
  0x20  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x84  , 0x0E  , 0x01  , 0x0E  , 0x01  , 0x10,
  0x10  , 0x0A  , 0x0A  , 0x00  , 0x00  , 0x08,
  0x00  , 0x04  , 0x10  , 0x00  , 0x00  , 0x05,
  0x00  , 0x03  , 0x0E  , 0x00  , 0x00  , 0x0A,
  0x00  , 0x23  , 0x00  , 0x00  , 0x00  , 0x01
};


//////////////////////////////////
/// composite (no invert/clear) 7 shades + white (3bpp)
//RED
#define LUT_VCOM_DC_RED_COUNT1 		33
#define LUT_VCOM_DC_RED_COUNT2 		34
#define LUT_VCOM_DC_RED_REPCOUNT 	37

#define LUT_RED_COUNT1 				31
#define LUT_RED_COUNT2 				32
#define LUT_RED_REPCOUNT 			35

#define LUT_RED_REPS_7 				0x0F	
#define LUT_RED_C1_7 				0x03
#define LUT_RED_C2_7 				0x0E
		
#define LUT_RED_REPS_6 				0x01	
#define LUT_RED_C1_6 				0x01
#define LUT_RED_C2_6 				0x0C
		
#define LUT_RED_REPS_5 				0x01	
#define LUT_RED_C1_5 				0x01
#define LUT_RED_C2_5 				0x0C
		
#define LUT_RED_REPS_4 				0x01	
#define LUT_RED_C1_4 				0x01
#define LUT_RED_C2_4 				0x0C
		
#define LUT_RED_REPS_3 				0x01	
#define LUT_RED_C1_3 				0x01
#define LUT_RED_C2_3 				0x0C
		
#define LUT_RED_REPS_2 				0x01	
#define LUT_RED_C1_2 				0x01
#define LUT_RED_C2_2 				0x0C
		
#define LUT_RED_REPS_1 				0x01	
#define LUT_RED_C1_1 				0x01
#define LUT_RED_C2_1 				0x0C

//BLACK
#define LUT_VCOM_DC_BLACK_COUNT1 	9
#define LUT_VCOM_DC_BLACK_COUNT2 	10
#define LUT_VCOM_DC_BLACK_REPCOUNT 	13

#define LUT_BLACK_COUNT1 			7
#define LUT_BLACK_COUNT2 			8
#define LUT_BLACK_REPCOUNT 			11

#define LUT_BLACK_REPS_7 			0x08	//8*10*2 = 160
#define LUT_BLACK_C1_7 				0x0A
#define LUT_BLACK_C2_7 				0x0A
		
#define LUT_BLACK_REPS_6 			0x03	//8*3*2 = 48
#define LUT_BLACK_C1_6 				0x03
#define LUT_BLACK_C2_6 				0x03
		
#define LUT_BLACK_REPS_5 			0x03	//4*3*2 = 24
#define LUT_BLACK_C1_5 				0x02
#define LUT_BLACK_C2_5				0x02
		
#define LUT_BLACK_REPS_4 			0x03	//2*3*2 = 12
#define LUT_BLACK_C1_4 				0x01
#define LUT_BLACK_C2_4 				0x01
		
#define LUT_BLACK_REPS_3 			0x03	//1*3*2 = 6
#define LUT_BLACK_C1_3 				0x01
#define LUT_BLACK_C2_3 				0x01
		
#define LUT_BLACK_REPS_2 			0x03	//1*2*2 = 4
#define LUT_BLACK_C1_2 				0x01
#define LUT_BLACK_C2_2 				0x01
		
#define LUT_BLACK_REPS_1 			0x03	//1*1*2 = 2
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
		
//#define _2BPP_LUT_BLACK_REPS_4 		0x03	//2*3*2 = 12
//#define _2BPP_LUT_BLACK_C1_4 		0x01
//#define _2BPP_LUT_BLACK_C2_4 		0x01
//		
//#define _2BPP_LUT_BLACK_REPS_3 		0x03	//1*3*2 = 6
//#define _2BPP_LUT_BLACK_C1_3 		0x01
//#define _2BPP_LUT_BLACK_C2_3 		0x01
//		
//#define _2BPP_LUT_BLACK_REPS_2 		0x03	//1*2*2 = 4
//#define _2BPP_LUT_BLACK_C1_2 		0x01
//#define _2BPP_LUT_BLACK_C2_2 		0x01
//		
//#define _2BPP_LUT_BLACK_REPS_1 		0x03	//1*1*2 = 2
//#define _2BPP_LUT_BLACK_C1_1 		0x01
//#define _2BPP_LUT_BLACK_C2_1 		0x01

									
									
uint8_t GxGDEW027C44::lut_vcom_dc_comp[] =
{
0x00, 0x00,
0x00, 0x1A, 0x1A, 0x00, 0x00, 0x00,        //0x00 0x1A 0x1A 0x00 0x00 0x00
0x00, 0x02, 0x02, 0x00, 0x00, 0x01,        
0x00, 0x0E, 0x01, 0x0E, 0x01, 0x00,        //0x0E 0x01 0x0E 0x01
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x00,        
0x00, 0x04, 0x10, 0x00, 0x00, 0x00,        
0x00, 0x03, 0x0E, 0x00, 0x00, 0x02,        
0x00, 0x01, 0x00, 0x00, 0x00, 0x01
};

//R21H
uint8_t GxGDEW027C44::lut_ww_comp[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x00,
0x40, 0x02, 0x02, 0x00, 0x00, 0x01,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x00,
0x80, 0x0A, 0x0A, 0x00, 0x00, 0x00,
0x00, 0x04, 0x10, 0x00, 0x00, 0x00,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x01, 0x00, 0x00, 0x00, 0x01
};

//R22H    r
uint8_t GxGDEW027C44::lut_bw_comp[] =
{
0xA0, 0x1A, 0x1A, 0x00, 0x00, 0x00,
0x00, 0x02, 0x02, 0x00, 0x00, 0x01,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x00,
0x90, 0x0A, 0x0A, 0x00, 0x00, 0x00,
0xB0, 0x04, 0x10, 0x00, 0x00, 0x00,
0xB0, 0x03, 0x0E, 0x00, 0x00, 0x02,
0xC0, 0x01, 0x00, 0x00, 0x00, 0x01
};

//R23H    w
uint8_t GxGDEW027C44::lut_bb_comp[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x00,
0x40, 0x02, 0x02, 0x00, 0x00, 0x01,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x00,
0x80, 0x0A, 0x0A, 0x00, 0x00, 0x00,
0x00, 0x04, 0x10, 0x00, 0x00, 0x00,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x01, 0x00, 0x00, 0x00, 0x01
};

//R24H    b
uint8_t GxGDEW027C44::lut_wb_comp[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x00,	// applies the image in inverse (0x01). At this point, red is shown as black. As an alternative to this, the screen is cleared separately, before any drawing takes place
0x20, 0x02, 0x02, 0x00, 0x00, 0x01,	// 0x04 shows the image in non-inverse (0x08). At this point, red and black are only faintly visible (as whiter-than-white) 0x20, 0x0A, 0x0A, 0x00, 0x00, 0x01
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x00,
0x10, 0x0A, 0x0A, 0x00, 0x00, 0x00,
0x00, 0x04, 0x10, 0x00, 0x00, 0x00,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x02,	// 0x02 does the red (0x0A). With line 2 set to 0x08 (and all the rest 0x00), get pink from red areas
0x00, 0x01, 0x00, 0x00, 0x00, 0x01	// turns reds into browns (adds black) 0x00 0x23 0x00 0x00 0x00 0x01 // 0x00 0x01 0x00 0x00 0x00 0x01
};

GxGDEW027C44::GxGDEW027C44(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT), IO(io),
    _current_page(-1), _using_partial_mode(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEW027C44::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW027C44_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW027C44_WIDTH - x - 1;
      y = GxGDEW027C44_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW027C44_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW027C44_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW027C44_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW027C44_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW027C44_WIDTH / 8;
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


void GxGDEW027C44::drawPixel(int16_t x, int16_t y, uint16_t color, int saturation)
{		
	uint8_t threshold = 0;

	const uint8_t weights_red_3bpp[8]   = {0, 7, 6, 4, 2, 1, 1, 1}; // add a bias to the left, to intensify lighter intensities (which are weak in its value scale)
	const uint8_t weights_black_3bpp[8] = {0, 7, 6, 5, 4, 3, 2, 1};

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
					threshold = weights_red_3bpp[_refreshnumber];
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
void GxGDEW027C44::resetRefreshNumber(int bpp = LUT_MODE_1BPP) 
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

int GxGDEW027C44::getRefreshNumber() 
{
	return _refreshnumber;
}

bool GxGDEW027C44::decRefreshNumber() 
{	
	if (_refreshnumber > 0) {
		_refreshnumber--;
	}
	
	return (_refreshnumber > 0);
}

int GxGDEW027C44::getMaxRefreshNumber()
{
	return _epd_refresh_max;
}

void GxGDEW027C44::init(uint32_t serial_diag_bitrate)
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

/*void GxGDEW027C44::setMode(int mode)
{
	_lut_mode = mode;
	resetRefreshNumber();
}
*/

void GxGDEW027C44::fillScreen(uint16_t color)
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

void GxGDEW027C44::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_black_buffer)) ? _black_buffer[i] : 0x00);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_red_buffer)) ? _red_buffer[i] : 0x00);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW027C44::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_normal; // no change
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW027C44::drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size)
{
  drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_normal);
}

void GxGDEW027C44::drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true;
    _wakeUp();
    _setPartialRamArea(0x14, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      uint8_t data = 0x00; // white is 0x00 on device
      if (i < black_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&black_bitmap[i]);
#else
        data = black_bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data);
    }
    _setPartialRamArea(0x15, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      uint8_t data = 0x00; // white is 0x00 on device
      if (i < red_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&red_bitmap[i]);
#else
        data = red_bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data);
    }
    _refreshWindow(0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    _waitWhileBusy("drawPicture");
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      uint8_t data = 0x00; // white is 0x00 on device
      if (i < black_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&black_bitmap[i]);
#else
        data = black_bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data);
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      uint8_t data = 0x00; // white is 0x00 on device
      if (i < red_size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&red_bitmap[i]);
#else
        data = red_bitmap[i];
#endif
        if (mode & bm_invert_red) data = ~data;
      }
      _writeData(data);
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("drawPicture");
    _sleep();
  }
}

void GxGDEW027C44::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_normal; // no change
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true;
    _wakeUp();
    _setPartialRamArea(0x14, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      uint8_t data = 0x00; // white is 0x00 on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data);
    }
    _setPartialRamArea(0x15, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      _writeData(0);
    }
    _refreshWindow(0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    _waitWhileBusy("drawBitmap");
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      uint8_t data = 0x00; // white is 0x00 on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data);
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      _writeData(0);
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    _sleep();
  }
}

void GxGDEW027C44::eraseDisplay(bool using_partial_update)
{  
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true; // remember
    _setPartialRamArea(0x14, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      _writeData(0x00);
    }
    _setPartialRamArea(0x15, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      _writeData(0x00);
    }
    _refreshWindow(0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT);
    _waitWhileBusy("drawBitmap");
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      _writeData(0x00);
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
    {
      _writeData(0x00);
    }
    _writeCommand(0x12);      //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEW027C44::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEW027C44_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEW027C44_WIDTH - x - w - 1;
        y = GxGDEW027C44_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEW027C44_HEIGHT - y  - h - 1;
        break;
    }
  }
  //fillScreen(0x0);
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _writeToWindow(x, y, x, y, w, h);
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateWindow");
}

void GxGDEW027C44::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEW027C44_WIDTH - xs - w - 1;
        xd = GxGDEW027C44_WIDTH - xd - w - 1;
        break;
      case 2:
        xs = GxGDEW027C44_WIDTH - xs - w - 1;
        ys = GxGDEW027C44_HEIGHT - ys - h - 1;
        xd = GxGDEW027C44_WIDTH - xd - w - 1;
        yd = GxGDEW027C44_HEIGHT - yd - h - 1;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEW027C44_HEIGHT - ys  - h - 1;
        yd = GxGDEW027C44_HEIGHT - yd  - h - 1;
        break;
    }
  }
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _writeToWindow(xs, ys, xd, yd, w, h);
  _refreshWindow(xd, yd, w, h);
  _waitWhileBusy("updateToWindow");
  delay(500); // don't stress this display
}

void GxGDEW027C44::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
  // the screen limits are the hard limits
  if (xs >= GxGDEW027C44_WIDTH) return;
  if (ys >= GxGDEW027C44_HEIGHT) return;
  if (xd >= GxGDEW027C44_WIDTH) return;
  if (yd >= GxGDEW027C44_HEIGHT) return;
  w = gx_uint16_min(w + 7, GxGDEW027C44_WIDTH - xd) + (xd % 8);
  h = gx_uint16_min(h, GxGDEW027C44_HEIGHT - yd);
  _setPartialRamArea(0x14, xd, yd, w, h);
  int16_t xe = (xs / 8) + (w / 8);
  for (int16_t y1 = ys; y1 < ys + h; y1++)
  {
    for (int16_t x1 = xs / 8; x1 < xe; x1++)
    {
      uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
      IO.writeDataTransaction(data);
    }
  }
  delay(2);
  _setPartialRamArea(0x15, xd, yd, w, h);
  for (int16_t y1 = ys; y1 < ys + h; y1++)
  {
    for (int16_t x1 = xs / 8; x1 < xe; x1++)
    {
      uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
      IO.writeDataTransaction(data);
    }
  }
  delay(2);
}

void GxGDEW027C44::powerDown()
{
  _using_partial_mode = false; // force _wakeUp()
  _sleep();
}

void GxGDEW027C44::_setPartialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  IO.writeCommandTransaction(command);
  IO.writeDataTransaction(x >> 8);
  IO.writeDataTransaction(x & 0xf8);
  IO.writeDataTransaction(y >> 8);
  IO.writeDataTransaction(y & 0xff);
  IO.writeDataTransaction(w >> 8);
  IO.writeDataTransaction(w & 0xf8);
  IO.writeDataTransaction(h >> 8);
  IO.writeDataTransaction(h & 0xff);
}

void GxGDEW027C44::_refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  w += (x % 8) + 7;
  h = gx_uint16_min(h, 256); // strange controller error
  IO.writeCommandTransaction(0x16);
  IO.writeDataTransaction(x >> 8);
  IO.writeDataTransaction(x & 0xf8);
  IO.writeDataTransaction(y >> 8);
  IO.writeDataTransaction(y & 0xff);
  IO.writeDataTransaction(w >> 8);
  IO.writeDataTransaction(w & 0xf8);
  IO.writeDataTransaction(h >> 8);
  IO.writeDataTransaction(h & 0xff);
}

void GxGDEW027C44::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEW027C44::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEW027C44::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (digitalRead(_busy) == 1) break;
    delay(1);
    if (micros() - start > 20000000) // > 15.5s !
    {
      if (_diag_enabled) Serial.println("Busy Timeout!");
      break;
    }
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

void GxGDEW027C44::_wakeUp()
{
  // reset required for wakeup
  if (_rst >= 0)
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }

  _writeCommand(0x01);
  _writeData (0x03);
  _writeData (0x00);
  _writeData (0x2b);
  _writeData (0x2b);
  _writeData (0x09);

  _writeCommand(0x06);
  _writeData (0x07);
  _writeData (0x07);
  _writeData (0x17);

  _writeCommand(0xF8);
  _writeData (0x60);
  _writeData (0xA5);

  _writeCommand(0xF8);
  _writeData (0x89);
  _writeData (0xA5);

  _writeCommand(0xF8);
  _writeData (0x90);
  _writeData (0x00);

  _writeCommand(0xF8);
  _writeData (0x93);
  _writeData (0x2A);

  _writeCommand(0xF8);
  _writeData (0x73);
  _writeData (0x41);

  _writeCommand(0x16);
  _writeData(0x00);

  _writeCommand(0x04);
  _waitWhileBusy("_wakeUp Power On");

  _writeCommand(0x00);
  _writeData(0xaf); // by register LUT

  _writeCommand(0x30);      //PLL�趨 // define by OTP
  _writeData (0x3a);       //3A 100HZ   29 150Hz 39 200HZ 31 171HZ

  _writeCommand(0x61);      //�����趨 // define by OTP
  _writeData (0x00);
  _writeData (0xb0);       //176
  _writeData (0x01);
  _writeData (0x08);    //264

  _writeCommand(0x82);      //vcom�趨 // define by OTP
  _writeData (0x12);

  _writeCommand(0X50);      // define by OTP
  _writeData(0x87);   // define by OTP
  
  _writeLUT();              //д��lut
}

void GxGDEW027C44::_sleep(void)
{
  _writeCommand(0x02); // power off
  _waitWhileBusy("_sleep Power Off");
  if (_rst >= 0)
  {
    _writeCommand(0x07); // deep sleep
    _writeData (0xa5);
  }
}

void GxGDEW027C44::_writeLUT_Normal(void)
{
  Serial.println("_writeLUT_Normal()");
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
}

void GxGDEW027C44::_writeLUT_Composite(void)
{
  Serial.println("_writeLUT_Composite()");
  unsigned int count;
  {
    _writeCommand(0x20);							//vcom
    for (count = 0; count < 44; count++)
    {
      _writeData(lut_vcom_dc_comp[count]);
    }

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

void GxGDEW027C44::_writeLUT(void)
{
	switch(_lut_mode) {
		case LUT_MODE_3BPP:
			Serial.printf("_writeLUT() 3BPP: _refreshnumber=%d\n", _refreshnumber);
			switch(_refreshnumber) 
			{
				case 7:
					//BLACK
					_writeLUT_Normal();
					break;
					/*
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT1] = LUT_BLACK_C1_7;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_COUNT2] = LUT_BLACK_C2_7;
					lut_vcom_dc_comp[LUT_VCOM_DC_BLACK_REPCOUNT] = LUT_BLACK_REPS_7;
					
					lut_ww_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_7;
					lut_bw_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_7;
					lut_wb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_7;
					lut_bb_comp[LUT_BLACK_COUNT1] = LUT_BLACK_C1_7;

					lut_ww_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_7;
					lut_bw_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_7;
					lut_wb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_7;
					lut_bb_comp[LUT_BLACK_COUNT2] = LUT_BLACK_C2_7;				

					lut_ww_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_7;
					lut_bw_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_7;
					lut_wb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_7;
					lut_bb_comp[LUT_BLACK_REPCOUNT] = LUT_BLACK_REPS_7;

					//RED
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT1] = LUT_RED_C1_7;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_COUNT2] = LUT_RED_C2_7;
					lut_vcom_dc_comp[LUT_VCOM_DC_RED_REPCOUNT] = LUT_RED_REPS_7;

					lut_ww_comp[LUT_RED_COUNT1] = LUT_RED_C1_7;
					lut_bw_comp[LUT_RED_COUNT1] = LUT_RED_C1_7;
					lut_wb_comp[LUT_RED_COUNT1] = LUT_RED_C1_7;
					lut_bb_comp[LUT_RED_COUNT1] = LUT_RED_C1_7;
                                                      
					lut_ww_comp[LUT_RED_COUNT2] = LUT_RED_C2_7;
					lut_bw_comp[LUT_RED_COUNT2] = LUT_RED_C2_7;
					lut_wb_comp[LUT_RED_COUNT2] = LUT_RED_C2_7;
					lut_bb_comp[LUT_RED_COUNT2] = LUT_RED_C2_7;				

					lut_ww_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_7;
					lut_bw_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_7;
					lut_wb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_7;
					lut_bb_comp[LUT_RED_REPCOUNT] = LUT_RED_REPS_7;
					
					_writeLUT_Composite();		
					break;
					*/
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
			Serial.printf("_writeLUT() 2BPP: _refreshnumber=%d\n", _refreshnumber);
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
			Serial.printf("_writeLUT() 1BPP: _refreshnumber=%d\n", _refreshnumber);
			_writeLUT_Normal();
			break;
			
		default:
			Serial.printf("_writeLUT() unknown BPP: _refreshnumber=%d\n", _refreshnumber);
			_writeLUT_Normal();
			break;
	}
}


void GxGDEW027C44::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027C44::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027C44::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027C44::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027C44::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEW027C44_WIDTH - x - w - 1;
      break;
    case 2:
      x = GxGDEW027C44_WIDTH - x - w - 1;
      y = GxGDEW027C44_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEW027C44_HEIGHT - y - h - 1;
      break;
  }
}

void GxGDEW027C44::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027C44_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027C44_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEW027C44_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027C44::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027C44_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027C44_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      //fillScreen(p);
      uint16_t ys = yds % GxGDEW027C44_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027C44::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027C44_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027C44_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEW027C44_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027C44::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027C44_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027C44_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEW027C44_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027C44::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t y = 0; y < GxGDEW027C44_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW027C44_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW027C44_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW027C44_WIDTH / 8 - 4) && (y > GxGDEW027C44_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW027C44_HEIGHT - 33)) data = 0x00;
      _writeData(~data);
    }
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData(0);
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}

