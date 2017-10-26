#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include "BibleVerse.h"

BibleVerse::BibleVerse(I18n* i)
{
	//Serial.println("BibleVerse::BibleVerse()");
	_I18n = i;
	//Serial.println("+");
}

BibleVerse::~BibleVerse()
{
}

bool BibleVerse::get(int book, int chapter, int verse, String* verse_text) {
	//Serial.println("BibleVerse::get() " + String(book) + " " + String(chapter) + ":" + String(verse));
	
	//if (!initializeSD()) return String("");

	book++;

	if (_I18n == NULL) return false;

	if (book < 1 || book > 73) return false;

	String index_filename = "Bible/" + String(book) + "/" + String(chapter) + "/" + String(verse);

	//Serial.println("Index filename is " + index_filename);
	
	File file = _I18n->openFile(index_filename, FILE_READ);

	if (!file.available()) return false;

	String fileOffsetStr = _I18n->readLine(file);
	
	long fileOffset = atol(fileOffsetStr.c_str());	// first line must contain a 32 bit number, the offset into the bible csv file containing the record of the verse

	//Serial.println("file offset string is " + fileOffsetStr);
	//Serial.println("file offset = " + String(fileOffset));
	
	_I18n->closeFile(file);

	file = _I18n->openFile(_bible_filename, FILE_READ);

	if (!file.available()) return false;

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