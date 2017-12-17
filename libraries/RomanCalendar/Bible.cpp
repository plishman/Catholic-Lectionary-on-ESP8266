//#include "stdafx.h"
#include <string.h>
#include "Bible.h"
#include "BibleVerse.h"
#include "LinkedList.h"

const char* const Bible::books[73] = {
	"Genesis", "Exodus", "Leviticus", "Numbers", "Deuteronomy", "Joshua", "Judges", "Ruth", "1 Samuel", "2 Samuel", 
	"1 Kings", "2 Kings", "1 Chronicles", "2 Chronicles", "Ezra", "Nehemiah", /*"Tobit", "Judith",*/ "Esther", /*"1 Maccabees", "2 Maccabees" */ "Job", "Psalm", "Proverbs", 
	"Ecclesiastes", "Song of Solomon", /*"Wisdom", "Sirach",*/ "Isaiah", "Jeremiah", "Lamentations", /*"Baruch",*/ "Ezekiel", "Daniel", "Hosea", "Joel", 
	"Amos", "Obadiah", "Jonah", "Micah", "Nahum", "Habakkuk", "Zephaniah", "Haggai", "Zechariah", "Malachi", 
	"Matthew", "Mark", "Luke", "John", "Acts", "Romans", "1 Corinthians", "2 Corinthians", "Galatians", "Ephesians", 
	"Philippians", "Colossians", "1 Thessalonians", "2 Thessalonians", "1 Timothy", "2 Timothy", "Titus", "Philemon", 
	"Hebrews", "James", "1 Peter", "2 Peter", "1 John", "2 John", "3 John", "Jude", "Revelation",
	"Judith", "Wisdom", "Tobit", "Sirach", "Baruch", "1 Maccabees", "2 Maccabees"
};

//"Tob", "Judith"|"1 Macc", "2 Macc", | "Wis", "Sir", | "Bar",

const char* const Bible::books_shortnames[73] = {
	"Gen", "Exod", "Lev", "Num", "Deut", "Josh", "Judg", "Ruth", "1 Sam", "2 Sam",
	"1 Kgs", "2 Kgs", "1 Chr", "2 Chr", "Ezra", "Neh", "Esth", "Job", "Ps", "Prov", 
	"Eccl", "Cant"/*Song of Songs (canticles)*/, "Isa", "Jer", "Lam", "Ezek", "Dan", "Hos", "Joel", 
	"Amos", "Obad", "Jon", "Mic", "Nah", "Hab",	"Zeph", "Hag", "Zech", "Mal", 
	"Matt", "Mark", "Luke", "John", "Acts", "Rom", "1 Cor", "2 Cor", "Gal", "Eph", 
	"Phil", "Col", "1 Thess", "2 Thess", "1 Tim", "2 Tim", "Titus", "Phlm", 
	"Heb", "Jas", "1 Pet", "2 Pet", "1 John", "2 John", "3 John", "Jude", "Rev",
	"Judith", "Wis", "Tob", "Sir", "Bar", "1 Macc", "2 Macc"
};

const int Bible::books_chaptercounts[73] = {
	50, 40, 27, 36, 34, 24, 21, 4, 31, 24, 22, 25, 29, 36, 10, 13, 10, 42, 150, 
	31, 12, 8, 66, 52, 5, 48, 12, 14, 3, 9, 1, 4, 7, 3, 3, 3, 2, 14, 4, 28, 16, 24,
	21, 28, 16, 16, 13, 6, 6, 4, 4, 5, 3, 6, 4, 3, 1, 13, 5, 5, 3, 5, 1, 1, 1, 22,
	16, 19, 14, 51, 6, 16, 15
};

/*
const char* const Bible::books_with_apocrypha[73] = {
	"Genesis", "Exodus", "Leviticus", "Numbers", "Deuteronomy", "Joshua", "Judges", "Ruth", "1 Samuel", "2 Samuel", 
	"1 Kings", "2 Kings", "1 Chronicles", "2 Chronicles", "Ezra", "Nehemiah", "Tobit", "Judith", "Esther", "1 Maccabees", 
	"2 Maccabees", "Job", "Psalms", "Proverbs", "Ecclesiastes", "Song of Songs", "Wisdom", "Sirach", "Isaiah", "Jeremiah", 
	"Lamentations", "Baruch", "Ezekiel", "Daniel", "Hosea", "Joel", "Amos", "Obadiah", "Jonah", "Micah", "Nahum", "Habakkuk", 
	"Zephaniah", "Haggai", "Zechariah", "Malachi", "Matthew", "Mark", "Luke", "John", "Acts", "Romans", 
	"1 Corinthians", "2 Corinthians", "Galatians", "Ephesians", "Philippians", "Colossians", "1 Thessalonians", "2 Thessalonians", 
	"1 Timothy", "2 Timothy", "Titus", "Philemon", "Hebrews", "James", "1 Peter", "2 Peter", "1 John", "2 John", "3 John", "Jude", 
	"Revelation"
};
*/
//const char* const Bible::books_shortnames_with_apocrypha[73] = {
//	"Gen", "Exod", "Lev", "Num", "Deut", "Josh", "Judg", "Ruth", "1 Sam", "2 Sam",
//	"1 Kgs", "2 Kgs", "1 Chr", "2 Chr", "Ezra", "Neh", "Tob", "Judith", "Esth", "1 Macc",
//	"2 Macc", "Job", "Ps", "Prov", "Eccl", "Cant"/*Song of Songs (canticles)*/, "Wis", "Sir", "Isa", "Jer",
//	"Lam", "Bar", "Ezek", "Dan", "Hos", "Joel", "Amos", "Obad", "Jon", "Mic", "Nah", "Hab",
//	"Zeph", "Hag", "Zech", "Mal", "Matt", "Mark", "Luke", "John", "Acts", "Rom",
//	"1 Cor", "2 Cor", "Gal", "Eph", "Phil", "Col", "1 Thess", "2 Thess",
//	"1 Tim", "2 Tim", "Titus", "Phlm", "Heb", "Jas", "1 Pet", "2 Pet", "1 John", "2 John", "3 John", "Jude",
//	"Rev"
//};
/*
const int Bible::books_chaptercounts_with_apocrypha[73] = {
	50, 40, 27, 36, 34, 24, 21, 4, 31, 24, 22, 25, 29, 36, 10, 13, 14, 16, 10, 16,
	15, 42, 150, 31, 12, 8, 19, 51, 66, 52, 5, 6, 48, 14, 14, 4, 9, 1, 4, 7, 3, 3,
	3, 2, 14, 3, 28, 16, 24, 21, 28, 16, 16, 13, 6, 6, 4, 4, 5, 3, 6, 4, 3, 1, 13,
	5, 5, 3, 5, 1, 1, 1, 22
};
*/

Bible::Bible(I18n* i)
{
	I2CSerial.println("Bible::Bible()");
	
	_I18n = i;
	_bibleverse = new BibleVerse(_I18n);
	
	_book_count = 73; //_bibleverse->_book_count;
	
	//I2CSerial.println("book count is " + String(_book_count));
	
	//if (_book_count == 66) {
	//	books = books_no_apocrypha;
	//	books_shortnames = books_shortnames_no_apocrypha;
	//	books_chaptercounts = books_chaptercounts_no_apocrypha;
	//} 
	//else {
	//	books = books_with_apocrypha;	// has apocrypha
	//	books_shortnames = books_shortnames_with_apocrypha;	
	//	books_chaptercounts = books_chaptercounts_with_apocrypha;
	//}
}

Bible::~Bible(void)
{
	refsList.clear();
}

bool Bible::get(String refs) {
	if (refs == NULL) {
		I2CSerial.println("Bible::get() refs is null");
		return false;
	}
	
	int book_index = -1;
	int start_chapter;
	int end_chapter;
	int startpos = 0;
	int start_verse;
	int end_verse;
	int start_first_sentence;
	int start_last_sentence;
	int end_first_sentence;
	int end_last_sentence;
	// need to handle: Matt >>9:35-10:1<<, 5a, 6-8
	refsList.clear();

	bool bResult;
	int len = refs.length();

	bool b_use_same_chapter = false;


	// reference contains at least one of book chapter:verse[-end verse]
	bResult = expect_book(refs, &startpos, &book_index);
	//if this book has only one chapter, the chapter may not be specified in the reference, so set it to 1
	if (book_index == -1) return false; // no book was found on first pass

	expect(refs, " ", &startpos); // skip optional space

	bResult = expect_chapter(refs, &startpos, &start_chapter);

	if (!bResult && books_chaptercounts[book_index] == 1) { // chapter is sometimes not specified for books with only one chapter
		start_chapter = 1;
	}
	else if (!bResult) { // more than one chapter in the selected book, but no chapter specified
		return false;
	}

	bResult = parse_verse_range(refs, &startpos, &start_chapter, &end_chapter, &start_verse, &end_verse, &start_first_sentence, &start_last_sentence, &end_first_sentence, &end_last_sentence);

	if (!bResult) {
		return false;
	}
	else {
		add_reference(refs, book_index, start_chapter, end_chapter, start_verse, end_verse, start_first_sentence, start_last_sentence, end_first_sentence, end_last_sentence);
	}

	while (startpos < len && bResult && (expect(refs, ",", &startpos) || 
											expect(refs, "+", &startpos) || 
											expect(refs, ";", &startpos) || 
											expect(refs, " or ", &startpos))) {
		expect(refs, " ", &startpos); //skip optional space

		if (expect_book(refs, &startpos, &book_index)) expect(refs, " ", &startpos); // if no book specified, assume refers to the same book

		expect_chapter(refs, &startpos, &start_chapter); // if no chapter specified, assume refers to the same chapter

		bResult = parse_verse_range(refs, &startpos, &start_chapter, &end_chapter, &start_verse, &end_verse, &start_first_sentence, &start_last_sentence, &end_first_sentence, &end_last_sentence);
		if (bResult) {
			add_reference(refs, book_index, start_chapter, end_chapter, start_verse, end_verse, start_first_sentence, start_last_sentence, end_first_sentence, end_last_sentence);
		}
	}

	return true;
}

void Bible::dump_refs() {
	Ref* r;
	
	int i = 0;
	r = refsList.get(i);

	while (r != NULL) {

		I2CSerial.printf("%s", r->refs.c_str());
		if (r->start_verse == r->end_verse && r->start_chapter == r->end_chapter) {
			I2CSerial.printf("\t%s, %d:%d%s\n", books[r->book_index], r->start_chapter, r->start_verse, sentence_ref(r->start_first_sentence, r->start_last_sentence).c_str());
		}
		else {
			if (r->start_chapter == r->end_chapter) {
				I2CSerial.printf("\t%s, %d:%d%s-%d%s\n", books[r->book_index],
					r->start_chapter,
					r->start_verse,
					sentence_ref(r->start_first_sentence, r->start_last_sentence).c_str(),
					r->end_verse,
					sentence_ref(r->end_first_sentence, r->end_last_sentence).c_str());
			}
			else {
				I2CSerial.printf("\t%s, %d:%d%s-%d:%d%s\n", books[r->book_index],
					r->start_chapter,
					r->start_verse,
					sentence_ref(r->start_first_sentence, r->start_last_sentence).c_str(),
					r->end_chapter,
					r->end_verse,
					sentence_ref(r->end_first_sentence, r->end_last_sentence).c_str());
			}
		}

		i++;
		r = refsList.get(i);
		//I2CSerial.println(".");
	} 
	//I2CSerial.println("finished");
}

String Bible::sentence_ref(int from, int to) {
	String s = "abcdefghijklmnopqrstuvwxyz";

	if (from == -1 && to == -1) return "";

	String r;

	if (from > -1 && to == -1) {
		r = s.substring(from, from + 1);
	}
	else if (from == -1 && to > -1) {
		r = s.substring(to, to + 1);
	}
	else  if (from > -1 && to > -1) {
		for (int i = from; i <= to; i++) {
			r += s.substring(i, i + 1);
		}
	}

	return r;
}

void Bible::add_reference(String refs, int book_index, 
	int start_chapter, int end_chapter,
	int start_verse, int end_verse,
	int start_first_sentence, int start_last_sentence,
	int end_first_sentence, int end_last_sentence) {

	Ref* r = new Ref();
	r->book_index = book_index;
	r->start_chapter = start_chapter;
	r->end_chapter = end_chapter;
	r->start_verse = start_verse;
	r->start_first_sentence = start_first_sentence;
	r->start_last_sentence = start_last_sentence;
	r->end_verse = end_verse;
	r->end_first_sentence = end_first_sentence;
	r->end_last_sentence = end_last_sentence;
	r->refs = refs;
	r->book_count = _book_count;
	refsList.add(r);
}

bool Bible::is_book(String refs, int startpos) {
	int book_index = 0;
	return expect_book(refs, &startpos, &book_index); // like expect_book, but doesn't change the startpos or book_index variables
}

bool Bible::expect_book(String refs, int* startpos, int* book_index) {
	int len;
	String book;

	bool bFound = false;
	int refs_start = 0;
	int bi = 0;

	while (!bFound && bi < _book_count) {
		book = books[bi];	// try the long name
		len = book.length();

		refs_start = refs.indexOf(book, *startpos);
		
		if (refs_start == -1) {
			book = books_shortnames[bi];// try the short name - important to try the long name first, since the short name will generally match first
			len = book.length();		// even if the long name is used, since the short name is usually a substring of the long name
			refs_start = refs.indexOf(book, *startpos);
		}

		if (refs_start == 0) {
			bFound = true;
		}
		else {
			bi++;
		}
	}

	if (bFound) {
		*startpos = refs_start + len;
		*book_index = bi; // leaves *book_index and *startpos unchanged if a book name was not found
		expect(refs, "/Qoh", startpos); // special case for Ecclesiates - sometimes the short name is Eccl/Qoh (qoholeth), sometimes just Eccl
	}

	return bFound;
}

bool Bible::is_chapter(String refs, int startpos) {
	int chapter;
	return expect_chapter(refs, &startpos, &chapter); // like expect_chapter, but doesn't update either the chapter variable of the startpos variable
}

bool Bible::expect_chapter(String refs, int* startpos, int* chapter) {
	int sp = *startpos; // preserve startpos until we know if the colon is also present (as well as the number)
	int ch = readnumber(refs, &sp); // expect chapter number

	if (ch == 0) return false;

	bool bResult = expect(refs, ":", &sp);	    // followed by a colon :

	if (bResult) {
		*chapter = ch;
		*startpos = sp;
		expect(refs, " ", startpos); // skip any trailing space
	} // preserves startpos and chapter variables if a new chapter <chapter>: is not found
	
	return bResult;
}

bool Bible::parse_verse_range(String refs, int* startpos, 
				  int* start_chapter, int* end_chapter,
				  int* start_verse, int* end_verse, 
				  int* start_first_sentence, int* start_last_sentence,
				  int* end_first_sentence, int* end_last_sentence) {
	// on entry, start_chapter is expected to have been read and set by the Bible::get function.
	bool bResult;

	*end_chapter = *start_chapter; // make sure end_chapter is initialized. If it is not found in the end reference, it will be assumed to be the same as the start chapter

	bResult = parse_verse(refs, startpos, start_chapter, start_verse, start_first_sentence, start_last_sentence);  // followed by either one or more letters, in order (eg. abc)
	
	if (!bResult) return false; // need at least one verse, or <start verse>-<end-verse>

	bResult = expect(refs, "-", startpos); // looking to see if it specifies a verse range separated by -
			
	if (bResult) {
		bResult = parse_verse(refs, startpos, end_chapter, end_verse, end_first_sentence, end_last_sentence); // verse range, look for end verse in range
		if (*end_first_sentence != -1 && *end_last_sentence == -1) {
			//in this case swap end_first_sentence and start_first_sentence, since references like 10:12-15c mean take all sentences *up to* c in verse 15, rather than c..end of chapter
			int t = *end_first_sentence;
			*end_first_sentence = *end_last_sentence;
			*end_last_sentence = t;
		}
		
		return true;
	} 
	//otherwise
	*end_verse = *start_verse; // otherwise set end verse to be the same as start verse
	*end_first_sentence = *start_first_sentence;
	*end_last_sentence = *start_last_sentence;
	return true;
}

bool Bible::parse_verse(String refs, int* startpos, int* chapter, int* verse, int* start_sentence, int* end_sentence) {
	bool bResult= true;

	expect_chapter(refs, startpos, chapter); // optional chapter ref

	*verse = readnumber(refs, startpos);

	if (*verse == 0) return false;

	get_sentences(refs, startpos, start_sentence, end_sentence); // sentence references are optional, if not present, no problem - though it does not distinguish malformed references at this point

	return true;
}

bool Bible::expect(String s, String c, int* pos) {
	if (s.indexOf(c, *pos) == *pos) {
		(*pos)+= c.length();
		return true;
	}
	return false;
}

bool Bible::get_sentences(String s, int* startpos, int* start_sentence, int* end_sentence) { 
	String letters = "abcdefghijklmnopqrstuvwxyz";										// *end_sentence == -1 for all sentences from *start_sentence
	int pos = *startpos;

	*start_sentence = -1; // -1 means "use all sentences in this verse" (sentence references abc etc. are not specified)
	*end_sentence = -1;

	if (*startpos >= s.length()) {
		return false;
	}

	char letter = s.charAt(pos);
	*start_sentence = letters.indexOf(letter);
	*end_sentence = -1;
	if (*start_sentence != -1) { // found start letter, eg a
		//look for more letters, eg. bcd
		
		int last_end_sentence;
		do {
			pos++;
			letter = s.charAt(pos);
			last_end_sentence = *end_sentence; // will be -1 to start with
			*end_sentence = letters.indexOf(letter);
		} while (*end_sentence != -1 && pos < s.length());

		//here last_end_sentence is either the last sentence number to return, or -1 if no more letters were found.
		*end_sentence = last_end_sentence; // will take the last value of *end_sentence before it reached -1, or -1 if no further letters were found
	}

	*startpos = pos;

	//I2CSerial.printf("Bible::get_sentences() start_sentence=%d, end_sentence=%d\n", *start_sentence, *end_sentence);
	
	return true;
}

int Bible::readnumber(String s, int* nextpos) {
	String digits = "0123456789";
	String n;

	while (*nextpos < s.length() && digits.indexOf(s.charAt(*nextpos)) != -1) {
		n += s.charAt((*nextpos)++);
	}

	return n.toInt();
}

/*
bool Bible::getVerse(String book, int chapter, int verse, String* verse_text) {
	BibleVerse b;
	int book_index = 0;
	int startpos = 0;

	if (expect_book(book, &startpos, &book_index)) {
		return b.get(book_index, chapter, verse, verse_text);
	}

	return false;
}
*/

/*
gets the number of a bible book (0-72) from a bible reference using short names, eg 2 Pet 1:2-7
returns a pointer to the string containing the name of the book, and a pointer to the start of the reference string
containing the chapter:verses part of the reference
*/
int Bible::getBookIndex(const char* book) {
	const char* b = NULL;
	const char* v = NULL;

	return getBookIndex(book, b, v);
}

int Bible::getBookIndex(const char* ref, const char* book, const char* verses)
{
	int len;

	bool bFound = false;
	int bookIndex = 0;

	while (!bFound && bookIndex < _book_count) {
		book = books_shortnames[bookIndex];
		len = strlen(book);

		if (strncmp(book, ref, len) == 0) {
			bFound = true;
		}
		else {
			bookIndex++;
		}
	}

	if (bFound) {
		verses = ref + len; 
		return bookIndex;
	}

	return 0;
}

String get_delimeter(String s, int startpos)
{
	return String();
}
