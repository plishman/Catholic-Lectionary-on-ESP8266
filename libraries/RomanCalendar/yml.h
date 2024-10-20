#ifndef _YML_H
#define _YML_H

#ifdef ESP8266
    #include "Arduino.h"
    #include <SPI.h>
    #include <SD.h>
    #include <pins_arduino.h>
    #include <TimeLib.h>
    #include "DebugPort.h"
#else
    #include "WString.h"
#endif

#define TR_LIT_COLOUR_UNSET  0
#define TR_LIT_COLOUR_WHITE  1
#define TR_LIT_COLOUR_GREEN  2
#define TR_LIT_COLOUR_RED    3
#define TR_LIT_COLOUR_VIOLET 4
#define TR_LIT_COLOUR_ROSE   5
#define TR_LIT_COLOUR_BLACK  6
#define TR_LIT_COLOUR_GOLD   7


struct Tr_Fixed_Feast {
	bool IsFeast;
	uint16_t Lectionary;
	bool Holy_Day_Of_Obligation;
	bool Feast_Of_The_Lord;
	bool ImmaculateConception; // this is an exception in that it can override a sunday in Advent
	uint8_t Class;
	uint8_t Colour;
	String Mass;
	bool bMassIsCommemoration; // sometimes both the first and second lines read will be commemorations
	String Commemoration;
	bool bHasCommemoration;
};


class Yml {
public:
	static String lang;
	static String yml_filename;
	static String sanct_filename;
	static void SetConfig(String lang, String yml_filename, String sanct_filename);

	static int _callcount;
	static String getdate(time64_t t);
	//void SetConfig(String filename, String lang);
	static String get(String I18nPath);
	static String get(String I18nPath, bool& bError);

	static int dayofmonth(time64_t datetime);
	static bool get_fixed_feast(time64_t date, Tr_Fixed_Feast& feast);
	static uint16_t setLectionaryNumber(String s);

	static String readLineAtEnd(File file, bool& bOk, bool bFromEnd = false);
    static String readLine(File file);
    static String readLine(File file, bool& bOk, int endfilepos = -1, int maxbytes = 255);
};

#endif
