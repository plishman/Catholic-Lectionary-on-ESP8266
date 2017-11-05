#pragma once

#ifndef _I18n_H
#define _I18n_H

#ifndef _WIN32
	#include "Arduino.h"
	#include <SPI.h>
	#include <SD.h>
#else
	#include "WString.h"
#endif


#include "Enums.h"

class I18n
{
public:
	bool _suppress_output = false;
	
	String _lang = "";
	String _yml_filename = "";
	String _sanctorale_filename = "";
	String _bible_filename = "";
	bool _have_config = false;
	
	Enums::I18nLanguages _locale;
	static const char* const I18n_LANGUAGES[5];
	static const char* const I18n_SANCTORALE[5];
	static const char* const I18n_SEASONS[5];
	static const char* const I18n_COLOURS[4];
	static const char* const I18n_RANK_NAMES[14];
	static const char* const I18n_SOLEMNITIES[17];

	int _callcount = 0;
#ifndef _WIN32
	int _CS_PIN = 10;

	I18n(int CS_PIN);
	void suppress_output(bool s);
	bool initializeSD();
	String readLine(File file);
	File openFile(String filename, uint8_t mode);
	void closeFile(File file);
#else
	I18n( void );
#endif
	bool get_config( void );
	String get(String I18nPath);
	~I18n();
};
#endif