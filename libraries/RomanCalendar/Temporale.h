#pragma once
#ifndef _TEMPORALE_H
#define _TEMPORALE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
	#include "Arduino.h"
	#include "../Time/TimeLib.h"
#else
	#include <time.h>
	#include "WString.h"
#endif

#include "Enums.h"
#include "I18n.h"

#ifdef _WIN32
	const int BEGIN_EPOCH = 1900; // 1900 for 64-bit time_t, sometimes 1970 (may be on embedded system)
#else
	const int BEGIN_EPOCH = 1970; // 1900 for 64-bit time_t, sometimes 1970 (may be on embedded system)
#endif

const int WEEK = 7;
const int DAY = 24 * 3600;

class Temporale {
public:
	//Enums::I18nLanguages _locale;
	I18n* _I18n;

	String _day;
	String _rank;
	String _season;
	String _colour;

	Enums::Ranks _rank_e;
	Enums::Colours _colour_e;

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

	//String _buffer;
	//String _ordinal;
	//String _rank;
	//String _Enums::Season;
	//String _colour;
	//String _I18nbuf;

	//char _buffer[1024];
	//char _ordinal[32];

	//String _liturgical_year;
	//String _liturgical_cycle;

	//const char* _colour;
	//int _bufferlength = 1024;
	//char _I18nbuf[1024];
	
	//char* I18n(const char* str);
	//*********String I18n(String I18nPath);
	//const char* getNextToken(char* buf, const char* I18n_path);
	//bool parseline(const char* buf, char* readtokenbuf, char* readtokendata);
	//char* strstrip(char *s);
	//bool isspace(int c);
	//String removeTemplateVars(String inString);
/*
	enum Liturgical_Year {
		LITURGICAL_YEAR_A = 0,
		LITURGICAL_YEAR_B,
		LITURGICAL_YEAR_C
	};
*/
/*	
	static const char* const LITURGICAL_YEARS[3];
	Liturgical_Year liturgical_year_letter(time_t date);

	enum Liturgical_Cycle {
		LITURGICAL_CYCLE_I = 0,
		LITURGICAL_CYCLE_II
	};

	static const char* const LITURGICAL_CYCLES[2];
	Liturgical_Cycle liturgical_cycle(time_t date);
*/
/*
	enum Enums::Season {
		Enums::Season_ADVENT,
		Enums::Season_CHRISTMAS,
		Enums::Season_LENT,
		Enums::Season_EASTER,
		Enums::Season_ORDINARY
	};

	enum Enums::Ranks { // the ordinal number is used as an index into the array of Enums::Ranks (the description), *and also for comparing seniority*
		Enums::Ranks_TRIDUUM = 13,									//= Rank.new(1.1, 'rank.1_1'),
		Enums::Ranks_PRIMARY = 12,									//= Rank.new(1.2, 'rank.1_2'), # description may not be exact
		Enums::Ranks_SOLEMNITY_GENERAL = 11,						//= Rank.new(1.3, 'rank.1_3', 'rank.short.solemnity'), # description may not be exact
		Enums::Ranks_SOLEMNITY_PROPER = 10,						//= Rank.new(1.4, 'rank.1_4', 'rank.short.solemnity'),
															//
		Enums::Ranks_FEAST_LORD_GENERAL = 9,						//= Rank.new(2.5, 'rank.2_5', 'rank.short.feast'),
		Enums::Ranks_SUNDAY_UNPRIVILEGED = 8,						//= Rank.new(2.6, 'rank.2_6', 'rank.short.sunday'),
		Enums::Ranks_FEAST_GENERAL = 7,							//= Rank.new(2.7, 'rank.2_7', 'rank.short.feast'),
		Enums::Ranks_FEAST_PROPER = 6,								//= Rank.new(2.8, 'rank.2_8', 'rank.short.feast'),
		Enums::Ranks_FERIAL_PRIVILEGED = 5,						//= Rank.new(2.9, 'rank.2_9', 'rank.short.ferial'),

		Enums::Ranks_MEMORIAL_GENERAL = 4,							//= Rank.new(3.10, 'rank.3_10', 'rank.short.memorial'),
		Enums::Ranks_MEMORIAL_PROPER = 3,							//= Rank.new(3.11, 'rank.3_11', 'rank.short.memorial'),
		Enums::Ranks_MEMORIAL_OPTIONAL = 2,						//= Rank.new(3.12, 'rank.3_12', 'rank.short.memorial_opt'),
		Enums::Ranks_FERIAL = 1,									//= Rank.new(3.13, 'rank.3_13', 'rank.short.ferial'),
		//# not included as a celebration rank on it's own
		//# in the Table of Liturgical Days
		Enums::Ranks_COMMEMORATION = 0,							//= Rank.new(4.0, 'rank.4_0', 'rank.short.commemoration')
	};
*/
/*
	enum RankType {
		RANKTYPE_EASTER_TRIDUUM, // in the original ruby code, these first two were not included. They map to null text in the short descriptions
		RANKTYPE_PRIMARY_LITURGICAL_DAYS, // 
		RANKTYPE_SOLEMNITY,
		RANKTYPE_FEAST,
		RANKTYPE_SUNDAY,
		RANKTYPE_MEMORIAL,
		RANKTYPE_MEMORIAL_OPT,
		RANKTYPE_FERIAL,
		RANKTYPE_COMMEMORATION
	};

	static const RankType RANK_TYPE[14];
*/
/*
	enum Solemnities {
		SOLEMNITIES_NATIVITY = 0,
		SOLEMNITIES_HOLY_FAMILY,
		SOLEMNITIES_MOTHER_OF_GOD,
		SOLEMNITIES_EPIPHANY,
		SOLEMNITIES_BAPTISM_OF_LORD,
		SOLEMNITIES_ASH_WEDNESDAY,
		SOLEMNITIES_PALM_SUNDAY,
		SOLEMNITIES_GOOD_FRIDAY,
		SOLEMNITIES_HOLY_SATURDAY,
		SOLEMNITIES_EASTER_SUNDAY,
		SOLEMNITIES_ASCENSION,
		SOLEMNITIES_PENTECOST,
		SOLEMNITIES_HOLY_TRINITY,
		SOLEMNITIES_CORPUS_CHRISTI,
		SOLEMNITIES_SACRED_HEART,
		SOLEMNITIES_IMMACULATE_HEART,
		SOLEMNITIES_CHRIST_KING
	};
*/
/*
	enum Colours {
		COLOURS_GREEN = 0,
		COLOURS_VIOLET,
		COLOURS_WHITE,
		COLOURS_RED,
	};
*/
/*
	enum Sundays_and_Ferials {
		SUNDAYS_OF_ADVENT = 0,
		SUNDAYS_AFTER_NATIVITY,
		SUNDAYS_OF_EASTER,
		SUNDAYS_OF_LENT,
		SUNDAYS_OF_ORDINARY_TIME,
		DAYS_DECEMBER_BEFORE_CHRISTMAS,
		DAYS_OF_CHRISTMAS_OCTAVE,
		DAYS_AFTER_EPIPHANY,
		DAYS_AFTER_ASH_WEDNESDAY,
		DAYS_OF_HOLY_WEEK,
		DAYS_OF_EASTER_WEEK,
		DAYS_OF_ADVENT,
		DAYS_AFTER_CHRISTMAS_OCTAVE,
		DAYS_OF_LENT,
		DAYS_OF_EASTER,
		DAYS_OF_ORDINARY_TIME
	};
*/
/*
	enum I18nFilenames {
		FILENAMES_DAYSOFWEEK = 0,
		FILENAMES_Enums::SeasonS,
		FILENAMES_COLOURS,
		FILENAMES_SOLEMNITIES,
		FILENAMES_RANKNAMES,
		FILENAMES_RANKTYPES,
		FILENAMES_SUNDAYSFERIALS
	};
*/
//	static const char* const I18n_LANGUAGES[5];			// stores the string associated with the language, indexed by enum I18nLanguages
//	static const char* const I18n_SANCTORALE[5];	// list of filenames for files containing all of the commemorations, memorials and feasts throughout the year, by locale
//	static const char* const I18n_FILENAMES[7];			// stores filenames of txt files containing localized strings, indexed by enum I18nFilenames
//	static const char* const I18n_COLOURS[4];			// stores the yml paths to get the localized strings for the colours
//	static const char* const I18n_RANK_NAMES[14];		// stores the yml paths to get the localized strings for the names of the Enums::Ranks
//	static const char* const I18n_Enums::SeasonS[5];			// stores the yml paths to get the localized strings for the names of the liturgical Enums::Seasons

	//static const char* const DAYS_OF_WEEK[7];


	//static const char* const RANK_NAME[14];
	//static const char* const RANK_TYPES[9];
	//static const char* const COLOURS[4];
	//static const char* const Enums::SeasonS[5];

	//static const char* const SOLEMNITIES[17];
	//static const char* const SUNDAYS_AND_FERIALS[16];
	
	
	/*
	struct Celebration {
	CelebrationName celebration_name;
	CelebrationFullName celebration_full_name;
	Colour colour;
	Enums::Season Enums::Season;
	RankEnum rank;
	};
	*/
	bool _transfer_to_sunday; // flag determines whether epiphany, ascension and corpus Christi should be transferred to sunday (us, uk etc)

	Temporale(bool transfer_to_sunday, I18n* i);
	~Temporale(void);
	
	//void loadI18nStrings(void);
	//bool readI18nfile(char* strArray[], int num, I18nFilenames f, I18nLanguages l);
	//char* getLocaleWeekday(int dayofweek, char* buf);
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
	//bool sanctorale_get(time_t date, bool move_to_monday);
	String sunday_temporale(time_t date);
	String ferial_temporale(time_t date);
	bool get(time_t date);

	bool do_solemnities(time_t date);
	//bool do_feasts_and_memorials(time_t date);
	bool do_sundays(time_t date);
	bool do_ferials(time_t date);
	
	//char* ordinalize(int number);
	String ordinalize(int number);
	bool includes(const char* s, const char* const strarray[]);
	void temporaletests();
	int get_monthdays(int mon, int year);
	bool yisleap(int year);

	void easter_tests();
	void epiphany_tests(void);
	static void print_date(time_t t);
};
#endif
