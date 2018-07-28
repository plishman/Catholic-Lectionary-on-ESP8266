#pragma once
#ifndef _SANCTORALE_H
#define _SANCTORALE_H

#ifndef _WIN32
	#include "Arduino.h"
	#include "../Time/TimeLib.h"
	#include "I2CSerialPort.h"
#else
	#include <time.h>
	#include "WString.h"
#endif

#include "Enums.h"
#include "Temporale.h"
#include "Enums.h"
#include "I18n.h"

class Sanctorale {
public:
	Temporale* _temporale;
	I18n* _I18n;
	
	String _sanctorale;
	String _rank;
	String _colour;

	String _holy_day_of_obligation = "";
	
	int _Lectionary = 0;

	Enums::Colours _colour_e;
	Enums::Ranks _rank_e;

	bool _bIsSolemnity = false;
	
	bool _hdo = false;

	Enums::I18nLanguages _locale;
	bool _transfer_to_sunday;

	Sanctorale(bool transfer_to_sunday, I18n* i);
	~Sanctorale();
	bool get(time64_t date);
	void setLectionaryNumber(String s);
	void setColour(Enums::Colours c);
	Enums::Colours getColour(void);
	void setRank(Enums::Ranks r);
	Enums::Ranks getRank(void);
};

#endif