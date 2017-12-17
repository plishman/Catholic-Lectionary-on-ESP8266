#pragma once

#ifndef _BIBLEVERSE_H
#define _BIBLEVERSE_H

#define VERSE_MAX_SIZE 3072

#ifdef _WIN32
	#include "WString.h"
#else
	#include <SPI.h>
	#include <SD.h>
#endif

#include "I18n.h"
#include "Csv.h"
#include "I2CSerialPort.h"

class BibleVerse
{
public:
	I18n* _I18n;
	int _book_count = 73;
	BibleVerse(I18n* i);
	~BibleVerse();
	//bool get_bible_filename(String* filename);
	bool get(int book, int chapter, int verse, String* verse_text, int* numRecords);
	//bool initializeSD();
	//void closeFile();
	//int openFile(String filename, uint8_t mode);
	//String readLine();
};

#endif