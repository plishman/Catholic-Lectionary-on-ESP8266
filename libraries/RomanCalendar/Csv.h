#pragma once

#ifndef _CSV_H
#define _CSV_H

#ifdef _WIN32
	#include "WString.h"
#else
	#include "Arduino.h"
	#include "I2CSerialPort.h"
#endif

#include <utf8string.h>

class Csv
{
public:
	Csv();
	~Csv();
	//int charLenBytesUTF8(char s);
	//String utf8CharAt(String s, int pos);
	String getCsvField(String csvLine, int * ppos);
	String readCsvNumber(String csvLine, int * ppos);
	String readCsvString(String csvLine, int * ppos, bool bSingleWordUnquoted);
	bool isDigit(char c);
};
#endif