#ifndef _ORDINALIZER_H
#define _ORDINALIZER_H

#include "RCGlobals.h"

#include "Arduino.h"
#include "DebugPort.h"

#define ORDINAL_COUNT 8

class Ordinalizer {
public:
	static const String types[8];
	byte _ordinal_type = 0;
	
	Ordinalizer();
	Ordinalizer(String type);
	void SetType(String type);
	String ordinalize(int number);
	static String ord_french(int number);
	static String ord_english(int number);
	static String to_roman(unsigned int value);
};

#endif