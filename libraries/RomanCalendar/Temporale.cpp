//#include "stdafx.h" // comment out for cross-platform/embedded. It doesn't play well with Windows Visual Studio if you wrap it in a #ifdef _WIN32!
#include "Temporale.h"

//const char* const RomanCalendar::DAYS_OF_WEEK[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//const char* const Temporale::SUNDAY_TRANSFERABLE_SOLEMNITIES[3] = { "epiphany", "ascension", "corpus_christi" };

/*
const char* const RomanCalendar::SOLEMNITIES[17] = {
	"The Nativity of the Lord",							//nativity
	"The Holy Family of Jesus, Mary and Joseph",		//holy_family
	"Octave Day of Christmas, of Mary, Mother of God",	//mother_of_god
	"The Epiphany of the Lord",							//epiphany
	"The Baptism of the Lord",							//baptism_of_lord
	"Ash Wednesday",									//ash_wednesday
	"Palm Sunday of the Passion of the Lord",			//palm_sunday
	"Friday of the Passion of the Lord",				//good_friday
	"Holy Saturday",									//holy_saturday
	"Easter Sunday of the Resurrection of the Lord",	//easter_sunday
	"Ascension of the Lord",							//ascension
	"Pentecost Sunday",									//pentecost
	"The Most Holy Trinity",							//holy_trinity
	"The Most Holy Body and Blood of Christ",			//corpus_christi
	"The Most Sacred Heart of Jesus",					//sacred_heart
	"Immaculate Heart of Mary",							//immaculate_heart
	"Our Lord Jesus Christ, King of the Universe"		//christ_king
};
*/
/*
const char* const Temporale::SOLEMNITIES_I18n[17] = {
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
*/
//const char* const Temporale::LITURGICAL_YEARS[3] = { "A", "B", "C" };
//const char* const Temporale::LITURGICAL_CYCLES[2] = { "I", "II" };

const Enums::Ranks Temporale::SOLEMNITIES_RANKS[17] = {		// the ranks of the solemnities, in the same order as in solemnities[17]
	Enums::RANKS_PRIMARY,										//nativity
	Enums::RANKS_FEAST_LORD_GENERAL,							//holy_family
	Enums::RANKS_SOLEMNITY_GENERAL,							//mother_of_god
	Enums::RANKS_PRIMARY,										//epiphany
	Enums::RANKS_FEAST_LORD_GENERAL,							//baptism_of_lord
	Enums::RANKS_PRIMARY,										//ash_wednesday
	Enums::RANKS_PRIMARY,										//palm_sunday
	Enums::RANKS_TRIDUUM,										//good_friday
	Enums::RANKS_TRIDUUM,										//holy_saturday
	Enums::RANKS_TRIDUUM,										//easter_sunday
	Enums::RANKS_PRIMARY,										//ascension
	Enums::RANKS_PRIMARY,										//pentecost
	Enums::RANKS_SOLEMNITY_GENERAL,							//holy_trinity
	Enums::RANKS_SOLEMNITY_GENERAL,							//corpus_christi
	Enums::RANKS_SOLEMNITY_GENERAL,							//sacred_heart
	Enums::RANKS_MEMORIAL_GENERAL,								//immaculate_heart
	Enums::RANKS_SOLEMNITY_GENERAL								//christ_king
};

const Enums::Colours Temporale::SOLEMNITIES_COLOURS[17] = {	// the colours of the solemnities, in the same order as in solemnities[17]
	Enums::COLOURS_WHITE,										//nativity
	Enums::COLOURS_WHITE,										//holy_family
	Enums::COLOURS_WHITE,										//mother_of_god
	Enums::COLOURS_WHITE,										//epiphany
	Enums::COLOURS_WHITE,										//baptism_of_lord
	Enums::COLOURS_VIOLET,										//ash_wednesday
	Enums::COLOURS_RED,											//palm_sunday
	Enums::COLOURS_RED,											//good_friday
	Enums::COLOURS_WHITE,										//holy_saturday
	Enums::COLOURS_WHITE,										//easter_sunday
	Enums::COLOURS_WHITE,										//ascension
	Enums::COLOURS_RED,											//pentecost
	Enums::COLOURS_WHITE,										//holy_trinity
	Enums::COLOURS_WHITE,										//corpus_christi
	Enums::COLOURS_WHITE,										//sacred_heart
	Enums::COLOURS_WHITE,										//immaculate_heart
	Enums::COLOURS_WHITE										//christ_king
};

/*
const char* const RomanCalendar::SEASONS[5] = {
	"Advent",
	"Christmas Season",
	"Lent",
	"Easter Season",
	"Ordinary Time"
};
*/

/*
const char* const RomanCalendar::I18n_SEASONS[5] = {
	"temporale.season.advent",
	"temporale.season.christmas",
	"temporale.season.lent",
	"temporale.season.easter",
	"temporale.season.ordinary"
};
*/
/*
const char* const Temporale::I18n_SEASONS[5] = {
	"advent",
	"christmas",
	"lent",
	"easter",
	"ordinary"
};
*/
/*
const char* const Temporale::RANK_PRIORITY[14] = {
	"4.0", "3.13", "3.12", "3.11", "3.10", "2.9", "2.8", "2.7", "2.6", "2.5", "1.4", "1.3", "1.2", "1.1"
};
*/
/*
const Temporale::RankType Temporale::RANK_TYPE[14] = {
	RANKTYPE_EASTER_TRIDUUM,
	RANKTYPE_PRIMARY_LITURGICAL_DAYS,
	RANKTYPE_SOLEMNITY,
	RANKTYPE_SOLEMNITY,
	RANKTYPE_FEAST,
	RANKTYPE_SUNDAY,
	RANKTYPE_FEAST,
	RANKTYPE_FEAST,
	RANKTYPE_FERIAL,
	RANKTYPE_MEMORIAL,
	RANKTYPE_MEMORIAL,
	RANKTYPE_MEMORIAL_OPT,
	RANKTYPE_FERIAL,
	RANKTYPE_COMMEMORATION
};
*/
/****
const char* const Temporale::I18n_COLOURS[4] = {
	"colour.green",	
	"colour.violet",
	"colour.white",	
	"colour.red"
};

const char* const Temporale::I18n_RANK_NAMES[14] = {
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
****/
/*
const char* const RomanCalendar::RANK_NAME[14] = { // order of array items is significant, since they are indexed by Ranks enum members
	"Commemorations",
	"Ferials",
	"Optional memorials",
	"Proper obligatory memorials",
	"Obligatory memorials in the General Calendar",
	"Privileged ferials",
	"Proper feasts",
	"Feasts of saints in the General Calendar",
	"Unprivileged Sundays",
	"Feasts of the Lord in the General Calendar",
	"Proper solemnities",
	"Solemnities in the General Calendar",
	"Primary liturgical days",
	"Easter triduum"
};
*/
/*
const char* const RomanCalendar::RANK_TYPES[9] = {
	"", // these first two are for easter and primary liturgical days, where no short description is needed
	"",
	"solemnity",
	"feast",
	"Sunday",
	"memorial",
	"optional memorial",
	"ferial",
	"commemoration"
};
*/
/*
const char* const RomanCalendar::SUNDAYS_AND_FERIALS[16] = {
	"%s Sunday of Advent",
	"%s Sunday after the Nativity of the Lord",
	"%s Sunday of Easter",
	"%s Sunday of Lent",
	"%s Sunday in Ordinary Time",
	"%s December",
	"%s day of Christmas Octave",
	"%s after Epiphany",
	"%s after Ash Wednesday",
	"%s of Holy Week",
	"Easter %s"
	"%s, %s week of Advent",
	"%s after Christmas Octave",
	"%s, %s week of Lent",
	"%s, %s week of Easter",
	"%s, %s week in Ordinary Time"
};
*/

Temporale::Temporale(bool transfer_to_sunday, I18n* i ) {
	_transfer_to_sunday = transfer_to_sunday;
	_I18n = i;
	return;
}

Temporale::~Temporale(void) {
	return;
}
/*
const char* const Temporale::I18n_FILENAMES[7] = {
	"daysofweek.txt",
	"seasons.txt",
	"colours.txt",
	"solemnities.txt",
	"ranknames.txt",
	"ranktypes.txt",
	"sundays_ferials.txt"
};
*/
/*
const char* const Temporale::I18n_LANGUAGES[5] = {
	"en",
	"it",
	"fr",
	"la",
	"cs"
};

const char* const Temporale::I18n_SANCTORALE[5] = {
	"universal-en.txt",
	"universal-it.txt",
	"universal-fr.txt",
	"universal-la.txt",
	"universal-cs.txt"
};
*/

/*
void RomanCalendar::loadI18nStrings(void) {
	I18nLanguages l = LANGUAGE_EN;

	readI18nfile(DAYS_OF_WEEK, 7, FILENAMES_SEASONS, l);					//readSeasons();
	readI18nfile(SEASONS, 5, FILENAMES_SEASONS, l);							//readSeasons();
	readI18nfile(COLOURS, 4, FILENAMES_COLOURS, l);							//readColours();
	readI18nfile(SOLEMNITIES, 17, FILENAMES_SOLEMNITIES, l);				//readSolemnities/celebrations();
	readI18nfile(RANK_NAME, 14, FILENAMES_RANKNAMES, l);					//readRanks();
	readI18nfile(RANK_TYPES, 9, FILENAMES_RANKTYPES, l);					//readRankTypes();
	readI18nfile(SUNDAYS_AND_FERIALS, 16, FILENAMES_SUNDAYSFERIALS, l);		//readSundaysFerials();
}

bool RomanCalendar::readI18nfile(char* strArray[], int num, I18nFilenames f, I18nLanguages l) {
	char filename[128];

	sprintf(filename, "locale/%s/%s", I18n_LANGUAGES[l], I18n_FILENAMES[f]);
	
	//FILE* fpi = fopen()
	
	return false; // placeholder
}
*/

// need to make this yml parser work=======================================================================
/*
String Temporale::_I18n->get(String I18nPath) {
	char* filestr;

	I18nPath = I18n_LANGUAGES[_locale] + String(".") + I18nPath;
	String I18nFilename = "locales/" + String(I18n_LANGUAGES[_locale]) + ".yml";

	FILE* fpi = fopen(I18nFilename.c_str(), "r");

	char buf[1024];
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
		filestr = fgets(buf, 1024, fpi);
		if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found

		readLine = String(buf);
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
	} while (filestr != NULL && !bTokenMatched); // to loop, File needs to have returned data and token remained unmatched

	if (!bTokenMatched) { // reached end of file without finding the token
		printf("token %s not found (EOF)\n", lookingforToken.c_str());
		fclose(fpi);
		return String("");
	}

	fclose(fpi);
	
	l = readData.length() - 1;

	if ((readData.charAt(0) == '\'') && (readData.charAt(l) == (const char)'\'')) {	// strip enclosing single quotes
		readData = readData.substring(1, l);
	}

	//printf("Found: read data: %s\n", readData.c_str());
	return readData;
}
*/

/*
bool RomanCalendar::parseline(const char* buf, char* readtokenbuf, char* readtokendata) {
	const char *e;
	int index;

	if ((buf == NULL) || (readtokenbuf == NULL) || (readtokendata == NULL)) return false;

	e = strchr(buf, ':');
	if (e == NULL) return false;
	
	index = (int)(e - buf);

	strncpy(readtokenbuf, buf, index);
	readtokenbuf[index] = '\0';

	e += sizeof(char); // skip over the period, so the returned string points to the start of the next toke, or NULL if at the end of the string

	strcpy(readtokendata, e); // copy the rest of the string to readtokendata: e now points to the remaining characters in the string (after the ':')

	return true;
}
*/

// strstrip from Linux kernel code - see https://stackoverflow.com/questions/1488372/mimic-pythons-strip-function-in-c
/*
char* RomanCalendar::strstrip(char *s)
{
	size_t size;
	char *end;

	size = strlen(s);

	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	while (*s && isspace(*s))
		s++;

	return s;
}

bool RomanCalendar::isspace(int c) {
	switch (c) {
	case ' ':
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
		return true;
		break;
	default:
		return false;
	}
}

String RomanCalendar::removeTemplateVars(String inString) {	// replaces all the occurences of templated yaml variables in the string with %s
	inString.replace("%{week}", "%s");
	inString.replace("%{weekday}", "%s");
	inString.replace("%{day}", "%s");

	return inString;
}

const char* RomanCalendar::getNextToken(char* buf, const char* I18n_path) {
	const char *e;
	int index;

	if ((buf == NULL) || (I18n_path == NULL) || (strlen(I18n_path) == 0)) return NULL;

	e = strchr(I18n_path, '.');
	if (e == NULL) {
		strcpy(buf, I18n_path);
	}
	else {
		index = (int)(e - I18n_path); 

		strncpy(buf, I18n_path, index);

		buf[index] = '\0';

		e += sizeof(char); // skip over the period, so the returned string points to the start of the next toke, or NULL if at the end of the string
	}
	printf("token: %s\n", buf);
	printf("I18n_path: %s\n", I18n_path);
	return e;
}
*/

/*
char* RomanCalendar::getLocaleWeekday(int dayofweek, char* buf) {
	if (dayofweek < 0 || dayofweek > 6) return NULL;

	char ymlpath[128];

	sprintf(ymlpath, "weekday.'%d'", dayofweek);
	strcpy(buf, _I18n->get(ymlpath).c_str());
	return buf;
}
*/

String Temporale::getLocaleWeekday(int dayofweek) {
	if (dayofweek < 0 || dayofweek > 6) return String("");
	return _I18n->get("weekday." + String(dayofweek));
}


void Temporale::datestests() {
#ifdef _WIN32
	printf("\n\ndatestests()\n");

	struct tm ts;
	time_t t;

	bool bResult = getTm(29, 10, 2017, 2, 30, 0, &ts);

	if (bResult) {
		t = tuesday_before(mktime(&ts));
		t = tuesday_after(mktime(&ts));
	}
#endif
}

bool Temporale::getTm(int day, int month, int year, int hours, int minutes, int seconds, struct tm* ts) {
#ifdef _WIN32
	ts->tm_sec = seconds;					/* seconds,  range 0 to 59          */
	ts->tm_min = minutes;					/* minutes, range 0 to 59           */
	ts->tm_hour = hours;					/* hours, range 0 to 23             */
	ts->tm_mday = day;						/* day of the month, range 1 to 31  */
	ts->tm_mon = month - 1;					/* month, range 0 to 11             */ // subtract 1, make it 1-12
	ts->tm_year = year - BEGIN_EPOCH;		/* The number of years since 1900   */
	ts->tm_isdst = -1;

	time_t checkValid = mktime(ts);
	if (checkValid == ((time_t)-1))
	{
		return false;
	}
#endif
	return true;
}

time_t Temporale::date(int day, int month, int year) {
#ifdef _WIN32
	struct tm ts;
	ts.tm_sec = 0;							/* seconds,  range 0 to 59          */
	ts.tm_min = 0;							/* minutes, range 0 to 59           */
	ts.tm_hour = 0;							/* hours, range 0 to 23             */
	ts.tm_mday = day;						/* day of the month, range 1 to 31  */
	ts.tm_mon = month - 1;					/* month, range 0 to 11             */ // subtract 1, make it 1-12
	ts.tm_year = year - BEGIN_EPOCH;		/* The number of years since 1900   */
	ts.tm_isdst = 0;

	return mktime(&ts);
#else
	::tmElements_t ts;						// for arduino
	ts.Second = 0;							/* seconds,  range 0 to 59          */
	ts.Minute = 0;							/* minutes, range 0 to 59           */
	ts.Hour = 0;							/* hours, range 0 to 23             */
	ts.Day = day;							/* day of the month, range 1 to 31  */
	ts.Month = month;						/* month, range 1 to 12             */
	ts.Year = year - BEGIN_EPOCH;			/* The number of years since 1970   */

	return ::makeTime(ts);
#endif
}

time_t Temporale::weekday_before(int weekdayBefore, time_t date) {
	if (weekdayBefore < 0 || weekdayBefore > 6) return (time_t)-1;

#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	int currentWeekday = ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	int currentWeekday = ts.Wday - 1;
#endif`
	//	s->tm_isdst = -1; // set is dst to auto


	int daysBefore = WEEK; // if the date is exactly one week ago

	if (currentWeekday != weekdayBefore) { // otherwise (will be true in most cases)
		daysBefore = currentWeekday - weekdayBefore;
		if (daysBefore < 0) daysBefore += WEEK;
	}

	time_t outputDate = date - (daysBefore * DAY); // subtract number of days (in seconds) from date

												   /*
												   char inDateStr[100];
												   char outDateStr[100];
												   sprintf(inDateStr, "%s", ctime(&date));
												   sprintf(outDateStr,"%s", ctime(&outputDate));
												   printf("Output: the %s before %s is %s\n", DAYS_OF_WEEK[weekdayBefore], inDateStr, outDateStr);
												   */

	return outputDate;

	// bug: when crossing dst, may be ahead or behind by one hour - will not affect actual date.
}

time_t Temporale::sunday_before(time_t date) { return weekday_before(0, date); }
time_t Temporale::monday_before(time_t date) { return weekday_before(1, date); }
time_t Temporale::tuesday_before(time_t date) { return weekday_before(2, date); }
time_t Temporale::wednesday_before(time_t date) { return weekday_before(3, date); }
time_t Temporale::thursday_before(time_t date) { return weekday_before(4, date); }
time_t Temporale::friday_before(time_t date) { return weekday_before(5, date); }
time_t Temporale::saturday_before(time_t date) { return weekday_before(6, date); }

time_t Temporale::weekday_after(int weekdayAfter, time_t date) {
	if (weekdayAfter < 0 || weekdayAfter > 6) return (time_t)-1;

#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	int currentWeekday = ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	int currentWeekday = ts.Wday - 1;
#endif

	int daysAfter = WEEK; // if the date is exactly one week later

	if (currentWeekday != weekdayAfter) { // otherwise (will be true in most cases)
		daysAfter = weekdayAfter - currentWeekday;
		if (daysAfter < 0) daysAfter += WEEK;
	}

	time_t outputDate = date + (daysAfter * DAY);  // add number of days (in seconds) from date

												   /*
												   char inDateStr[100];
												   char outDateStr[100];
												   sprintf(inDateStr, "%s", ctime(&date));
												   sprintf(outDateStr, "%s", ctime(&outputDate));
												   printf("Output: the %s after %s is %s\n", DAYS_OF_WEEK[weekdayAfter], inDateStr, outDateStr);
												   */

	return outputDate;
	// bug: when crossing dst, may be ahead or behind by one hour - will not affect actual date.
}

time_t Temporale::sunday_after(time_t date) { return weekday_after(0, date); }
time_t Temporale::monday_after(time_t date) { return weekday_after(1, date); }
time_t Temporale::tuesday_after(time_t date) { return weekday_after(2, date); }
time_t Temporale::wednesday_after(time_t date) { return weekday_after(3, date); }
time_t Temporale::thursday_after(time_t date) { return weekday_after(4, date); }
time_t Temporale::friday_after(time_t date) { return weekday_after(5, date); }
time_t Temporale::saturday_after(time_t date) { return weekday_after(6, date); }

int Temporale::dayofweek(time_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_wday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	return ts.Wday - 1;
#endif
}

bool Temporale::sunday(time_t date) { return (dayofweek(date) == 0); }
bool Temporale::monday(time_t date) { return (dayofweek(date) == 1); }
bool Temporale::tuesday(time_t date) { return (dayofweek(date) == 2); }
bool Temporale::wednesday(time_t date) { return (dayofweek(date) == 3); }
bool Temporale::thursday(time_t date) { return (dayofweek(date) == 4); }
bool Temporale::friday(time_t date) { return (dayofweek(date) == 5); }
bool Temporale::saturday(time_t date) { return (dayofweek(date) == 6); }

int Temporale::year(time_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_year + BEGIN_EPOCH;
#else
	return ::year(date); // for arduino
#endif
}

int Temporale::month(time_t date)
{
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_mon + 1;
#else
	return ::month(date); // for arduino // check this!! not tested
#endif
}

int Temporale::dayofmonth(time_t date) {
#ifdef _WIN32
	struct tm* ts = gmtime(&date);
	return ts->tm_mday;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date, ts);
	return ts.Day;
#endif
}

bool Temporale::issameday(time_t date1, time_t date2) {
	/*
	printf("date1:");
	print_date(date1);
	printf(": date2:");
	print_date(date2);
	printf("\n\n");
	*/

#ifdef _WIN32
	struct tm* ts = gmtime(&date1);
	int d1 = ts->tm_mday;
	int m1 = ts->tm_mon;
	int y1 = ts->tm_year;

	ts = gmtime(&date2); // there is only one copy of the tm structure, so subsequent calls to gmtime will overwrite the values from earlier calls

	if (ts->tm_mday == d1 && ts->tm_mon == m1 && ts->tm_year == y1) return true;
#else
	::tmElements_t ts;						// for arduino
	::breakTime(date1, ts);

	int d1 = ts.Day;
	int m1 = ts.Month;
	int y1 = ts.Year;

	::breakTime(date2, ts);
	if (ts.Day == d1 && ts.Month == m1 && ts.Year == y1) return true;
#endif

	return false;

}

int Temporale::date_difference(time_t date1, time_t date2) { // assumes that time_t values are seconds since 1970 or 1900, and can have arithmetic
																 //if (date1 >= date2) {								 // performed on them. Not guaranteed on all systems, but should work on arduino embedded.
																 //	return (int)(date1 - date2);
																 //}
																 //else {
	return (int)(date2 - date1);
	//}
}

time_t Temporale::start_date(int year) {
	return first_advent_sunday(year);
}

time_t Temporale::end_date(int year) {
	return first_advent_sunday(year + 1) - DAY;
}

time_t Temporale::first_advent_sunday(int year) {
	return sunday_before(nativity(year)) - (3 * WEEK * DAY);
}

time_t Temporale::nativity(int year) {
	return date(25, 12, year);
}

time_t Temporale::holy_family(int year) {
	time_t xmas = nativity(year);

	if (sunday(xmas)) {
		return date(30, 12, year);
	}
	else {
		return sunday_after(xmas);
	}
}

time_t Temporale::mother_of_god(int year) {
	return octave_of(nativity(year));
}

time_t Temporale::epiphany(int year) {
	if (_transfer_to_sunday) {
		return sunday_after(date(1, 1, year + 1));
	}

	return date(6, 1, year + 1);
}

time_t Temporale::baptism_of_lord(int year) { // maybe watch this, since its had the most modding from the ruby version
	time_t e = epiphany(year);
	if (_transfer_to_sunday) {
		e += DAY;
	}
	else {
		e = sunday_after(e);
	}
	/*
	printf("baptism_of_lord=");
	print_date(e);
	printf("\n");
	*/
	return e;
}

time_t Temporale::ash_wednesday(int year) {
	return easter_sunday(year) - (((6 * WEEK) + 4) * DAY);
}

time_t Temporale::easter_sunday(int year) {
	year += 1;

	//# algorithm below taken from the 'easter' gem:
	//# https ://github.com/jrobertson/easter

	int golden_number = (year % 19) + 1;
	int dominical_number = (year + (year / 4) - (year / 100) + (year / 400)) % 7;
	int solar_correction = (year - 1600) / 100 - (year - 1600) / 400;
	int lunar_correction = (((year - 1400) / 100) * 8) / 25;
	int paschal_full_moon = (3 - 11 * golden_number + solar_correction - lunar_correction) % 30;

	while (!(dominical_number > 0)) {
		dominical_number += 7;
	}

	while (!(paschal_full_moon > 0)) {
		paschal_full_moon += 30;
	}

	if ((paschal_full_moon == 29) || (paschal_full_moon == 28 && golden_number > 11)) {
		paschal_full_moon -= 1;
	}

	int difference = (4 - paschal_full_moon - dominical_number) % 7;
	if (difference < 0) difference += 7;

	int day_easter = paschal_full_moon + difference + 1;
	if (day_easter < 11) {
		//# Easter occurs in March.
		return date(day_easter + 21, 3, year);
	}
	else {
		//# Easter occurs in April.
		return date(day_easter - 10, 4, year);
	}
}

time_t Temporale::palm_sunday(int year) {
	return easter_sunday(year) - (7 * DAY);
}

time_t Temporale::good_friday(int year) {
	return easter_sunday(year) - (2 * DAY);
}

time_t Temporale::holy_saturday(int year) {
	return easter_sunday(year) - (1 * DAY);
}

time_t Temporale::ascension(int year) {
	if (_transfer_to_sunday) {
		return easter_sunday(year) + ((6 * WEEK) * DAY); // return 42 days after Easter sunday
	}

	return pentecost(year) - (10 * DAY); // return ascension Thursday (40 days after Easter sunday)
}

time_t Temporale::pentecost(int year) {
	return easter_sunday(year) + ((7 * WEEK) * DAY);
}

time_t Temporale::holy_trinity(int year) {
	return octave_of(pentecost(year));
}

time_t Temporale::corpus_christi(int year) {
	if (_transfer_to_sunday) {
		return holy_trinity(year) + (WEEK * DAY); // should be 60 days after Easter Sunday if on a Thursday, or 63 days after if transferred to Sunday
	}
	return holy_trinity(year) + (4 * DAY);
}

time_t Temporale::sacred_heart(int year) {
	return corpus_christi(year) + (8 * DAY);
}

time_t Temporale::immaculate_heart(int year) {
	return pentecost(year) + (20 * DAY);
}

time_t Temporale::christ_king(int year) {
	return first_advent_sunday(year) - (7 * DAY);
}

time_t Temporale::octave_of(time_t date) {
	return date + (WEEK * DAY);
}

int Temporale::liturgical_year(time_t date) {
	int _year = year(date);
	if (date < first_advent_sunday(_year)) {
		return _year - 1;
	}

	return _year;
}
/*
Temporale::Liturgical_Year Temporale::liturgical_year_letter(time_t date) {
	int year = liturgical_year(date) + 1;
	
	int r = year % 3; // 0 = C, 1 = A, 2 = B
	switch (r) {
	case 0:
		return Temporale::LITURGICAL_YEAR_C;
		break;
	
	case 1:
		return Temporale::LITURGICAL_YEAR_A;
		break;
	
	case 2:
		return Temporale::LITURGICAL_YEAR_B;
		break;
	}
}

Temporale::Liturgical_Cycle Temporale::liturgical_cycle(time_t date) {
	int year = liturgical_year(date) + 1;

	if ((year % 2) == 1) {
		return Temporale::LITURGICAL_CYCLE_I;	// odd years
	}
	else {
		return Temporale::LITURGICAL_CYCLE_II;	// even years
	}
}
*/

int Temporale::for_day(time_t date) {
	return liturgical_year(date);
}

//# which liturgical season is it ? (returns a 'Season' enum)
Enums::Season Temporale::season(time_t date) {
	int year = Temporale::year(date);

	//printf("season: year is %d\n", year);

	if ((first_advent_sunday(year) <= date) && (nativity(year) > date)) {
		return Enums::SEASON_ADVENT;
	}

	if ((nativity(liturgical_year(date)) <= date) && (baptism_of_lord(liturgical_year(date))) >= date) {
		/*printf("--dates of nativity for date, and baptism of lord\n");
		print_date(date);
		print_date(nativity(liturgical_year(date)));
		print_date(baptism_of_lord(liturgical_year(date)));
		printf("--\n");
		*/
		return Enums::SEASON_CHRISTMAS;
	}

	if ((ash_wednesday(liturgical_year(date)) <= date) && (easter_sunday(liturgical_year(date)) > date)) {
		return Enums::SEASON_LENT;
	}

	if ((easter_sunday(liturgical_year(date)) <= date) && (pentecost(liturgical_year(date)) >= date)) {
		return Enums::SEASON_EASTER;
	}

	return Enums::SEASON_ORDINARY;
}

time_t Temporale::season_beginning(Enums::Season s, time_t date) {
	int year = liturgical_year(date);

	if (s == Enums::SEASON_ADVENT) return first_advent_sunday(year);
	if (s == Enums::SEASON_CHRISTMAS) return nativity(year);
	if (s == Enums::SEASON_LENT) return ash_wednesday(year);
	if (s == Enums::SEASON_EASTER) return easter_sunday(year);
	if (s == Enums::SEASON_ORDINARY) return baptism_of_lord(year) + DAY;

	return (time_t)-1;
}

void Temporale::setColour(Enums::Colours c) {
	_colour_e = c;
	_colour = _I18n->get(_I18n->I18n_COLOURS[_colour_e]);
}

Enums::Colours Temporale::getColour(void) {
	return _colour_e;
}

void Temporale::setRank(Enums::Ranks r) {
	_rank_e = r;
	_rank = _I18n->get(_I18n->I18n_RANK_NAMES[_rank_e]);
}

Enums::Ranks Temporale::getRank(void) {
	return _rank_e;
}

int Temporale::season_week(Enums::Season seasonn, time_t date) {
	int year = Temporale::year(date);

	time_t week1_beginning = season_beginning(seasonn, date);
	int week = 0;

	/*
	if (seasonn == SEASON_ORDINARY) {

	printf("----\n");
	printf("input date: ");
	print_date(date);
	printf(" week1_beginning before munging:");
	print_date(week1_beginning);

	printf("issunday(input date) = %s\n", sunday(date) ? "true":"false");
	printf("issunday(week1_beginning) = %s\n", sunday(week1_beginning) ? "true" : "false");

	}
	*/

	if (!sunday(week1_beginning)) { // Lent begins on Ash Wednesday. For advent, first sunday is in week one, not second sunday
		week1_beginning = sunday_after(week1_beginning);
	}

	if (week1_beginning <= date) {
		week = (date_difference(week1_beginning, date) / (DAY * WEEK)) + 1;
		//printf("date_difference:%d\t", date_difference(week1_beginning, date));
	}
	else {
		week = 0;
		//printf("|");
	}

	if (seasonn == Enums::SEASON_LENT && week == 0) {
		week = sunday(date) ? 1 : 0;
		//printf("*");
	}

	if (seasonn == Enums::SEASON_ORDINARY) {
		//# ordinary time does not begin with Sunday, but the first week
		//# is week 1, not 0
		week += 1;					// first period of ordinary time

									/*
									printf("year: %d", year);
									printf(" pentecost: ");
									print_date(pentecost(year - 1));
									printf(" this date: ");
									print_date(date);
									printf("------------------------------------------------\n");
									*/

		if (date > pentecost(year - 1)) { // second period of ordinary time
										  /*
										  printf("first_advent_sunday = ");
										  print_date(first_advent_sunday(RomanCalendar::year(date)));
										  printf("\t");
										  */
			int weeks_after_date = (date_difference(date, first_advent_sunday(Temporale::year(date)))) / (WEEK * DAY);
			week = 34 - weeks_after_date;
			if (sunday(date)) week += 1;
		}
	}
	/*
	printf("week1_beginning: ");
	print_date(week1_beginning);
	printf("\t");

	printf("season: %s\t week: %d\t", SEASONS[seasonn], week);
	*/
	return week;

	/*
	int year = RomanCalendar::year(date);
	time_t week1_beginning = season_beginning(seasonn, date);

	if (!sunday(week1_beginning)) { // Lent begins on Ash Wednesday, this will set the starting week to the following sunday
	week1_beginning = sunday_after(week1_beginning);
	}

	int week = (date_difference(week1_beginning, date) / (DAY * WEEK)) + 1;

	if (seasonn == SEASON_ORDINARY) {
	week += 1;

	if (date > pentecost(year)) { // second period of ordinary time
	int weeks_after_date = (date_difference(date, first_advent_sunday(RomanCalendar::year(date)))) / (WEEK * DAY);
	week = 34 - weeks_after_date;
	if (sunday(date)) week += 1;
	}
	}

	printf("week == %d\n", week);
	return week;
	*/
}

/*
bool Temporale::sanctorale_get(time_t date, bool move_to_monday) { // in lent and advent, solemnities falling on a sunday are moved to monday.
															     		     // On mondays in lent and advent, need to look back and check if there was a
																			 // solemnity the previous sunday.
	char buf[1024];
	String readtoken;
	String readtokendata;
	char* filestr;

	int m = month(date);
	int d = dayofmonth(date);

	String month_token = "= " + String(m);
	String readLine;

	//I18nPath = I18n_LANGUAGES[_locale] + String(".") + I18nPath;
	String I18nFilename = "data/" + String(I18n_SANCTORALE[_locale]);

	FILE* fpi = fopen(I18nFilename.c_str(), "r");

	// scan for "= <month_number>"
	bool bFound = false;
	do {
		filestr = fgets(buf, 1024, fpi);
		if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found

		readLine = String(buf);
		if (readLine.indexOf(month_token) == 0) {
			bFound = true;
		}
	} while (!bFound && filestr != NULL);

	if (!bFound) return false; // got to the end of the file without finding it

	// scan for string starting with the same number as the day of the month
	bFound = false;
	bool bEndOfMonth = false;
	String daynumber = String(d) + " ";
	do {
		filestr = fgets(buf, 1024, fpi);
		if (filestr == NULL) continue; // EOF: if at end, drop out of loop and check if anything was found

		readLine = String(buf);
		if (readLine.indexOf(daynumber) == 0) {
			bFound = true;
		}

		if (readLine == "" || readLine.indexOf("=") == 0) { // scanned past end of month
			bEndOfMonth = true;
		}
	} while (!bFound && !bEndOfMonth && filestr != NULL);

	if (!bFound || bEndOfMonth) return false; // got to the end of the file without finding it

	String monthdayandflags = readLine.substring(0, readLine.indexOf(":"));
	Season seas = season(date);
	Ranks rank = _rank_e;
	Colours colour = _colour_e;

	if (monthdayandflags.indexOf("s") != -1) {	// solemnity
		if (move_to_monday) {
			if ((seas == SEASON_ADVENT || seas == SEASON_LENT) && sunday(date)) return false; // if a solemnity falls on a sunday in Lent or Advent, it is moved to the Monday after
		}
		rank = RANKS_SOLEMNITY_GENERAL;
		colour = COLOURS_WHITE;
	}
	else if (monthdayandflags.indexOf("m") != -1) {	//memorial
		if (monthdayandflags.indexOf("m2.5") != -1) {
			colour = COLOURS_WHITE;
			rank = RANKS_FEAST_LORD_GENERAL; // rank is 2.5
		}
		else {
			if (seas != SEASON_LENT) {
				colour = COLOURS_WHITE;
				rank = RANKS_MEMORIAL_GENERAL;
			}
			else {
				rank = RANKS_COMMEMORATION;
			}
		}
	}
	else if (monthdayandflags.indexOf("f") != -1) {
		colour = COLOURS_WHITE;
		rank = RANKS_FEAST_GENERAL;
	}
	else {
		if (seas != SEASON_LENT) {
			rank = RANKS_MEMORIAL_OPTIONAL;	// colour is the colour of the season
		}
		else {
			rank = RANKS_COMMEMORATION; // commemorations fall in lent
		}
	}

//	if ((monthdayandflags.indexOf("s") == -1) && (monthdayandflags.indexOf("m") == -1) && (monthdayandflags.indexOf("f") == -1)) {
//		// if none of s, m, or f flags are set, the entry is a commemoration.
//		rank = RANKS_COMMEMORATION;	// colour is the colour of the season
//	}

	if (monthdayandflags.indexOf("R") != -1) {
		if (!(((seas == SEASON_LENT) && (rank == RANKS_MEMORIAL_GENERAL)) || (rank == RANKS_MEMORIAL_OPTIONAL))) {
			colour = COLOURS_RED;
		}
	}

	//if ((rank > _rank_e) || () ) {
		_sanctorale = readLine.substring(readLine.indexOf(":") + 1);
		_sanctorale.trim();
		_sanctorale_colour_e = colour;
		_sanctorale_rank_e = rank;
		return true;
	//}

	//_sanctorale = "";
	//return false;
}
*/

const Enums::Season SEASONS_SUNDAY_PRIMARY[3] = { Enums::SEASON_ADVENT, Enums::SEASON_LENT, Enums::SEASON_EASTER };
String Temporale::sunday_temporale(time_t date) {
	if (!(sunday(date))) return String("");

	Enums::Season seas = season(date);
	Enums::Ranks rank = Enums::RANKS_SUNDAY_UNPRIVILEGED;

	if (seas == Enums::SEASON_ADVENT || seas == Enums::SEASON_LENT || seas == Enums::SEASON_EASTER) {
		rank = Enums::RANKS_PRIMARY;
	}

	int week = season_week(seas, date);
	ordinalize(week);

	//char I18nbuf[1024];
	//sprintf(I18nbuf, "temporale.%s.sunday", SEASONS_I18n[seas]);
	
	//String I18nbuf;
	_day = _I18n->get("temporale." + String(_I18n->I18n_SEASONS[seas]) + ".sunday");
	_day.replace("%{week}", ordinalize(week));
	//_rank = _I18n->get(I18n_RANK_NAMES[rank]);
	_rank_e = rank;
	return _day;
	
	//I18nbuf = "temporale." + String(SEASONS_I18n[seas]) + ".sunday";
	//sprintf(_buffer, _I18n->get(I18nbuf).c_str(), _ordinal);
	/*
	switch (seas)
	{
	case SEASON_ADVENT:
		sprintf(_buffer, SUNDAYS_AND_FERIALS[SUNDAYS_OF_ADVENT], _ordinal); // %s = week
		break;

	case SEASON_CHRISTMAS:
		sprintf(_buffer, SUNDAYS_AND_FERIALS[SUNDAYS_AFTER_NATIVITY], _ordinal); // %s = week
		break;

	case SEASON_EASTER:
		sprintf(_buffer, SUNDAYS_AND_FERIALS[SUNDAYS_OF_EASTER], _ordinal); // %s = week
		break;

	case SEASON_LENT:
		sprintf(_buffer, SUNDAYS_AND_FERIALS[SUNDAYS_OF_LENT], _ordinal); // %s = week
		break;

	case SEASON_ORDINARY:
		sprintf(_buffer, SUNDAYS_AND_FERIALS[SUNDAYS_OF_ORDINARY_TIME], _ordinal); // %s = week
		break;
	}
	
	return _buffer;
	*/
}

String Temporale::ferial_temporale(time_t date) {
	//char I18nbuf[1024];
	//String I18nbuf;
	String weekday = getLocaleWeekday(dayofweek(date));

	Enums::Season seas = season(date);
	int week = season_week(seas, date);
	Enums::Ranks rank = Enums::RANKS_FERIAL;

	bool bIsSet = false;

	switch (seas)
	{
	case Enums::SEASON_ADVENT:
		if (date >= Temporale::date(17, 12, year(date))) {
			rank = Enums::RANKS_FERIAL_PRIVILEGED;
			//ordinalize(dayofmonth(date)); // writes it into the object string variable "_ordinal"
			//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_DECEMBER_BEFORE_CHRISTMAS], _ordinal); // %s = ordinal day of month
			//sprintf(_buffer, _I18n->get("temporale.advent.before_christmas").c_str(), _ordinal);
			
			_day = _I18n->get("temporale.advent.before_christmas");
			_day.replace("%{day}", ordinalize(dayofmonth(date)));
			bIsSet = true;
		}
		break;

	case Enums::SEASON_CHRISTMAS:
		if (date < mother_of_god(liturgical_year(date))) {
			rank = Enums::RANKS_FERIAL_PRIVILEGED;

			//print_date(date);
			//print_date(mother_of_god(liturgical_year(date)));
			//printf("dayofmonth(date) = %d, dayofmonth(nativity() = %d\n", dayofmonth(date), dayofmonth(nativity(year(date))));
			//printf("mother_of_god(%d)=", RomanCalendar::year(date));
			//print_date(mother_of_god(RomanCalendar::year(date)));
			//printf("*\n");

			//ordinalize(dayofmonth(date) - dayofmonth(nativity(RomanCalendar::year(date))) + 1); //# 1 - based counting;
			//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_CHRISTMAS_OCTAVE], _ordinal);
			//sprintf(_buffer, _I18n->get("temporale.christmas.nativity_octave.ferial").c_str(), _ordinal);
			_day = _I18n->get("temporale.christmas.nativity_octave.ferial");
			_day.replace("%{day}", ordinalize(dayofmonth(date) - dayofmonth(nativity(Temporale::year(date))) + 1));
			bIsSet = true;
		}
		else if (date > epiphany(Temporale::year(date))) {
			//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_AFTER_EPIPHANY], DAYS_OF_WEEK[dayofweek(date)]);
			//sprintf(_buffer, _I18n->get("temporale.christmas.after_epiphany.ferial").c_str(), weekday);
			_day = _I18n->get("temporale.christmas.after_epiphany.ferial");
			_day.replace("%{weekday}", weekday);
			bIsSet = true;
		}
		break;

	case Enums::SEASON_LENT:
/*		printf("============= date = ");
		print_date(date);
		printf("\t palm sunday = ");
		print_date(palm_sunday(year(date)));
		printf("year(date) = %d\n", year(date));
		printf("-------------\n");
*/
		if (week == 0) {
			//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_AFTER_ASH_WEDNESDAY], DAYS_OF_WEEK[dayofweek(date)]);
			//sprintf(_buffer, _I18n->get("temporale.lent.after_ashes.ferial").c_str(), weekday);
			_day = _I18n->get("temporale.lent.after_ashes.ferial");
			_day.replace("%{weekday}", weekday);
			bIsSet = true;
		}
		else if (date > palm_sunday(liturgical_year(date))) {
			rank = Enums::RANKS_PRIMARY;
			//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_HOLY_WEEK], DAYS_OF_WEEK[dayofweek(date)]);
			//sprintf(_buffer, _I18n->get("temporale.lent.holy_week.ferial").c_str(), weekday);
			if (!thursday(date)) {
				_day = _I18n->get("temporale.lent.holy_week.ferial");
				_day.replace("%{weekday}", weekday);
			}
			else {
				_day = _I18n->get("temporale.lent.holy_week.thursday");
				_colour_e = Enums::COLOURS_WHITE;
			}
			bIsSet = true;
		}
		rank = (rank > Enums::RANKS_FERIAL_PRIVILEGED) ? rank : Enums::RANKS_FERIAL_PRIVILEGED; // watch this - comparison is dependent on the in which each enum member represents, which have been chosen to make this work
		break;

	case Enums::SEASON_EASTER:
		if (week == 1) {
			rank = Enums::RANKS_PRIMARY;
			//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_EASTER_WEEK], DAYS_OF_WEEK[dayofweek(date)]);
			//sprintf(_buffer, _I18n->get("temporale.easter.octave.ferial").c_str(), weekday);
			_day = _I18n->get("temporale.easter.octave.ferial");
			_day.replace("%{weekday}", weekday);
			bIsSet = true;
		}
		break;
	}

	//_rank = _I18n->get(I18n_RANK_NAMES[rank]);
	_rank_e = rank;

	if (bIsSet) return _day;

	//sprintf(I18nbuf, "temporale.%s.ferial", I18n_SEASONS[seas]);
	//sprintf(_buffer, _I18n->get(I18nbuf).c_str(), weekday);

	_day = _I18n->get("temporale." + String(_I18n->I18n_SEASONS[seas]) + ".ferial");
	_day.replace("%{weekday}", weekday);
	_day.replace("%{week}", ordinalize(week));
	
	return _day;
	/*
	switch (seas)
	{
	case SEASON_ADVENT:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_ADVENT], DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		sprintf(_buffer, _I18n->get(I18nbuf).c_str(), weekday, _ordinal);
		break;

	case SEASON_CHRISTMAS:
		//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_AFTER_CHRISTMAS_OCTAVE], DAYS_OF_WEEK[dayofweek(date)]); // %s1 = day of week
		sprintf(_buffer, _I18n->get(I18nbuf).c_str(), weekday);
		break;

	case SEASON_LENT:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_LENT], DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		sprintf(_buffer, _I18n->get(I18nbuf).c_str(), weekday, _ordinal);
		break;

	case SEASON_EASTER:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_EASTER], DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		sprintf(_buffer, _I18n->get(I18nbuf).c_str(), weekday, _ordinal);
		break;

	case SEASON_ORDINARY:
		ordinalize(week); // writes it into the object string variable "_ordinal"
		//sprintf(_buffer, SUNDAYS_AND_FERIALS[DAYS_OF_ORDINARY_TIME], DAYS_OF_WEEK[dayofweek(date)], _ordinal); // %s1 = day of week, %s2 ordinal week of season
		sprintf(_buffer, _I18n->get(I18nbuf).c_str(), weekday, _ordinal);
		break;
	}
	
	return _buffer;
	*/
}

bool Temporale::do_solemnities(time_t date) {
	//Serial.println("Temporale::do_solemnities()");

	int year = liturgical_year(date);
	bool bIsSolemnity = false;
	Enums::Solemnities s;

//	_liturgical_year = String(LITURGICAL_YEARS[liturgical_year_letter(date)]);
//	_liturgical_cycle = String(LITURGICAL_CYCLES[liturgical_cycle(date)]);

	//if (issameday(date, first_advent_sunday(year)) {s = ;
	if (issameday(date, nativity(year))) { s = Enums::SOLEMNITIES_NATIVITY; bIsSolemnity = true; }
	if (issameday(date, holy_family(year))) { s = Enums::SOLEMNITIES_HOLY_FAMILY; bIsSolemnity = true; }
	if (issameday(date, mother_of_god(year))) { s = Enums::SOLEMNITIES_MOTHER_OF_GOD;  bIsSolemnity = true; }
	if (issameday(date, epiphany(year))) { s = Enums::SOLEMNITIES_EPIPHANY;  bIsSolemnity = true; }
	if (issameday(date, baptism_of_lord(year))) { s = Enums::SOLEMNITIES_BAPTISM_OF_LORD;  bIsSolemnity = true; }
	if (issameday(date, ash_wednesday(year))) { s = Enums::SOLEMNITIES_ASH_WEDNESDAY;  bIsSolemnity = true; }
	if (issameday(date, palm_sunday(year))) { s = Enums::SOLEMNITIES_PALM_SUNDAY;  bIsSolemnity = true; }
	if (issameday(date, good_friday(year))) { s = Enums::SOLEMNITIES_GOOD_FRIDAY;  bIsSolemnity = true; }
	if (issameday(date, holy_saturday(year))) { s = Enums::SOLEMNITIES_HOLY_SATURDAY;  bIsSolemnity = true; }
	if (issameday(date, easter_sunday(year))) { s = Enums::SOLEMNITIES_EASTER_SUNDAY;  bIsSolemnity = true; }
	if (issameday(date, ascension(year))) { s = Enums::SOLEMNITIES_ASCENSION;  bIsSolemnity = true; }
	if (issameday(date, pentecost(year))) { s = Enums::SOLEMNITIES_PENTECOST;  bIsSolemnity = true; }
	if (issameday(date, holy_trinity(year))) { s = Enums::SOLEMNITIES_HOLY_TRINITY;  bIsSolemnity = true; }
	if (issameday(date, corpus_christi(year))) { s = Enums::SOLEMNITIES_CORPUS_CHRISTI;  bIsSolemnity = true; }
	if (issameday(date, sacred_heart(year))) { s = Enums::SOLEMNITIES_SACRED_HEART;  bIsSolemnity = true; }
	if (issameday(date, immaculate_heart(year))) { s = Enums::SOLEMNITIES_IMMACULATE_HEART;  bIsSolemnity = true; }
	if (issameday(date, christ_king(year))) { s = Enums::SOLEMNITIES_CHRIST_KING;  bIsSolemnity = true; }

	if (bIsSolemnity) {
		_rank_e = SOLEMNITIES_RANKS[s];
		_colour_e = SOLEMNITIES_COLOURS[s];
		//_colour = _I18n->get(I18n_COLOURS[_colour_e]);
		_day = _I18n->get(_I18n->I18n_SOLEMNITIES[s]);
	}

	return bIsSolemnity;
}
/*
bool Temporale::do_feasts_and_memorials(time_t date) {
	Season seas = season(date);
	bool b_is_feast_or_memorial = false;

	if ((seas == SEASON_ADVENT || seas == SEASON_LENT)) {
		if (monday(date)) { // if on monday, need to back-check to see if there was a solemnity on the day before
							// if a solemnity falls on a sunday in Lent or Advent, it is moved to the Monday after, so need to check if this happened the previous day 
			if (!(feasts_and_memorials(sunday_before(date), false))) { // call with the flag "move to monday" set to false, for the sunday before the monday.
				b_is_feast_or_memorial = feasts_and_memorials(date, true); // if there wasn't a memorial on the sunday, check for today (monday) instead
			}
		}
		else {
			b_is_feast_or_memorial = feasts_and_memorials(date, true); // not on a monday, but in Advent or Lent, so move to monday should be set.
		}
	}
	else {
		b_is_feast_or_memorial = feasts_and_memorials(date, false); // if not in Advent or Lent, check for today instead, with move_to_monday set to false, since memorials can occur on sundays outside this Lent and Advent
	}

	return b_is_feast_or_memorial;
}
*/

bool Temporale::do_sundays(time_t date) {
	//Serial.println("Temporale::do_sundays()");
	if (sunday(date)) {
		sunday_temporale(date);
		return true;
	}
	return false;
}

bool Temporale::do_ferials(time_t date) {
	//Serial.println("Temporale::do_ferials()");
	
	if (!sunday(date)) {
		ferial_temporale(date);
		return true;
	}

	return false;

}

bool Temporale::get(time_t date) {
	//Serial.print("Temporale::get() ");
	//print_date(date);
	//Serial.println();
	
	_day = "";
	_rank = "";
	_season = "";
	//_sanctorale = "";
	_colour = "";
	//_liturgical_year = "";
	//_liturgical_cycle = "";

	Enums::Season seas = season(date);
	_season = _I18n->get("temporale.season." + String(_I18n->I18n_SEASONS[seas]));

	switch (seas) {
	case Enums::SEASON_ADVENT:
	case Enums::SEASON_LENT:
		_colour_e = Enums::COLOURS_VIOLET;
		break;

	case Enums::SEASON_CHRISTMAS:
	case Enums::SEASON_EASTER:
		_colour_e = Enums::COLOURS_WHITE;
		break;

	case Enums::SEASON_ORDINARY:
		_colour_e = Enums::COLOURS_GREEN;
		break;

	default:
		printf("Colours:Season not set!\n");
		_colour_e = Enums::COLOURS_GREEN;
	}

	do_ferials(date);
	do_sundays(date);
	_bIsSolemnity = do_solemnities(date);
/*
	bool isSanctorale = false;
	if ((seas == SEASON_ADVENT || seas == SEASON_LENT)) {
		if (monday(date)) { // if on monday, need to back-check to see if there was a solemnity on the day before
							// if a solemnity falls on a sunday in Lent or Advent, it is moved to the Monday after, so need to check if this happened the previous day 
			if (!(sanctorale_get(sunday_before(date), false))) { // call with the flag "move to monday" set to false, for the sunday before the monday.
				isSanctorale = sanctorale_get(date, true); // if there wasn't a memorial on the sunday, check for today (monday) instead
			}
		}
		else { // Day in Lent or Advent not monday
			isSanctorale = sanctorale_get(date, true); // not on a monday, but in Advent or Lent, so move to monday should be set.
		}
	}
	else { // for seasons other than Advent or Lent
		isSanctorale = sanctorale_get(date, false); // if not in Advent or Lent, check for today instead, with move_to_monday set to false, since memorials can occur on sundays outside this Lent and Advent
	}
*/

	_colour = _I18n->get(_I18n->I18n_COLOURS[_colour_e]);
	_rank = _I18n->get(_I18n->I18n_RANK_NAMES[_rank_e]);

	return _day;
}


/*
String RomanCalendar::liturgical_day(time_t date) {
	_day = "";
	_rank = "";
	_season = "";
	_celebration = "";
	_colour = "";
	_liturgical_year = "";
	_liturgical_cycle = "";
	
	Season seas = season(date);
	_season = _I18n->get("temporale.season." + String(I18n_SEASONS[seas]));

	switch (seas) {
	case SEASON_ADVENT:
	case SEASON_LENT:
		_colour_e = COLOURS_VIOLET;
		break;

	case SEASON_CHRISTMAS:
	case SEASON_EASTER:
		_colour_e = COLOURS_WHITE;
		break;

	case SEASON_ORDINARY:
		_colour_e = COLOURS_GREEN;
		break;

	default:
		printf("Colours:Season not set!\n");
		_colour_e = COLOURS_GREEN;
	}

	int year = liturgical_year(date);
	bool bIsSolemnity = false;
	Solemnities s;

	_liturgical_year = String(LITURGICAL_YEARS[liturgical_year_letter(date)]);
	_liturgical_cycle = String(LITURGICAL_CYCLES[liturgical_cycle(date)]);
	
	//if (issameday(date, first_advent_sunday(year)) {s = ;
	if (issameday(date, nativity(year))) { s = SOLEMNITIES_NATIVITY; bIsSolemnity = true; }
	if (issameday(date, holy_family(year))) { s = SOLEMNITIES_HOLY_FAMILY; bIsSolemnity = true; }
	if (issameday(date, mother_of_god(year))) { s = SOLEMNITIES_MOTHER_OF_GOD;  bIsSolemnity = true; }
	if (issameday(date, epiphany(year))) { s = SOLEMNITIES_EPIPHANY;  bIsSolemnity = true; }
	if (issameday(date, baptism_of_lord(year))) { s = SOLEMNITIES_BAPTISM_OF_LORD;  bIsSolemnity = true; }
	if (issameday(date, ash_wednesday(year))) { s = SOLEMNITIES_ASH_WEDNESDAY;  bIsSolemnity = true; }
	if (issameday(date, palm_sunday(year))) { s = SOLEMNITIES_PALM_SUNDAY;  bIsSolemnity = true; }
	if (issameday(date, good_friday(year))) { s = SOLEMNITIES_GOOD_FRIDAY;  bIsSolemnity = true; }
	if (issameday(date, holy_saturday(year))) { s = SOLEMNITIES_HOLY_SATURDAY;  bIsSolemnity = true; }
	if (issameday(date, easter_sunday(year))) { s = SOLEMNITIES_EASTER_SUNDAY;  bIsSolemnity = true; }
	if (issameday(date, ascension(year))) { s = SOLEMNITIES_ASCENSION;  bIsSolemnity = true; }
	if (issameday(date, pentecost(year))) { s = SOLEMNITIES_PENTECOST;  bIsSolemnity = true; }
	if (issameday(date, holy_trinity(year))) { s = SOLEMNITIES_HOLY_TRINITY;  bIsSolemnity = true; }
	if (issameday(date, corpus_christi(year))) { s = SOLEMNITIES_CORPUS_CHRISTI;  bIsSolemnity = true; }
	if (issameday(date, sacred_heart(year))) { s = SOLEMNITIES_SACRED_HEART;  bIsSolemnity = true; }
	if (issameday(date, immaculate_heart(year))) { s = SOLEMNITIES_IMMACULATE_HEART;  bIsSolemnity = true; }
	if (issameday(date, christ_king(year))) { s = SOLEMNITIES_CHRIST_KING;  bIsSolemnity = true; }

	if (bIsSolemnity) {
		_rank = _I18n->get(I18n_RANK_NAMES[SOLEMNITIES_RANKS[s]]);
		_colour_e = SOLEMNITIES_COLOURS[s];
		_colour = _I18n->get(I18n_COLOURS[_colour_e]);
		_day = _I18n->get(SOLEMNITIES_I18n[s]);
		return _day;
	}

	_day = sunday_temporale(date);

	if (_day == "") {
		_day = ferial_temporale(date);
	}

	if ((seas == SEASON_ADVENT || seas == SEASON_LENT)) {
		if (monday(date)) { // if on monday, need to back-check to see if there was a solemnity on the day before
			// if a solemnity falls on a sunday in Lent or Advent, it is moved to the Monday after, so need to check if this happened the previous day 
			if (!(feasts_and_memorials(sunday_before(date), false))) { // call with the flag "move to monday" set to false, for the sunday before the monday.
				feasts_and_memorials(date, true); // if there wasn't a memorial on the sunday, check for today (monday) instead
			}
		}
		else {
			feasts_and_memorials(date, true); // not on a monday, but in Advent or Lent, so move to monday should be set.
		}
	}
	else {
		feasts_and_memorials(date, false); // if not in Advent or Lent, check for today instead, with move_to_monday set to false, since memorials can occur on sundays outside this Lent and Advent
	}

	_colour = _I18n->get(I18n_COLOURS[_colour_e]);
	_rank = _I18n->get(I18n_RANK_NAMES[_rank_e]);

	return _day;


	//return _buffer;
}
*/

bool Temporale::includes(const char* s, const char* const strarray[]) {
	int i = 0;
	while (strarray[i] != NULL) {
		if (strcmp(strarray[i], s) == 0) {
			return true;
			break;
		}
		i++;
	}
	return false;
}

void Temporale::temporaletests() {
#ifdef _WIN32
	printf("\n\ntemporaletests()\n");
	//printf("epiphany is in SUNDAY_TRANSFERABLE_SOLEMNITIES[]: %s\n", includes("epiphany", SUNDAY_TRANSFERABLE_SOLEMNITIES) ? "true" : "false");
	//printf("christmas is in SUNDAY_TRANSFERABLE_SOLEMNITIES[]: %s\n", includes("christmas", SUNDAY_TRANSFERABLE_SOLEMNITIES) ? "true" : "false");

	//easter_tests();
	//return;
	//epiphany_tests();
/*	
	time_t t;
	struct tm* ts;

	char datetime[128];

	int y = 2017;

	FILE* fpo = fopen(".\\liturgical-calendar.csv", "w");

	if (!fpo) {
		printf("unable to open file for output\n");
		return;
	}

	for (int m = 1; m <= 12; m++) {
		int days = get_monthdays(m, y);

		for (int d = 1; d <= days; d++) {
			t = date(d, m, y);
			if (t != (time_t)-1) {
				ts = gmtime(&t);
				strftime(datetime, 128, "%d/%m/%Y", ts);
				get(t);
				fprintf(fpo, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n", datetime, _season.c_str(), _day.c_str(), _colour.c_str(), _rank.c_str(), _celebration.c_str());
				//printf("%s\t%s\t%s\t%s\t%s\n", datetime, _liturgical_year.c_str(), _liturgical_cycle.c_str(), _season.c_str(), _day.c_str());
				if (_celebration != "") printf("%s\n", _celebration.c_str());
				printf("Colour: %s\t Rank: %s\t\n\n", _colour.c_str(), _rank.c_str());
			}
		}
	}
	
	fclose(fpo);
	printf("wrote csv\n");
*/
#else
#endif
}

int Temporale::get_monthdays(int mon, int year) {
	static const int days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int leap = yisleap(year) ? 1 : 0;

	return days[mon] + leap;
}

bool Temporale::yisleap(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void Temporale::easter_tests() {
	time_t t;

	for (int y = 1970; y < 2038; y++) {
		t = date(1, 8, y);
		print_date(t);
		printf(": Easter sunday is on ");
		print_date(easter_sunday(y));
		printf(" is sunday: %s", sunday(easter_sunday(y)) ? "true" : "false");
		printf("\n");
	}
}

void Temporale::epiphany_tests(void) {
	time_t t;
	for (int y = 1970; y < 2038; y++) {
		t = epiphany(y);
		print_date(t);
		printf("\n");
	}
}

String Temporale::ordinalize(int number) {
	int modulo = number % 10;
	if ((number / 10) == 1) modulo = 9;

	String ord;

	switch (modulo) {
	case 1:
		ord = "st";
		break;

	case 2:
		ord = "nd";
		break;

	case 3:
		ord = "rd";
		break;

	default:
		ord = "th";
	}

	ord = String(number) + ord;
	return ord;
}

void Temporale::print_date(time_t t) {
#ifdef _WIN32
	struct tm* ts;

	char buffer[128];

	ts = gmtime(&t);
	strftime(buffer, 128, "%Y-%m-%d", ts);
	printf("%s ", buffer);
#else
	::tmElements_t ts;						// for arduino
	::breakTime(t, ts);

	Serial.print(ts.Day);
	Serial.print("-");
	Serial.print(ts.Month);
	Serial.print("-");
	Serial.print(ts.Year + BEGIN_EPOCH);
#endif
}