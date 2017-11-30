//#include "stdafx.h"
#include "Csv.h"
#include "I18n.h"

const char* const I18n::I18n_LANGUAGES[5] = {
	"en",
	"it",
	"fr",
	"la",
	"cs"
};

const char* const I18n::I18n_SANCTORALE[5] = {
	"un-en.txt",
	"un-it.txt",
	"un-fr.txt",
	"un-la.txt",
	"czech-cs.txt"
};

const char* const I18n::I18n_SEASONS[5] = {
	"advent",
	"christmas",
	"lent",
	"easter",
	"ordinary"
};

const char* const I18n::I18n_COLOURS[4] = {
	"colour.green",
	"colour.violet",
	"colour.white",
	"colour.red"
};

const char* const I18n::I18n_RANK_NAMES[14] = {
	"rank.4_0",
	"rank.3_13",
	"rank.3_12",
	"rank.3_11",
	"rank.3_10",
	"rank.2_9",
	"rank.2_8",
	"rank.2_7",
	"rank.2_6",
	"rank.2_5",
	"rank.1_4",
	"rank.1_3",
	"rank.1_2",
	"rank.1_1"
};

const char* const I18n::I18n_SOLEMNITIES[17] = {
	"temporale.solemnity.nativity",						//nativity
	"temporale.solemnity.holy_family",					//holy_family
	"temporale.solemnity.mother_of_god",				//mother_of_god
	"temporale.solemnity.epiphany",						//epiphany
	"temporale.solemnity.baptism_of_lord",				//baptism_of_lord
	"temporale.solemnity.ash_wednesday",				//ash_wednesday
	"temporale.solemnity.palm_sunday",					//palm_sunday
	"temporale.solemnity.good_friday",					//good_friday
	"temporale.solemnity.holy_saturday",				//holy_saturday
	"temporale.solemnity.easter_sunday",				//easter_sunday
	"temporale.solemnity.ascension",					//ascension
	"temporale.solemnity.pentecost",					//pentecost
	"temporale.solemnity.holy_trinity",					//holy_trinity
	"temporale.solemnity.corpus_christi",				//corpus_christi
	"temporale.solemnity.sacred_heart",					//sacred_heart
	"temporale.solemnity.immaculate_heart",				//immaculate_heart
	"temporale.solemnity.christ_king"					//christ_king
};

#ifndef _WIN32
I18n::I18n(int CS_PIN, int lectionary_config_number) {
	_CS_PIN = CS_PIN;
	_lectionary_config_number = lectionary_config_number;
	
	if (!initializeSD()) {
		Serial.println("Failed to initialize SD card");
	}
	
	get_config();	
}

I18n::I18n( void ) {
//	I18n(_CS_PIN);
	if (!initializeSD()) {
		Serial.println("Failed to initialize SD card");
	}
	
	get_config();
}

#else
I18n::I18n( void ) {
	get_config();
}
#endif

I18n::~I18n()
{
}

void I18n::suppress_output(bool s) {
	_suppress_output = s;
}

bool I18n::get_config( void ) {
	Csv csv;

	String config_filename = String("/config.csv");	
#ifndef _WIN32	
	File file = openFile(config_filename, FILE_READ);
	if (!file.available()) return false;
#else
	FILE* fpi = fopen(config_filename.c_str(), "r");
	if (fpi == NULL) return false; 
	char* filestr;
	char buf[1024];
#endif

	String csv_record;
	bool bFoundSelection = false;
	String desc;
	String lang;
	String yml_filename;
	String sanctorale_filename;
	String bible_filename;
	int pos = 0;
	int i = 1;
	
	do {
	#ifndef _WIN32
		csv_record = readLine(file);
	#else
		filestr = fgets(buf, 1024, fpi);
		if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found
		readLine = String(buf);
	#endif		
		if (_lectionary_config_number == i) {
			pos = 0;
			Serial.println("csv_record: " + csv_record);
			desc = csv.getCsvField(csv_record, &pos);
			Serial.println("\tdesc=" + desc + " pos=" + String(pos));
			lang = csv.getCsvField(csv_record, &pos);
			Serial.println("\tlang=" + lang + " pos=" + String(pos));
			yml_filename = csv.getCsvField(csv_record, &pos);
			Serial.println("\tyml_filename=" + yml_filename + " pos=" + String(pos));
			sanctorale_filename = csv.getCsvField(csv_record, &pos);
			Serial.println("\tsanctorale_filename=" + sanctorale_filename + " pos=" + String(pos));
			bible_filename = csv.getCsvField(csv_record, &pos);
			Serial.println("\tbible_filename=" + bible_filename + " pos=" + String(pos));		

			bFoundSelection = true;
			Serial.println("* selected");
			break;
		} 
		else {
			//Serial.println("Not selected");	
			i++;
		}
		//if (csv.getCsvField(csv_record, &pos) == "selected") {
		//	bFoundSelection = true;
		//	Serial.println("* selected");
		//	break;
		//} 
		//else {
		//	Serial.println("Not selected");
		//}
#ifndef _WIN32
	} while (file.available());
	closeFile(file);
#else
	} while (filestr != NULL);
	fclose(fpi);
#endif

	if (!bFoundSelection) return false;
	
	_lang = lang;
	_yml_filename = yml_filename;
	_sanctorale_filename = sanctorale_filename;
	_bible_filename = bible_filename;
	_have_config = true;
	
	return true;
}

String I18n::get(String I18nPath) {
	//Serial.println("I18n::get()");
	
	if (!_have_config) {
		Serial.println("I18n::get(): No config");
		return "";
	}
	
	if (_suppress_output == true) return "";
	
#ifndef _WIN32
//	if (!initializeSD()) return String("");
#else
	char* filestr;
#endif

	I18nPath = _lang + String(".") + I18nPath;
	String I18nFilename = _yml_filename; //"locales/" + String(I18n_LANGUAGES[_locale]) + ".yml";

#ifndef _WIN32
	File file = openFile(I18nFilename, FILE_READ);
	if (!file) return "";
#else
	FILE* fpi = fopen(I18nFilename.c_str(), "r");
	char buf[1024];
#endif

	bool bTokenMatched = false;
	String lookingforToken;
	String readToken;
	String readData;
	String readLine;
	String I18nCurrentPath;

	int l;

	int currenttokendepth = 0;
	int readtokendepth = 0;

	int numtokensinI18nPath = 1;
	l = I18nPath.length();
	for (int i = 0; i < l; i++) {
		if (I18nPath.charAt(i) == '.') numtokensinI18nPath++;
	}

	int numtokensinI18nCurrentPath = 0;

	int tokenindentwidth = 0;
	bool bFirstPass = true;

	do {
		#ifndef _WIN32
		#else
			filestr = fgets(buf, 1024, fpi);
			if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found
		#endif

		#ifndef _WIN32
			readLine = this->readLine(file);
			if (readLine == "") continue;
		#else
			readLine = String(buf);
		#endif
		
		readToken = readLine.substring(0, readLine.indexOf(":"));

		readtokendepth = readToken.lastIndexOf(" ") + 1;
		readToken.trim();

		l = readToken.length() - 1;

		if ((readToken.charAt(0) == '\'') && (readToken.charAt(l) == (const char)'\'')) {	// strip enclosing single quotes from token, if present
			readToken = readToken.substring(1, l);
		}

		if (bFirstPass == true && readtokendepth > 0) {
			tokenindentwidth = readtokendepth;		// calculate the width in characters of the indents (yaml files are structured by the indentation)
			bFirstPass = false;
		}

		if (readtokendepth == 0) {
			I18nCurrentPath = readToken;
			numtokensinI18nCurrentPath = 1;
		}

		if (currenttokendepth < readtokendepth) {
			I18nCurrentPath += "." + readToken;	// one level deeper, add on new token to path
			numtokensinI18nCurrentPath++;
		}

		if ((currenttokendepth != 0) && (currenttokendepth == readtokendepth)) { // same level, replace last token with read token
			I18nCurrentPath = I18nCurrentPath.substring(0, I18nCurrentPath.lastIndexOf("."));		// remove last field,
			I18nCurrentPath += "." + readToken;														// and replace it with the token just read
		}

		if ((readtokendepth != 0) && (currenttokendepth > readtokendepth)) { // most complicated case: remove number of tokens indicated by change in indentation, and replace end token
			for (int i = readtokendepth; i <= currenttokendepth; i += tokenindentwidth) {
				I18nCurrentPath = I18nCurrentPath.substring(0, I18nCurrentPath.lastIndexOf("."));	// remove last field,
				numtokensinI18nCurrentPath--;
			}
			I18nCurrentPath += "." + readToken;														// and replace it with the token just read
			numtokensinI18nCurrentPath++;
		}

		currenttokendepth = readtokendepth;

		if ((I18nPath.indexOf(I18nCurrentPath) == 0) && numtokensinI18nCurrentPath == numtokensinI18nPath) { // found our key
			readData = readLine.substring(readLine.indexOf(":") + 1);
			readData.trim();
			bTokenMatched = true;
		}
	} 
	#ifndef _WIN32
		while (file.available() && !bTokenMatched); // to loop, File needs to have returned data and token remained unmatched
	#else
		while (filestr != NULL && !bTokenMatched); // to loop, File needs to have returned data and token remained unmatched
	#endif
	
	if (!bTokenMatched) { // reached end of file without finding the token
		#ifndef _WIN32
			Serial.println("token " + lookingforToken + " not found (EOF)\n");
			closeFile(file);
		#else
			printf("token %s not found (EOF)\n", lookingforToken.c_str());
			fclose(fpi);
		#endif
		return String("");
	}

	#ifndef _WIN32
		closeFile(file);
	#else
		fclose(fpi);
	#endif

	l = readData.length() - 1;

	if ((readData.charAt(0) == '\'') && (readData.charAt(l) == (const char)'\'')) {	// strip enclosing single quotes
		readData = readData.substring(1, l);
	}

	_callcount++;

	//printf("Found: read data: %s\n", readData.c_str());
	return readData;
}

#ifndef _WIN32
bool I18n::initializeSD() {
 //Serial.println("Initializing SD card...");
 //Serial.println("_CS_PIN is " + String(_CS_PIN));
  
  pinMode(_CS_PIN, OUTPUT);
  digitalWrite(_CS_PIN, HIGH);
  
  return SD.begin(_CS_PIN, SPI_HALF_SPEED);
}

String I18n::readLine(File file) { 
  //Serial.println("readLine(): position=" + String(file.position()));
  String received = "";
  char ch;
  
  if (!file) {
    Serial.println("file is not available");
	return "";
  }
  
  while (file.available()) {
    ch = file.read();
    
	if (ch == '\r' && file.available()) { // skip over windows line ending 
		ch = file.read();		
	}
	
	if (ch == '\n') {
	  //Serial.println("received lf len=" + String(received.length()));
	  return received;
    } else {
      received += ch;
    }
	
	if (file.position() == file.size()) {
		//Serial.println("EOF");
		return received;
	} 
  }
  //Serial.println("dropped through");
  return "";
}

File I18n::openFile(String filename, uint8_t mode) {
  initializeSD();
  
  File file = SD.open(filename.c_str(), mode);
  if (file) {
    //Serial.println("File opened with success!");
    return file;
  } else {
    Serial.println("Error opening file " + filename);
    return file;
  }
}

void I18n::closeFile(File file) {
  if (file) {
    file.close();
    //Serial.println("File closed");
  }
}
#endif