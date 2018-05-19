//#include <LiquidCrystal.h>

// Catholic Lectionary on ESP
// Copyright (c) 2017 Philip Lishman, All rights reserved.

extern const int COLORED    = 1;
extern const int UNCOLORED  = 0;

extern const int PANEL_SIZE_X = 264;
extern const int PANEL_SIZE_Y = 176;

//ESP8266---
#include "ESP8266WiFi.h"
//----------
#include <pins_arduino.h>
#include <I2CSerialPort.h>
#include <TimeLib.h>
#include <Enums.h>
#include <I18n.h>
#include <Csv.h>
#include <Calendar.h>
#include <Temporale.h>
#include <Lectionary.h>
#include <Bible.h>
#include <SPI.h>
#include <epd2in7b.h>
#include <epdpaint.h>
#include <calibri10pt.h>
#include <edb.h>
#include <pgmspace.h>
#include <Network.h>
#include <Battery.h>
#include <Config.h>
#include <images.h>
#include <DiskFont.h>

extern "C" {
#include "user_interface.h"
}

//#define COLORED     1
//#define UNCOLORED   0

//#define PANEL_SIZE_X 264
//#define PANEL_SIZE_Y 176

#define SLEEP_HOUR 60*60e6

I2CSerialPort I2CSerial;

Epd epd;
//DiskFont diskfont;

unsigned char image[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];
unsigned char image_red[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];

//-- for EDB
File dbFile;
EDB db(&writer, &reader);
#include <BibleVerse.h>
//--

Battery battery;
Network network;

bool bEEPROM_checksum_good = false;

void setup() { 
  //Serial.begin(9600);
  I2CSerial.begin(1,3,8);

  I2CSerial.println("--------------------------");
  I2CSerial.println("abcdefghijklmnopqrstuvwxyz");
  I2CSerial.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  I2CSerial.println("0123456789");
  I2CSerial.println("--------------------------");

  //while(!Serial) {
  //}
  
  I2CSerial.println("Running");

  //WiFi.disconnect(); // testing - so have to connect to a network each reboot

  battery_test();
//  display_image(battery_recharge_image); // testing

//  while(1){
//    delay(5000);
//    Serial.print(".");
//  }

  if (battery.power_connected()) {
    if (!network.connect()){
      //I2CSerial.println("Need to configure Wifi with WPS for time service");
      I2CSerial.println("Need to configure Wifi with WPS to enable web configuration");
      I2CSerial.println("On USB power and no network configured: Prompting user to connect using WPS button");
      if (!connect_wps()) {
        I2CSerial.println("Failed to configure Wifi network via WPS - sleeping until USB power is reconnected");
        display_image(connect_power_image);
        ESP.deepSleep(0); // sleep indefinitely, reset pulse will wake the ESP when USB power is unplugged and plugged in again. //sleep for an hour (71minutes is the maximum!), or until power is connected SLEEP_HOUR
      } else {
        ESP.deepSleep(1e6); // reset esp, network is configured
        //ESP.reset();
      }
    }
  }
  else {
    I2CSerial.println("On battery power - not attempting to connect to network. Connect power to use lectionary setup (http://lectionary.local/).");
    //I2CSerial.println("On battery power and no network configured: Sleeping until USB power is attached and network is configured");
    //display_image(connect_power_image);
    //while(1) {
    //  delay(5000);
    //}
    //ESP.deepSleep(SLEEP_HOUR); // sleep for an hour, or until power is connected
  }

  // Check if EEPROM checksum is good

  Config c;
  bEEPROM_checksum_good = c.EEPROMChecksumValid();

  if (!bEEPROM_checksum_good && !battery.power_connected()) {
    I2CSerial.printf("On battery power and EEPROM checksum invalid: Sleeping until USB power is attached and web interface is used to configure EEPROM");
    display_image(connect_power_image);
    ESP.deepSleep(0); //sleep until USB power is reconnected // sleep for an hour (71minutes is the maximum!), or until power is connected (SLEEP_HOUR)
  }
}

void battery_test() {
  I2CSerial.println("Battery voltage is " + String(battery.battery_voltage()));
  if (!battery.power_connected()) {
    if (battery.recharge_level_reached()) {
      I2CSerial.println("Battery recharge level is " + String(MIN_BATT_VOLTAGE));
      I2CSerial.println("Battery recharge level reached - sleeping until power is connected");
      display_image(battery_recharge_image);
      //while(!battery.power_connected()) {
      //  wdt_reset();
      //  delay(2000); // testing - when finished, will be sleep (indefinite, wakes when charger is connected through reset pulse)
      //}
      ESP.deepSleep(0); // sleep indefinitely (will wake when the power is connected, which applies a reset pulse) //sleep for an hour or until power is connected SLEEP_HOUR
    }
  }
  else {
    I2CSerial.println("Battery is charging");
  }
}

bool connect_wps(){
  I2CSerial.println("Please press WPS button on your router.\n Press any key to continue...");
  display_image(wps_connect_image);
  wdt_reset();
  delay(10000);
  bool connected = network.startWPSPBC();      
    
  if (!connected) {
    I2CSerial.println("Failed to connect with WPS :-(");  
    return false;
  }
  
  return true;
}

void display_image(EPD_DISPLAY_IMAGE i) {
  if (i.bitmap_black != NULL) {
    memcpy(image, i.bitmap_black, i.bitmap_bytecount);
  }
  
  if (i.bitmap_red != NULL) {
    memcpy(image_red, i.bitmap_red, i.bitmap_bytecount);
  }
  
  init_panel();
  Paint paint(image, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  if (i.bitmap_black == NULL) {
    paint.Clear(UNCOLORED);
  }
  
  Paint paint_red(image_red, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  if (i.bitmap_red == NULL) {
    paint_red.Clear(UNCOLORED);    
  }

  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.TransmitPartialRed(paint_red.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  

  epd.DisplayFrame();
  epd.Sleep();
}

void loop(void) { 
  /************************************************/ 
  // *1* Create calendar object and load config.csv
  wdt_reset();

  I2CSerial.println("*1*\n");
  //timeserver.gps_wake();
  Calendar c(D1);

  if (bEEPROM_checksum_good) {  
    if (!c._I18n->_have_config) {
      I2CSerial.println("Error: Failed to get config: config.csv is missing or bad or no SD card inserted");
      display_image(sd_card_not_inserted_image);
      ESP.deepSleep(SLEEP_HOUR);    
    }

    /************************************************/ 
    // *2* Get date and time from DS3231 
    wdt_reset();
    I2CSerial.println("*2*\n");
    //time64_t date = c.temporale->date(12,11,2017);
    I2CSerial.println("Getting datetime...");
    //time64_t date = timeserver.local_datetime();
    time64_t date;
    c._config->getDS3231DateTime(&date);
    tmElements_t ts;
    breakTime(date, ts);
  
    //while (!network.get_ntp_time(&date)) {
    //  Serial.print(".");
    //  delay(1000);
    //}
    I2CSerial.println("Got datetime.");
    //network.wifi_sleep(); // no longer needed, sleep wifi to save power
  
  
  
    /************************************************/ 
    // *3* get Bible reference for date (largest task)
    wdt_reset();
    I2CSerial.println("*3*\n");
    c.get(date);
  
  
  
    /************************************************/ 
    // *4* Make calendar entry text string for day
    I2CSerial.println("*4*\n");
    String mth = c._I18n->get("month." + String(ts.Month));
    //String datetime = String(ts.Day) + " " + String(m) + " " + String(ts.Year + 1970);
    String datetime = String(ts.Day) + " " + mth + " " + String(ts.Year + 1970);
    I2CSerial.println(datetime + "\t" + c.day.season + "\t" + c.day.day + "\t" + c.day.colour + "\t" + c.day.rank);
    if (c.day.is_sanctorale) {
      I2CSerial.println("\t" + c.day.sanctorale + "\t" + c.day.sanctorale_colour + "\t" + c.day.sanctorale_rank);
    }
    
  
    
    /************************************************/ 
    // *5* Get lectionary (readings) for this date
    I2CSerial.println("*5*\n");
    String refs = "";
    wdt_reset();
    Lectionary l(c._I18n);
  
    bool b_OT = false;
    bool b_NT = false; 
    bool b_PS = false; 
    bool b_G = false;
  
    l.test(c.day.lectionary, c.day.liturgical_year, c.day.liturgical_cycle, &b_OT, &b_NT, &b_PS, &b_G);    
    
    Lectionary::ReadingsFromEnum r;
    
    if (getLectionaryReading(date, &r, battery.power_connected(), b_OT, b_NT, b_PS, b_G)) {      
      l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);    
  
      if (refs == "") { // 02-01-18 in case there is no reading, default to Gospel, since there will always be a Gospel reading
        r=Lectionary::READINGS_G; // there may be a bug: during weekdays, when the Lectionary number comes from a Saints' day and not the Calendar (Temporale), during Advent, Christmas
                                  // and Easter there may be a reading from NT (Christmas and Easter, when normally they're absent), or the NT (normally absent during Advent).
                                  // Need to check this works properly, but I don't have the Lectionary numbers for all the Saints' days, so currently the Temporale Lectionary numbers are
                                  // returned on Saints days (apart from those following Christmas day, which I did have).
        l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);    
      }
            
      /************************************************/ 
      // *6* Update epaper display with reading, use disk font (from SD card) if selected in config
      display_calendar(datetime, &c, refs);
    }
    else {
      I2CSerial.println("On battery, reading will not be updated for this hour (updates every 4 hours, at 8am, 12pm, 4pm, 8pm and midnight");    
    }
  
    I2CSerial.println("*6*\n");
  } // if(bEEPROM_checksum_good)
  
  
  /************************************************/ 
  // *7* Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  I2CSerial.println("*7*\n");
  //timeserver.gps_sleep();

  if (!bEEPROM_checksum_good) {
    if (battery.power_connected()) {
      display_image(clock_not_set_image);
    } else {
      display_image(connect_power_image);
    }
  }

  bool bSettingsUpdated = false;

  if (battery.power_connected()) {
    I2CSerial.println("Power is connected, starting config web server");
    I2CSerial.println("Battery voltage is " + String(battery.battery_voltage()));

    // Network should already be connected if we got in here, since when on usb power network connects at start, or prompts to configure if not already done
    //if (!network.connect()) {
    //  I2CSerial.println("Network is not configured, starting WPS setup");
    //  connect_wps();
    //  ESP.reset();
    //}
    
    uint32_t free = system_get_free_heap_size();
    I2CSerial.println("free memory = " + String(free));

    unsigned long server_start_time = millis();
    bool bTimeUp = false;
   
    if (c._config->StartServer(c._I18n)) {
      I2CSerial.println("Config web server started, listening for requests...");
      while(battery.power_connected() && !bSettingsUpdated && !bTimeUp) {
        wdt_reset();
        c._config->ServeClient(&bSettingsUpdated);
        delay(1000);
        //I2CSerial.println("Battery voltage is " + String(battery.battery_voltage()));
        if (millis() > (server_start_time + 1000*8*60)) bTimeUp = true; // run the server for an 10 minutes max, then sleep. If still on usb power, the web server will run again.
      }

      c._config->StopServer();

      if (bSettingsUpdated) {
        I2CSerial.println("Settings updated, resetting lectionary...");
        ESP.deepSleep(1e6); //reboot after 1 second
        //ESP.reset();
      }
      else if (bTimeUp) {
        I2CSerial.println("Server timed out, stopping web server and going to sleep");
        //ESP.deepSleep(SLEEP_HOUR - (1000*8*60));
        SleepUntilStartOfHour();
      }
      else {
        I2CSerial.println("Power disconnected, stopping web server and going to sleep");
      }
  
      //I2CSerial.println("Battery voltage is " + String(battery.battery_voltage()));
      
      free = system_get_free_heap_size();
      I2CSerial.println("free memory = " + String(free));
    }
  }


  
  /************************************************/ 
  // *8* completed all tasks, go to sleep
  I2CSerial.println("*8*\n");
  
  while(1) {
    I2CSerial.println("Going to sleep");
    //ESP.deepSleep(SLEEP_HOUR); //1 hour
    SleepUntilStartOfHour();
  }
}

void SleepUntilStartOfHour() {
    Config conf;
    time64_t date;
    conf.getDS3231DateTime(&date);
    tmElements_t ts;
    breakTime(date, ts);

    uint32_t sleepduration_minutes = (60 - ts.Minute + 10); // should wake up at around 10 minutes past the hour (the sleep timer is not terribly accurate!)
    
    I2CSerial.printf("Sleeping %d minutes: Will wake at around 10 minutes past the hour\n", sleepduration_minutes);
    ESP.deepSleep(sleepduration_minutes * 60e6);
    return; // should never return because ESP should be asleep!
}


bool getLectionaryReading(time64_t date, Lectionary::ReadingsFromEnum* r, bool bReturnReadingForAllHours, bool b_OT, bool b_NT, bool b_PS, bool b_G) {
  I2CSerial.printf("getLectionaryReading() bReturnReadingFromAllHours=%s\n", bReturnReadingForAllHours?"true":"false");
  //Lectionary::ReadingsFromEnum r;
  tmElements_t tm;
  breakTime(date, tm);

  bool bHaveLectionaryValue = false;

  if(tm.Day == 24 && tm.Month == 12 && tm.Hour >= 18) { // Christmas Eve Vigil Mass
    I2CSerial.printf("Christmas Eve vigil Mass\n");
    bHaveLectionaryValue = true;
    
    switch(tm.Hour) { // covers hours 18:00 - 23:59
    case 18:
      *r=Lectionary::READINGS_OT;    
      break;
    
    case 19:
      *r=Lectionary::READINGS_NT;    
      break;
      
    case 20:
      *r=Lectionary::READINGS_PS;    
      break;

    case 21:
    case 22:
    case 23:
      *r=Lectionary::READINGS_G;
      break;
          
    default:
      bHaveLectionaryValue = false;
      break;
    }
  } 
  
  if(tm.Day == 25 && tm.Month == 12) { // Christmas Day: Midnight Mass, Mass at Dawn
    I2CSerial.printf("Christmas Day\n");
    bHaveLectionaryValue = true;
    switch(tm.Hour) { // covers hours 00:00 - 07:59. Later hours (Mass during the day) are handled by the last switch statement (used for all other days also).
    case 0: // mass at midnight
    case 4: // mass at dawn
      *r=Lectionary::READINGS_OT;
      break;
      
    case 1: // mass at midnight
    case 5: // mass at dawn
      *r=Lectionary::READINGS_NT;
      break;
      
    case 2: // mass at midnight
    case 6: // mass at dawn
      *r=Lectionary::READINGS_PS;
      break;
      
    case 3: // mass at midnight
    case 7: // mass at dawn
      *r=Lectionary::READINGS_G;
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
          *r=Lectionary::READINGS_G;

          if (!b_OT) {
            *r=Lectionary::READINGS_NT;
          }
          
          if (!b_NT) {
            *r=Lectionary::READINGS_OT; // one or other of b_OT, b_NT should be true. Defaults to G if both are false.
          }
          break;
          
        case 14:
          *r=Lectionary::READINGS_PS;
          break;
        
        case 0:
        case 20:
          *r=Lectionary::READINGS_G;
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
        case 14:
          *r=Lectionary::READINGS_G;

          if (!b_OT) {
            *r=Lectionary::READINGS_NT;
          }
          
          if (!b_NT) {
            *r=Lectionary::READINGS_OT; // one or other of b_OT, b_NT should be true. Defaults to G if both are false.
          }
          break;
          
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
          *r=Lectionary::READINGS_PS;
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
          *r=Lectionary::READINGS_G;
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
          *r=Lectionary::READINGS_OT;
          break;
          
        case 12:
          *r=Lectionary::READINGS_NT;
          break;
    
        case 16:
          *r=Lectionary::READINGS_PS;
          break;
    
        case 0:
        case 20:
          *r=Lectionary::READINGS_G;
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
          *r=Lectionary::READINGS_OT;
          break;
          
        case 12:
        case 13:
        case 14:
        case 15:
          *r=Lectionary::READINGS_NT;
          break;
    
        case 16:
        case 17:
        case 18:
        case 19:
          *r=Lectionary::READINGS_PS;
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
          *r=Lectionary::READINGS_G;
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
bool epd_init(void) {
  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return false;
  }
  return true;
}

void init_panel() {
  epd_init();
  epd.ClearFrame();  
}

void display_calendar(String date, Calendar* c, String refs) {  
  DiskFont diskfont;
  
  if ((c->_I18n->_font_filename) != "builtin" && (c->_I18n->_font_filename) != "") {
    I2CSerial.printf("Using disk font\n");
    diskfont.begin(c->_I18n->_font_filename);
  }
  else {
    I2CSerial.printf("Using internal font\n");    
  }
  
  FONT_INFO* font = &calibri_10pt;
  
  //FONT_INFO* font_bold = &calibri_8ptBold;
  init_panel();
  Paint paint(image, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint.SetRotate(ROTATE_90);
  paint.Clear(UNCOLORED);

  Paint paint_red(image_red, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint_red.SetRotate(ROTATE_90);
  paint_red.Clear(UNCOLORED);
  
  bool bRed = (c->temporale->getColour() == Enums::COLOURS_RED) ? true : false;
  
  I2CSerial.println("Displaying calendar");
  if (c->day.is_sanctorale) {
    display_day(c->day.sanctorale, &paint, &paint_red, font, &diskfont, bRed); // sanctorale in struct day is the feast day, displayed at the top of the screen on feast days
    if (c->day.sanctorale == c->day.day) {                                     // otherwise the liturgical day is shown at the top of the screen.
      //display_date(date, "", &paint, font, &diskfont);                       // If it is a feast day, the liturgical day is displayed at the bottom left. Otherwise the bottom left
    } else {                                                                   // is left blank.
      display_date(date, c->day.day, &paint, font, &diskfont); // "day" in struct day is the liturgical day
    }
  } else {
    display_day(c->day.day, &paint, &paint_red, font, &diskfont, bRed);    
    display_date(date, "", &paint, font, &diskfont);
  }

  I2CSerial.println("Displaying verses");
//  //refs = "Ps 85:9ab+10, 11-12, 13-14"; //debugging
//  //refs="1 Chr 29:9-10bc";              //debugging
//  refs="John 3:16"; // debugging
//  refs="2 John 1:1"; // debugging
  display_verses(c, refs, &paint, &paint_red, font, &diskfont);

  //diskfont.end();

  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.TransmitPartialRed(paint_red.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.DisplayFrame();

  epd.Sleep();
  uint32_t free = system_get_free_heap_size();
  I2CSerial.println("free memory = " + String(free));
  I2CSerial.println("done");  
}

void display_day(String d, Paint* paint, Paint* paint_red, FONT_INFO* font, DiskFont* diskfont, bool bRed) {
  I2CSerial.println("display_day() d=" + d);
  
  if (diskfont->available) {
    int text_xpos = (paint->GetHeight() / 2) - ((diskfont->GetTextWidth(d))/2);
  
    if (bRed) {
      diskfont->DrawStringAt(text_xpos, 0, d, paint_red, COLORED, false);
    } else {
      diskfont->DrawStringAt(text_xpos, 0, d, paint, COLORED, false);
    }

    int charheight = diskfont->_FontHeader.charheight;
    
    paint->DrawLine(0, charheight, 264, charheight, COLORED);
  }
  else {
    //Paint paint(image, title_bar_y_height, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
    //paint.SetRotate(ROTATE_90);
  
    int text_xpos = (paint->GetHeight() / 2) - ((paint->GetTextWidth(d, font))/2);
  
    if (bRed) {
      paint_red->DrawStringAt(text_xpos, 0, d, font, COLORED, false);
    } else {
      paint->DrawStringAt(text_xpos, 0, d, font, COLORED, false);
    }
    paint->DrawLine(0, 15, 264, 15, COLORED);
    //epd.TransmitPartialBlack(paint.GetImage(), PANEL_SIZE_Y - title_bar_y_height, 0, paint.GetWidth(), paint.GetHeight());
  }
}

void display_date(String date, String day, Paint* paint, FONT_INFO* font, DiskFont* diskfont) {
  I2CSerial.println("\ndisplay_date: s=" + date);

  if (diskfont->available) {
    int text_xpos = PANEL_SIZE_X - (diskfont->GetTextWidth(date)); // right justified
   
    diskfont->DrawStringAt(text_xpos, PANEL_SIZE_Y - diskfont->_FontHeader.charheight, date, paint, COLORED, false);
    diskfont->DrawStringAt(0, PANEL_SIZE_Y - diskfont->_FontHeader.charheight, day, paint, COLORED, false);
  }
  else {
    //Paint paint(image, font->heightPages, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
    //paint.SetRotate(ROTATE_90);
  
    int text_xpos = PANEL_SIZE_X - (paint->GetTextWidth(date, font)); // right justified
   
    //paint.Clear(UNCOLORED);
    paint->DrawStringAt(text_xpos, PANEL_SIZE_Y - font->heightPages, date, font, COLORED, false);
    paint->DrawStringAt(0, PANEL_SIZE_Y - font->heightPages, day, font, COLORED, false);
    
    //paint.DrawLine(0, 15, 264, 15, COLORED);
    //epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  }
}

#define FORMAT_EMPHASIS_ON String("on")
#define FORMAT_EMPHASIS_OFF String("off")
#define FORMAT_DEFAULT String("") // keep whatever formatting is currently selected
#define FORMAT_LINEBREAK String("br")

bool display_verses(Calendar* c, String refs, Paint* paint_black, Paint* paint_red, FONT_INFO* font, DiskFont* diskfont) {
  bool right_to_left = c->_I18n->_right_to_left;

  I2CSerial.printf("refs from lectionary: [%s]\n", refs.c_str());
  
  Bible b(c->_I18n);
  if (!b.get(refs)) return false;
    
  b.dump_refs();

  Ref* r;
  int i = 0;

  r = b.refsList.get(i++);

  bool bEndOfScreen = false;

  int xpos = 0;
  int ypos = font->heightPages;

  if (diskfont->available) {
    ypos = diskfont->_FontHeader.charheight;
  }

  String line_above = "";
  bool bDisplayRefs = true;

  while (r != NULL && !bEndOfScreen) {     
    int start_chapter = r->start_chapter;
    int end_chapter = r->end_chapter;
    int verse = r->start_verse;
    int start_verse;
    int end_verse;
    String sentence_range;
    String verse_record;
    String verse_text;
    String output;

    for (int c = start_chapter; c <= end_chapter; c++) {
      I2CSerial.printf("\n%d:", c);
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

      String format_state = FORMAT_DEFAULT;
    
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
          I2CSerial.printf(" %d ", v);
          I2CSerial.printf("sentence_range=%s, numRecords=%d\n", sentence_range == "" ? "[All]" : sentence_range.c_str(), numRecords);
          verse_text = get_verse(verse_record, &book_name, sentence_range, numRecords);

          if (bDisplayRefs) {
            String refs_i18n = refs;
            if (book_name != "") refs_i18n.replace(b.books_shortnames[r->book_index], book_name);
            
            refs_i18n = "The quick brown fox jumps over the lazy dog. Now is the time for all good men to come to the aid of the party. ";

            I2CSerial.printf("refs_i18n = %s\n", refs_i18n.c_str());
            format_state = FORMAT_EMPHASIS_ON;
            bEndOfScreen = epd_verse(refs_i18n, paint_black, paint_red, &xpos, &ypos, font, diskfont, &format_state, right_to_left); // returns false if at end of screen
            bDisplayRefs = false;
            format_state = FORMAT_EMPHASIS_OFF;
          }
          //
          //bEndOfScreen = epd_verse(String(v), paint_red, &xpos, &ypos, font); // returns false if at end of screen
          bEndOfScreen = epd_verse(verse_text, paint_black, paint_red, &xpos, &ypos, font, diskfont, &format_state, right_to_left); // returns false if at end of screen
          I2CSerial.println("epd_verse returned " + String(bEndOfScreen ? "true":"false"));
          I2CSerial.printf("\n");
          v++;
          if (v > end_verse) bDone = true; // end_verse will be set to -1 if all verses up to the end of the chapter are to be returned.
        }
        else {
          I2CSerial.printf("Error\n");
          bDone = true;
        }
      }
      if (bEndOfScreen) break; // out of chapter for loop
    }
    r = b.refsList.get(i++);
  }

  return true;
}


bool epd_verse(String verse, Paint* paint_black, Paint* paint_red, int* xpos, int* ypos, FONT_INFO* font, DiskFont* diskfont, String* format_state, bool right_to_left) {
  I2CSerial.println("epd_verse() verse=" + verse);

  String word_part = "";
  String strOverflow = " . . .";
  bool bEOL = false;
  int width = 0;

  String ch = "";
  
  int ellipsiswidth = 0;
  int line_height = font->heightPages;

  if (diskfont->available) {
    line_height = diskfont->_FontHeader.charheight;
    ellipsiswidth = diskfont->GetTextWidth(strOverflow);
  }
  else {
    ellipsiswidth = paint_black->GetTextWidth(strOverflow, font);    
  }
  
  if (*ypos >= (PANEL_SIZE_Y - line_height)) return true;

  // Defaults to FORMAT_DEFAULT
  bool b_emphasis_on = false; // <i> or <b> italic/bold enclosing tags in the text will cause text to be output in red
  bool b_linebreak = false; // will be set by hyphenate_word if it encounters a <br> tag

  if (*format_state == FORMAT_EMPHASIS_ON) {
    b_emphasis_on = true;
  }
  else if (*format_state == FORMAT_EMPHASIS_OFF) {
    b_emphasis_on = false;
  }
  else if (*format_state == FORMAT_LINEBREAK) {
    b_linebreak = true;
  }

  int i = 0;
  bool bRtl = false;
  bool bEndOfRtlPart = false;
  
  DiskFont_FontCharInfo fci;
  String rtlreversedpart = "";
  
  while(verse.length() > 0) {    
    I2CSerial.printf(".");
    if (rtlreversedpart.length() == 0) {
      I2CSerial.printf("+");
            
      int charwidthpx = 0;
      
      ch = utf8CharAt(verse, 0);
      
      if (diskfont->available) {
        diskfont->getCharInfo(ch, &fci);
        bRtl = (((!right_to_left) && (fci.rtlflag > 0)) || ((right_to_left) && (fci.rtlflag == 0)));
        charwidthpx = fci.widthbits;
      }
      else {
        bRtl = false; //placeholder - will use method from epdpaint Paint object.
      }
      
      String rtlword = "";
      int rtlwordwidthpx = 0;
      int rtlstringwidthpx = 0;
      int lastrtlstringwidthpx = 0;
      bool found_word = false;

      if (bRtl)
      {
        I2CSerial.printf("verse=[%s]\n", verse.c_str());
        int stringpos = 0;
        //int last_stringpos = 0;
        String rtlpart = "";
        String lastrtlpart = "";
                
        do {
          // scan whole rtlword
          if (!found_word) {
            rtlwordwidthpx = 0;
          }
          else {
            I2CSerial.printf("overflow word = [%s]\n", rtlword.c_str());
          }
          
          //rtlword = "";
          //last_stringpos = stringpos; // save last scanned word's stringpos, since when the word overflows the line we need to backtrack to the last word which didn't overflow the line
          
          while (bRtl && !found_word && stringpos < verse.length() && ch.length() != 0) {   // scan rtl word into variable rtlword. On entry, ch and fci have already been set to the first rtl character data
            I2CSerial.printf("%d %s", stringpos, ch.c_str());
            rtlword = ch + rtlword; // store back to front for easier rendering
            rtlwordwidthpx += charwidthpx;
            stringpos += ch.length();            

            if (ch.length() == 0) {
              I2CSerial.printf("ch.length=0, verse = %s\n", verse.c_str());
            }
  
            ch = utf8CharAt(verse, stringpos);

            if (diskfont->available) { //
              I2CSerial.printf("<"); 
              diskfont->getCharInfo(ch, &fci);
              charwidthpx = fci.widthbits;

              if (ch != " ") {
                bRtl = (((!right_to_left) && (fci.rtlflag > 0)) || ((right_to_left) && (fci.rtlflag == 0)));
              } // otherwise, inherit the rtl state of the space from the last character
              else {
                rtlwordwidthpx += charwidthpx; // add trailing character (should be a space), since the last character read is appended at the beginning of the while loop above
                rtlword = ch + rtlword;        // so it won't get done on the last character, when it drops out.
                //stringpos+= ch.length();
                found_word = true;
              }
              
              I2CSerial.printf(">"); 
            }
            else {
              //if using internal font
            }
          }

          I2CSerial.printf("verselen=%d, stringpos=%d\n", verse.length(), stringpos);
                    
          I2CSerial.printf("rtlword=[%s]\n", rtlword.c_str());

          lastrtlpart = rtlpart;
          lastrtlstringwidthpx = rtlstringwidthpx;
          
          rtlpart = rtlword + rtlpart;
          rtlstringwidthpx += rtlwordwidthpx;

          if ((lastrtlstringwidthpx + rtlwordwidthpx + *xpos) <= PANEL_SIZE_X) {
            rtlword = ""; // if the word hasn't overflowed the line, then clear it. If it has (will drop out of while loop), save the word found for the next iteration
            found_word = false;              // reset found word flag
          }
 
          I2CSerial.printf("lastrtlpart=[%s]\n", lastrtlpart.c_str());
 
          I2CSerial.printf("%"); 

        } while ((bRtl) && ((lastrtlstringwidthpx + rtlwordwidthpx + *xpos) <= PANEL_SIZE_X) && (stringpos < verse.length()));
        
        //stringpos = last_stringpos;      // go back to last word that didn't overflow the line

        rtlreversedpart = lastrtlpart;
        verse = verse.substring(rtlreversedpart.length() - 1);

        I2CSerial.printf("lastrtlpart.length()=%d\n", lastrtlpart.length());
        I2CSerial.printf("lastrtlpart=[%s]\n", lastrtlpart.c_str());

        I2CSerial.printf("rtlreversedpart.length()=%d\n", rtlreversedpart.length());
        I2CSerial.printf("rtlreversedpart=[%s]\n", rtlreversedpart.c_str());
        
      }
    }
    
    if (rtlreversedpart.length() > 0) {
      I2CSerial.printf("(");
      bEndOfRtlPart = hyphenate_word(&rtlreversedpart, &word_part, &width, font, diskfont, paint_black, format_state); // emphasis_state will be set to "on" by hyphenate_word when an opening <i> or <b> tag is found, and 
                                                                                         // "off" when a closing </i> or </b> tag is found, or "" otherwise.
                                                                                         // paint_black is used by hyphenate_word for getting the width of the text to be printed, so either
                                                                                         // paint_black or paint_red could be used for this, since it is a nonprinting call.    
      
      I2CSerial.printf(")");
    } else {
      I2CSerial.printf("[");
      bEOL = hyphenate_word(&verse, &word_part, &width, font, diskfont, paint_black, format_state); // emphasis_state will be set to "on" by hyphenate_word when an opening <i> or <b> tag is found, and 
                                                                                         // "off" when a closing </i> or </b> tag is found, or "" otherwise.
                                                                                         // paint_black is used by hyphenate_word for getting the width of the text to be printed, so either
                                                                                         // paint_black or paint_red could be used for this, since it is a nonprinting call.    
      I2CSerial.printf("]");
    }
    
    if (*format_state == FORMAT_EMPHASIS_ON) {
      b_emphasis_on = true;
    }
    else if (*format_state == FORMAT_EMPHASIS_OFF) {
      b_emphasis_on = false;
    }
    else if (*format_state == FORMAT_LINEBREAK) {
      b_linebreak = true;
    }

    // if display_verses() called this function with format_state set to something other than FORMAT_DEFAULT (e.g. for printing the Bible reference at the top of the reading), then this 
    // selection will be left unchanged and will be acted upon, unless it is cancelled by tags in the verse text parsed by the call to hyphenate_word() (mostly it will not be). This should 
    // allow emphasised text to continue over more than one call to this function from display_verses().

    if (((*ypos + (line_height * 2)) >= (PANEL_SIZE_Y - line_height)) && (*xpos + width >= PANEL_SIZE_X - ellipsiswidth) && (verse.length() > 0)) {
      //if true, the present line of text is the last to be displayed on the screen (at the bottom right), and there is more text to display
      //some text left that won't fit on the screen - display ellipsis
      //line_height * 2 because want to consider if the *bottom* of the *next* line will overflow into the calendar area (the bottom line)

      word_part = strOverflow;
    }
    else {
      if (*xpos + width > PANEL_SIZE_X || b_linebreak == true) {            
        *xpos = 0;      
        (*ypos)+= line_height;      
        
        if (*ypos + line_height >= (PANEL_SIZE_Y - line_height)) return true; // + line_height because want to know if the *bottom* of the character overflows the calendar line (bottom line)
        
        b_linebreak = false; // reset linebreak flag
      }
    }
        
    wdt_reset();
    if (width != 0) { // width will be 0 when a tag <i>, </i>, <b>, </b> or <br> has been encountered (which are non-printing) so no need to call paint in these cases          
      if (!b_emphasis_on) {
        if (diskfont->available) {
          diskfont->DrawStringAt(*xpos, *ypos, word_part, paint_black, COLORED, right_to_left);
        }
        else {
          paint_black->DrawStringAt(*xpos, *ypos, word_part, font, COLORED, right_to_left);
        }
      } 
      else {
        if (diskfont->available) {
          diskfont->DrawStringAt(*xpos, *ypos, word_part, paint_red, COLORED, right_to_left);
        }
        else {
          paint_red->DrawStringAt(*xpos, *ypos, word_part, font, COLORED, right_to_left);
        }
      }
      (*xpos) += width;
      wdt_reset();
    }
  }
  
  return false;
}

String utf8CharAt(String s, int pos) { 
  //I2CSerial.println("String=" + s);
  
  if (pos >= s.length()) {
    //I2CSerial.println("utf8CharAt string length is " + String(ps->length()) + " *ppos = " + String(*ppos));
    return String("");
  }
  
  int charLen = charLenBytesUTF8(s.charAt(pos));

  //I2CSerial.println("char at pos " + String(*ppos) + " = " + String(ps->charAt(*ppos)) + "utf8 charLen = " + String(charLen));

  if (charLen == 0) {
    return String("");
  } 
  else {
    //Serial.print("substring is" + s.substring(pos, pos+charLen));
    return s.substring(pos, pos + charLen);
  }
}

int charLenBytesUTF8(char s) {
  byte ch = (byte) s;
  //I2CSerial.println(String(ch)+ ";");

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

int lines(String s) {
  
}

String get_verse(String verse_record, String* book_name, String sentence_range, int numRecords) { // a bit naughty, verse_record strings can contain multiple lines of csv records for verses that span more than one line.
  I2CSerial.printf("get_verse() %s", verse_record.c_str());
  I2CSerial.printf("get_verse() sentence_range=%s, numRecords=%d\n", sentence_range == "" ? "[All]" : sentence_range.c_str(), numRecords);
  Csv csv;

  int pos = 0;

  String fragment = "";
  int sentence_number = 1;
  int sentence_count = 0;
  String verse = "";
  bool more_than_one = false;

  String verse_fragment_array[26];
  int verse_fragment_array_index = 0;

  for (int i = 0; i<26; i++) {
    verse_fragment_array[i] = ""; // intialize array
  }

  I2CSerial.printf("get_verse() *1*");      
  do {
    int i = 0;
    do {
      I2CSerial.println("pos = " + String(pos));
      fragment = csv.getCsvField(verse_record, &pos);
      I2CSerial.printf("fragment=%s\n", fragment.c_str());
      
      if (i == 0 && !more_than_one) {
        *book_name = fragment;
      }

      if (i == 4) {
        I2CSerial.printf("fragment=%s\n", fragment.c_str());
        verse += ((more_than_one?" ":"") + fragment);
        verse_fragment_array[verse_fragment_array_index++] = ((more_than_one?" ":"") + fragment);
        if (verse_fragment_array_index == 26) return ("verse fragment array index out of range");
      }
    } while (pos < verse_record.length() && i++ != 4);
    //I2CSerial.println("pos = " + String(pos) + " charAt pos = [" + String(verse_record.charAt(pos)) + "]");
    more_than_one = true;
    sentence_count++;
  } while (pos < verse_record.length());

  I2CSerial.printf("get_verse() *2*");      

  String letters = "abcdefghijklmnopqrstuvwxyz";

  int max_fragment_number = 0;
  for (int i = 0; i < sentence_range.length(); i++) {
    int fragment_number = letters.indexOf(sentence_range.charAt(i));

    if (fragment_number > max_fragment_number) max_fragment_number = fragment_number;
  }
  I2CSerial.printf("get_verse() *3*");      

  if (sentence_range == "" || max_fragment_number > (numRecords - 1)) {
    I2CSerial.println("Sentence range is empty or max fragment number > numRecords: returning whole verse");
    return verse;  
  }
  else {
    I2CSerial.println("parsing subrange reference");
    int charpos = 0;
    int fragment_number = 0;
    String output = "";
    
    char c = sentence_range.charAt(charpos);
    if (c == '-') {
      charpos++;

      if (charpos >= sentence_range.length()) return verse; // return whole verse if only the - is present (malformed subrange)
      
      fragment_number = letters.indexOf(sentence_range.charAt(charpos));

      I2CSerial.printf("adding all fragments up to %d\n", fragment_number);

      if (fragment_number == -1) return verse; // next character is not a letter - return whole verse (malformed subrange)

      for (int i = 0; i <= fragment_number; i++) { // if the subrange starts with a '-', add all verse fragments up to the first lettered verse fragment
        output += verse_fragment_array[i];
      }
      charpos++;
    }

    I2CSerial.printf("get_verse() *4*");      
  
    c = sentence_range.charAt(charpos);
    while (c != '-' && charpos < sentence_range.length()) {
      fragment_number = letters.indexOf(sentence_range.charAt(charpos));
      I2CSerial.printf("adding fragment %d(%c)\n", fragment_number, c);
      output += verse_fragment_array[fragment_number];      
      charpos++;
      c = sentence_range.charAt(charpos);
    }

    I2CSerial.printf("get_verse() *5*");      

    if (c == '-') {
      I2CSerial.printf("adding fragments at end from %d to %d\n", fragment_number + 1, verse_fragment_array_index - 1);
      for (int i = fragment_number + 1; i < verse_fragment_array_index; i++) {
        output += verse_fragment_array[i];
      }
    }

    I2CSerial.printf("get_verse() *6*");      

    return output;
  }

  I2CSerial.printf("get_verse() *7*");      

  return verse;
}

bool hyphenate_word(String *w, String* word_part, int* x_width, FONT_INFO* font, DiskFont* diskfont, Paint* paint, String* format_state) {
  int j = 0;
  bool bHyphenDone = false;
  String w_ch;
  String w_str;
  String w_str_last;
  int hyphen_width;
  
  if (diskfont->available) {
    hyphen_width = diskfont->GetTextWidth("-");
  }
  else {
    hyphen_width = paint->GetTextWidth("-", font);
  }
  
  int len = w->length();
  String hyphen = "-";
  bool bEOL = false;
  bool bExpectingTag = false;
  bool bIsWordNotTag = false;

  *format_state = FORMAT_DEFAULT;
  
  while(!bHyphenDone) {
    w_str_last = w_str;
    w_ch = utf8CharAt(*w, j);
    w_str += w_ch;

    if ((j==0) && (w_ch != "<")) {
      bIsWordNotTag = true; //some tags may be added after a word, but with no space inbetween. Must return a word to print when either a space, an eol or a start tag "<" is found.
    }                       //then the tag will be processed on the next call to the function.

    if (!bIsWordNotTag) {
      if (w_ch == "<" && bExpectingTag == false) {
        bExpectingTag = true;
      }
  
      if (w_ch == ">") {         
        if (w_str == "<br>") {
          *format_state = FORMAT_LINEBREAK;
          *x_width = 0;
        }
    
        if (w_str == "<i>" || w_str == "<b>") {
          *format_state = FORMAT_EMPHASIS_ON;
          *x_width = 0;
        }
    
        if (w_str == "</i>" || w_str == "</b>") {
          *format_state = FORMAT_EMPHASIS_OFF;
        }
      }
  
      if (bExpectingTag && w_ch == ">") {
        *w = w->substring(w_str.length());
        *x_width = 0;
        return false;
      }
    }

    bool bTextWidthGtThanPanelWidth = false;
    if (diskfont->available) {
      bTextWidthGtThanPanelWidth = (diskfont->GetTextWidth(w_str) > (PANEL_SIZE_X - hyphen_width));
    }
    else {
      bTextWidthGtThanPanelWidth = (paint->GetTextWidth(w_str, font) > (PANEL_SIZE_X - hyphen_width));
    }
        
    if (bTextWidthGtThanPanelWidth) {
      bEOL = true;
      bHyphenDone = true;
    } else {
      bEOL = false;
      
      if (w_ch != "<" && bIsWordNotTag) {
        w_str_last = w_str;
      }
      
      j += w_ch.length();

      if (w_ch == " " || (w_ch == "<" && bIsWordNotTag) || j >= len) {
        hyphen = "";
        bHyphenDone = true;
      }
    }
  }
  //Serial.print("[" + String(w->length()) + " " + String(w_str_last.length()) + "]");
  *w = w->substring(w_str_last.length());
  *word_part = (w_str_last + hyphen);
  
  if (diskfont->available) {
    *x_width = diskfont->GetTextWidth(*word_part);
    I2CSerial.printf("*x_width: %d\n", *x_width);
  }
  else {
    *x_width = paint->GetTextWidth(*word_part, font);  
  }
  
  return bEOL;
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
    Serial.print("ERROR: ");
    switch (err)
    {
        case EDB_OUT_OF_RANGE:
            Serial.println("Recno out of range");
            break;
        case EDB_TABLE_FULL:
            Serial.println("Table full");
            break;
        case EDB_OK:
        default:
            Serial.println("OK");
            break;
    }
}
