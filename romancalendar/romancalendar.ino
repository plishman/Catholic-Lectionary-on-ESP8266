//ESP8266---
#include "ESP8266WiFi.h"
//----------
#include <pins_arduino.h>
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
#include <TimeServer.h>

#define COLORED     1
#define UNCOLORED   0

#define PANEL_SIZE_X 264
#define PANEL_SIZE_Y 176

Epd epd;

unsigned char image[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];
unsigned char image_red[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];


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

void setup() {
  //ESP8266------
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  //-------------
  
  Serial.begin(9600);

  while(!Serial) {
  }
  
  Serial.println("Running");
}

TimeServer timeserver;

void loop(void) {
  /************************************************/ 
  wdt_reset();
  timeserver.gps_wake();
  
  Serial.println("*1*\n");
  Calendar c(true, D1);
  Serial.println("*2*\n");
  wdt_reset();
  Lectionary l(c._I18n);
  Serial.println("*3*\n");

  wdt_reset();
  //time_t date = c.temporale->date(7,11,2017);
  Serial.println("Getting local datetime...");
  time_t date = timeserver.local_datetime();
  Serial.println("Got local datetime.");
  c.get(date);
  Serial.println("*4*\n");

  tmElements_t ts;
  breakTime(date, ts);

  char m[16];
  strcpy_P(m, (const char*)pgm_read_ptr(&(months[ts.Month - 1])));

  String datetime = String(ts.Day) + " " + String(m) + " " + String(ts.Year + 1970);
  Serial.println("*5*\n");

  Serial.print(datetime + "\t" + c.day.season + "\t" + c.day.day + "\t" + c.day.colour + "\t" + c.day.rank);
  if (c.day.is_sanctorale) {
    Serial.print("\t" + c.day.sanctorale + "\t" + c.day.sanctorale_colour + "\t" + c.day.sanctorale_rank);
  }
  Serial.println("*6*\n");

  Serial.println();

  String refs;
  l.get(c.day.liturgical_year, c.day.liturgical_cycle, Lectionary::READINGS_G, c.day.lectionary, &refs);

  display_calendar(datetime, &c, refs);

  Serial.println("*8*\n");

  timeserver.gps_sleep();
  
  while(1) {
    delay(15000);
    //ESP.deepSleep(20e6); //20 seconds
  }
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
//    if (bEOL) Serial.println();
//  }
//  return;

  bool bRed = (c->temporale->getColour() == Enums::COLOURS_RED) ? true : false;
  
  Serial.println("Displaying calendar");
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

  Serial.println("Displaying verses");
  display_verses(c, refs, &paint, font);

  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  
  epd.TransmitPartialRed(paint_red.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  

  epd.DisplayFrame();
  epd.Sleep();
  Serial.println("done");  
}

void display_day(String d, Paint* paint, Paint* paint_red, FONT_INFO* font, bool bRed) {
  //Serial.println("display_day() d=" + d);
  
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
  Serial.println("\ndisplay_date: s=" + date);

  //Paint paint(image, font->heightPages, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);

  int text_xpos = PANEL_SIZE_X - (paint->GetTextWidth(date, font)); // right justified
 
  //paint.Clear(UNCOLORED);
  paint->DrawStringAt(text_xpos, PANEL_SIZE_Y - font->heightPages, date, font, COLORED);
  paint->DrawStringAt(0, PANEL_SIZE_Y - font->heightPages, day, font, COLORED);
  
  //paint.DrawLine(0, 15, 264, 15, COLORED);
  //epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
}

void display_verses(Calendar* c, String refs, Paint* paint, FONT_INFO* font) {
  Bible b(c->_I18n);
  if (!b.get(refs)) return;
  Serial.println("*7*\n");
    
  b.dump_refs();

  Ref* r;

  int i = 0;

  r = b.refsList.get(i++);

  bool bEndOfScreen = false;

  int xpos = 0;
  int ypos = font->heightPages;

  String line_above = "";

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
      printf("\n%d:", c);
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
    
      while (!bDone && !bEndOfScreen) {
        if (b._bibleverse->get(r->book_index, c, v, &verse_record)) {
          printf(" %d ", v);
          verse_text = get_verse(verse_record);

          bEndOfScreen = epd_verse(verse_text, paint, &xpos, &ypos, font); // returns false if at end of screen
          Serial.println("epd_verse returned " + String(bEndOfScreen ? "true":"false"));
          printf("\n");
          v++;
          if (v > end_verse) bDone = true; // end_verse will be set to -1 if all verses up to the end of the chapter are to be returned.
        }
        else {
          printf("Error\n");
          bDone = true;
        }
      }
      if (bEndOfScreen) break; // out of chapter for loop
    }
    r = b.refsList.get(i++);
  }
}

bool epd_verse(String verse, Paint* paint, int* xpos, int* ypos, FONT_INFO* font) {
  Serial.println("epd_verse() verse=" + verse);

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
  //Serial.println("String=" + s);
  
  if (pos >= s.length()) {
    //Serial.println("utf8CharAt string length is " + String(ps->length()) + " *ppos = " + String(*ppos));
    return String("");
  }
  
  int charLen = charLenBytesUTF8(s.charAt(pos));

  //Serial.println("char at pos " + String(*ppos) + " = " + String(ps->charAt(*ppos)) + "utf8 charLen = " + String(charLen));

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
  //Serial.println(String(ch)+ ";");

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

String get_verse(String verse_record) { // a bit naughty, verse_record strings can contain multiple lines of csv records for verses that span more than one line.
  Serial.println(verse_record);
  Csv csv;

  int pos = 0;

  String fragment = "";
  String verse = "";
  bool more_than_one = false;
  
  do {
    int i = 0;
    do {
      //Serial.println("pos = " + String(pos));
      fragment = csv.getCsvField(verse_record, &pos);
      if (i == 4) {
        verse+=((more_than_one?" ":"") + fragment);
        //return f;
      }
    } while (pos < verse_record.length() && i++ != 4);
    //Serial.println("pos = " + String(pos) + " charAt pos = [" + String(verse_record.charAt(pos)) + "]");
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


