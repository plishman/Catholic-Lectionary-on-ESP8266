#pragma once

#ifndef _BIBLEVERSE_H
#define _BIBLEVERSE_H

#define VERSE_MAX_SIZE 3072

#ifdef _WIN32
	#include "WString.h"
#endif

#include <I18n.h>

class BibleVerse
{
public:
	String _bible_filename = "njb.csv";

	I18n* _I18n;
	BibleVerse(I18n* i);
	~BibleVerse();
	bool get(int book, int chapter, int verse, String* verse_text);
	//bool initializeSD();
	//void closeFile();
	//int openFile(String filename, uint8_t mode);
	//String readLine();
};

#endif