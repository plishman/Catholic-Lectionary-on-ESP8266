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
#include <pgmspace.h>

#define COLORED     1
#define UNCOLORED   0

#define PANEL_SIZE_X 264
#define PANEL_SIZE_Y 176

Epd epd;

unsigned char image[PANEL_SIZE_X*(PANEL_SIZE_Y/8)];

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

  Serial.begin(115200);

  while(!Serial) {
  }
  
  Serial.println("Running");
}

void loop(void) {
  /************************************************/ 
  Serial.println("*1*\n");
  Calendar c(true, Enums::LANGUAGE_EN, D1);
  Serial.println("*2*\n");
  Lectionary l(c._I18n);
  Serial.println("*3*\n");

  time_t date = c.temporale->date(29,10,2017);
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
  
  while(1) {
    delay(2000);
    //ESP.deepSleep(20e6); //20 seconds
  }
}

String output_verse(String verse, int start_sentence, int end_sentence) {
  Csv csv;

  int pos = 0;
  int i = 0;

  String f;
  do {
    //Serial.println("pos = " + String(pos));
    f = csv.getCsvField(verse, &pos);
    if (i == 5) {
      if (start_sentence == -1 && end_sentence == -1) {
        printf("%s", f.c_str());
      }
      else {
        //printf("start_sentence=%d, end_sentence=%d\n", start_sentence, end_sentence);

        int start_charpos = 0;
        int end_charpos = 0;

        if (start_sentence > -1 && end_sentence == -1) { // print from start_sentence to end of verse
          end_charpos = f.length();

          for (int i = 0; i <= start_sentence; i++) { // must get done at least once
            start_charpos = f.indexOf(".", start_charpos);
          }
          
        }

        if (start_sentence == -1 && end_sentence > -1) { // print from start of verse to end_sentence
          start_charpos = 0;

          for (int i = 0; i <= end_sentence; i++) { // must get done at least once
            end_charpos = f.indexOf(".", end_charpos);
          }
        }

        //printf("start_charpos=%d, end_charpos=%d, f.length()=%d", start_charpos, end_charpos, f.length());
        printf("%s\n", f.substring(start_charpos, end_charpos).c_str());
        //printf("[%s]\n", f.c_str());
      }
    }
  } while (pos < verse.length() && i++ < 6);

  return f;
}

bool epd_init(void) {
  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return false;
  }
  return true;
}

void epd_write(String s) {
  epd_init();
  Serial.print("writing to the display");
  epd.ClearFrame();

  /**
    * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
    * In this case, a smaller image buffer is allocated and you have to 
    * update a partial display several times.
    * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
    */
  unsigned char image[1024];
  Paint paint(image, 24, 264);    //width should be the multiple of 8 

  paint.SetRotate(ROTATE_90);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 0, s.c_str(), &Font12, COLORED);
  epd.TransmitPartialBlack(paint.GetImage(), 154, 0, paint.GetWidth(), paint.GetHeight());

  epd.DisplayFrame();
  epd.Sleep();
  Serial.println("done");
}

//---------------------------------------------------------------------------calendar
void init_panel() {
  epd_init();
  epd.ClearFrame();  
}

void display_calendar(String date, Calendar* c, String refs) {
  FONT_INFO* font = &calibri_8pt;
  
  init_panel();
  Paint paint(image, PANEL_SIZE_Y, PANEL_SIZE_X); //5808 bytes used (full frame) //792bytes used    //width should be the multiple of 8 
  paint.SetRotate(ROTATE_90);
  paint.Clear(UNCOLORED);
  
  if (c->day.is_sanctorale) {
    display_day(c->day.sanctorale, &paint, font);
    if (c->day.sanctorale == c->day.day) {
      display_date(date, "", &paint, font);  
    } else {
      display_date(date, c->day.day, &paint, font);
    }
  } else {
    display_day(c->day.day, &paint, font);    
    display_date(date, "", &paint, font);
  }
  
  display_verses(c, refs, &paint, font);

  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());  

  epd.DisplayFrame();
  epd.Sleep();
  Serial.println("done");  
}

void display_day(String d, Paint* paint, FONT_INFO* font) {
  //Serial.println("display_day() d=" + d);
  
  //Paint paint(image, title_bar_y_height, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);

  int text_xpos = (paint->GetHeight() / 2) - ((paint->GetTextWidth(d.c_str(), font))/2);
  
  paint->DrawStringAt(text_xpos, 0, d.c_str(), font, COLORED);
  paint->DrawLine(0, 15, 264, 15, COLORED);
  //epd.TransmitPartialBlack(paint.GetImage(), PANEL_SIZE_Y - title_bar_y_height, 0, paint.GetWidth(), paint.GetHeight());
}

void display_date(String date, String day, Paint* paint, FONT_INFO* font) {
  printf("\ndisplay_date: s= %s\n", date.c_str());

  //Paint paint(image, font->heightPages, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);

  int text_xpos = PANEL_SIZE_X - (paint->GetTextWidth(date.c_str(), font)); // right justified
 
  //paint.Clear(UNCOLORED);
  paint->DrawStringAt(text_xpos, PANEL_SIZE_Y - font->heightPages, date.c_str(), font, COLORED);
  paint->DrawStringAt(0, PANEL_SIZE_Y - font->heightPages, day.c_str(), font, COLORED);
  
  //paint.DrawLine(0, 15, 264, 15, COLORED);
  //epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
}

void display_verses(Calendar* c, String refs, Paint* paint, FONT_INFO* font) {
  Bible b;
  b.get(refs);
  Serial.println("*7*\n");
    
  b.dump_refs();

  BibleVerse bv(c->_I18n);

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
        if (bv.get(r->book_index, c, v, &verse_record)) {
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
  
  int line_height = font->heightPages;
  
  if (*ypos > (PANEL_SIZE_Y + line_height)) return false;
  
  //Paint paint(image, (8*((font->heightPages)/8)) + 8, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  //paint.SetRotate(ROTATE_90);

  int xsize;
  int next_space_pos;
  String w = "";
  String s = "";
  String last_s = "";

  int len = verse.length();
  int s_len = 0;
  int i = 0;
  int last_space_index = 0;
  String ch;
  
  bool bDone = false;
  
  while (!bDone) {
    ch = utf8CharAt(verse, i);
    //Serial.print(ch);
    //Serial.print(String(ch.length()));
    
    if (ch == " " || i == len) {
      last_s = s;
      w = verse.substring(last_space_index, i);
      s += w;
      last_space_index = i;
      
      s_len = s.length();

      Serial.println("i=" + String(i) + " len=" + String(len) + " xpos=" + String(*xpos) + " ypos=" + String(*ypos));

      if (paint->GetTextWidth(s.c_str(), font) > (PANEL_SIZE_X - (*xpos)) || i == len) {
        //paint->Clear(UNCOLORED);
        if (i == len) {
          paint->DrawStringAt(*xpos, *ypos, s.c_str(), font, COLORED);
          bDone = true;
          //epd.TransmitPartialBlack(paint.GetImage(), y, *xpos, paint.GetWidth(), paint.GetHeight());
        } else {
          paint->DrawStringAt(*xpos, *ypos, last_s.c_str(), font, COLORED);
          s = w;
          //epd.TransmitPartialBlack(paint.GetImage(), y, *xpos, paint.GetWidth(), paint.GetHeight());  
          *xpos = 0;
          (*ypos)+= font->heightPages; // higher coordinates are towards to top of the screen, so subsequent lines are at lower y coordinates
        }

        if (i == len) {
          *xpos = paint->GetTextWidth(s.c_str(), font);
        } 
        
        Serial.println("*ypos=" + String(*ypos) + " line_height=" + String(line_height));
        if (*ypos > (PANEL_SIZE_Y - line_height)) return true;
      }
    }

    i += ch.length();
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

String get_verse(String verse_record) {
  Csv csv;

  int pos = 0;
  int i = 0;

  String f;
  do {
    //Serial.println("pos = " + String(pos));
    f = csv.getCsvField(verse_record, &pos);
    if (i == 5) {
      return f;
    }
  } while (pos < verse_record.length() && i++ < 6);

  return "";
}

