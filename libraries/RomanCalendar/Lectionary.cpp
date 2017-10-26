#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include "Lectionary.h"

Lectionary::Lectionary(I18n* i)
{
	_I18n = i;
}

Lectionary::~Lectionary()
{
}

bool Lectionary::get(String liturgical_year, String liturgical_cycle, Lectionary::ReadingsFromEnum readings_from, int number, String* refs_text) {
	if (_I18n == NULL) return false;

	String filename;
	
	switch(readings_from) {
	case READINGS_OT:
		filename = "OT";
		break;

	case READINGS_NT:
		filename = "NT";
		break;

	case READINGS_PS:
		filename = "PS";
		break;

	case READINGS_G:
		filename = "G";
		break;

	}

	filename = "Lect/" + String(number) + "/" + filename;
	if(liturgical_cycle == "I") liturgical_cycle = "1";
	if(liturgical_cycle == "II") liturgical_cycle = "2"; // just to be sure, so that information is not returned if the value is neither I nor II
	
	//printf("\nLectionary::get() ly = %s, lc = %s, filename = %s\n", liturgical_year.c_str(), liturgical_cycle.c_str(), filename.c_str());
	
	// don't know if the requested lectionary number will be for a liturgical cycle or a liturgical year
	// so will try liturgical year first, then liturgical cycle. Both should never be present in the same directory in Lectionary directory Lect/

	File file;
	
	file = _I18n->openFile(filename + "/" + liturgical_year, FILE_READ);

	if (!file.available()) file = _I18n->openFile(filename + "/" + liturgical_cycle, FILE_READ);

	if (!file.available()) return false;
	
	*refs_text = _I18n->readLine(file);
	
	_I18n->closeFile(file);
		
	return true;
}
