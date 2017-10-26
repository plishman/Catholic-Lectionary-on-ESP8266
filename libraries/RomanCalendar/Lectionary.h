#ifndef _LECTIONARY_H
#define _LECTIONARY_H

#define REF_MAX_SIZE 1500

#ifdef _WIN32
	#include "WString.h"
#endif

#include <I18n.h>

class Lectionary {
public:
	enum ReadingsFromEnum {
		READINGS_OT = 0,
		READINGS_NT,
		READINGS_PS,
		READINGS_G 
	};
	
	I18n* _I18n;
	Lectionary(I18n* i);
	~Lectionary();
	bool get(String liturgical_year, String liturgical_cycle, Lectionary::ReadingsFromEnum readings_from, int number, String* refs_text);
};


#endif