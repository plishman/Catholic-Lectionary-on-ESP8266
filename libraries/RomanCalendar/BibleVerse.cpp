#include "Arduino.h"
#include "BibleVerse.h"
extern "C" {
#include "user_interface.h"
}

BibleVerse::BibleVerse(I18n* i)
{
	DEBUG_PRT.println(F("BibleVerse::BibleVerse()"));
	_I18n = i;
	//String filename;
	//get_bible_filename(&filename); // populates the _book_count variable when the bible filename has been determined from the locale
	//DEBUG_PRT.println("+");
}

BibleVerse::~BibleVerse()
{
}

/*
bool BibleVerse::get_bible_filename(String* filename) {
	Csv csv;

	String bibleslist_filename = String("/locales/Bibles.csv");	
	
	File file = _I18n->openFile(bibleslist_filename, FILE_READ);

	if (!file.available()) return false;
		
	String csv_record;
	String lang;
	int pos = 0;
	
	DEBUG_PRT.println("selected lang=" + String(_I18n->I18n_LANGUAGES[_I18n->_locale]));
	
	while (file.available() && lang != _I18n->I18n_LANGUAGES[_I18n->_locale]) {
		csv_record = _I18n->readLine(file);
		pos = 0;
		DEBUG_PRT.println("csv_record: " + csv_record);
		lang = csv.getCsvField(csv_record, &pos);
		DEBUG_PRT.println("loop:lang=" + lang + " pos=" + String(pos));
	} 

	DEBUG_PRT.println("lang=" + lang + " pos=" + String(pos));
	
	_I18n->closeFile(file);
	
	if (lang == _I18n->I18n_LANGUAGES[_I18n->_locale]) {
		*filename = "/Bibles/" + csv.getCsvField(csv_record, &pos);
		DEBUG_PRT.println("filename=" + *filename);
		//String book_count = csv.getCsvField(csv_record, &pos);
		//_book_count = book_count.toInt();
		//DEBUG_PRT.println("book count=" + book_count + " int is " + String(_book_count));
		return true;
	}
	
	return false;
}
*/
String BibleVerse::getBookNameFromBible(int booknum) {
	Csv csv;
	
	String vt = "";
	int nr = 0;
	
	if (get(booknum, 1, 1, &vt, &nr)) {
		int pos = 0;
		return csv.getCsvField(vt, &pos); // first csv string should be book name in the language of the currently selected Bible
	}
	
	return "";
}

bool BibleVerse::get(int book, int chapter, int verse, String* verse_text, int* numRecords) {
    File file;
	Csv csv;
	
	DEBUG_PRT.println("BibleVerse::get() " + String(book) + " " + String(chapter) + ":" + String(verse));
	
	book++;

	if (_I18n == NULL) {
		DEBUG_PRT.println(F("BibleVerse::get(): _I18n is null"));
		return false;
	} 

	if (!(_I18n->configparams.have_config)) {
		DEBUG_PRT.println(F("BibleVerse::get(): No config"));
		return false;
	}

	String bookstr = book < 10 ? "0" + String(book) : String(book);
	
	String chapterstr = chapter < 10 ? "0" + String(chapter) : String(chapter);
	chapterstr = chapter < 100 ? "0" + chapterstr : chapterstr;

	String edb_filename = bookstr + "_" + chapterstr + ".edb";
	String edb_filedir = (_I18n->configparams.bible_filename.substring(0, _I18n->configparams.bible_filename.lastIndexOf(".")));

	String index_filename = (_I18n->configparams.bible_filename.substring(0, _I18n->configparams.bible_filename.lastIndexOf(".")));
	index_filename += "/Bible/" + edb_filename;
/*	
	//String index_filename = String("/Bibles/") + String(_I18n->I18n_LANGUAGES[_I18n->_locale]) + "/Bible/" + String(book) + "/" + String(chapter) + "/" + String(verse);	
	String index_filename = (_I18n->_bible_filename.substring(0, _I18n->_bible_filename.lastIndexOf(".")));
	index_filename += "/Bible/" + String(book) + "/" + String(chapter) + "/" + String(verse);	
*/	
	DEBUG_PRT.println("Index filename is " + index_filename);
	
	::dbFile = _I18n->openFile(index_filename, FILE_READ);

	if (!::dbFile) {
		DEBUG_PRT.println("could not open " + index_filename);
		return false;
	}
	
	String fileOffsetStr;
	
	Serial.print("Opening verse offset table... ");
	EDB_Status result = EDB_OK;
	
	result = ::db.open(0);
	if (result != EDB_OK) {
		DEBUG_PRT.println(F("ERROR"));
		DEBUG_PRT.print(F("Did not find database in the file "));
		DEBUG_PRT.println(edb_filename);
		_I18n->closeFile(::dbFile);		
		return false;
	}

	VerseEntry verseentry = { 0 };
	
	result = EDB_OK;
	result = ::db.readRec(verse, EDB_REC verseentry);
	if (result != EDB_OK)
	{
		::printDbError(result);
		_I18n->closeFile(::dbFile);		
		return false;
	}

	if (verseentry.fragment_count > 31) {
		_I18n->closeFile(::dbFile);
		DEBUG_PRT.println(F("Malformed database record: fragment_count for verse > 31"));
		return false;
	}	
	
	for (int i = 0; i < verseentry.fragment_count; i++) {
		fileOffsetStr += String(verseentry.csv_offsets[i]);
		if (i < (verseentry.fragment_count - 1)) fileOffsetStr += ",";		
	}


/*	
	while(file.available()) {
		String f = _I18n->readLine(file);
		if (f.length() == 0) break;
		fileOffsetStr += f;
		fileOffsetStr.trim();
		if (file.available()) {
			fileOffsetStr += ",";
		}
		//DEBUG_PRT.print("fileOffsetStr=" + fileOffsetStr);
	}
*/		
	_I18n->closeFile(::dbFile);

	if (fileOffsetStr.length() == 0) {
		DEBUG_PRT.println(F("fileOffset not found"));
		return false;
	}
	
	String bible_filename = _I18n->configparams.bible_filename;
	
	//if (!get_bible_filename(&bible_filename)) return false;

	DEBUG_PRT.println("bible filename is " + bible_filename);

	if (book < 1 || book > _book_count) return false; // _book_count is set by get_bible_filename, since it's part of the csv record that associates bible filenames with languages (non-apocrypha versions have 66 books, otherwise 73)

	file = _I18n->openFile(bible_filename, FILE_READ);

	if (!file){
		DEBUG_PRT.println(F("BibleVerse::get() can't open bible"));
		return false;
	} 
	wdt_reset();
	*verse_text = "";
	*numRecords = 0;
	int pos = 0;
	int len = fileOffsetStr.length();
	String fragment = "";
	DEBUG_PRT.print(F("fileOffsetStr.length() = "));
	DEBUG_PRT.println(String(len));
	DEBUG_PRT.print(F("fileOffsetStr = "));
	DEBUG_PRT.println(fileOffsetStr);
	long fileOffset;
	
	while(pos < len) {
		fileOffset = atol(csv.getCsvField(fileOffsetStr, &pos).c_str());
		
		DEBUG_PRT.print(F("file offset string is "));
		DEBUG_PRT.print(fileOffsetStr);
		DEBUG_PRT.print(F("\t file offset = "));
		DEBUG_PRT.println(String(fileOffset));

		file.seek(fileOffset);
		
		if (file.available()) {
			(*verse_text) += _I18n->readLine(file) + "\n";
			(*numRecords)++;
			
		} else {
			_I18n->closeFile(file);
			DEBUG_PRT.print(F("BibleVerse::get() verse text not found at file offset "));
			DEBUG_PRT.print(String(fileOffset));
			DEBUG_PRT.print(F(" for Bible filename "));
			DEBUG_PRT.println(bible_filename);
			return false;
		}
		
	}
	DEBUG_PRT.print(F("verse_text="));
	DEBUG_PRT.println(*verse_text);
	_I18n->closeFile(file);	
	return true;
}
