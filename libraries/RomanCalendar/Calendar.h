#pragma once

#ifndef _CALENDAR_H
#define _CALENDAR_H

#ifndef _WIN32
	#include "Arduino.h"
	#include "../Time/TimeLib.h"
#else
	#include <time.h>
	#include "WString.h"
#endif

#include "Enums.h"
#include "I18n.h"
#include "Temporale.h"
#include "Sanctorale.h"
#include "Transfers.h"

class Calendar {
public:
	typedef struct Day {
		time_t date;
		String liturgical_year;
		String liturgical_cycle;
		String season;
		String colour;
		String rank;
		String day;
		int lectionary;				 
		bool is_sanctorale;
		String sanctorale;
		String sanctorale_colour;
		String sanctorale_rank;
	} Day;

	Day day;

	I18n* _I18n;
	Temporale* temporale;
	Sanctorale* sanctorale;
	Transfers* transfers;

	static const char* const LITURGICAL_CYCLES[2];
	static const char* const LITURGICAL_YEARS[3];

	static const char* const I18n_SEASONS[5];
	static const char* const I18n_LANGUAGES[5];
	static const char* const I18n_COLOURS[4];
	static const char* const I18n_RANK_NAMES[14];

	Enums::I18nLanguages _locale;

	bool _transfer_to_sunday;
	time_t _date;

#ifndef _WIN32
	int _CS_PIN = 10;
	Calendar(bool transfer_to_sunday, Enums::I18nLanguages l, int CS_PIN);
#else
	Calendar(bool transfer_to_sunday, Enums::I18nLanguages l);
#endif
	~Calendar();
	bool get(time_t date);
	//String I18n(String I18nPath);
	Enums::Liturgical_Year liturgical_year_letter(int year);
	Enums::Liturgical_Cycle liturgical_cycle(int year);
};
#endif