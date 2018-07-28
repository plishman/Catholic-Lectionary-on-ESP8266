#include "stdafx.h" // comment out for cross-platform/embedded. It doesn't play well with Windows Visual Studio if you wrap it in a #ifdef _WIN32!
#include "Temporale.h"

/*
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
*/

const Enums::Ranks Temporale::SOLEMNITIES_RANKS[18] = {		// the ranks of the solemnities, in the same order as in solemnities[17]
	Enums::RANKS_PRIMARY,										//nativity
	Enums::RANKS_FEAST_LORD_GENERAL,							//holy_family
	Enums::RANKS_SOLEMNITY_GENERAL,							//mother_of_god
	Enums::RANKS_PRIMARY,										//epiphany
	Enums::RANKS_FEAST_LORD_GENERAL,							//baptism_of_lord
	Enums::RANKS_PRIMARY,										//ash_wednesday
	Enums::RANKS_PRIMARY,										//palm_sunday
	Enums::RANKS_TRIDUUM,										//good_friday
	Enums::RANKS_TRIDUUM,										//holy_saturday
	Enums::RANKS_TRIDUUM,										//easter_sunday
	Enums::RANKS_PRIMARY,										//ascension
	Enums::RANKS_PRIMARY,										//pentecost
	Enums::RANKS_FEAST_PROPER,									//Christ the Priest (optional in some areas)
	Enums::RANKS_SOLEMNITY_GENERAL,							//holy_trinity
	Enums::RANKS_SOLEMNITY_GENERAL,							//corpus_christi
	Enums::RANKS_SOLEMNITY_GENERAL,							//sacred_heart
	Enums::RANKS_MEMORIAL_GENERAL,								//immaculate_heart
	Enums::RANKS_SOLEMNITY_GENERAL								//christ_king
};

const Enums::Colours Temporale::SOLEMNITIES_COLOURS[18] = {	// the colours of the solemnities, in the same order as in solemnities[17]
	Enums::COLOURS_WHITE,										//nativity
	Enums::COLOURS_WHITE,										//holy_family
	Enums::COLOURS_WHITE,										//mother_of_god
	Enums::COLOURS_WHITE,										//epiphany
	Enums::COLOURS_WHITE,										//baptism_of_lord
	Enums::COLOURS_VIOLET,										//ash_wednesday
	Enums::COLOURS_RED,											//palm_sunday
	Enums::COLOURS_RED,											//good_friday
	Enums::COLOURS_WHITE,										//holy_saturday
	Enums::COLOURS_WHITE,										//easter_sunday
	Enums::COLOURS_WHITE,										//ascension
	Enums::COLOURS_RED,											//pentecost
	Enums::COLOURS_WHITE,										//Christ the Priest (optional in some areas)
	Enums::COLOURS_WHITE,										//holy_trinity
	Enums::COLOURS_WHITE,										//corpus_christi
	Enums::COLOURS_WHITE,										//sacred_heart
	Enums::COLOURS_WHITE,										//immaculate_heart
	Enums::COLOURS_WHITE										//christ_king
};

Temporale::Temporale(bool transfer_to_sunday, I18n* i ) {
	_transfer_to_sunday = transfer_to_sunday;
	_I18n = i;
	_ordinalizer = new Ordinalizer(_I18n->get("ordinals"));
	return;
}

Temporale::~Temporale(void) {
	return;
}

String Temporale::getLocaleWeekday(int dayofweek) {
	if (dayofweek < 0 || dayofweek > 6) return String("");
	return _I18n->get("weekday." + String(dayofweek));
}

void Temporale::datestests() {
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

bool Temporale::getTm(int day, int month, int year, int hours, int minutes, int seconds, struct tm* ts) {
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

time64_t Temporale::date(int day, int month, int year) {
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

time64_t Temporale::weekday_before(int weekdayBefore, time64_t date) {
	if (weekdayBefore < 0 || weekdayBefore > 6) return (time64_t)-1;

#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	int currentWeekday = ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	int currentWeekday = ts.Wday - 1;
#endif
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

time64_t Temporale::sunday_before(time64_t date) { return weekday_before(0, date); }
time64_t Temporale::monday_before(time64_t date) { return weekday_before(1, date); }
time64_t Temporale::tuesday_before(time64_t date) { return weekday_before(2, date); }
time64_t Temporale::wednesday_before(time64_t date) { return weekday_before(3, date); }
time64_t Temporale::thursday_before(time64_t date) { return weekday_before(4, date); }
time64_t Temporale::friday_before(time64_t date) { return weekday_before(5, date); }
time64_t Temporale::saturday_before(time64_t date) { return weekday_before(6, date); }

time64_t Temporale::weekday_after(int weekdayAfter, time64_t date) {
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

time64_t Temporale::sunday_after(time64_t date) { return weekday_after(0, date); }
time64_t Temporale::monday_after(time64_t date) { return weekday_after(1, date); }
time64_t Temporale::tuesday_after(time64_t date) { return weekday_after(2, date); }
time64_t Temporale::wednesday_after(time64_t date) { return weekday_after(3, date); }
time64_t Temporale::thursday_after(time64_t date) { return weekday_after(4, date); }
time64_t Temporale::friday_after(time64_t date) { return weekday_after(5, date); }
time64_t Temporale::saturday_after(time64_t date) { return weekday_after(6, date); }

int Temporale::dayofweek(time64_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	return ts.Wday - 1;		// weekday sunday=0, mon=1, sat=6
#endif
}

bool Temporale::sunday(time64_t date) { return (dayofweek(date) == 0); }
bool Temporale::monday(time64_t date) { return (dayofweek(date) == 1); }
bool Temporale::tuesday(time64_t date) { return (dayofweek(date) == 2); }
bool Temporale::wednesday(time64_t date) { return (dayofweek(date) == 3); }
bool Temporale::thursday(time64_t date) { return (dayofweek(date) == 4); }
bool Temporale::friday(time64_t date) { return (dayofweek(date) == 5); }
bool Temporale::saturday(time64_t date) { return (dayofweek(date) == 6); }

int Temporale::year(time64_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_year + BEGIN_EPOCH;
#else
	return ::year(date); // for arduino
#endif
}

int Temporale::month(time64_t date)
{
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_mon + 1;
#else
	return ::month(date); // for arduino // check this!! not tested
#endif
}

int Temporale::dayofmonth(time64_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_mday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	return ts.Day;
#endif
}

bool Temporale::issameday(time64_t date1, time64_t date2) {
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

int Temporale::date_difference(time64_t date1, time64_t date2) { // assumes that time64_t values are seconds since 1970 or 1900, and can have arithmetic
																 //if (date1 >= date2) {								 // performed on them. Not guaranteed on all systems, but should work on arduino embedded.
																 //	return (int)(date1 - date2);
																 //}
																 //else {
	return (int)(date2 - date1);
	//}
}

int Temporale::hour_of_day(time64_t time) {
#ifdef _WIN32
	struct tm* ts = gmtime(&time);
	return ts->tm_hour;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(time, ts);
	return ts.Hour;
#endif
}

time64_t Temporale::start_date(int year) {
	return first_advent_sunday(year);
}

time64_t Temporale::end_date(int year) {
	return first_advent_sunday(year + 1) - DAY;
}

time64_t Temporale::first_advent_sunday(int year) {
	return sunday_before(nativity(year)) - (3 * WEEK * DAY);
}

time64_t Temporale::nativity(int year) {
	return date(25, 12, year);
}

time64_t Temporale::holy_family(int year) {
	time64_t xmas = nativity(year);

	if (sunday(xmas)) {
		return date(30, 12, year);
	}
	else {
		return sunday_after(xmas);
	}
}

time64_t Temporale::mother_of_god(int year) {
	return octave_of(nativity(year));
}

time64_t Temporale::epiphany(int year) {
	if (_transfer_to_sunday) {
		return sunday_after(date(1, 1, year + 1));
	}

	return date(6, 1, year + 1);
}

time64_t Temporale::baptism_of_lord(int year) { // maybe watch this, since its had the most modding from the ruby version
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

time64_t Temporale::ash_wednesday(int year) {
	return easter_sunday(year) - (((6 * WEEK) + 4) * DAY);
}

time64_t Temporale::easter_sunday(int year) {
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

time64_t Temporale::palm_sunday(int year) {
	return easter_sunday(year) - (7 * DAY);
}

time64_t Temporale::good_friday(int year) {
	return easter_sunday(year) - (2 * DAY);
}

time64_t Temporale::holy_saturday(int year) {
	return easter_sunday(year) - (1 * DAY);
}

time64_t Temporale::ascension(int year) {
	if (_transfer_to_sunday) {
		return easter_sunday(year) + ((6 * WEEK) * DAY); // return 42 days after Easter sunday
	}

	return pentecost(year) - (10 * DAY); // return ascension Thursday (40 days after Easter sunday)
}

time64_t Temporale::pentecost(int year) {
	return easter_sunday(year) + ((7 * WEEK) * DAY);
}

time64_t Temporale::christ_eternal_priest(int year) {
	return thursday_after(pentecost(year)); // Feast of Christ the Eternal Priest (Optional in some areas)
}

time64_t Temporale::holy_trinity(int year) {
	return octave_of(pentecost(year));
}

time64_t Temporale::corpus_christi(int year) {
	if (_transfer_to_sunday) {
		return holy_trinity(year) + (WEEK * DAY); // should be 60 days after Easter Sunday if on a Thursday, or 63 days after if transferred to Sunday
	}
	return holy_trinity(year) + (4 * DAY);
}

time64_t Temporale::sacred_heart(int year) {
	//return corpus_christi(year) + (8 * DAY);
	return pentecost(year) + (19 * DAY);
}

time64_t Temporale::immaculate_heart(int year) {
	return pentecost(year) + (20 * DAY);
}

time64_t Temporale::christ_king(int year) {
	return first_advent_sunday(year) - (7 * DAY);
}

time64_t Temporale::octave_of(time64_t date) {
	return date + (WEEK * DAY);
}

int Temporale::liturgical_year(time64_t date) {
	int _year = year(date);
	if (date < first_advent_sunday(_year)) {
		return _year - 1;
	}

	return _year;
}
/*
Temporale::Liturgical_Year Temporale::liturgical_year_letter(time64_t date) {
	int year = liturgical_year(date) + 1;
	
	int r = year % 3; // 0 = C, 1 = A, 2 = B
	switch (r) {
	case 0:
		return Temporale::LITURGICAL_YEAR_C;
		break;
	
	case 1:
		return Temporale::LITURGICAL_YEAR_A;
		break;
	
	case 2:
		return Temporale::LITURGICAL_YEAR_B;
		break;
	}
}

Temporale::Liturgical_Cycle Temporale::liturgical_cycle(time64_t date) {
	int year = liturgical_year(date) + 1;

	if ((year % 2) == 1) {
		return Temporale::LITURGICAL_CYCLE_I;	// odd years
	}
	else {
		return Temporale::LITURGICAL_CYCLE_II;	// even years
	}
}
*/

int Temporale::for_day(time64_t date) {
	return liturgical_year(date);
}

//# which liturgical season is it ? (returns a 'Season' enum)
Enums::Season Temporale::season(time64_t date) {
	int year = Temporale::year(date);

	//printf("season: year is %d\n", year);

	if ((first_advent_sunday(year) <= date) && (nativity(year) > date)) {
		return Enums::SEASON_ADVENT;
	}

	if ((nativity(liturgical_year(date)) <= date) && (baptism_of_lord(liturgical_year(date))) >= date) {
		/*printf("--dates of nativity for date, and baptism of lord\n");
		print_date(date);
		print_date(nativity(liturgical_year(date)));
		print_date(baptism_of_lord(liturgical_year(date)));
		printf("--\n");
		*/
		return Enums::SEASON_CHRISTMAS;
	}

	if ((ash_wednesday(liturgical_year(date)) <= date) && (easter_sunday(liturgical_year(date)) > date)) {
		return Enums::SEASON_LENT;
	}

	if ((easter_sunday(liturgical_year(date)) <= date) && (pentecost(liturgical_year(date)) >= date)) {
		return Enums::SEASON_EASTER;
	}

	return Enums::SEASON_ORDINARY;
}

time64_t Temporale::season_beginning(Enums::Season s, time64_t date) {
	int year = liturgical_year(date);

	if (s == Enums::SEASON_ADVENT) return first_advent_sunday(year);
	if (s == Enums::SEASON_CHRISTMAS) return nativity(year);
	if (s == Enums::SEASON_LENT) return ash_wednesday(year);
	if (s == Enums::SEASON_EASTER) return easter_sunday(year);
	if (s == Enums::SEASON_ORDINARY) return baptism_of_lord(year) + DAY;

	return (time64_t)-1;
}

void Temporale::setColour(Enums::Colours c) {
	_colour_e = c;
	_colour = _I18n->get(_I18n->I18n_COLOURS[_colour_e]);
}

Enums::Colours Temporale::getColour(void) {
	return _colour_e;
}

void Temporale::setRank(Enums::Ranks r) {
	_rank_e = r;
	_rank = _I18n->get(_I18n->I18n_RANK_NAMES[_rank_e]);
}

Enums::Ranks Temporale::getRank(void) {
	return _rank_e;
}

int Temporale::season_week(Enums::Season seasonn, time64_t date) {
	date = Temporale::date(dayofmonth(date), month(date), year(date)); // midnight, morning of (date)
	
	int year = Temporale::year(date);

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

	if (seasonn == Enums::SEASON_LENT && week == 0) {
		week = sunday(date) ? 1 : 0;
		//printf("*");
	}

	if (seasonn == Enums::SEASON_ORDINARY) {
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
			int weeks_after_date = (date_difference(date, first_advent_sunday(Temporale::year(date)))) / (WEEK * DAY);
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
}

const Enums::Season SEASONS_SUNDAY_PRIMARY[3] = { Enums::SEASON_ADVENT, Enums::SEASON_LENT, Enums::SEASON_EASTER };
String Temporale::sunday_temporale(time64_t date) {
	if (!(sunday(date))) return String("");

	Enums::Season seas = season(date);
	Enums::Ranks rank = Enums::RANKS_SUNDAY_UNPRIVILEGED;

	if (seas == Enums::SEASON_ADVENT || seas == Enums::SEASON_LENT || seas == Enums::SEASON_EASTER) {
		rank = Enums::RANKS_PRIMARY;
	}

	int week = season_week(seas, date);
	_ordinalizer->ordinalize(week);

	_day = _I18n->get("temporale." + String(_I18n->I18n_SEASONS[seas]) + ".sunday");
	_day.replace("%{week}", _ordinalizer->ordinalize(week));
	_rank_e = rank;
	
	int lit_year = liturgical_year(date) % 3; // 0 == A, 1 == B, 2 == C
	//printf("Lit_year=%d", lit_year);

	switch (seas)
	{
	case Enums::SEASON_ADVENT:
		_Lectionary = (3 * (week - 1)) + lit_year + 1; 
		christmas_vigil(date); // check to see if after 6pm on Christmas eve
		break;

	case Enums::SEASON_LENT:
		_Lectionary = (3 * (week - 1)) + lit_year + 22;
		break;

	case Enums::SEASON_ORDINARY:
		_Lectionary = (3 * (week - 2)) + lit_year + 64;
		break;

	case Enums::SEASON_EASTER:
		if (week > 1 && week < 7) { // weeks 2-6
			_Lectionary = (3 * (week - 2)) + lit_year + 43; // was week - 1
		}
		if (week == 7) {
			_Lectionary = 59 + lit_year;
		}
		break;

	case Enums::SEASON_CHRISTMAS:
		christmas_lectionary(date);
		break;

	}

	return _day;
}

String Temporale::ferial_temporale(time64_t date) {
	String weekday = getLocaleWeekday(dayofweek(date));

	Enums::Season seas = season(date);
	int week = season_week(seas, date);
	Enums::Ranks rank = Enums::RANKS_FERIAL;

	bool bIsSet = false;

	switch (seas)
	{
	case Enums::SEASON_ADVENT:
		if (date >= Temporale::date(17, 12, year(date))) {
			rank = Enums::RANKS_FERIAL_PRIVILEGED;

			_day = _I18n->get("temporale.advent.before_christmas");
			_day.replace("%{day}", _ordinalizer->ordinalize(dayofmonth(date)));
			bIsSet = true;

			_Lectionary = dayofmonth(date) + 176;
		}
		else {
			_Lectionary = (6 * (week - 1)) + (dayofweek(date) - 1) + 175;
		}

		christmas_vigil(date); // check to see if after 6pm on Christmas eve
		break;

	case Enums::SEASON_CHRISTMAS:
		if (date < mother_of_god(liturgical_year(date))) {
			rank = Enums::RANKS_FERIAL_PRIVILEGED;

			//print_date(date);
			//print_date(mother_of_god(liturgical_year(date)));
			//printf("dayofmonth(date) = %d, dayofmonth(nativity() = %d\n", dayofmonth(date), dayofmonth(nativity(year(date))));
			//printf("mother_of_god(%d)=", RomanCalendar::year(date));
			//print_date(mother_of_god(RomanCalendar::year(date)));
			//printf("*\n");

			_day = _I18n->get("temporale.christmas.nativity_octave.ferial");
			_day.replace("%{day}", _ordinalizer->ordinalize(dayofmonth(date) - dayofmonth(nativity(Temporale::year(date))) + 1));
			bIsSet = true;
		}
		else if (date > epiphany(Temporale::year(date))) {
			_day = _I18n->get("temporale.christmas.after_epiphany.ferial");
			_day.replace("%{weekday}", weekday);
			bIsSet = true;
		}
		christmas_lectionary(date);
		break;

	case Enums::SEASON_LENT:
/*		printf("============= date = ");
		print_date(date);
		printf("\t palm sunday = ");
		print_date(palm_sunday(year(date)));
		printf("year(date) = %d\n", year(date));
		printf("-------------\n");
*/
		_Lectionary = (6 * (week - 1)) + (dayofweek(date) - 1) + 224;
		if (week == 3) _Lectionary += 1;
		if (week == 4) _Lectionary += 2;
		if (week == 5) _Lectionary += 3;

		if (week == 0) _Lectionary--;

		if (week == 0) {
			_day = _I18n->get("temporale.lent.after_ashes.ferial");
			_day.replace("%{weekday}", weekday);
			bIsSet = true;
		}
		else if (date > palm_sunday(liturgical_year(date))) {
			rank = Enums::RANKS_PRIMARY;
			if (!thursday(date)) {
				_day = _I18n->get("temporale.lent.holy_week.ferial");
				_day.replace("%{weekday}", weekday);
			}
			else {
				_day = _I18n->get("temporale.lent.holy_week.thursday");
				_colour_e = Enums::COLOURS_WHITE;
			}
			bIsSet = true;

			_Lectionary = (dayofweek(date) - 1) + 257; // lectionary for Holy Week

		}
		rank = (rank > Enums::RANKS_FERIAL_PRIVILEGED) ? rank : Enums::RANKS_FERIAL_PRIVILEGED; // watch this - comparison is dependent on the in which each enum member represents, which have been chosen to make this work

		break;

	case Enums::SEASON_EASTER:
		if (week == 1) {
			rank = Enums::RANKS_PRIMARY;
			_day = _I18n->get("temporale.easter.octave.ferial");
			_day.replace("%{weekday}", weekday);
			bIsSet = true;
			_Lectionary = 261 + (dayofweek(date) - 1);
		}
		else {
			_Lectionary = (6 * (week - 2)) + (dayofweek(date) - 1) + 267;
		}
		break;

	case Enums::SEASON_ORDINARY:
		_Lectionary = (6 * (week - 1)) + (dayofweek(date) - 1) + 305;
		break;

	}

	_rank_e = rank;

	if (bIsSet) return _day;

	_day = _I18n->get("temporale." + String(_I18n->I18n_SEASONS[seas]) + ".ferial");
	_day.replace("%{weekday}", weekday);
	_day.replace("%{week}", _ordinalizer->ordinalize(week));
	
	return _day;
}

void Temporale::christmas_vigil(time64_t date) {
	if (month(date) == 12 && dayofmonth(date) == 24) { // Christmas eve
		if (hour_of_day(date) >= 18) { // after 6pm
			_Lectionary = 13; // Christmas eve, vigil mass
			_day = _I18n->get("temporale.advent.vigil");
		}
	}
}

void Temporale::christmas_lectionary(time64_t date) {
	Enums::Season seas = season(date);
	//int week = season_week(seas, date);

	if (seas != Enums::SEASON_CHRISTMAS) return;

	int day_of_month = dayofmonth(date);
	int mon = month(date);
	int yr = year(date);
	int lit_yr = liturgical_year(date);

	if (issameday(nativity(yr), date)) {
		//Christmas day
		int hour = hour_of_day(date);

		if (hour >= 0 && hour < 4) { // midnight mass
			_Lectionary = 14;
		}

		if (hour >= 4 && hour < 8) { // dawn mass
			_Lectionary = 15;
		}

		if (hour >= 8) { // mass during the day 
			_Lectionary = 16;
		}

		return;
	}


	if (mon == 12 && day_of_month >= 26 && day_of_month <= 28) { // 29th Dec to 31st Dec
		switch (day_of_month) {
		case 26:
			_Lectionary = 696; // Feast of St. Stephen - should be handled in Sanctorale *now is*
			break;

		case 27:
			_Lectionary = 697; // Feast of St. John - should be handled in Sanctorale *now is*
			break;

		case 28:
			_Lectionary = 698; // Feast of Holy Innocents - should be handled in Sanctorale *now is*
			break;
		}
		
		return;
	}

	if (mon == 12 && day_of_month >= 29 && day_of_month <= 31) { // 29th Dec to 31st Dec
		_Lectionary = 202 + day_of_month - 29;
		return;
	}

	if (mon == 1 && day_of_month >= 2 && day_of_month <= 5) { // 2nd Jan - 5th Jan
		_Lectionary = 205 + day_of_month - 2;
		return;
	}

	if (mon == 1 && date <= baptism_of_lord(yr)) {
		int epiphany_month_day = dayofmonth(epiphany(lit_yr));

		if (epiphany_month_day == 6) {
			if (day_of_month >= 7 && day_of_month <= 12) {
				_Lectionary = 212 + day_of_month - 7;
				return;
			}
		}
		else {
			time64_t monday_after_epiphany = monday_after(epiphany(lit_yr));
			int epiphany_week_start_month_day = dayofmonth(monday_after_epiphany);

			_Lectionary = 212 + day_of_month - epiphany_week_start_month_day;
			return;
		}

		if (epiphany_month_day == 7 || epiphany_month_day == 8) {
			if (day_of_month == 6) _Lectionary = 209; return;
			if (day_of_month == 7) _Lectionary = 210; return;
		}
	}

	//printf("christmas_lectionary(): Didn't set lectionary number");
}

bool Temporale::do_solemnities(time64_t date) {
	int lit_year = liturgical_year(date);
	//int cal_year = Temporale::year(date);
	bool bIsSolemnity = false;
	Enums::Solemnities s;

	bool bIsHDO = false; // is holy day of obligation
	
	//printf("lit_year=%d\t", lit_year);

	if (issameday(date, nativity(lit_year)))        { s = Enums::SOLEMNITIES_NATIVITY; bIsSolemnity = true; bIsHDO = true; christmas_lectionary(date);}
	if (issameday(date, holy_family(lit_year)))     { s = Enums::SOLEMNITIES_HOLY_FAMILY; bIsSolemnity = true; _Lectionary = 17; }
	if (issameday(date, mother_of_god(lit_year)))   { s = Enums::SOLEMNITIES_MOTHER_OF_GOD;  bIsSolemnity = true; bIsHDO = true; _Lectionary = 18; }
	if (issameday(date, epiphany(lit_year)))        { s = Enums::SOLEMNITIES_EPIPHANY;  bIsSolemnity = true; bIsHDO = true; _Lectionary = 20; }
	if (issameday(date, baptism_of_lord(lit_year))) { s = Enums::SOLEMNITIES_BAPTISM_OF_LORD;  bIsSolemnity = true; _Lectionary = 21; }
	if (issameday(date, ash_wednesday(lit_year)))   { s = Enums::SOLEMNITIES_ASH_WEDNESDAY;  bIsSolemnity = true; _Lectionary = 219; }
	if (issameday(date, palm_sunday(lit_year)))     { s = Enums::SOLEMNITIES_PALM_SUNDAY;  bIsSolemnity = true; _Lectionary = 38; }
	if (issameday(date, good_friday(lit_year)))     { s = Enums::SOLEMNITIES_GOOD_FRIDAY;  bIsSolemnity = true; _Lectionary = 40; }
	if (issameday(date, holy_saturday(lit_year)))   { s = Enums::SOLEMNITIES_HOLY_SATURDAY;  bIsSolemnity = true; _Lectionary = 41; }
	if (issameday(date, easter_sunday(lit_year)))   { s = Enums::SOLEMNITIES_EASTER_SUNDAY;  bIsSolemnity = true; _Lectionary = 42; }
	if (issameday(date, ascension(lit_year)))       { s = Enums::SOLEMNITIES_ASCENSION;  bIsSolemnity = true; bIsHDO = true; _Lectionary = 58; }
	if (issameday(date, pentecost(lit_year)))       { s = Enums::SOLEMNITIES_PENTECOST;  bIsSolemnity = true; _Lectionary = 62; }

	if (_I18n->configparams.celebrate_feast_of_christ_priest) {
		if (issameday(date, christ_eternal_priest(lit_year))) { s = Enums::SOLEMNITIES_CHRIST_PRIEST;  bIsSolemnity = true; _Lectionary = 982; }		
	}

	if (issameday(date, holy_trinity(lit_year)))     { s = Enums::SOLEMNITIES_HOLY_TRINITY;  bIsSolemnity = true; _Lectionary = (lit_year % 3) + 164; } //check lectionary
	if (issameday(date, corpus_christi(lit_year)))   { s = Enums::SOLEMNITIES_CORPUS_CHRISTI;  bIsSolemnity = true; bIsHDO = true; _Lectionary = (lit_year % 3) + 167; }
	if (issameday(date, sacred_heart(lit_year)))     { s = Enums::SOLEMNITIES_SACRED_HEART;  bIsSolemnity = true; _Lectionary = (lit_year % 3) + 170; }
	if (issameday(date, immaculate_heart(lit_year))) { s = Enums::SOLEMNITIES_IMMACULATE_HEART;  bIsSolemnity = true; _Lectionary = 622; }
	if (issameday(date, christ_king(lit_year + 1/*is relative to next liturgical year's first advent sunday*/))) { s = Enums::SOLEMNITIES_CHRIST_KING;  bIsSolemnity = true; _Lectionary = (lit_year % 3) + 160; }

	if (bIsSolemnity) {
		_rank_e = SOLEMNITIES_RANKS[s];
		_colour_e = SOLEMNITIES_COLOURS[s];
		_day = _I18n->get(_I18n->I18n_SOLEMNITIES[s]);
		//printf("*********************\n");
	}

	if (bIsHDO) _hdo = true;
	
	return bIsSolemnity;
}

bool Temporale::do_sundays(time64_t date) {
	if (sunday(date)) {
		_hdo = true;
		sunday_temporale(date);
		return true;
	}
	return false;
}

bool Temporale::do_ferials(time64_t date) {
	if (!sunday(date)) {
		ferial_temporale(date);
		return true;
	}

	return false;

}

bool Temporale::get(time64_t date) {
	_Lectionary = 0;
	_day = "";
	_rank = "";
	_season = "";
	//_sanctorale = "";
	_colour = "";
	//_liturgical_year = "";
	//_liturgical_cycle = "";
	_hdo = false;
	_holy_day_of_obligation = "";

	Enums::Season seas = season(date);
	_season = _I18n->get("temporale.season." + String(_I18n->I18n_SEASONS[seas]));

	switch (seas) {
	case Enums::SEASON_ADVENT:
		_colour_e = Enums::COLOURS_VIOLET;
		
		//TODO
		//if (((season_week(Enums::SEASON_ADVENT, date)) == 3) && sunday(date)) { // Gaudete Sunday, 3rd Sunday in Advent - Rose vestments
		//	_colour_e = Enums::COLOURS_ROSE;			
		//}
		break;		
	
	case Enums::SEASON_LENT:
		_colour_e = Enums::COLOURS_VIOLET;

		//TODO
		//if (((season_week(Enums::SEASON_LENT, date)) == 4) && sunday(date)) { // Laetare Sunday Sunday, 4th Sunday in Lent - Rose vestments
		//	_colour_e = Enums::COLOURS_ROSE;			
		//}
		break;

	case Enums::SEASON_CHRISTMAS:
	case Enums::SEASON_EASTER:
		_colour_e = Enums::COLOURS_WHITE;
		break;

	case Enums::SEASON_ORDINARY:
		_colour_e = Enums::COLOURS_GREEN;
		break;

	default:
		I2CSerial.printf("Colours:Season not set!\n");
		_colour_e = Enums::COLOURS_GREEN;
	}

	do_ferials(date);
	do_sundays(date);
	_bIsSolemnity = do_solemnities(date);

	_colour = _I18n->get(_I18n->I18n_COLOURS[_colour_e]);
	_rank = _I18n->get(_I18n->I18n_RANK_NAMES[_rank_e]);

	if (_hdo) {
		_holy_day_of_obligation = _I18n->get(I18n_HOLY_DAY_OF_OBLIGATION);
	}
	
	return (_day != "");
}

bool Temporale::includes(const char* s, const char* const strarray[]) {
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

void Temporale::temporaletests() {
//#ifdef _WIN32
	//printf("\n\ntemporaletests()\n");
	//printf("epiphany is in SUNDAY_TRANSFERABLE_SOLEMNITIES[]: %s\n", includes("epiphany", SUNDAY_TRANSFERABLE_SOLEMNITIES) ? "true" : "false");
	//printf("christmas is in SUNDAY_TRANSFERABLE_SOLEMNITIES[]: %s\n", includes("christmas", SUNDAY_TRANSFERABLE_SOLEMNITIES) ? "true" : "false");

	//easter_tests();
	//return;
	//epiphany_tests();
/*	
	time64_t t;
	struct tm* ts;

	char datetime[128];

	int y = 2017;

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
				get(t);
				fprintf(fpo, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n", datetime, _season.c_str(), _day.c_str(), _colour.c_str(), _rank.c_str(), _celebration.c_str());
				//printf("%s\t%s\t%s\t%s\t%s\n", datetime, _liturgical_year.c_str(), _liturgical_cycle.c_str(), _season.c_str(), _day.c_str());
				if (_celebration != "") printf("%s\n", _celebration.c_str());
				printf("Colour: %s\t Rank: %s\t\n\n", _colour.c_str(), _rank.c_str());
			}
		}
	}
	
	fclose(fpo);
	printf("wrote csv\n");
*/
//#else
//#endif
}

int Temporale::get_monthdays(int mon, int year) {
	static const int days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int leap = yisleap(year) ? 1 : 0;

	return days[mon] + leap;
}

bool Temporale::yisleap(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void Temporale::easter_tests() {
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

void Temporale::epiphany_tests(void) {
	time64_t t;
	for (int y = 1970; y < 2038; y++) {
		t = epiphany(y);
		print_date(t);
		printf("\n");
	}
}

String Temporale::ordinalize(int number) {
	int modulo = number % 10;
	if ((number / 10) == 1) modulo = 9;

	String ord;

	switch (modulo) {
	case 1:
		ord = "st";
		break;

	case 2:
		ord = "nd";
		break;

	case 3:
		ord = "rd";
		break;

	default:
		ord = "th";
	}

	ord = String(number) + ord;
	return ord;
}

void Temporale::print_date(time64_t t) {
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
	I2CSerial.print(ts.Year + BEGIN_EPOCH);
#endif
}

void Temporale::print_time(time64_t t) {
#ifdef _WIN32
	struct tm* ts;

	char buffer[128];

	ts = gmtime(&t);
	strftime(buffer, 128, "%H:%M:%S", ts);
	printf("%s ", buffer);
#else
	::tmElements_t ts;						// for arduino
	::breakTime(t, ts);

	I2CSerial.print(ts.Hour);
	I2CSerial.print(F(":"));
	I2CSerial.print(ts.Minute);
	I2CSerial.print(F(":"));
	I2CSerial.print(ts.Second);
#endif
}