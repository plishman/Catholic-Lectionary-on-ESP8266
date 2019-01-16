#include <RCGlobals.h>

// Catholic Lectionary on ESP
// Copyright (c) 2017-2019 Philip Lishman, Licensed under GPL3, see LICENSE

//extern const int COLORED    = 1;
//extern const int UNCOLORED  = 0;

//extern const int PANEL_SIZE_X = 264;
//extern const int PANEL_SIZE_Y = 176;

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

I2CSerialPort I2CSerial;

GxIO_Class io(SPI, /*CS=D16*/ D8, /*DC=D4*/ -1, /*RST=D5*/ D2);  // dc=-1 -> not used (using 3-wire SPI)
GxEPD_Class ePaper(io, D2, D3 /*RST=D5*/ /*BUSY=D12*/);

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

TextBuffer tb;
DiskFont diskfont;

void updateDisplay(DISPLAY_UPDATE_TYPE d);
void updateDisplay(DISPLAY_UPDATE_TYPE d, String messagetext, uint16_t messagecolor);
void epaperUpdate();
void epaperDisplayImage();

//#ifdef DISPLAY_SPI_3WIRE
//Epd epd(SPI_3WIRE);
//#else
//Epd epd();
//#endif

//DiskFont diskfont;

//unsigned char image[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];
//unsigned char image_red[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];

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
  char reset_info_buf[200];
  
  struct rst_info *rtc_info = system_get_rst_info();
  
  os_sprintf(reset_info_buf, "reset reason: %x\n",  rtc_info->reason);
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
      return true;
  }

  return false;
}


void setup() { 
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  SD.begin(D1, SPI_HALF_SPEED);
    
  SPIFFS.begin();
  //Serial.begin(9600);
  DEBUG_PRT.begin(1,3,8);
  delay(100);

  DEBUG_PRT.print(F("free memory = "));
  DEBUG_PRT.println(String(system_get_free_heap_size()));

  String resetreason = "";
  bool bCrashed = false;
  
  bCrashed = CrashCheck(resetreason);
  DEBUG_PRT.println(resetreason);

  if (bCrashed) {
    display_image(crash_bug, resetreason, false);
    Config::PowerOff(0); // power off until USB5V is (re)connected
    ESP.deepSleep(0); 
  }

  wake_reason = Config::Wake_Reason();

  if (wake_reason != WAKE_ALARM_1) Config::SetPowerOn(); //attempt to hold up alarm 1 flag A1F - not writable in the DS3231 spec, but useful to keep the power on if the wake reason was not an alarm

  DEBUG_PRT.println("--------------------------");
  DEBUG_PRT.println("abcdefghijklmnopqrstuvwxyz");
  DEBUG_PRT.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  DEBUG_PRT.println("0123456789");
  DEBUG_PRT.println("--------------------------");
  
  DEBUG_PRT.println("Running");

  //WiFi.disconnect(); // testing - so have to connect to a network each reboot

  battery_test();

  clock_battery_test();

  // Check if EEPROM checksum is good

  //Config c;
  bEEPROM_checksum_good = Config::EEPROMChecksumValid();

  if (!bEEPROM_checksum_good && !Battery::power_connected()) {
    DEBUG_PRT.println(F("On battery power and EEPROM checksum invalid: Sleeping until USB power is attached and web interface is used to configure EEPROM"));
    display_image(connect_power);
    if (!Config::PowerOff(0)) {
      DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
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
      bDisplayWifiConnectedScreen = false;      
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
            if (!Config::PowerOff(0)) {
              DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
              ESP.deepSleep(0); // sleep indefinitely, reset pulse will wake the ESP when USB power is unplugged and plugged in again. //sleep for an hour (71minutes is the maximum!), or until power is connected SLEEP_HOUR
            }
          }
        }
        else {
          //ESP.deepSleep(1e6); // reset esp, network is configured
          //ESP.reset();
          ESP.restart();
        }
      }
    }
    else {
      bNetworkAvailable = true;
      if (bDisplayWifiConnectedScreen) {
        display_image(wireless_network_connected, Network::getDHCPAddressAsString(), true);
        rtcData.data.wake_flags = WAKE_FLAGS_STILL_CHARGING_DONT_REDISPLAY_WIFI_CONNECTED_IMAGE;
        Config::writeRtcMemoryData(rtcData); // set the wake flag to say don't redisplay the wifi connected image on next boot, if still charging by 
                                             // that time.
      }
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
          DEBUG_PRT.printf("No reading this hour, and on battery, so going back to sleep immediately.\nNumber of hours to next reading is %d\n", rtcData.data.wake_hour_counter);
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

      if (!Config::PowerOff(0)) { // **24-11-2018 use RTC power switch to turn off all peripherals rather than just place esp8266 in deepSleep mode
        DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
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

void display_image(DISPLAY_UPDATE_TYPE i) {
  display_image(i, "", false);
}

void display_image(DISPLAY_UPDATE_TYPE d, String messagetext, bool bMessageRed) {
  uint16_t messagecolor = GxEPD_BLACK;
  if (bMessageRed) messagecolor = GxEPD_RED;
  
  updateDisplay(d, messagetext, messagecolor);  
}

void loop(void) { 
  /************************************************/ 
  // *0* Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  config_server();  // config server will run if battery power is connected
  
  /************************************************/ 
  // *1* Create calendar object and load config.csv
  wdt_reset();

  int next_wake_hours_count = 1; // will store the number of hours until the next wake up
  
  DEBUG_PRT.println(F("*1*\n"));
  //timeserver.gps_wake();
  Calendar c(D1);

  if (bEEPROM_checksum_good) {  
    if (!c._I18n->configparams.have_config) {
      DEBUG_PRT.println(F("Error: Failed to get config: config.csv is missing or bad or no SD card inserted"));
      display_image(sd_card_not_inserted);
      //ESP.deepSleep(SLEEP_HOUR);    
      if (!Config::PowerOff(0)) {
        DEBUG_PRT.println(F("Attempt to power off via DS3231 failed, using deepsleep mode"));
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
    
    Lectionary::ReadingsFromEnum r;

    bool getLectionaryReadingEveryHour = true; // was false. If we get here, must return a reading, since reading scheduling when using DeepSleep mode 
                                               // (which wakes *every hour* to check if a reading is due) is now handled in the init() code
    //if (wake_reason != WAKE_ALARM_1 || wake_reason == WAKE_USB_5V || Battery::power_connected()) getLectionaryReadingEveryHour = true;    
        
    if (getLectionaryReading(date, &r, getLectionaryReadingEveryHour/*true Battery::power_connected()*/, b_OT, b_NT, b_PS, b_G)) {      
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
      if (!display_calendar(datetime, &c, refs, right_to_left, verse_per_line, show_verse_numbers)) { // if there is no reading for the current part of the day, display the Gospel reading instead (rare)
        DEBUG_PRT.println(F("No reading found (Apocrypha missing from this Bible?). Displaying Gospel reading instead\n"));
        r=Lectionary::READINGS_G;
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
    
    if (Hour != 23) { // if crossing a day boundary, the season and/or number of readings may change, so will always wake on a day boundary, so make the next wake time default to 1 hour
      while (!getLectionaryReading(date + (SECS_PER_HOUR * i), &r, false, b_OT, b_NT, b_PS, b_G)) { // check if there will be a reading next hour
        rtcData.data.wake_hour_counter++;
        i++;
        Hour = (Hour + 1 > 23) ? 0 : Hour + 1;
        if (Hour == 0) break;
      }
    }

    DEBUG_PRT.printf("Next reading is in %d hour(s)\n", rtcData.data.wake_hour_counter); 

    Config::writeRtcMemoryData(rtcData);        

    next_wake_hours_count = rtcData.data.wake_hour_counter; // for setting wake alarm
  
    DEBUG_PRT.println(F("*6*\n"));
  } // if(bEEPROM_checksum_good)
  
  
  /************************************************/ 
  // *7* Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
/*
  DEBUG_PRT.println(F("*7*\n"));
  //timeserver.gps_sleep();

  if (!bEEPROM_checksum_good) {
    if (Battery::power_connected() && bNetworkAvailable) {
      display_image(clock_not_set, Network::getDHCPAddressAsString(), true);
    } else {
      display_image(connect_power);
    }
  }

  bool bSettingsUpdated = false;

  if (!bNetworkAvailable && Battery::power_connected()) {
    DEBUG_PRT.println(F("On USB power, but Network is not available - disconnect and reconnect power to use WPS setup"));
  }

  if (Battery::power_connected() && bNetworkAvailable) {
    DEBUG_PRT.println(F("Power is connected, starting config web server"));
    DEBUG_PRT.print(F("USB voltage is "));
    DEBUG_PRT.println(String(Battery::battery_voltage()));

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
   
    if (Config::StartServer()) {
      DEBUG_PRT.println(F("Config web server started, listening for requests..."));
      while(Battery::power_connected() && !Config::bSettingsUpdated && !bTimeUp) {
        server.handleClient();
        wdt_reset();
        delay(1000);
        //DEBUG_PRT.println("Battery voltage is " + String(Battery::battery_voltage()));
        if (millis() > (server_start_time + 1000*8*60)) bTimeUp = true; // run the server for an 10 minutes max, then sleep. If still on usb power, the web server will run again.
      }

      Config::StopServer();

      if (Config::bSettingsUpdated) {
        DEBUG_PRT.println(F("Settings updated, resetting lectionary..."));
        ESP.deepSleep(1e6); //reboot after 1 second
        //ESP.reset();
      }
      else if (bTimeUp) {
        DEBUG_PRT.println(F("Server timed out, stopping web server and going to sleep"));
        //ESP.deepSleep(SLEEP_HOUR - (1000*8*60));
        SleepForHours(next_wake_hours_count);
        //SleepUntilStartOfHour();
      }
      else {
        DEBUG_PRT.println(F("Power disconnected, stopping web server and going to sleep"));
      }
  
      //DEBUG_PRT.println("Battery voltage is " + String(Battery::battery_voltage()));
      
      DEBUG_PRT.print(F("free memory = "));
      DEBUG_PRT.println(String(system_get_free_heap_size()));
    }
  }
  else {
    DEBUG_PRT.print(F("Battery voltage is "));
    DEBUG_PRT.println(String(Battery::battery_voltage()));
  }
*/
  /************************************************/ 
  // *8* completed all tasks, go to sleep
  DEBUG_PRT.println(F("*8*\n"));
  
  DEBUG_PRT.println(F("Going to sleep"));
  SleepForHours(next_wake_hours_count);
}

void config_server()
{
  // Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  DEBUG_PRT.println(F("*0*\n"));

  if (!bDisplayWifiConnectedScreen) return; // only run the webserver if the wifi connected screen is shown (will only show once, when power is first connected). So after resetting (eg. when settings have been updated) it the webserver should not be run

  if (!bEEPROM_checksum_good) {
    if (Battery::power_connected() && bNetworkAvailable) {
      display_image(clock_not_set, Network::getDHCPAddressAsString(), true);
    } else {
      display_image(connect_power);
    }
  }

  bool bSettingsUpdated = false;

  if (!bNetworkAvailable && Battery::power_connected()) {
    DEBUG_PRT.println(F("On USB power, but Network is not available - disconnect and reconnect power to use WPS setup"));
  }

  if (Battery::power_connected() && bNetworkAvailable) {
    DEBUG_PRT.println(F("Power is connected, starting config web server"));
    DEBUG_PRT.print(F("USB voltage is "));
    DEBUG_PRT.println(String(Battery::battery_voltage()));

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
   
    if (Config::StartServer()) {
      DEBUG_PRT.println(F("Config web server started, listening for requests..."));
      while(Battery::power_connected() && !Config::bSettingsUpdated && !bTimeUp) {
        server.handleClient();
        wdt_reset();
        delay(1000);
        //DEBUG_PRT.println("Battery voltage is " + String(Battery::battery_voltage()));
        if (millis() > (server_start_time + 1000*8*60)) bTimeUp = true; // run the server for an 10 minutes max, then sleep. If still on usb power, the web server will run again.
      }

      Config::StopServer();

      if (Config::bSettingsUpdated) {
        DEBUG_PRT.println(F("Settings updated, resetting lectionary..."));
        ESP.deepSleep(1e6); //reboot after 1 second
      }
      else if (bTimeUp) {
        DEBUG_PRT.println(F("Server timed out, stopping web server and displaying reading"));
        //ESP.deepSleep(SLEEP_HOUR - (1000*8*60));
        //SleepForHours(next_wake_hours_count);
        //SleepUntilStartOfHour();
      }
      else {
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
    ESP.deepSleep(sleepduration_minutes * 60e6);
    return; // should never return because ESP should be asleep!
}


bool getLectionaryReading(time64_t date, Lectionary::ReadingsFromEnum* r, bool bReturnReadingForAllHours, bool b_OT, bool b_NT, bool b_PS, bool b_G) {
  DEBUG_PRT.printf("getLectionaryReading() bReturnReadingFromAllHours=%s ", bReturnReadingForAllHours?"true":"false");
  //Lectionary::ReadingsFromEnum r;
  tmElements_t tm;
  breakTime(date, tm);

  DEBUG_PRT.printf("hour=%02d\n", tm.Hour);

  bool bHaveLectionaryValue = false;

  if(tm.Day == 24 && tm.Month == 12 && tm.Hour >= 18) { // Christmas Eve Vigil Mass
    DEBUG_PRT.println(F("Christmas Eve vigil Mass"));
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
    DEBUG_PRT.println(F("Christmas Day"));
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
          *r=Lectionary::READINGS_G;

          if (!b_OT) {
            *r=Lectionary::READINGS_NT;
          }
          
          if (!b_NT) {
            *r=Lectionary::READINGS_OT; // one or other of b_OT, b_NT should be true. Defaults to G if both are false.
          }
          break;
          
        case 14:
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
/*
bool epd_init(void) {
  if (epd.Init() != 0) {
    DEBUG_PRT.printf("e-Paper init failed\n");
    return false;
  }
  return true;
}

void init_panel() {
  DEBUG_PRT.printf("Initializing panel...");
  if (epd_init()) {
    epd.ClearFrame();
    DEBUG_PRT.printf("done\n");
  }
}
*/

bool display_calendar(String date, Calendar* c, String refs, bool right_to_left, bool verse_per_line, bool show_verse_numbers) {  
  //DiskFont diskfont;
  
  DEBUG_PRT.print(F("Selected font is "));
  DEBUG_PRT.println(c->_I18n->configparams.font_filename);
  
  if (diskfont.begin(c->_I18n->configparams)) {
    DEBUG_PRT.println(F("Font opened successfully"));
  }
  else {
    display_image(font_missing, c->_I18n->configparams.font_filename, false);
    DEBUG_PRT.println(F("Font not found, using internal font"));    
  }
  
  //FONT_INFO* font = &calibri_10pt;
  
  //FONT_INFO* font_bold = &calibri_8ptBold;
  //Paint paint(image, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);
  //paint.Clear(UNCOLORED);

  //Paint paint_red(image_red, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  //paint_red.SetRotate(ROTATE_90);
  //paint_red.Clear(UNCOLORED);
  
  bool bRed = (c->temporale->getColour() == Enums::COLOURS_RED) ? true : false;
  
  DEBUG_PRT.println(F("Displaying calendar"));
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
//  display_verses(c, refs, &paint, &paint_red, font, &diskfont);
  
  //diskfont.end();
  
  updateDisplay(display_reading);
  
  //init_panel();
  //epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  //epd.TransmitPartialRed(paint_red.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  //epd.DisplayFrame();
  //epd.Sleep();
  
  uint32_t free = system_get_free_heap_size();
  DEBUG_PRT.print(F("free memory = "));
  DEBUG_PRT.println(String(free));
  DEBUG_PRT.println(F("done")); 

  return true;
}

void display_day(String d, bool bRed, bool right_to_left) {
  DEBUG_PRT.print(F("display_day() d="));
  DEBUG_PRT.println(String(d));
  
  int text_xpos = (PANEL_SIZE_X / 2) - (int)((diskfont.GetTextWidthA(d, true))/2); // true => shape text before calculating width
  //DEBUG_PRT.println("display_day() text_xpos = " + String(text_xpos));
  
  int text_ypos = 0;
  
  int fbwidth = PANEL_SIZE_X;
  int fbheight = PANEL_SIZE_Y;

  Bidi::RenderText(d, &text_xpos, &text_ypos, tb, diskfont, &bRed, fbwidth, fbheight, right_to_left, false);

  //int charheight = diskfont->_FontHeader.charheight;  
  //paint->DrawLine(0, charheight, 264, charheight, COLORED);
}

void display_date(String date, String day, bool right_to_left) {
  DEBUG_PRT.print(F("\ndisplay_date: s="));
  DEBUG_PRT.println(date);

  bool bEmphasisOn = false;

  int fbwidth = PANEL_SIZE_X;
  int fbheight = PANEL_SIZE_Y;

  int text_xpos = 0;
  int text_ypos = 0;

  text_xpos = PANEL_SIZE_X - (int)(diskfont.GetTextWidthA(date, true)); // right justified, shape text before calculating width
  text_ypos = PANEL_SIZE_Y - diskfont._FontHeader.charheight;
  Bidi::RenderText(date, &text_xpos, &text_ypos, tb, diskfont, &bEmphasisOn, fbwidth, fbheight, right_to_left, false);

  text_xpos = 0;
  text_ypos = PANEL_SIZE_Y - diskfont._FontHeader.charheight;
  Bidi::RenderText(day, &text_xpos, &text_ypos, tb, diskfont, &bEmphasisOn, fbwidth, fbheight, right_to_left, false);
   
  //diskfont->DrawStringAt(text_xpos, PANEL_SIZE_Y - diskfont->_FontHeader.charheight, date, paint, COLORED, false);
  //diskfont->DrawStringAt(0, PANEL_SIZE_Y - diskfont->_FontHeader.charheight, day, paint, COLORED, false);
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

  int fbwidth = PANEL_SIZE_X;
  int fbheight = PANEL_SIZE_Y;
  
//  Bidi bidi;

  fbheight = PANEL_SIZE_Y - diskfont._FontHeader.charheight;
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
