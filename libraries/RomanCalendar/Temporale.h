#pragma once
#ifndef RomanCalender_h
#define RomanCalendar_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "WString.h"
#include "Enums.h"
#include "I18n.h"

#ifndef _WIN32
	#include "Arduino.h"
	#include "../Time/TimeLib.h"
#else
	#include <time.h>
#endif

#ifdef _WIN32
	const int BEGIN_EPOCH = 1900; // 1900 for 64-bit time_t, sometimes 1970 (may be on embedded system)
#else
	const int BEGIN_EPOCH = 1970; // 1900 for 64-bit time_t, sometimes 1970 (may be on embedded system)
#endif

const int WEEK = 7;
const int DAY = 24 * 3600;

class Temporale {
public:
	I18n* _I18n;

	String _day;
	String _rank;
	String _season;
	String _colour;

	Enums::Ranks _rank_e;
	Enums::Colours _colour_e;

	int _Lectionary = 0;

	bool _bIsSolemnity = false;

	typedef struct {
		Enums::Solemnities s;
		Enums::Colours c;
		Enums::Ranks r;
	} Solemnity;

	char* DAYS_OF_WEEK[7];
	char* RANK_NAME[14];
	char* RANK_TYPES[9];
	char* COLOURS[4];
	char* SEASONS[5];
	char* SOLEMNITIES[17];
	char* SUNDAYS_AND_FERIALS[16];

	static const char* const RANK_PRIORITY[14];
	static const char* const SUNDAY_TRANSFERABLE_SOLEMNITIES[3];
	//static const Enums::Ranks SOLEMNITIES[17];
	static const Enums::Colours SOLEMNITIES_COLOURS[17];
	static const Enums::Ranks SOLEMNITIES_RANKS[17];

	bool _transfer_to_sunday; // flag determines whether epiphany, ascension and corpus Christi should be transferred to sunday (us, uk etc)

	Temporale(bool transfer_to_sunday, I18n* i);
	~Temporale(void);
	
	String getLocaleWeekday(int dayofweek);
	static const char* const SOLEMNITIES_I18n[17];

	void datestests();
	bool getTm(int day, int month, int year, int hours, int minutes, int seconds, struct tm* ts);
	time_t date(int day, int month, int year);

	time_t weekday_before(int weekdayBefore, time_t date);
	time_t sunday_before(time_t date);
	time_t monday_before(time_t date);
	time_t tuesday_before(time_t date);
	time_t wednesday_before(time_t date);
	time_t thursday_before(time_t date);
	time_t friday_before(time_t date);
	time_t saturday_before(time_t date);

	time_t weekday_after(int weekdayAfter, time_t date);
	time_t sunday_after(time_t date);
	time_t monday_after(time_t date);
	time_t tuesday_after(time_t date);
	time_t wednesday_after(time_t date);
	time_t thursday_after(time_t date);
	time_t friday_after(time_t date);
	time_t saturday_after(time_t date);

	int dayofweek(time_t date);
	bool sunday(time_t date);
	bool monday(time_t date);
	bool tuesday(time_t date);
	bool wednesday(time_t date);
	bool thursday(time_t date);
	bool friday(time_t date);
	bool saturday(time_t date);

	int date_difference(time_t date1, time_t date2);
	int hour_of_day(time_t time);
	int year(time_t date);
	static int month(time_t date);
	static int dayofmonth(time_t date);
	bool issameday(time_t date1, time_t date2);

	time_t start_date(int year);
	time_t end_date(int year);
	time_t first_advent_sunday(int year);
	time_t nativity(int year);
	time_t holy_family(int year);
	time_t mother_of_god(int year);
	time_t epiphany(int year);
	time_t baptism_of_lord(int year);
	time_t ash_wednesday(int year);
	time_t easter_sunday(int year);
	time_t palm_sunday(int year);
	time_t good_friday(int year);
	time_t holy_saturday(int year);
	time_t ascension(int year);
	time_t pentecost(int year);
	time_t holy_trinity(int year);
	time_t corpus_christi(int year);
	time_t sacred_heart(int year);
	time_t immaculate_heart(int year);
	time_t christ_king(int year);
	time_t octave_of(time_t date);

	int liturgical_year(time_t date);
	int for_day(time_t date);
	Enums::Season season(time_t date);
	time_t season_beginning(Enums::Season s, time_t date);
	int season_week(Enums::Season seasonn, time_t date);

	void setColour(Enums::Colours c);
	Enums::Colours getColour(void);

	void setRank(Enums::Ranks r);

	Enums::Ranks getRank(void);

	static const Enums::Season SEASONS_SUNDAY_PRIMARY[3];
	String sunday_temporale(time_t date);
	String ferial_temporale(time_t date);
	void christmas_vigil(time_t date);
	void christmas_lectionary(time_t date);
	bool get(time_t date);

	bool do_solemnities(time_t date);
	bool do_sundays(time_t date);
	bool do_ferials(time_t date);
	
	String ordinalize(int number);
	bool includes(const char* s, const char* const strarray[]);
	void temporaletests();
	int get_monthdays(int mon, int year);
	bool yisleap(int year);

	void easter_tests();
	void epiphany_tests(void);
	static void print_date(time_t t);
	static void print_time(time_t t);
};
#endif
