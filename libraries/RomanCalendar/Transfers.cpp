//#include "stdafx.h"
#include "Transfers.h"

Transfers::Transfers(bool transfer_to_sunday, I18n* i) {
	_I18n = i;
	_transfer_to_sunday = transfer_to_sunday;
	_tc = new Temporale(_transfer_to_sunday, _I18n);
	_sc = new Sanctorale(_transfer_to_sunday, _I18n);
}

Transfers::~Transfers() {
	transfersList.clear();
	if (_tc != NULL ) { delete _tc; I2CSerial.println(F("Transfers:~Transfers: deleted _tc")); }
	if (_sc != NULL ) { delete _sc; I2CSerial.println(F("Transfers:~Transfers: deleted _sc")); }
	_year = 0;
}

bool Transfers::do_transfers(time64_t date) {
	//I2CSerial.println("Transfers::do_transfers()");
	
	int year = _tc->liturgical_year(date);
	if (_year == year) return true;

	transfersList.clear();
	_year = year;

	transfers_lent(year, _tc);
	
	time64_t startDate = _tc->start_date(_year);
	time64_t endDate = _tc->end_date(_year);

	//tc.print_date(startDate);
	//tc.print_date(endDate);

	time64_t currDate = startDate;
	time64_t to;
	
	do {
		if (do_transferred_from(currDate, &to)) {
			// day has already been transferred - either annunciation or joseph husband of mary in easter (is in the list)
			currDate = next_day(currDate); // go to next day
			continue;
		}

		_tc->get(currDate);
		//tc.print_date(currDate);
		//I2CSerial.println();
		
		Enums::Season seas = _tc->season(currDate);

		if (_tc->_bIsSolemnity || ((seas == Enums::SEASON_ADVENT || seas == Enums::SEASON_LENT) && _tc->sunday(currDate))) {
			bool bIsSanctorale = _sc->get(currDate);
			if (bIsSanctorale && _sc->_rank_e == Enums::RANKS_SOLEMNITY_GENERAL) {
				if (_tc->_rank_e >= _sc->_rank_e) {
					time64_t transfer_to = currDate;
					bool bFound = false;
					int i = 365;
					do {
						wdt_reset();
						transfer_to = next_day(transfer_to);
						if (i-- < 0)
						{
							_year = 0;
							transfersList.clear();
							return false; // sanity check: it shouldn't take more than a few iterations max to find a suitable transfer date, but if something went wrong, this will get us out of the loop and prevent a lockup
						}
					} while (valid_destination(transfer_to, _tc, _sc) == false);
					/// add to list
					Transfer* t = new Transfer();
					t->from = currDate;
					t->to = transfer_to;
					transfersList.add(t);

					Temporale::print_date(currDate);
					I2CSerial.print(F(" has been transferred to "));
					Temporale::print_date(transfer_to);
					I2CSerial.println();
				}
			}
		}

		currDate = next_day(currDate);
#ifndef _WIN32
		ESP.wdtFeed();
#endif
	} while (currDate <= endDate);

	I2CSerial.println(F("do_transfers() complete"));
	
	return true;
}

void Transfers::transfers_lent(int year, Temporale* tc) {
	// If the Annunciation(Mar 25) falls on Palm Sunday, it is celebrated on the Saturday preceding.If it falls during Holy Week or within 
	// the Octave of Easter, the Annunciation is transferred to the Monday of the Second Week of Easter.
	//
	// If Joseph, Husband of Mary(Mar 19) falls on Palm Sunday or during Holy Week, it is moved to the Saturday preceding Palm Sunday.
	// This was first addressed in the Decree Cum Proximo Anno issued in 1966. Versions of ROMCAL prior to version 4 incorrectly moved 
	// it to the Monday of the Second Week of Easter, where it was overwritten by the Annunciation.

	time64_t palm_sunday = tc->palm_sunday(year);
	time64_t annunciation = tc->date(25, 3, year + 1);

	time64_t start_holy_week = next_day(palm_sunday);
	time64_t end_easter_octave = tc->sunday_after(tc->easter_sunday(year));

	I2CSerial.print(F("year = ")); I2CSerial.print(String(year));
	I2CSerial.print(F("\tPalm Sunday:"));
	tc->print_date(palm_sunday);
	I2CSerial.print(F("\tAnnunciation: "));
	tc->print_date(annunciation);
	I2CSerial.print(F("\tStart of Holy Week: "));
	tc->print_date(start_holy_week);
	I2CSerial.print(F("\tEnd of Easter Octave: "));
	tc->print_date(end_easter_octave);
	
	Transfer* t;

	if (tc->issameday(palm_sunday, annunciation)) {
		t = new Transfer();
		t->from = annunciation;
		t->to = tc->saturday_before(palm_sunday); // transfer to saturday before palm sunday (Romcal states this, some others state it should be
		transfersList.add(t);					  // handled as though it fell at any other time in Holy Week or Easter Octave).

		I2CSerial.print(F("\nAnnunciation on Palm Sunday: Annunciation transferred from "));
		tc->print_date(annunciation);
		I2CSerial.print(F(" to "));
		tc->print_date(t->to);
		I2CSerial.println();
	}
	else if (annunciation >= start_holy_week && annunciation <= end_easter_octave) {
		t = new Transfer();
		t->from = annunciation;
		t->to = tc->monday_after(end_easter_octave); // transfer to second monday of easter
		transfersList.add(t);

		I2CSerial.print(F("\nAnnunciation in Holy Week or Easter Octave: Annunciation transferred from "));
		tc->print_date(annunciation);
		I2CSerial.print(F(" to "));
		tc->print_date(t->to);
		I2CSerial.println();
	}

	time64_t joseph_hom = tc->date(19, 3, year + 1);

	I2CSerial.print(F("Joseph Husband of Mary: "));
	tc->print_date(joseph_hom);
	I2CSerial.println();

	if (tc->issameday(joseph_hom, palm_sunday) || (joseph_hom >= start_holy_week && joseph_hom <= tc->easter_sunday(year))) {
		t = new Transfer();
		t->from = joseph_hom;
		t->to = tc->saturday_before(palm_sunday); // transfer to saturday before palm sunday. Since joseph_hom and annunciation cannot both be
		transfersList.add(t);					  // on palm sunday in the same year, annunciation and joseph_hom should never conflict, 
												  // even if they are both moved
		I2CSerial.print(F("Joseph Husband of Mary transferred from "));
		tc->print_date(joseph_hom);
		I2CSerial.print(F(" to "));
		tc->print_date(t->to);
		I2CSerial.println();
	}											  
}

bool Transfers::get(time64_t date) {
	_I18n->suppress_output(true);
	
	bool bResult = do_transfers(date);
	
	_I18n->suppress_output(false);
	
	return bResult;
}

bool Transfers::do_transferred_from(time64_t date, time64_t* to) {
	Transfer* t;
	for (int i = 0; i < transfersList.size(); i++) {
		t = transfersList.get(i);
		if (t->from == date) {
			*to = t->to;
			return true;
		}
	}
	return false; // not transferred
}

bool Transfers::do_transferred_to(time64_t date, time64_t* from) {	// returns date from which the date 'date' has been transferred
	Transfer* t;
	for (int i = 0; i < transfersList.size(); i++) {
		t = transfersList.get(i);
		if (t->to == date) {
			*from = t->from;
			return true;
		}
	}
	return false; // not transferred
}

bool Transfers::transferred_from(time64_t date, time64_t* to) { // returns date to which the date 'date' is to be transferred
	if (get(date)) {
		return do_transferred_from(date, to);
	}
	return false;
}

bool Transfers::transferred_to(time64_t date, time64_t* from) {	// returns date from which the date 'date' has been transferred
	if (get(date)) {
		return do_transferred_to(date, from);
	}
	return false;
}

bool Transfers::valid_destination(time64_t day, Temporale* tc, Sanctorale* sc) {
	tc->get(day);
	if (tc->_rank_e >= Enums::RANKS_FEAST_PROPER) return false;

	if (sc->get(day)) {
		if (sc->_rank_e >= Enums::RANKS_FEAST_PROPER) return false;
	}

	return true;
}

time64_t Transfers::next_day(time64_t day) {
#ifndef _WIN32
	::tmElements_t ts;						// for arduino
	::breakTime(day, ts);

	int mdays = get_monthdays(ts.Month, ts.Year + BEGIN_EPOCH);
	int mday = ts.Day + 1;
	if (mday > mdays) {
		ts.Month++;
		ts.Day = 1;
		if (ts.Month > 12) {
			ts.Day = 1;
			ts.Year++;
		}
	}
	else {
		ts.Day = mday;
	}
	
	return ::makeTime(ts);
#else
	tm* ts = gmtime(&day);

	int mdays = get_monthdays(ts->tm_mon + 1, ts->tm_year + BEGIN_EPOCH);
	int mday = ts->tm_mday + 1;
	if (mday > mdays) {
		ts->tm_mon++;
		ts->tm_mday = 1;
		if (ts->tm_mon > 11) {
			ts->tm_mon = 0;
			ts->tm_year++;
		}
	}
	else {
		ts->tm_mday = mday;
	}

	return mktime(ts);
#endif
}


int Transfers::get_monthdays(int mon, int year) {
	static const int days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int leap = yisleap(year) ? 1 : 0;

	return days[mon] + leap;
}

bool Transfers::yisleap(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}


/*
module CalendariumRomanum

  # Resolves transfers of solemnities.
  class Transfers
    def initialize(temporale, sanctorale)
      @transferred = {}
      @temporale = temporale
      @sanctorale = sanctorale

      dates = sanctorale.solemnities.keys.collect do |abstract_date|
        concretize_abstract_date abstract_date
      end.sort

      dates.each do |date|
        tc = temporale.get(date)
        next unless tc.solemnity?

        sc = sanctorale.get(date)
        next unless sc.size == 1 && sc.first.solemnity?

        loser = [tc, sc.first].sort_by(&:rank).first

        transfer_to = date
        begin
          transfer_to = transfer_to.succ
        end until valid_destination?(transfer_to)
        @transferred[transfer_to] = loser
      end
    end

    def get(date)
      @transferred[date]
    end

    private

    def valid_destination?(day)
      return false if @temporale.get(day).rank >= Ranks::FEAST_PROPER

      sc = @sanctorale.get(day)
      return false if sc.size > 0 && sc.first.rank >= Ranks::FEAST_PROPER

      true
    end

    # Converts an AbstractDate to a Date in the given
    # liturgical year.
    # It isn't guaranteed to work well (and probably doesn't work well)
    # for the grey zone of dates between earliest and latest
    # possible date of the first Advent Sunday, but that's no problem
    # as long as there are no sanctorale solemnities in this
    # date range.
    def concretize_abstract_date(abstract_date)
      d = abstract_date.concretize(@temporale.year + 1)
      if @temporale.date_range.include? d
        d
      else
        abstract_date.concretize(@temporale.year)
      end
    end
  end
end
*/