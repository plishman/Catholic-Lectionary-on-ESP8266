//#include "stdafx.h" // comment out for cross-platform/embedded. It doesn't play well with Windows Visual Studio if you wrap it in a #ifdef _WIN32!
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
	#include "Arduino.h"
	#include "../Time/TimeLib.h"
#else
	#include <time.h>
#endif

#include "RomanCalendar.h"

const char* const RomanCalendar::DAYS_OF_WEEK[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

const char* const RomanCalendar::SUNDAY_TRANSFERABLE_SOLEMNITIES[3] = { "epiphany", "ascension", "corpus_christi" };

const char* const RomanCalendar::SOLEMNITIES[17] = {
	"The Nativity of the Lord",							//nativity
	"The Holy Family of Jesus, Mary and Joseph",		//holy_family
	"Octave Day of Christmas, of Mary, Mother of God",	//mother_of_god
	"The Epiphany of the Lord",							//epiphany
	"The Baptism of the Lord",							//baptism_of_lord
	"Ash Wednesday",									//ash_wednesday
	"Palm Sunday of the Passion of the Lord",			//palm_sunday
	"Friday of the Passion of the Lord",				//good_friday
	"Holy Saturday",									//holy_saturday
	"Easter Sunday of the Resurrection of the Lord",	//easter_sunday
	"Ascension of the Lord",							//ascension
	"Pentecost Sunday",									//pentecost
	"The Most Holy Trinity",							//holy_trinity
	"The Most Holy Body and Blood of Christ",			//corpus_christi
	"The Most Sacred Heart of Jesus",					//sacred_heart
	"Immaculate Heart of Mary",							//immaculate_heart
	"Our Lord Jesus Christ, King of the Universe"		//christ_king
};

const RomanCalendar::Ranks RomanCalendar::SOLEMNITIES_RANKS[17] = {		// the ranks of the solemnities, in the same order as in solemnities[17]
	RANKS_PRIMARY,										//nativity
	RANKS_FEAST_LORD_GENERAL,							//holy_family
	RANKS_SOLEMNITY_GENERAL,							//mother_of_god
	RANKS_PRIMARY,										//epiphany
	RANKS_FEAST_LORD_GENERAL,							//baptism_of_lord
	RANKS_PRIMARY,										//ash_wednesday
	RANKS_PRIMARY,										//palm_sunday
	RANKS_TRIDUUM,										//good_friday
	RANKS_TRIDUUM,										//holy_saturday
	RANKS_TRIDUUM,										//easter_sunday
	RANKS_PRIMARY,										//ascension
	RANKS_PRIMARY,										//pentecost
	RANKS_SOLEMNITY_GENERAL,							//holy_trinity
	RANKS_SOLEMNITY_GENERAL,							//corpus_christi
	RANKS_SOLEMNITY_GENERAL,							//sacred_heart
	RANKS_MEMORIAL_GENERAL,								//immaculate_heart
	RANKS_SOLEMNITY_GENERAL								//christ_king
};

const RomanCalendar::Colours RomanCalendar::SOLEMNITIES_COLOURS[17] = {	// the colours of the solemnities, in the same order as in solemnities[17]
	COLOURS_WHITE,										//nativity
	COLOURS_WHITE,										//holy_family
	COLOURS_WHITE,										//mother_of_god
	COLOURS_WHITE,										//epiphany
	COLOURS_WHITE,										//baptism_of_lord
	COLOURS_VIOLET,										//ash_wednesday
	COLOURS_RED,										//palm_sunday
	COLOURS_RED,										//good_friday
	COLOURS_VIOLET,										//holy_saturday
	COLOURS_WHITE,										//easter_sunday
	COLOURS_WHITE,										//ascension
	COLOURS_RED,										//pentecost
	COLOURS_WHITE,										//holy_trinity
	COLOURS_WHITE,										//corpus_christi
	COLOURS_WHITE,										//sacred_heart
	COLOURS_WHITE,										//immaculate_heart
	COLOURS_WHITE										//christ_king
};

const char* const RomanCalendar::SEASONS[5] = {
	"Advent",
	"Christmas Season",
	"Lent",
	"Easter Season",
	"Ordinary Time"
};

const char* const RomanCalendar::RANK_PRIORITY[14] = {
	"4.0", "3.13", "3.12", "3.11", "3.10", "2.9", "2.8", "2.7", "2.6", "2.5", "1.4", "1.3", "1.2", "1.1"
};

const RomanCalendar::RankType RomanCalendar::RANK_TYPE[14] = {
	RANKTYPE_EASTER_TRIDUUM,
	RANKTYPE_PRIMARY_LITURGICAL_DAYS,
	RANKTYPE_SOLEMNITY,
	RANKTYPE_SOLEMNITY,
	RANKTYPE_FEAST,
	RANKTYPE_SUNDAY,
	RANKTYPE_FEAST,
	RANKTYPE_FEAST,
	RANKTYPE_FERIAL,
	RANKTYPE_MEMORIAL,
	RANKTYPE_MEMORIAL,
	RANKTYPE_MEMORIAL_OPT,
	RANKTYPE_FERIAL,
	RANKTYPE_COMMEMORATION
};

const char* const RomanCalendar::COLOURS[4] = {
	"Green",	//GREEN = Colour.new(:green),
	"Violet",	//VIOLET = Colour.new(:violet),
	"White",	//WHITE = Colour.new(:white),
	"Red"		//RED = Colour.new(:red)
};

const char* const RomanCalendar::RANK_NAME[14] = { // order of array items is significant, since they are indexed by Ranks enum members
	"Commemorations",
	"Ferials",
	"Optional memorials",
	"Proper obligatory memorials",
	"Obligatory memorials in the General Calendar",
	"Privileged ferials",
	"Proper feasts",
	"Feasts of saints in the General Calendar",
	"Unprivileged Sundays",
	"Feasts of the Lord in the General Calendar",
	"Proper solemnities",
	"Solemnities in the General Calendar",
	"Primary liturgical days",
	"Easter triduum",
};

const char* const RomanCalendar::RANK_TYPES[9] = {
	"", // these first two are for easter and primary liturgical days, where no short description is needed
	"",
	"solemnity",
	"feast",
	"Sunday",
	"memorial",
	"optional memorial",
	"ferial",
	"commemoration"
};

RomanCalendar::RomanCalendar(bool transfer_to_sunday) {
	_transfer_to_sunday = transfer_to_sunday;
	return;
}

RomanCalendar::~RomanCalendar(void) {
	return;
}


void RomanCalendar::datestests() {
#ifdef _WIN32
	printf("\n\ndatestests()\n");

	struct tm ts;
	time64_t t;

	bool bResult = getTm(29, 10, 2017, 2, 30, 0, &ts);

	if (bResult) {
		t = tuesday_before(mktime(&ts));
		t = tuesday_after(mktime(&ts));
	}
#endif
}

bool RomanCalendar::getTm(int day, int month, int year, int hours, int minutes, int seconds, struct tm* ts) {
#ifdef _WIN32
	ts->tm_sec = seconds;					/* seconds,  range 0 to 59          */
	ts->tm_min = minutes;					/* minutes, range 0 to 59           */
	ts->tm_hour = hours;					/* hours, range 0 to 23             */
	ts->tm_mday = day;						/* day of the month, range 1 to 31  */
	ts->tm_mon = month - 1;					/* month, range 0 to 11             */ // subtract 1, make it 1-12
	ts->tm_year = year - BEGIN_EPOCH;		/* The number of years since 1900   */
	ts->tm_isdst = -1;

	time64_t checkValid = mktime(ts);
	if (checkValid == ((time64_t)-1))
	{
		return false;
	}
#endif
	return true;
}

time64_t RomanCalendar::date(int day, int month, int year) {
#ifdef _WIN32
	struct tm ts;
	ts.tm_sec = 0;							/* seconds,  range 0 to 59          */
	ts.tm_min = 0;							/* minutes, range 0 to 59           */
	ts.tm_hour = 0;							/* hours, range 0 to 23             */
	ts.tm_mday = day;						/* day of the month, range 1 to 31  */
	ts.tm_mon = month - 1;					/* month, range 0 to 11             */ // subtract 1, make it 1-12
	ts.tm_year = year - BEGIN_EPOCH;		/* The number of years since 1900   */
	ts.tm_isdst = 0;

	return mktime(&ts);
#else
	::tmElements_t ts;						// for arduino
	ts.Second = 0;							/* seconds,  range 0 to 59          */
	ts.Minute = 0;							/* minutes, range 0 to 59           */
	ts.Hour = 0;							/* hours, range 0 to 23             */
	ts.Day = day;							/* day of the month, range 1 to 31  */
	ts.Month = month;						/* month, range 1 to 12             */
	ts.Year = year - BEGIN_EPOCH;			/* The number of years since 1970   */

	return ::makeTime(ts);
#endif
}

time64_t RomanCalendar::weekday_before(int weekdayBefore, time64_t date) {
	if (weekdayBefore < 0 || weekdayBefore > 6) return (time64_t)-1;

#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	int currentWeekday = ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	int currentWeekday = ts.Wday - 1;
#endif`
	//	s->tm_isdst = -1; // set is dst to auto


	int daysBefore = WEEK; // if the date is exactly one week ago

	if (currentWeekday != weekdayBefore) { // otherwise (will be true in most cases)
		daysBefore = currentWeekday - weekdayBefore;
		if (daysBefore < 0) daysBefore += WEEK;
	}

	time64_t outputDate = date - (daysBefore * DAY); // subtract number of days (in seconds) from date

												   /*
												   char inDateStr[100];
												   char outDateStr[100];
												   sprintf(inDateStr, "%s", ctime(&date));
												   sprintf(outDateStr,"%s", ctime(&outputDate));
												   printf("Output: the %s before %s is %s\n", DAYS_OF_WEEK[weekdayBefore], inDateStr, outDateStr);
												   */

	return outputDate;

	// bug: when crossing dst, may be ahead or behind by one hour - will not affect actual date.
}

time64_t RomanCalendar::sunday_before(time64_t date) { return weekday_before(0, date); }
time64_t RomanCalendar::monday_before(time64_t date) { return weekday_before(1, date); }
time64_t RomanCalendar::tuesday_before(time64_t date) { return weekday_before(2, date); }
time64_t RomanCalendar::wednesday_before(time64_t date) { return weekday_before(3, date); }
time64_t RomanCalendar::thursday_before(time64_t date) { return weekday_before(4, date); }
time64_t RomanCalendar::friday_before(time64_t date) { return weekday_before(5, date); }
time64_t RomanCalendar::saturday_before(time64_t date) { return weekday_before(6, date); }

time64_t RomanCalendar::weekday_after(int weekdayAfter, time64_t date) {
	if (weekdayAfter < 0 || weekdayAfter > 6) return (time64_t)-1;

#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	int currentWeekday = ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	int currentWeekday = ts.Wday - 1;
#endif

	int daysAfter = WEEK; // if the date is exactly one week later

	if (currentWeekday != weekdayAfter) { // otherwise (will be true in most cases)
		daysAfter = weekdayAfter - currentWeekday;
		if (daysAfter < 0) daysAfter += WEEK;
	}

	time64_t outputDate = date + (daysAfter * DAY);  // add number of days (in seconds) from date

												   /*
												   char inDateStr[100];
												   char outDateStr[100];
												   sprintf(inDateStr, "%s", ctime(&date));
												   sprintf(outDateStr, "%s", ctime(&outputDate));
												   printf("Output: the %s after %s is %s\n", DAYS_OF_WEEK[weekdayAfter], inDateStr, outDateStr);
												   */

	return outputDate;
	// bug: when crossing dst, may be ahead or behind by one hour - will not affect actual date.
}

time64_t RomanCalendar::sunday_after(time64_t date) { return weekday_after(0, date); }
time64_t RomanCalendar::monday_after(time64_t date) { return weekday_after(1, date); }
time64_t RomanCalendar::tuesday_after(time64_t date) { return weekday_after(2, date); }
time64_t RomanCalendar::wednesday_after(time64_t date) { return weekday_after(3, date); }
time64_t RomanCalendar::thursday_after(time64_t date) { return weekday_after(4, date); }
time64_t RomanCalendar::friday_after(time64_t date) { return weekday_after(5, date); }
time64_t RomanCalendar::saturday_after(time64_t date) { return weekday_after(6, date); }

int RomanCalendar::dayofweek(time64_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	return ts.Wday - 1;
#endif
}

bool RomanCalendar::sunday(time64_t date) { return (dayofweek(date) == 0); }
bool RomanCalendar::monday(time64_t date) { return (dayofweek(date) == 1); }
bool RomanCalendar::tuesday(time64_t date) { return (dayofweek(date) == 2); }
bool RomanCalendar::wednesday(time64_t date) { return (dayofweek(date) == 3); }
bool RomanCalendar::thursday(time64_t date) { return (dayofweek(date) == 4); }
bool RomanCalendar::friday(time64_t date) { return (dayofweek(date) == 5); }
bool RomanCalendar::saturday(time64_t date) { return (dayofweek(date) == 6); }

int RomanCalendar::year(time64_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_year + BEGIN_EPOCH;
#else
	return ::year(date); // for arduino
#endif
}

int RomanCalendar::dayofmonth(time64_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_mday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	return ts.Day;
#endif
}

bool RomanCalendar::issameday(time64_t date1, time64_t date2) {
	/*
	printf("date1:");
	print_date(date1);
	printf(": date2:");
	print_date(date2);
	printf("\n\n");
	*/

#ifdef _WIN32
	struct tm* ts = gmtime(&date1);
	int d1 = ts->tm_mday;
	int m1 = ts->tm_mon;
	int y1 = ts->tm_year;

	ts = gmtime(&date2); // there is only one copy of the tm structure, so subsequent calls to gmtime will overwrite the values from earlier calls

	if (ts->tm_mday == d1 && ts->tm_mon == m1 && ts->tm_year == y1) return true;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date1, ts);

	int d1 = ts.Day;
	int m1 = ts.Month;
	int y1 = ts.Year;

	::breakTime(date2, ts);
	if (ts.Day == d1 && ts.Month == m1 && ts.Year == y1) return true;
#endif

	return false;

}

int RomanCalendar::date_difference(time64_t date1, time64_t date2) { // assumes that time64_t values are seconds since 1970 or 1900, and can have arithmetic
																 //if (date1 >= date2) {								 // performed on them. Not guaranteed on all systems, but should work on arduino embedded.
																 //	return (int)(date1 - date2);
																 //}
																 //else {
	return (int)(date2 - date1);
	//}
}

time64_t RomanCalendar::start_date(int year) {
	return first_advent_sunday(year);
}

time64_t RomanCalendar::end_date(int year) {
	return first_advent_sunday(year + 1) - DAY;
}

time64_t RomanCalendar::first_advent_sunday(int year) {
	return sunday_before(nativity(year)) - (3 * WEEK * DAY);
}

time64_t RomanCalendar::nativity(int year) {
	return date(25, 12, year);
}

time64_t RomanCalendar::holy_family(int year) {
	time64_t xmas = nativity(year);

	if (sunday(xmas)) {
		return date(30, 12, year);
	}
	else {
		return sunday_after(xmas);
	}
}

time64_t RomanCalendar::mother_of_god(int year) {
	return octave_of(nativity(year));
}

time64_t RomanCalendar::epiphany(int year) {
	if (_transfer_to_sunday) {
		return sunday_after(date(1, 1, year + 1));
	}

	return date(6, 1, year + 1);
}

time64_t RomanCalendar::baptism_of_lord(int year) { // maybe watch this, since its had the most modding from the ruby version
	time64_t e = epiphany(year);
	if (_transfer_to_sunday) {
		e += DAY;
	}
	else {
		e = sunday_after(e);
	}
	/*
	printf("baptism_of_lord=");
	print_date(e);
	printf("\n");
	*/
	return e;
}

time64_t RomanCalendar::ash_wednesday(int year) {
	return easter_sunday(year) - (((6 * WEEK) + 4) * DAY);
}

time64_t RomanCalendar::easter_sunday(int year) {
	year += 1;

	//# algorithm below taken from the 'easter' gem:
	//# https ://github.com/jrobertson/easter

	int golden_number = (year % 19) + 1;
	int dominical_number = (year + (year / 4) - (year / 100) + (year / 400)) % 7;
	int solar_correction = (year - 1600) / 100 - (year - 1600) / 400;
	int lunar_correction = (((year - 1400) / 100) * 8) / 25;
	int paschal_full_moon = (3 - 11 * golden_number + solar_correction - lunar_correction) % 30;

	while (!(dominical_number > 0)) {
		dominical_number += 7;
	}

	while (!(paschal_full_moon > 0)) {
		paschal_full_moon += 30;
	}

	if ((paschal_full_moon == 29) || (paschal_full_moon == 28 && golden_number > 11)) {
		paschal_full_moon -= 1;
	}

	int difference = (4 - paschal_full_moon - dominical_number) % 7;
	if (difference < 0) difference += 7;

	int day_easter = paschal_full_moon + difference + 1;
	if (day_easter < 11) {
		//# Easter occurs in March.
		return date(day_easter + 21, 3, year);
	}
	else {
		//# Easter occurs in April.
		return date(day_easter - 10, 4, year);
	}
}

time64_t RomanCalendar::palm_sunday(int year) {
	return easter_sunday(year) - (7 * DAY);
}

time64_t RomanCalendar::good_friday(int year) {
	return easter_sunday(year) - (2 * DAY);
}

time64_t RomanCalendar::holy_saturday(int year) {
	return easter_sunday(year) - (1 * DAY);
}

time64_t RomanCalendar::ascension(int year) {
	if (_transfer_to_sunday) {
		return easter_sunday(year) + ((6 * WEEK) * DAY); // return 42 days after Easter sunday
	}

	return pentecost(year) - (10 * DAY); // return ascension Thursday (40 days after Easter sunday)
}

time64_t RomanCalendar::pentecost(int year) {
	return easter_sunday(year) + ((7 * WEEK) * DAY);
}

time64_t RomanCalendar::holy_trinity(int year) {
	return octave_of(pentecost(year));
}

time64_t RomanCalendar::corpus_christi(int year) {
	if (_transfer_to_sunday) {
		return holy_trinity(year) + (WEEK * DAY); // should be 60 days after Easter Sunday if on a Thursday, or 63 days after if transferred to Sunday
	}
	return holy_trinity(year) + (4 * DAY);
}

time64_t RomanCalendar::sacred_heart(int year) {
	return corpus_christi(year) + (8 * DAY);
}

time64_t RomanCalendar::immaculate_heart(int year) {
	return pentecost(year) + (20 * DAY);
}

time64_t RomanCalendar::christ_king(int year) {
	return first_advent_sunday(year) - (7 * DAY);
}

time64_t RomanCalendar::octave_of(time64_t date) {
	return date + (WEEK * DAY);
}

int RomanCalendar::liturgical_year(time64_t date) {
	int _year = year(date);
	if (date < first_advent_sunday(_year)) {
		return _year - 1;
	}

	return _year;
}

int RomanCalendar::for_day(time64_t date) {
	return liturgical_year(date);
}

//# which liturgical season is it ? (returns a 'Season' enum)
RomanCalendar::Season RomanCalendar::season(time64_t date) {
	int year = RomanCalendar::year(date);

	//printf("season: year is %d\n", year);

	if ((first_advent_sunday(year) <= date) && (nativity(year) > date)) {
		return SEASON_ADVENT;
	}

	if ((nativity(liturgical_year(date)) <= date) && (baptism_of_lord(liturgical_year(date))) >= date) {
		/*printf("--dates of nativity for date, and baptism of lord\n");
		print_date(date);
		print_date(nativity(liturgical_year(date)));
		print_date(baptism_of_lord(liturgical_year(date)));
		printf("--\n");
		*/
		return SEASON_CHRISTMAS;
	}

	if ((ash_wednesday(liturgical_year(date)) <= date) && (easter_sunday(liturgical_year(date)) > date)) {
		return SEASON_LENT;
	}

	if ((easter_sunday(liturgical_year(date)) <= date) && (pentecost(liturgical_year(date)) >= date)) {
		return SEASON_EASTER;
	}

	return SEASON_ORDINARY;
}

time64_t RomanCalendar::season_beginning(RomanCalendar::Season s, time64_t date) {
	int year = liturgical_year(date);

	if (s == SEASON_ADVENT) return first_advent_sunday(year);
	if (s == SEASON_CHRISTMAS) return nativity(year);
	if (s == SEASON_LENT) return ash_wednesday(year);
	if (s == SEASON_EASTER) return easter_sunday(year);
	if (s == SEASON_ORDINARY) return baptism_of_lord(year) + DAY;

	return (time64_t)-1;
}

int RomanCalendar::season_week(RomanCalendar::Season seasonn, time64_t date) {
	int year = RomanCalendar::year(date);

	time64_t week1_beginning = season_beginning(seasonn, date);
	int week = 0;

	/*
	if (seasonn == SEASON_ORDINARY) {

	printf("----\n");
	printf("input date: ");
	print_date(date);
	printf(" week1_beginning before munging:");
	print_date(week1_beginning);

	printf("issunday(input date) = %s\n", sunday(date) ? "true":"false");
	printf("issunday(week1_beginning) = %s\n", sunday(week1_beginning) ? "true" : "false");

	}
	*/

	if (!sunday(week1_beginning)) { // Lent begins on Ash Wednesday. For advent, first sunday is in week one, not second sunday
		week1_beginning = sunday_after(week1_beginning);
	}

	if (week1_beginning <= date) {
		week = (date_difference(week1_beginning, date) / (DAY * WEEK)) + 1;
		//printf("date_difference:%d\t", date_difference(week1_beginning, date));
	}
	else {
		week = 0;
		//printf("|");
	}

	if (seasonn == SEASON_LENT && week == 0) {
		week = sunday(date) ? 1 : 0;
		//printf("*");
	}

	if (seasonn == SEASON_ORDINARY) {
		//# ordinary time does not begin with Sunday, but the first week
		//# is week 1, not 0
		week += 1;					// first period of ordinary time

									/*
									printf("year: %d", year);
									printf(" pentecost: ");
									print_date(pentecost(year - 1));
									printf(" this date: ");
									print_date(date);
									printf("------------------------------------------------\n");
									*/

		if (date > pentecost(year - 1)) { // second period of ordinary time
										  /*
										  printf("first_advent_sunday = ");
										  print_date(first_advent_sunday(RomanCalendar::year(date)));
										  printf("\t");
										  */
			int weeks_after_date = (date_difference(date, first_advent_sunday(RomanCalendar::year(date)))) / (WEEK * DAY);
			week = 34 - weeks_after_date;
			if (sunday(date)) week += 1;
		}
	}
	/*
	printf("week1_beginning: ");
	print_date(week1_beginning);
	printf("\t");

	printf("season: %s\t week: %d\t", SEASONS[seasonn], week);
	*/
	return week;

	/*
	int year = RomanCalendar::year(date);
	time64_t week1_beginning = season_beginning(seasonn, date);

	if (!sunday(week1_beginning)) { // Lent begins on Ash Wednesday, this will set the starting week to the following sunday
	week1_beginning = sunday_after(week1_beginning);
	}

	int week = (date_difference(week1_beginning, date) / (DAY * WEEK)) + 1;

	if (seasonn == SEASON_ORDINARY) {
	week += 1;

	if (date > pentecost(year)) { // second period of ordinary time
	int weeks_after_date = (date_difference(date, first_advent_sunday(RomanCalendar::year(date)))) / (WEEK * DAY);
	week = 34 - weeks_after_date;
	if (sunday(date)) week += 1;
	}
	}

	printf("week == %d\n", week);
	return week;
	*/
}

const RomanCalendar::Season SEASONS_SUNDAY_PRIMARY[3] = { RomanCalendar::SEASON_ADVENT, RomanCalendar::SEASON_LENT, RomanCalendar::SEASON_EASTER };
char* RomanCalendar::sunday_temporale(time64_t date) {
	if (!(sunday(date))) return NULL;

	Season seas = season(date);
	Ranks rank = RANKS_SUNDAY_UNPRIVILEGED;

	if (seas == SEASON_ADVENT || seas == SEASON_LENT || seas == SEASON_EASTER) {
		rank = RANKS_PRIMARY;
	}

	int week = season_week(seas, date);
	ordinalize(week);

	switch (seas)
	{
	case SEASON_ADVENT:
		sprintf(_buffer, "%s Sunday of Advent", _ordinal); // %s = week
		break;

	case SEASON_CHRISTMAS:
		sprintf(_buffer, "%s Sunday after the Nativity of the Lord", _ordinal); // %s = week
		break;

	case SEASON_EASTER:
		sprintf(_buffer, "%s Sunday of Easter", _ordinal); // %s = week
		break;

	case SEASON_LENT:
		sprintf(_buffer, "%s Sunday of Lent", _ordinal); // %s = week
		break;

	case SEASON_ORDINARY:
		sprintf(_buffer, "%s Sunday in Ordinary Time", _ordinal); // %s = week
		break;
	}
	return _buffer;
}

char* RomanCalendar::ferial_temporale(time64_t date) {
	Season seas = season(date);
	int week = season_week(seas, date);
	Ranks rank = RANKS_FERIAL;

	bool bIsSet = false;

	switch (seas)
	{
	case SEASON_ADVENT:
		if (date >= RomanCalendar::date(17, 12, year(date))) {
			rank = RANKS_FERIAL_PRIVILEGED;
			ordinalize(dayofmonth(date)); // writes it into the object string variable "_ordinal"
			sprintf(_buffer, "%s December", _ordinal); // %s = ordinal day of month
			bIsSet = true;
		}
		break;

	case SEASON_CHRISTMAS:
		if (date < mother_of_god(liturgical_year(date))) {
			rank = RANKS_FERIAL_PRIVILEGED;

			//print_date(date);
			//print_date(mother_of_god(liturgical_year(date)));
			//printf("dayofmonth(date) = %d, dayofmonth(nativity() = %d\n", dayofmonth(date), dayofmonth(nativity(year(date))));
			//printf("mother_of_god(%d)=", RomanCalendar::year(date));
			//print_date(mother_of_god(RomanCalendar::year(date)));
			//printf("*\n");

			ordinalize(dayofmonth(date) - dayofmonth(nativity(RomanCalendar::year(date))) + 1); //# 1 - based counting;
			sprintf(_buffer, "%s day of Christmas Octave", _ordinal);
			bIsSet = true;
		}
		else if (date > epiphany(RomanCalendar::year(date))) {
			sprintf(_buffer, "%s after Epiphany", DAYS_OF_WEEK[dayofweek(date)]);
			bIsSet = true;
		}
		break;

	case SEASON_LENT:
		if (week == 0) {
			sprintf(_buffer, "%s after Ash Wednesday", DAYS_OF_WEEK[dayofweek(date)]);
			bIsSet = true;
		}
		else if (date > palm_sunday(year(date))) {
			rank = RANKS_PRIMARY;
			sprintf(_buffer, "%s of Holy Week", DAYS_OF_WEEK[dayofweek(date)]);
			bIsSet = true;
		}
		rank = (rank > RANKS_FERIAL_PRIVILEGED) ? rank : RANKS_FERIAL_PRIVILEGED; // watch this - comparison is dependent on the in which each enum member represents, which have been chosen to make this work
		break;

	case SEASON_EASTER:
		if (week == 1) {
			rank = RANKS_PRIMARY;
			sprintf(_buffer, "Easter %s", DAYS_OF_WEEK[dayofweek(date)]);
			bIsSet = true;
		}
		break;
	}

	if (bIsSet) return _buffer;

	switch (seas)
	{
	case SEASON_ADVENT:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		sprintf(_buffer, "%s, %s week of Advent", DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		break;

	case SEASON_CHRISTMAS:
		sprintf(_buffer, "%s after Christmas Octave", DAYS_OF_WEEK[dayofweek(date)]); // %s1 = day of week
		break;

	case SEASON_LENT:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		sprintf(_buffer, "%s, %s week of Lent", DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		break;

	case SEASON_EASTER:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		sprintf(_buffer, "%s, %s week of Easter", DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		break;

	case SEASON_ORDINARY:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		sprintf(_buffer, "%s, %s week in Ordinary Time", DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		break;
	}

	return _buffer;
}

char* RomanCalendar::liturgical_day(time64_t date) {
	int year = liturgical_year(date);
	bool bIsSolemnity = false;
	Solemnities s;

	//if (issameday(date, first_advent_sunday(year)) {s = ;
	if (issameday(date, nativity(year))) { s = SOLEMNITIES_NATIVITY; bIsSolemnity = true; }
	if (issameday(date, holy_family(year))) { s = SOLEMNITIES_HOLY_FAMILY; bIsSolemnity = true; }
	if (issameday(date, mother_of_god(year))) { s = SOLEMNITIES_MOTHER_OF_GOD;  bIsSolemnity = true; }
	if (issameday(date, epiphany(year))) { s = SOLEMNITIES_EPIPHANY;  bIsSolemnity = true; }
	if (issameday(date, baptism_of_lord(year))) { s = SOLEMNITIES_BAPTISM_OF_LORD;  bIsSolemnity = true; }
	if (issameday(date, ash_wednesday(year))) { s = SOLEMNITIES_ASH_WEDNESDAY;  bIsSolemnity = true; }
	if (issameday(date, palm_sunday(year))) { s = SOLEMNITIES_PALM_SUNDAY;  bIsSolemnity = true; }
	if (issameday(date, good_friday(year))) { s = SOLEMNITIES_GOOD_FRIDAY;  bIsSolemnity = true; }
	if (issameday(date, holy_saturday(year))) { s = SOLEMNITIES_HOLY_SATURDAY;  bIsSolemnity = true; }
	if (issameday(date, easter_sunday(year))) { s = SOLEMNITIES_EASTER_SUNDAY;  bIsSolemnity = true; }
	if (issameday(date, ascension(year))) { s = SOLEMNITIES_ASCENSION;  bIsSolemnity = true; }
	if (issameday(date, pentecost(year))) { s = SOLEMNITIES_PENTECOST;  bIsSolemnity = true; }
	if (issameday(date, holy_trinity(year))) { s = SOLEMNITIES_HOLY_TRINITY;  bIsSolemnity = true; }
	if (issameday(date, corpus_christi(year))) { s = SOLEMNITIES_CORPUS_CHRISTI;  bIsSolemnity = true; }
	if (issameday(date, sacred_heart(year))) { s = SOLEMNITIES_SACRED_HEART;  bIsSolemnity = true; }
	if (issameday(date, immaculate_heart(year))) { s = SOLEMNITIES_IMMACULATE_HEART;  bIsSolemnity = true; }
	if (issameday(date, christ_king(year))) { s = SOLEMNITIES_CHRIST_KING;  bIsSolemnity = true; }

	_season = SEASONS[season(date)];

	if (bIsSolemnity) {
		sprintf(_buffer, SOLEMNITIES[s]);
		sprintf(_rank, RANK_NAME[SOLEMNITIES_RANKS[s]]);
		_colour = COLOURS[SOLEMNITIES_COLOURS[s]];
		return _buffer;
	}

	if (!sunday_temporale(date)) {
		ferial_temporale(date);
	}

	_colour = COLOURS[COLOURS_GREEN];

	return _buffer;
}


bool RomanCalendar::includes(const char* s, const char* const strarray[]) {
	int i = 0;
	while (strarray[i] != NULL) {
		if (strcmp(strarray[i], s) == 0) {
			return true;
			break;
		}
		i++;
	}
	return false;
}

void RomanCalendar::temporaletests() {
#ifdef _WIN32
	printf("\n\ntemporaletests()\n");
	//printf("epiphany is in SUNDAY_TRANSFERABLE_SOLEMNITIES[]: %s\n", includes("epiphany", SUNDAY_TRANSFERABLE_SOLEMNITIES) ? "true" : "false");
	//printf("christmas is in SUNDAY_TRANSFERABLE_SOLEMNITIES[]: %s\n", includes("christmas", SUNDAY_TRANSFERABLE_SOLEMNITIES) ? "true" : "false");

	//easter_tests();
	//return;
	//epiphany_tests();
	
	time64_t t;
	struct tm* ts;

	char datetime[128];

	int y = 2018;

	FILE* fpo = fopen(".\\liturgical-calendar.csv", "w");

	if (!fpo) {
		printf("unable to open file for output\n");
		return;
	}

	for (int m = 1; m <= 12; m++) {
		int days = get_monthdays(m, y);

		for (int d = 1; d <= days; d++) {
			t = date(d, m, y);
			if (t != (time64_t)-1) {
				ts = gmtime(&t);
				strftime(datetime, 128, "%d/%m/%Y", ts);
				liturgical_day(t);
				fprintf(fpo, "\"%s\"\t\"%s\"\t\"%s\"\n", datetime, _season, _buffer);
				printf("%s\t\"%s\"\t\"%s\"\t\"%s\"\n", datetime, _season, _colour, _buffer);
			}
		}
	}
	
	fclose(fpo);
	printf("wrote csv\n");
#else
#endif
}

int RomanCalendar::get_monthdays(int mon, int year) {
	static const int days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int leap = yisleap(year) ? 1 : 0;

	return days[mon] + leap;
}

bool RomanCalendar::yisleap(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void RomanCalendar::easter_tests() {
	time64_t t;

	for (int y = 1970; y < 2038; y++) {
		t = date(1, 8, y);
		print_date(t);
		printf(": Easter sunday is on ");
		print_date(easter_sunday(y));
		printf(" is sunday: %s", sunday(easter_sunday(y)) ? "true" : "false");
		printf("\n");
	}
}

void RomanCalendar::epiphany_tests(void) {
	time64_t t;
	for (int y = 1970; y < 2038; y++) {
		t = epiphany(y);
		print_date(t);
		printf("\n");
	}
}

char* RomanCalendar::ordinalize(int number) {
	int modulo = number % 10;
	if ((number / 10) == 1) modulo = 9;

	char* str;

	switch (modulo) {
	case 1:
		str = "%dst";
		break;

	case 2:
		str = "%dnd";
		break;

	case 3:
		str = "%drd";
		break;

	default:
		str = "%dth";
	}

	sprintf(_ordinal, str, number);

	return _ordinal;
}

void RomanCalendar::print_date(time64_t t) {
#ifdef _WIN32
	struct tm* ts;

	char buffer[128];

	ts = gmtime(&t);
	strftime(buffer, 128, "%Y-%m-%d", ts);
	printf("%s ", buffer);
#else
	::tmElements_t ts;						// for arduino
	::breakTime(t, ts);

	I2CSerial.print(ts.Day);
	I2CSerial.print(F("-"));
	I2CSerial.print(ts.Month);
	I2CSerial.print(F("-"));
	I2CSerial.print(ts.Year);
#endif
}