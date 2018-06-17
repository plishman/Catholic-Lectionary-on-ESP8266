#pragma once

#ifndef _I18n_H
#define _I18n_H

#ifndef _WIN32
	#include "Arduino.h"
	#include <SPI.h>
	#include <SD.h>
	#include <pins_arduino.h>
	#include "I2CSerialPort.h"
#else
	#include "WString.h"
#endif


#include "Enums.h"
class ConfigParams
{
public:
	String desc = "";
	String lang = "";
	String yml_filename = "";
	String sanctorale_filename = "";
	String bible_filename = "";
	String font_filename = "builtin";
	bool transfer_to_sunday = true;
	bool celebrate_feast_of_christ_priest = true;
	bool have_config = false;
	bool right_to_left = false;
	double font_tuning_percent = 50.0;
	bool font_use_fixed_spacing = false;
	bool font_use_fixed_spacecharwidth = false;
	int font_fixed_spacing = 1;
	int font_fixed_spacecharwidth = 2;
	
	void Clear() {
		desc = "";
		lang = "";
		yml_filename = "";
		sanctorale_filename = "";
		bible_filename = "";
		font_filename = "builtin";
		transfer_to_sunday = true;
		celebrate_feast_of_christ_priest = true;
		have_config = false;
		right_to_left = false;
		font_tuning_percent = 50.0;
		font_use_fixed_spacing = false;
		font_use_fixed_spacecharwidth = false;
		font_fixed_spacing = 1;
		font_fixed_spacecharwidth = 2;		
	}
	
	void Dump() {
		I2CSerial.printf("\tdesc=%s\n", desc.c_str());
		I2CSerial.printf("\tlang=%s\n", lang.c_str());
		I2CSerial.printf("\tyml_filename=%s\n", yml_filename.c_str());
		I2CSerial.printf("\tsanctorale_filename=%s\n", sanctorale_filename.c_str());
		I2CSerial.printf("\tbible_filename=%s\n", bible_filename.c_str());
		I2CSerial.printf("\ttransfer_to_sunday=%s\n", String(transfer_to_sunday).c_str());
		I2CSerial.printf("\tcelebrate_feast_of_christ_eternal_priest=%s\n", String(celebrate_feast_of_christ_priest).c_str());
		I2CSerial.printf("\tfont filename=%s\n", font_filename.c_str());
		I2CSerial.printf("\tright_to_left=%s\n", String(right_to_left).c_str());			
		I2CSerial.printf("\tfont fixed spacing=%d\n", font_fixed_spacing);
		I2CSerial.printf("\tfont fixed spacecharwidth=%d\n", font_fixed_spacecharwidth);
		I2CSerial.printf("\tfont tuning=%s%%\n", String(font_tuning_percent).c_str());	
		I2CSerial.printf("\tfont use fixed spacing: %s\n", font_use_fixed_spacing ? "Yes" : "No (If not using builtin font, will use character advanceWidths and % tuning value instead)");	
		I2CSerial.printf("\tfont use fixed spacecharwidth: %s\n", font_use_fixed_spacecharwidth ? "Yes" : "No (If not using builtin font, will use space character advanceWidth and % tuning value instead)");				
	}
	
	ConfigParams() {};
	
	// copy constructor
	ConfigParams(const ConfigParams &p2) {
		desc = p2.desc;
		lang = p2.lang;
		yml_filename = p2.yml_filename;
		sanctorale_filename = p2.sanctorale_filename;
		bible_filename = p2.bible_filename;
		font_filename = p2.font_filename;
		transfer_to_sunday = p2.transfer_to_sunday;
		celebrate_feast_of_christ_priest = p2.celebrate_feast_of_christ_priest;
		have_config = p2.have_config;
		right_to_left = p2.right_to_left;
		font_tuning_percent = p2.font_tuning_percent;
		font_use_fixed_spacing = p2.font_use_fixed_spacing;
		font_use_fixed_spacecharwidth = p2.font_use_fixed_spacecharwidth;
		font_fixed_spacing = p2.font_fixed_spacing;
		font_fixed_spacecharwidth = p2.font_fixed_spacecharwidth;		
	}
};


class I18n
{
public:
	ConfigParams configparams;
	
	bool _suppress_output = false;
/*	
	String _lang = "";
	String _yml_filename = "";
	String _sanctorale_filename = "";
	String _bible_filename = "";
	String _font_filename = "builtin";
	bool _transfer_to_sunday = true;
	bool _celebrate_feast_of_christ_priest = true;
	bool _have_config = false;
	bool _right_to_left = false;
	double _font_tuning_percent = 50.0;
	bool _font_use_fixed_spacing = false;
	bool _font_use_fixed_spacecharwidth = false;
	int _font_fixed_spacing = 1;
	int _font_fixed_spacechar_width = 2;
*/	
	Enums::I18nLanguages _locale;
	static const char* const I18n_LANGUAGES[5];
	static const char* const I18n_SANCTORALE[5];
	static const char* const I18n_SEASONS[5];
	static const char* const I18n_COLOURS[4];
	static const char* const I18n_RANK_NAMES[14];
	static const char* const I18n_SOLEMNITIES[18];

	int _callcount = 0;
	int _lectionary_config_number = 1;
#ifndef _WIN32
	int _CS_PIN = D1;
	
	I18n(int CS_PIN, int lectionary_config_number);
	I18n( void );
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