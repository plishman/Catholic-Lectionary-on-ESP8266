#include <RCGlobals.h>
// Catholic Lectionary on ESP
// Copyright (c) 2017-2020 Philip Lishman, Licensed under GPL3, see LICENSE

// Built with NodeMCU1.0(ESP12E module) config (Tools->Board) Using ESP8266 Community version 2.4.0 (Tools->Board->Boards Manager)
// Upload speed 921600
// CPU Freq 80MHz
// Debug Port: Disabled/Debug Level: None
// Flash Size 4M (1M SPIFFS) (1M spiffs allows space for OTA update)
// IWIP Variant: v1.4 Prebuilt

//ESP8266---
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <ESP8266WebServer.h>
//#ifndef CORE_v3_EXPERIMENTAL
  #include <SD.h>
  //#include <SdFat.h>
  #include <FS.h>
  #define FS_NO_GLOBALS
//#else
//  #include <SDFS.h>
//  //using namespace sdfat;
//  #define SD_FAT_TYPE 3
//#endif

#include "SimpleFTPServer.h"  // PLL-08-04-2021
FtpServer ftpSrv;

//----------
#include <pins_arduino.h>
//#include <I2CSerialPort.h>
#include <DebugPort.h>
#include <utf8string.h>
#include <TimeLib.h>
#include <Enums.h>
#include <I18n.h>
#include <Csv.h>
#include <Calendar.h>
#include <Temporale.h>
#include <Lectionary.h>
#include <Bible.h>
#include <Tridentine.h>
#include <Precedence.h>
#include <SPI.h>
//#include <epd2in7b.h>
//#include <epdpaint.h>
//#include <calibri10pt.h>
//#include <times10x4.h>
#include <edb.h>
#include <pgmspace.h>
#include <Network.h>
#include <Battery.h>
#include <RCConfig.h>
#include <rcimages.h>
#include <DiskFont.h>
#include <Bidi.h>
#include <FrameBuffer.h>

#include <GxEPD.h>

#ifdef EPAPER_GxGDEW027C44
  #include <GxGDEW027C44/GxGDEW027C44.cpp>      // 2.7" b/w/r 176x264 GxGDEW027C44/GxGDEW027C44.cpp
#endif
#ifdef EPAPER_GxGDEW042Z15
  #include <GxGDEW042Z15/GxGDEW042Z15.cpp>      // 4.2" b/w/r
#endif

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h> //(used for crash dump output)

extern "C" {
#include "user_interface.h"
}

#define SLEEP_HOUR 60*60e6

DebugPort Debug_Prt;

#ifdef LM_DEBUG
  File cfile;
  bool cclosefile = false;
#endif

GxIO_Class io(SPI, /*CS=D16*/ D8, /*DC=D4*/ -1, /*RST=D5*/ D2);  // dc=-1 -> not used (using 3-wire SPI)
GxEPD_Class ePaper(io, D2, D3 /*RST=D5*/ /*BUSY=D12*/);

#ifdef USE_SPI_RAM_FRAMEBUFFER
FrameBuffer fb(SPI, D4, 0, PANEL_SIZE_X, PANEL_SIZE_Y);
TextBuffer tb(fb);
DiskFont diskfont(fb);

// PLL-07-07-2020 fonts for Latin Mass Propers
  DiskFont diskfont_normal(fb);
  DiskFont diskfont_i(fb);
  DiskFont diskfont_plus1_bi(fb); 
  DiskFont diskfont_plus2_bi(fb);
// PLL-07-07-2020 fonts for Latin Mass Propers

#else
TextBuffer tb(ePaper);
DiskFont diskfont(ePaper);

// PLL-15-12-2020 fonts for Latin Mass Propers
  DiskFont diskfont_normal(ePaper);
  DiskFont diskfont_i(ePaper);
  DiskFont diskfont_plus1_bi(ePaper); 
  DiskFont diskfont_plus2_bi(ePaper);
// PLL-15-12-2020 fonts for Latin Mass Propers

#endif


enum DISPLAY_UPDATE_TYPE {
  display_reading, 
  battery_recharge, 
  connect_power, 
  wps_connect, 
  clock_not_set, 
  sd_card_not_inserted, 
  wireless_network_connected, 
  wps_setup_failed, 
  font_missing, 
  crash_bug
};


// Although function prototypes are usually handled by the Arduino IDE for .ino sketches, these prototypes 
// are necessary to allow default values to be set for the functions
void updateDisplay(DISPLAY_UPDATE_TYPE d, int8_t lut_mode = LUT_MODE_1BPP);
void updateDisplay(DISPLAY_UPDATE_TYPE d, String messagetext, uint16_t messagecolor, int8_t lut_mode = LUT_MODE_1BPP);

//void updateDisplay(DISPLAY_UPDATE_TYPE d);
//void updateDisplay(DISPLAY_UPDATE_TYPE d, String messagetext, uint16_t messagecolor);
//void epaperUpdate();
//void epaperDisplayImage();
////void display_image(DISPLAY_UPDATE_TYPE i);
////void display_image(DISPLAY_UPDATE_TYPE d, String messagetext, bool bMessageRed);

void display_image(DISPLAY_UPDATE_TYPE i) {
	display_image(i, "", false);
}

void display_image(DISPLAY_UPDATE_TYPE d, String messagetext, bool bMessageRed) {
	uint16_t messagecolor = GxEPD_BLACK;
	if (bMessageRed) messagecolor = GxEPD_RED;

	updateDisplay(d, messagetext, messagecolor);
}

//#include "rcdisplay.h";

/*
void updateDisplay(DISPLAY_UPDATE_TYPE d);
void updateDisplay(DISPLAY_UPDATE_TYPE d, String messagetext, uint16_t messagecolor);
void epaperUpdate();
void epaperDisplayImage();
*/

EPD_DISPLAY_IMAGE* epd_image = NULL;
String epaper_messagetext = "";
uint16_t epaper_messagetext_color = GxEPD_BLACK;
int displayPage = 0;

void updateDisplay(DISPLAY_UPDATE_TYPE d, int8_t lut_mode) {
	updateDisplay(d, "", GxEPD_BLACK, lut_mode);
}

void updateDisplay(DISPLAY_UPDATE_TYPE d, String messagetext, uint16_t messagecolor, int8_t lut_mode) {
	displayPage = 0;

	DEBUG_PRT.println(F("Updating Display"));
	ePaper.init(); // disable diagnostic output on Serial (use 115200 as parameter to enable diagnostic output on Serial)
	DEBUG_PRT.println(F("Init Display"));

	if (d == display_reading) {
		DEBUG_PRT.println(F("display_reading"));

		//ePaper.setMode(LUT_MODE_CLEAR);
		//ePaper.eraseDisplay(true);


		//ePaper.setMode(LUT_MODE_COMPOSITE);

		int epd_contrast = Config::GetEPDContrast();

		//switch (diskfont._FontHeader.antialias_level)
    switch (lut_mode)
		{
		case 4:
			ePaper.resetRefreshNumber(LUT_MODE_3BPP);
			break;

		case 2:
			ePaper.resetRefreshNumber(LUT_MODE_2BPP);
			epd_contrast = epd_contrast >> 1;
			break;

		case 1:
		default:
			ePaper.resetRefreshNumber(LUT_MODE_1BPP);
			epd_contrast = 1;
			break;
		}

// testing

#ifdef USE_SPI_RAM_FRAMEBUFFER
#if FRAMEBUFFER_TEST_GRADIENT
  int16_t x=10;
  int16_t y=150;
  for (int16_t j = y; j < y+16; j++) {
    for (int16_t i = x; i < x+16; i++) {
      for(int16_t k=0; k<8; k++) {
        int16_t xp = i + (16*k);
        //fb.drawPixel(xp, j, GxEPD_BLACK, k*2, false*/);
        //fb.drawPixel(xp, j+16, GxEPD_BLACK, k*2, true);
        //fb.drawPixel(xp, j+32, GxEPD_RED, k*2, false);
        //fb.drawPixel(xp, j+48, GxEPD_RED, k*2, true);
        fb.drawPixel(xp, j, GxEPD_BLACK, k*2);
        fb.drawPixel(xp, j+16, GxEPD_BLACK, k*2);
        fb.drawPixel(xp, j+32, GxEPD_RED, k*2);
        fb.drawPixel(xp, j+48, GxEPD_RED, k*2);
      }
    }
  }
#endif
#endif

// testing

		do {
			//Serial.printf("refresh number = %d\n", ePaper.getRefreshNumber());
			ePaper.drawPaged(epaperUpdate);
			DEBUG_PRT.println();
		} while (ePaper.decRefreshNumber() && ePaper.getRefreshNumber() >= epd_contrast);

		return;
	}
	//  if (d == display_reading) {
	//    DEBUG_PRT.println("display_reading");
	//    ePaper.drawPaged(epaperUpdate);
	//    return;    
	//  }

	ePaper.resetRefreshNumber(LUT_MODE_1BPP);

	epaper_messagetext = messagetext;
	epaper_messagetext_color = messagecolor;

	switch (d) {
	case battery_recharge:
		DEBUG_PRT.println(F("display battery_recharge image"));
		epd_image = &battery_recharge_image;
		break;

	case connect_power:
		DEBUG_PRT.println(F("display connect_power image"));
		epd_image = &connect_power_image;
		break;

	case wps_connect:
		DEBUG_PRT.println(F("display wps_connect image"));
		epd_image = &wps_connect_image;
		break;

	case clock_not_set:
		DEBUG_PRT.println(F("display clock_not_set image"));
		epd_image = &clock_not_set_image;
		break;

	case sd_card_not_inserted:
		DEBUG_PRT.println(F("display sd_card_not_inserted image"));
		epd_image = &sd_card_not_inserted_image;
		break;

	case wireless_network_connected:
		DEBUG_PRT.println(F("display wireless_network_connected image"));
		epd_image = &wireless_network_connected_image;
		break;

	case wps_setup_failed:
		DEBUG_PRT.println(F("display wps_setup_failed image"));
		epd_image = &wps_setup_failed_no_network_image;
		break;

	case font_missing:
		DEBUG_PRT.println(F("display font_missing image"));
		epd_image = &font_missing_image;
		break;

	case crash_bug:
		DEBUG_PRT.println(F("display crash_bug image"));
		epd_image = &crash_bug_image;
		break;
	}

	ePaper.drawPaged(epaperDisplayImage);
}

void epaperUpdate() {
	wdt_reset();
	ePaper.eraseDisplay();

#ifdef USE_SPI_RAM_FRAMEBUFFER
  //ePaper.setRotation(0); //90 degrees
  fb.setDisplayPage(displayPage); 
  fb.render(ePaper);
#else
  ePaper.setRotation(EPD_ROTATION);
  tb.render(ePaper, diskfont, displayPage);
  int charheight = diskfont._FontHeader.charheight;
  ePaper.drawFastHLine(0, charheight, ePaper.width(), GxEPD_BLACK);
#endif

	displayPage++;
	if (displayPage >= PAGE_COUNT) displayPage = 0;

	DEBUG_PRT.print(F("."));
}

void epaperDisplayImage() {
	if (epd_image == NULL) return;

	int image_size_x = 176;
	int image_size_y = 264;
 
	wdt_reset();
  uint8_t rot = 0;
  
  switch (EPD_ROTATION) {
    case 0:
    rot = 3;
    break; 

    case 1:
    rot = 0;
    break; 

    case 2:
    rot = 1;
    break; 

    case 3:
    rot = 2;
    break; 

    default:
    rot = 0;
    break; 
  }
  //(EPD_ROTATION == 1 ? 0 : 3);  // may render picture upside-down if EPD_ROTATION is not 1 or 0 (ie. if it is 2 or 3)
	ePaper.setRotation(rot); //These pictures are pre-rotated 90 degrees, for use on the 2.7 in display, byte order of which is portrait mode

  int image_offset_x = (ePaper.width() - image_size_x) / 2;
  int image_offset_y = (ePaper.height() - image_size_y) / 2;

	ePaper.eraseDisplay();
	if (epd_image->bitmap_black != NULL) ePaper.drawBitmap(epd_image->bitmap_black, image_offset_x, image_offset_y, image_size_x, image_size_y, GxEPD_BLACK, GxEPD::bm_transparent);
	if (epd_image->bitmap_red != NULL) ePaper.drawBitmap(epd_image->bitmap_red, image_offset_x, image_offset_y, image_size_x, image_size_y, GxEPD_RED, GxEPD::bm_transparent);

	if (epaper_messagetext != "") {
		//int strwidth = diskfont.GetTextWidth(epaper_messagetext);
		ePaper.setRotation(EPD_ROTATION);        
		ePaper.setFont(&FreeMonoBold9pt7b);

		int strend = 0;
		int strstart = 0;
		int cursor_y = (image_offset_x + 12) < 0 ? 12 : image_offset_x + 12;
		int cursor_x = (image_offset_y + 0) < 0 ? 0 : image_offset_y + 0;

		String substr = "";

		String charheightstr = "Ag";
		int16_t x1, y1;
		uint16_t w, h;
		ePaper.getTextBounds(charheightstr, 0, 0, &x1, &y1, &w, &h);

		int fontheight = h;

		if (!epaper_messagetext.endsWith("\n")) epaper_messagetext += "\n";

		while (strend != -1) {
			strend = epaper_messagetext.indexOf("\n", strstart);

			if (strend != -1) {
				substr = epaper_messagetext.substring(strstart, strend);
			}
			else {
				substr = epaper_messagetext.substring(strstart);
			}

			if (substr != "") {
				ePaper.setTextColor(GxEPD_WHITE);
				ePaper.setCursor(cursor_x - 1, cursor_y - 1);
				ePaper.print(substr);

				ePaper.setCursor(cursor_x + 1, cursor_y + 1);
				ePaper.print(substr);

				ePaper.setCursor(cursor_x - 1, cursor_y + 1);
				ePaper.print(substr);

				ePaper.setCursor(cursor_x + 1, cursor_y - 1);
				ePaper.print(substr);

				ePaper.setTextColor(epaper_messagetext_color);
				ePaper.setCursor(cursor_x, cursor_y);
				ePaper.print(substr);

				cursor_y += fontheight; // if the substring has a \n at the end, Y cursor should have been moved to the next line by the GFX library
			}
			strstart = strend + 1; // ready to move onto the next substring
		}
	}
	DEBUG_PRT.print(F("."));
}

//-- for EDB
File dbFile;
EDB db(&writer, &reader);
#include <BibleVerse.h>
//--

Battery battery;
Network network;

ESP8266WebServer server(80);

bool bEEPROM_checksum_good = false;

wake_reasons wake_reason = WAKE_UNKNOWN;
bool bNetworkAvailable = false; 
bool bRetryWPSConnect = true;
bool bDisplayWifiConnectedScreen = true;      

bool CrashCheck(String& resetreason) { // returns true if crash is detected
  checkSpiffsDump();
  
  char reset_info_buf[200];
  
  struct rst_info *rtc_info = system_get_rst_info();
  
  os_sprintf(reset_info_buf, "reset reason: %x\n",  rtc_info->reason);
  resetreason += String(reset_info_buf);

  os_sprintf(reset_info_buf, "exception cause: %x\n",  rtc_info->exccause);
  resetreason += String(reset_info_buf);

  os_sprintf(reset_info_buf, "Lectionary " LECT_VER "\n");
  resetreason += String(reset_info_buf);

  if (rtc_info->reason == REASON_WDT_RST || rtc_info->reason == REASON_EXCEPTION_RST || rtc_info->reason == REASON_SOFT_WDT_RST)  
  {
      if (rtc_info->reason == REASON_EXCEPTION_RST) {
        os_sprintf(reset_info_buf, "Fatal exception (%d):\n", rtc_info->exccause);
        resetreason += String(reset_info_buf);
      }
      
      os_sprintf(reset_info_buf, "epc1=0x%08x\nepc2=0x%08x\nepc3=0x%08x\nexcvaddr=0x%08x\ndepc=0x%08x\n", 
                 rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr,  rtc_info->depc); 
                 //The address of the last crash is printed, which is used to debug garbled output.

      resetreason += String(reset_info_buf);

      time64_t date = 0;
      Config::getLocalDateTime(&date);
      tmElements_t ts;
      breakTime(date, ts);
      os_sprintf(reset_info_buf,"\nat %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

      resetreason += String(reset_info_buf);
      
      return true;
  }

  return false;
}

void checkSpiffsDump() {
  // copy crashdump file from spiffs to sd card (should happen after rebooting)
  
  fs::File f_spiffs = SPIFFS.open(F(CRASHFILEPATH), "r");
  if(!f_spiffs) {
    DEBUG_PRT.println(F("No crash file found in SPIFFS"));
    return;
  }

  File f_sd = SD.open(F(CRASHFILEPATH), sdfat::O_RDWR | sdfat::O_APPEND);
  if(!f_sd) f_sd = SD.open(F(CRASHFILEPATH), sdfat::O_RDWR | sdfat::O_CREAT);
  if(f_sd) {
    time64_t date = 0;
    Config::getLocalDateTime(&date);
    tmElements_t ts;
    breakTime(date, ts);
    char buf[100];
    os_sprintf(buf,"\nat %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
    f_sd.write((const uint8_t*)buf, strlen(buf));  // write out the current date/time

    size_t st = 0;
    while (f_spiffs.available()) {    // then copy the crashdump file to the sd card
      st = f_spiffs.readBytes(buf, 100);
      f_sd.write((const uint8_t*)buf, st);
    }
    
    f_spiffs.close();
    f_sd.close();

    SPIFFS.remove(F(CRASHFILEPATH));
    DEBUG_PRT.println("Crashdump file in SPIFFS was copied to the SD card");
  } 
  else {
    DEBUG_PRT.println("Crashdump file exists in SPIFFS, but it could not be copied to the SD card");
  }
}

void savetoSpiffs(struct rst_info * rst_info, uint32_t stack, uint32_t stack_end,Print& outputDev ){

  uint32_t crashTime = millis();
  outputDev.printf("Crash # at %ld ms\n",crashTime);

  outputDev.printf("Reason of restart: %d\n", rst_info->reason);
  outputDev.printf("Exception cause: %d\n", rst_info->exccause);
  outputDev.printf("epc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\n", rst_info->epc1, rst_info->epc2, rst_info->epc3, rst_info->excvaddr, rst_info->depc);
  outputDev.println(">>>stack>>>");
  int16_t stackLength = stack_end - stack;
  uint32_t stackTrace;
  // write stack trace to EEPROM
  for (int16_t i = 0; i < stackLength; i += 0x10)
  {
    outputDev.printf("%08x: ", stack + i);
    for (byte j = 0; j < 4; j++)
    {
      uint32_t* byteptr = (uint32_t*) (stack + i+j*4);
      stackTrace=*byteptr;
      outputDev.printf("%08x ", stackTrace);
    }
    outputDev.println();

  }
  outputDev.println("<<<stack<<<\n");
}

/**
 * This function is called automatically if ESP8266 suffers an exception
 * It should be kept quick / concise to be able to execute before hardware wdt may kick in
 */
extern "C" void custom_crash_callback(struct rst_info * rst_info, uint32_t stack, uint32_t stack_end )
{
//  Serial.println("Custom save crash");
  //savetoSpiffs(rst_info, stack, stack_end,fileprinter);
  //fileprinter.close();
  class StringPrinter2 : public Print {
  public:
    String str="";
    StringPrinter2(){};
    virtual size_t write(const uint8_t character){str+=character;};
    virtual size_t write(const uint8_t *buffer, size_t size){
      String str2=String((const char *)buffer);
      str2.remove(size);
        str+=str2;
      };
  } strprinter2;
  savetoSpiffs(rst_info, stack, stack_end,strprinter2);

  SPIFFS.begin();
  fs::File f = SPIFFS.open(F(CRASHFILEPATH), "a");
  if(!f) f= SPIFFS.open(F(CRASHFILEPATH), "w");
  if(f) {
    unsigned int w=f.write((uint8_t*)strprinter2.str.c_str(), strprinter2.str.length());
    f.close();
  }
}

#ifdef CORE_v3_EXPERIMENTAL
//------------------------------------------------------------------------------
// Call back for file timestamps.  Only called for file create and sync().
void dateTime(uint16_t* date, uint16_t* time) {

  time64_t t;
  Config::getDateTime(&t);  // need to get the time without DST for file datestamps, as these are calculated by the ftp client etc, hence not using getLocalDateTime.

  DEBUG_PRT.printf("dateTime() Timestamp is %02d/%02d/%04d %02d:%02d:%02d\n", day(t), month(t), year(t), hour(t), minute(t), second(t));

  // Return date using FS_DATE macro to format fields.
  *date = sdfat::FS_DATE(year(t), month(t), day(t));

  // Return time using FS_TIME macro to format fields.
  *time = sdfat::FS_TIME(hour(t), minute(t), second(t));

  DEBUG_PRT.printf("FAT_DATE: %02d/%02d/%04d %02d:%02d:%02d\n", sdfat::FS_DAY(*date), sdfat::FS_MONTH(*date), sdfat::FS_YEAR(*date), sdfat::FS_HOUR(*time), sdfat::FS_MINUTE(*time), sdfat::FS_SECOND(*time));
}

time_t fsTimeStampCallback() {
  #if TIME_NATIVE_64BIT == 0
    time_t t = now32();
  #else
    time_t t = now();
  #endif

  DEBUG_PRT.printf("fsTimeStampCallback(): %02d/%02d/%04d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));
  return t;
}
#endif

/**
 * setup
 **/
void setup() { 
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH); // set sd card to not selected

  pinMode(D4, OUTPUT); 
  digitalWrite(D4, HIGH); // set SPI ram chip to not selected (if attached)

  pinMode(D8, OUTPUT); 
  digitalWrite(D8, HIGH); // set epaper display to not selected

  bool bSDOk = SD.begin(D1, SPI_HALF_SPEED);
  bool bSpiffsOk = SPIFFS.begin();
    
  //DEBUG_PRT.begin(1,3,8);

  #ifdef CORE_v3_EXPERIMENTAL
  // Timestamping works correctly when Lectionary is built with v3.0 of the ESP8266 libaries and tools (still in development)
  sdfat::FsDateTime::setCallback(dateTime); 
  SDFS.setTimeCallback(fsTimeStampCallback);
  #endif
  
  DEBUGPRT_BEGIN
  delay(100);

  DEBUG_PRT.print(F("\n**Lectionary************\nMount:"));
  DEBUG_PRT.print(F(" SD FS: "));
  DEBUG_PRT.print(bSDOk);
  DEBUG_PRT.print(F(", SPIFFS FS: "));
  DEBUG_PRT.println(bSpiffsOk);
  
  DEBUG_PRT.print(F("free memory = "));
  //Serial.println("=");
  
  DEBUG_PRT.println(String(system_get_free_heap_size()));

  String resetreason = "";
  bool bCrashed = false;
  
  bCrashed = CrashCheck(resetreason);
  DEBUG_PRT.println(resetreason);

  if (bCrashed) {
    display_image(crash_bug, resetreason, false);
    DEBUGPRT_END
    Config::PowerOff(0); // power off until USB5V is (re)connected
    ESP.deepSleep(0); 
  }

  DEBUG_PRT.println(F("Checking wake reason..."));
  wake_reason = Config::Wake_Reason();
  DEBUG_PRT.println(F("Finished checking wake reason."));

  if (wake_reason != WAKE_ALARM_1) Config::SetPowerOn(); //attempt to hold up alarm 1 flag A1F - not writable in the DS3231 spec, but useful to keep the power on if the wake reason was not an alarm

  DEBUG_PRT.println(F("--------------------------"));
  DEBUG_PRT.println(F("abcdefghijklmnopqrstuvwxyz"));
  DEBUG_PRT.println(F("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  DEBUG_PRT.println(F("0123456789"));
  DEBUG_PRT.println(F("--------------------------"));
  
  DEBUG_PRT.print(F("Running version: "));
  DEBUG_PRT.println(ESP.getFullVersion());

  //WiFi.disconnect(); // testing - so have to connect to a network each reboot

  battery_test();
  clock_battery_test();

/*
  #ifdef CORE_v3_EXPERIMENTAL
  // Timestamping works correctly when Lectionary is built with v3.0 of the ESP8266 libaries and tools (still in development)
  // (needed to merge v3 SPI driver with old version to support 9bit transfers to epaper display)
  sdfat::FsDateTime::setCallback(dateTime); 
  #endif
*/
  // Check if EEPROM checksum is good

  //Config c;
  bEEPROM_checksum_good = Config::EEPROMChecksumValid();

  if (!bEEPROM_checksum_good && !Battery::power_connected()) {
    DEBUG_PRT.println(F("On battery power and EEPROM checksum invalid: Sleeping until USB power is attached and web interface is used to configure EEPROM"));
    display_image(connect_power);
    DEBUGPRT_END
    if (!Config::PowerOff(0)) {
      DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
      DEBUGPRT_END
      ESP.deepSleep(0); //sleep until USB power is reconnected // sleep for an hour (71minutes is the maximum!), or until power is connected (SLEEP_HOUR)
    }
  }

  rtcData_t rtcData = {0};
//  bool bRetryWPSConnect = true; // is now module level variable (used in loop to replicate state of this flag in the RTC memory)
  if (Config::readRtcMemoryData(rtcData)) {
    DEBUG_PRT.printf("Read rtc memory data, wake_flags == %d\n", rtcData.data.wake_flags);
    
    if (rtcData.data.wake_flags == WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING) {
      DEBUG_PRT.println(F("rtcData flags = WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING, setting bRetryWPSConnect to False"));
	  bRetryWPSConnect = false;
    }

    if (rtcData.data.wake_flags == WAKE_FLAGS_STILL_CHARGING_DONT_REDISPLAY_WIFI_CONNECTED_IMAGE) {
      DEBUG_PRT.println(F("rtcData flags = WAKE_FLAGS_STILL_CHARGING_DONT_REDISPLAY_WIFI_CONNECTED_IMAGE, setting bDisplayWifiConnectedScreen to False"));
	  if (bEEPROM_checksum_good) {
		  bDisplayWifiConnectedScreen = false;
	  }
	  else {
		  bDisplayWifiConnectedScreen = true; // always display wifi connected screen if the EEPROM contents are not valid
	  }
    }
  } else {
      DEBUG_PRT.println(F("rtcData memory is invalid (ESP8266 has been powered off), bRetryWPSConnect defaulting to True"));    
  }

  if (Battery::power_connected()) {
    if (!network.connect()){
      DEBUG_PRT.println(F("Need to configure Wifi with WPS to enable web configuration"));
      if (bRetryWPSConnect) {
        DEBUG_PRT.println(F("On USB power and no network configured: Prompting user to connect using WPS button"));
        if (!connect_wps()) {
          DEBUG_PRT.println(F("Failed to configure Wifi network via WPS - will wake in 10 minutes to display reading if power is not disconnected and reconnected to start WPS configuration again."));
  
          /////
          // 24-11-2018 Fix to allow lectionary to display verses even when no network is configured, following USB5V being connected (which will try to
          // detect a network).
          
          if (bEEPROM_checksum_good) { // * If the EEPROM contains sensible values, lectionary will wake after 1 minute and display verses as normal, *if 
            display_image(wps_setup_failed); // USB5V is not still connected when the lectionary wakes (ie, the USB5V as not been removed and/or reattached)*. 
                                       // * This means that the lectionary can be charged and used without configuring a network if none is available, 
                                       //   provided it is already configured (so it works and can be charged if taken on holiday etc, when no WiFi is available).
                                       // * If USB5V is disconnected then reconnected though at any time, the lectionary will prompt to connect to a network 
                                       //   again, until WPS configuration is completed successfully.
                                       // * The lectionary will not wake by itself again without disconnecting and reconnecting 5V if the 5V power is left 
                                       //   connected after WPS configuration has failed.
            DEBUG_PRT.println(F("EEPROM checksum is good, so will setting WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING in RTC memory"));
          
            rtcData.data.wake_hour_counter = 1;  // Write 1 hour until display of next reading in RTC memory. This will mean that, after being decremented 
                                                 // to 0, a reading should be displayed immediately (at next wake from deepSleep mode, if 5V power remains 
                                                 // applied.
            rtcData.data.wake_flags = WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING; // will ensure that if power remains applied (lectionary is
                                                                                             // left charging), it will not attempt to retry the WPS connect
                                                                                             // on waking, but will instead display a reading. Only if the power
                                                                                             // is disconnected then reconnected should it reattempt WPS config.
            Config::writeRtcMemoryData(rtcData); //
  
            SleepFor(0,1,0);                    // Sleep for 0hrs 2minutes 0sec - will display reading after that, if power is not disconnected and reconnected
                                                 // in which case, will reattempt WPS config.
            //SleepForHours(1);                    // SleepForHours() will most probably end up using deepSleep mode, since the USB5V has been
                                                 // detected as connected already at this point (which keeps the esp8266 powered). If the USB5V  
                                                 // is disconnected sometime during the deepSleep the DS3231 should still wake up the lectionary
                                                 // on time.                                                 
          }
          else {
          /////
            DEBUG_PRT.println(F("EEPROM checksum is bad and no network configured, sleeping until power is disconnected and reconnected, to restart WPS configuration"));
            display_image(connect_power);
            DEBUGPRT_END
            if (!Config::PowerOff(0)) {
              DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
              DEBUGPRT_END
              ESP.deepSleep(0); // sleep indefinitely, reset pulse will wake the ESP when USB power is unplugged and plugged in again. //sleep for an hour (71minutes is the maximum!), or until power is connected SLEEP_HOUR
            }
          }
        }
        else {
          //ESP.deepSleep(1e6); // reset esp, network is configured
          //ESP.reset();
          DEBUGPRT_END
          ESP.restart();
        }
      }
    }
    else {
      bNetworkAvailable = true;
      if (bDisplayWifiConnectedScreen) {
        display_image(wireless_network_connected, "http://" + Network::getDHCPAddressAsString(), true);
        rtcData.data.wake_flags = WAKE_FLAGS_STILL_CHARGING_DONT_REDISPLAY_WIFI_CONNECTED_IMAGE;
        Config::writeRtcMemoryData(rtcData); // set the wake flag to say don't redisplay the wifi connected image on next boot, if still charging by 
                                             // that time.
      }
      //PLL-13-06-2020 on usb power for some time, and woke from deep sleep (rather than powered off by clearing A1F in the DS3231)
      else {
        if (wake_reason == WAKE_DEEPSLEEP) { // ESP woke from deep sleep - so was still powered, hence RTC memory (in the ESP8266) may be set. This is a backup/legacy code, the DS3231 clock chip will apply power when its alarm asserts
          if (Config::readRtcMemoryData(rtcData)) { // if CRC fails for values, this is probably a cold boot (power up), so will always update the reading in this case
            if (rtcData.data.wake_hour_counter > 8) { // sanity check - should never be more than 8 hours between readings.
              rtcData.data.wake_hour_counter = 1; // this will mean that, after being decremented to 0, a reading should be displayed immediately
            }
          
            rtcData.data.wake_hour_counter--;
            Config::writeRtcMemoryData(rtcData);
    
            if (rtcData.data.wake_hour_counter > 0) {
              DEBUG_PRT.print(F("No reading this hour, still on USB5V, going back to sleep immediately.\nNumber of hours to next reading is "));
              DEBUG_PRT.println(rtcData.data.wake_hour_counter);
              //SleepUntilStartOfHour(); // no reading this hour, go back to sleep immediately
              SleepForHours(rtcData.data.wake_hour_counter);
            }
          }
          else {
            DEBUG_PRT.println(F("Updating now - Woken from deepsleep while still charging but RTC memory (which should contain time of next reading update) contains no valid data"));
          }
        }
      } // PLL-13-06-2020
    }
  }
  else {
    DEBUG_PRT.println(F("On battery power - not attempting to connect to network. Connect power to use lectionary setup (http://lectionary.local/)."));  

//    rst_info *rinfo;
//    rinfo = ESP.getResetInfoPtr();
//    DEBUG_PRT.println(String("ResetInfo.reason = ") + (*rinfo).reason);

//    if ((*rinfo).reason == REASON_DEEP_SLEEP_AWAKE) { // only check the hour count to the next reading if we awoke because of the deepsleep timer

    if (wake_reason == WAKE_DEEPSLEEP) { // ESP woke from deep sleep - so was still powered, hence RTC memory (in the ESP8266) may be set. This is a backup/legacy code, the DS3231 clock chip will apply power when its alarm asserts
      if (Config::readRtcMemoryData(rtcData)) { // if CRC fails for values, this is probably a cold boot (power up), so will always update the reading in this case
        if (rtcData.data.wake_hour_counter > 8) { // sanity check - should never be more than 8 hours between readings.
          rtcData.data.wake_hour_counter = 1; // this will mean that, after being decremented to 0, a reading should be displayed immediately
        }
        
        rtcData.data.wake_hour_counter--;
        Config::writeRtcMemoryData(rtcData);
  
        if (rtcData.data.wake_hour_counter > 0) {
          DEBUG_PRT.print(F("No reading this hour, and on battery, so going back to sleep immediately.\nNumber of hours to next reading is "));
          DEBUG_PRT.println(rtcData.data.wake_hour_counter);
          //SleepUntilStartOfHour(); // no reading this hour, go back to sleep immediately
          SleepForHours(rtcData.data.wake_hour_counter);
        }
      }
      else {
        DEBUG_PRT.println(F("Updating now - Woken from deepsleep but RTC memory (which should contain time of next reading update) contains no valid data"));
      }
    }
  }

//  // Check if EEPROM checksum is good
//
//  Config c;
//  bEEPROM_checksum_good = c.EEPROMChecksumValid();
//
//  if (!bEEPROM_checksum_good && !Battery::power_connected()) {
//    DEBUG_PRT.printf("On battery power and EEPROM checksum invalid: Sleeping until USB power is attached and web interface is used to configure EEPROM");
//    display_image(connect_power);
//    ESP.deepSleep(0); //sleep until USB power is reconnected // sleep for an hour (71minutes is the maximum!), or until power is connected (SLEEP_HOUR)
//  }
}

void battery_test() {
  DEBUG_PRT.print(F("Battery voltage is "));
  DEBUG_PRT.println(String(Battery::battery_voltage()));
  
  if (!Battery::power_connected()) {
    if (Battery::recharge_level_reached()) {
      DEBUG_PRT.print(F("Battery recharge level is "));
      DEBUG_PRT.println(String(MIN_BATT_VOLTAGE));
      DEBUG_PRT.println(F("Battery recharge level reached - sleeping until power is connected"));
      display_image(battery_recharge);
      //while(!Battery::power_connected()) {
      //  wdt_reset();
      //  delay(2000); // testing - when finished, will be sleep (indefinite, wakes when charger is connected through reset pulse)
      //}
      DEBUGPRT_END
      if (!Config::PowerOff(0)) { // **24-11-2018 use RTC power switch to turn off all peripherals rather than just place esp8266 in deepSleep mode
        DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
        DEBUGPRT_END
        ESP.deepSleep(0); // sleep indefinitely, reset pulse will wake the ESP when USB power is unplugged and plugged in again. //sleep for an hour (71minutes is the maximum!), or until power is connected SLEEP_HOUR
      }
      
      //ESP.deepSleep(0); // sleep indefinitely (will wake when the power is connected, which applies a reset pulse) //sleep for an hour or until power is connected SLEEP_HOUR
    }
  }
  else {
    DEBUG_PRT.println(F("Battery is charging"));
  }
}

void clock_battery_test() {
  int retries = 10;
  bool ok = false;
  bool clockstopped = false;
  while (!ok && --retries != 0) {
    clockstopped = Config::ClockStopped(ok);
    delay(50);
  }

  if (clockstopped) {
    DEBUG_PRT.println(F("Clock stopped"));

    bool ok2 = false;
    while (!ok2 && --retries != 0) {
      ok2 = Config::ClearClockStoppedFlag();
      delay(50);
    }
    // the clock now runs on the main battery, which will output between about 2.7v (empty) and 4.2v (full). The clock will run on the full range of these voltages,
    // but the ESP8266 and peripherals will only run until the battery gets down to about 3.4 volts, when the 5v boost circuit will stop producing power for the 3.3v
    // LDO regulator. So the clock should always run, even when the "charge battery" screen is displayed, and for quite some time after that.
    // Running the clock chip directly on the battery should not affect charging, since when 5v power is applied via usb, which is when the battery charges, the clock
    // chip will switch over to the 3.3v LDO output supply automatically, even though it has a lower voltage than the battery (typically). See table 1, "power 
    // management" on page 10 of  the DS3231 data sheet for details of power management:.
    //
    // SUPPLY CONDITION ACTIVE SUPPLY
    // VCC < VPF, VCC < VBAT VBAT <- (VCC is less than power fail voltage (off), and VCC is less than vbat. This is the case when the 3.3V LDO output is off).
    // VCC < VPF, VCC > VBAT VCC
    // VCC > VPF, VCC < VBAT VCC  <- (VCC is on (3.3v), but VCC is less than VBAT - VCC will be 3.3v, vbat will be about 3.7v nominal, but the chip will still use VCC for power).
    // VCC > VPF, VCC > VBAT VCC
    
    DEBUG_PRT.println(F("Cleared OSF status bit (clock stopped flag)"));
/*
    if (!Config::ClockWasReset()) { // clock should reset when the battery is actually removed and replaced, if so, we will assume the battery has already been replaced.
                                    // since even a dead battery should have enough energy to maintain the last time setting, but with the oscillator stopped.
      DEBUG_PRT.println(F("Clock was not reset, assuming battery has not yet been changed, so displaying replace_cr2032_image"));
      display_image(replace_cr2032);

      if (!Config::PowerOff(0)) {
        DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
        ESP.deepSleep(0); //sleep until USB power is reconnected
      }
    }
*/
    Config::InvalidateEEPROM(); // if the clock stopped, invalidate the eeprom contents, to force the user to reset them on next boot (connect usb 5v).
    setTime(0, 0, 0, 1, 1, 2000); // default to midnight on 1-1-2000 if clock has stopped
  }
  else {
    setSyncProvider(timeProvider);
    setSyncInterval(120);    
  }
}

time64_t timeProvider()
{
  time64_t t;
  Config::getLocalDateTime(&t);
  return t;
}

/*
void datestampProvider(uint16_t* stamp_date, uint16_t* stamp_time) {
  *stamp_date = FAT_DATE(year(), month(), day());
  *stamp_time = FAT_TIME(hour(), minute(), second());
}
*/

bool connect_wps(){
  DEBUG_PRT.println(F("Please press WPS button on your router.\n Press any key to continue..."));
  display_image(wps_connect);
  wdt_reset();
  delay(10000);
  bool connected = network.startWPSPBC();      
    
  if (!connected) {
    DEBUG_PRT.println(F("Failed to connect with WPS :-("));  
    return false;
  }
  
  return true;
}




/**
 * loop
 **/
void loop(void) {   
  /************************************************/ 
  // *0* Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  Calendar c(D1); // PLL 29-04-2020 now create calendar object before running config server, so that the language setting can be determined and sent to the config webpage

  String config_server_lang = "default";
  if (c._I18n->configparams.have_config) {
    config_server_lang = c._I18n->configparams.lang;
  }
  DEBUG_PRT.println(F("*0*\n"));
  config_server(config_server_lang);  // config server will run if battery power is connected

  //- uncomment to cause a crash (debugging/testing) --------------------------------------
  //while(true){};
  //---------------------------------------------------------------------------------------

  /************************************************/ 
  // *1* Create calendar object and load config.csv
  wdt_reset();

  int next_wake_hours_count = 1; // will store the number of hours until the next wake up
  
  DEBUG_PRT.println(F("*1*\n"));
  //timeserver.gps_wake();
  //Calendar c(D1); // PLL 29-04-2020 now calendar object only reads the config in its constructor, temporale and sanctorale objects (which use memory) are created only when c.get() is called

  if (bEEPROM_checksum_good) {  
    if (!c._I18n->configparams.have_config) {
      DEBUG_PRT.println(F("Error: Failed to get config: config.csv is missing or bad or no SD card inserted"));
      display_image(sd_card_not_inserted);
      //ESP.deepSleep(SLEEP_HOUR);    
      DEBUGPRT_END
      if (!Config::PowerOff(0)) {
        DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
        DEBUGPRT_END
        ESP.deepSleep(0); // sleep indefinitely, reset pulse will wake the ESP when USB power is unplugged and plugged in again. //sleep for an hour (71minutes is the maximum!), or until power is connected SLEEP_HOUR
      }

    }
  
    bool right_to_left = c._I18n->configparams.right_to_left;
    bool verse_per_line = c._I18n->configparams.cr_after_verse;
    bool show_verse_numbers = c._I18n->configparams.show_verse_numbers;
    
    Bidi::SetEllipsisText(c._I18n->get("ellipsis"));

    /************************************************/ 
    // *2* Get date and time from DS3231 clock chip
    wdt_reset();
    DEBUG_PRT.println(F("*2*\n"));
    //time64_t date = c.temporale->date(12,11,2017);
    DEBUG_PRT.println(F("Getting datetime..."));
    //time64_t date = timeserver.local_datetime();
    //time64_t date;
    //Config::getLocalDateTime(&date);
    time64_t date = now();

    roundupdatetohour(date); // the esp8266 wake timer is not very accurate - about +-3minutes per hour, so if the date is within 5 mins of the hour, close to an hour, 
                     // will round to the hour.
    
    tmElements_t ts;
    breakTime(date, ts);
  
    //while (!network.get_ntp_time(&date)) {
    //  Serial.print(".");
    //  delay(1000);
    //}
    DEBUG_PRT.println(F("Got datetime."));
    //network.wifi_sleep(); // no longer needed, sleep wifi to save power
  
    bool bLatinMass = c._I18n->configparams.b_use_extraordinary_form; //lectionary_path.startsWith("/Lect/EF"); //true;
    int8_t latinmass_custom_waketime = -1;
    
    if(!bLatinMass) {  
      /************************************************/ 
      // *3* get Bible reference for date (largest task)
      wdt_reset();
      DEBUG_PRT.println(F("*3*\n"));
      c.get(date);
    
    
    
      /************************************************/ 
      // *4* Make calendar entry text string for day
      DEBUG_PRT.println(F("*4*\n"));
      //String mth = c._I18n->get("month." + String(ts.Month));
      ////String datetime = String(ts.Day) + " " + String(m) + " " + String(ts.Year + 1970);
      //String datetime = String(ts.Day) + " " + mth + " " + String(ts.Year + 1970);
      String datetime = c._I18n->getdate(date);
      DEBUG_PRT.println(datetime + "\t" + c.day.season + "\t" + c.day.day + "\t" + c.day.colour + "\t" + c.day.rank);
      if (c.day.is_sanctorale) {
        DEBUG_PRT.println("\t" + c.day.sanctorale + "\t" + c.day.sanctorale_colour + "\t" + c.day.sanctorale_rank);
      }
      
    
      
      /************************************************/ 
      // *5* Get lectionary (readings) for this date
      DEBUG_PRT.println(F("*5*\n"));
      String refs = "";
      wdt_reset();
      Lectionary l(c._I18n);
    
      bool b_OT = false;
      bool b_NT = false; 
      bool b_PS = false; 
      bool b_G = false;
    
      l.test(c.day.lectionary, c.day.liturgical_year, c.day.liturgical_cycle, &b_OT, &b_NT, &b_PS, &b_G);    
      DEBUG_PRT.printf("OT:%s NT:%s PS:%s G:%s\n", String(b_OT).c_str(), String(b_NT).c_str(), String(b_PS).c_str(), String(b_G).c_str());
      
      ReadingsFromEnum r;
  
      bool getLectionaryReadingEveryHour = true; // was false. If we get here, must return a reading, since reading scheduling when using DeepSleep mode 
                                                 // (which wakes *every hour* to check if a reading is due) is now handled in the init() code
      //if (wake_reason != WAKE_ALARM_1 || wake_reason == WAKE_USB_5V || Battery::power_connected()) getLectionaryReadingEveryHour = true;    
          
      if (getLectionaryReading(date, &r, getLectionaryReadingEveryHour/*true Battery::power_connected()*/, b_OT, b_NT, b_PS, b_G)) {      
        l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);    
    
        if (refs == "") { // 02-01-18 in case there is no reading, default to Gospel, since there will always be a Gospel reading
          r=READINGS_G; // there may be a bug: during weekdays, when the Lectionary number comes from a Saints' day and not the Calendar (Temporale), during Advent, Christmas
                                    // and Easter there may be a reading from NT (Christmas and Easter, when normally they're absent), or the NT (normally absent during Advent).
                                    // Need to check this works properly, but I don't have the Lectionary numbers for all the Saints' days, so currently the Temporale Lectionary numbers are
                                    // returned on Saints days (apart from those following Christmas day, which I did have).
          l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);    
        }
              
        /************************************************/ 
        // *6* Update epaper display with reading, use disk font (from SD card) if selected in config
        DEBUG_PRT.println(F("*6*\n"));
        if (!display_calendar(datetime, &c, refs, right_to_left, verse_per_line, show_verse_numbers)) { // if there is no reading for the current part of the day, display the Gospel reading instead (rare)
          DEBUG_PRT.println(F("No reading found (Apocrypha missing from this Bible?). Displaying Gospel reading instead\n"));
          r=READINGS_G;
          l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);
          display_calendar(datetime, &c, refs, right_to_left, verse_per_line, show_verse_numbers);
        }
      }
      else {
        if (wake_reason == WAKE_DEEPSLEEP) {
          DEBUG_PRT.println(F("Lectionary was woken from ESP8266 DeepSleep mode (wake_reason == WAKE_DEEPSLEEP. ESP8266 DeepSleep mode must wake every hour, even if the reading does not need to be updated. Reading not scheduled to be updated for this hour (updates every 4 hours, at 8am, 12pm, 4pm, 8pm and midnight"));    
        }
        else {
          DEBUG_PRT.println(F("Lectionary was woken from unknown cause (wake_reason == WAKE_UNKNOWN). Reading not scheduled to be updated for this hour (updates every 4 hours, at 8am, 12pm, 4pm, 8pm and midnight"));            
        }
      }
    }
    else { // if bLatinMass

//#define LM_DEBUG 1 - see RCGlobals.h
#ifdef LM_DEBUG
      cclosefile = false; // global
      
      WriteCalendarDay("<table>\n");

      time64_t testdate = Temporale::date(1, 1, 2021);
      for (int t = 0; t < 365; t++) {
        DEBUG_PRT.print(".");
        WriteCalendarDay("<tr>\n");
        String datestring = c._I18n->getdate(testdate);
        DoLatinMassPropers(testdate, datestring, c._I18n->configparams.right_to_left, c._I18n->configparams.lectionary_path, c._I18n->configparams.lang, latinmass_custom_waketime);
        testdate += SECS_PER_DAY;
        WriteCalendarDay("\n</tr>\n");
      }

      cclosefile = true;
      WriteCalendarDay("</table>\n");
      DEBUGPRT_END 
      SleepForHours(next_wake_hours_count);
#endif
      
      String datestring = c._I18n->getdate(date);
      DoLatinMassPropers(date, datestring, c._I18n->configparams.right_to_left, c._I18n->configparams.lectionary_path, c._I18n->configparams.lang, latinmass_custom_waketime);
    }
    DEBUG_PRT.println(F("Calculating next wake time:"));

    rtcData_t rtcData = {0};

    //preserve wake flags
    if (!Config::readRtcMemoryData(rtcData)) {
      rtcData = {0}; // if crc failed, make sure rtcData struct is emptied
      rtcData.data.wake_flags = WAKE_FLAGS_NONE; // if the RTC memory was corrupted (crc fail), treat this is a cold boot and make sure wake flags cleared
    } // otherwise, preserve the wake_flags read from the rtc memory.

    //at present, wake_flags should be one of: * WAKE_FLAGS_NONE
    //                                         * WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING
    //                                         * WAKE_FLAGS_STILL_CHARGING_DONT_REDISPLAY_WIFI_CONNECTED_IMAGE
    // As coded, they are not presently combinable - only one of these flags is set at a time.
    
    //rtcData.data.wake_flags = (bRetryWPSConnect ? WAKE_FLAGS_NONE : WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING);
    
    //calculate number of hours to next wake up
    rtcData.data.wake_hour_counter = 1;
    uint8_t Hour = ts.Hour;
    int i = 1;
    
    if(!bLatinMass) { 
      ReadingsFromEnum r;
    
      bool b_OT = false;
      bool b_NT = false; 
      bool b_PS = false; 
      bool b_G = false;

      if (Hour != 23) { // if crossing a day boundary, the season and/or number of readings may change, so will always wake on a day boundary, so make the next wake time default to 1 hour
        while (!getLectionaryReading(date + (SECS_PER_HOUR * i), &r, false, b_OT, b_NT, b_PS, b_G)) { // check if there will be a reading next hour
          rtcData.data.wake_hour_counter++;
          i++;
          Hour = (Hour + 1 > 23) ? 0 : Hour + 1;
          if (Hour == 0) break;
        }
      }
    }
    else { // if bLatinMass
      DEBUG_PRT.print(F("latinmass_custom_waketime = "));
      DEBUG_PRT.println(latinmass_custom_waketime);
     
      if (latinmass_custom_waketime == -1) {
        if (ts.Hour >= 0 && ts.Hour < 9) {
          rtcData.data.wake_hour_counter = 9 - ts.Hour; // midnight to 8am Introit (Gloria at 9am)
          Hour = 9;
        }
        else if (ts.Hour >= 19) {
          rtcData.data.wake_hour_counter = 24 - ts.Hour;  // 7pm to midnight Postcommunio (followed by next day's Introit at midnight)
          Hour = 0;
        }
        else if (ts.Hour > 8 && ts.Hour < 19) {         // remaining parts of Mass propers at 1 hour intervals between 9 am (Gloria) and 7pm (Postcommunio)
          rtcData.data.wake_hour_counter = 1;
          Hour = ts.Hour + 1;
        }
      }
      else {          
          Hour = latinmass_custom_waketime;       
          if (latinmass_custom_waketime == 0) latinmass_custom_waketime = 24; // handle wrap around
          rtcData.data.wake_hour_counter = latinmass_custom_waketime - ts.Hour;
      }
    }
    
    DEBUG_PRT.printf("Next reading is in %d hour(s)\n", rtcData.data.wake_hour_counter); 

    Config::writeRtcMemoryData(rtcData);        

    next_wake_hours_count = rtcData.data.wake_hour_counter; // for setting wake alarm  
  } // if(bEEPROM_checksum_good)
  else {
    DEBUGPRT_END 
    ESP.deepSleep(1e6); //reboot after 1 second. After reset, the "connect power" screen should be shown.
  }
   
  /************************************************/ 
  // *7* completed all tasks, go to sleep
  DEBUG_PRT.println(F("*7*\n"));  
  DEBUG_PRT.println(F("Going to sleep"));
  SleepForHours(next_wake_hours_count);
}

void config_server(String lang)
{  
  // Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  if (!bDisplayWifiConnectedScreen) {
    network.wifi_sleep(); // PLL-29-12-2020 no longer needed, sleep wifi to save power
    return; // only run the webserver if the wifi connected screen is shown (will only show once, when power is first connected). So after resetting (eg. when settings have been updated) it the webserver should not be run
  }

  if (!bEEPROM_checksum_good) {
    if (Battery::power_connected() && bNetworkAvailable) {
      display_image(clock_not_set, "http://" + Network::getDHCPAddressAsString(), true);
    } else {
      display_image(connect_power);
    }
  }

  bool bSettingsUpdated = false;

  if (!bNetworkAvailable && Battery::power_connected()) {
    DEBUG_PRT.println(F("On USB power, but Network is not available - disconnect and reconnect power to use WPS setup"));
  }

  if (Battery::power_connected() && bNetworkAvailable) {
    DEBUG_PRT.println(F("Power is connected, starting config web server, FTP Server and OTA update server"));
    DEBUG_PRT.print(F("USB voltage is "));
    DEBUG_PRT.println(String(Battery::battery_voltage()));

    // PLL-08-04-2021 FTP Server
    ftpSrv.begin(LECT_FTP_USER, LECT_FTP_PASS);    //username, password for ftp.   (default 21, 50009 for PASV)
    
    // PLL-27-04-2020 OTA Update code (https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/)   
    ArduinoOTA.onStart([]() {
      DEBUG_PRT.println(F("OTA Start"));
    });
    ArduinoOTA.onEnd([]() {
      DEBUG_PRT.println(F("\nOTA End"));
      updateDisplay(crash_bug, "Flash update SUCCESS!", GxEPD_RED);
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      DEBUG_PRT.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      DEBUG_PRT.printf("OTA Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) DEBUG_PRT.println(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR) DEBUG_PRT.println(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR) DEBUG_PRT.println(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR) DEBUG_PRT.println(F("Receive Failed"));
      else if (error == OTA_END_ERROR) DEBUG_PRT.println(F("End Failed"));
      updateDisplay(crash_bug, "Flash update FAIL", GxEPD_RED);
    });
    ArduinoOTA.begin();
    DEBUG_PRT.println(F("Ready"));
    DEBUG_PRT.print(F("IP address: "));
    DEBUG_PRT.println(WiFi.localIP());
    //
   
    // Network should already be connected if we got in here, since when on usb power network connects at start, or prompts to configure if not already done
    //if (!network.connect()) {
    //  DEBUG_PRT.println("Network is not configured, starting WPS setup");
    //  connect_wps();
    //  ESP.reset();
    //}
    
    DEBUG_PRT.print(F("free memory = "));
    DEBUG_PRT.println(String(system_get_free_heap_size()));

    #define SERVER_DELAY_SLOW 10;
    #define SERVER_DELAY_FAST 2;

    #ifdef CORE_v3_EXPERIMENTAL
    #define FINALDELAY 20000 // milliseconds  (web serving is slower due to needed streamFile() workaround with current dev version of ESP8266 Core and libraries)
    #else
    #define FINALDELAY 7000 // milliseconds
    #endif
    
    #define FINAL_DELAY_SLOW FINALDELAY / SERVER_DELAY_SLOW; // number of further loops of the server mainloop to run before shutting down
    #define FINAL_DELAY_FAST FINALDELAY / SERVER_DELAY_FAST;

    unsigned long delay_msec = SERVER_DELAY_SLOW;

    unsigned long server_start_time = millis();
    bool bTimeUp = false;    
    uint8_t ftpsrv_status = 0; //PLL-10-04-2021 TODO: finish code to lengthen timeout when FTP server activity is detected
    uint8_t last_ftpsrv_status = 0;
    bool bPowerConnected = Battery::power_connected();
    unsigned long batt_conn_interval_start = millis();

    bool bSettingsUpdated = false;
    
    if (Config::StartServer(lang)) {
      DEBUG_PRT.println(F("Config web server started, listening for requests..."));

      unsigned long finaldelay = FINAL_DELAY_SLOW; // this should give time for the "settings updated" page to download its UI language JSON file (7 seconds max)
      while(bPowerConnected && (!Config::bSettingsUpdated || finaldelay > 0) && !bTimeUp && !Config::bComplete) {
        server.handleClient();  // Web server
        ArduinoOTA.handle(); // PLL-27-04-2020 OTA Update server
        
        ftpsrv_status = ftpSrv.handleFTP(); //PLL-08-04-2021 FTP Server

        if (ftpsrv_status != last_ftpsrv_status) {
          DEBUG_PRT.print(F("FTP server status change: "));
          DEBUG_PRT.print(last_ftpsrv_status);
          DEBUG_PRT.print(F(" to "));
          DEBUG_PRT.print(ftpsrv_status);
          last_ftpsrv_status = ftpsrv_status;
          DEBUG_PRT.print(F(". ftpSrv.isInUse() = "));
          DEBUG_PRT.println(ftpSrv.isInUse());
        }
        
        // run as fast as possible if the FTP server is active, delay 100ms/loop otherwise  
        if (!bSettingsUpdated) {  // only if not in finaldelay timeout (occurs after settings have been updated via the web interface)
          if (ftpSrv.isInUse()) { 
            delay_msec = SERVER_DELAY_FAST; 
            finaldelay = FINAL_DELAY_FAST;
          }
          else {
            delay_msec = SERVER_DELAY_SLOW; 
            finaldelay = FINAL_DELAY_SLOW;
          }
        }
        
        delay(delay_msec);
        wdt_reset();
        //delay(5);
        //DEBUG_PRT.println("Battery voltage is " + String(Battery::battery_voltage()));

        if (Config::bSettingsUpdated) {
          if (!bSettingsUpdated) { // first time entering this, clock will just have been set, so power line enable from clock chip will be off, but still held up by USB 5V input
            Config::SetPowerOn();         // hold up the power enable line. If this is not done, then if USB power is disconnected before reset (but after settings updated), ESP8266 will not restart
            bSettingsUpdated = true;      // if USB power is disconnected after update but before reset, then the alarm should trigger in 3 seconds and switch on the ESP8266 rather than immediately
          }                         
          finaldelay--;
        }

        if (!ftpSrv.isInUse() && millis() > (server_start_time + 1000*8*60)) {
          bTimeUp = true; // run the server for an 10 minutes max, then sleep. If still on usb power, the web server will run again. Will not happen until the FTP server is idle
        }

        if (millis() > batt_conn_interval_start + 1000) {
          batt_conn_interval_start = millis();
          bPowerConnected = Battery::power_connected(); // sample the state of the ADC/power connected sensor every 1 second only, since it interferes with internet connectivity at higher rates
        }
      }

      delay(1000);  // give last file (typically the <lang>.jsn file) time to complete sending before shutting down the webserver

      Config::StopServer();

      if (Config::bSettingsUpdated) {
        DEBUG_PRT.println(F("Settings updated, resetting lectionary..."));
        DEBUGPRT_END
        ESP.deepSleep(1e6); //reboot after 1 second
      }
      else if (bTimeUp) {
        DEBUG_PRT.println(F("Server timed out, stopping web server and displaying reading"));
        //ESP.deepSleep(SLEEP_HOUR - (1000*8*60));
        //SleepForHours(next_wake_hours_count);
        //SleepUntilStartOfHour();
      }
      else // battery power was disconnected
      {
        DEBUG_PRT.println(F("Power disconnected, stopping web server and displaying reading"));
        network.wifi_sleep(); // PLL-29-12-2020 no longer needed, sleep wifi to save power
      }
    }
  }
}
void roundupdatetohour(time64_t& date) {
  tmElements_t ts;
  breakTime(date, ts);
  DEBUG_PRT.printf("Input date = %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

  if (ts.Minute >= 53) {
    date += ((59 - ts.Minute) * 60) + (60 - ts.Second);
  }

  tmElements_t tso;
  breakTime(date, tso);
  DEBUG_PRT.printf("Rounded date = %02d/%02d/%04d %02d:%02d:%02d\n", tso.Day, tso.Month, tmYearToCalendar(tso.Year), tso.Hour, tso.Minute, tso.Second);
}

// Powers down ESP and peripherals for the number of hours, minutes and seconds specified (no rounding), using the DS3231 clock chip alarm,
// or if that fails (because micro is on 5V etc) then deepSleep is used. Sleep duration of up to a year should be possible
void SleepFor(int hours, int minutes, int seconds) {
    //time64_t date;
    //Config::getLocalDateTime(&date);

    time64_t date = now();
    
    tmElements_t ts;
    breakTime(date, ts);

    //ts.Minute = 0;
    //ts.Second = 0; // reset ts to top of current hour

    time64_t waketime = makeTime(ts);
    waketime += ((hours * 3600) + (minutes * 60) + seconds);
    
    DEBUGPRT_END
    Config::PowerOff(waketime);
    
    delay(250);
    //SleepUntilStartOfHour(); // shouldn't happen - should be powered off by this point (use the deepsleep timer as a backup for the DS3231 alarm)  

    DEBUG_PRT.println(F("Failed to shutdown using DS3231, using deepSleep mode"));

    uint32_t deepSleepDurationSeconds = (hours * 3600) + (minutes * 60) + seconds;
    
    if (deepSleepDurationSeconds > MAX_DEEPSLEEP_SECONDS) {
      DEBUG_PRT.println(F("Requested sleep period exceeds maximum deepSleep, setting deepSleep to maximum 1 hour 11 mins 34 sec."));      
      deepSleepDurationSeconds = MAX_DEEPSLEEP_SECONDS;
    }
    else {
      DEBUG_PRT.print(F("Using deepSleep mode, will wake in "));
      DEBUG_PRT.print(String(deepSleepDurationSeconds));
      DEBUG_PRT.println(F(" seconds"));
    }
    
    DEBUGPRT_END
    ESP.deepSleep(deepSleepDurationSeconds * 1e6); // maximum sleep duration using deepSleep is 4294 seconds, or ((1<<32)-1) / 1e6) seconds, or 
                                                   // 1 hr 11 mins and 34 sec (deepSleep timing is in uSeconds).
}

// Powers down ESP and peripherals for the number of hours specified (rounded down to the top of the hour), using the DS3231 clock chip alarm,
// or if that fails (because micro is on 5V etc), then deepSleep is used.
void SleepForHours(int num_hours) {
    //time64_t date;
    //Config::getLocalDateTime(&date);

    time64_t date = now();
    
    tmElements_t ts;
    breakTime(date, ts);

    ts.Minute = 0;
    ts.Second = 0; // reset ts to top of current hour

    time64_t waketime = makeTime(ts);

    waketime += (num_hours * 3600);
    
    DEBUGPRT_END
    Config::PowerOff(waketime);
    
    delay(250);
    SleepUntilStartOfHour(); // shouldn't happen - should be powered off by this point (use the deepsleep timer as a backup for the DS3231 alarm)
}

void SleepUntilStartOfHour() {
    //time64_t date;
    //Config::getLocalDateTime(&date);

    time64_t date = now();
    
    tmElements_t ts;
    breakTime(date, ts);

    int hourskip = 0;

    uint32_t sleepduration_minutes = (60 - ts.Minute); // should wake up at around 10 minutes past the hour (the sleep timer is not terribly accurate!)
    if (sleepduration_minutes <= 7) { // if only a few minutes before the top of the hour, round it up to the next hour and skip the hour plus the difference
      sleepduration_minutes += 60;  // this can occur because the wake timer is inaccurate (+- about 3 minutes per hour). 
      hourskip = 1; // for the debug output, so that the correct hour is output if the current hour is rounded up
    }
    
    DEBUG_PRT.printf("Sleeping %d minutes: Will wake at around %02d:00\n", sleepduration_minutes, (ts.Hour + 1 + hourskip)%24);
    DEBUGPRT_END
    ESP.deepSleep(sleepduration_minutes * 60e6);
    return; // should never return because ESP should be asleep!
}


bool getLectionaryReading(time64_t date, ReadingsFromEnum* r, bool bReturnReadingForAllHours, bool b_OT, bool b_NT, bool b_PS, bool b_G) {
  DEBUG_PRT.printf("getLectionaryReading() bReturnReadingFromAllHours=%s ", bReturnReadingForAllHours?"true":"false");
  //ReadingsFromEnum r;
  tmElements_t tm;
  breakTime(date, tm);

  DEBUG_PRT.printf("hour=%02d\n", tm.Hour);

  bool bHaveLectionaryValue = false;

  if(tm.Day == 24 && tm.Month == 12 && tm.Hour >= 18) { // Christmas Eve Vigil Mass
    DEBUG_PRT.println(F("Christmas Eve vigil Mass"));
    bHaveLectionaryValue = true;
    
    switch(tm.Hour) { // covers hours 18:00 - 23:59
    case 18:
      *r=READINGS_OT;    
      break;
    
    case 19:
      *r=READINGS_NT;    
      break;
      
    case 20:
      *r=READINGS_PS;    
      break;

    case 21:
    case 22:
    case 23:
      *r=READINGS_G;
      break;
          
    default:
      bHaveLectionaryValue = false;
      break;
    }
  } 
  
  if(tm.Day == 25 && tm.Month == 12) { // Christmas Day: Midnight Mass, Mass at Dawn
    DEBUG_PRT.println(F("Christmas Day"));
    bHaveLectionaryValue = true;
    switch(tm.Hour) { // covers hours 00:00 - 07:59. Later hours (Mass during the day) are handled by the last switch statement (used for all other days also).
    case 0: // mass at midnight
    case 4: // mass at dawn
      *r=READINGS_OT;
      break;
      
    case 1: // mass at midnight
    case 5: // mass at dawn
      *r=READINGS_NT;
      break;
      
    case 2: // mass at midnight
    case 6: // mass at dawn
      *r=READINGS_PS;
      break;
      
    case 3: // mass at midnight
    case 7: // mass at dawn
      *r=READINGS_G;
      break;
    
    default:
      bHaveLectionaryValue = false;
      break;
    }
  } 
  
  if (!bHaveLectionaryValue) {
    if (!b_OT || !b_NT) {
      // 3 readings on weekdays of Advent and Lent: OT, PS, G. Will show Gospel reading between midnight and 8am, and 8pm and midnight, and OT between 8am and 2pm, and PS between 2pm and 8pm
      // 3 readings on weekdays of Easter and Christmas: NT, PS, G. Will show Gospel reading between midnight and 8am, and 8pm and midnight, and NT between 8am and 2pm, and PS between 2pm and 8pm
      bHaveLectionaryValue = true;
      if (!bReturnReadingForAllHours) {
        switch(tm.Hour) {
        case 8:
          *r=READINGS_G;

          if (!b_OT) {
            *r=READINGS_NT;
          }
          
          if (!b_NT) {
            *r=READINGS_OT; // one or other of b_OT, b_NT should be true. Defaults to G if both are false.
          }
          break;
          
        case 14:
          *r=READINGS_PS;
          break;
        
        case 0:
        case 20:
          *r=READINGS_G;
          break;
        
        default:
          bHaveLectionaryValue = false;
          break;
        }
      }
      else {
        switch(tm.Hour) {
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
          *r=READINGS_G;

          if (!b_OT) {
            *r=READINGS_NT;
          }
          
          if (!b_NT) {
            *r=READINGS_OT; // one or other of b_OT, b_NT should be true. Defaults to G if both are false.
          }
          break;
          
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
          *r=READINGS_PS;
          break;
    
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 20:
        case 21:
        case 22:
        case 23:
          *r=READINGS_G;
          break;
        
        default:
          bHaveLectionaryValue = false; // shouldn't happen
          break;
        }      
      }
    }

    // this will show the Gospel reading between the hours of midnight and 8am, and 8pm and midnight
    if (!bHaveLectionaryValue && b_OT && b_NT) {
      bHaveLectionaryValue = true;
      if (!bReturnReadingForAllHours) {
        switch(tm.Hour) {
        case 8:
          *r=READINGS_OT;
          break;
          
        case 12:
          *r=READINGS_NT;
          break;
    
        case 16:
          *r=READINGS_PS;
          break;
    
        case 0:
        case 20:
          *r=READINGS_G;
          break;
        
        default:
          bHaveLectionaryValue = false;
          break;
        }
      }
      else {
        switch(tm.Hour) {
        case 8:
        case 9:
        case 10:
        case 11:
          *r=READINGS_OT;
          break;
          
        case 12:
        case 13:
        case 14:
        case 15:
          *r=READINGS_NT;
          break;
    
        case 16:
        case 17:
        case 18:
        case 19:
          *r=READINGS_PS;
          break;
    
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 20:
        case 21:
        case 22:
        case 23:
          *r=READINGS_G;
          break;
        
        default:
          bHaveLectionaryValue = false; // shouldn't happen
          break;
        }      
      }
    }
  }
  return bHaveLectionaryValue; // will be false when on battery power, if the hour is not the specific hour that an update is to occur. On battery, updates are repeated every hour
}



//---------------------------------------------------------------------------calendar

bool display_calendar(String date, Calendar* c, String refs, bool right_to_left, bool verse_per_line, bool show_verse_numbers) {   
  DEBUG_PRT.print(F("Selected font is "));
  DEBUG_PRT.println(c->_I18n->configparams.font_filename);
  
  if (diskfont.begin(c->_I18n->configparams)) {
    DEBUG_PRT.println(F("Font opened successfully"));
  }
  else {
    display_image(font_missing, c->_I18n->configparams.font_filename, false);
    DEBUG_PRT.println(F("Font not found, using internal font"));    
  }
    
  bool bRed = (c->temporale->getColour() == Enums::COLOURS_RED) ? true : false;
  
  DEBUG_PRT.println(F("Displaying calendar"));

#ifdef USE_SPI_RAM_FRAMEBUFFER
  fb.cls();   // clear framebuffer ready for text to be drawn
  fb.setRotation(EPD_ROTATION); //90 degrees
  int charheight = diskfont._FontHeader.charheight;
  fb.drawFastHLine(0, charheight, fb.width(), GxEPD_BLACK);


  //fb.drawLine(0,0, fb.width(), fb.height(), GxEPD_BLACK);
  // test pattern
/*  
  for(int16_t i=0; i<8; i++){
    int16_t x=10 + (i * 16);
    int16_t y=150;
    uint16_t color_bw = fb.makeColor(i, false);
    uint16_t color_red = fb.makeColor(i, true);
    
    fb.fillRect(x, y, 16, 16, color_bw);
    fb.fillRect(x, y + 20, 16, 16, color_red);
  }
*/
//  updateDisplay(display_reading);
//  return true;
#else
    ePaper.setRotation(EPD_ROTATION);
#endif

//// PLL 07-07-2020 Test for Latin Mass propers (comment out in production)
//    #ifdef USE_SPI_RAM_FRAMEBUFFER
//      int fbwidth = fb.width();
//      int fbheight = fb.height();
//    #else
//      int fbwidth = ePaper.width();
//      int fbheight = ePaper.height();
//    #endif
//    DEBUG_PRT.print(F("fbwidth x fbheight = "));
//    DEBUG_PRT.print(fbwidth);
//    DEBUG_PRT.print(F(" x "));
//    DEBUG_PRT.println(fbheight);
//
//  //PLL-25-07-2020 test harness for Latin Mass Propers
//    //Bidi::TestHarnessLa(tb, fbwidth, fbheight, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
//    Bidi::TestHarnessLa2(tb, fbwidth, fbheight, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
//    //DEBUG_PRT.print(F("Bidi::TestHarnessLa returned, updating display.."));
//    updateDisplay(display_reading);
//    DEBUG_PRT.println(F("OK"));
//    return true;
//  //PLL-25-07-2020 testharness for Latin Mass Propers
  
  DEBUG_PRT.println(F("Displaying verses"));
//  //refs = "Ps 85:9ab+10, 11-12, 13-14"; //debugging
//  //refs="1 Chr 29:9-10bc";              //debugging
//  refs="John 3:16"; // debugging
//  refs="2 John 1:1"; // debugging

  if (!display_verses(c, refs, right_to_left, verse_per_line, show_verse_numbers)) {
    DEBUG_PRT.println(F("display_verses returned false"));
    return false;
  }


  if (c->day.is_sanctorale) {
    String sanctorale_day = c->day.sanctorale;
    if (c->day.is_holy_day_of_obligation) {
      sanctorale_day = sanctorale_day + " " + c->day.holy_day_of_obligation;
    }
    
    display_day(sanctorale_day, bRed, right_to_left); // sanctorale in struct day is the feast day, displayed at the top of the screen on feast days
    if (c->day.sanctorale == c->day.day) {                                     // otherwise the liturgical day is shown at the top of the screen.
      //display_date(date, "", &paint, font, &diskfont);                       // If it is a feast day, the liturgical day is displayed at the bottom left. Otherwise the bottom left
    } else {                                                                   // is left blank.
      display_date(date, c->day.day, right_to_left); // "day" in struct day is the liturgical day
    }
  } else {
    String liturgical_day = c->day.day;
    if (c->day.is_holy_day_of_obligation) {
      liturgical_day = liturgical_day + " " + c->day.holy_day_of_obligation;
    }
    display_day(liturgical_day, bRed, right_to_left);    
    display_date(date, "", right_to_left);
  }

//  DEBUG_PRT.println("Displaying verses");
////  //refs = "Ps 85:9ab+10, 11-12, 13-14"; //debugging
////  //refs="1 Chr 29:9-10bc";              //debugging
////  refs="John 3:16"; // debugging
////  refs="2 John 1:1"; // debugging
  
  updateDisplay(display_reading, diskfont._FontHeader.antialias_level);
    
  uint32_t free = system_get_free_heap_size();
  DEBUG_PRT.print(F("free memory = "));
  DEBUG_PRT.println(String(free));
  DEBUG_PRT.println(F("done")); 

  return true;
}

void display_day(String d, bool bRed, bool right_to_left) {
  DEBUG_PRT.print(F("display_day() d="));
  DEBUG_PRT.println(String(d));

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int fbwidth = fb.width();
  int fbheight = fb.height();
#else
  int fbwidth = ePaper.width();
  int fbheight = ePaper.height();
#endif
  
  int text_xpos = (fbwidth / 2) - (int)((diskfont.GetTextWidthA(d, true))/2); // true => shape text before calculating width
  //DEBUG_PRT.println("display_day() text_xpos = " + String(text_xpos));
  
  int text_ypos = 0;
  
  Bidi::RenderText(d, &text_xpos, &text_ypos, tb, diskfont, &bRed, fbwidth, fbheight, right_to_left, false);

  //int charheight = diskfont->_FontHeader.charheight;  
  //paint->DrawLine(0, charheight, 264, charheight, COLORED);
}

void display_date(String date, String day, bool right_to_left) {
  DEBUG_PRT.print(F("\ndisplay_date: s="));
  DEBUG_PRT.println(date);

  bool bEmphasisOn = false;

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int fbwidth = fb.width();
  int fbheight = fb.height();
#else
  int fbwidth = ePaper.width();
  int fbheight = ePaper.height();
#endif

  int text_xpos = 0;
  int text_ypos = 0;

  text_xpos = fbwidth - (int)(diskfont.GetTextWidthA(date, true)); // right justified, shape text before calculating width
  text_ypos = fbheight - diskfont._FontHeader.charheight;
  Bidi::RenderText(date, &text_xpos, &text_ypos, tb, diskfont, &bEmphasisOn, fbwidth, fbheight, right_to_left, false);

  text_xpos = 0;
  text_ypos = fbheight - diskfont._FontHeader.charheight;
  Bidi::RenderText(day, &text_xpos, &text_ypos, tb, diskfont, &bEmphasisOn, fbwidth, fbheight, right_to_left, false);
}


//#define FORMAT_EMPHASIS_ON String("on")
//#define FORMAT_EMPHASIS_OFF String("off")
//#define FORMAT_DEFAULT String("") // keep whatever formatting is currently selected
//#define FORMAT_LINEBREAK String("br")

//bool bVersePerLine = true;
//bool bShowVerseNumbers = true;

bool display_verses(Calendar* calendar, String refs, bool right_to_left, bool bVersePerLine, bool bShowVerseNumbers) {
//  bool right_to_left = c->_I18n->configparams.right_to_left;

  DEBUG_PRT.print(F("refs from lectionary: "));
  DEBUG_PRT.println(refs);
  
  Bible b(calendar->_I18n);
  if (!b.get(refs)) {
    DEBUG_PRT.println(F("Couldn't get refs (no Apocrypha?)"));
    return false;
  }
    
  b.dump_refs();

  Ref* r;
  int i = 0;

  r = b.refsList.get(i++);

  bool bEndOfScreen = false;

  int xpos = 0;
  int ypos = diskfont._FontHeader.charheight;

  String line_above = "";
  bool bDisplayRefs = true;

  bool bGotVerses = false; // I found that Psalm 85:14 is missing from the NJB, but not the French version, because in the English version verse 14 is included in verse 13.
                           // If something was found, this flag will be set, so only if nothing was found will the function return false.
//  bool bShowChapter = false;

  while (r != NULL && !bEndOfScreen) {     
    int start_chapter = r->start_chapter;
    int end_chapter = r->end_chapter;
    int start_verse = 0;
    int end_verse = 0;
    String sentence_range = "";
    String verse_record = "";
    String verse_text = "";
    String output = "";
                           
    for (int c = start_chapter; c <= end_chapter; c++) {
      DEBUG_PRT.printf("\n%d:", c);
      if (c < end_chapter) {
        end_verse = b.books_chaptercounts[r->book_index]; // -1 -> output until last verse
      }
      else {
        end_verse = r->end_verse;
      }

      if (c > start_chapter) {
        start_verse = 1;
      }
      else {
        start_verse = r->start_verse;
      }

      int v = start_verse;
      bool bDone = false;

      String book_name;

      //String format_state = FORMAT_DEFAULT;
      bool bEmphasis_On = false;
    
      while (!bDone && !bEndOfScreen) {
        int numRecords = 0;
        sentence_range = "";
        if (v == r->start_verse && c == r->start_chapter) {
          sentence_range = r->start_verse_sentence_range;
        } 
        else if (v == r->end_verse && c == r->end_chapter) {
          sentence_range = r->end_verse_sentence_range;
        }
            
        if (b._bibleverse->get(r->book_index, c, v, &verse_record, &numRecords)) {
          bGotVerses = true;
          
          DEBUG_PRT.printf(" %d ", v);
          DEBUG_PRT.printf("sentence_range=%s, numRecords=%d\n", sentence_range == "" ? "[All]" : sentence_range.c_str(), numRecords);
          verse_text = get_verse(verse_record, &book_name, sentence_range, numRecords);

          if (bShowVerseNumbers) {
            verse_text = "<i>" + SuperScriptNumber(v) + "</i>" + verse_text;
          }

          if (bDisplayRefs) {
            String refs_i18n = refs;
            int booknum = -1;
            int refsindex = 0;
            while(b.getUniqueBookInRefs(booknum, refsindex)){
              String bookname_I18n = b._bibleverse->getBookNameFromBible(booknum);
              if (bookname_I18n != "") refs_i18n.replace(b.getBookShortName(booknum), bookname_I18n);
            }

            String ortext = calendar->_I18n->get("or");
            if(ortext != "or" && ortext != "") {
              ortext = " " + ortext + " ";
              refs_i18n.replace(" or ", ortext);
            }
            
            //if (book_name != "") refs_i18n.replace(b.books_shortnames[r->book_index], book_name);
            refs_i18n = "<i>" + refs_i18n + "</i>";
            epd_verse(refs_i18n, &xpos, &ypos, &bEmphasis_On, right_to_left, true);

            DEBUG_PRT.printf("refs_i18n = %s\n", refs_i18n.c_str());
            bDisplayRefs = false;
            verse_text = " " + verse_text;
          }

          if (bVersePerLine) {
            verse_text = verse_text + "<br>";
          }
          else {
            if (!(verse_text.endsWith(" "))) {
              verse_text = verse_text + " ";
            }
          }

          bEndOfScreen = epd_verse(verse_text, &xpos, &ypos, &bEmphasis_On, right_to_left, (v != end_verse)); // returns false if at end of screen
          DEBUG_PRT.println("epd_verse returned " + String(bEndOfScreen ? "true":"false"));
          DEBUG_PRT.println();
          v++;
          if (v > end_verse) bDone = true; // end_verse will be set to -1 if all verses up to the end of the chapter are to be returned.
        }
        else {
          DEBUG_PRT.println(F("Verse is missing from this Bible (variation in Psalms?)\n"));
          //return false;
          //bDone = true;
          v++;
          if (v > end_verse) bDone = true;
        }
      }
      if (bEndOfScreen) break; // out of chapter for loop
    }
    r = b.refsList.get(i++);
  }

  return bGotVerses;
}

bool epd_verse(String verse, int* xpos, int* ypos, bool* bEmphasis_On, bool right_to_left, bool bMoreText) {
  DEBUG_PRT.print(F("epd_verse() verse="));
  DEBUG_PRT.println(verse);

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int fbwidth = fb.width();
  int fbheight = fb.height();
#else
  int fbwidth = ePaper.width();
  int fbheight = ePaper.height();
#endif
  
//  Bidi bidi;

  fbheight = fbheight - diskfont._FontHeader.charheight;
  return Bidi::RenderText(verse, xpos, ypos, tb, diskfont, bEmphasis_On, fbwidth, fbheight, right_to_left, true, bMoreText);    

  return true;
}

String get_verse(String verse_record, String* book_name, String sentence_range, int numRecords) { // a bit naughty, verse_record strings can contain multiple lines of csv records for verses that span more than one line.
  DEBUG_PRT.printf("get_verse() %s", verse_record.c_str());
  DEBUG_PRT.printf("get_verse() sentence_range=%s, numRecords=%d\n", sentence_range == "" ? "[All]" : sentence_range.c_str(), numRecords);
  Csv csv;

  int pos = 0;

  String fragment = "";
  //int sentence_number = 1;
  int sentence_count = 0;
  String verse = "";
  bool more_than_one = false;

  String verse_fragment_array[26];
  int verse_fragment_array_index = 0;

  for (int i = 0; i<26; i++) {
    verse_fragment_array[i] = ""; // intialize array
  }

  DEBUG_PRT.println(F("get_verse() *1*"));      
  do {
    int i = 0;
    do {
      DEBUG_PRT.println("pos = " + String(pos));
      fragment = csv.getCsvField(verse_record, &pos);
      DEBUG_PRT.printf("fragment=%s\n", fragment.c_str());
      
      if (i == 0 && !more_than_one) {
        *book_name = fragment;
      }

      if (i == 4) {
        DEBUG_PRT.printf("fragment=%s\n", fragment.c_str());
        verse += ((more_than_one?" ":"") + fragment);
        verse_fragment_array[verse_fragment_array_index++] = ((more_than_one?" ":"") + fragment);
        if (verse_fragment_array_index == 26) return ("verse fragment array index out of range");
      }
    } while (pos < verse_record.length() && i++ != 4);
    //DEBUG_PRT.println("pos = " + String(pos) + " charAt pos = [" + String(verse_record.charAt(pos)) + "]");
    more_than_one = true;
    sentence_count++;
  } while (pos < verse_record.length());

  DEBUG_PRT.println(F("get_verse() *2*"));      

  String letters = "abcdefghijklmnopqrstuvwxyz";

  int max_fragment_number = 0;
  for (unsigned int i = 0; i < sentence_range.length(); i++) {
    int fragment_number = letters.indexOf(sentence_range.charAt(i));

    if (fragment_number > max_fragment_number) max_fragment_number = fragment_number;
  }
  DEBUG_PRT.println(F("get_verse() *3*"));      

  if (sentence_range == "" || max_fragment_number > (numRecords - 1)) {
    DEBUG_PRT.println(F("Sentence range is empty or max fragment number > numRecords: returning whole verse"));
    return verse;  
  }
  else {
    DEBUG_PRT.println(F("parsing subrange reference"));
    unsigned int charpos = 0;
    int fragment_number = 0;
    String output = "";
    
    char c = sentence_range.charAt(charpos);
    if (c == '-') {
      charpos++;

      if (charpos >= sentence_range.length()) return verse; // return whole verse if only the - is present (malformed subrange)
      
      fragment_number = letters.indexOf(sentence_range.charAt(charpos));

      DEBUG_PRT.printf("adding all fragments up to %d\n", fragment_number);

      if (fragment_number == -1) return verse; // next character is not a letter - return whole verse (malformed subrange)

      for (int i = 0; i <= fragment_number; i++) { // if the subrange starts with a '-', add all verse fragments up to the first lettered verse fragment
        output += verse_fragment_array[i];
      }
      charpos++;
    }

    DEBUG_PRT.println(F("get_verse() *4*"));      
  
    c = sentence_range.charAt(charpos);
    while (c != '-' && charpos < sentence_range.length()) {
      fragment_number = letters.indexOf(sentence_range.charAt(charpos));
      DEBUG_PRT.printf("adding fragment %d(%c)\n", fragment_number, c);
      output += verse_fragment_array[fragment_number];      
      charpos++;
      c = sentence_range.charAt(charpos);
    }

    DEBUG_PRT.println(F("get_verse() *5*"));      

    if (c == '-') {
      DEBUG_PRT.printf("adding fragments at end from %d to %d\n", fragment_number + 1, verse_fragment_array_index - 1);
      for (int i = fragment_number + 1; i < verse_fragment_array_index; i++) {
        output += verse_fragment_array[i];
      }
    }

    DEBUG_PRT.println(F("get_verse() *6*"));      

    return output;
  }

  DEBUG_PRT.println(F("get_verse() *7*"));      

  return verse;
}

// EDB SD card reader/writer functions
void writer (unsigned long address, const byte* data, unsigned int recsize) {
    dbFile.seek(address);
    dbFile.write(data,recsize);
    dbFile.flush();
}

void reader (unsigned long address, byte* data, unsigned int recsize) {
    dbFile.seek(address);
    dbFile.read(data,recsize);
}

void printDbError(EDB_Status err)
{
    DEBUG_PRT.print(F("ERROR: "));
    switch (err)
    {
        case EDB_OUT_OF_RANGE:
            DEBUG_PRT.println(F("Recno out of range"));
            break;
        case EDB_TABLE_FULL:
            DEBUG_PRT.println(F("Table full"));
            break;
        case EDB_OK:
        default:
            DEBUG_PRT.println(F("OK"));
            break;
    }
}

uint8_t Mass_Type_From_EF_Path(String& ef_path)
{
  uint8_t MassType = MASS_1960;
  if (ef_path.indexOf("1570") != -1) { MassType = MASS_TRIDENTINE_1570; }
  if (ef_path.indexOf("1910") != -1) { MassType = MASS_TRIDENTINE_1910; }
  if (ef_path.indexOf("Affla") != -1) { MassType = MASS_DIVINEAFFLATU; }
  if (ef_path.indexOf("1955") != -1) { MassType = MASS_1955; }
  if (ef_path.indexOf("1960New") != -1) { MassType = MASS_1960NEW; }
  if (ef_path.indexOf("1965-67") != -1) { MassType = MASS_1965_67; }

  return MassType;
}

bool DoLatinMassPropers(time64_t date, String datestring, bool right_to_left, String lectionary_path, String lang, int8_t& waketime) {
  DEBUG_PRT.println(F("Displaying calendar"));

#ifdef USE_SPI_RAM_FRAMEBUFFER
#ifndef LM_DEBUG
  fb.cls();   // clear framebuffer ready for text to be drawn
#endif
  fb.setRotation(EPD_ROTATION); //90 degrees
#else
  ePaper.setRotation(EPD_ROTATION);
#endif

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int fbwidth = fb.width();
  int fbheight = fb.height();
#else
  int fbwidth = ePaper.width();
  int fbheight = ePaper.height();
#endif
  DEBUG_PRT.print(F("fbwidth x fbheight = "));
  DEBUG_PRT.print(fbwidth);
  DEBUG_PRT.print(F(" x "));
  DEBUG_PRT.println(fbheight);

  uint8_t mass_type = Mass_Type_From_EF_Path(lectionary_path);
  Tridentine::mass_type = mass_type;  // affects HolyFamily date - see Tridentine.cpp
  
/*
  uint8_t MassType = MASS_1960;
  if (lectionary_path.indexOf("1570") != -1) { MassType = MASS_TRIDENTINE_1570; }
  if (lectionary_path.indexOf("1910") != -1) { MassType = MASS_TRIDENTINE_1910; }
  if (lectionary_path.indexOf("Affla") != -1) { MassType = MASS_DIVINEAFFLATU; }
  if (lectionary_path.indexOf("1955") != -1) { MassType = MASS_1955; }
*/  
  //Yml::SetConfig(lang, "en-1962.yml", "en-1962.txt"); //removed - using Tridentine::GetFileDir function instead - PLL 19-10-2020
  Tr_Calendar_Day td;
  //Tridentine::get(date, td, true);
  Tridentine::GetFileDir2(date, td.FileDir_Season, td.FileDir_Saint, td.FileDir_Votive, td.HolyDayOfObligation, td.SeasonImageFilename, td.SaintsImageFilename, td.VotiveImageFilename, mass_type);

  LatinMassPropers(date, datestring, td, tb, lectionary_path, lang, fbwidth, fbheight, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, right_to_left, waketime, mass_type);

#ifndef _WIN32
#ifndef LM_DEBUG
  updateDisplay(display_reading, diskfont_normal._FontHeader.antialias_level);
  DEBUG_PRT.println(F("OK"));
#endif
#endif
  return true;
}

void LatinMassPropers(time64_t& date,
  String datestring,
  Tr_Calendar_Day& td,
  TextBuffer& tb,
  String lect, String lang,
  int fbwidth, int fbheight,
  DiskFont& diskfont_normal,
  DiskFont& diskfont_i,
  DiskFont& diskfont_plus1_bi,
  DiskFont& diskfont_plus2_bi,
  bool right_to_left,
  int8_t& waketime,
  uint8_t mass_type) 
{

  // setup fonts and variables for text output

  waketime = -1; // tell caller to use default waketime

  bool bBold = false;
  bool bItalic = false;
  bool bRed = false;
  int8_t fontsize_rel = 0;

  DEBUG_PRT.print(F("Done"));
  DEBUG_PRT.print(F("Loading fonts.."));
  if (!diskfont_normal.available) diskfont_normal.begin("droid11.lft");
  if (!diskfont_i.available) diskfont_i.begin("droid11i.lft");
  if (!diskfont_plus1_bi.available) diskfont_plus1_bi.begin("droi12bi.lft");
  if (!diskfont_plus2_bi.available) diskfont_plus2_bi.begin("droi13bi.lft");
  DEBUG_PRT.println(F("Done"));
  DiskFont* pDiskfont = &diskfont_normal;
  DEBUG_PRT.println(F("Assigned font to diskfont"));

  int xpos = 0;
  int ypos = pDiskfont->_FontHeader.charheight;
  fbheight = fbheight - pDiskfont->_FontHeader.charheight;

  int8_t line_number = 0;
  bool bRenderRtl = right_to_left;
  bool bWrapText = true;
  bool bMoreText = false;

  Bidi::tageffect_init();
  Bidi::tageffect_reset(bBold, bItalic, bRed, fontsize_rel);
  DEBUG_PRT.println(F("Set default tag effects"));
  pDiskfont = Bidi::SelectDiskFont(bBold, bItalic, fontsize_rel, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);
  DEBUG_PRT.print(F("Selected default diskfont"));

  bool bLiturgical_Colour_Red = false;

  // text output

  /*
   * PLL-19-10-2020 test code removed
  String mass_version = "1955";
  //String lang = "en";
  Yml::SetConfig(lang, "en-1962.yml", "en-1962.txt");
  */
  // PLL-05-04-2021 Fix for Liturgical class comparison function which was only partially working
  //bool bUseNewClasses = (lect.indexOf("1960") > -1); // Use new table for assessing Liturgical class priority for versions 1960 and 1960New
  bool bUseNewClasses = (mass_type >= MASS_1960); // Use new table for assessing Liturgical class priority for versions 1960 and 1960New

  tmElements_t ts;
  breakTime(date, ts);

#ifdef __AVR__
  DEBUG_PRT.printf("\nDatetime: %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
#else
  printf("\nDatetime: %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
#endif
  //DEBUG_PRT.println(td.DayofWeek);
  //DEBUG_PRT.println(td.Cls);
  //DEBUG_PRT.println(td.Colour);
  //DEBUG_PRT.println(td.Mass);
  //DEBUG_PRT.println(td.Commemoration);
  DEBUG_PRT.print(F("td.HolyDayOfObligation=["));
  DEBUG_PRT.print(td.HolyDayOfObligation);
  DEBUG_PRT.print(F("]\ntd.FileDir_Season=["));
  DEBUG_PRT.print(td.FileDir_Season);
  DEBUG_PRT.print(F("]\ntd.FileDir_Saint=["));
  DEBUG_PRT.print(td.FileDir_Saint);
  DEBUG_PRT.print(F("]\ntd.FileDir_Votive=["));
  DEBUG_PRT.print(td.FileDir_Votive);
  DEBUG_PRT.print(F("]\ntd.SeasonImageFilename=["));
  DEBUG_PRT.print(td.SeasonImageFilename);
  DEBUG_PRT.print(F("]\ntd.SaintsImageFilename=["));
  DEBUG_PRT.print(td.SaintsImageFilename);
  DEBUG_PRT.print(F("]\ntd.VotiveImageFilename=["));
  DEBUG_PRT.print(td.VotiveImageFilename);
  DEBUG_PRT.println(F("]"));

  int8_t filenumber = ts.Hour - 8;    // (Done) - TODO: need to have special cases for Easter, All Souls and Christmas (which have a larger number of parts/different structure to standard 12-part propers)

  if (ts.Hour >= 0 && ts.Hour < 9) {
    filenumber = 0;
  }

  if (ts.Hour >= 19) {
    filenumber = 11;
  }

  int8_t next_hour_filenumber = filenumber + 1;

  //String fileroot = "/Lect/EF/" + mass_version + "/" + lang; // test code removed
  bool b_is_1570_Mass = (lect.indexOf("1570") > -1);

  String fileroot = "/" + lect + "/" + lang;
  String fileroot_img = "/" + lect + "/images";
  String liturgical_day = "";
  String sanctoral_day = "";
  String seasonimagefilename = td.SeasonImageFilename != "" ? fileroot_img + td.SeasonImageFilename + ".bwr" : "";
  String saintsimagefilename = td.SaintsImageFilename != "" ? fileroot_img + td.SaintsImageFilename + ".bwr" : "";
  String votiveimagefilename = td.VotiveImageFilename != "" ? fileroot_img + td.VotiveImageFilename + ".bwr" : "";

  bool bFeastDayOnly = (td.FileDir_Season == td.FileDir_Saint); // will be true if there is no seasonal day
  bool bIsFeast = (td.FileDir_Saint != "" && SD.exists(fileroot + td.FileDir_Saint)); // if there is no feast day on this day, the directory/propers for this day will not exist
  bool bIsVotive = (td.FileDir_Votive != "" && SD.exists(fileroot + td.FileDir_Votive)); // if there is no votive day on this day, the directory/propers for this day will not exist
  bool bHasSeasonImage = (td.SeasonImageFilename != "" && SD.exists(seasonimagefilename)); // if there is an image accompanying the Saint's day on this day, it will be displayed between midnight and 8am, before the introit at 9am
  bool bHasSaintsImage = (td.SaintsImageFilename != "" && SD.exists(saintsimagefilename)); // if there is an image accompanying the Saint's day on this day, it will be displayed between midnight and 8am, before the introit at 9am
  bool bHasVotiveImage = (td.VotiveImageFilename != "" && SD.exists(votiveimagefilename)); // if the Mass for the day is a Votive Mass, an image will be displayed if available
  int8_t seasonimagecount = -1;
  int8_t saintsimagecount = -1;
  int8_t votiveimagecount = -1;

  // image filenames are of the form 10-11.bwr, and 10-11-2.bwr, 10-11-3.bwr etc if there is more than one image
  if (bHasSeasonImage) seasonimagecount = GetImageCount(fileroot_img + td.SeasonImageFilename); // get count of images to be displayed this day
  if (bHasSaintsImage) saintsimagecount = GetImageCount(fileroot_img + td.SaintsImageFilename); // get count of images to be displayed this day
  if (bHasVotiveImage) votiveimagecount = GetImageCount(fileroot_img + td.VotiveImageFilename); // get count of votive images to be displayed this day (if applicable)

  int8_t imagecount = saintsimagecount; // is assigned either seasonimagecount or saintsimagecount as appropriate for the day, feast and season
  bool bHasImage = bHasSaintsImage;
  String imagefilename = saintsimagefilename;

  DEBUG_PRT.print(F("seasonimagefilename is ["));
  DEBUG_PRT.print(seasonimagefilename);
  DEBUG_PRT.print(F("]\nsaintsimagefilename is ["));
  DEBUG_PRT.print(saintsimagefilename);
  DEBUG_PRT.print(F("]\nvotiveimagefilename is ["));
  DEBUG_PRT.print(votiveimagefilename);
  DEBUG_PRT.print(F("]\nbHasSeasonImage is "));
  DEBUG_PRT.println(bHasSeasonImage);
  DEBUG_PRT.print(F("bHasSaintsImage is "));
  DEBUG_PRT.println(bHasSaintsImage);
  DEBUG_PRT.print(F("bHasVotiveImage is "));
  DEBUG_PRT.println(bHasVotiveImage);
  DEBUG_PRT.print(F("seasonimagecount="));
  DEBUG_PRT.println(seasonimagecount);
  DEBUG_PRT.print(F("saintsimagecount="));
  DEBUG_PRT.println(saintsimagecount);
  DEBUG_PRT.print(F("votiveimagecount="));
  DEBUG_PRT.println(votiveimagecount);

  MissalReading season;
  MissalReading feast;
  MissalReading votive;

  if (bIsFeast) {
    feast.open(fileroot + td.FileDir_Saint);
  }

  if (bIsVotive) {
    votive.open(fileroot + td.FileDir_Votive);
  }

  if (!bFeastDayOnly) {
    season.open(fileroot + td.FileDir_Season);
  }

  bool bSunday = Tridentine::sunday(date);
/*
  int8_t cls_season = Tridentine::getClassIndex(season.cls(), bUseNewClasses);
  int8_t cls_feast = Tridentine::getClassIndex(feast.cls(), bUseNewClasses);
  
  DEBUG_PRT.print(F("cls_season="));
  DEBUG_PRT.print(cls_season);
  DEBUG_PRT.print(F(" cls_feast="));
  DEBUG_PRT.println(cls_feast);
*/
  Ordering ordering;
  //DEBUG_PRT.on();
  Precedence::doOrdering(date, mass_type, season, feast, votive, ordering);
  //DEBUG_PRT.off();

  bFeastDayOnly = (ordering.ordering[0] == MR_FEAST && ordering.ordering[1] == MR_NONE);
  bIsFeast = (bFeastDayOnly || ordering.ordering[1] != MR_NONE);  // ordering[0]=index into heading[] for object to be used for heading (season=0, feast=1 or votive=2), ordering[1]=index into heading[] for object to be used for subheading (0, 1, or 2 as before if applicable. -1 if no subheading)

  bool b_is_purification_of_mary = (ts.Month == 2 && ts.Day == 2) && ordering.ordering[0] == MR_FEAST;  // first item is the feast, it is not the 12 part one that D.O. outputs for the non-1570 Latin version of the Purification of Mary (bug in D.O.?), because it has 14 parts not 12
                                  
  /*
  // need to exclude commemoration on Sundays and Feasts of the Lord
  bIsFeast = (bFeastDayOnly ||
    (bIsFeast && (                                        // PLL-12-04-2021 Can only be a feast day if bIsFeast is already true, indicating that there is a 
    (!bSunday && cls_feast >= cls_season) ||              //  calendar entry for a feast on this day in the database
      (cls_feast >= 6) ||                                   // the additions in the commented three lines below may have introduced bugs - need testing  
      (!bSunday && cls_feast == 0 && cls_season <= 2) ||    // PLL-12-01-2021 commemoration is 0, need to display these eg. on 5 Dec 2020, which is the commemoration of S. Sabbae Abbatis. To display on Seasonal days <= Semiduplex
      (cls_season - cls_feast == 1) ||                      // PLL-12-01-2021 display feast if it is 1 class lower than that of the season
      (bSunday && cls_feast > cls_season && cls_feast >= 5) // PLL-12-01-2021 Patched for Feast of St Luke Evangelist on October 18 (Duplex II. classis) if on a Sunday
      )));
  
  bool bIsImmaculateConception = Tridentine::IsImmaculateConception(date);
  bool bIsLadyDay = Tridentine::IsLadyDay(date);  // shouldn't both be true!
  */
  //bFeastDayOnly = (bFeastDayOnly || cls_feast >= 6 && !bIsImmaculateConception && !bIsLadyDay /*Tridentine::Season(date) != SEASON_ADVENT*/); // If Feast of the Lord, Class I or Duplex I classis, don't show seasonal day
  
  
  // Feast of Immaculate Conception (8 Dec, even if Sunday), the day of Advent is the Commemorio (there may be other exceptions)
  // PLL-26-12-2020 Added bFeastDayOnly || check so that if the test Saints Day == the Feast Day (above) then this overrides the rest of this test

  // If it is a votive day and the feast is of the same or lower priority, 
  // or if it is a seasonal day and the seasonal day is of the same or lower priority, the votive mass is observed
  bIsVotive = ordering.b_is_votive; //(ordering.ordering[0] == MR_VOTIVE); // votive=2
  /*
  bIsVotive = (bIsVotive && ((bIsFeast && Tridentine::getClassIndex(feast.cls(), bUseNewClasses) <= Tridentine::getClassIndex(votive.cls(), bUseNewClasses))
    || (!bIsFeast && Tridentine::getClassIndex(season.cls(), bUseNewClasses) <= Tridentine::getClassIndex(votive.cls(), bUseNewClasses)))
    );
  */
  if (bIsVotive) {
    DEBUG_PRT.println(F("Votive Mass"));
  }

  // Now have the indexrecords for the season and saint (if also a feast), and the filepointers pointing to the start of the text.

    //if (Tridentine::getClassIndex(indexheader_saint.cls, bUseNewClasses) > Tridentine::getClassIndex(indexheader_season.cls, bUseNewClasses)) {
    // Feast day takes precendence
    /*
      In this case:
      0 Introitus   - Comes from the feast
      1 Gloria    - Omit if season requires
      2 Collect     - Seasonal day becomes commemoration (after the Collect text for the Feast day:
            "Commemoratio <indexheader_season.name> followed by the Collect text from the season.
      3, 4, 5 Lesson, Gradual, Gospel - Comes from the Feast
      6 Credo     - Omit if season requires
      7 Offertorium   - Comes from the Feast
      8 Secreta     - Like collect (both)
      9 Prefatio    - From the Season
      10 Communio   - From the Feast
      11 Postcommunio - From both
    */
    //}

  bool bFileOk = false;
  String s = "";
  bool bOverflowedScreen = true;
  int8_t subpart = 0;
  bool bRTL = right_to_left;

  bool bResetPropersFilePtr = false;

  if (bIsFeast) { // there is a feast on this day
    if (bFeastDayOnly) { // is a saint's feast only
      DEBUG_PRT.println(F("Feast day only"));
      ypos = display_day_ex(date, feast.name(), feast.colour(), td.HolyDayOfObligation, right_to_left, bLiturgical_Colour_Red, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);   // feast day is displayed at the top of the screen on feast days otherwise the liturgical day is 
      bool bImageIsDisplayed = false;

      if (b_is_purification_of_mary) {
        // purification of Mary propers
        waketime = DoPurificationOfMary(&feast, NULL, false,
          filenumber, ts.Hour, lang, mass_type,
          xpos, ypos,
          pDiskfont,
          bBold, bItalic, bRed,
          fontsize_rel, line_number,
          fbwidth, fbheight,
          bRTL, bRenderRtl, bWrapText, bHasImage, imagefilename);
      }
      else {
        if (bHasImage) {
          if (ts.Hour == 19) {
            waketime = 20; // at 7pm, if there are any images to display, wake at 8pm to begin displaying them
          }
          else if (ts.Hour >= 0 && ts.Hour < 8) { // will display image between midnight and 8am, so need to wake at 8 to display the Introit for an hour
            waketime = 8;
          }
        }

        if (bHasImage && ((ts.Hour >= 0 && ts.Hour < 8) || (ts.Hour >= 20 && ts.Hour < 24))) { // display image if so
          GetImageFilenameAndWakeTime(imagefilename, ts.Hour, imagecount, waketime);
          bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
        }

        if (filenumber == 0 || filenumber == 5) { // Introitus or Gospel, next part is the Gloria or Credo respectively, which may be omitted in some Masses
          subpart = 0;
          feast.get(next_hour_filenumber, subpart, s, bMoreText);
          if (feast.curr_subpartlen < 200) {
            // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so skip the next hour's reading (filenumber == 1 or 6)
            DEBUG_PRT.println(F("Skipping next hour because Gloria or Credo is omitted today (feastday)"));
            if (next_hour_filenumber == 6) {
              waketime = ts.Hour + 2; // skip the next hour's reading            
            }
            else {
              waketime = 10; // skip the reading at 9AM (Gloria is at 9AM)
            }
          }
          bResetPropersFilePtr = true;
          bMoreText = false;
          s = "";
        }
        else if (b_is_1570_Mass && feast.filecount == 11 && filenumber == 2) {  // 1570 Mass has no Lectio (filenumber == 3), so skip the next hour when it would have been displayed
          DEBUG_PRT.println(F("This Mass does not include Lectio scheduled for next hour's filenumber (Tridentine 1570) (feastday) - skipping next hour"));
          waketime = ts.Hour + 2;
        }

        if (filenumber == 1 || filenumber == 6) { // Introitus or Gospel, next part is the Gloria or Credo respectively, which may be omitted in some Masses
          subpart = 0;
          feast.get(filenumber, subpart, s, bMoreText);
          if (feast.curr_subpartlen < 200) {
            // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so display the Collect (2) or the Offertorium (7) (if filenumber == 1 or 6)
            DEBUG_PRT.println(F("Woken during Gloria or Credo on feastday when not to be displayed - displaying next filenumber"));
            filenumber++;
          }
          bResetPropersFilePtr = true;
          bMoreText = false;
          s = "";
        }
        else if (b_is_1570_Mass && feast.filecount == 11 && filenumber == 3) {  // 1570 Mass has no Lectio (filenumber == 3), so display the Gradual if woken (eg by USB power) during this hour
          DEBUG_PRT.println(F("Woken during Lectio hour when this Mass does not include it (Tridentine 1570) (feastday) - displaying next filenumber"));
          filenumber++;
        }

        if (!bImageIsDisplayed) { // from 8pm until midnight and midnight to 8am, display the Saint's image(s) (if available)
          bOverflowedScreen = false;
          subpart = 0;
          while (!bOverflowedScreen && feast.get(filenumber, subpart, s, bMoreText, bResetPropersFilePtr)) { // bResetPropersFilePtr gets reset to false if get() is called with it set to true
            bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
              pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
              bBold, bItalic, bRed, fontsize_rel,
              line_number, fbwidth, fbheight,
              bRTL, bRenderRtl, bWrapText, bMoreText);
          }
        }
      }
    }
    else { // there is both a feast and a seasonal day
      DEBUG_PRT.println(F("Feast day and seasonal day"));
      MissalReading* p_mr_Day = ordering.b_is_votive ? &votive : ordering.headings[ordering.ordering[0]]; //&feast or &votive;
      MissalReading* p_mr_Comm = ordering.b_is_votive ? &feast : ordering.headings[ordering.ordering[1]]; //&season or &feast (if votive);

      // PLL-24-12-2020 This needs understanding and fixing!
      //bool bSaintsDayTakesPrecedence = (cls_feast >= cls_season && !bSunday/*|| (cls_season < 2 && !(cls_feast == cls_season))*/); // Seasonal day takes precedence if of same class as the feast day (usually with class IV commemorations)
      //PLL-12-01-2021 Saint's day takes precedence if its class is greater the class of the season [and it is not a Sunday - PLL-12-01-2021]

      bool bSaintsDayTakesPrecedence = (ordering.ordering[0] == MR_FEAST);

      //bool bDisplayComm = ((cls_feast >= cls_season || cls_feast == 0 /*PLL-12-01-2021 or commemoration*/) && cls_season >= 2 /*Semiduplex*/); // PLL-15-12-2020 display Commemoration if the seasonal day is >= SemiDuplex
      /* // now handled above PLL-20-10-2020 - maybe display the seasonal day in the superior position if the feast and seasonal day are of the same class? [No - not in Epiphany/days of January]
      // need to exclude commemoration on Sundays and Feasts of the Lord
      bool bSaintsDayTakesPrecedence = (Tridentine::getClassIndex(feast.cls(), bUseNewClasses) >= Tridentine::getClassIndex(season.cls(), bUseNewClasses));

      if (Tridentine::sunday(date)) { // TODO: may need to make allowance for feasts that move, such as feast of St Matthias, and for feasts of the Lord which override Sundays
        bSaintsDayTakesPrecedence = false;
      }
      */
      bool bDisplayComm = (ordering.b_feast_is_commemoration || ordering.b_com || ordering.b_com_at_lauds || ordering.b_com_at_vespers);
      //bDisplayComm = (bDisplayComm || bIsLadyDay && (cls_season >= 2 || Tridentine::IsPassionWeek(date))); // PLL-01-06-2021 Display commemoration of the seasonal day if the seasonal day is Class III (Semiduplex) or above
      // PLL-01-06-2021 in the version of Divinum Officium I based the Latin Mass Propers database on, weekdays of Passion Week were reported as Class IV, which
      // breaks the logic which decides whether to show the commemoration of the seasonal day if Lady Day falls within Passion Week. This is a workaround for that

      //if (!bSaintsDayTakesPrecedence || feast.isCommemorationOnly()) { // PLL-23-04-2021 seasonal day takes precedence if feast is commemoration only (collect, secreta and postcommunio have commemoration text)
      //  p_mr_Day = &season;
      //  p_mr_Comm = &feast;

      switch (ordering.ordering[0])
      {
      case MR_SEASON:
        if (bHasSeasonImage || (bIsVotive && bHasVotiveImage)) {  // hack because Votive is in ordering.headings[0] if it is a votive day (swapped seasonal day with headings[3] if votive day)
          if (!bIsVotive) {
            DEBUG_PRT.println(F("Using image from the season"));
            imagecount = seasonimagecount; // is assigned either seasonimagecount or saintsimagecount as appropriate for the day, feast and season
            bHasImage = bHasSeasonImage;
            imagefilename = seasonimagefilename;
          }
          else {
            DEBUG_PRT.println(F("Using image from the votive"));
            imagecount = votiveimagecount; // is assigned either seasonimagecount or saintsimagecount as appropriate for the day, feast and season
            bHasImage = bHasVotiveImage;
            imagefilename = votiveimagefilename;
          }
        }
        break;

      case  MR_FEAST:
        if (bHasSaintsImage) {
          DEBUG_PRT.println(F("Using image from the feast"));
          imagecount = saintsimagecount; // is assigned either seasonimagecount or saintsimagecount as appropriate for the day, feast and season
          bHasImage = bHasSaintsImage;
          imagefilename = saintsimagefilename;
        }
        break;

      case MR_VOTIVE:
        if (bHasVotiveImage) {
          DEBUG_PRT.println(F("Using image from the votive"));
          imagecount = votiveimagecount; // is assigned either seasonimagecount or saintsimagecount as appropriate for the day, feast and season
          bHasImage = bHasVotiveImage;
          imagefilename = votiveimagefilename;
        }
        break;
      }

      /*
      if (bIsVotive) { // in this case, the feast is shown in the commemoration position (bottom left of the screen), and the votive is shown as the heading. The seasonal day is not shown
        p_mr_Day = &votive;
        p_mr_Comm = &feast;
      }
      */
      
      if (ordering.ordering[1] != -1) {
        bool b_use_commemoration_title = ordering.headings[ordering.ordering[1]]->isCommemorationOnly();
        sanctoral_day = ordering.headings[ordering.ordering[1]]->name(b_use_commemoration_title);
      }
      else {
        sanctoral_day = bIsVotive ? feast.name() : "";
      }
        
      //sanctoral_day = bIsVotive ? feast.name() : sanctoral_day;

      //sanctoral_day = bIsVotive ? feast.name() :
      //              ordering.ordering[1] != -1 ? ordering.headings[ordering.ordering[1]]->name() : "";
      
      bool bIsCommemorationAndSeasonalDayOnly = feast.isCommemorationOnly(); // && !bIsVotive;

      // on commemorational day, when not also a votive mass day, want the commemoration in the top position on the display (display_date_ex call)
      //if (bIsCommemorationAndSeasonalDayOnly) { // in this case p_mr_Comm will point to the feast, want day name from this. p_mr_Day will point to the season
      //  sanctoral_day = p_mr_Day->name(); // text in lower position is from the season, text in upper position will be the name of the commemoration
      //}
      //else {
      //  sanctoral_day = p_mr_Comm->name(); // text in lower position will be from the season, or the feast if !bSaintsDayTakesPrecedence (based on rank)
      //}

      ypos = display_day_ex(date,
        //bIsCommemorationAndSeasonalDayOnly ? p_mr_Comm->commemoration() : p_mr_Day->name(),
        p_mr_Day->name(),
        p_mr_Day->colour(),         // will be the colour of the season if a commemoration (because in this case p_mr_Day will point to the season,
        td.HolyDayOfObligation,     // according to the logic (feast.isCommemorationOnly() && !bIsVotive)), or the colour of the feast if not  
        right_to_left,
        bLiturgical_Colour_Red,
        diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);   // feast day is displayed at the top of the screen on feast days otherwise the liturgical day is

      if (!b_is_purification_of_mary) {

        DEBUG_PRT.print(F("filenumber is "));
        DEBUG_PRT.println(filenumber);

        // Feast day takes precedence, seasonal day is commemoration
        switch (filenumber) {
        case 0: // Introitus (from the feast (PLL-23-04-2021 Unless is a commemoration only, in which case use the introitus from the season))
        case 5: // Gospel
          subpart = 0;
          p_mr_Day->get(next_hour_filenumber, subpart, s, bMoreText);
          if (p_mr_Day->curr_subpartlen < 200) {
            // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so skip the next hour's reading (filenumber == 1 or 6)
            DEBUG_PRT.println(F("Skipping next hour because Gloria or Credo is omitted today (seasonal day and/or feastday)"));
            if (next_hour_filenumber == 6) {
              waketime = ts.Hour + 2; // skip the next hour's reading            
            }
            else {
              waketime = 10; // skip the reading at 9AM (Gloria is at 9AM)
            }
          }
          bResetPropersFilePtr = true;
          bMoreText = false;
          s = "";

        case 3: // Lesson
          if (b_is_1570_Mass && p_mr_Day->filecount == 11 && filenumber == 3) { // 1570 Mass has no Lectio (filenumber == 3), so display the Gradual if woken (eg by USB power) during this hour
            DEBUG_PRT.println(F("Woken during Lectio hour when this Mass does not include it (Tridentine 1570) (seasonal day + feastday) - displaying next filenumber"));
            filenumber++;
          }

        case 4: // Gradual
        case 7: // Offertorium
        case 10: // Communio

          if (bHasImage && ts.Hour >= 0 && ts.Hour <= 7) {
            waketime = 8;
          }

          if (!(bHasImage && ts.Hour >= 0 && ts.Hour <= 7 && DisplayImage(imagefilename, 0, ypos))) { // from midnight until 8am, display the Saint's image (if available)
            bOverflowedScreen = false;
            subpart = 0;
            while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText, bResetPropersFilePtr)) {
              bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
                pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
                bBold, bItalic, bRed, fontsize_rel,
                line_number, fbwidth, fbheight,
                bRTL, bRenderRtl, bWrapText, bMoreText);
            }
          }
          break;

        case 1: // Gloria PLL-03-08-2020 From the day which has precedence (feast or seasonal day)     //(from the season, omit if required)
        case 6: // Credo
          subpart = 0;
          p_mr_Day->get(filenumber, subpart, s, bMoreText);
          if (p_mr_Day->curr_subpartlen < 200) {
            // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so skip the next hour's reading (filenumber == 1 or 6)
            DEBUG_PRT.println(F("Woken during Gloria or Credo hour, but they are omitted today (seasonal day and/or feastday), so displaying next filenumber"));
            filenumber++;
          }
          bResetPropersFilePtr = true;
          bMoreText = false;
          s = "";

        case 9: // Prefatio
          bOverflowedScreen = false;
          subpart = 0;

          while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText, bResetPropersFilePtr)) {
            bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
              pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
              bBold, bItalic, bRed, fontsize_rel,
              line_number, fbwidth, fbheight,
              bRTL, bRenderRtl, bWrapText, bMoreText);
          }
          break;

        case 2: // Collect
          if (b_is_1570_Mass && p_mr_Day->filecount == 11 && filenumber == 2) { // 1570 Mass has no Lectio (filenumber == 3), so skip the next hour when it would have been displayed
            DEBUG_PRT.println(F("This Mass does not include Lectio scheduled for next hour's filenumber (Tridentine 1570) (seasonal day + feastday) - skipping next hour"));
            waketime = ts.Hour + 2;
          }

        case 8: // Secreta
        case 11: // Postcommunio
        {
          if (bHasImage && ts.Hour == 19) {
            waketime = ts.Hour + 1;
          }

          if (bHasImage) { // between 8pm and midnight
            GetImageFilenameAndWakeTime(imagefilename, ts.Hour, imagecount, waketime);
          }

          if (!(bHasImage && ts.Hour > 19 && DisplayImage(imagefilename, 0, ypos))) { // from 8pm until midnight, display the Saint's image (if available)
            bOverflowedScreen = false;
            subpart = 0;

            bool b_votive_has_extra_commemoration_text = false;

            while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText) && !b_votive_has_extra_commemoration_text) {
              // will stop reading Votive text when it reaches any of the Commemoratio lines, which are incorrectly 
              // present (baked in for a certain day) in the Votive mass Propers

              b_votive_has_extra_commemoration_text = (bIsVotive && (
                s.indexOf("Blasii") != -1 ||
                s.indexOf("Marcellini") != -1 ||
                s.indexOf("Telesphor") != -1 ||
                s.indexOf("Faustini") != -1 ||
                s.indexOf("Undecima") != -1
                ));

              if (!bIsVotive || !b_votive_has_extra_commemoration_text) { // bit of a hack to get around the Votive masses, which have picked up the Commemoration of the day they were taken from (which is not always the day they are shown)
                bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
                  pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
                  bBold, bItalic, bRed, fontsize_rel,
                  line_number, fbwidth, fbheight,
                  bRTL, bRenderRtl, bWrapText, bMoreText);
              }
            }
            //// TODO: PLL-25-07-2020 Find out how this works! 
            //         PLL-15-12-2020 Found a problem with days of Advent, patched, though still not sure how it works
            if (!bSaintsDayTakesPrecedence || bDisplayComm || bIsCommemorationAndSeasonalDayOnly) {  // PLL-01-06-2021 Added LadyDay because Divinum Officium displays the commemoration on this day, despite it being a Class I feast //PLL-23-04-2021 added bIsCommemorationAndSeasonalDayOnly (St George's Day commemoration and others. Bit byzantine, but hopefully works)
              //if (!bOverflowedScreen && (indexrecord_saint.filenumber == 2 || indexrecord_saint.filenumber == 11)) {   // do a line feed before the text of the commemoration
              if (!bOverflowedScreen && !bIsVotive && (filenumber == 2 || filenumber == 11)) {   // do a line feed before the text of the commemoration
                String crlf = F(" <BR>"); // hack: the space char should give a line height to the typesetter, otherwise it would be 0 since there are no printing characters on the line

                bOverflowedScreen = Bidi::RenderTextEx(crlf, &xpos, &ypos, tb,
                  pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
                  bBold, bItalic, bRed, fontsize_rel,
                  line_number, fbwidth, fbheight,
                  bRTL, bRenderRtl, bWrapText, true);
              }
              // ******* PLL-01-06-2021 need to make sure that Postcommunio etc displays commemoration for seasonal day on Lady Day (25th March unless moved)
              int8_t linecount = 0;
              subpart = 0;
              p_mr_Comm->get(filenumber, subpart, s, bMoreText); // eat a line (the heading), should then get a <BR> on its own line after the heading

              bool b_is_ember_day = Tridentine::IsEmberDay(date);
              bool b_is_ember_wednesday = (b_is_ember_day && weekday(date) == dowWednesday);
              bool b_is_ember_friday = (b_is_ember_day && weekday(date) == dowFriday);
              bool b_is_ember_saturday = (b_is_ember_day && weekday(date) == dowSaturday);
              bool b_is_matching_lent_collect = (td.FileDir_Season.indexOf(F("Lent/1/Saturday")) > -1 || td.FileDir_Season.indexOf(F("Lent/1/Wednesday")) > -1 || td.FileDir_Season.indexOf(F("Lent/4/Wednesday")) > -1);

              //int8_t br_count_max = b_is_matching_lent_collect ? 2 : 1;

              if (bIsCommemorationAndSeasonalDayOnly) {
                // eat three more lines if so
                p_mr_Comm->get(filenumber, subpart, s, bMoreText); // eat a line (<BR>)
                p_mr_Comm->get(filenumber, subpart, s, bMoreText); // eat a line (Let us pray), (This is in the text where the seasonal day's text goes, which we've already printed from a separate file)
                p_mr_Comm->get(filenumber, subpart, s, bMoreText); // eat a line ("Oratio missing"), should then get a " <BR>" on its own line after (placeholder for seasonal day's text, already printed from a separate file)
              }

              while (!bOverflowedScreen && p_mr_Comm->get(filenumber, subpart, s, bMoreText)) {
                DEBUG_PRT.print(F("\nlinecount: "));
                DEBUG_PRT.println(linecount);
                //Bidi::printf("linecount=%d, filenumber=%d ", linecount, filenumber);
                if (bIsVotive && (linecount == 1 && (filenumber == 2 || filenumber == 11))) {
                  linecount++;
                  continue;
                }

                if ((((b_is_ember_wednesday || b_is_ember_saturday ) && filenumber == 2 && linecount == 2) || (b_is_matching_lent_collect && filenumber == 2 && linecount == 2)) && s.length() < 30) { // skip "Let us kneel/Arise lines (if present), which are between two <BR> tags each on their own line. Using the length < 30 test to try to catch when these short lines are present. Hopefully the text we're after will be longer in all cases
                  int br_count = 0;
                  do {
                    p_mr_Comm->get(filenumber, subpart, s, bMoreText);
                    if (s.indexOf("<BR>") == 0) {
                      br_count++;
                    }
                  } while (bMoreText && br_count < 1);
                }

                if (!bIsCommemorationAndSeasonalDayOnly)
                {
                  if ((linecount == 0 && filenumber == 8) ||                       // commemoratio line is first line in this case
                    (linecount == 2 && (filenumber == 2 || filenumber == 11))) { // commemoratio line comes after <br>"Let us pray"<br> line
                    String commtext = "<FONT COLOR=\"red\"><I>Commemoratio " + p_mr_Comm->name() + "</I></FONT>";
                    commtext.replace(" %{monthweek} %{month}", "");
                    //Bidi::printf("linecount=%d, filenumber=%d", linecount, filenumber);

                    if (!bIsVotive || ((bIsVotive && linecount == 0 && filenumber == 8) || (linecount == 0 && filenumber == 8))) {
                      commtext = " <BR>" + commtext;
                    }

                    bool b_text_starts_with_br = (s.indexOf("<BR>") == 0);

                    // add linebreak after where necessary (according to the structure of the propers text files)
                    if (!bIsVotive && !b_is_ember_wednesday && !b_is_ember_friday && !b_is_ember_saturday && filenumber != 8 && !(b_is_matching_lent_collect && filenumber == 2) 
                      || (b_is_ember_saturday && filenumber == 11)
                      || (b_is_ember_friday && (filenumber == 2 || filenumber == 11))
                      || (b_is_ember_wednesday && filenumber == 11)
                      || (bIsVotive && linecount == 2 && (filenumber == 2 || filenumber == 11))
                      || !b_text_starts_with_br) {
                      commtext = commtext + "<BR>";
                    }

                    if (!bOverflowedScreen) {
                      bOverflowedScreen = Bidi::RenderTextEx(commtext, &xpos, &ypos, tb,
                        pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
                        bBold, bItalic, bRed, fontsize_rel,
                        line_number, fbwidth, fbheight,
                        bRTL, bRenderRtl, bWrapText, true);
                    }
                  }
                }
                else {                 
                  if ((linecount == 0 && filenumber == 8) ||                       // commemoratio line is first line in this case
                    (linecount == 2 && (filenumber == 2 || filenumber == 11)))   // commemoratio line comes after <br>"Let us pray"<br> line
                  {
                    String commtext = "<BR><FONT COLOR=\"red\"><I>" + p_mr_Comm->commemoration() + "</I></FONT><BR>"; // text will include "Commemoratio" and the Saint's name in this case

                    if (filenumber == 2 || filenumber == 11) // if not secreta, there will be an extra "Commemoratio" line in the text to eat, which will be replaced with the full text "Commemoratio: <Saint's name>"
                    {
                      p_mr_Comm->get(filenumber, subpart, s, bMoreText); // eat the line ("Commemoratio:")
                    }

                    if (filenumber == 8) // add a linefeed before the text in this case (overprints on last line of seasonal text otherwise)
                    {
                      DEBUG_PRT.println("Prepending linebreak to commtext");
                      commtext = " <BR>" + commtext;
                    }

                    if (!bOverflowedScreen)
                    {
                      bOverflowedScreen = Bidi::RenderTextEx(commtext, &xpos, &ypos, tb,
                        pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
                        bBold, bItalic, bRed, fontsize_rel,
                        line_number, fbwidth, fbheight,
                        bRTL, bRenderRtl, bWrapText, true);
                    }
                  }
                }

                if (!bOverflowedScreen) {
                  //if (s.length() == 4 && s == "<BR>") {
                  //  DEBUG_PRT.println("adding space to <BR> for font height measurement");
                  //  s = " <BR>";
                  //}
                  bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
                    pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
                    bBold, bItalic, bRed, fontsize_rel,
                    line_number, fbwidth, fbheight,
                    bRTL, bRenderRtl, bWrapText, bMoreText);
                }
                if (s.indexOf("<FONT COLOR=\"red\"><I>R.</I></FONT>") != -1) break; // Only output commemoration of the Saint, not commemorations of other Saints on the same feast day
                linecount++;
              }
            }
          }
          //// PLL-25-07-2020
          break;
        }

        default:
          break;
        }
      }
      else 
      { // purification of Mary propers
        waketime = DoPurificationOfMary(p_mr_Day, p_mr_Comm,
          (ordering.b_com || ordering.b_com_at_lauds || ordering.b_com_at_vespers),
          filenumber, ts.Hour, lang, mass_type,
          xpos, ypos,
          pDiskfont,
          bBold, bItalic, bRed,
          fontsize_rel, line_number,
          fbwidth, fbheight,
          bRTL, bRenderRtl, bWrapText, bHasImage, imagefilename);
      }
    }
  } // if (bIsFeast)
  else if (season.isOpen() || (bIsVotive && votive.isOpen())) // could be that there is no feast, season or votive for the day due to a mistake in the propers database
  { // is a seasonal day only                 // If so, will fail gracefully by displaying an image after the else case of this else if...
    DEBUG_PRT.println(F("Seasonal day only"));
    if (bHasSeasonImage) {
      DEBUG_PRT.println(F("Using image from the season"));
    }
    imagecount = seasonimagecount; // is assigned either seasonimagecount or saintsimagecount as appropriate for the day, feast and season
    bHasImage = bHasSeasonImage;
    imagefilename = seasonimagefilename;

    String name = bIsVotive ? votive.name() : season.name();
    String colour = bIsVotive ? votive.colour() : season.colour();

    ypos = display_day_ex(date, name, colour, td.HolyDayOfObligation, right_to_left, bLiturgical_Colour_Red, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);   // feast day is displayed at the top of the screen on feast days otherwise the liturgical day is 

    subpart = 0;
    bool b_read_only_one_subpart = false; // For Good Friday, the Great Intercessions and the readings from Scripture, Passion, display one at a time over the selected hours of the day
    int8_t read_subpart_count = 0;

    bool bImageIsDisplayed = false;

    bool b_is_Good_Friday = Tridentine::IsGoodFriday(date);
    bool b_is_Ash_Wednesday = Tridentine::issameday(date, Tridentine::AshWednesday(Tridentine::year(date)));
    bool b_is_Holy_Saturday = Tridentine::IsHolySaturday(date);
    bool b_is_Palm_Sunday = Tridentine::issameday(date, Tridentine::PalmSunday(Tridentine::year(date)));

    if (/*!bIsVotive && (season.filecount != 12 || (b_is_1570_Mass && feast.filecount != 11)) || */ b_is_Ash_Wednesday || b_is_Palm_Sunday || b_is_Good_Friday || b_is_Holy_Saturday)
    {
      /*
      if (b_is_purification_of_mary) {
        // purification of Mary propers
        waketime = DoPurificationOfMary(&season, NULL,
          filenumber, ts.Hour, lang, mass_type,
          xpos, ypos,
          pDiskfont,
          bBold, bItalic, bRed,
          fontsize_rel, line_number,
          fbwidth, fbheight,
          bRTL, bRenderRtl, bWrapText, bHasImage, imagefilename);
      }
      else */

      if (b_is_Ash_Wednesday) {
        DEBUG_PRT.print(F("Ash Wednesday: ts.Hour="));
        DEBUG_PRT.println(ts.Hour);

        if (mass_type > MASS_TRIDENTINE_1570) {
          waketime = (ts.Hour < 3) ? 3 :
            (ts.Hour == 8 || ts.Hour == 13) ? ts.Hour + 2 :   // no Gloria or Creed
            (ts.Hour == 9 || ts.Hour == 14) ? ts.Hour + 2 :   // no Gloria or Creed (skip next hour if woken at these times, since it will be the same reading)
            (ts.Hour >= 3 && ts.Hour <= 18) ? ts.Hour + 1 : 0;  // last reading at 7pm

          filenumber = (ts.Hour < 3) ? 0 :
            (ts.Hour >= 3 && ts.Hour <= 8) ? 0 :
            (ts.Hour == 9 || ts.Hour == 14) ? ts.Hour - 8 + 1 : // no Gloria or Creed
            (ts.Hour >= 9 && ts.Hour <= 19) ? ts.Hour - 8 : 0;

          subpart = (ts.Hour < 3) ? 0 :
            (ts.Hour >= 3 && ts.Hour <= 8) ? ts.Hour - 3 : 0;   // 0..5 of filenumber 0 between 3 and 8am

          b_read_only_one_subpart = (ts.Hour < 8);
        }
        else {
          waketime = (ts.Hour < 3) ? 3 :
            (ts.Hour == 8 || ts.Hour == 13 || ts.Hour == 10) ? ts.Hour + 2 :    // no Gloria, Creed or Lesson
            (ts.Hour == 9 || ts.Hour == 14 || ts.Hour == 11) ? ts.Hour + 2 :    // no Gloria, Creed or Lesson (skip next hour if woken at these times, since it will be the same reading)
            (ts.Hour >= 3 && ts.Hour <= 18) ? ts.Hour + 1 : 0;  // last reading at 6pm (no Lesson in 1570 mass)

          filenumber = (ts.Hour < 3) ? 0 :
            (ts.Hour >= 3 && ts.Hour <= 8) ? 0 :
            (ts.Hour == 11) ? 4 :               // read Graduale at 11am, since there is no Lesson in the 1570 Mass (which would otherwise be shown)
            (ts.Hour == 9 || ts.Hour == 14) ? ts.Hour - 8 + 1 : // no Gloria or Creed
            (ts.Hour >= 9 && ts.Hour <= 19) ? ts.Hour - 8 : 0;

          subpart = (ts.Hour < 3) ? 0 :
            (ts.Hour >= 3 && ts.Hour <= 8) ? ts.Hour - 3 : 0;   // 0..5 of filenumber 0 between 3 and 8am

          b_read_only_one_subpart = (ts.Hour < 8);
        }

        if (mass_type == MASS_1960) {
          b_read_only_one_subpart = false;
          read_subpart_count = (ts.Hour <= 6) ? 2 : 
            (ts.Hour >= 7 && ts.Hour <= 8) ? 1 : -1;  // -1 = read all available subparts

          subpart = (ts.Hour < 3) ? 0 :
            (ts.Hour >= 3 && ts.Hour <= 7) ? (ts.Hour - 3) * 2 :  // ten subparts upto and including introitus - read them two at a time from 3am, up until the introitus (pt 10)(8am)
            (ts.Hour == 8) ? 9 : 0;
        }

        if (bHasImage && (ts.Hour >= 0 && ts.Hour <= 2 || ts.Hour >= 20)) {
          GetImageFilenameAndWakeTime(imagefilename, ts.Hour, imagecount, waketime);
          bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
        }
      }
      else if (b_is_Palm_Sunday) {
        const int8_t filenumbers_1570_la[24]    = { 0, 0, 1, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const int8_t fileparts_1570_la[24]      = { 0, 1, 0, 0, 0, 1, 3, 5, 0, 1, 2, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0 };
        const int8_t filenumbers_1570[24]     = { 0, 1, 2, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 7, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const int8_t fileparts_1570[24]       = { 0, 0, 0, 0, 0, 1, 3, 5, 0, 1, 2, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0 };

        const int8_t filenumbers_1910_and_DA_la[24] = { 0, 0, 1, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const int8_t fileparts_1910_and_DA_la[24] = { 0, 1, 0, 0, 0, 1, 3, 5, 0, 1, 2, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0 };
        const int8_t filenumbers_1910_and_DA[24]  = { 0, 1, 2, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
        const int8_t fileparts_1910_and_DA[24]    = { 0, 0, 0, 0, 0, 1, 3, 5, 0, 1, 2, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

        const int8_t filenumbers_1955_60_la[24]   = { 0, 1, 1, 2, 3, 4, 5, 5, 5, 5, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16 };
        const int8_t fileparts_1955_60_la[24]   = { 0, 0, 1, 0, 0, 0, 0, 1, 2, 3, 3, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
        const int8_t filenumbers_1955_60[24]    = { 0, 1, 1, 2, 3, 4, 5, 5, 6, 6, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 17, 17, 17 };
        const int8_t fileparts_1955_60[24]      = { 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

        const int8_t filenumbers_60New_67_la[24]  = { 0, 0, 1, 2, 3, 3, 3, 3, 3, 4, 4, 5, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const int8_t fileparts_60New_67_la[24]    = { 0, 1, 0, 0, 0, 1, 3, 4, 6, 0, 1, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0 };
        const int8_t filenumbers_60New_67[24]   = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 5, 5, 6, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
        const int8_t fileparts_60New_67[24]     = { 0, 0, 0, 0, 0, 1, 3, 4, 6, 0, 1, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

        // for clarity of code, and because they're almost all different, using lookup tables for Palm Sunday Mass parts (total space for luts=384 bytes)
    
        switch (mass_type)
        {
        case MASS_TRIDENTINE_1570:
          if (lang == "la") {
            filenumber = filenumbers_1570_la[ts.Hour];
            subpart = fileparts_1570_la[ts.Hour];
            read_subpart_count = (filenumber == 3 && subpart > 0) ? 2 : 0;
          }
          else {
            filenumber = filenumbers_1570[ts.Hour];
            subpart = fileparts_1570[ts.Hour];
            read_subpart_count = (filenumber == 4 && subpart > 0) ? 2 : 0;
          }
          waketime = ts.Hour < 23 ? ts.Hour + 1 : -1;
          b_read_only_one_subpart = (ts.Hour < 11 && read_subpart_count == 0);

          if (bHasImage && ts.Hour == 12) { // display Palm Sunday image at 1pm
            bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
          }

          if ((!bHasImage && ts.Hour == 11) || ts.Hour == 14) { // No Image, so skip hour 12 (when it would be shown) || No Lesson in 1570 Mass, so skip hour 14 when it would be shown
            waketime++; // if no image, skip 1pm
          }
          break;

        case MASS_TRIDENTINE_1910:
        case MASS_DIVINEAFFLATU:
          if (lang == "la") {
            filenumber = filenumbers_1910_and_DA_la[ts.Hour];
            subpart = fileparts_1910_and_DA_la[ts.Hour];
            read_subpart_count = (filenumber == 3 && subpart > 0) ? 2 : 0;
          }
          else {
            filenumber = filenumbers_1910_and_DA[ts.Hour];
            subpart = fileparts_1910_and_DA[ts.Hour];
            read_subpart_count = (filenumber == 4 && subpart > 0) ? 2 : 0;
          }
          waketime = ts.Hour < 23 ? ts.Hour + 1 : -1;
          b_read_only_one_subpart = (ts.Hour < 11 && read_subpart_count == 0);

          if (bHasImage && ts.Hour == 12) { // display Palm Sunday image at 1pm
            bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
          }

          if (!bHasImage && ts.Hour == 11) {
            waketime++; // if no image, skip 1pm
          }

          break;

        case MASS_1955:
        case MASS_1960:
          if (lang == "la") {
            filenumber = filenumbers_1955_60_la[ts.Hour];
            subpart = fileparts_1955_60_la[ts.Hour];
          }
          else {
            filenumber = filenumbers_1955_60[ts.Hour];
            subpart = fileparts_1955_60[ts.Hour];
          }
          waketime = ts.Hour < 20 ? ts.Hour + 1 : -1;
          b_read_only_one_subpart = (ts.Hour <= 8 && filenumber != 2);

          if (bHasImage && ts.Hour == 9) {  // display Palm Sunday image at 9am
            bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
          }

          if (!bHasImage && ts.Hour == 8) {
            waketime++; // if no image, skip 9am
          }

          break;

        case MASS_1960NEW:
        case MASS_1965_67:
          if (lang == "la") {
            filenumber = filenumbers_60New_67_la[ts.Hour];
            subpart = fileparts_60New_67_la[ts.Hour];
          }
          else {
            filenumber = filenumbers_60New_67[ts.Hour];
            subpart = fileparts_60New_67[ts.Hour];
          }
          waketime = ts.Hour < 23 ? ts.Hour + 1 : -1;
          b_read_only_one_subpart = (ts.Hour < 11 && ts.Hour != 5 && ts.Hour != 7);
          read_subpart_count = (ts.Hour == 5 || ts.Hour == 7) ? 2 : 0;

          if (bHasImage && ts.Hour == 12) { // display Palm Sunday image at 9am
            bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
          }

          if (!bHasImage && ts.Hour == 11) {
            waketime++; // if no image, skip 9am
          }
          break;

        default:
          filenumber = 0;
          subpart = 0;
          waketime = 0;

          if (bHasImage) {
            bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
          }
          else {
            String imagefilename = fileroot_img + "/Error.bwr";
            bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
          }

          DEBUG_PRT.println("Unknown Mass type for Palm Sunday Mass");
          break;
        }
      }
      else if (b_is_Good_Friday) {
        b_read_only_one_subpart = true;

        DEBUG_PRT.print(F("Good Friday: ts.Hour="));
        DEBUG_PRT.println(ts.Hour);

        if (mass_type != MASS_1960) {
          switch (ts.Hour)
          {
          case 0:
          case 1:
          case 2:
            if (!(ts.Hour == 2 && mass_type == MASS_1955)) {
              if (bHasImage) {    // Picture
                bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
              }
            }
            filenumber = 0;
            subpart = 0;

            waketime = (mass_type == MASS_1955 && (ts.Hour == 0 || ts.Hour == 1)) ? 2 : 3;
            //waketime = 3;
            break;

          case 3:
          case 4:
            filenumber = (mass_type == MASS_1955) ? 1 : 0;      // Lectiones
            subpart = ts.Hour - 3;
            waketime = ts.Hour + 1;
            break;

          case 5:
            filenumber = (mass_type == MASS_1955) ? 2 : 1;      // PASSION
            subpart = 0;
            waketime = ts.Hour + 1;
            break;

          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
          case 12:
          case 13:
          case 14:
            filenumber = (mass_type == MASS_1955) ? 3 : 2;      // THE GREAT INTERCESSIONS
            subpart = ts.Hour - 6;
            waketime = ts.Hour + 1;
            break;

          case 15:          // Picture or Adoration of the Cross
            if (bHasImage) {
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = (mass_type == MASS_1955) ? 4 : 3;
            subpart = 0;
            waketime = ts.Hour + 1;
            break;

          case 16:
          case 17:
          case 18:
            filenumber = (mass_type == MASS_1955) ? 4 : 3;      // Adoration of the Cross
            subpart = ts.Hour - 16;
            waketime = ts.Hour + 1;
            
            b_read_only_one_subpart = !(mass_type == MASS_1955 && ts.Hour == 18); // read two subparts at 6pm for the 1955 Mass, since there are four Adoration parts not three
            break;

          case 19:
          case 20:
            filenumber = (mass_type == MASS_1955) ? 5 : 4;      // Communion
            subpart = ts.Hour - 19;
            waketime = ts.Hour + 1;
            if (!bHasImage && waketime == 21) {
              waketime = -1;
            }
            break;

          case 21:
          case 22:
          case 23:          // Picture
            if (bHasImage) {
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = (mass_type == MASS_1955) ? 5 : 4;
            subpart = 1;
            waketime = -1;
            break;
          
          default:
            if (bHasImage) {
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = (mass_type == MASS_1955) ? 4 : 3;      // Adoration of the Cross
            subpart = 0;
            waketime = -1;
            break;
          }
          
        }
        else {
          switch (ts.Hour)
          {
          case 0:
          case 1:
            if (bHasImage) {    // Picture
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = 0;
            subpart = 0;
            waketime = 2;
            break;
          
          case 2:
            filenumber = 0;
            subpart = 0;
            waketime = 3;
            break;

          case 3:
          case 4:
            filenumber = 1;     // Lectiones (Readings from Scripture, Passion)
            subpart = ts.Hour - 3;
            waketime = ts.Hour + 1;
            break;

          case 5:
            filenumber = 2;     // PASSION
            subpart = 0;
            waketime = ts.Hour + 1;
            break;

          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
          case 12:
          case 13:
          case 14:
            filenumber = 3;     // THE GREAT INTERCESSIONS
            subpart = ts.Hour - 6;
            waketime = ts.Hour + 1;
            break;

          case 15:          // Picture or Adoration of the Cross
            if (bHasImage) {
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = 4;
            subpart = 0;
            waketime = ts.Hour + 1;
            break;

          case 16:
          case 17:
          case 18:
          case 19:
            filenumber = 4;     // Adoration of the Cross
            subpart = ts.Hour - 16;
            waketime = ts.Hour + 1;
            break;

          case 20:
          case 21:
            filenumber = 5;     // Communion
            subpart = ts.Hour - 20;
            waketime = ts.Hour + 1;
            if (!bHasImage && waketime == 22) {
              waketime = -1;
            }
            break;

          case 22:
          case 23:          // Picture
            if (bHasImage) {
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = 5;
            subpart = 1;
            waketime = -1;
            break;

          default:
            if (bHasImage) {
              bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
            }
            filenumber = 4;     // Adoration of the Cross
            subpart = 0;
            waketime = -1;
            break;
          }
        }
/*
        if (ts.Hour == 0) {
          filenumber = 0; // Descriptive heading
          subpart = 0;
          waketime = 1;
        }

        if (ts.Hour >= 1 && ts.Hour <= 2) {
          filenumber = 1; // Readings from Scripture, Passion
          subpart = ts.Hour - 1;
          waketime = ts.Hour + 1;
        }

        if (ts.Hour >= 3 && ts.Hour <= 5) {
          filenumber = 2; // Passion
          subpart = 0;
          if (ts.Hour == 3) waketime = 6;
        }

        if (ts.Hour >= 6 && ts.Hour <= 14) {
          filenumber = 3; // II. The Great Intercessions
          subpart = ts.Hour - 6; // 0..8
          waketime = ts.Hour + 1;
        }
*/
        /*
        if (ts.Hour >= 15 && ts.Hour <= 18) {
          filenumber = 4; // Adoration of the Cross
          subpart = ts.Hour - 15; // 0..3
          waketime = ts.Hour + 1;
        }
        */
/*
        uint8_t hour_offset_for_image_display = bHasImage ? 1 : 0; // everything will be shifted an hour later from 3pm, to give an hour for the image of the Cross to be displayed
        if (ts.Hour >= 15 && ts.Hour <= 18 + hour_offset_for_image_display) {
          if (bHasImage) {
            if (ts.Hour == 15 && DisplayImage(imagefilename, 0, ypos)) {
              //filenumber = 4; // Adoration of the Cross, display image of Christ crucified for one hour, then readings at 4 and 5pm
              //subpart = ts.Hour - 15; // 0..3
              bImageIsDisplayed = true;
              waketime = ts.Hour + 1;
            }
            else if (ts.Hour >= 16) {
              filenumber = 4; // Adoration of the Cross
              subpart = ts.Hour - 16; // 0..3
              waketime = ts.Hour + 1;
            }
          }
          else {
            filenumber = 4; // Adoration of the Cross, display readings in three parts from 3pm (time of Christ's death)
            subpart = ts.Hour - 15; // 0..3
            waketime = ts.Hour + 1;
          }
        }

        if (ts.Hour >= 19 + hour_offset_for_image_display && ts.Hour <= 20 + hour_offset_for_image_display) {
          filenumber = 5; // Communion
          subpart = ts.Hour - (19 + hour_offset_for_image_display); // 0 or 1
          if (ts.Hour == 19 + hour_offset_for_image_display) {
            waketime = 20 + hour_offset_for_image_display;
          }
          else {
            //waketime = 0; //Allow to default (will be midnight)
          }
        }

        if (ts.Hour >= 21 + hour_offset_for_image_display) {
          filenumber = 5;
          subpart = 1;  // if woken after the last reading (Communion pt 2), display this same reading again.
        }
*/      
      }
      else if (Tridentine::IsHolySaturday(date)) {
        int8_t num_parts = 22;

        if (mass_type <= MASS_DIVINEAFFLATU) {
          num_parts = 16;
        }

        if (ts.Hour <= num_parts - 1) { // 22 file parts unless Tridentine Mass (1570, 1910, DivinoAfflatu)
          filenumber = ts.Hour;
          subpart = 0;
          waketime = ts.Hour < (num_parts - 1) ? ts.Hour + 1 : 0;
        }
        else {
          filenumber = num_parts - 1; // if Lectionary is woken after last reading
          subpart = 0;
          waketime = 0;
        }
      }
    }
    else
    { // if is a 12-part Proper
      MissalReading* p_mr_Day = bIsVotive ? &votive : &season;

      if (next_hour_filenumber == 1 || next_hour_filenumber == 6) { // removed !bIsVotive from clause. if the next reading will be Gloria or Credo in a non-votive Mass
        subpart = 0;
        p_mr_Day->get(next_hour_filenumber, subpart, s, bMoreText);
        if (p_mr_Day->curr_subpartlen < 200) {
          // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so skip the next hour's reading (filenumber == 1 or 6)
          DEBUG_PRT.println(F("Skipping next hour because Gloria or Credo is omitted today (seasonal day only)"));
          if (next_hour_filenumber == 6) {
            waketime = ts.Hour + 2; // skip the next hour's reading            
          }
          else {
            waketime = 10; // skip the reading at 9AM (Gloria is at 9AM)
          }
        }
        bResetPropersFilePtr = true;
        bMoreText = false;
        s = "";
      }
      else if (b_is_1570_Mass && p_mr_Day->filecount == 11 && filenumber == 2) {  // 1570 Mass has no Lectio (filenumber == 3), so skip the next hour when it would have been displayed
        DEBUG_PRT.println(F("This Mass does not include Lectio scheduled for next hour's filenumber (Tridentine 1570) (seasonal day) - skipping next hour"));
        waketime = ts.Hour + 2;
      }

      if (filenumber == 1 || filenumber == 6) { // removed !bIsVotive from clause. Introitus or Gospel, next part is the Gloria or Credo respectively, which may be omitted in some Masses
        subpart = 0;
        p_mr_Day->get(filenumber, subpart, s, bMoreText);
        if (p_mr_Day->curr_subpartlen < 200) {
          // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so display the Collect (2) or the Offertorium (7) (if filenumber == 1 or 6)
          DEBUG_PRT.println(F("Woken during Gloria or Credo on seasonal day when not to be displayed - displaying next filenumber (seasonal day)"));
          filenumber++;
          subpart = 0;
        }
        bResetPropersFilePtr = true;
        bMoreText = false;
        s = "";
      }
      else if (b_is_1570_Mass && feast.filecount == 11 && filenumber == 3) {  // 1570 Mass has no Lectio (filenumber == 3), so display the Gradual if woken (eg by USB power) during this hour
        DEBUG_PRT.println(F("Woken during Lectio hour when this Mass does not include it (Tridentine 1570) (seasonal day) - displaying next filenumber"));
        filenumber++;
      }
    }

    // image will be displayed (if available) in this case between midnight and 8am, and 8pm and midnight
    if (bIsVotive && bHasVotiveImage && ((ts.Hour >= 0 && ts.Hour < 8) || (ts.Hour >= 20 && ts.Hour < 24))) { // display image for votive Mass if so
      GetImageFilenameAndWakeTime(votiveimagefilename, ts.Hour, votiveimagecount, waketime);
      bImageIsDisplayed = DisplayImage(votiveimagefilename, 0, ypos);
    }
    else if (bHasImage && ((ts.Hour >= 0 && ts.Hour < 8 && !b_is_Good_Friday && !b_is_Ash_Wednesday && !b_is_Palm_Sunday) || (!b_is_Good_Friday && !b_is_Palm_Sunday && ts.Hour >= 20 && ts.Hour < 24))) { // display image if so
      GetImageFilenameAndWakeTime(imagefilename, ts.Hour, imagecount, waketime);
      bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
    }

    MissalReading* p_mr_Day = bIsVotive ? &votive : &season;

    //subpart = 0; // should be initialized to the correct value by this point (usually 0, but the propers handling for Good Friday above modifies it on some hours)
    //int8_t first_subpart = subpart;
    bOverflowedScreen = false;

    if (!bImageIsDisplayed) {
      bool b_votive_has_extra_commemoration_text = false;
      int8_t linecount = 0;

      //bool b_end_of_subpart_reached_break_now = false;

      int8_t last_subpart = subpart;

      while (!bOverflowedScreen && /*!b_end_of_subpart_reached_break_now && */ p_mr_Day->get(filenumber, subpart, s, bMoreText, bResetPropersFilePtr) && !b_votive_has_extra_commemoration_text) {
        //Bidi::printf("<span style='color: blue;'>fn=%d, sp=%d</span>", filenumber, subpart);

        if (read_subpart_count != -1 && last_subpart != subpart) {
          read_subpart_count--;
          if (read_subpart_count == 0) break;
        }

        if (b_read_only_one_subpart && last_subpart != subpart) break;
        last_subpart = subpart;

        //Bidi::printf("linecount=%d, filenumber=%d ", linecount, filenumber);
        b_votive_has_extra_commemoration_text = (bIsVotive && (
          s.indexOf("Blasii") != -1 ||
          s.indexOf("Marcellini") != -1 ||
          s.indexOf("Telesphor") != -1 ||
          s.indexOf("Faustini") != -1 ||
          s.indexOf("Undecima") != -1 ||
          (linecount == 7 && filenumber == 2) || (linecount == 6 && filenumber == 11) // remove Oremus and linebreak before extra commemoration lines in Votive Mass propers for Collect and Postcommunio
          ));

        if (!bIsVotive || !b_votive_has_extra_commemoration_text) { // bit of a hack to get around the Votive masses, which have picked up the Commemoration of the day they were taken from (which is not always the day they are shown)
          bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
            pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
            bBold, bItalic, bRed, fontsize_rel,
            line_number, fbwidth, fbheight,
            bRTL, bRenderRtl, bWrapText, bMoreText);
        }
        linecount++;

        //if (b_read_only_one_subpart && first_subpart != subpart) {
        //  b_end_of_subpart_reached_break_now = true; // when only outputting one subsection per hour, eg for Good Friday Great Intercessions
        //}
      }
    }

    if ((bHasImage || (bIsVotive && bHasVotiveImage)) && !b_is_Good_Friday && !b_is_Ash_Wednesday && !b_is_Palm_Sunday) {
      if (ts.Hour == 19) {
        waketime = 20; // at 7pm the Postcommunio is displayed until midnight. If there is an image to display, wake up at 8pm to display the image between 8pm and midnight if so
      }
      else if (ts.Hour >= 0 && ts.Hour < 8) { // will display image between midnight and 8am, so need to wake at 8 to display the Introit for an hour
        waketime = 8;
      }
    }
  }
  else {
    DEBUG_PRT.println(F("No Seasonal, Feast or Votive day! (Mistake in Propers database) - Displaying Error image"));
#ifdef _WIN32    
    Bidi::printf("***********No Seasonal, Feast or Votive day! (Mistake in Propers database) - Displaying Error image<br>");
#endif

    bool bImageIsDisplayed = false;

    if (bHasSeasonImage) {
      bImageIsDisplayed = DisplayImage(seasonimagefilename, 0, ypos);
    }
    else {
      String imagefilename = fileroot_img + "/Error.bwr";
      bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
    }
    waketime = 0;
  }
  votive.close();
  season.close();
  feast.close();

  DEBUG_PRT.println(F("Done. Now flushing remaining text from textbuffer.."));
  tb.flush();

  DEBUG_PRT.println(F("Displaying date.."));
  display_date_ex(date, datestring, sanctoral_day, right_to_left, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);  // shown at the top of the screen. If it is a feast day, the liturgical day is displayed at the                                                               // bottom left. Otherwise the bottom left is left blank.
  tb.flush();

  DEBUG_PRT.println(F("\nCompleted displaying reading - Leaving LatinMassPropers()"));
}

int display_day_ex(time64_t date, String d, String lit_colour, bool holy_day_of_obligation, bool right_to_left, bool bRed, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi) { // uses Bidi::RenderTextEx and more advanced tag handling
  bool bRTL = right_to_left;

  DEBUG_PRT.print(F("display_day_ex() d="));
  DEBUG_PRT.println(String(d));

  d = replacefields(d, date);

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int fbwidth = fb.width();
  int fbheight = fb.height();
#else
  int fbwidth = ePaper.width();
  int fbheight = ePaper.height();
#endif

  if (lit_colour == "red") {
    d = "<FONT COLOR=\"red\">" + d + "</FONT>";
  }

  if (holy_day_of_obligation) {
    d = d + " " + Yml::get("holy_day_of_obligation");
  }

  //int text_xpos = (fbwidth / 2) - (int)((diskfont.GetTextWidthA(d, true))/2); // true => shape text before calculating width
  //DEBUG_PRT.println("display_day() text_xpos = " + String(text_xpos));

  int text_xpos = 0;
  int text_ypos = 0;

  DiskFont* pDiskfont = &diskfont_normal;
  bool bBold = false;
  bool bItalic = false;
  bRed = false;
  int8_t fontsize_rel = 0;
  int8_t line_number = 0;
  int16_t line_height = 0;

#ifdef _WIN32
  Bidi::printf("<span style='font-weight: bold; color: blue; text-decoration: underline'>");
#endif

  Bidi::RenderTextEx(d, &text_xpos, &text_ypos, tb,
    pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
    bBold, bItalic, bRed, fontsize_rel,
    line_number, fbwidth, fbheight,
    bRTL, right_to_left, true, false,
    line_height,
    TB_FORMAT_CENTRE);

#ifdef _WIN32 
  Bidi::printf("</span><br>");
#endif

#ifdef LM_DEBUG
  WriteCalendarDay("<td>" + d + "</td>");
#endif

  //fb.drawFastHLine(0, (int)line_height, fb.width(), GxEPD_BLACK);
#ifdef USE_SPI_RAM_FRAMEBUFFER
  fb.drawFastHLine(0, text_ypos + line_height, fb.width(), GxEPD_BLACK);
#else
  ePaper.drawFastHLine(0, text_ypos + line_height, ePaper.width(), GxEPD_BLACK);
#endif

  return text_ypos + line_height;
}

void display_date_ex(time64_t date, String datestr, String day, bool right_to_left, DiskFont& diskfont_normal, DiskFont& diskfont_i, DiskFont& diskfont_plus1_bi, DiskFont& diskfont_plus2_bi) {
  bool bRTL = right_to_left;

  DEBUG_PRT.print(F("\ndisplay_date: s="));
  DEBUG_PRT.println(datestr);

  day = replacefields(day, date);

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int fbwidth = fb.width();
  int fbheight = fb.height();
#else
  int fbwidth = ePaper.width();
  int fbheight = ePaper.height();
#endif

  int text_xpos = 0;
  int text_ypos = 0;

  DiskFont* pDiskfont = &diskfont_normal;
  bool bBold = false;
  bool bItalic = false;
  bool bRed = false;

  int8_t fontsize_rel = 0;
  int8_t line_number = 0;

  text_xpos = 0; // will set text to be right justified in RenderTextEx call
  text_ypos = fbheight - diskfont_normal._FontHeader.charheight - 1;  // TODO: calculate maximum height of text before rendering rather than relying on the smallest 

  DEBUG_PRT.print(F("display_date_ex() text_xpos = "));
  DEBUG_PRT.print(text_xpos);
  DEBUG_PRT.print(F(", text_ypos = "));
  DEBUG_PRT.println(text_ypos);

#ifdef _WIN32
  Bidi::printf("<span style='font-weight: bold;'>");
#endif

  Bidi::RenderTextEx(datestr, &text_xpos, &text_ypos, tb,            //       (default) font height for the offset
    pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
    bBold, bItalic, bRed, fontsize_rel,
    line_number, fbwidth, fbheight,
    bRTL, right_to_left, false, false,
    TB_FORMAT_RJUSTIFY);

#ifdef _WIN32
  Bidi::printf("</span>");
#endif
  text_xpos = 0;
  text_ypos = fbheight - diskfont_normal._FontHeader.charheight - 1;  // TODO: calculate maximum height of text before rendering rather than relying on the smallest 
  bRed = false; //       (default) font height for the offset

#ifdef LM_DEBUG
  WriteCalendarDay("<td>" + datestr + "</td>");
#endif

  day.replace("Hebdomadam", "Hebd."); // bid of a bodge to make it fit in the bottom right Sanctoral field on the display - should have a measure text function 
  day.replace("Octavam", "Oct.");  // to check if the text fits first, then do the replacement if it does not.

#ifdef _WIN32
  Bidi::printf("<span style='font-weight: bold; color: blue;'>");
#endif
  Bidi::RenderTextEx(day, &text_xpos, &text_ypos, tb,
    pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
    bBold, bItalic, bRed, fontsize_rel,
    line_number, fbwidth, fbheight,
    bRTL, right_to_left, false, false,
    TB_FORMAT_LJUSTIFY);
#ifdef _WIN32
  Bidi::printf("</span>");
#endif

#ifdef LM_DEBUG
  WriteCalendarDay("<td>" + day + "</td>");
#endif
}

int8_t DoPurificationOfMary(MissalReading* p_mr_Day, MissalReading* p_mr_Comm, bool b_is_com,
  int8_t filenumber, int8_t hour, String lang, int8_t mass_type, 
  int& xpos, int& ypos,
  DiskFont* pDiskfont,
  bool& bBold, bool& bItalic, bool& bRed,
  int8_t& fontsize_rel, int8_t& line_number,
  int fbwidth, int fbheight,
  bool& bRTL, bool& bRenderRtl, bool& bWrapText, bool bHasImage, String& imagefilename
)
{
  bool bMoreText = true;

  DEBUG_PRT.println("Purification of Mary propers");

  DEBUG_PRT.print(F("filenumber is "));
  DEBUG_PRT.println(filenumber);

  // 1960[en,la], 1955[en], DivAffla[en], 1910[en], 14 parts:
  const int8_t filenumbers[24]      = { 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13 };
  const int8_t fileparts[24]        = { 0, 1, 2, 3, 4, 0, 1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0 };
  const int8_t comms[3] = { 4, 10, 13 };

  // 1955[la], 1910[la], 12 parts:
  const int8_t filenumbers_12part_la[24]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11 };
  const int8_t fileparts_12part_la[24]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  const int8_t comms_12part_la[3] = { 2, 8, 11 };

  //Trid1570[en], 13 parts
  const int8_t filenumbers_1570[24]   = { 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12 };
  const int8_t fileparts_1570[24]     = { 0, 1, 2, 3, 4, 0, 1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0 };
  const int8_t comms_1570[3] = { 4, 9, 12 };

  //Trid1570[la], 13 parts
  const int8_t filenumbers_1570_la[24]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11 };
  const int8_t fileparts_1570_la[24]    = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0 };
  const int8_t comms_1570_la[3] = { 2, 8, 11 };

  const int8_t comms_indexes[3] = { 2, 8, 11 };

  filenumber = filenumbers[hour];
  int8_t subpart = fileparts[hour];
  bool b_only_show_one_subpart = (hour < 10); // prevents showing more than one subpart for the Blessing of Candles section of the 1570 latin Mass, which has all of the sections before the introitus grouped under one filenumber

  int8_t firstindex = -1;
  bool b_show_commemoration_with = (p_mr_Comm != NULL && b_is_com) && IsInArray(comms, filenumber, firstindex, 0, sizeof(comms));
  int8_t waketime_last_today = 20;

  switch(mass_type) {
  case MASS_TRIDENTINE_1570:
    if (lang == "la") {
      filenumber = filenumbers_1570_la[hour];
      subpart = fileparts_1570_la[hour];
      b_show_commemoration_with = (p_mr_Comm != NULL && b_is_com) && IsInArray(comms_1570_la, filenumber, firstindex, 0, sizeof(comms_1570_la));
      waketime_last_today = 20;
      b_only_show_one_subpart = (hour < 10);
    }
    else {
      filenumber = filenumbers_1570[hour];
      subpart = fileparts_1570[hour];
      b_show_commemoration_with = (p_mr_Comm != NULL && b_is_com) && IsInArray(comms_1570, filenumber, firstindex, 0, sizeof(comms_1570));
      waketime_last_today = 20;
    }
    break;
  
  case MASS_TRIDENTINE_1910:
  case MASS_DIVINEAFFLATU:
  case MASS_1955:
  case MASS_1960:
    if ((mass_type == MASS_TRIDENTINE_1910 || mass_type == MASS_1955) && lang == "la") {
      filenumber = filenumbers_12part_la[hour];
      subpart = fileparts_12part_la[hour];
      b_show_commemoration_with = (p_mr_Comm != NULL && b_is_com) && IsInArray(comms_12part_la, filenumber, firstindex, 0, sizeof(comms_12part_la));
      waketime_last_today = 19;
    }
    else {
      filenumber = filenumbers[hour];
      subpart = fileparts[hour];
      b_show_commemoration_with = (p_mr_Comm != NULL && b_is_com) && IsInArray(comms, filenumber, firstindex, 0, sizeof(comms));
      waketime_last_today = 21;
    }
    break;
  }

  int8_t waketime = hour < waketime_last_today ? hour + 1 :
    !bHasImage && hour == waketime_last_today ? 0 :
    bHasImage && hour == waketime_last_today ? waketime_last_today + 1 : 0;
  
  bool bImageIsDisplayed = false;

  if (hour > waketime_last_today && bHasImage) {
    bImageIsDisplayed = DisplayImage(imagefilename, xpos, ypos);
  }
  
  if (!bImageIsDisplayed) {
    bool bOverflowedScreen = false;
    String s;

    int8_t last_subpart = subpart;

    while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText)) {
      if (b_only_show_one_subpart && last_subpart != subpart) break;
      last_subpart = subpart;

      bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
        pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
        bBold, bItalic, bRed, fontsize_rel,
        line_number, fbwidth, fbheight,
        bRTL, bRenderRtl, bWrapText, bMoreText);

    }

    if (p_mr_Comm != NULL && b_show_commemoration_with) {
      int8_t comm_filenumber = comms_indexes[firstindex];

      String commtext = "<BR><FONT COLOR=\"red\"><I>Commemoratio " + p_mr_Comm->name() + "</I></FONT>";
      if (!bOverflowedScreen) {
        bOverflowedScreen = Bidi::RenderTextEx(commtext, &xpos, &ypos, tb,
          pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
          bBold, bItalic, bRed, fontsize_rel,
          line_number, fbwidth, fbheight,
          bRTL, bRenderRtl, bWrapText, bMoreText);
      }

      p_mr_Comm->get(comm_filenumber, subpart, s, bMoreText); // eat the heading of the commemoration (eg. "Collect")

      while (!bOverflowedScreen && p_mr_Comm->get(comm_filenumber, subpart, s, bMoreText)) {
        bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb,
          pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi,
          bBold, bItalic, bRed, fontsize_rel,
          line_number, fbwidth, fbheight,
          bRTL, bRenderRtl, bWrapText, bMoreText);
      }
      if (s.indexOf("<FONT COLOR=\"red\"><I>R.</I></FONT>") != -1) { bMoreText = false; } // Only output commemoration of the Saint, not commemorations of other Saints on the same feast day
    }
  }
  return waketime;
}

bool IsInArray(const int8_t* ar, int8_t value, int8_t& firstindex, int8_t start, size_t ar_size) {
  firstindex = -1;

  if (start >= ar_size) {
    return false;
  }

  const int8_t* arptr = ar + start;
  int8_t i = start;

  while (i < ar_size && *arptr != value) {
    arptr++;
    i++;
  }

  if (*arptr == value) {
    firstindex = i;
    return true;
  }
  
  return false;
}

String replacefields(String s, time64_t date) {
  s.replace("%{monthweek}", "");  // don't yet understand the calculation for the week number, plus including this tends to overflow the line, so omitting for now.
  s.replace("%{month}", "");
  return s;

  /*
  const char* const months[5] = {"Augusti", "Septembris", "Octobris", "Novembris", "Decembris"};
  const char* const weeks[5] = {"I.", "II.", "III.", "IV.", "V."};

  tmElements_t ts;
  breakTime(date, ts);

  //DEBUG_PRT.printf("Datetime: %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

  if (ts.Month > 7 && ts.Month < 13) {
    time64_t week_start = date;

    if (ts.Wday != 1) {
    week_start = Tridentine::sunday_before(date);
    }

    breakTime(week_start, ts);
    int8_t week_number = 1;
    int8_t dy = ts.Day;

    while (dy - 7 > 1) {
    dy -= 7;
    week_number += 1;
    }

    if (week_number > 5) {
    DEBUG_PRT.println("replacefields() week_number > 5!");
    week_number = 5;
    }

    if (week_number < 1) {
    DEBUG_PRT.println("replacefields() week_number < 1!");
    week_number = 1;
    }

    String m = months[ts.Month - 8];
    String mw = weeks[week_number - 1];

    s.replace("%{monthweek}", mw);
    s.replace("%{month}", m);
  }

  return s;
  */
}

int8_t GetImageCount(String imagefilename) {
  // returns -1 if no image is found
  // returns number of images with imagefilename.bwr, imagefilename-2.bwr etc if imagefilename is found.

  int8_t imagecount = 0;
  bool bFound = false;

  while (SD.exists(imagecount < 1 ? imagefilename + String(F(".bwr")) : imagefilename + String(F("-")) + String(imagecount + 1) + String(F(".bwr")))) {
    DEBUG_PRT.println(String(F("GetImageCount(): found image ")) + (imagecount < 1 ? imagefilename + String(F(".bwr")) : imagefilename + String(F("-")) + String(imagecount + 1) + String(F(".bwr"))));

    imagecount++;
    bFound = true;
  }

  return bFound ? imagecount : -1;
}

void GetImageFilenameAndWakeTime(String& imagefilename, uint8_t Hour, int8_t imagecount, int8_t& waketime) {
  DEBUG_PRT.print(F("GetImageFilenameAndWakeTime() Hour="));
  DEBUG_PRT.print(Hour);
  DEBUG_PRT.print(F(" imagecount="));
  DEBUG_PRT.println(imagecount);

  if (imagecount <= 1) return; // only one image, so waketime will be default and imagefilename need not be modified

  int8_t imagenumber = 1;

  if (Hour >= 20 && Hour < 24) { // evening
    switch (imagecount) {
    case 2:
      imagenumber = (Hour >= 20 && Hour < 22) ? 1 : 2;
      waketime = (Hour >= 20 && Hour < 22) ? 22 : -1; // -1 indicates use default waketime - will be midnight
      break;

    case 3:
      imagenumber = (Hour == 20) ? 1 :
        (Hour == 21) ? 2 : 3;

      waketime = (Hour == 20) ? 21 :
        (Hour == 21) ? 22 : -1; // -1 indicates use default waketime - will be midnight
      break;

    default:
      return;
      break;
    }

    DEBUG_PRT.print(F("GetImageFilenameAndWakeTime() imagenumber="));
    DEBUG_PRT.print(waketime);
    DEBUG_PRT.print(F(" waketime="));
    DEBUG_PRT.println(waketime);

    if (imagenumber == 1) return; // no need to modify the filename for first image (no "-1.bwr" - subpart number is omitted in naming scheme for first image)
    imagefilename = imagefilename.substring(0, imagefilename.lastIndexOf(".bwr")) + String(F("-")) + String(imagenumber) + String(F(".bwr"));

    DEBUG_PRT.print(F("GetImageFilenameAndWakeTime() calculated imagefilename="));
    DEBUG_PRT.println(imagefilename);
  }

  return;
}

bool DisplayImage(String filename, int16_t xpos, int16_t ypos) {
#ifdef LM_DEBUG
  return false;
#endif

  // Display 8 greyscale image from SD card in native format for display (4bpp, b3=red/black)
  if (!SD.exists(filename)) {
    DEBUG_PRT.print(F("DisplayImage() Image ["));
    DEBUG_PRT.print(filename);
    DEBUG_PRT.println(F("] not found"));
    return false;

#ifdef _WIN32
    Bidi::printf("DisplayImage() Image [%s] not found<br>", filename.c_str());
#endif
  }

  DEBUG_PRT.print(F("DisplayImage() Displaying image ["));
  DEBUG_PRT.print(filename);
  DEBUG_PRT.println(F("]"));

#ifdef _WIN32
  Bidi::printf("DisplayImage() Displaying image [%s]<br>", filename.c_str());
#endif

#ifdef USE_SPI_RAM_FRAMEBUFFER
  int16_t fbwidth = fb.width();
  int16_t fbheight = fb.height();
#else
  int16_t fbwidth = ePaper.width();
  int16_t fbheight = ePaper.height();
#endif

  File fileImg = SD.open(filename, sdfat::O_READ);
  int x = xpos;
  int y = ypos;
  int pxNum = 0;
  uint8_t px2 = 0;

  while (fileImg.available() && x < fbwidth && y < fbheight) {

    if (!(pxNum & 0x1)) px2 = fileImg.read();

    uint8_t px = !(pxNum & 0x1) ? px2 >> 4 : px2 & 0x0F; // even pixel is high 4 bits, odd pixel is low 4 bits
    uint16_t colour = px & 0x08 ? GxEPD_RED : GxEPD_BLACK; //check bit 4, colour is GxEPD_RED if set

    if (x < fbwidth && y < fbheight) { // clip unless. Optimisation (TODO): don't draw white pixels to save time, since the background is cleared before drawing begins anyway
#ifdef USE_SPI_RAM_FRAMEBUFFER
      fb.drawPixel(x, y, colour, (px & 0x7) << 1); // drawPixel takes a 4 bit value for saturation, but throws away the lsbit for 8 grey/red shades (rather than 16)
#else
      ePaper.drawPixel(x, y, colour, (px & 0x7) << 1); // drawPixel takes a 4 bit value for saturation, but throws away the lsbit for 8 grey/red shades (rather than 16)
#endif
    }

    x++;
    if (x == fbwidth) {
      y++;
      x = xpos;
    }
    pxNum++;
  }

  fileImg.close();
  return true;
}

#ifdef LM_DEBUG
//File cfile;
//bool cclosefile = false;

void WriteCalendarDay(String text) {
  //if (!cfile) {
  //  cfile = SD.open("/calendar.txt", sdfat::O_RDWR | sdfat::O_APPEND);
  if (!cfile) {
    SD.remove("/calendar.txt");
    cfile = SD.open("/calendar.txt", sdfat::O_RDWR | sdfat::O_CREAT);
  }
  //}

  if (cfile) {
    cfile.print(text);
  }

  if (cclosefile) {
    cfile.close();
  }
}
#endif
