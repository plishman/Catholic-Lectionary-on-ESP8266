//#include "stdafx.h"
#include "Calendar.h"

const char* const Calendar::I18n_SEASONS[5] = {
	"advent",
	"christmas",
	"lent",
	"easter",
	"ordinary"
};

const char* const Calendar::I18n_LANGUAGES[5] = {
	"en",
	"it",
	"fr",
	"la",
	"cs"
};

const char* const Calendar::I18n_COLOURS[4] = {
	"colour.green",
	"colour.violet",
	"colour.white",
	"colour.red"
};

const char* const Calendar::I18n_RANK_NAMES[14] = {
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

const char* const Calendar::LITURGICAL_YEARS[3] = { "A", "B", "C" };
const char* const Calendar::LITURGICAL_CYCLES[2] = { "I", "II" };

#ifndef _WIN32
Calendar::Calendar(int CS_PIN) {
	//_config = new Config();	
	config_t c = {0};
	if (!Config::GetConfig(c)) {
		DEBUG_PRT.println(F("GetConfig returned false"));
		_lectionary_config_number = 0;
		_timezone_offset = 0;
	}
	else {
		_timezone_offset = c.data.timezone_offset;
		_lectionary_config_number = c.data.lectionary_config_number; // is the line number of the entry in config.csv, specifies the Bible, language and sanctorale files to use
	}
	
	_CS_PIN = CS_PIN;
	_I18n = new I18n(_CS_PIN, _lectionary_config_number);
	
	//_config->_I18n = _I18n;
#else
Calendar::Calendar() {	
	_I18n = new I18n();
#endif
	_date = (time64_t)-1;
	_transfer_to_sunday = _I18n->configparams.transfer_to_sunday;
/*
	transfers = new Transfers(_transfer_to_sunday, _I18n);		// PLL 29-04-2020 (commented out 3 lines, these objects now created when get() is called)
	temporale = new Temporale(_transfer_to_sunday, _I18n);
	sanctorale = new Sanctorale(_transfer_to_sunday, _I18n);
*/	
	DEBUG_PRT.println(F("Created new calendar object"));
}

Calendar::~Calendar() {
	if (_I18n != NULL) delete _I18n;
//#ifndef _WIN32
//	if (_config != NULL) delete _config;
//#endif
	if (transfers != NULL) delete transfers;
	if (temporale != NULL) delete temporale;
	if (sanctorale != NULL) delete sanctorale;
}

bool Calendar::get(time64_t date) { // date passed in will now be local time, so do not need to add/subtract the timezone offset, it is already done (03-07-2018)
	if (date < 3600 * 24 * 365) return false; // need some overhead. The liturgical calendar starts in 1970 (time_t value == 0), but the first season (Advent) begins in 1969, 
											  // which is outside the range of a time64_t value, and will occur in calculations for year 1970 if not trapped
	
//	int tz_offset = (int) (_timezone_offset * 3600);
	DEBUG_PRT.print(F("Timezone offset is "));
	DEBUG_PRT.println(String(_timezone_offset));

//	DEBUG_PRT.print("The UTC datetime is ");
//	temporale->print_date(date);
//	DEBUG_PRT.print(" ");
//	temporale->print_time(date);
//	DEBUG_PRT.println();
	
//	date += tz_offset; // quick and dirty - if bug occurs, need to split the time64_t value into a TMelements_t struct, and perform the arithmetic.
	
	DEBUG_PRT.print(F("The local time is "));
	Temporale::print_date(date);
	DEBUG_PRT.print(" ");
	Temporale::print_time(date);
	DEBUG_PRT.println();

	DEBUG_PRT.println(F("Creating Transfers, Temporale and Sanctorale objects..."));
	transfers = new Transfers(_transfer_to_sunday, _I18n);		//PLL 29-04-2020 moved these lines from the constructor to here, so that the calendar object can be created
	DEBUG_PRT.print(F("Transfers.."));
	temporale = new Temporale(_transfer_to_sunday, _I18n);		//earlier in the program, without using the memory these three objects take up when they're initialized
	DEBUG_PRT.print(F("Temporale.."));
	sanctorale = new Sanctorale(_transfer_to_sunday, _I18n);	//The _I18n object is however still created in the constructor, so that it can be used by the main program to get the language setting.
	DEBUG_PRT.print(F("Sanctorale.."));
	DEBUG_PRT.println(F("Done."));

	//bool bTransfersSuccess = transfers->get(date);
	
	//DEBUG_PRT.println("Calendar::get()");
	
	//bool bTransfersSuccess = transfers->get(date);
	bool bTemporaleSuccess = temporale->get(date);
	if (!bTemporaleSuccess) {
		DEBUG_PRT.println(F("get temporale failed!"));
		return false;
	}

	DEBUG_PRT.println(F("got temporale"));

	time64_t transferred_from;
	bool bWasTransferred = transfers->transferred_to(date, &transferred_from);

	DEBUG_PRT.print(F("bWasTransferred = "));
	DEBUG_PRT.println(bWasTransferred);

	bool bIsSanctorale = false;

	if (!bWasTransferred) {
		bIsSanctorale = sanctorale->get(date);
		DEBUG_PRT.println(F("got sanctorale"));
	}
	else {
		bIsSanctorale = sanctorale->get(transferred_from);
		DEBUG_PRT.println(F("got sanctorale (transferred)"));
	}

	DEBUG_PRT.print(F("bIsSanctorale = "));
	DEBUG_PRT.println(bIsSanctorale);

	int lit_year = temporale->liturgical_year(date);

	day.liturgical_year = LITURGICAL_YEARS[liturgical_year_letter(lit_year)];
	day.liturgical_cycle = LITURGICAL_CYCLES[liturgical_cycle(lit_year)];

	day.is_sanctorale = bIsSanctorale;

	Enums::Ranks rank_t = temporale->getRank();
	Enums::Ranks rank_s = sanctorale->getRank();
	Enums::Season seas = temporale->season(date);

	//DEBUG_PRT.println("Checking day");
	
	if (bIsSanctorale) { // there is a sanctorale for this day. All conflicting Solemnities will already have been moved
		if (rank_s > rank_t) { // sanctorale's rank is above temporale's rank
			if (rank_s == Enums::RANKS_MEMORIAL_OPTIONAL) {
				// only ranks below this are Ferial days and Commemorations, which means this memorial is to be displayed
				// return st.dup.unshift t (what does this do?)
				sanctorale->setColour(temporale->getColour()); //Optional memorials have colours of the season
			}
			else {
				//return st
				temporale->setColour(sanctorale->getColour()); // Otherwise, the colour of the sanctorale is used
			}
		}
		else {
			day.is_sanctorale = false; // suppress sanctorale if its rank is lower than that of the temporale day.
		}
		
		if (rank_t == Enums::RANKS_FERIAL_PRIVILEGED && (rank_s == Enums::RANKS_MEMORIAL_GENERAL || rank_s == Enums::RANKS_MEMORIAL_OPTIONAL)) {
			// in advent and lent, memorials become commemorations
			//sanctorale->_rank_e = Enums::RANKS_COMMEMORATION; // need to change this with a getter/setter so that changes to _rank_e update the _rank variable
			if (seas == Enums::SEASON_LENT) { // some sites (romcal) say that opt mems and mems are reduced to commems only in lent, others say also in advent
				sanctorale->setRank(Enums::RANKS_COMMEMORATION); 
				sanctorale->setColour(temporale->getColour()); // colour of memorials is colour of the season
			}
			else if (seas == Enums::SEASON_ADVENT && rank_s == Enums::RANKS_MEMORIAL_OPTIONAL) {
				sanctorale->setColour(temporale->getColour()); //Optional memorials have colours of the season
			}
			day.is_sanctorale = true; // suppress sanctorale if its rank is lower than that of the temporale day.
		}
		
		if (temporale->issameday(date, temporale->immaculate_heart(lit_year)) && (sanctorale->_rank_e == Enums::RANKS_MEMORIAL_GENERAL || sanctorale->_rank_e == Enums::RANKS_MEMORIAL_PROPER)) {
			// a sanctorale memorial proper or memorial general falls on the same day as immaculate heart, so change the rank to optional memorial
			sanctorale->setRank(Enums::RANKS_MEMORIAL_OPTIONAL); // change the conflicting proper or general memorial to optional.
			temporale->setRank(Enums::RANKS_MEMORIAL_OPTIONAL); // change immaculate heart to an optional memorial
		}
		
		day.sanctorale = sanctorale->_sanctorale;
		day.sanctorale_rank = sanctorale->_rank;
		day.sanctorale_colour = sanctorale->_colour;
	}

	day.date = date;
	day.colour = temporale->_colour;
	day.day = temporale->_day;
	day.rank = temporale->_rank;
	day.season = temporale->_season;
	
	if (day.is_sanctorale) {
		day.is_holy_day_of_obligation = sanctorale->_hdo;
		day.holy_day_of_obligation = sanctorale->_holy_day_of_obligation;
	}
	else {
		day.is_holy_day_of_obligation = temporale->_hdo;
		day.holy_day_of_obligation = temporale->_holy_day_of_obligation;
	}
	
	if (day.is_holy_day_of_obligation) {
		DEBUG_PRT.println("Holy day of obligation");
	}
	
	if (sanctorale->_Lectionary == 0) {
		day.lectionary = temporale->_Lectionary;
		DEBUG_PRT.print(F("Lectionary number is from temporale, L"));
	} else {
		day.lectionary = sanctorale->_Lectionary;
		DEBUG_PRT.print(F("Lectionary number is from sanctorale, L"));
	}
	DEBUG_PRT.println(String(day.lectionary));
	
	return true;
}

/*
    def celebrations_for(date)
      tr = @transferred.get(date)
      return [tr] if tr

      t = @temporale.get date
      st = @sanctorale.get date

      unless st.empty?
        if st.first.rank > t.rank
          if st.first.rank == Ranks::MEMORIAL_OPTIONAL
            return st.dup.unshift t
          else
            return st
          end
        elsif t.rank == Ranks::FERIAL_PRIVILEGED && st.first.rank.memorial?
          st = st.collect do |c|
            Celebration.new(c.title, Ranks::COMMEMORATION, t.colour)
          end
          st.unshift t
          return st
        elsif t.symbol == :immaculate_heart &&
              [Ranks::MEMORIAL_GENERAL, Ranks::MEMORIAL_PROPER].include?(st.first.rank)
          optional_memorials = ([t] + st).collect do |celebration|
            celebration.change rank: Ranks::MEMORIAL_OPTIONAL
          end
          ferial = temporale.send :ferial, date # ugly and evil
          return [ferial] + optional_memorials
        end
      end

      return [t]
    end
*/

Enums::Liturgical_Year Calendar::liturgical_year_letter(int year) {
	//int year = liturgical_year(date) + 1;

	int r = year % 3; // 0 = C, 1 = A, 2 = B

	return (Enums::Liturgical_Year) r;
}

Enums::Liturgical_Cycle Calendar::liturgical_cycle(int year) {
	//int year = liturgical_year(date) + 1;

	int r = year % 2;

	return (Enums::Liturgical_Cycle) r;
}
