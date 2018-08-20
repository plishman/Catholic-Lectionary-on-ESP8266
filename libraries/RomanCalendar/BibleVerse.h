#pragma once

#ifndef _BIBLEVERSE_H
#define _BIBLEVERSE_H

#include "RCGlobals.h"

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
#include "EDB.h"

struct VerseEntry {
	uint16_t verse_number;
	uint16_t fragment_count;
	uint32_t csv_offsets[31];
};

#define TABLE_SIZE 32768  // needs room for 176 verses/chapter, plus the header

extern File dbFile;
extern EDB db;
extern void printDbError(EDB_Status err);

class BibleVerse
{
public:
	I18n* _I18n;
	int _book_count = 73;
	BibleVerse(I18n* i);
	~BibleVerse();
	
	bool get(int book, int chapter, int verse, String* verse_text, int* numRecords);
};

#endif