#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include "Csv.h"
#include "BibleVerse.h"

BibleVerse::BibleVerse(I18n* i)
{
	//Serial.println("BibleVerse::BibleVerse()");
	_I18n = i;
	String filename;
	get_bible_filename(&filename); // populates the _book_count variable when the bible filename has been determined from the locale
	//Serial.println("+");
}

BibleVerse::~BibleVerse()
{
}

bool BibleVerse::get_bible_filename(String* filename) {
	Csv csv;

	String bibleslist_filename = String("/locales/Bibles.csv");	
	
	File file = _I18n->openFile(bibleslist_filename, FILE_READ);

	if (!file.available()) return false;
		
	String csv_record;
	String lang;
	int pos = 0;
	
	Serial.println("selected lang=" + String(_I18n->I18n_LANGUAGES[_I18n->_locale]));
	
	while (file.available() && lang != _I18n->I18n_LANGUAGES[_I18n->_locale]) {
		csv_record = _I18n->readLine(file);
		pos = 0;
		Serial.println("csv_record: " + csv_record);
		lang = csv.getCsvField(csv_record, &pos);
		Serial.println("loop:lang=" + lang + " pos=" + String(pos));
	} 

	Serial.println("lang=" + lang + " pos=" + String(pos));
	
	_I18n->closeFile(file);
	
	if (lang == _I18n->I18n_LANGUAGES[_I18n->_locale]) {
		*filename = "/Bibles/" + csv.getCsvField(csv_record, &pos);
		Serial.println("filename=" + *filename);
		String book_count = csv.getCsvField(csv_record, &pos);
		_book_count = book_count.toInt();
		Serial.println("book count=" + book_count + " int is " + String(_book_count));
		return true;
	}
	
	return false;
}

bool BibleVerse::get(int book, int chapter, int verse, String* verse_text) {
	//Serial.println("BibleVerse::get() " + String(book) + " " + String(chapter) + ":" + String(verse));
	
	//if (!initializeSD()) return String("");

	book++;

	if (_I18n == NULL) return false;

	String index_filename = String("/Bibles/") + String(_I18n->I18n_LANGUAGES[_I18n->_locale]) + "/Bible/" + String(book) + "/" + String(chapter) + "/" + String(verse);	

	Serial.println("Index filename is " + index_filename);
	
	File file = _I18n->openFile(index_filename, FILE_READ);

	if (!file.available()) return false;

	String fileOffsetStr = _I18n->readLine(file);
	
	long fileOffset = atol(fileOffsetStr.c_str());	// first line must contain a 32 bit number, the offset into the bible csv file containing the record of the verse

	Serial.println("file offset string is " + fileOffsetStr);
	Serial.println("file offset = " + String(fileOffset));
	
	_I18n->closeFile(file);

	String bible_filename;
	
	if (!get_bible_filename(&bible_filename)) return false;

	Serial.println("bible filename is " + bible_filename);

	if (book < 1 || book > _book_count) return false; // _book_count is set by get_bible_filename, since it's part of the csv record that associates bible filenames with languages (non-apocrypha versions have 66 books, otherwise 73)

	file = _I18n->openFile(bible_filename, FILE_READ);

	if (!file.available()){
		Serial.println("BibleVerse::get() can't open bible");
		return false;
	} 

	file.seek(fileOffset);
	
	*verse_text = _I18n->readLine(file);
	
	_I18n->closeFile(file);
	
	return true;
}
/*
bool BibleVerse::initializeSD() {
  Serial.println("Initializing SD card...");
  pinMode(_CS_PIN, OUTPUT);

  bool bResult = SD.begin();
   
  if (bResult) {
    Serial.println("SD card is ready to use.");
  } else {
    Serial.println("SD card initialization failed");
  }
  
  return bResult;
}

void BibleVerse::closeFile() {
  if (_file) {
    _file.close();
    Serial.println("File closed");
  }
}

int BibleVerse::openFile(String filename, uint8_t mode) {
  _file = SD.open(filename.c_str(), mode);
  if (_file) {
    Serial.println("File opened with success!");
    return 1;
  } else {
    Serial.println("Error opening file...");
    return 0;
  }
}

String BibleVerse::readLine() {
  String received = "";
  char ch;
  while (_file.available()) {
    ch = _file.read();
    if (ch == '\n') {
      return String(received);
    } else {
      received += ch;
    }
	if (_file.position() == _file.size()) {
		return String(received);
	} 
  }
  return "";
}

*/