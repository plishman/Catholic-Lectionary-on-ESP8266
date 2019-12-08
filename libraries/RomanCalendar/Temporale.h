#pragma once
#ifndef RomanCalender_h
#define RomanCalendar_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "WString.h"
#include "Enums.h"
#include "I18n.h"
#include "Ordinalizer.h"

#ifndef _WIN32
	#include "Arduino.h"
	#include "../Time/TimeLib.h"
	#include "DebugPort.h"
#else
	#include <time.h>
#endif

#ifdef _WIN32
	const int BEGIN_EPOCH = 1900; // 1900 for 64-bit time64_t, sometimes 1970 (may be on embedded system)
#else
	const int BEGIN_EPOCH = 1970; // 1900 for 64-bit time64_t, sometimes 1970 (may be on embedded system)
#endif

const int WEEK = 7;
const int DAY = 24 * 3600;

class Temporale {
public:
	I18n* _I18n;
	Ordinalizer* _ordinalizer;
	
	String _day = "";
	String _rank = "";
	String _season = "";
	String _colour = "";
	
	String _holy_day_of_obligation = "";

	Enums::Ranks _rank_e;
	Enums::Colours _colour_e;

	int _Lectionary = 0;

	bool _bIsSolemnity = false;

	bool _hdo = false;
	
	typedef struct {
		Enums::Solemnities s;
		Enums::Colours c;
		Enums::Ranks r;
	} Solemnity;

	//char* DAYS_OF_WEEK[7];
	//char* RANK_NAME[14];
	//char* RANK_TYPES[9];
	//char* COLOURS[4];
	//char* SEASONS[5];
	//char* SOLEMNITIES[17];
	//char* SUNDAYS_AND_FERIALS[16];

	static const char* const RANK_PRIORITY[14];
	static const char* const SUNDAY_TRANSFERABLE_SOLEMNITIES[3];
	//static const Enums::Ranks SOLEMNITIES[17];
	static const Enums::Colours SOLEMNITIES_COLOURS[19];
	static const Enums::Ranks SOLEMNITIES_RANKS[19];

	bool _transfer_to_sunday; // flag determines whether epiphany, ascension and corpus Christi should be transferred to sunday (us, uk etc)

	Temporale(bool transfer_to_sunday, I18n* i);
	~Temporale(void);
	
	String getLocaleWeekday(int dayofweek);
	static const char* const SOLEMNITIES_I18n[17];

	void datestests();
	bool getTm(int day, int month, int year, int hours, int minutes, int seconds, struct tm* ts);
	static time64_t date(int day, int month, int year);
    
	static time64_t weekday_before(int weekdayBefore, time64_t date);
	static time64_t sunday_before(time64_t date);
	static time64_t monday_before(time64_t date);
	static time64_t tuesday_before(time64_t date);
	static time64_t wednesday_before(time64_t date);
	static time64_t thursday_before(time64_t date);
	static time64_t friday_before(time64_t date);
	static time64_t saturday_before(time64_t date);
     
	static time64_t weekday_after(int weekdayAfter, time64_t date);
	static time64_t sunday_after(time64_t date);
	static time64_t monday_after(time64_t date);
	static time64_t tuesday_after(time64_t date);
	static time64_t wednesday_after(time64_t date);
	static time64_t thursday_after(time64_t date);
	static time64_t friday_after(time64_t date);
	static time64_t saturday_after(time64_t date);

	static int dayofweek(time64_t date);
	static bool sunday(time64_t date);
	static bool monday(time64_t date);
	static bool tuesday(time64_t date);
	static bool wednesday(time64_t date);
	static bool thursday(time64_t date);
	static bool friday(time64_t date);
	static bool saturday(time64_t date);

	int date_difference(time64_t date1, time64_t date2);
	int hour_of_day(time64_t time);
	int year(time64_t date);
	static int month(time64_t date);
	static int dayofmonth(time64_t date);
	bool issameday(time64_t date1, time64_t date2);

	time64_t start_date(int year);
	time64_t end_date(int year);
	time64_t first_advent_sunday(int year);
	time64_t immaculate_conception(int year);
	time64_t nativity(int year);
	time64_t holy_family(int year);
	time64_t mother_of_god(int year);
	time64_t epiphany(int year);
	time64_t baptism_of_lord(int year);
	time64_t ash_wednesday(int year);
	time64_t easter_sunday(int year);
	time64_t palm_sunday(int year);
	time64_t good_friday(int year);
	time64_t holy_saturday(int year);
	time64_t ascension(int year);
	time64_t pentecost(int year);
	time64_t christ_eternal_priest(int year);
	time64_t holy_trinity(int year);
	time64_t corpus_christi(int year);
	time64_t sacred_heart(int year);
	time64_t immaculate_heart(int year);
	time64_t christ_king(int year);
	time64_t octave_of(time64_t date);

	int liturgical_year(time64_t date);
	int for_day(time64_t date);
	Enums::Season season(time64_t date);
	time64_t season_beginning(Enums::Season s, time64_t date);
	int season_week(Enums::Season seasonn, time64_t date);

	void setColour(Enums::Colours c);
	Enums::Colours getColour(void);

	void setRank(Enums::Ranks r);

	Enums::Ranks getRank(void);

	static const Enums::Season SEASONS_SUNDAY_PRIMARY[3];
	String sunday_temporale(time64_t date);
	String ferial_temporale(time64_t date);
	void christmas_vigil(time64_t date);
	void christmas_lectionary(time64_t date);
	bool get(time64_t date);

	bool do_solemnities(time64_t date);
	bool do_sundays(time64_t date);
	bool do_ferials(time64_t date);
	
	String ordinalize(int number);
	bool includes(const char* s, const char* const strarray[]);
	void temporaletests();
	int get_monthdays(int mon, int year);
	bool yisleap(int year);

	void easter_tests();
	void epiphany_tests(void);
	static void print_date(time64_t t);
	static void print_time(time64_t t);
};
#endif
