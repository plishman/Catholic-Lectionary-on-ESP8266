// Catholic Lectionary on ESP
// Copyright (c) 2017 Philip Lishman, All rights reserved.

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
#include <BibleVerse.h>
#include <SPI.h>
#include <epd2in7b.h>
#include <epdpaint.h>
#include <calibri8pt.h>
//#include <calibri8ptbold.h>
#include <pgmspace.h>
#include <Network.h>
#include <Battery.h>
#include <Config.h>
#include <images.h>

extern "C" {
#include "user_interface.h"
}

#define COLORED     1
#define UNCOLORED   0

#define PANEL_SIZE_X 264
#define PANEL_SIZE_Y 176

#define SLEEP_HOUR 60*60e6

I2CSerialPort I2CSerial;

Epd epd;

unsigned char image[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];
unsigned char image_red[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];

/*
char jan[] PROGMEM = "Jan";
char feb[] PROGMEM = "Feb";
char mar[] PROGMEM = "Mar";
char apr[] PROGMEM = "Apr";
char may[] PROGMEM = "May";
char jun[] PROGMEM = "Jun";
char jul[] PROGMEM = "Jul";
char aug[] PROGMEM = "Aug";
char sep[] PROGMEM = "Sep";
char oct[] PROGMEM = "Oct";
char nov[] PROGMEM = "Nov";
char dec[] PROGMEM = "Dec";

PGM_P months[] PROGMEM = {jan, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec};
*/

Battery battery;
Network network;

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
        ESP.deepSleep(SLEEP_HOUR); // sleep for an hour (71minutes is the maximum!), or until power is connected
      } else {
        ESP.deepSleep(1e6); // reset esp, network is configured
        //ESP.reset();
      }
    }
  }
  else {
    I2CSerial.println("On battery power and no network configured - will not be able to use web interface until network is connected. Connect power to start setup.");
    //I2CSerial.println("On battery power and no network configured: Sleeping until USB power is attached and network is configured");
    //display_image(connect_power_image);
    //while(1) {
    //  delay(5000);
    //}
    //ESP.deepSleep(SLEEP_HOUR); // sleep for an hour, or until power is connected
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
      ESP.deepSleep(SLEEP_HOUR); // sleep for an hour or until power is connected
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
  Calendar c(true, D1);

  if (!c._I18n->_have_config) {
    I2CSerial.println("Error: Failed to get config: will reboot after 30 seconds...");
    ESP.deepSleep(30e6);    
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
  String refs;
  wdt_reset();
  Lectionary l(c._I18n);
  
  Lectionary::ReadingsFromEnum r;
  if (getLectionaryReading(date, &r, battery.power_connected())) {  
    l.get(c.day.liturgical_year, c.day.liturgical_cycle, r, c.day.lectionary, &refs);    
    
    /************************************************/ 
    // *6* Update epaper display with reading
    display_calendar(datetime, &c, refs);
  }
  else {
    I2CSerial.println("On battery, reading will not be updated for this hour (updates every 4 hours, at 8am, 12pm, 4pm, 8pm and midnight");    
  }

  I2CSerial.println("*6*\n");
  /************************************************/ 
  // *7* Check battery and if on usb power, start web server to allow user to use browser to configure lectionary (address is http://lectionary.local)
  I2CSerial.println("*7*\n");
  //timeserver.gps_sleep();
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
        delay(250);
        if (millis() > (server_start_time + 1000*3600*3)) bTimeUp = true; // run the server for an 3 hours max, then reboot. If still on usb power, the web server will run again.
      }

      c._config->StopServer();

      if (bSettingsUpdated) {
        I2CSerial.println("Settings updated, resetting lectionary...");
        ESP.deepSleep(1e6);
        //ESP.reset();
      }
      else if (bTimeUp) {
        I2CSerial.println("Server timed out, stopping web server restarting lectionary...");
        ESP.deepSleep(1e6);
      }
      else {
        I2CSerial.println("Power disconnected, stopping web server and going to sleep");
      }
  
      I2CSerial.println("Battery voltage is " + String(battery.battery_voltage()));
      
      free = system_get_free_heap_size();
      I2CSerial.println("free memory = " + String(free));
    }
  }


  
  /************************************************/ 
  // *8* completed all tasks, go to sleep
  I2CSerial.println("*8*\n");
  
  while(1) {
    I2CSerial.println("Going to sleep");
    ESP.deepSleep(SLEEP_HOUR); //1 hour
  }
}


bool getLectionaryReading(time64_t date, Lectionary::ReadingsFromEnum* r, bool bReturnReadingForAllHours) {
  //Lectionary::ReadingsFromEnum r;
  tmElements_t tm;
  breakTime(date, tm);

  bool bHaveLectionaryValue = false;

  if(tm.Day == 24 && tm.Month == 12 && tm.Hour >= 18) { // Christmas Eve Vigil Mass
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
    // this will show the Gospel reading between the hours of midnight and 8am, and 8pm and midnight
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

  return bHaveLectionaryValue;
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
  FONT_INFO* font = &calibri_8pt;
  //FONT_INFO* font_bold = &calibri_8ptBold;
  init_panel();
  Paint paint(image, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint.SetRotate(ROTATE_90);
  paint.Clear(UNCOLORED);

  Paint paint_red(image_red, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint_red.SetRotate(ROTATE_90);
  paint_red.Clear(UNCOLORED);

//  String w = "abcdefghijklmnop qrstuvw xyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrs tuvwxyzABCDE FGHIJKLMNOPQRSTU VWXYZ0123456789";
//  String word_part = "";
//  int width = 0;
//  bool bEOL = false;
//  
//  while(w.length() != 0) {
//    bEOL = hyphenate_word(&w, &word_part, &width, font, &paint);
//    Serial.print(word_part);
//    if (bEOL) I2CSerial.println();
//  }
//  return;

  bool bRed = (c->temporale->getColour() == Enums::COLOURS_RED) ? true : false;
  
  I2CSerial.println("Displaying calendar");
  if (c->day.is_sanctorale) {
    display_day(c->day.sanctorale, &paint, &paint_red, font, bRed);
    if (c->day.sanctorale == c->day.day) {
      display_date(date, "", &paint, font);  
    } else {
      display_date(date, c->day.day, &paint, font);
    }
  } else {
    display_day(c->day.day, &paint, &paint_red, font, bRed);    
    display_date(date, "", &paint, font);
  }

  I2CSerial.println("Displaying verses");
  display_verses(c, refs, &paint, &paint_red, font);

  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.TransmitPartialRed(paint_red.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.DisplayFrame();

  epd.Sleep();
  I2CSerial.println("done");  
}

void display_day(String d, Paint* paint, Paint* paint_red, FONT_INFO* font, bool bRed) {
  I2CSerial.println("display_day() d=" + d);
  
  //Paint paint(image, title_bar_y_height, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);

  int text_xpos = (paint->GetHeight() / 2) - ((paint->GetTextWidth(d, font))/2);

  if (bRed) {
    paint_red->DrawStringAt(text_xpos, 0, d, font, COLORED);
  } else {
    paint->DrawStringAt(text_xpos, 0, d, font, COLORED);
  }
  paint->DrawLine(0, 15, 264, 15, COLORED);
  //epd.TransmitPartialBlack(paint.GetImage(), PANEL_SIZE_Y - title_bar_y_height, 0, paint.GetWidth(), paint.GetHeight());
}

void display_date(String date, String day, Paint* paint, FONT_INFO* font) {
  I2CSerial.println("\ndisplay_date: s=" + date);

  //Paint paint(image, font->heightPages, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);

  int text_xpos = PANEL_SIZE_X - (paint->GetTextWidth(date, font)); // right justified
 
  //paint.Clear(UNCOLORED);
  paint->DrawStringAt(text_xpos, PANEL_SIZE_Y - font->heightPages, date, font, COLORED);
  paint->DrawStringAt(0, PANEL_SIZE_Y - font->heightPages, day, font, COLORED);
  
  //paint.DrawLine(0, 15, 264, 15, COLORED);
  //epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
}

bool display_verses(Calendar* c, String refs, Paint* paint_black, Paint* paint_red, FONT_INFO* font) {
  Bible b(c->_I18n);
  if (!b.get(refs)) return false;
    
  b.dump_refs();

  Ref* r;

  int i = 0;

  r = b.refsList.get(i++);

  bool bEndOfScreen = false;

  int xpos = 0;
  int ypos = font->heightPages;

  String line_above = "";
  bool bDisplayRefs = true;

  while (r != NULL && !bEndOfScreen) {     
    int start_chapter = r->start_chapter;
    int end_chapter = r->end_chapter;
    int verse = r->start_verse;
    int start_verse;
    int end_verse;
    int start_sentence;
    int end_sentence;
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
    
      while (!bDone && !bEndOfScreen) {
        if (b._bibleverse->get(r->book_index, c, v, &verse_record)) {
          I2CSerial.printf(" %d ", v);
          verse_text = get_verse(verse_record, &book_name);

          if (bDisplayRefs) {
            String refs_i18n = refs;
            if (book_name != "") refs_i18n.replace(b.books_shortnames[r->book_index], book_name);
            I2CSerial.printf("refs_i18n = %s\n", refs_i18n.c_str());
            bEndOfScreen = epd_verse(refs_i18n, paint_red, &xpos, &ypos, font); // returns false if at end of screen
            bDisplayRefs = false;
          }
          //
          //bEndOfScreen = epd_verse(String(v), paint_red, &xpos, &ypos, font); // returns false if at end of screen
          bEndOfScreen = epd_verse(verse_text, paint_black, &xpos, &ypos, font); // returns false if at end of screen
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

bool epd_verse(String verse, Paint* paint, int* xpos, int* ypos, FONT_INFO* font) {
  I2CSerial.println("epd_verse() verse=" + verse);

  String word_part = "";
  bool bEOL = false;
  
  int width = 0;
  
  int line_height = font->heightPages;
  
  if (*ypos > (PANEL_SIZE_Y + line_height)) return true;
  
  while(verse.length() > 0) {
    bEOL = hyphenate_word(&verse, &word_part, &width, font, paint);

    if (*xpos + width > PANEL_SIZE_X) {
      *xpos = 0;
      (*ypos)+= font->heightPages;      
      if (*ypos > (PANEL_SIZE_Y - line_height)) return true;
    }
    
    wdt_reset();
    paint->DrawStringAt(*xpos, *ypos, word_part, font, COLORED);
    (*xpos) += width;
    wdt_reset();
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

String get_verse(String verse_record, String* book_name) { // a bit naughty, verse_record strings can contain multiple lines of csv records for verses that span more than one line.
  I2CSerial.println(verse_record);
  Csv csv;

  int pos = 0;

  String fragment = "";
  String verse = "";
  bool more_than_one = false;
  
  do {
    int i = 0;
    do {
      //I2CSerial.println("pos = " + String(pos));
      fragment = csv.getCsvField(verse_record, &pos);
      if (i == 0 && !more_than_one) {
        *book_name = fragment;
      }

      if (i == 4) {
        verse+=((more_than_one?" ":"") + fragment);
      }
    } while (pos < verse_record.length() && i++ != 4);
    //I2CSerial.println("pos = " + String(pos) + " charAt pos = [" + String(verse_record.charAt(pos)) + "]");
    more_than_one = true;
  } while (pos < verse_record.length());
  
  return verse;
}

bool hyphenate_word(String *w, String* word_part, int* x_width, FONT_INFO* font, Paint* paint) {
  int j = 0;
  bool bHyphenDone = false;
  String w_ch;
  String w_str;
  String w_str_last;
  int hyphen_width = paint->GetTextWidth("-", font);
  int len = w->length();
  String hyphen = "-";
  bool bEOL = false;
  
  while(!bHyphenDone) {
    w_str_last = w_str;
    w_ch = utf8CharAt(*w, j);
    w_str += w_ch;

    if (paint->GetTextWidth(w_str, font) > (PANEL_SIZE_X - hyphen_width)) {
      bEOL = true;
      bHyphenDone = true;
    } else {
      bEOL = false;
      w_str_last = w_str;
      j += w_ch.length();

      if (w_ch == " " || j >= len) {
        hyphen = "";
        bHyphenDone = true;
      }
    }
  }
  //Serial.print("[" + String(w->length()) + " " + String(w_str_last.length()) + "]");
  *w = w->substring(w_str_last.length());
  *word_part = (w_str_last + hyphen);
  *x_width = paint->GetTextWidth(*word_part, font);
  return bEOL;
}
