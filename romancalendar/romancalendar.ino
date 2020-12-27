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
#include <SD.h>
#define FS_NO_GLOBALS
#include <FS.h>

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
/*
#ifdef USE_SPI_RAM_FRAMEBUFFER
  int16_t x=10;
  int16_t y=150;
  for (int16_t j = y; j < y+16; j++) {
    for (int16_t i = x; i < x+16; i++) {
      for(int16_t k=0; k<8; k++) {
        int16_t xp = i + (16*k);
        fb.drawPixel(xp, j, GxEPD_BLACK, k*2, false);
        fb.drawPixel(xp, j+16, GxEPD_BLACK, k*2, true);
        fb.drawPixel(xp, j+32, GxEPD_RED, k*2, false);
        fb.drawPixel(xp, j+48, GxEPD_RED, k*2, true);
      }
    }
  }

#endif
*/
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
  if(!f_spiffs) return;

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
 * It should be kept quick / consise to be able to execute before hardware wdt may kick in
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

  SD.begin(D1, SPI_HALF_SPEED);
    
  SPIFFS.begin();
  //DEBUG_PRT.begin(1,3,8);
  DEBUGPRT_BEGIN
  delay(100);
  //Serial.println("-");
  
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
  
  DEBUG_PRT.println(F("Running"));

  //WiFi.disconnect(); // testing - so have to connect to a network each reboot

  battery_test();

  clock_battery_test();

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
  }
}

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
    time64_t date;
    Config::getLocalDateTime(&date);

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
  
    bool bLatinMass = c._I18n->configparams.lectionary_path.startsWith("Lect/EF"); //true;
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
  if (!bDisplayWifiConnectedScreen) return; // only run the webserver if the wifi connected screen is shown (will only show once, when power is first connected). So after resetting (eg. when settings have been updated) it the webserver should not be run

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
    DEBUG_PRT.println(F("Power is connected, starting config web server and OTA update server"));
    DEBUG_PRT.print(F("USB voltage is "));
    DEBUG_PRT.println(String(Battery::battery_voltage()));

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

    unsigned long server_start_time = millis();
    bool bTimeUp = false;
   
    if (Config::StartServer(lang)) {
      DEBUG_PRT.println(F("Config web server started, listening for requests..."));

      #define FINALDELAY 70
      int finaldelay = FINALDELAY; // this should give time for the "settings updated" page to download its UI language JSON file (7 seconds max)
      while(Battery::power_connected() && (!Config::bSettingsUpdated || finaldelay > 0) && !bTimeUp && !Config::bComplete) {
        server.handleClient();  // Web server
        ArduinoOTA.handle(); // PLL-27-04-2020 OTA Update server
        wdt_reset();
        delay(100);
        //DEBUG_PRT.println("Battery voltage is " + String(Battery::battery_voltage()));

        if (Config::bSettingsUpdated) {
          if (finaldelay == FINALDELAY) { // first time entering this, clock will just have been set, so power line enable from clock chip will be off, but still held up by USB 5V input
            Config::SetPowerOn();         // hold up the power enable line. If this is not done, then if USB power is disconnected before reset (but after settings updated), ESP8266 will not restart
          }                               // if USB power is disconnected after update but before reset, then the alarm should trigger in 3 seconds and switch on the ESP8266 rather than immediately
          finaldelay--;
        }

        if (millis() > (server_start_time + 1000*8*60)) {
          bTimeUp = true; // run the server for an 10 minutes max, then sleep. If still on usb power, the web server will run again.
        }
      }

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
    time64_t date;
    Config::getLocalDateTime(&date);
    
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
    time64_t date;
    Config::getLocalDateTime(&date);
    
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
    time64_t date;
    Config::getLocalDateTime(&date);
    
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

bool DoLatinMassPropers(time64_t date, String datestring, bool right_to_left, String lectionary_path, String lang, int8_t& waketime) {
  DEBUG_PRT.println(F("Displaying calendar"));

#ifdef USE_SPI_RAM_FRAMEBUFFER
  fb.cls();   // clear framebuffer ready for text to be drawn
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

  
  //Yml::SetConfig(lang, "en-1962.yml", "en-1962.txt"); //removed - using Tridentine::GetFileDir function instead - PLL 19-10-2020
  Tr_Calendar_Day td;
  //Tridentine::get(date, td, true);
  Tridentine::GetFileDir2(date, td.FileDir_Season, td.FileDir_Saint, td.FileDir_Votive, td.HolyDayOfObligation, td.ImageFilename, td.VotiveImageFilename);
  
  LatinMassPropers(date, datestring, td, tb, lectionary_path, lang, fbwidth, fbheight, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, right_to_left, waketime);
      
  updateDisplay(display_reading, diskfont_normal._FontHeader.antialias_level);
  DEBUG_PRT.println(F("OK"));
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
                      int8_t& waketime) {
  
  // setup fonts and variables for text output

  waketime = -1; // tell caller to use default waketime

  bool bBold = false;
  bool bItalic = false;
  bool bRed = false;
  int8_t fontsize_rel = 0;

  DEBUG_PRT.print(F("Done"));
  DEBUG_PRT.print(F("Loading fonts.."));
  diskfont_normal.begin("/Fonts/droid11.lft");
  diskfont_i.begin("/Fonts/droid11i.lft");
  diskfont_plus1_bi.begin("/Fonts/droi12bi.lft");
  diskfont_plus2_bi.begin("/Fonts/droi13bi.lft");
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

  tmElements_t ts;
  breakTime(date, ts);

  DEBUG_PRT.printf("Datetime: %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
  //DEBUG_PRT.println(td.DayofWeek);
  //DEBUG_PRT.println(td.Cls);
  //DEBUG_PRT.println(td.Colour);
  //DEBUG_PRT.println(td.Mass);
  //DEBUG_PRT.println(td.Commemoration);
  DEBUG_PRT.println(td.HolyDayOfObligation);
  DEBUG_PRT.println(td.FileDir_Season);
  DEBUG_PRT.println(td.FileDir_Saint);
  DEBUG_PRT.println(td.ImageFilename);

  int8_t filenumber = ts.Hour - 8;    // (Done) - TODO: need to have special cases for Easter, All Souls and Christmas (which have a larger number of parts/different structure to standard 12-part propers)

  if (ts.Hour >= 0 && ts.Hour < 9) {
    filenumber = 0;
  }

  if (ts.Hour >= 19) {
    filenumber = 11;
  }

  int8_t next_hour_filenumber = filenumber + 1;

  //String fileroot = "/Lect/EF/" + mass_version + "/" + lang; // test code removed
  String fileroot = "/" + lect + "/" + lang;
  String fileroot_img = "/" + lect + "/images";
  String liturgical_day = "";
  String sanctoral_day = "";
  String imagefilename = td.ImageFilename != "" ? fileroot_img + td.ImageFilename + ".bwr" : "";
  String votiveimagefilename = td.VotiveImageFilename != "" ? fileroot_img + td.VotiveImageFilename + ".bwr" : "";

  bool bFeastDayOnly = (td.FileDir_Season == td.FileDir_Saint); // will be true if there is no seasonal day
  bool bIsFeast = SD.exists(fileroot + td.FileDir_Saint); // if there is no feast day on this day, the directory/propers for this day will not exist
  bool bIsVotive = (td.FileDir_Votive != "" && SD.exists(fileroot + td.FileDir_Votive)); // if there is no votive day on this day, the directory/propers for this day will not exist
  bool bHasImage = (td.ImageFilename != "" && SD.exists(imagefilename)); // if there is an image accompanying the Saint's day on this day, it will be displayed between midnight and 8am, before the introit at 9am
  bool bHasVotiveImage = (td.VotiveImageFilename != "" && SD.exists(votiveimagefilename)); // if the Mass for the day is a Votive Mass, an image will be displayed if available
  int8_t imagecount = -1;
  int8_t votiveimagecount = -1;

  // image filenames are of the form 10-11.bwr, and 10-11-2.bwr, 10-11-3.bwr etc if there is more than one image
  if (bHasImage) imagecount = GetImageCount(fileroot_img + td.ImageFilename); // get count of images to be displayed this day
  if (bHasVotiveImage) votiveimagecount = GetImageCount(fileroot_img + td.VotiveImageFilename); // get count of votive images to be displayed this day (if applicable)
  
  DEBUG_PRT.print(F("imagefilename is ["));
  DEBUG_PRT.print(imagefilename);
  DEBUG_PRT.print(F("]\nvotiveimagefilename is ["));
  DEBUG_PRT.print(votiveimagefilename);
  DEBUG_PRT.print(F("]\nbHasImage is "));
  DEBUG_PRT.println(bHasImage);
  DEBUG_PRT.print(F("bHasVotiveImage is "));
  DEBUG_PRT.println(bHasVotiveImage);
  DEBUG_PRT.print(F("imagecount="));
  DEBUG_PRT.println(imagecount);
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
  
  season.open(fileroot + td.FileDir_Season);

  bool bSunday = Tridentine::sunday(date);
  int8_t cls_season = Tridentine::getClassIndex(season.cls());
  int8_t cls_feast = Tridentine::getClassIndex(feast.cls());

  // need to exclude commemoration on Sundays and Feasts of the Lord
  bIsFeast = (bFeastDayOnly || !bSunday && cls_feast >= cls_season || cls_feast >= 6);
  bFeastDayOnly = (bFeastDayOnly || cls_feast >= 6 && !Tridentine::IsImmaculateConception(date)/*Tridentine::Season(date) != SEASON_ADVENT*/); // If Feast of the Lord, Class I or Duplex I classis, don't show seasonal day
  // Feast of Immaculate Conception (8 Dec, even if Sunday), the day of Advent is the Commemorio (there may be other exceptions)
  // PLL-26-12-2020 Added bFeastDayOnly || check so that if the test Saints Day == the Feast Day (above) then this overrides the rest of this test
  
  // If it is a votive day and the feast is of the same or lower priority, 
  // or if it is a seasonal day and the seasonal day is of the same or lower priority, the votive mass is observed
  bIsVotive = (bIsVotive && ((bIsFeast && Tridentine::getClassIndex(feast.cls()) <= Tridentine::getClassIndex(votive.cls())) || (!bIsFeast && Tridentine::getClassIndex(season.cls()) <= Tridentine::getClassIndex(votive.cls()))));

  if (bIsVotive) {
    DEBUG_PRT.println(F("Votive Mass"));
  }
  
  // Now have the indexrecords for the season and saint (if also a feast), and the filepointers pointing to the start of the text.

    //if (Tridentine::getClassIndex(indexheader_saint.cls) > Tridentine::getClassIndex(indexheader_season.cls)) {
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

  bool bFeastIsCommemoration = false;
  bool bFileOk = false;
  String s = "";
  bool bOverflowedScreen = true;
  int8_t subpart = 0;
  bool bRTL = right_to_left;
  
  if (bIsFeast) { // there is a feast on this day
    if (bFeastDayOnly) { // is a saint's feast only
      DEBUG_PRT.println(F("Feast day only"));
      ypos = display_day_ex(date, feast.name(), feast.colour(), td.HolyDayOfObligation, right_to_left, bLiturgical_Colour_Red, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);   // feast day is displayed at the top of the screen on feast days otherwise the liturgical day is 

      if (bHasImage) {
        if (ts.Hour == 19) { // at 7pm, if there are any images to display, wake at 8pm to begin displaying them
          waketime = ts.Hour + 1;
        }
        else { // between 8pm and midnight
          GetImageFilenameAndWakeTime(imagefilename, ts.Hour, imagecount, waketime);
        }
      }
      
      if (!(bHasImage && ts.Hour > 19 && DisplayImage(imagefilename, 0, ypos))) { // from 8pm until midnight, display the Saint's image (if available)
        bOverflowedScreen = false;
        subpart = 0;
        while (!bOverflowedScreen && feast.get(filenumber, subpart, s, bMoreText)) {
          bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb, 
                            pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                            bBold, bItalic, bRed, fontsize_rel, 
                            line_number, fbwidth, fbheight, 
                            bRTL, bRenderRtl, bWrapText, bMoreText);
        }
      }
    }
    else { // there is both a feast and a seasonal day
      DEBUG_PRT.println(F("Feast day and seasonal day"));
      MissalReading* p_mr_Day = &feast;
      MissalReading* p_mr_Comm = &season;

      // PLL-24-12-2020 This needs understanding and fixing!
      bool bSaintsDayTakesPrecedence = (!(cls_feast == cls_season)); // Seasonal day takes precedence if of same class as the feast day (usually with class IV commemorations)
      bool bDisplayComm = (cls_feast >= cls_season && cls_season >= 2 /*Semiduplex*/); // PLL-15-12-2020 display Commemoration if the seasonal day is >= SemiDuplex
      /* // now handled above PLL-20-10-2020 - maybe display the seasonal day in the superior position if the feast and seasonal day are of the same class?
      // need to exclude commemoration on Sundays and Feasts of the Lord
      bool bSaintsDayTakesPrecedence = (Tridentine::getClassIndex(feast.cls()) >= Tridentine::getClassIndex(season.cls()));
      
      if (Tridentine::sunday(date)) { // TODO: may need to make allowance for feasts that move, such as feast of St Matthias, and for feasts of the Lord which override Sundays
        bSaintsDayTakesPrecedence = false;
      }
      */
            
      if (!bSaintsDayTakesPrecedence) {
        p_mr_Day = &season;
        p_mr_Comm = &feast;
      }

      if (bIsVotive) { // in this case, the feast is shown in the commemoration position (bottom left of the screen), and the votive is shown as the heading. The seasonal day is not shown
        p_mr_Day = &votive;
        p_mr_Comm = &feast; 
      }
            
      sanctoral_day = p_mr_Comm->name();
      ypos = display_day_ex(date, p_mr_Day->name(), p_mr_Day->colour(), td.HolyDayOfObligation, right_to_left, bLiturgical_Colour_Red, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);   // feast day is displayed at the top of the screen on feast days otherwise the liturgical day is

      DEBUG_PRT.print(F("filenumber is "));
      DEBUG_PRT.println(filenumber);
      
      // Feast day takes precedence, seasonal day is commemoration
      switch(filenumber) {
        case 0: // Introitus (from the feast)  
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
            bMoreText = false;
            s = "";
                    
        case 3: // Lesson
        case 4: // Gradual
        case 7: // Offertorium
        case 10: // Communio
        
          if (bHasImage && ts.Hour >= 0 && ts.Hour <= 7) {
            waketime = 8;
          }
      
          if (!(bHasImage && ts.Hour >= 0 && ts.Hour <= 7 && DisplayImage(imagefilename, 0, ypos))) { // from midnight until 8am, display the Saint's image (if available)
            bOverflowedScreen = false;
            subpart = 0;
            while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText)) {
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
        case 9: // Prefatio
          bOverflowedScreen = false;
          subpart = 0;

          while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText)) {           
            bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb, 
                              pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                              bBold, bItalic, bRed, fontsize_rel, 
                              line_number, fbwidth, fbheight, 
                              bRTL, bRenderRtl, bWrapText, bMoreText);
          }
        break;

        case 2: // Collect
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
              while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText)) {
                bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb, 
                                  pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                                  bBold, bItalic, bRed, fontsize_rel, 
                                  line_number, fbwidth, fbheight, 
                                  bRTL, bRenderRtl, bWrapText, bMoreText);
              }
              //// TODO: PLL-25-07-2020 Find out how this works! 
              //         PLL-15-12-2020 Found a problem with days of Advent, patched, though still not sure how it works
              if (!bSaintsDayTakesPrecedence || bDisplayComm) {
                //if (!bOverflowedScreen && (indexrecord_saint.filenumber == 2 || indexrecord_saint.filenumber == 11)) {   // do a line feed before the text of the commemoration
                if (!bOverflowedScreen && (filenumber == 2 || filenumber == 11)) {   // do a line feed before the text of the commemoration
                  String crlf = F(" <BR>"); // hack: the space char should give a line height to the typesetter, otherwise it would be 0 since there are no printing characters on the line
                  
                  bOverflowedScreen = Bidi::RenderTextEx(crlf, &xpos, &ypos, tb, 
                                  pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                                  bBold, bItalic, bRed, fontsize_rel, 
                                  line_number, fbwidth, fbheight, 
                                  bRTL, bRenderRtl, bWrapText, true);
                }          
                
                int8_t linecount = 0;
                subpart = 0;
                p_mr_Comm->get(filenumber, subpart, s, bMoreText); // eat a line (the heading), should then get a <BR> on its own line after the heading
                while (!bOverflowedScreen && p_mr_Comm->get(filenumber, subpart, s, bMoreText)) {
                  if ((linecount == 0 && filenumber == 8) ||                       // commemorio line is first line in this case
                      (linecount == 2 && (filenumber == 2 || filenumber == 11))) { // commemorio line comes after <br>"Let us pray"<br> line
                    String commtext = "<BR><FONT COLOR=\"red\"><I>Commemorio " + p_mr_Comm->name() + "</I></FONT><BR>";
                    if (!bOverflowedScreen) {
                      bOverflowedScreen = Bidi::RenderTextEx(commtext, &xpos, &ypos, tb, 
                                          pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                                          bBold, bItalic, bRed, fontsize_rel, 
                                          line_number, fbwidth, fbheight, 
                                          bRTL, bRenderRtl, bWrapText, true);
                    }          
                  }
                  
                  if (!bOverflowedScreen) {
                    bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb, 
                                    pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi,  diskfont_plus2_bi, 
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
  }
  else { // is a seasonal day only
    DEBUG_PRT.println(F("Seasonal day only"));
    ypos = display_day_ex(date, season.name(), season.colour(), td.HolyDayOfObligation, right_to_left, bLiturgical_Colour_Red, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);   // feast day is displayed at the top of the screen on feast days otherwise the liturgical day is 

    subpart = 0;
    bool bImageIsDisplayed = false;

    if (season.filecount != 12) {
      if (Tridentine::IsGoodFriday(date)) {      
        DEBUG_PRT.print(F("Good Friday: ts.Hour="));
        DEBUG_PRT.println(ts.Hour);

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

        /*
        if (ts.Hour >= 15 && ts.Hour <= 18) {
          filenumber = 4; // Adoration of the Cross
          subpart = ts.Hour - 15; // 0..3
          waketime = ts.Hour + 1;
        }
        */
        
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
          subpart = ts.Hour - 19 + hour_offset_for_image_display; // 0 or 1
          if (ts.Hour == 19 + hour_offset_for_image_display) {
            waketime = 20 + hour_offset_for_image_display;
          }
          else {
            //waketime = 0; //Allow to default (will be midnight)
          }
        }
      }
      else if (Tridentine::IsHolySaturday(date)) {
        if (ts.Hour <= 21) { // 22 file parts
          filenumber = ts.Hour;
          subpart = 0;
          waketime = ts.Hour < 21 ? ts.Hour + 1 : 0;
        }
      }
    }
    else { // if is a 12-part Proper
      if (next_hour_filenumber == 1 || next_hour_filenumber == 6) { // if the next reading will be Gloria or Credo
        subpart = 0;
        season.get(next_hour_filenumber, subpart, s, bMoreText);
        if (season.curr_subpartlen < 200) { 
          // if they are < 200 bytes, probably means the Gloria or Credo is omitted for this day, so skip the next hour's reading (filenumber == 1 or 6)
          DEBUG_PRT.println(F("Skipping next hour because Gloria or Credo is omitted today (seasonal day only)"));
          if (next_hour_filenumber == 6) {
            waketime = ts.Hour + 2; // skip the next hour's reading            
          }
          else {
            waketime = 10; // skip the reading at 9AM (Gloria is at 9AM)
          }
        }
        bMoreText = false;
        s = "";
      }
    }
    
    // image will be displayed (if available) in this case between midnight and 8am, and 8pm and midnight
    if (bIsVotive && bHasVotiveImage && ((ts.Hour >= 0 && ts.Hour < 8) || (ts.Hour >= 20 && ts.Hour < 24))) { // display image for votive Mass if so
      GetImageFilenameAndWakeTime(votiveimagefilename, ts.Hour, votiveimagecount, waketime);    
      bImageIsDisplayed = DisplayImage(votiveimagefilename, 0, ypos);
    }
    else {
      if (bHasImage && ((ts.Hour >= 0 && ts.Hour < 8) || (ts.Hour >= 20 && ts.Hour < 24))) { // display image if so
        GetImageFilenameAndWakeTime(imagefilename, ts.Hour, imagecount, waketime);    
        bImageIsDisplayed = DisplayImage(imagefilename, 0, ypos);
      }
    }

    MissalReading* p_mr_Day = bIsVotive ? &votive : &season;

    subpart = 0;
    bOverflowedScreen = false;

    if (!bImageIsDisplayed) {
      while (!bOverflowedScreen && p_mr_Day->get(filenumber, subpart, s, bMoreText)) {
        bOverflowedScreen = Bidi::RenderTextEx(s, &xpos, &ypos, tb, 
                          pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                          bBold, bItalic, bRed, fontsize_rel, 
                          line_number, fbwidth, fbheight, 
                          bRTL, bRenderRtl, bWrapText, bMoreText);
      }
    }
    
    if (bHasImage || bHasVotiveImage) {
      if (ts.Hour == 19) {
        waketime = 20; // at 7pm the Postcommunio is displayed until midnight. If there is an image to display, wake up at 8pm to display the image between 8pm and midnight if so
      }
      else if (ts.Hour >= 0 && ts.Hour < 8) { // will display image between midnight and 8am, so need to wake at 8 to display the Introit for an hour
        waketime = 8;
      }
    }
  }

  votive.close();
  season.close();
  feast.close();

  DEBUG_PRT.println(F("Done. Now flushing remaining text from textbuffer.."));
  tb.flush();

  DEBUG_PRT.println(F("Displaying date.."));
  display_date_ex(date, datestring, sanctoral_day, right_to_left, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi);  // shown at the top of the screen. If it is a feast day, the liturgical day is displayed at the                                                               // bottom left. Otherwise the bottom left is left blank.
  tb.flush();

  DEBUG_PRT.println(F("Completed displaying reading - Leaving LatinMassPropers()"));
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
  
  Bidi::RenderTextEx(d, &text_xpos, &text_ypos, tb, 
                     pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                     bBold, bItalic, bRed, fontsize_rel, 
                     line_number, fbwidth, fbheight, 
                     bRTL, right_to_left, true, false,
                     line_height, 
                     TB_FORMAT_CENTRE);

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
  
  Bidi::RenderTextEx(datestr, &text_xpos, &text_ypos, tb,            //       (default) font height for the offset
                     pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                     bBold, bItalic, bRed, fontsize_rel, 
                     line_number, fbwidth, fbheight, 
                     bRTL, right_to_left, false, false,
                     TB_FORMAT_RJUSTIFY);

  text_xpos = 0;
  text_ypos = fbheight - diskfont_normal._FontHeader.charheight - 1;  // TODO: calculate maximum height of text before rendering rather than relying on the smallest 
  bRed = false; //       (default) font height for the offset

  day.replace("Hebdomadam", "Hebd."); // bid of a bodge to make it fit in the bottom right Sanctoral field on the display - should have a measure text function 
  day.replace("Octavam", "Oct.");  // to check if the text fits first, then do the replacement if it does not.
                                      
  Bidi::RenderTextEx(day, &text_xpos, &text_ypos, tb,             
                     pDiskfont, diskfont_normal, diskfont_i, diskfont_plus1_bi, diskfont_plus2_bi, 
                     bBold, bItalic, bRed, fontsize_rel, 
                     line_number, fbwidth, fbheight, 
                     bRTL, right_to_left, false, false,
                     TB_FORMAT_LJUSTIFY);
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

  while(SD.exists(imagecount < 1 ? imagefilename + String(F(".bwr")) : imagefilename + String(F("-")) + String(imagecount + 1) + String(F(".bwr")))) {
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
        waketime    = (Hour >= 20 && Hour < 22) ? 22 : -1; // -1 indicates use default waketime - will be midnight
        break;
      
      case 3:
        imagenumber = (Hour == 20) ? 1 : 
                      (Hour == 21) ? 2 : 3;

        waketime    = (Hour == 20) ? 21 : 
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
  // Display 8 greyscale image from SD card in native format for display (4bpp, b3=red/black)
  if (!SD.exists(filename)) {
    DEBUG_PRT.print(F("DisplayImage() Image ["));
    DEBUG_PRT.print(filename);
    DEBUG_PRT.println(F("] not found"));
    return false;
  }

  DEBUG_PRT.print(F("DisplayImage() Displaying image ["));
  DEBUG_PRT.print(filename);
  DEBUG_PRT.println(F("]"));
  
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
  
  while(fileImg.available() && x < fbwidth && y < fbheight) {
    
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
