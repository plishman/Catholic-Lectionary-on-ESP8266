#pragma once

#ifndef _BIBLE_H
#define _BIBLE_H

#include "RCGlobals.h"

#ifdef _WIN32
	#include "LinkedList/LinkedList.h"
	#include "WString.h"
#else
	#include "Arduino.h"
	#include "LinkedList.h"
	#include "DebugPort.h"
#endif

#include "I18n.h"
#include "BibleVerse.h"

class Ref {
public:
	int book_index;
	int start_chapter;
	int start_verse;
	String start_verse_sentence_range; // eg a
	int end_chapter;
	int end_verse;
	String end_verse_sentence_range; // eg c
	String refs;
	int book_count;
};

class Bible {
public:
	static const char s_00[] PROGMEM;
	static const char s_01[] PROGMEM;
	static const char s_02[] PROGMEM;
	static const char s_03[] PROGMEM;
	static const char s_04[] PROGMEM;
	static const char s_05[] PROGMEM;
	static const char s_06[] PROGMEM;
	static const char s_07[] PROGMEM;
	static const char s_08[] PROGMEM;
	static const char s_09[] PROGMEM;
	static const char s_10[] PROGMEM;
	static const char s_11[] PROGMEM;
	static const char s_12[] PROGMEM;
	static const char s_13[] PROGMEM;
	static const char s_14[] PROGMEM;
	static const char s_15[] PROGMEM; /*"Tobit"; "Judith";*/ 
	static const char s_16[] PROGMEM; /*"1 Maccabees"; "2 Maccabees" */ 
	static const char s_17[] PROGMEM;
	static const char s_18[] PROGMEM; 
	static const char s_19[] PROGMEM; 
	static const char s_20[] PROGMEM; 
	static const char s_21[] PROGMEM; /*"Wisdom"; "Sirach";*/ 
	static const char s_22[] PROGMEM;
	static const char s_23[] PROGMEM;
	static const char s_24[] PROGMEM; /*"Baruch";*/ 
	static const char s_25[] PROGMEM;
	static const char s_26[] PROGMEM; 
	static const char s_27[] PROGMEM;
	static const char s_28[] PROGMEM; 
	static const char s_29[] PROGMEM; 
	static const char s_30[] PROGMEM;
	static const char s_31[] PROGMEM;
	static const char s_32[] PROGMEM;
	static const char s_33[] PROGMEM;
	static const char s_34[] PROGMEM;
	static const char s_35[] PROGMEM;
	static const char s_36[] PROGMEM;
	static const char s_37[] PROGMEM;
	static const char s_38[] PROGMEM;
	static const char s_39[] PROGMEM;
	static const char s_40[] PROGMEM; 
	static const char s_41[] PROGMEM; 
	static const char s_42[] PROGMEM; 
	static const char s_43[] PROGMEM; 
	static const char s_44[] PROGMEM; 
	static const char s_45[] PROGMEM;
	static const char s_46[] PROGMEM;
	static const char s_47[] PROGMEM;
	static const char s_48[] PROGMEM;
	static const char s_49[] PROGMEM;
	static const char s_50[] PROGMEM;
	static const char s_51[] PROGMEM;
	static const char s_52[] PROGMEM;
	static const char s_53[] PROGMEM;
	static const char s_54[] PROGMEM;
	static const char s_55[] PROGMEM;
	static const char s_56[] PROGMEM;
	static const char s_57[] PROGMEM;
	static const char s_58[] PROGMEM;
	static const char s_59[] PROGMEM;
	static const char s_60[] PROGMEM;
	static const char s_61[] PROGMEM; 
	static const char s_62[] PROGMEM; 
	static const char s_63[] PROGMEM; 
	static const char s_64[] PROGMEM; 
	static const char s_65[] PROGMEM;
	static const char s_66[] PROGMEM;  
	static const char s_67[] PROGMEM;  
	static const char s_68[] PROGMEM; 
	static const char s_69[] PROGMEM;  
	static const char s_70[] PROGMEM;  
	static const char s_71[] PROGMEM; 
	static const char s_72[] PROGMEM; 

	static const char* const books[73] PROGMEM;

	static const char s2_00[] PROGMEM;
	static const char s2_01[] PROGMEM;
	static const char s2_02[] PROGMEM;
	static const char s2_03[] PROGMEM;
	static const char s2_04[] PROGMEM;
	static const char s2_05[] PROGMEM;
	static const char s2_06[] PROGMEM;
	static const char s2_07[] PROGMEM;
	static const char s2_08[] PROGMEM;
	static const char s2_09[] PROGMEM;
	static const char s2_10[] PROGMEM;
	static const char s2_11[] PROGMEM;
	static const char s2_12[] PROGMEM;
	static const char s2_13[] PROGMEM;
	static const char s2_14[] PROGMEM;
	static const char s2_15[] PROGMEM;
	static const char s2_16[] PROGMEM;
	static const char s2_17[] PROGMEM;
	static const char s2_18[] PROGMEM;
	static const char s2_19[] PROGMEM;
	static const char s2_20[] PROGMEM;
	static const char s2_21[] PROGMEM;
	static const char s2_22[] PROGMEM;
	static const char s2_23[] PROGMEM;
	static const char s2_24[] PROGMEM;
	static const char s2_25[] PROGMEM;
	static const char s2_26[] PROGMEM;
	static const char s2_27[] PROGMEM;
	static const char s2_28[] PROGMEM;
	static const char s2_29[] PROGMEM;
	static const char s2_30[] PROGMEM;
	static const char s2_31[] PROGMEM;
	static const char s2_32[] PROGMEM;
	static const char s2_33[] PROGMEM;
	static const char s2_34[] PROGMEM;
	static const char s2_35[] PROGMEM;
	static const char s2_36[] PROGMEM;
	static const char s2_37[] PROGMEM;
	static const char s2_38[] PROGMEM;
	static const char s2_39[] PROGMEM;
	static const char s2_40[] PROGMEM;
	static const char s2_41[] PROGMEM;
	static const char s2_42[] PROGMEM;
	static const char s2_43[] PROGMEM;
	static const char s2_44[] PROGMEM;
	static const char s2_45[] PROGMEM;
	static const char s2_46[] PROGMEM;
	static const char s2_47[] PROGMEM;
	static const char s2_48[] PROGMEM;
	static const char s2_49[] PROGMEM;
	static const char s2_50[] PROGMEM;
	static const char s2_51[] PROGMEM;
	static const char s2_52[] PROGMEM;
	static const char s2_53[] PROGMEM;
	static const char s2_54[] PROGMEM;
	static const char s2_55[] PROGMEM;
	static const char s2_56[] PROGMEM;
	static const char s2_57[] PROGMEM;
	static const char s2_58[] PROGMEM;
	static const char s2_59[] PROGMEM;
	static const char s2_60[] PROGMEM;
	static const char s2_61[] PROGMEM;
	static const char s2_62[] PROGMEM;
	static const char s2_63[] PROGMEM;
	static const char s2_64[] PROGMEM;
	static const char s2_65[] PROGMEM;
	static const char s2_66[] PROGMEM;
	static const char s2_67[] PROGMEM;
	static const char s2_68[] PROGMEM;
	static const char s2_69[] PROGMEM;
	static const char s2_70[] PROGMEM;
	static const char s2_71[] PROGMEM;
	static const char s2_72[] PROGMEM;

	static const char* const books_shortnames[73] PROGMEM;


	LinkedList<Ref*> refsList = LinkedList<Ref*>();
	I18n* _I18n;
	BibleVerse* _bibleverse;
	//static const char* const books[];
	//static const char* const books_shortnames[];
	static const int books_chaptercounts[] PROGMEM;

	//static const char* const books_with_apocrypha[];
	//static const char* const books_shortnames_with_apocrypha[];
	//static const int books_chaptercounts_with_apocrypha[];
	
	//const char* const *books;
	//const char* const *books_shortnames;
	//const int* books_chaptercounts;
	int _book_count = 73;
	
	Bible(I18n* i);
	~Bible(void);

	bool get(String refs);

	void dump_refs();
	String getBookShortName(int booknum);
	bool getUniqueBookInRefs(int& booknum, int& index);
	
	String sentence_ref(int from, int to);

	uint32_t add_reference(String refs, int book_index, 
		int start_chapter, int end_chapter,
		int start_verse, int end_verse,
		String start_verse_sentence_range, String end_verse_sentence_range);

	bool is_book(String refs, int startpos);

	bool expect_book(String refs, int * startpos, int* book_index);

	bool is_chapter(String refs, int startpos);

	bool expect_chapter(String refs, int * startpos, int* chapter);

	bool parse_verse_range(String refs, int* startpos, 
		int* start_chapter, int* end_chapter,
		int* start_verse, int* end_verse,
		String* start_verse_sentence_range,
		String* end_verse_sentence_range);

	bool parse_verse(String refs, int* startpos, int* chapter, int* verse, String* sentence_range);

	bool expect(String s, String c, int * pos);

	bool get_sentences(String s, int* startpos, int* start_sentence, int* end_sentence);

	int readnumber(String s, int * nextpos);

	//bool getVerse(String book, int chapter, int verse, String* verse_text);
	int getBookIndex(const char* book);
	int getBookIndex(const char* ref, const char* book, const char* verses);
};

#endif