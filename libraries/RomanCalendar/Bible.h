#pragma once

#ifndef _BIBLE_H
#define _BIBLE_H

#ifdef _WIN32
	#include "LinkedList/LinkedList.h"
	#include "WString.h"
#else
	#include "Arduino.h"
	#include "LinkedList.h"
#endif

class Ref {
public:
	int book_index;
	int start_chapter;
	int start_verse;
	int start_first_sentence; // eg a
	int start_last_sentence;
	int end_chapter;
	int end_verse;
	int end_first_sentence; // eg c
	int end_last_sentence;
	String refs;
};

class Bible {
public:
	LinkedList<Ref*> refsList = LinkedList<Ref*>();
	static const char* const books[];
	static const char* const books_shortnames[];
	static const int books_chaptercounts[73];

	Bible(void);
	~Bible(void);

	bool get(String refs);

	void dump_refs();
	String sentence_ref(int from, int to);

	void add_reference(String refs, int book_index, 
		int start_chapter, int end_chapter,
		int start_verse, int end_verse,
		int start_first_sentence, int start_last_sentence,
		int end_first_sentence, int end_last_sentence);

	bool is_book(String refs, int startpos);

	bool expect_book(String refs, int * startpos, int* book_index);

	bool is_chapter(String refs, int startpos);

	bool expect_chapter(String refs, int * startpos, int* chapter);

	bool parse_verse_range(String refs, int* startpos, 
		int* start_chapter, int* end_chapter,
		int* start_verse, int* end_verse,
		int* start_first_sentence, int* start_last_sentence,
		int* end_first_sentence, int* end_last_sentence);

	bool parse_verse(String refs, int* startpos, int* chapter, int* verse, int* start_sentence, int* end_sentence);

	bool expect(String s, String c, int * pos);

	bool get_sentences(String s, int* startpos, int* start_sentence, int* end_sentence);

	int readnumber(String s, int * nextpos);

	//bool getVerse(String book, int chapter, int verse, String* verse_text);
	int getBookIndex(const char* book);
	int getBookIndex(const char* ref, const char* book, const char* verses);
};

#endif