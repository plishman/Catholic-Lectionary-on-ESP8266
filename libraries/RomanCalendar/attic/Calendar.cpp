#include "stdafx.h"
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

Calendar::Calendar(bool transfer_to_sunday, Enums::I18nLanguages l) {
	_locale = l;
	_date = (time64_t)-1;
	_transfer_to_sunday = transfer_to_sunday;
	_I18n = new I18n(_locale);

	transfers = new Transfers(_transfer_to_sunday, _I18n);
	temporale = new Temporale(_transfer_to_sunday, _I18n);
	sanctorale = new Sanctorale(_transfer_to_sunday, _I18n);
}

Calendar::~Calendar() {
	if (transfers) delete transfers;
	if (temporale) delete temporale;
	if (sanctorale) delete sanctorale;
}

bool Calendar::get(time64_t date) {
	//bool bTransfersSuccess = transfers->get(date);
	bool bTemporaleSuccess = temporale->get(date);
	if (!bTemporaleSuccess) return false;

	time64_t transferred_from = transfers->transferred_to(date);

	bool bIsSanctorale = false;
	if (transferred_from == (time64_t)-1) {
		bIsSanctorale = sanctorale->get(date);
	}
	else {
		bIsSanctorale = sanctorale->get(transferred_from);
	}

	int lit_year = temporale->liturgical_year(date);

	day.liturgical_year = LITURGICAL_YEARS[liturgical_year_letter(lit_year)];
	day.liturgical_cycle = LITURGICAL_CYCLES[liturgical_cycle(lit_year)];

	day.is_sanctorale = bIsSanctorale;

	if (bIsSanctorale) { // there is a sanctorale for this day. All conflicting Solemnities will already have been moved
		if (sanctorale->getRank() > temporale->getRank()) { // sanctorale's rank is above temporale's rank
			if (sanctorale->getRank() == Enums::RANKS_MEMORIAL_OPTIONAL) {
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
		
		if (temporale->getRank() == Enums::RANKS_FERIAL_PRIVILEGED && (sanctorale->getRank() == Enums::RANKS_MEMORIAL_GENERAL || sanctorale->getRank() == Enums::RANKS_MEMORIAL_OPTIONAL)) {
			// in advent and lent, memorials become commemorations
			//sanctorale->_rank_e = Enums::RANKS_COMMEMORATION; // need to change this with a getter/setter so that changes to _rank_e update the _rank variable
			sanctorale->setRank(Enums::RANKS_COMMEMORATION);
			sanctorale->setColour(temporale->getColour()); // colour of memorials is colour of the season
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
