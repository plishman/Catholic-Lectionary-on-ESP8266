#ifndef _ORDINALIZER_H
#define _ORDINALIZER_H

#include "RCGlobals.h"

#include "Arduino.h"
#include "I2CSerialPort.h"

#define ORDINAL_COUNT 8

class Ordinalizer {
public:
	static const String types[8];
	byte _ordinal_type = 0;
	
	Ordinalizer(String type);
	String ordinalize(int number);
	String ord_french(int number);
	String ord_english(int number);
	String to_roman(unsigned int value);
};

#endif