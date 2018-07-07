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

bool Lectionary::test(int number, String liturgical_year, String liturgical_cycle, bool *b_OT, bool *b_NT, bool *b_PS, bool *b_G) {
	if (_I18n == NULL) return false;

	*b_OT = false;
	*b_NT = false;
	*b_PS = false;
	*b_G = false;
	
	String filename;

	if(liturgical_cycle == "I") liturgical_cycle = "1";
	if(liturgical_cycle == "II") liturgical_cycle = "2"; // just to be sure, so that information is not returned if the value is neither I nor II

	bool b_use_liturgical_year = false;
	String lect_num = String(number);

	I2CSerial.println("testing Lectionary number " + lect_num);

	b_use_liturgical_year = testFile("Lect/" + lect_num + "/" + "G" + "/" + liturgical_year);

	if (!b_use_liturgical_year) {
		if (!(testFile("Lect/" + lect_num + "/" + "G" + "/" + liturgical_cycle))) {
			I2CSerial.println("Lectionary::test() Malformed lectionary entry (No Gospel reading in either liturgical cycle or liturgical year)");
			return false;
		}
	}

	*b_G = true;
	
	String lit_year_or_cycle = b_use_liturgical_year ? liturgical_year : liturgical_cycle;
	
	I2CSerial.println("Using Liturgical Year/Cycle " + lit_year_or_cycle);	
	
	*b_OT = testFile("Lect/" + lect_num + "/" + "OT" + "/" + lit_year_or_cycle);
	*b_NT = testFile("Lect/" + lect_num + "/" + "NT" + "/" + lit_year_or_cycle);
	*b_PS = testFile("Lect/" + lect_num + "/" + "PS" + "/" + lit_year_or_cycle);

	if (!*b_OT) I2CSerial.println("No OT reading");
	if (!*b_NT) I2CSerial.println("No NT reading");
	if (!*b_PS) I2CSerial.println("No PS reading");
	
	return true;
}

bool Lectionary::testFile(String filename) {
	I2CSerial.printf("testing file %s : ", filename.c_str());
	
	char filenamestr[80];
	filename.toCharArray(filenamestr, 80);
	
	if (SD.exists(filenamestr)) {		
		I2CSerial.printf("present\n");
		return true;
	}

	I2CSerial.printf("not found\n");	
	return false;
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
	
	I2CSerial.printf("\nLectionary::get() ly = %s, lc = %s, filename = %s\n", liturgical_year.c_str(), liturgical_cycle.c_str(), filename.c_str());
	
	// don't know if the requested lectionary number will be for a liturgical cycle or a liturgical year
	// so will try liturgical year first, then liturgical cycle. Both should never be present in the same directory in Lectionary directory Lect/

	File file;
	
	I2CSerial.print("Looking for entry for liturgical year...");
	file = /*_I18n->openFile*/SD.open(filename + "/" + liturgical_year, FILE_READ);

	if (!file.available()) {
		I2CSerial.print("not found. Looking for entry for liturgical cycle...");
		file = /*_I18n->openFile*/SD.open(filename + "/" + liturgical_cycle, FILE_READ);
	}

	if (!file.available()) {
		I2CSerial.println("not found - Error: failed to get lectionary entry");
		return false;
	} else {
		I2CSerial.println("done.");
	}
	
	*refs_text = _I18n->readLine(file);
	
	_I18n->closeFile(file);
		
	return true;
}
