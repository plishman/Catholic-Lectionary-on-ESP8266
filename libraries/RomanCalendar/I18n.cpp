//#include "stdafx.h"
#include "Csv.h"
#include "I18n.h"

//ConfigParams class
void ConfigParams::Clear() {
	desc = "";
	lang = "";
	yml_filename = "";
	sanctorale_filename = "";
	bible_filename = "";
	String lectionary_path = "/Lect";
	font_filename = "builtin";
	transfer_to_sunday = true;
	celebrate_feast_of_christ_priest = true;
	have_config = false;
	right_to_left = false;
	font_tuning_percent = 50.0;
	font_use_fixed_spacing = false;
	font_use_fixed_spacecharwidth = false;
	font_fixed_spacing = 1;
	font_fixed_spacecharwidth = 2;		
	cr_after_verse = false;
	show_verse_numbers = false;
}

void ConfigParams::Dump() {
	DEBUG_PRT.print(F("\tdesc=")); 										DEBUG_PRT.println(desc);
	DEBUG_PRT.print(F("\tlang=")); 										DEBUG_PRT.println(lang);
	DEBUG_PRT.print(F("\tyml_filename=")); 								DEBUG_PRT.println(yml_filename);
	DEBUG_PRT.print(F("\tsanctorale_filename=")); 						DEBUG_PRT.println(sanctorale_filename);
	DEBUG_PRT.print(F("\tbible_filename=")); 							DEBUG_PRT.println(bible_filename);
	DEBUG_PRT.print(F("\tlectionary path=")); 							DEBUG_PRT.println(lectionary_path);
	DEBUG_PRT.print(F("\ttransfer_to_sunday=")); 						DEBUG_PRT.println(String(transfer_to_sunday));
	DEBUG_PRT.print(F("\tcelebrate_feast_of_christ_eternal_priest=")); 	DEBUG_PRT.println(String(celebrate_feast_of_christ_priest));
	DEBUG_PRT.print(F("\tfont filename=")); 							DEBUG_PRT.println(font_filename);
	DEBUG_PRT.print(F("\tright_to_left=")); 							DEBUG_PRT.println(String(right_to_left));			
	DEBUG_PRT.print(F("\tfont fixed spacing=")); 						DEBUG_PRT.println(font_fixed_spacing);
	DEBUG_PRT.print(F("\tfont fixed spacecharwidth=")); 				DEBUG_PRT.println(font_fixed_spacecharwidth);
	DEBUG_PRT.print(F("\tfont tuning=")); 								DEBUG_PRT.print(String(font_tuning_percent)); DEBUG_PRT.println(F(" percent"));	
	
	DEBUG_PRT.print(F("\tfont use fixed spacing: ")); 					
	if(font_use_fixed_spacing) {
		DEBUG_PRT.println(F("Yes"));
	}
	else {
		DEBUG_PRT.println(F("No (If not using builtin font, will use character advanceWidths and percent tuning value instead)"));	
	}
	
	DEBUG_PRT.print(F("\tfont use fixed spacecharwidth: "));
	if (font_use_fixed_spacecharwidth) {
		DEBUG_PRT.println(F("Yes"));
	}
	else {
		DEBUG_PRT.println(F("No (If not using builtin font, will use space character advanceWidth and percent tuning value instead)"));				
	}
	
	DEBUG_PRT.print(F("\t<cr> after verse=")); 							DEBUG_PRT.println(String(cr_after_verse));
	DEBUG_PRT.print(F("\tshow verse numbers=")); 						DEBUG_PRT.println(String(show_verse_numbers));
}

ConfigParams::ConfigParams() {};

// copy constructor
ConfigParams::ConfigParams(const ConfigParams &p2) {
	desc = p2.desc;
	lang = p2.lang;
	yml_filename = p2.yml_filename;
	sanctorale_filename = p2.sanctorale_filename;
	bible_filename = p2.bible_filename;
	lectionary_path = p2.lectionary_path;
	font_filename = p2.font_filename;
	transfer_to_sunday = p2.transfer_to_sunday;
	celebrate_feast_of_christ_priest = p2.celebrate_feast_of_christ_priest;
	have_config = p2.have_config;
	right_to_left = p2.right_to_left;
	font_tuning_percent = p2.font_tuning_percent;
	font_use_fixed_spacing = p2.font_use_fixed_spacing;
	font_use_fixed_spacecharwidth = p2.font_use_fixed_spacecharwidth;
	font_fixed_spacing = p2.font_fixed_spacing;
	font_fixed_spacecharwidth = p2.font_fixed_spacecharwidth;
	cr_after_verse = p2.cr_after_verse;
	show_verse_numbers = p2.show_verse_numbers;
	
}
///////



//I18n class
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

const char* const I18n::I18n_SOLEMNITIES[19] = {
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
	"temporale.solemnity.christ_priest",				//christ_priest (optional in some areas)
	"temporale.solemnity.holy_trinity",					//holy_trinity
	"temporale.solemnity.corpus_christi",				//corpus_christi
	"temporale.solemnity.sacred_heart",					//sacred_heart
	"temporale.solemnity.immaculate_heart",				//immaculate_heart
	"temporale.solemnity.christ_king",					//christ_king
	"temporale.solemnity.immaculate_conception"			//immaculate_conception
};

#ifndef _WIN32
I18n::I18n(int CS_PIN, int lectionary_config_number) {
	_CS_PIN = CS_PIN;
	_lectionary_config_number = lectionary_config_number;
	
	if (!initializeSD()) {
		DEBUG_PRT.println(F("Failed to initialize SD card"));
	}
	
	get_config();	
}

I18n::I18n( void ) {
//	I18n(_CS_PIN);
	if (!initializeSD()) {
		DEBUG_PRT.println(F("Failed to initialize SD card"));
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
	DEBUG_PRT.print(F("I18n::get_config() lectionary_config_number = "));
	DEBUG_PRT.println(String(_lectionary_config_number));

	Csv csv;

	String config_filename = String("/config.csv");	
#ifndef _WIN32	
	File file = openFile(config_filename, FILE_READ);
	if (!file.available()) {
		DEBUG_PRT.println(F("Couldn't open config file"));
		return false;
	}
#else
	FILE* fpi = fopen(config_filename.c_str(), "r");
	if (fpi == NULL) return false; 
	char* filestr;
	char buf[1024];
#endif

	String csv_record = "";
	bool bFoundSelection = false;
/*
	String desc = "";
	String lang = "";
	String yml_filename = "";
	String sanctorale_filename = "";
	String bible_filename = "";
	String font_filename = "builtin";
*/
	String s_transfer_to_sunday = "";
	String s_celebrate_feast_of_christ_priest = "";
	String s_right_to_left = "";
	String s_font_tuning = "";
	
	String s_cr_after_verse = "";
	String s_show_verse_numbers = "";
/*
	bool transfer_to_sunday = false;
	bool celebrate_feast_of_christ_priest = false;
	bool right_to_left = false;
	double font_tuning_percent = 50.0;	//defaults to 50%
	bool font_use_fixed_spacing = false;
	bool font_use_fixed_spacecharwidth = false;
	int font_fixed_spacing = 1;
	int font_fixed_spacecharwidth = 2;
*/
	ConfigParams c;

	int pos = 0;
	int i = 0;
	
	readLine(file); // skip first line, as it contains the column headings
	
	do {
		c.Clear();
	#ifndef _WIN32
		csv_record = readLine(file);
	#else
		filestr = fgets(buf, 1024, fpi);
		if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found
		readLine = String(buf);
	#endif		
		pos = 0;
		DEBUG_PRT.print(F("csv_record: "));
		DEBUG_PRT.println(csv_record);
		
		c.desc = csv.getCsvField(csv_record, &pos);
		c.lang = csv.getCsvField(csv_record, &pos);
		c.yml_filename = csv.getCsvField(csv_record, &pos);
		c.sanctorale_filename = csv.getCsvField(csv_record, &pos);
		c.bible_filename = csv.getCsvField(csv_record, &pos);
		c.lectionary_path = csv.getCsvField(csv_record, &pos);
		
		s_transfer_to_sunday = csv.getCsvField(csv_record, &pos);
		c.transfer_to_sunday = ((s_transfer_to_sunday.indexOf("true") != -1) || s_transfer_to_sunday == "1");
		
		s_celebrate_feast_of_christ_priest = csv.getCsvField(csv_record, &pos);
		c.celebrate_feast_of_christ_priest = ((s_celebrate_feast_of_christ_priest.indexOf("true") != -1) || s_celebrate_feast_of_christ_priest == "1");
		
		c.font_filename = csv.getCsvField(csv_record, &pos);
		
		s_right_to_left = csv.getCsvField(csv_record, &pos);
		c.right_to_left = ((s_right_to_left.indexOf("true") != -1) || s_right_to_left == "1");		
		
		s_font_tuning = csv.getCsvField(csv_record, &pos);
		// font tuning can be either a percentage 0-100, or npx for fixed intercharacter spacing (in pixels), or npx mpx for fixed interchar spacing and fixed spacechar width
		// check for fixed intercharacter distance:
				
		int pxoffset = s_font_tuning.indexOf("px",0);
		if (pxoffset != -1) {
			c.font_fixed_spacing = s_font_tuning.substring(0, pxoffset).toInt();
			c.font_use_fixed_spacing = true;
			
			int px2offset = s_font_tuning.indexOf("px", pxoffset + 2); // +2 skip over the first "px" to see if there is another occurrence of px, indicating the spacechar width is specified also
			if (px2offset != -1) {
				c.font_fixed_spacecharwidth = s_font_tuning.substring(pxoffset + 2, px2offset).toInt();
				c.font_use_fixed_spacecharwidth = true;
			}
		}
		else {
			c.font_tuning_percent = (double)s_font_tuning.toFloat();
		}

		s_cr_after_verse = csv.getCsvField(csv_record, &pos);
		c.cr_after_verse = ((s_cr_after_verse.indexOf("true") != -1) || s_cr_after_verse == "1");		
		s_show_verse_numbers = csv.getCsvField(csv_record, &pos);
		c.show_verse_numbers = ((s_show_verse_numbers.indexOf("true") != -1) || s_show_verse_numbers == "1");		
		
		if (_lectionary_config_number == i) {
			c.Dump();
			bFoundSelection = true;
			DEBUG_PRT.println(F("* selected"));
			break;
		} 
		else {
			i++;
			//c.Clear();
		}

#ifndef _WIN32
	} while (file.available() && !bFoundSelection);
	closeFile(file);
#else
	} while (filestr != NULL);
	fclose(fpi);
#endif

	configparams.Clear();
	configparams = c;
/*	
	configparams.lang = lang;
	configparams.yml_filename = yml_filename;
	configparams.sanctorale_filename = sanctorale_filename;
	configparams.bible_filename = bible_filename;
	configparams.transfer_to_sunday = transfer_to_sunday;
	configparams.celebrate_feast_of_christ_priest = celebrate_feast_of_christ_priest;
	configparams.font_filename = font_filename;
	configparams.right_to_left = right_to_left;
	configparams.font_tuning_percent = font_tuning_percent;
	configparams.font_fixed_spacing = font_fixed_spacing;
	configparams.font_fixed_spacecharwidth = font_fixed_spacecharwidth;
	configparams.font_use_fixed_spacing = font_use_fixed_spacing;
	configparams.font_use_fixed_spacecharwidth = font_use_fixed_spacecharwidth;
*/	
	configparams.have_config = true;
	
	if (!bFoundSelection) {
		DEBUG_PRT.print(F("Can't find lectionary config entry number, using last scanned entry in config.csv"));
		DEBUG_PRT.println(String(_lectionary_config_number));
		return false; // will simply use the last entry in the file if not found
	}
	
	return true;
}

String I18n::getdate(time64_t t) {
	tmElements_t ts;
	breakTime(t, ts);
	
	String date_format = get("date");
	
	date_format.replace("%{d}", String(ts.Day));
	date_format.replace("%{dd}", ts.Day > 9 ? String(ts.Day) : "0" + String(ts.Day));
	
	date_format.replace("%{m}", String(ts.Month));
	date_format.replace("%{mm}", ts.Month > 9 ? String(ts.Month) : "0" + String(ts.Month));
	date_format.replace("%{mmm}", get("month." + String(ts.Month)));
	
	String yy = tmYearToCalendar(ts.Year) % 100 < 10 ? "0" + String(tmYearToCalendar(ts.Year) % 100) : String(tmYearToCalendar(ts.Year) % 100);
	
	date_format.replace("%{yy}", yy);
	date_format.replace("%{yyyy}", String(tmYearToCalendar(ts.Year)));	

	date_format.replace("%{day}", get("weekday." + String(ts.Wday - 1)));
	
	return date_format;
}

String I18n::get(String I18nPath) {
	//DEBUG_PRT.println("I18n::get()");
	
	if (!configparams.have_config) {
		DEBUG_PRT.println(F("I18n::get(): No config"));
		return "";
	}

	DEBUG_PRT.print(F("I18n::get(): "));
	DEBUG_PRT.println(I18nPath);
	
	if (_suppress_output == true) 
	{
		DEBUG_PRT.println(F("_suppress_output == true"));
		return "";	
	}

#ifndef _WIN32
//	if (!initializeSD()) return String("");
#else
	char* filestr;
#endif

	I18nPath = configparams.lang + String(".") + I18nPath;
	String I18nFilename = configparams.yml_filename; //"locales/" + String(I18n_LANGUAGES[_locale]) + ".yml";

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

		//if ((I18nPath.indexOf(I18nCurrentPath) == 0) && numtokensinI18nCurrentPath == numtokensinI18nPath) { // found our key
		if (I18nPath == I18nCurrentPath) { // found our key
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
			DEBUG_PRT.print(F("token ")); DEBUG_PRT.print(lookingforToken); DEBUG_PRT.println(F(" not found (EOF)"));
			closeFile(file);
		#else
			print(F("token ")); DEBUG_PRT.print(lookingforToken); DEBUG_PRT.println(F("not found (EOF)"));
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
 //DEBUG_PRT.println("Initializing SD card...");
 //DEBUG_PRT.println("_CS_PIN is " + String(_CS_PIN));
	return true;
 
  //pinMode(_CS_PIN, OUTPUT);
  //digitalWrite(_CS_PIN, HIGH);
  
  //return SD.begin(_CS_PIN, SPI_HALF_SPEED);
}

String I18n::readLine(File file) { 
  //DEBUG_PRT.println("readLine(): position=" + String(file.position()));
  String received = "";
  char ch;
  
  if (!file) {
    DEBUG_PRT.println(F("file is not available"));
	return "";
  }
  
  while (file.available()) {
    ch = file.read();
    
	if (ch == '\r' && file.available()) { // skip over windows line ending 
		ch = file.read();		
	}
	
	if (ch == '\n') {
	  //DEBUG_PRT.println("received lf len=" + String(received.length()));
	  return received;
    } else {
      received += ch;
    }
	
	if (file.position() == file.size()) {
		//DEBUG_PRT.println("EOF");
		return received;
	} 
  }
  //DEBUG_PRT.println("dropped through");
  return "";
}

File I18n::openFile(String filename, uint8_t mode) {
  initializeSD();
  
  File file = SD.open(filename.c_str(), mode);
  if (file) {
    //DEBUG_PRT.println("File opened with success!");
    return file;
  } else {
    DEBUG_PRT.print(F("Error opening file ")); 
	DEBUG_PRT.println(filename);
	
    return file;
  }
}

void I18n::closeFile(File file) {
  if (file) {
    file.close();
    //DEBUG_PRT.println("File closed");
  }
}
#endif