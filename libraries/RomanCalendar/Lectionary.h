#ifndef _LECTIONARY_H
#define _LECTIONARY_H

#include "RCGlobals.h"

#define REF_MAX_SIZE 1500

#ifdef _WIN32
	#include "WString.h"
#endif

#include <I18n.h>
#include "DebugPort.h"

enum ReadingsFromEnum {
	READINGS_OT = 0,
	READINGS_NT,
	READINGS_PS,
	READINGS_G 
};

class Lectionary {
public:	
	I18n* _I18n;
	Lectionary(I18n* i);
	~Lectionary();
	bool test(int number, String liturgical_year, String liturgical_cycle, bool *b_OT, bool *b_NT, bool *b_PS, bool *b_G);
	bool testFile(String filename);
	bool get(String liturgical_year, String liturgical_cycle, ReadingsFromEnum readings_from, int number, String* refs_text);
};


#endif