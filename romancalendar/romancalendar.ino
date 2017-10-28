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

Epd epd;
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

  time_t date = c.temporale->date(1,11,2017);
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

  display_calendar(c.day, datetime);

  Serial.println("*7*\n");

  

  String refs;
  l.get(c.day.liturgical_year, c.day.liturgical_cycle, Lectionary::READINGS_G, c.day.lectionary, &refs);

  Bible b;
  b.get(refs);
  Serial.println("*8*\n");
    
  //b.get("Eccl 1:2, 2:21-23");
  //b.get("John 3:16");
  //b.get("Wisdom 3:1-2");
  //b.get("Gen 37:3-4, 12-13a, 17b-28a");
  //b.get("Ps 22:1-23:1");
  //b.get("Jonah 4:1-11");
  //b.get("Gen 16:1-12, 15-16 or 16:6b-12, 15-16");
  //b.get("Matt 9:35-10:1, 5a, 6-8");
  //b.get("Deut 32:35cd-36ab");
  //b.get("Josh 3:7-10a, 11, 13-17");
  //b.get("Ps 51:3-4, 5-6, 12-13, 14+17");
  //b.get("Ps 118:1-2, 16-17, 22-23");
  b.dump_refs();

  //Serial.print("creating BibleVerse object: ");
  //I18n* pI18n = &i18n;
  BibleVerse bv(c._I18n);
  //Serial.println("done.");  

  Ref* r;
  int i = 0;

  r = b.refsList.get(0);

  while (r != NULL) {
    int start_chapter = r->start_chapter;
    int end_chapter = r->end_chapter;
    int verse = r->start_verse;
    int start_verse;
    int end_verse;
    int start_sentence;
    int end_sentence;
    String verse_text;
    String output;

    for (int c = start_chapter; c <= end_chapter; c++) {
      printf("\n%d:", c);
      if (c < end_chapter) {
        end_verse = -1; //b.books_chaptercounts[r->book_index]; // -1 -> output until last verse
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
    
      //for (int v = start_verse; v <= end_verse; v++) {
      while (!bDone) {
        if (bv.get(r->book_index, c, v, &verse_text)) {
          printf(" %d ", v);

          if (c == start_chapter && v == start_verse) { // this deals with the intra-verse references a,b,c etc. It might not work
            start_sentence = r->start_first_sentence;     // exactly, since boundaries may not align with commas or sentences, this is a best guess.
            end_sentence = r->start_last_sentence;
          }
          else if (c == end_chapter && v == end_verse) {
            start_sentence = r->end_first_sentence;
            end_sentence = r->end_last_sentence;
          }
          else {
            start_sentence = -1;
            end_sentence = -1;
          }

          start_sentence = -1; // this feature doesn't work as expected, so return all parts of verse
          end_sentence = -1;   // 

          output += output_verse(verse_text, start_sentence, end_sentence) + "\n";
          printf("\n");
          v++;
          if (end_verse != -1 && v > end_verse) bDone = true; // end_verse will be set to -1 if all verses up to the end of the chapter are to be returned.
        }
        else {
          printf("Error\n");
          bDone = true;
        }
      } 
    }

    //epd_write(output);
    
    //i++;
    //r = b.refsList.get(i);
    r = NULL;
  }
  
  while(1) {
    ESP.deepSleep(20e6); //20 seconds
    Serial.print(".");
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
#define PANEL_SIZE_X 264
#define PANEL_SIZE_Y 176

void init_panel() {
  epd_init();
  epd.ClearFrame();  
}

void display_calendar(Calendar::Day day, String date) {
  FONT_INFO* font = &calibri_8pt;
  
  init_panel();
  unsigned char image[1024];
  
  /**
    * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
    * In this case, a smaller image buffer is allocated and you have to 
    * update a partial display several times.
    * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
    */
  if (day.is_sanctorale) {
    display_day(day.sanctorale, image, font);
    if (day.sanctorale == day.day) {
      display_date(date, "", image, font);  
    } else {
      display_date(date, day.day, image, font);
    }
  } else {
    display_day(day.day, image, font);    
    display_date(date, "", image, font);
  }
  

  epd.DisplayFrame();
  epd.Sleep();
  Serial.println("done");  
}

void display_day(String d, unsigned char* image, FONT_INFO* font) {
  int title_bar_y_height = font->heightPages + 1;

  Paint paint(image, title_bar_y_height, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  paint.SetRotate(ROTATE_90);

  int text_xpos = (paint.GetHeight() / 2) - ((paint.GetTextWidth(d.c_str(), font))/2);
  
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(text_xpos, 0, d.c_str(), font, COLORED);
  paint.DrawLine(0, 15, 264, 15, COLORED);
  epd.TransmitPartialBlack(paint.GetImage(), PANEL_SIZE_Y - title_bar_y_height, 0, paint.GetWidth(), paint.GetHeight());
}

void display_date(String date, String day, unsigned char* image, FONT_INFO* font) {
  printf("\ndisplay_date: s= %s\n", date.c_str());

  /**
    * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
    * In this case, a smaller image buffer is allocated and you have to 
    * update a partial display several times.
    * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
    */
  Paint paint(image, font->heightPages, PANEL_SIZE_X); //792bytes used    //width should be the multiple of 8 
  paint.SetRotate(ROTATE_90);

  int text_xpos = paint.GetHeight() - (paint.GetTextWidth(date.c_str(), font)); // right justified
 
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(text_xpos, 0, date.c_str(), font, COLORED);

  paint.DrawStringAt(0, 0, day.c_str(), font, COLORED);
  
  //paint.DrawLine(0, 15, 264, 15, COLORED);
  epd.TransmitPartialBlack(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
}
