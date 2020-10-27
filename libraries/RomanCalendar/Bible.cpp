//#include "stdafx.h"
extern "C" {
#include "user_interface.h"
}

#include <string.h>
#include "Bible.h"
#include "BibleVerse.h"
#include "LinkedList.h"

//const char* const Bible::books[73] = {
//	"Genesis", "Exodus", "Leviticus", "Numbers", "Deuteronomy", "Joshua", "Judges", "Ruth", "1 Samuel", "2 Samuel", 
//	"1 Kings", "2 Kings", "1 Chronicles", "2 Chronicles", "Ezra", "Nehemiah", /*"Tobit", "Judith",*/ "Esther", /*"1 Maccabees", "2 Maccabees" */ "Job", "Psalm", "Proverbs", 
//	"Ecclesiastes", "Song of Solomon", /*"Wisdom", "Sirach",*/ "Isaiah", "Jeremiah", "Lamentations", /*"Baruch",*/ "Ezekiel", "Daniel", "Hosea", "Joel", 
//	"Amos", "Obadiah", "Jonah", "Micah", "Nahum", "Habakkuk", "Zephaniah", "Haggai", "Zechariah", "Malachi", 
//	"Matthew", "Mark", "Luke", "John", "Acts", "Romans", "1 Corinthians", "2 Corinthians", "Galatians", "Ephesians", 
//	"Philippians", "Colossians", "1 Thessalonians", "2 Thessalonians", "1 Timothy", "2 Timothy", "Titus", "Philemon", 
//	"Hebrews", "James", "1 Peter", "2 Peter", "1 John", "2 John", "3 John", "Jude", "Revelation",
//	"Judith", "Wisdom", "Tobit", "Sirach", "Baruch", "1 Maccabees", "2 Maccabees"
//};
//
////"Tob", "Judith"|"1 Macc", "2 Macc", | "Wis", "Sir", | "Bar",
//
//const char* const Bible::books_shortnames[73] = {
//	"Gen", "Exod", "Lev", "Num", "Deut", "Josh", "Judg", "Ruth", "1 Sam", "2 Sam",
//	"1 Kgs", "2 Kgs", "1 Chr", "2 Chr", "Ezra", "Neh", "Esth", "Job", "Ps", "Prov", 
//	"Eccl", "Cant"/*Song of Songs (canticles)*/, "Isa", "Jer", "Lam", "Ezek", "Dan", "Hos", "Joel", 
//	"Amos", "Obad", "Jon", "Mic", "Nah", "Hab",	"Zeph", "Hag", "Zech", "Mal", 
//	"Matt", "Mark", "Luke", "John", "Acts", "Rom", "1 Cor", "2 Cor", "Gal", "Eph", 
//	"Phil", "Col", "1 Thess", "2 Thess", "1 Tim", "2 Tim", "Titus", "Phlm", 
//	"Heb", "Jas", "1 Pet", "2 Pet", "1 John", "2 John", "3 John", "Jude", "Rev",
//	"Judith", "Wis", "Tob", "Sir", "Bar", "1 Macc", "2 Macc"
//};

const int Bible::books_chaptercounts[73] PROGMEM = {
	50, 40, 27, 36, 34, 24, 21, 4, 31, 24, 22, 25, 29, 36, 10, 13, 10, 42, 150, 
	31, 12, 8, 66, 52, 5, 48, 12, 14, 3, 9, 1, 4, 7, 3, 3, 3, 2, 14, 4, 28, 16, 24,
	21, 28, 16, 16, 13, 6, 6, 4, 4, 5, 3, 6, 4, 3, 1, 13, 5, 5, 3, 5, 1, 1, 1, 22,
	16, 19, 14, 51, 6, 16, 15
};


	const char Bible::s_00[] PROGMEM = "Genesis";
	const char Bible::s_01[] PROGMEM = "Exodus";
	const char Bible::s_02[] PROGMEM = "Leviticus";
	const char Bible::s_03[] PROGMEM = "Numbers";
	const char Bible::s_04[] PROGMEM = "Deuteronomy";
	const char Bible::s_05[] PROGMEM = "Joshua";
	const char Bible::s_06[] PROGMEM = "Judges";
	const char Bible::s_07[] PROGMEM = "Ruth";
	const char Bible::s_08[] PROGMEM = "1 Samuel";
	const char Bible::s_09[] PROGMEM = "2 Samuel";
	const char Bible::s_10[] PROGMEM = "1 Kings";
	const char Bible::s_11[] PROGMEM = "2 Kings";
	const char Bible::s_12[] PROGMEM = "1 Chronicles";
	const char Bible::s_13[] PROGMEM = "2 Chronicles";
	const char Bible::s_14[] PROGMEM = "Ezra";
	const char Bible::s_15[] PROGMEM = "Nehemiah";/*"Tobit"; "Judith";*/ 
	const char Bible::s_16[] PROGMEM = "Esther";/*"1 Maccabees"; "2 Maccabees" */ 
	const char Bible::s_17[] PROGMEM = "Job";
	const char Bible::s_18[] PROGMEM = "Psalm";
	const char Bible::s_19[] PROGMEM = "Proverbs";
	const char Bible::s_20[] PROGMEM = "Ecclesiastes";
	const char Bible::s_21[] PROGMEM = "Song of Solomon";/*"Wisdom"; "Sirach";*/ 
	const char Bible::s_22[] PROGMEM = "Isaiah";
	const char Bible::s_23[] PROGMEM = "Jeremiah";
	const char Bible::s_24[] PROGMEM = "Lamentations";/*"Baruch";*/ 
	const char Bible::s_25[] PROGMEM = "Ezekiel";
	const char Bible::s_26[] PROGMEM = "Daniel";
	const char Bible::s_27[] PROGMEM = "Hosea";
	const char Bible::s_28[] PROGMEM = "Joel";
	const char Bible::s_29[] PROGMEM = "Amos";
	const char Bible::s_30[] PROGMEM = "Obadiah";
	const char Bible::s_31[] PROGMEM = "Jonah";
	const char Bible::s_32[] PROGMEM = "Micah";
	const char Bible::s_33[] PROGMEM = "Nahum";
	const char Bible::s_34[] PROGMEM = "Habakkuk";
	const char Bible::s_35[] PROGMEM = "Zephaniah";
	const char Bible::s_36[] PROGMEM = "Haggai";
	const char Bible::s_37[] PROGMEM = "Zechariah";
	const char Bible::s_38[] PROGMEM = "Malachi";
	const char Bible::s_39[] PROGMEM = "Matthew";
	const char Bible::s_40[] PROGMEM = "Mark";
	const char Bible::s_41[] PROGMEM = "Luke";
	const char Bible::s_42[] PROGMEM = "John";
	const char Bible::s_43[] PROGMEM = "Acts";
	const char Bible::s_44[] PROGMEM = "Romans";
	const char Bible::s_45[] PROGMEM = "1 Corinthians";
	const char Bible::s_46[] PROGMEM = "2 Corinthians";
	const char Bible::s_47[] PROGMEM = "Galatians";
	const char Bible::s_48[] PROGMEM = "Ephesians";
	const char Bible::s_49[] PROGMEM = "Philippians";
	const char Bible::s_50[] PROGMEM = "Colossians";
	const char Bible::s_51[] PROGMEM = "1 Thessalonians";
	const char Bible::s_52[] PROGMEM = "2 Thessalonians";
	const char Bible::s_53[] PROGMEM = "1 Timothy";
	const char Bible::s_54[] PROGMEM = "2 Timothy";
	const char Bible::s_55[] PROGMEM = "Titus";
	const char Bible::s_56[] PROGMEM = "Philemon";
	const char Bible::s_57[] PROGMEM = "Hebrews";
	const char Bible::s_58[] PROGMEM = "James";
	const char Bible::s_59[] PROGMEM = "1 Peter";
	const char Bible::s_60[] PROGMEM = "2 Peter";
	const char Bible::s_61[] PROGMEM = "1 John";
	const char Bible::s_62[] PROGMEM = "2 John";
	const char Bible::s_63[] PROGMEM = "3 John";
	const char Bible::s_64[] PROGMEM = "Jude";
	const char Bible::s_65[] PROGMEM = "Revelation";
	const char Bible::s_66[] PROGMEM = "Judith";
	const char Bible::s_67[] PROGMEM = "Wisdom";
	const char Bible::s_68[] PROGMEM = "Tobit";
	const char Bible::s_69[] PROGMEM = "Sirach";
	const char Bible::s_70[] PROGMEM = "Baruch";
	const char Bible::s_71[] PROGMEM = "1 Maccabees";
	const char Bible::s_72[] PROGMEM = "2 Maccabees";

	const char* const Bible::books[73] PROGMEM = {
		s_00, s_01, s_02, s_03, s_04, s_05, s_06, s_07, s_08, s_09, s_10, s_11, s_12, s_13, s_14, s_15,
		s_16, s_17, s_18, s_19, s_20, s_21, s_22, s_23, s_24, s_25, s_26, s_27, s_28, s_29, s_30, s_31, 
		s_32, s_33, s_34, s_35, s_36, s_37, s_38, s_39, s_40, s_41, s_42, s_43, s_44, s_45, s_46, s_47, 
		s_48, s_49, s_50, s_51, s_52, s_53, s_54, s_55, s_56, s_57, s_58, s_59, s_60, s_61, s_62, s_63, 
		s_64, s_65, s_66, s_67, s_68, s_69, s_70, s_71, s_72
	};

	const char Bible::s2_00[] PROGMEM = "Gen";
	const char Bible::s2_01[] PROGMEM = "Exod";
	const char Bible::s2_02[] PROGMEM = "Lev";
	const char Bible::s2_03[] PROGMEM = "Num";
	const char Bible::s2_04[] PROGMEM = "Deut";
	const char Bible::s2_05[] PROGMEM = "Josh";
	const char Bible::s2_06[] PROGMEM = "Judg";
	const char Bible::s2_07[] PROGMEM = "Ruth";
	const char Bible::s2_08[] PROGMEM = "1 Sam";
	const char Bible::s2_09[] PROGMEM = "2 Sam";
	const char Bible::s2_10[] PROGMEM = "1 Kgs";
	const char Bible::s2_11[] PROGMEM = "2 Kgs";
	const char Bible::s2_12[] PROGMEM = "1 Chr";
	const char Bible::s2_13[] PROGMEM = "2 Chr";
	const char Bible::s2_14[] PROGMEM = "Ezra";
	const char Bible::s2_15[] PROGMEM = "Neh";
	const char Bible::s2_16[] PROGMEM = "Esth";
	const char Bible::s2_17[] PROGMEM = "Job";
	const char Bible::s2_18[] PROGMEM = "Ps";
	const char Bible::s2_19[] PROGMEM = "Prov";
	const char Bible::s2_20[] PROGMEM = "Eccl";
	const char Bible::s2_21[] PROGMEM = "Cant"; /*Song of Songs (canticles)*/
	const char Bible::s2_22[] PROGMEM = "Isa";
	const char Bible::s2_23[] PROGMEM = "Jer";
	const char Bible::s2_24[] PROGMEM = "Lam";
	const char Bible::s2_25[] PROGMEM = "Ezek";
	const char Bible::s2_26[] PROGMEM = "Dan";
	const char Bible::s2_27[] PROGMEM = "Hos";
	const char Bible::s2_28[] PROGMEM = "Joel";
	const char Bible::s2_29[] PROGMEM = "Amos";
	const char Bible::s2_30[] PROGMEM = "Obad";
	const char Bible::s2_31[] PROGMEM = "Jon";
	const char Bible::s2_32[] PROGMEM = "Mic";
	const char Bible::s2_33[] PROGMEM = "Nah";
	const char Bible::s2_34[] PROGMEM = "Hab";
	const char Bible::s2_35[] PROGMEM = "Zeph";
	const char Bible::s2_36[] PROGMEM = "Hag";
	const char Bible::s2_37[] PROGMEM = "Zech";
	const char Bible::s2_38[] PROGMEM = "Mal";
	const char Bible::s2_39[] PROGMEM = "Matt";
	const char Bible::s2_40[] PROGMEM = "Mark";
	const char Bible::s2_41[] PROGMEM = "Luke";
	const char Bible::s2_42[] PROGMEM = "John";
	const char Bible::s2_43[] PROGMEM = "Acts";
	const char Bible::s2_44[] PROGMEM = "Rom";
	const char Bible::s2_45[] PROGMEM = "1 Cor";
	const char Bible::s2_46[] PROGMEM = "2 Cor";
	const char Bible::s2_47[] PROGMEM = "Gal";
	const char Bible::s2_48[] PROGMEM = "Eph";
	const char Bible::s2_49[] PROGMEM = "Phil";
	const char Bible::s2_50[] PROGMEM = "Col";
	const char Bible::s2_51[] PROGMEM = "1 Thess";
	const char Bible::s2_52[] PROGMEM = "2 Thess";
	const char Bible::s2_53[] PROGMEM = "1 Tim";
	const char Bible::s2_54[] PROGMEM = "2 Tim";
	const char Bible::s2_55[] PROGMEM = "Titus";
	const char Bible::s2_56[] PROGMEM = "Phlm";
	const char Bible::s2_57[] PROGMEM = "Heb";
	const char Bible::s2_58[] PROGMEM = "Jas";
	const char Bible::s2_59[] PROGMEM = "1 Pet";
	const char Bible::s2_60[] PROGMEM = "2 Pet";
	const char Bible::s2_61[] PROGMEM = "1 John";
	const char Bible::s2_62[] PROGMEM = "2 John";
	const char Bible::s2_63[] PROGMEM = "3 John";
	const char Bible::s2_64[] PROGMEM = "Jude";
	const char Bible::s2_65[] PROGMEM = "Rev";
	const char Bible::s2_66[] PROGMEM = "Judith";
	const char Bible::s2_67[] PROGMEM = "Wis";
	const char Bible::s2_68[] PROGMEM = "Tob";
	const char Bible::s2_69[] PROGMEM = "Sir";
	const char Bible::s2_70[] PROGMEM = "Bar";
	const char Bible::s2_71[] PROGMEM = "1 Macc";
	const char Bible::s2_72[] PROGMEM = "2 Macc";

	const char* const Bible::books_shortnames[73] PROGMEM = {
		s2_00, s2_01, s2_02, s2_03, s2_04, s2_05, s2_06, s2_07, s2_08, s2_09, s2_10, s2_11, s2_12, s2_13, s2_14, s2_15, 
		s2_16, s2_17, s2_18, s2_19, s2_20, s2_21, s2_22, s2_23, s2_24, s2_25, s2_26, s2_27, s2_28, s2_29, s2_30, s2_31, 
		s2_32, s2_33, s2_34, s2_35, s2_36, s2_37, s2_38, s2_39, s2_40, s2_41, s2_42, s2_43, s2_44, s2_45, s2_46, s2_47, 
		s2_48, s2_49, s2_50, s2_51, s2_52, s2_53, s2_54, s2_55, s2_56, s2_57, s2_58, s2_59, s2_60, s2_61, s2_62, s2_63, 
		s2_64, s2_65, s2_66, s2_67, s2_68, s2_69, s2_70, s2_71, s2_72 
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
	DEBUG_PRT.println(F("Bible::Bible()"));
	
	_I18n = i;
	_bibleverse = new BibleVerse(_I18n);
	
	_book_count = 73; //_bibleverse->_book_count;
	
	//DEBUG_PRT.println("book count is " + String(_book_count));
	
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
	for (int i = 0; i < refsList.size(); i++) { // delete refslist objects stored in linked list, before deleting list itself
		Ref* pRef = refsList.get(i);
		delete pRef;
	}
	
	refsList.clear();
}

bool Bible::get(String refs) {
	if (refs == NULL) {
		DEBUG_PRT.println(F("Bible::get() refs is null"));
		return false;
	}

	uint32_t startfreemem = system_get_free_heap_size();
	DEBUG_PRT.print("Bible::get(): on entry, free mem is:");
	DEBUG_PRT.println(startfreemem);
	
	int book_index = -1;
	int start_chapter;
	int end_chapter;
	int startpos = 0;
	int start_verse;
	int end_verse;
	String start_verse_sentence_range;
	String end_verse_sentence_range;
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

	bResult = parse_verse_range(refs, &startpos, &start_chapter, &end_chapter, &start_verse, &end_verse, &start_verse_sentence_range, &end_verse_sentence_range);

	if (!bResult) {
		return false;
	}
	else {
		if ((startfreemem - add_reference(refs, book_index, start_chapter, end_chapter, start_verse, end_verse, start_verse_sentence_range, end_verse_sentence_range)) > MAX_MEM_BIBLE_REFS) {
			DEBUG_PRT.println("Add Bible refs: below memory floor");
			return true;
		}
	}

	//DEBUG_PRT.printf("Bible::get(): remaining refs=[%s]\n", refs.substring(startpos).c_str());

	while (startpos < len && bResult && (expect(refs, ",", &startpos) || 
											expect(refs, "+", &startpos) || 
											expect(refs, ";", &startpos) || 
											expect(refs, " or ", &startpos))) {
		expect(refs, " ", &startpos); //skip optional space

		
		//DEBUG_PRT.printf("Bible::get(): remaining refs=[%s]", refs.substring(startpos).c_str());

		if (expect_book(refs, &startpos, &book_index)) expect(refs, " ", &startpos); // if no book specified, assume refers to the same book

		//DEBUG_PRT.printf("Bible::get(): remaining refs=[%s]", refs.substring(startpos).c_str());

		expect_chapter(refs, &startpos, &start_chapter); // if no chapter specified, assume refers to the same chapter

		bResult = parse_verse_range(refs, &startpos, &start_chapter, &end_chapter, &start_verse, &end_verse, &start_verse_sentence_range, &end_verse_sentence_range);
		if (bResult) {
			if ((startfreemem - add_reference(refs, book_index, start_chapter, end_chapter, start_verse, end_verse, start_verse_sentence_range, end_verse_sentence_range)) > MAX_MEM_BIBLE_REFS) {
				DEBUG_PRT.println("Add Bible refs: below memory floor");
				return true;
			}
		}
	}

	return true;
}
void Bible::dump_refs() {
	Ref* r;
	
	int i = 0;
	r = refsList.get(i);

	while (r != NULL) {

		DEBUG_PRT.print(r->refs);
		if (r->start_verse == r->end_verse && r->start_chapter == r->end_chapter) {
			//DEBUG_PRT.printf("\t%s, %d:%d%s\n", books[r->book_index], r->start_chapter, r->start_verse, r->start_verse_sentence_range.c_str());
			DEBUG_PRT.print(F("\t"));
			DEBUG_PRT.print(FPSTR(books[r->book_index]));
			DEBUG_PRT.print(F(", "));
			DEBUG_PRT.print(r->start_chapter);
			DEBUG_PRT.print(F(":"));
			DEBUG_PRT.print(r->start_verse);
			DEBUG_PRT.print(r->start_verse_sentence_range);
		}
		else {
			if (r->start_chapter == r->end_chapter) {
				//DEBUG_PRT.printf("\t%s, %d:%d%s-%d%s\n", 

				DEBUG_PRT.print(F("\t"));
				DEBUG_PRT.print(FPSTR(books[r->book_index]));
				DEBUG_PRT.print(F(", "));
				DEBUG_PRT.print(r->start_chapter);
				DEBUG_PRT.print(F(":"));
				DEBUG_PRT.print(r->start_verse);
				DEBUG_PRT.print(r->start_verse_sentence_range);
				DEBUG_PRT.print(F("-"));
				DEBUG_PRT.print(r->end_verse);
				DEBUG_PRT.print(r->end_verse_sentence_range);
			}
			else {
				//DEBUG_PRT.printf("\t%s, %d:%d%s-%d:%d%s\n", 
				DEBUG_PRT.print(F("\t"));
				DEBUG_PRT.print(FPSTR(books[r->book_index]));
				DEBUG_PRT.print(r->start_chapter);
				DEBUG_PRT.print(F(":"));
				DEBUG_PRT.print(r->start_verse);
				DEBUG_PRT.print(r->start_verse_sentence_range);
				DEBUG_PRT.print(F("-"));
				DEBUG_PRT.print(r->end_chapter);
				DEBUG_PRT.print(F(":"));
				DEBUG_PRT.print(r->end_verse);
				DEBUG_PRT.print(r->end_verse_sentence_range);
			}
		}

		DEBUG_PRT.println();

		i++;
		r = refsList.get(i);
		//DEBUG_PRT.println(".");
	} 
	//DEBUG_PRT.println("finished");
}

String Bible::getBookShortName(int booknum) {
	if (booknum >= 0 && booknum < _book_count) {
		return FPSTR(books_shortnames[booknum]);
	}
	return "";
}

bool Bible::getUniqueBookInRefs(int& booknum, int& index) {
	Ref* r;
	
	//index = 0;
	r = refsList.get(index);

	bool bResult = false;
	
	while (r != NULL && bResult == false) {
		//DEBUG_PRT.printf("getUniqueBookInRefs() refs:[%s] refs->book_index[%d] in:booknum=%d in:index=%d\n", r->refs.c_str(), r->book_index, booknum, index);
		
		if (r->book_index != booknum) {
			booknum = r->book_index;
			bResult = true;
		}
		else {
			r = refsList.get(index);
		}
		index++;
	}
	
	//DEBUG_PRT.printf("getUniqueBookInRefs() bResult=%d out:booknum=%d out:index=%d\n", bResult, booknum, index);	
	
	return bResult;
}

/*
void Bible::dump_refs() {
	Ref* r;
	
	int i = 0;
	r = refsList.get(i);

	while (r != NULL) {

		DEBUG_PRT.printf("%s", r->refs.c_str());
		if (r->start_verse == r->end_verse && r->start_chapter == r->end_chapter) {
			DEBUG_PRT.printf("\t%s, %d:%d%s\n", books[r->book_index], r->start_chapter, r->start_verse, sentence_ref(r->start_first_sentence, r->start_last_sentence).c_str());
		}
		else {
			if (r->start_chapter == r->end_chapter) {
				DEBUG_PRT.printf("\t%s, %d:%d%s-%d%s\n", books[r->book_index],
					r->start_chapter,
					r->start_verse,
					sentence_ref(r->start_first_sentence, r->start_last_sentence).c_str(),
					r->end_verse,
					sentence_ref(r->end_first_sentence, r->end_last_sentence).c_str());
			}
			else {
				DEBUG_PRT.printf("\t%s, %d:%d%s-%d:%d%s\n", books[r->book_index],
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
		//DEBUG_PRT.println(".");
	} 
	//DEBUG_PRT.println("finished");
}
*/

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

uint32_t Bible::add_reference(String refs, int book_index, 
	int start_chapter, int end_chapter,
	int start_verse, int end_verse,
	String start_verse_sentence_range, String end_verse_sentence_range) {

	DEBUG_PRT.print(F("Bible::add_reference(): refs="));
	//%s [%d %d:%d - %d:%d]\n", 
	DEBUG_PRT.print(refs);
	DEBUG_PRT.print(F(" ["));
	DEBUG_PRT.print(book_index);
	DEBUG_PRT.print(F(" "));
	DEBUG_PRT.print(start_chapter);
	DEBUG_PRT.print(F(":"));
	DEBUG_PRT.print(start_verse);
	DEBUG_PRT.print(F(" - "));
	DEBUG_PRT.print(end_chapter);
	DEBUG_PRT.print(F(":")); //pll 11-04-2020 corrected missing : in debug output
	DEBUG_PRT.print(end_verse);
	DEBUG_PRT.print(F("] "));

	Ref* r = new Ref();
	r->book_index = book_index;
	r->start_chapter = start_chapter;
	r->end_chapter = end_chapter;
	r->start_verse = start_verse;
	r->start_verse_sentence_range = start_verse_sentence_range;
	r->end_verse = end_verse;
	r->end_verse_sentence_range = end_verse_sentence_range;
	r->refs = refs;
	r->book_count = _book_count;
	refsList.add(r);
	DEBUG_PRT.print(F("freemem = "));
    DEBUG_PRT.println(String(system_get_free_heap_size()));
	DEBUG_PRT.print(F("\n"));

	return system_get_free_heap_size(); //pll 11-04-2020 return available memory so that max usage can be constrained (problem in Easter with very large Bible refs)
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
		book = FPSTR(books[bi]);	// try the long name
		len = book.length();

		refs_start = refs.indexOf(book, *startpos);
		
		if (refs_start == -1) {
			book = FPSTR(books_shortnames[bi]);// try the short name - important to try the long name first, since the short name will generally match first
			len = book.length();		// even if the long name is used, since the short name is usually a substring of the long name
			refs_start = refs.indexOf(book, *startpos);
		}

		if (refs_start == *startpos) {
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
/*
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
*/

bool Bible::parse_verse_range(String refs, int* startpos, 
				  int* start_chapter, int* end_chapter,
				  int* start_verse, int* end_verse, 
				  String* start_verse_sentence_range,
				  String* end_verse_sentence_range) {
	// on entry, start_chapter is expected to have been read and set by the Bible::get function.
	bool bResult;
	*start_verse_sentence_range = "";
	*end_verse_sentence_range = "";
	
	*end_chapter = *start_chapter; // make sure end_chapter is initialized. If it is not found in the end reference, it will be assumed to be the same as the start chapter

	bResult = parse_verse(refs, startpos, start_chapter, start_verse, start_verse_sentence_range);  // followed by either one or more letters, in order (eg. abc)
	
	if (!bResult) return false; // need at least one verse, or <start verse>-<end-verse>

	bResult = expect(refs, "-", startpos); // looking to see if it specifies a verse range separated by -
			
	if (bResult) {
		(*start_verse_sentence_range) += "-"; // indicates from start verse sentence range to end of verse
		(*end_verse_sentence_range) = "-"; // indicates from start of last verse to fragment indicated by letter
		
		bResult = parse_verse(refs, startpos, end_chapter, end_verse, end_verse_sentence_range); // verse range, look for end verse in range		
		return true;
	} 
	//otherwise
	*end_verse = *start_verse; // otherwise set end verse to be the same as start verse
	*end_verse_sentence_range = *start_verse_sentence_range;
	return true;
}

bool Bible::parse_verse(String refs, int* startpos, int* chapter, int* verse, String* sentence_range) {
	bool bResult= true;

	expect_chapter(refs, startpos, chapter); // optional chapter ref

	*verse = readnumber(refs, startpos);

	//DEBUG_PRT.printf("Bible::parse_verse(): refs=[%s], startpos=%d, readnumber returned %d\n", refs.c_str(), *startpos, *verse);

	if (*verse == 0) return false;

	String letters = "abcdefghijklmnopqrstuvwxyz[]";
	
	char letter = refs.charAt(*startpos);
	while (letter != '\0' && letters.indexOf(letter) != -1) {
		if (letter != '[' && letter != ']') {			// PLL-13-04-2019 this will discard brackets [ and ], which indicate optional sentences.
			(*sentence_range) += letter;
		}
		(*startpos)++;
		letter = refs.charAt(*startpos);
	}
	
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

	//DEBUG_PRT.printf("Bible::get_sentences() start_sentence=%d, end_sentence=%d\n", *start_sentence, *end_sentence);
	
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
