#pragma once

#ifndef _CSV_H
#define _CSV_H

#ifdef _WIN32
	#include <string>
	using namespace std;
#else
	#include "Arduino.h"
	#include "I2CSerialPort.h"
#endif

class Csv
{
public:
	Csv();
	~Csv();
	int charLenBytesUTF8(char s);
	string utf8CharAt(string s, unsigned int pos);
	string getCsvField(string csvLine, unsigned int * ppos);
	string readCsvNumber(string csvLine, unsigned int * ppos);
	string readCsvString(string csvLine, unsigned int * ppos, bool bSingleWordUnquoted);
	bool isDigit(char c);
};
#endif