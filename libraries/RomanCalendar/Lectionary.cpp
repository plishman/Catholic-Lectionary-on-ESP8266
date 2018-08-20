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

	String lect_path = _I18n->configparams.lectionary_path;
	if (!lect_path.endsWith("/")) lect_path += "/";

	DEBUG_PRT.print(F("Lectionary path is ")); DEBUG_PRT.println(lect_path);
	
	*b_OT = false;
	*b_NT = false;
	*b_PS = false;
	*b_G = false;
	
	String filename;

	if(liturgical_cycle == "I") liturgical_cycle = "1";
	if(liturgical_cycle == "II") liturgical_cycle = "2"; // just to be sure, so that information is not returned if the value is neither I nor II

	bool b_use_liturgical_year = false;
	String lect_num = String(number);

	DEBUG_PRT.print(F("testing Lectionary number ")); DEBUG_PRT.println(String(lect_num));

	b_use_liturgical_year = testFile(lect_path + lect_num + "/" + "G" + "/" + liturgical_year);

	if (!b_use_liturgical_year) {
		if (!(testFile(lect_path + lect_num + "/" + "G" + "/" + liturgical_cycle))) {
			DEBUG_PRT.println(F("Lectionary::test() Malformed lectionary entry (No Gospel reading in either liturgical cycle or liturgical year)"));
			return false;
		}
	}

	*b_G = true;
	
	String lit_year_or_cycle = b_use_liturgical_year ? liturgical_year : liturgical_cycle;
	
	DEBUG_PRT.print(F("Using Liturgical Year/Cycle ")); DEBUG_PRT.println(lit_year_or_cycle);
	
	*b_OT = testFile(lect_path + lect_num + "/" + "OT" + "/" + lit_year_or_cycle);
	*b_NT = testFile(lect_path + lect_num + "/" + "NT" + "/" + lit_year_or_cycle);
	*b_PS = testFile(lect_path + lect_num + "/" + "PS" + "/" + lit_year_or_cycle);

	if (!*b_OT) DEBUG_PRT.println(F("No OT reading"));
	if (!*b_NT) DEBUG_PRT.println(F("No NT reading"));
	if (!*b_PS) DEBUG_PRT.println(F("No PS reading"));
	
	return true;
}

bool Lectionary::testFile(String filename) {
	DEBUG_PRT.print(F("testing file ")); DEBUG_PRT.print(filename); DEBUG_PRT.print(F(":"));
	
	char filenamestr[80];
	filename.toCharArray(filenamestr, 80);
	
	if (SD.exists(filenamestr)) {		
		DEBUG_PRT.println(F("present"));
		return true;
	}

	DEBUG_PRT.println(F("not found"));	
	return false;
}


bool Lectionary::get(String liturgical_year, String liturgical_cycle, Lectionary::ReadingsFromEnum readings_from, int number, String* refs_text) {
	if (_I18n == NULL) return false;

	String lect_path = _I18n->configparams.lectionary_path;
	if (!lect_path.endsWith("/")) lect_path += "/";

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

	filename = lect_path + String(number) + "/" + filename;
	if(liturgical_cycle == "I") liturgical_cycle = "1";
	if(liturgical_cycle == "II") liturgical_cycle = "2"; // just to be sure, so that information is not returned if the value is neither I nor II
	
	DEBUG_PRT.printf("\nLectionary::get() ly = %s, lc = %s, filename = %s\n", liturgical_year.c_str(), liturgical_cycle.c_str(), filename.c_str());
	
	// don't know if the requested lectionary number will be for a liturgical cycle or a liturgical year
	// so will try liturgical year first, then liturgical cycle. Both should never be present in the same directory in Lectionary directory Lect/

	File file;
	
	DEBUG_PRT.print(F("Looking for entry for liturgical year..."));
	file = /*_I18n->openFile*/SD.open(filename + "/" + liturgical_year, FILE_READ);

	if (!file.available()) {
		DEBUG_PRT.print(F("not found. Looking for entry for liturgical cycle..."));
		file = /*_I18n->openFile*/SD.open(filename + "/" + liturgical_cycle, FILE_READ);
	}

	if (!file.available()) {
		DEBUG_PRT.println(F("not found - Error: failed to get lectionary entry"));
		return false;
	} else {
		DEBUG_PRT.println(F("done."));
	}
	
	*refs_text = _I18n->readLine(file);
	
	_I18n->closeFile(file);
		
	return true;
}
