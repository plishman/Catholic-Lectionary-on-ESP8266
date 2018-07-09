#define DISPLAY_SPI_3WIRE

//#include <LiquidCrystal.h>

// Catholic Lectionary on ESP
// Copyright (c) 2017 Philip Lishman, All rights reserved.

extern const int COLORED    = 1;
extern const int UNCOLORED  = 0;

extern const int PANEL_SIZE_X = 264;
extern const int PANEL_SIZE_Y = 176;

//ESP8266---
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
//#define FS_NO_GLOBALS
//#include "FS.h"

//----------
#include <pins_arduino.h>
#include <I2CSerialPort.h>
#include <utf8string.h>
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
//#include <calibri10pt.h>
#include <edb.h>
#include <pgmspace.h>
#include <Network.h>
#include <Battery.h>
#include <Config.h>
#include <images.h>
#include <DiskFont.h>
#include <Bidi.h>

extern "C" {
#include "user_interface.h"
}

//#define COLORED     1
//#define UNCOLORED   0

//#define PANEL_SIZE_X 264
//#define PANEL_SIZE_Y 176

#define SLEEP_HOUR 60*60e6

I2CSerialPort I2CSerial;
 
#ifdef DISPLAY_SPI_3WIRE
Epd epd(SPI_3WIRE);
#else
Epd epd();
#endif

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

ESP8266WebServer server(80);

bool bEEPROM_checksum_good = false;

void setup() { 
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  SD.begin(D1, SPI_HALF_SPEED);
  
  SPIFFS.begin();
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

  // Check if EEPROM checksum is good

  //Config c;
  bEEPROM_checksum_good = Config::EEPROMChecksumValid();

  if (!bEEPROM_checksum_good && !battery.power_connected()) {
    I2CSerial.println("On battery power and EEPROM checksum invalid: Sleeping until USB power is attached and web interface is used to configure EEPROM");
    display_image(connect_power_image);
    ESP.deepSleep(0); //sleep until USB power is reconnected // sleep for an hour (71minutes is the maximum!), or until power is connected (SLEEP_HOUR)
  }

  rtcData_t rtcData = {0};

  if (battery.power_connected()) {
    if (!network.connect()){
      I2CSerial.println("Need to configure Wifi with WPS to enable web configuration");
      I2CSerial.println("On USB power and no network configured: Prompting user to connect using WPS button");
      if (!connect_wps()) {
        I2CSerial.println("Failed to configure Wifi network via WPS - sleeping until USB power is reconnected");
        display_image(connect_power_image);
        ESP.deepSleep(0); // sleep indefinitely, reset pulse will wake the ESP when USB power is unplugged and plugged in again. //sleep for an hour (71minutes is the maximum!), or until power is connected SLEEP_HOUR
      } else {
        //ESP.deepSleep(1e6); // reset esp, network is configured
        //ESP.reset();
        ESP.restart();
      }
    }
  }
  else {
    I2CSerial.println("On battery power - not attempting to connect to network. Connect power to use lectionary setup (http://lectionary.local/).");  

    rst_info *rinfo;
    rinfo = ESP.getResetInfoPtr();
    I2CSerial.println(String("ResetInfo.reason = ") + (*rinfo).reason);

    if ((*rinfo).reason == REASON_DEEP_SLEEP_AWAKE) { // only check the hour count to the next reading if we awoke because of the deepsleep timer
      if (Config::readRtcMemoryData(rtcData)) { // if CRC fails for values, this is probably a cold boot (power up), so will always update the reading in this case
        if (rtcData.data.wake_hour_counter > 8) { // sanity check - should never be more than 8 hours between readings.
          rtcData.data.wake_hour_counter = 1; // this will mean that, after being decremented to 0, a reading should be displayed immediately
        }
        
        rtcData.data.wake_hour_counter--;
        Config::writeRtcMemoryData(rtcData);
  
        if (rtcData.data.wake_hour_counter > 0) {
          I2CSerial.printf("No reading this hour, and on battery, so going back to sleep immediately.\nNumber of hours to next reading is %d\n", rtcData.data.wake_hour_counter);
          SleepUntilStartOfHour(); // no reading this hour, go back to sleep immediately
        }
      }
    }
  }

//  // Check if EEPROM checksum is good
//
//  Config c;
//  bEEPROM_checksum_good = c.EEPROMChecksumValid();
//
//  if (!bEEPROM_checksum_good && !battery.power_connected()) {
//    I2CSerial.printf("On battery power and EEPROM checksum invalid: Sleeping until USB power is attached and web interface is used to configure EEPROM");
//    display_image(connect_power_image);
//    ESP.deepSleep(0); //sleep until USB power is reconnected // sleep for an hour (71minutes is the maximum!), or until power is connected (SLEEP_HOUR)
//  }
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
  display_image(i, "", false);
}

void display_image(EPD_DISPLAY_IMAGE i, String messagetext, bool bMessageRed) {
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

  if (messagetext != "") {
    //I2CSerial.println("Message text is: " + messagetext);
    DiskFont diskfont;
    diskfont.begin();
    
    int strwidth = diskfont.GetTextWidth(messagetext);
    //I2CSerial.println("strwidth is " + String(strwidth) + "\n paint.GetHeight - strwidth = " + String(paint.GetHeight() - strwidth));
  
    if (bMessageRed) {
      paint_red.SetRotate(ROTATE_90);
      diskfont.DrawStringAt(paint.GetHeight() - strwidth, 0, messagetext, &paint_red, COLORED, false);
    } else {
      paint.SetRotate(ROTATE_90);
      diskfont.DrawStringAt(paint.GetHeight() - strwidth, 0, messagetext, &paint, COLORED, false);
    }
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
    if (!c._I18n->configparams.have_config) {
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
    Config::getLocalDS3231DateTime(&date);

    roundupdatetohour(date); // the esp8266 wake timer is not very accurate - about +-3minutes per hour, so if the date is within 5 mins of the hour, close to an hour, 
                     // will round to the hour.
    
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

    I2CSerial.printf("OT:%s NT:%s PS:%s G:%s\n", String(b_OT).c_str(), String(b_NT).c_str(), String(b_PS).c_str(), String(b_G).c_str());
    
    Lectionary::ReadingsFromEnum r;
    
    if (getLectionaryReading(date, &r, true/*battery.power_connected()*/, b_OT, b_NT, b_PS, b_G)) {      
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
      if (!display_calendar(datetime, &c, refs)) { // if there is no reading for the current part of the day, display the Gospel reading instead (rare)
        I2CSerial.printf("No reading found (Apocrypha missing from this Bible?). Displaying Gospel reading instead\n");
        r=Lectionary::READINGS_G;
        l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);
        display_calendar(datetime, &c, refs);
      }
    }
    else {
      I2CSerial.println("On battery, reading will not be updated for this hour (updates every 4 hours, at 8am, 12pm, 4pm, 8pm and midnight");    
    }

    I2CSerial.printf("Calculating next wake time:\n");

    rtcData_t rtcData = {0};
    //rtcData.data.dcs = dc_normal;        
    rtcData.data.wake_hour_counter = 1;

    uint8_t Hour = ts.Hour;
    int i = 1;
    
    if (Hour != 23) { // if crossing a day boundary, the season and/or number of readings may change, so will always wake on a day boundary, so make the next wake time default to 1 hour
      while (!getLectionaryReading(date + (SECS_PER_HOUR * i), &r, false, b_OT, b_NT, b_PS, b_G)) { // check if there will be a reading next hour
        rtcData.data.wake_hour_counter++;
        i++;
        Hour = (Hour + 1 > 23) ? 0 : Hour + 1;
        if (Hour == 0) break;
      }
    }

    I2CSerial.printf("Next reading is in %d hour(s)\n", rtcData.data.wake_hour_counter); 

    Config::writeRtcMemoryData(rtcData);        
  
    I2CSerial.println("*6*\n");
  } // if(bEEPROM_checksum_good)
  
  
  /************************************************/ 
  // *7* Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  I2CSerial.println("*7*\n");
  //timeserver.gps_sleep();

  if (!bEEPROM_checksum_good) {
    if (battery.power_connected()) {
      display_image(clock_not_set_image, Network::getDHCPAddressAsString(), true);
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
   
    if (Config::StartServer()) {
      I2CSerial.println("Config web server started, listening for requests...");
      while(battery.power_connected() && !Config::bSettingsUpdated && !bTimeUp) {
        server.handleClient();
        wdt_reset();
        delay(1000);
        //I2CSerial.println("Battery voltage is " + String(battery.battery_voltage()));
        if (millis() > (server_start_time + 1000*8*60)) bTimeUp = true; // run the server for an 10 minutes max, then sleep. If still on usb power, the web server will run again.
      }

      Config::StopServer();

      if (Config::bSettingsUpdated) {
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

void roundupdatetohour(time64_t& date) {
  tmElements_t ts;
  breakTime(date, ts);
  I2CSerial.printf("Input date = %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

  if (ts.Minute >= 53) {
    date += ((59 - ts.Minute) * 60) + (60 - ts.Second);
  }

  tmElements_t tso;
  breakTime(date, tso);
  I2CSerial.printf("Rounded date = %02d/%02d/%04d %02d:%02d:%02d\n", tso.Day, tso.Month, tmYearToCalendar(tso.Year), tso.Hour, tso.Minute, tso.Second);
}

void SleepUntilStartOfHour() {
    Config conf;
    time64_t date;
    conf.getLocalDS3231DateTime(&date);
    
    tmElements_t ts;
    breakTime(date, ts);

    int hourskip = 0;

    uint32_t sleepduration_minutes = (60 - ts.Minute); // should wake up at around 10 minutes past the hour (the sleep timer is not terribly accurate!)
    if (sleepduration_minutes <= 7) { // if only a few minutes before the top of the hour, round it up to the next hour and skip the hour plus the difference
      sleepduration_minutes += 60;  // this can occur because the wake timer is inaccurate (+- about 3 minutes per hour). 
      hourskip = 1; // for the debug output, so that the correct hour is output if the current hour is rounded up
    }
    
    I2CSerial.printf("Sleeping %d minutes: Will wake at around %02d:00\n", sleepduration_minutes, (ts.Hour + 1 + hourskip)%24);
    ESP.deepSleep(sleepduration_minutes * 60e6);
    return; // should never return because ESP should be asleep!
}


bool getLectionaryReading(time64_t date, Lectionary::ReadingsFromEnum* r, bool bReturnReadingForAllHours, bool b_OT, bool b_NT, bool b_PS, bool b_G) {
  I2CSerial.printf("getLectionaryReading() bReturnReadingFromAllHours=%s ", bReturnReadingForAllHours?"true":"false");
  //Lectionary::ReadingsFromEnum r;
  tmElements_t tm;
  breakTime(date, tm);

  I2CSerial.printf("hour=%02d\n", tm.Hour);

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
    I2CSerial.printf("e-Paper init failed\n");
    return false;
  }
  return true;
}

void init_panel() {
  I2CSerial.printf("Initializing panel...");
  if (epd_init()) {
    epd.ClearFrame();
    I2CSerial.printf("done\n");
  }
}

bool display_calendar(String date, Calendar* c, String refs) {  
  DiskFont diskfont;
  
  if (diskfont.begin(c->_I18n->configparams)) {
    I2CSerial.printf("Using disk font\n");
  }
  else {
    I2CSerial.printf("Using internal font\n");    
  }
  
  //FONT_INFO* font = &calibri_10pt;
  
  //FONT_INFO* font_bold = &calibri_8ptBold;
  Paint paint(image, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint.SetRotate(ROTATE_90);
  paint.Clear(UNCOLORED);

  Paint paint_red(image_red, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint_red.SetRotate(ROTATE_90);
  paint_red.Clear(UNCOLORED);
  
  bool bRed = (c->temporale->getColour() == Enums::COLOURS_RED) ? true : false;
  
  I2CSerial.println("Displaying calendar");
  I2CSerial.println("Displaying verses");
//  //refs = "Ps 85:9ab+10, 11-12, 13-14"; //debugging
//  //refs="1 Chr 29:9-10bc";              //debugging
//  refs="John 3:16"; // debugging
//  refs="2 John 1:1"; // debugging
  if (!display_verses(c, refs, &paint, &paint_red, &diskfont)) {
    I2CSerial.printf("display_verses returned false\n");
    return false;
  }

  if (c->day.is_sanctorale) {
    display_day(c->day.sanctorale, &paint, &paint_red, &diskfont, bRed); // sanctorale in struct day is the feast day, displayed at the top of the screen on feast days
    if (c->day.sanctorale == c->day.day) {                                     // otherwise the liturgical day is shown at the top of the screen.
      //display_date(date, "", &paint, font, &diskfont);                       // If it is a feast day, the liturgical day is displayed at the bottom left. Otherwise the bottom left
    } else {                                                                   // is left blank.
      display_date(date, c->day.day, &paint, &diskfont); // "day" in struct day is the liturgical day
    }
  } else {
    display_day(c->day.day, &paint, &paint_red, &diskfont, bRed);    
    display_date(date, "", &paint, &diskfont);
  }

//  I2CSerial.println("Displaying verses");
////  //refs = "Ps 85:9ab+10, 11-12, 13-14"; //debugging
////  //refs="1 Chr 29:9-10bc";              //debugging
////  refs="John 3:16"; // debugging
////  refs="2 John 1:1"; // debugging
//  display_verses(c, refs, &paint, &paint_red, font, &diskfont);
  
  //diskfont.end();

  init_panel();
  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.TransmitPartialRed(paint_red.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.DisplayFrame();

  epd.Sleep();
  uint32_t free = system_get_free_heap_size();
  I2CSerial.println("free memory = " + String(free));
  I2CSerial.println("done"); 

  return true;
}

void display_day(String d, Paint* paint, Paint* paint_red, DiskFont* diskfont, bool bRed) {
  I2CSerial.println("display_day() d=" + d);
  
//  if (diskfont->available) {
    int text_xpos = (paint->GetHeight() / 2) - (int)((diskfont->GetTextWidthA(d))/2);

    if (bRed) {
      diskfont->DrawStringAt(text_xpos, 0, d, paint_red, COLORED, false);
    } else {
      diskfont->DrawStringAt(text_xpos, 0, d, paint, COLORED, false);
    }

    int charheight = diskfont->_FontHeader.charheight;
    
    paint->DrawLine(0, charheight, 264, charheight, COLORED);
//  }
//  else {
//    //Paint paint(image, title_bar_y_height, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
//    //paint.SetRotate(ROTATE_90);
//  
//    int text_xpos = (paint->GetHeight() / 2) - ((paint->GetTextWidth(d, font))/2);
//  
//    if (bRed) {
//      paint_red->DrawStringAt(text_xpos, 0, d, font, COLORED, false);
//    } else {
//      paint->DrawStringAt(text_xpos, 0, d, font, COLORED, false);
//    }
//    paint->DrawLine(0, 15, 264, 15, COLORED);
//    //epd.TransmitPartialBlack(paint.GetImage(), PANEL_SIZE_Y - title_bar_y_height, 0, paint.GetWidth(), paint.GetHeight());
//  }
}

void display_date(String date, String day, Paint* paint, DiskFont* diskfont) {
  I2CSerial.println("\ndisplay_date: s=" + date);

//  if (diskfont->available) {
    int text_xpos = PANEL_SIZE_X - (int)(diskfont->GetTextWidthA(date)); // right justified
   
    diskfont->DrawStringAt(text_xpos, PANEL_SIZE_Y - diskfont->_FontHeader.charheight, date, paint, COLORED, false);
    diskfont->DrawStringAt(0, PANEL_SIZE_Y - diskfont->_FontHeader.charheight, day, paint, COLORED, false);
//  }
//  else {
//    //Paint paint(image, font->heightPages, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
//    //paint.SetRotate(ROTATE_90);
//  
//    int text_xpos = PANEL_SIZE_X - (paint->GetTextWidth(date, font)); // right justified
//   
//    //paint.Clear(UNCOLORED);
//    paint->DrawStringAt(text_xpos, PANEL_SIZE_Y - font->heightPages, date, font, COLORED, false);
//    paint->DrawStringAt(0, PANEL_SIZE_Y - font->heightPages, day, font, COLORED, false);
//    
//    //paint.DrawLine(0, 15, 264, 15, COLORED);
//    //epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
//  }
}

#define FORMAT_EMPHASIS_ON String("on")
#define FORMAT_EMPHASIS_OFF String("off")
#define FORMAT_DEFAULT String("") // keep whatever formatting is currently selected
#define FORMAT_LINEBREAK String("br")

bool display_verses(Calendar* c, String refs, Paint* paint_black, Paint* paint_red, DiskFont* diskfont) {
  bool right_to_left = c->_I18n->configparams.right_to_left;

  I2CSerial.printf("refs from lectionary: [%s]\n", refs.c_str());
  
  Bible b(c->_I18n);
  if (!b.get(refs)) {
    I2CSerial.printf("Couldn't get refs (no Apocrypha?)\n");
    return false;
  }
    
  b.dump_refs();

  Ref* r;
  int i = 0;

  r = b.refsList.get(i++);

  bool bEndOfScreen = false;

  int xpos = 0;
  int ypos = diskfont->_FontHeader.charheight;

  String line_above = "";
  bool bDisplayRefs = true;

  bool bGotVerses = false; // I found that Psalm 85:14 is missing from the NJB, but not the French version, because in the English version verse 14 is included in verse 13.
                           // If something was found, this flag will be set, so only if nothing was found will the function return false.

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
          
          I2CSerial.printf(" %d ", v);
          I2CSerial.printf("sentence_range=%s, numRecords=%d\n", sentence_range == "" ? "[All]" : sentence_range.c_str(), numRecords);
          verse_text = get_verse(verse_record, &book_name, sentence_range, numRecords);

          if (bDisplayRefs) {
            String refs_i18n = refs;
            if (book_name != "") refs_i18n.replace(b.books_shortnames[r->book_index], book_name);
            verse_text = "<i>" + refs_i18n + "</i> " + verse_text;
            
            //verse_text = "The quick brown fox لنور و jumps over the";// lazy dog. Now is the time for all good men to come to the aid of the party. ";

            I2CSerial.printf("refs_i18n = %s\n", refs_i18n.c_str());
            //format_state = FORMAT_EMPHASIS_ON;
            //bEndOfScreen = epd_verse(refs_i18n, paint_black, paint_red, &xpos, &ypos, font, diskfont, &format_state, right_to_left); // returns false if at end of screen
            bDisplayRefs = false;
            //format_state = FORMAT_EMPHASIS_OFF;
          }
          //
          //bEndOfScreen = epd_verse(String(v), paint_red, &xpos, &ypos, font); // returns false if at end of screen
          //bEndOfScreen = epd_verse(verse_text, paint_black, paint_red, &xpos, &ypos, font, diskfont, &format_state, right_to_left); // returns false if at end of screen
          bEndOfScreen = epd_verse(verse_text, paint_black, paint_red, &xpos, &ypos, diskfont, &bEmphasis_On, right_to_left); // returns false if at end of screen
          I2CSerial.println("epd_verse returned " + String(bEndOfScreen ? "true":"false"));
          I2CSerial.printf("\n");
          v++;
          if (v > end_verse) bDone = true; // end_verse will be set to -1 if all verses up to the end of the chapter are to be returned.
        }
        else {
          I2CSerial.printf("Verse is missing from this Bible (variation in Psalms?)\n");
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

bool epd_verse(String verse, Paint* paint_black, Paint* paint_red, int* xpos, int* ypos, DiskFont* diskfont, bool* bEmphasis_On, bool right_to_left) {
  I2CSerial.println("epd_verse() verse=" + verse);

  int fbwidth = PANEL_SIZE_X;
  int fbheight = PANEL_SIZE_Y;
  
  Bidi bidi;

//  if (diskfont->available) {
    fbheight = PANEL_SIZE_Y - diskfont->_FontHeader.charheight;
    return bidi.RenderText(verse, xpos, ypos, paint_black, paint_red, diskfont, bEmphasis_On, fbwidth, fbheight, right_to_left);    
//  }
//  else {
//    fbheight = PANEL_SIZE_Y - font->heightPages;
//    return bidi.RenderText(verse, xpos, ypos, paint_black, paint_red, font,     bEmphasis_On, fbwidth, fbheight, right_to_left);
//  }

  return true;
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
  
//  if (diskfont->available) {
    hyphen_width = (int)diskfont->GetTextWidthA("-");
//  }
//  else {
//    hyphen_width = paint->GetTextWidth("-", font);
//  }
  
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
//    if (diskfont->available) {
      bTextWidthGtThanPanelWidth = (int)((diskfont->GetTextWidthA(w_str)) > (PANEL_SIZE_X - hyphen_width));
//    }
//    else {
//     bTextWidthGtThanPanelWidth = (paint->GetTextWidth(w_str, font) > (PANEL_SIZE_X - hyphen_width));
//    }
        
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
  
//  if (diskfont->available) {
    *x_width = (int)(diskfont->GetTextWidthA(*word_part));
    I2CSerial.printf("*x_width: %d\n", *x_width);
//  }
//  else {
//    *x_width = paint->GetTextWidth(*word_part, font);  
//  }
  
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
