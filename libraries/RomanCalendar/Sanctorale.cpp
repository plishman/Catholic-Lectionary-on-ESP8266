//#include "stdafx.h"
#include "Sanctorale.h"

Sanctorale::Sanctorale(bool transfer_to_sunday, I18n* i) {
	//_temporale = tc; //new Temporale(transfer_to_sunday, i);
	_locale = i->_locale;
	_I18n = i;
	_transfer_to_sunday = transfer_to_sunday;
	_Lectionary = 0;
}

Sanctorale::~Sanctorale() {
}

bool Sanctorale::get(time64_t date) { // in lent and advent, solemnities falling on a sunday are moved to monday.
									// On mondays in lent and advent, need to look back and check if there was a
									// solemnity the previous sunday.
	if (_I18n == NULL) {
		DEBUG_PRT.println(F("Sanctorale::get(): _I18n is null"));
		return false;
	} 

	if (!(_I18n->configparams.have_config)) {
		DEBUG_PRT.println(F("Sanctorale::get(): No config"));
		return false;
	}

	_Lectionary = 0;
	
	//DEBUG_PRT.println("Sanctorale::get()");
#ifndef _WIN32
	//if (!_I18n->initializeSD()) return String("");
#else
	char buf[1024];
	char* filestr;
#endif
	
	String readtoken;
	String readtokendata;

	int m = Temporale::month(date);
	int d = Temporale::dayofmonth(date);

	String month_token = "= " + String(m);
	String readLine;

	String I18nFilename = _I18n->configparams.sanctorale_filename; //"data/" + String(_I18n->I18n_SANCTORALE[_locale]);

#ifndef _WIN32
	File file = _I18n->openFile(I18nFilename, FILE_READ);
	if (!file) {
		DEBUG_PRT.print(F("Sanctorale::get() couldn't open file ")); DEBUG_PRT.println(I18nFilename);
		return false;
	} 
#else
	FILE* fpi = fopen(I18nFilename.c_str(), "r");
#endif

	// scan for "= <month_number>"
	bool bFound = false;
	do {
		#ifndef _WIN32
			readLine = _I18n->readLine(file);
			if (readLine == "") continue;
		#else
			filestr = fgets(buf, 1024, fpi);
			if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found
			readLine = String(buf);
		#endif

		if (readLine.indexOf(month_token) == 0) {
			bFound = true;
		}
#ifndef _WIN32
	} while (!bFound && file.available());
#else
	} while (!bFound && filestr != NULL);
#endif

	if (!bFound) return false; // got to the end of the file without finding it

							   // scan for string starting with the same number as the day of the month
	bFound = false;
	bool bEndOfMonth = false;
	String daynumber = String(d) + " ";
	do {
		#ifndef _WIN32
			readLine = _I18n->readLine(file);
			if (readLine == "" || readLine.indexOf("#") == 0) continue; // lines which start with a # are comments, and are ignored
		#else
			filestr = fgets(buf, 1024, fpi);
			if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found
			readLine = String(buf);
			if (readLine == "" || readLine.indexOf("#") == 0) continue; // lines which start with a # are comments, and are ignored
		#endif

		if (readLine.indexOf(daynumber) == 0) {
			bFound = true;
		}

		if (readLine == "" || readLine.indexOf("=") == 0) { // scanned past end of month
			bEndOfMonth = true;
		}
#ifndef _WIN32
	} while (!bFound && !bEndOfMonth && file.available()); 
#else
	} while (!bFound && !bEndOfMonth && filestr != NULL);
#endif

#ifndef _WIN32
	_I18n->closeFile(file);
#else
	fclose(fpi);
#endif
	
	if (!bFound || bEndOfMonth) return false; // got to the end of the file without finding it
	
	String monthdayandflags = readLine.substring(0, readLine.indexOf(":"));
	
	//setLectionaryNumber(monthdayandflags);
	//Enums::Season seas = _temporale->season(date);
	//Enums::Ranks rank; //= _temporale->_rank_e;
	//Enums::Colours colour; //= _temporale->_colour_e;

	bool bIsSanctorale = false;
	
	if (monthdayandflags.indexOf("o") != -1 || Temporale::sunday(date)) {	// solemnity
		_hdo = true;
		_holy_day_of_obligation = _I18n->get(I18n_HOLY_DAY_OF_OBLIGATION);
	}
	else {
		_hdo = false;
		_holy_day_of_obligation = "";
	}
	
	if (monthdayandflags.indexOf("s") != -1) {	// solemnity
		//if (move_to_monday) {
		//	if ((seas == SEASON_ADVENT || seas == SEASON_LENT) && sunday(date)) return false; // if a solemnity falls on a sunday in Lent or Advent, it is moved to the Monday after
		//}
		setRank(Enums::RANKS_SOLEMNITY_GENERAL);
		setColour(Enums::COLOURS_WHITE);

		if (monthdayandflags.indexOf("R") != -1) {
			//if (!(((seas == Enums::SEASON_LENT) && (rank == Enums::RANKS_MEMORIAL_GENERAL)) || (rank == Enums::RANKS_MEMORIAL_OPTIONAL))) {
				setColour(Enums::COLOURS_RED);
			//}
		}
		bIsSanctorale = true;
	}
	if (monthdayandflags.indexOf("m") != -1 && !Temporale::sunday(date)) {	//memorial
		//if (monthdayandflags.indexOf("m2.5") != -1) {
		//	setColour(Enums::COLOURS_WHITE);
		//	setRank(Enums::RANKS_FEAST_LORD_GENERAL); // rank is 2.5
		//}
		//else {
			//if (seas != Enums::SEASON_LENT) {
				setColour(Enums::COLOURS_WHITE);
				setRank(Enums::RANKS_MEMORIAL_GENERAL);
				bIsSanctorale = true;
				//printf("**Memorial general\n");
			//}
			//else {
			//	rank = Enums::RANKS_COMMEMORATION;
			//}
		//}
	}
	else if (monthdayandflags.indexOf("f") != -1) {
		if (monthdayandflags.indexOf("f2.5") != -1) {
			setColour(Enums::COLOURS_WHITE);
			setRank(Enums::RANKS_FEAST_LORD_GENERAL); // rank is 2.5
			bIsSanctorale = true;
		}
		else if (!Temporale::sunday(date)) {
			setColour(Enums::COLOURS_WHITE);
			setRank(Enums::RANKS_FEAST_GENERAL);
			bIsSanctorale = true;
		}
	}
	else if (!Temporale::sunday(date)){
		//if (seas != Enums::SEASON_LENT) {
			setRank(Enums::RANKS_MEMORIAL_OPTIONAL);	// colour is the colour of the season
			bIsSanctorale = true;
		//}
		//else {
		//	rank = Enums::RANKS_COMMEMORATION; // commemorations fall in lent
		//}
	}

	//	if ((monthdayandflags.indexOf("s") == -1) && (monthdayandflags.indexOf("m") == -1) && (monthdayandflags.indexOf("f") == -1)) {
	//		// if none of s, m, or f flags are set, the entry is a commemoration.
	//		rank = RANKS_COMMEMORATION;	// colour is the colour of the season
	//	}

	if (monthdayandflags.indexOf("R") != -1 && bIsSanctorale) {
		//if (!(((seas == Enums::SEASON_LENT) && (rank == Enums::RANKS_MEMORIAL_GENERAL)) || (rank == Enums::RANKS_MEMORIAL_OPTIONAL))) {
			setColour(Enums::COLOURS_RED);
		//}
	}

	//if ((rank > _rank_e) || () ) {
	_sanctorale = readLine.substring(readLine.indexOf(":") + 1);
	_sanctorale.trim();
	//_colour_e = colour;
//	_rank_e = rank;
//	_colour = _I18n->get(_I18n->I18n_COLOURS[_colour_e]);
//	_rank = _I18n->get(_I18n->I18n_RANK_NAMES[_rank_e]);
//	return true;
	
	if (bIsSanctorale) setLectionaryNumber(monthdayandflags);
	
	return bIsSanctorale;


	//}

//	_sanctorale = "";
//	return false;
}

void Sanctorale::setLectionaryNumber(String s) {
	_Lectionary = 0;
	String digits = "0123456789";
	String n = "";
	
	// look for "L" followed immediately by an integer giving the lectionary number, e.g. L697
	int pos = s.indexOf("L");
	if (pos != -1) {
		pos++;
		while (pos < s.length() && digits.indexOf(s.charAt(pos)) != -1) {
			n += s.charAt(pos++);
		}

		_Lectionary = n.toInt();
	}
}

void Sanctorale::setColour(Enums::Colours c) {
	_colour_e = c;
	_colour = _I18n->get(_I18n->I18n_COLOURS[_colour_e]);
	//printf("set colour to %s\n", _colour.c_str());
}

Enums::Colours Sanctorale::getColour(void) {
	return _colour_e;
}

void Sanctorale::setRank(Enums::Ranks r)
{
	_rank_e = r;
	_rank = _I18n->get(_I18n->I18n_RANK_NAMES[_rank_e]);
}

Enums::Ranks Sanctorale::getRank(void) {
	return _rank_e;
}
