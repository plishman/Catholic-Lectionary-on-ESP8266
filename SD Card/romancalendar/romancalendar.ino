#include <TimeLib.h>
#include <RomanCalendar.h>

//#define c(character) (Serial.print(character);)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  RomanCalendar calendar = new RomanCalendar(true);

  // put your main code here, to run repeatedly:
  Serial.print("\n\ntemporaletests()\n");

  time_t t;
  tmElements_t ts;
  
  int y = 2017;
  
  for (int m = 1; m <= 12; m++) {
    int days = get_monthdays(m, y);

    for (int d = 1; d <= days; d++) {
      t = calendar.date(d, m, y);
      if (t != (time_t)-1) {
        breakTime(t, ts);
        calendar.liturgical_day(t);
        
        Serial.print(ts.Day);
        slash();
        Serial.print(ts.Month);
        slash();
        Serial.print(year(t));
        colon();
        tab();
        Serial.print(calendar._season);
        tab();
        Serial.print(calendar._buffer);
        cr();
        //Serial.printf("%s\t\"%s\"\t\"%s\"\t\"%s\"\n", datetime, calendar._season, calendar._colour, calendar._buffer);
      }
    }
  }
}

void slash(void) {
  Serial.print("/");
}

void colon(void) {
  Serial.print(":");
}

void tab(void) {
  Serial.print("\t");
}

void cr(void) {
  Serial.print("\n");
}

int get_monthdays(int mon, int year) {
    static const int days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int leap = yisleap(year) ? 1 : 0;
    return days[mon] + leap;
}

bool yisleap(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

