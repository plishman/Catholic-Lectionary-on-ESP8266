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

void setup() {

  Serial.begin(115200);

  while(!Serial) {
  }
  
  Serial.println("Running");

  //Serial.print("creating I18n object: ");
  //Serial.println("done.");
}

void loop(void) {
  Calendar c(true, Enums::LANGUAGE_EN, D8);
  Lectionary l(c._I18n);

  time_t date = c.temporale->date(25,12,2017);
  c.temporale->print_date(date);
  c.get(date);

  tmElements_t ts;
  breakTime(date, ts);
  String datetime = String(ts.Day) + "/" + String(ts.Month) + "/" + String(ts.Year + 1970);

  Serial.print(datetime + "\t" + c.day.season + "\t" + c.day.day + "\t" + c.day.colour + "\t" + c.day.rank);
  if (c.day.is_sanctorale) {
    Serial.print("\t" + c.day.sanctorale + "\t" + c.day.sanctorale_colour + "\t" + c.day.sanctorale_rank);
  }

  Serial.println();

  
  String refs;
  l.get(c.day.liturgical_year, c.day.liturgical_cycle, Lectionary::READINGS_G, c.day.lectionary, &refs);

  Bible b;
  b.get(refs);
    
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

          output_verse(verse_text, start_sentence, end_sentence);
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
    i++;
    r = b.refsList.get(i);
  }

  while(1) {
    delay(500);
    ESP.wdtFeed();
  }
}

void output_verse(String verse, int start_sentence, int end_sentence) {
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
}

