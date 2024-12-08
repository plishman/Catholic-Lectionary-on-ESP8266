#include "Precedence.h"
#ifdef _WIN32
#include "Bidi.h"
#endif

#ifdef _TEST_
#define FIRST 1		// corresponds to sanctorale feasts
#define SECOND 0	// corresponds to moveable and temporale feasts
#endif

const uint8_t precedence_1960[8][16] = { {0, 7, 1, 1, 1, 1, 6, 8, 1, 7, 3, 3, 3, 7, 3, 7},
								   {3, 7, 1, 1, 1, 1, 8, 7, 1, 7, 3, 3, 3, 7, 3, 7},
								   {3, 2, 4, 4, 4, 0, 2, 2, 4, 2, 3, 3, 3, 2, 5, 2},
								   {0, 2, 4, 4, 9, 5, 2, 2, 4, 2, 3, 3, 5, 2, 5, 2},
								   {0, 2, 5, 0, 5, 5, 2, 2, 5, 2, 5, 3, 5, 2, 2, 2},
								   {0, 2, 9, 4, 5, 5, 2, 2, 5, 2, 5, 3, 5, 2, 2, 2},
								   {0, 0, 4, 4, 5, 5, 2, 2, 0, 0, 0, 0, 0, 0, 2, 0},
								   {0, 0, 5, 0, 5, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0} };

const uint8_t precedence[10][17] = { {0, 1, 3, 1, 3, 3, 3, 3, 3, 3, 6, 5, 8, 6, 3, 3, 6},
							   {0, 3, 3, 1, 3, 6, 3, 3, 3, 3, 6, 8, 6, 6, 3, 6, 6},
							   {0, 3, 3, 3, 3, 4, 3, 3, 3, 7, 4, 4, 4, 0, 4, 4, 4},
							   {0, 3, 3, 3, 3, 4, 3, 3, 7, 4, 4, 4, 4, 4, 4, 4, 4},
							   {0, 3, 3, 3, 3, 4, 3, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4},
							   {0, 3, 3, 3, 3, 4, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
							   {0, 3, 3, 7, 4, 4, 4, 4, 4, 4, 4, 2, 2, 0, 4, 4, 4},
							   {0, 3, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 0, 0, 0},
							   {0, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 0, 4, 4, 4},
							   {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4} };


//void Precedence::doOrdering(time64_t datetime, uint8_t mass_type, MissalReading& season, MissalReading& feast, MissalReading& votive, Ordering& ordering) {
//	doOrdering(datetime, mass_type, season, feast, votive, ordering);
//}

// need to handle case of 1st to 5th Jan, which in 1955 Mass are Feria unless feasts - maybe not here, but in DoLatinMassPropers?

void Precedence::doOrdering(time64_t datetime, uint8_t mass_type, MissalReading& season, MissalReading& feast, MissalReading& votive, MissalReading& deferred, Ordering& ordering) {
	/// TODO: sometimes headings[0] will actually be a feast - a Feast of the Lord, and so the temporal (lesser) feast (&feast) should go here in headings[0], 
	/// and the Feast of the Lord, held in &season, should go in headings[1]
	ordering.headings[0] = &season; 
	ordering.headings[1] = &feast;
	ordering.headings[2] = &votive;	
	ordering.headings[3] = &deferred; //PLL 07-10-2024 adding support for celebrating deferred feasts *replacing feast day* (is this the best way to do it?)

	ordering.b_com_at_lauds = false;
	ordering.b_com_at_vespers = false;
	ordering.b_com = false;
	ordering.b_transfer_1st = false;
	ordering.b_transfer_2nd = false;
	ordering.transfer_heading_index = -1;
	ordering.b_feast_is_commemoration = false;
	ordering.b_is_votive = false;
	ordering.b_celebrate_deferred = false;
	ordering.ordering[0] = -1;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
	ordering.ordering[3] = -1;
	ordering.feast_classnumber = 0;
	ordering.season_classnumber = 0;
	ordering.votive_classnumber = 0;
	ordering.deferred_classnumber = 0;

	bool b_use_new_classes = (mass_type >= MASS_1960);

	if (!b_use_new_classes)
	{
		if (!season.isOpen() && !feast.isOpen() && !votive.isOpen()) {
			DEBUG_PRT.println(F("No feast, seasonal or votive day!"));
			return;
		}

		PrecedenceParams pp_season;	
		PrecedenceParams pp_feast;
		PrecedenceParams pp_deferred; // PLL-06-10-2024 for deferred feast (if present)

		bool b_is_saturday_of_our_lady = !deferred.isOpen() && votive.isOpen() && isSaturdayOfOurLady(datetime, /*b_use_new_classes*/mass_type, season, feast);
		// PLL 08-10-2024 Can't have a Saturday of Our Lady if there is a deferred feast to celebrate, since Saturdays of Our Lady are only celebrated if there is no other feast on that day (although there can sometimes be a commemoration)

		ordering.b_is_votive = b_is_saturday_of_our_lady;

		if (b_is_saturday_of_our_lady) {
			DEBUG_PRT.println(F("Saturday of Our Lady"));
			ordering.headings[0] = &votive;
			ordering.headings[2] = &season;		
			//ordering.ordering[0] = 2; // Votive mass as heading
			//ordering.ordering[1] = feast.isOpen() ? FIRST : 
			//					   season.isOpen() ? SECOND : -1; // Feast day as subheading (if available), else seasonal day as subheading, else not displayed
			//ordering.ordering[2] = -1; // third item not displayed
			//return;
		}
		pp_season.b_is_available = ordering.headings[0] != NULL && ordering.headings[0]->isOpen();
		pp_feast.b_is_available = ordering.headings[1] != NULL && ordering.headings[1]->isOpen();
		pp_deferred.b_is_available = ordering.headings[3] != NULL && ordering.headings[3]->isOpen();

		int8_t pr_index_x = -1;
		int8_t pr_index_y = -1;
		int8_t pr_index_y_deferred = -1;

		if (pp_season.b_is_available) {
			pp_season.b_is_saturday_of_our_lady = b_is_saturday_of_our_lady;
			if (b_is_saturday_of_our_lady) {
				pp_season.b_is_vigil = IsVigil(votive, datetime, pp_season.vigil_class, pp_season.vigil_type);
				pp_season.day = Class_pre1960(votive, datetime, pp_season.b_is_sunday, pp_season.sunday_class, pp_season.b_is_ferial, pp_season.ferial_class, pp_season.b_is_duplex, pp_season.duplex_class);
			}
			else {
				pp_season.b_is_vigil = IsVigil(season, datetime, pp_season.vigil_class, pp_season.vigil_type);
				pp_season.day = Class_pre1960(season, datetime, pp_season.b_is_sunday, pp_season.sunday_class, pp_season.b_is_ferial, pp_season.ferial_class, pp_season.b_is_duplex, pp_season.duplex_class);
			}
			pp_season.b_is_octave = getOctave(datetime, true, mass_type, pp_season.octave_type, pp_season.privileged_octave_order, pp_season.b_is_in_octave_day, pp_season.b_is_in_octave_feast_day);
			pp_season.b_is_vigil_of_epiphany = Tridentine::issameday(datetime, Tridentine::date(5, 1, year(datetime)));

			Set_Epiphany_Ferials(datetime, pp_season, mass_type);
			Set_Privileged_Ferias(datetime, pp_season, mass_type);
			//Promote_Sundays_1955_Advent_and_Lent(datetime, pp_season, mass_type);
			//Set_Ember_Days_to_Semiduplex_1955(datetime, pp_season, mass_type);
			
			pr_index_x = Index_x(pp_season);
		}

		if (pp_feast.b_is_available) {
			pp_feast.b_is_saturday_of_our_lady = b_is_saturday_of_our_lady;
			pp_feast.b_is_vigil = IsVigil(feast, datetime, pp_feast.vigil_class, pp_feast.vigil_type);
			pp_feast.day = Class_pre1960(feast, datetime, pp_feast.b_is_sunday, pp_feast.sunday_class, pp_feast.b_is_ferial, pp_feast.ferial_class, pp_feast.b_is_duplex, pp_feast.duplex_class);
			
			// need to handle that getOctave is neutral as to whether a feast or a seasonal day is being considered. This causes
			// a problem when the seasonal octave is movable
			pp_feast.b_is_octave = getOctave(datetime, false, mass_type, pp_feast.octave_type, pp_feast.privileged_octave_order, pp_feast.b_is_in_octave_day, pp_feast.b_is_in_octave_feast_day);
			pp_feast.b_is_vigil_of_epiphany = Tridentine::issameday(datetime, Tridentine::date(5, 1, year(datetime)));
			pr_index_y = Index_y(pp_feast);
		}

		// if it is neither a duplex day of CLASS I or II, nor a sunday, the transferred day will be celebrated instead of this day's feast (per Claude.ai response)
		// can do it if there is a deferred feast and today is not Duplex Class I or II

		int8_t day_duplex_class = DUPLEX_NA;
		bool b_day_is_duplex = false;
		
		if (pp_season.b_is_available) {
			day_duplex_class = pp_season.duplex_class;
			b_day_is_duplex = pp_season.b_is_duplex;
		}
		
		bool b_is_sunday = Tridentine::sunday(datetime);
		bool b_celebrate_deferred = (pp_deferred.b_is_available && !b_is_sunday && (!(b_day_is_duplex && day_duplex_class >= DUPLEX_CLASS_II)));
		
		if (pp_feast.b_is_available) {
			day_duplex_class = pp_feast.duplex_class > day_duplex_class ? pp_feast.duplex_class : day_duplex_class;
			b_day_is_duplex = (pp_feast.b_is_duplex || b_day_is_duplex);
			b_celebrate_deferred = (pp_deferred.b_is_available && !b_is_sunday && (!(b_day_is_duplex && day_duplex_class >= DUPLEX_CLASS_II)));
		}

		if (b_celebrate_deferred) { // PLL-06-10-2024 Adding support for deferred feasts				
			// can celebrate transferred feast if so
			pp_deferred.b_is_saturday_of_our_lady = b_is_saturday_of_our_lady;
			pp_deferred.b_is_vigil = IsVigil(deferred, datetime, pp_deferred.vigil_class, pp_deferred.vigil_type);
			pp_deferred.day = Class_pre1960(deferred, datetime, pp_deferred.b_is_sunday, pp_deferred.sunday_class, pp_deferred.b_is_ferial, pp_deferred.ferial_class, pp_deferred.b_is_duplex, pp_deferred.duplex_class);

			// need to handle that getOctave is neutral as to whether a feast or a seasonal day is being considered. This causes
			// a problem when the seasonal octave is movable
			pp_deferred.b_is_octave = getOctave(datetime, false, mass_type, pp_deferred.octave_type, pp_deferred.privileged_octave_order, pp_deferred.b_is_in_octave_day, pp_deferred.b_is_in_octave_feast_day);
			pp_deferred.b_is_vigil_of_epiphany = Tridentine::issameday(datetime, Tridentine::date(5, 1, year(datetime)));
			pr_index_y_deferred = Index_y(pp_deferred);
		}

		///TODO: if deferred feast can be celebrated on this day, need to make it the feast. The feast/seasonal days' references will not need to be deferred
		///      for a day which without the transferred feast is not a Class I or II day
		
		
		///

		uint8_t tablevalue = pr_index_x == -1 || pr_index_y == -1 ? 0 : precedence[pr_index_y][pr_index_x];
		
		uint8_t tablevalue_df = 0;
		
		if (b_celebrate_deferred) {
			tablevalue_df = pr_index_x == -1 || pr_index_y_deferred == -1 ? 0 : precedence[pr_index_y_deferred][pr_index_x];

			ordering.deferred_classnumber = pp_deferred.b_is_available ? (uint8_t)(18 - (Index_x(pp_deferred) + 1)) : 0;
			ordering.b_celebrate_deferred = b_celebrate_deferred;

			ordering.headings[3] = &feast;
			ordering.headings[1] = &deferred; //PLL 07-10-2024 adding support for celebrating deferred feasts *replacing feast day* (is this the best way to do it?)

#ifdef _WIN32
			//Bidi::printf("<span class='flash'>[celebrate deferred feast]</span>");
#endif
		}

		ordering.feast_classnumber = pp_feast.b_is_available ? (uint8_t)(18-(Index_x(pp_feast) + 1)) : 0; // makes a "score" from -1 to 16, to which I add 1 and subtract the result from 18, to make it from 18 to 1 (18=simplex, 1=Sunday of the first class). 0 = not available
		ordering.season_classnumber = pp_season.b_is_available ? (uint8_t)(18-(pr_index_x + 1)) : 0; //

		if (b_celebrate_deferred) {
			deferred._precedencescore = ordering.feast_classnumber;
		}
		else {
			if (b_is_saturday_of_our_lady) {
				votive._precedencescore = ordering.feast_classnumber;
			}
			else {
				feast._precedencescore = ordering.feast_classnumber;
			}
		}
		season._precedencescore = ordering.season_classnumber;

		/// TO FIX: a hack - to avoid having to reverify the whole year's output, have fixed this specific case. 
		/// Has the effect of promoting the Feast of the Lord over hthe Seasonal feast, effectively putting the FOL in the headings[1] (feast) position, 
		/// and the seasonal feast in the headings[0] position. 
		/// To do it properly, need to make sure that, when there are two feasts - a feast of the Lord and a fixed temporal feast, where the lesser feast is transferred, 
		/// that the FOL goes in the Feast headings array entry, and the temporal feast in the headings Seasonal array entry (ie, the reverse of the default situation)
		
		// NB. Now know this is because the feasts in the weeks of Pentecost (which move with the date of Pentecost each year), are stored in such as wy that they are opened
		// as seasonal, rather than feast days. This should fix this.
		int yr = year(datetime);
		if (tablevalue == 5 /*office of 1st, transfer of 2nd*/ && Tridentine::IsMoveableFeastOfTheLordInWeeksAfterPentecost(datetime))
		{
			tablevalue = 6 /* set to office of 2nd, transfer of 1st if so*/;
#ifdef _WIN32			
			Bidi::printf("[Set as feast: Moveable FOL in weeks after Pentecost]");
#endif
			DEBUG_PRT.println(F("[Set as feast: Moveable FOL in weeks after Pentecost]"));
		}
		///
		///

//		if (tablevalue = 6 && Tridentine::issameday(datetime, Tridentine::ImmaculateConception(yr)) && mass_type > MASS_TRIDENTINE_1570) {
//			tablevalue = 3; // Officium de 1, nihil de 2. (Following 1960 Rubrics and D.O. website, Occurrence of Immaculate Conception take precedence if it falls on on Sunday of Advent)
//			// D.O. website transfers 1570 Occurrence of Immaculate Conception if it falls on Sunday of Advent, but celbrates it for all other versions.
//			// I'm showing the Sunday of Advent as a Commemoration in this case!
//		}

		uint8_t tvalue = tablevalue;
		if (b_celebrate_deferred) tvalue = tablevalue_df; // if the deferred feast will be celebrated today

		//DEBUG_PRT.on();
		DEBUG_PRT.printf("x=%d y=%d table value=%d: ", pr_index_x, pr_index_y, tvalue);
#ifdef _WIN32
		Bidi::printf("x=%d y=%d table value=%d: ", pr_index_x, pr_index_y, tvalue);
#endif
		//DEBUG_PRT.off();

		switch (tvalue)
		{
		case 0:
			pr_0(ordering);
			break;

		case 1:
			pr_1(ordering);
			break;

		case 2:
			pr_2(ordering);
			break;

		case 3:
			pr_3(ordering);
			break;

		case 4:
			pr_4(ordering);
			break;

		case 5:
			pr_5(ordering);
			break;

		case 6:
			pr_6(ordering);
			break;

		case 7:
			pr_7(ordering);
			break;

		case 8:
			pr_8(ordering);
			break;

		default:
			DEBUG_PRT.println(F("Unknown table value"));
			break;
		}

		handleCommemorations(datetime, mass_type, ordering, pp_season, pp_feast, tablevalue);

//		if ((ordering.b_transfer_1st || ordering.b_transfer_2nd) && ordering.transfer_heading_index != -1) {
//			Tridentine::setDeferredFeast(ordering.headings[ordering.transfer_heading_index]->_filedir); // PLL-04-10-2024 save filename of Seasonal day to deferred list
//		}
	}
	else
	{
		int8_t octave_class = 0;
		bool b_is_octave = Precedence::getOctave1960(octave_class, datetime);

		patchAscensionVigilClass(season); // fix problem with Ascension Vigil, which records it as a Feria (in Divinum Officium.com software). It should be II. classis
		ordering.season_classnumber = Class_1960(season); // need to adapt for pre-1960 class names
		ordering.feast_classnumber = Class_1960(feast);
		ordering.votive_classnumber = Class_1960(votive);
		ordering.deferred_classnumber = Class_1960(deferred); // PLL-08-10-2024 adding support for recording and celebrating deferred feasts

		season._precedencescore = ordering.season_classnumber;
		feast._precedencescore = ordering.feast_classnumber;
		votive._precedencescore = ordering.votive_classnumber;
		deferred._precedencescore = ordering.deferred_classnumber;
	
		bool bIsAshWednesday = Tridentine::issameday(datetime, Tridentine::AshWednesday(year(datetime)));

		uint8_t seas = Tridentine::Season(datetime);
		if (ordering.feast_classnumber == 3 && seas == SEASON_LENT || bIsAshWednesday) { //https://en.wikipedia.org/wiki/General_Roman_Calendar_of_1960
			ordering.feast_classnumber = 4; // Class III feasts are celebrated as commemorations during Lent (PLL-21-03-2022 Ash Wednesday supersedes any feast on that day)
			ordering.b_feast_is_commemoration = true;
		}

		bool b_season_only = (!feast.isOpen() && ordering.feast_classnumber == 0);
		bool b_feast_only = (!season.isOpen() && ordering.season_classnumber == 0);
		bool b_is_saturday_of_our_lady = !deferred.isOpen() && ordering.votive_classnumber > 0 && isSaturdayOfOurLady(datetime, mass_type, season, feast);

		if (ordering.season_classnumber == 0 && ordering.feast_classnumber == 0 && ordering.votive_classnumber == 0) {
			DEBUG_PRT.println(F("No feast, seasonal or votive day!"));
			return;
		}
		
		ordering.b_is_votive = b_is_saturday_of_our_lady;

		if (b_is_saturday_of_our_lady) {
			DEBUG_PRT.println(F("Saturday of Our Lady"));
			ordering.headings[0] = &votive;
			ordering.headings[2] = &season;

			uint8_t season_classnumber = ordering.season_classnumber;
			ordering.season_classnumber = ordering.votive_classnumber;	
			ordering.votive_classnumber = season_classnumber;
			
			//ordering.ordering[0] = 2; // Votive mass as heading
			//ordering.ordering[1] = ordering.feast_classnumber > 0 ? FIRST : SECOND; // Feast day as subheading (if available), else seasonal day as subheading
			//ordering.ordering[2] = -1; // third item not displayed
			//return;
		}

		// Assumption: All feast days in are (should be in this implementation) from the General Roman Calendar, hence
		// all are Universal Feasts (rather than Particular Feasts). This simplifies implementation of the Table usage

		int8_t day_class = ordering.feast_classnumber > ordering.season_classnumber ? ordering.feast_classnumber : ordering.season_classnumber;
		bool b_Sunday = Tridentine::sunday(datetime);
		bool b_celebrate_deferred = (deferred.isOpen() && (!(day_class < 3 || b_Sunday))); // PLL-08-10-2024 Determine whether or not a deferred feast can be celebrated today. Will need to check this catches all cases in the simplified post 1960 liturgical classes - eg, can some deferred feasts be celebrated on a Sunday?
		if (b_celebrate_deferred) {
			ordering.headings[3] = &feast;
			ordering.headings[1] = &deferred; //PLL 07-10-2024 adding support for celebrating deferred feasts *replacing feast day* (is this the best way to do it?)
			
			int8_t classnumber_temp = ordering.feast_classnumber;
			ordering.feast_classnumber = ordering.deferred_classnumber;
			ordering.deferred_classnumber = classnumber_temp;

			ordering.b_celebrate_deferred = b_celebrate_deferred;
#ifdef _WIN32
			//Bidi::printf("<span class='flash'>[celebrate deferred feast]</span>");
#endif
		}

		bool b_season_is_vigil = false;
		bool b_feast_is_vigil = false;

		if (!b_feast_only) {
			b_season_is_vigil = IsVigil(season);
		}
	
		if (!b_season_only) {
			if (b_celebrate_deferred) {
				b_feast_is_vigil = IsVigil(deferred);
			}
			else {
				b_feast_is_vigil = IsVigil(feast);
			}
		}

		int8_t sunday_class = -1;
		if (b_Sunday) {
			sunday_class = Class_1960(season);
		}

		bool b_Feria = !b_Sunday;

		bool b_LentAndPassionTide = (seas == SEASON_LENT); // shouldn't both be true
		bool b_Advent = (seas == SEASON_ADVENT);		   //

		// now know if 
		// i) it is an octave, and whether class 1 or 2 octave if so; 
		// ii) if it is a vigil; 
		// iii) the class (I-IV) of the feast and seasonal day
		// whether it is a Sunday or Feria, and the Sunday's class (if applicable)
		// whether it is Lent/Passiontide or Advent, or neither

		//int8_t pr_1960_index_1960_x = Index1960_x(season_classnumber, b_season_is_vigil, b_Sunday, b_Feria, b_Advent, b_LentAndPassionTide, b_is_octave, octave_class);
		//int8_t pr_1960_index_1960_y = Index1960_y(feast_classnumber, b_feast_is_vigil);

		int8_t pr_index_1960_x = Index1960_x(ordering.season_classnumber, b_season_is_vigil, b_Sunday, sunday_class, b_Feria, b_Advent, b_LentAndPassionTide, b_is_octave, octave_class);
		int8_t pr_index_1960_y = Index1960_y(ordering.feast_classnumber, b_feast_is_vigil);

		uint8_t tablevalue = pr_index_1960_x == -1 || pr_index_1960_y == -1 ? 0 : precedence_1960[pr_index_1960_y][pr_index_1960_x];

		bool b_is_ember_day = Tridentine::IsEmberDay(datetime);



		if (feast.isOpen() && ordering.feast_classnumber == 0 && ordering.season_classnumber == 4) { // if this is a commemoration, with no class information. If the season is class IV, display the commemoration
#ifdef _WIN32
			Bidi::printf("<span style='background-color:orange;'> comm. only </span>");
#endif
			tablevalue = 5; //Office of 2nd, com. of 1st
		}

		if (tablevalue == 0 && Tridentine::issameday(datetime, Tridentine::AllSaints(year(datetime)))) {
			// PLL-01-11-2022 Hack: On All Saints, change tablevalue to 1 (from 0), to show no commemoration of Class IV day of the xxth week 
			// after Pentecost, per DivinumOfficium example (can't find rubric for it, but the pre-1960 All Saints day does not show a 
			// commemoration for this day)
#ifdef _WIN32
			Bidi::printf("<span style='background-color:grey;'>All Souls display feast only</span>");
#endif
			tablevalue = 1;
		}

		if (tablevalue == 7 && Tridentine::issameday(datetime, Tridentine::ImmaculateConception(year(datetime)))) {
			// General Rubrics 1960 (paragraph 15) Immaculate Conception take place of Sunday of Advent in case of Occurrence
			tablevalue = 3; // Office if 1st (feast), nothing of 2nd = 1
			// I'm showing the Sunday of Advent as a Commemoration in this case!
		}

		//DEBUG_PRT.on();
		DEBUG_PRT.printf("x=%d y=%d tablevalue=%d: ", pr_index_1960_x, pr_index_1960_y, tablevalue);
#ifdef _WIN32
		Bidi::printf("x=%d y=%d tablevalue=%d: ", pr_index_1960_x, pr_index_1960_y, tablevalue);
#endif
		//DEBUG_PRT.off();

		switch (tablevalue)
		{
		case 0:
			pr_1960_0(ordering);
			break;

		case 1:
			pr_1960_1(ordering);
			break;

		case 2:
			pr_1960_2(ordering);
			break;

		case 3:
			pr_1960_3(ordering);
			break;

		case 4:
			pr_1960_4(ordering);
			break;

		case 5:
			pr_1960_5(ordering);
			break;

		case 6:
			pr_1960_6(ordering);
			break;

		case 7:
			pr_1960_7(ordering);
			break;

		case 8:
			pr_1960_8(ordering);
			break;

		case 9:
			pr_1960_9(ordering);
			break;

		default:
			DEBUG_PRT.println(F("Unknown table value"));
			break;
		}



		int year = Tridentine::year(datetime);

		if (!b_Sunday && !b_is_saturday_of_our_lady
			&& datetime >= Tridentine::date(1, 1, year) && datetime < Tridentine::date(13, 1, year) 
			&& ordering.season_classnumber == 4 && ordering.feast_classnumber == 4 
			&& ordering.ordering[0] == SECOND) {

			ordering.ordering[0] = FIRST;
			ordering.ordering[1] = SECOND;
#ifdef _WIN32
			Bidi::printf(" (12 days of January have precedence) ");
#endif
		}

		// XI The Precedence of Liturgical Days (http://divinumofficium/www/horas/Help/Rubrics/General%20Rubrics.html) 7. Ferias of the 1st class not mentioned above, namely Ash Wednesday and Monday, Tuesday and Wednesday of Holy Week.
		if (datetime == Tridentine::AshWednesday(year)
			|| (Tridentine::IsHolyWeek(datetime) && (weekday(datetime) == dowMonday || weekday(datetime) == dowTuesday || weekday(datetime) == dowWednesday))) {
		
			ordering.ordering[0] = SECOND;
			ordering.ordering[1] = -1;
			ordering.b_com = false;
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;
#ifdef _WIN32
			Bidi::printf("<span style='color: mediumpurple;'> (No comm. on Ash Wednesday or Mon, Tue, Weds of Holy Week) </span>");
#endif
			return;
		}


		// A bit of a hack, can't find the rubric that says it (1960), but Purification of Mary when it falls on a Sunday has no commemoration of the Sunday in D.O.
		// The closest rubric I can find is:
		//
		// 1. A 1st or 2nd class feast of the Lord occurring on a Sunday takes the place of that Sunday with all rights and privileges; hence there is no commemoration of the Sunday.
		// (http://divinumofficium.com/www/horas/Help/Rubrics/Tables%201960.txt)
		// The underlying file for this feast, missa/Sancti/02-02.txt includes the rule "Festum Domini" - So I take it that this feast is also included as a Feast of the Lord.
		// The function IsAlsoFeastOfTheLord returns true if the date is that of the Purification of Mary, or of the Transfiguration of the Lord (which also has this rule).
		// The Feast will be displayed alone if it falls on a Class II Sunday, and is itself a Class II.
	
		// a) a 1st or 2nd class feast of the Lord occurring on a Sunday of the 2nd class takes the place of the Sunday itself with all its rights and privileges; hence, there is no commemoration of the Sunday;
		if (b_Sunday && ordering.season_classnumber == 2 && ordering.feast_classnumber == 2 && Tridentine::IsAlsoFeastOfTheLord(datetime) || Tridentine::IsAlsoFeastOfTheLord(datetime)) { // I know, but I'm comparing against D.O. as well as the Rubrics
			ordering.ordering[0] = FIRST;
			ordering.ordering[1] = -1;
#ifdef _WIN32
			Bidi::printf("(FOL patched)");
#endif
			return;
		}

		//bool b_is_ember_day = Tridentine::IsEmberDay(datetime);

		if ((!b_Sunday && (seas == SEASON_ADVENT || seas == SEASON_LENT)) || (month(datetime) == 9 && b_is_ember_day)) {
			// commemorate all Ferias of Advent and Lent, and the Ember Days of September (XVI Commemorations rule 108, privileged commemorations, http://divinumofficium/www/horas/Help/Rubrics/General%20Rubrics.html)
			if (ordering.ordering[0] == FIRST) {
				ordering.ordering[1] = SECOND;
				ordering.b_com = true;
#ifdef _WIN32
				Bidi::printf("<span style='color:purple;'> privileged commemoration </span>");
#endif
				return;
			}
		}

		// http://divinumofficium/www/horas/Help/Rubrics/General%20Rubrics.html
		// Ferias of the 2nd class, namely those of Advent from December 17 to 23 inclusive and the ember days of Advent, Lent and September. [have higher priority than 2nd or 3rd class feasts]
		if (b_is_ember_day && !IsUniversalFeast(datetime) && (seas == SEASON_ADVENT || seas == SEASON_LENT || seas == SEASON_AFTER_PENTECOST) && ordering.feast_classnumber >=2 && ordering.ordering[0] == FIRST && ordering.ordering[1] == SECOND) {
			ordering.ordering[0] = SECOND;
			ordering.ordering[1] = FIRST;
			ordering.b_com = true;
#ifdef _WIN32
			Bidi::printf("<span style='color:pink;'> Ember Day has precedence </span>");
#endif
			return;
		}


		if (b_Sunday && Tridentine::issameday(datetime, Tridentine::AllSouls(year))) {
			// b) a Sunday of the 2nd class is preferred to the commemoration of All the Faithful Departed. (rule 16(b), http://divinumofficium/www/horas/Help/Rubrics/General%20Rubrics.html)
			ordering.ordering[1] = SECOND;
			ordering.b_com = true;
#ifdef _WIN32
			Bidi::printf("<span style='color:darkpurple;'> rule 16(b) comm. of Sunday on All Souls </span>");
#endif
			return;
		}

		if (Tridentine::issameday(datetime, Tridentine::nativity(year) - SECS_PER_DAY)) { // is Christmas Eve
			ordering.ordering[0] = FIRST;
			ordering.ordering[1] = -1; // no subheading for Advent on Christmas Eve
			ordering.b_com = false;
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;
#ifdef _WIN32
			Bidi::printf("<span style='color:darkred;'> no comm. of day of advent on Christmas Eve </span>");
#endif
			return;
		}
	}
}

bool Precedence::IsUniversalFeast(time64_t datetime, bool b_fixed_feast_only)
{
	int year = Tridentine::year(datetime);

	bool b_is_Sacred_Heart = !b_fixed_feast_only && Tridentine::issameday(datetime, Tridentine::SacredHeart(year));
	bool b_is_Corpus_Christi = !b_fixed_feast_only && Tridentine::issameday(datetime, Tridentine::CorpusChristi(year));

	bool b_is_Circumcision = Tridentine::issameday(datetime, Tridentine::CircumcisionOfTheLord(year));
	bool b_is_Holy_Name = Tridentine::issameday(datetime, Tridentine::HolyName(year));
	bool b_is_Purification_Of_Mary = Tridentine::issameday(datetime, Tridentine::PurificationOfMary(year));

	bool b_is_NativityStJohnBaptist = Tridentine::issameday(datetime, Tridentine::NativityOfJohnBaptist(year));
	bool b_is_Immaculate_Conception = Tridentine::IsImmaculateConception(datetime);
	bool b_is_Annunciation = Tridentine::IsLadyDay(datetime);
	bool b_is_Precious_Blood_Of_Jesus = Tridentine::issameday(datetime, Tridentine::PreciousBloodOfJesus(year));
	bool b_is_Visitation_of_Mary = Tridentine::issameday(datetime, Tridentine::VisitationOfMary(year));
	bool b_is_Assumption_of_Mary = Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year));
	bool b_is_All_Saints = Tridentine::issameday(datetime, Tridentine::AllSaints(year));

	bool b_is_Finding_Of_Holy_Cross = Tridentine::issameday(datetime, Tridentine::FindingOfTheHolyCross(year));
	bool b_is_St_Joachim = Tridentine::issameday(datetime, Tridentine::StJoachimFatherOfMary(year));
	bool b_is_St_Anne = Tridentine::issameday(datetime, Tridentine::StAnneMotherOfMary(year));
	bool b_is_St_Lawrence = Tridentine::issameday(datetime, Tridentine::StLawrence(year));
	bool b_is_St_Michael = Tridentine::issameday(datetime, Tridentine::StMichaelArchangelDedication(year));
	bool b_is_Transfiguration_of_the_Lord = Tridentine::issameday(datetime, Tridentine::TransfigurationOfTheLord(year));

	bool b_is_PeterandPaul = Tridentine::issameday(datetime, Tridentine::MartyrdomOfSSPeterAndPaul(year));
	bool b_is_St_Matthias = Tridentine::issameday(datetime, Tridentine::StMatthias(year));
	bool b_is_SS_Philip_and_James = Tridentine::issameday(datetime, Tridentine::SSPhilipAndJames(year));
	bool b_is_St_James = Tridentine::issameday(datetime, Tridentine::StJames(year));
	bool b_is_St_Bartholomew = Tridentine::issameday(datetime, Tridentine::StBartholomew(year));
	bool b_is_St_Matthew = Tridentine::issameday(datetime, Tridentine::StMatthew(year));
	bool b_is_SS_Simon_and_Jude = Tridentine::issameday(datetime, Tridentine::SSSimonAndJude(year));
	bool b_is_St_Andrew = Tridentine::issameday(datetime, Tridentine::StAndrew(year));
	bool b_is_St_Thomas = Tridentine::issameday(datetime, Tridentine::StThomas(year));

	return (b_is_Sacred_Heart || b_is_Corpus_Christi || b_is_Circumcision || b_is_Holy_Name || b_is_Purification_Of_Mary
		|| b_is_NativityStJohnBaptist || b_is_Immaculate_Conception || b_is_Annunciation || b_is_Precious_Blood_Of_Jesus
		|| b_is_Visitation_of_Mary || b_is_Assumption_of_Mary || b_is_All_Saints || b_is_Finding_Of_Holy_Cross
		|| b_is_St_Joachim || b_is_St_Anne || b_is_St_Lawrence || b_is_St_Michael || b_is_Transfiguration_of_the_Lord
		|| b_is_PeterandPaul || b_is_St_Matthias || b_is_SS_Philip_and_James || b_is_St_James || b_is_St_Bartholomew
		|| b_is_St_Matthew || b_is_SS_Simon_and_Jude || b_is_St_Andrew || b_is_St_Thomas);

}

void Precedence::Priority(uint8_t mass_type, time64_t datetime, MissalReading& mr_feast, MissalReading& mr_season, PrecedenceParams& pp_feast, PrecedenceParams& pp_season, int8_t& season_priority, int8_t& feast_priority) {
	int year = Tridentine::year(datetime);
	season_priority = 255;
	feast_priority = 255;
	uint8_t season = Tridentine::Season(datetime);
	bool b_is_sunday = Tridentine::sunday(datetime);

	if (mass_type == MASS_1960) {
		season_priority = Tridentine::issameday(datetime, Tridentine::nativity(year)) || Tridentine::issameday(datetime, Tridentine::Easter(year))
			|| Tridentine::issameday(datetime, Tridentine::Pentecost(year)) ? 1 :

			Tridentine::issameday(datetime, Tridentine::MaundyThursday(year)) || Tridentine::IsGoodFriday(datetime) || Tridentine::IsHolySaturday(datetime) ? 2 :

			/*Tridentine::issameday(datetime, Tridentine::Epiphany(year)) ||*/ Tridentine::issameday(datetime, Tridentine::Ascension(year))
			|| Tridentine::issameday(datetime, Tridentine::TrinitySunday(year)) || Tridentine::issameday(datetime, Tridentine::CorpusChristi(year))
			|| Tridentine::issameday(datetime, Tridentine::SacredHeart(year)) /* || Tridentine::issameday(datetime, Tridentine::ChristTheKing(year))*/ ? 3 :

			/*Tridentine::IsImmaculateConception(datetime) || Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year)) ? 4 :*/

			/*Tridentine::issameday(datetime, Tridentine::nativity(year)) - SECS_PER_DAY || Tridentine::issameday(datetime, Tridentine::CircumcisionOfTheLord(year)) ? 5 :*/

			b_is_sunday && ((season == SEASON_LENT || season == SEASON_ADVENT) || (Tridentine::issameday(datetime, Tridentine::QuasimodoSunday(year)))) ? 6 :

			(Tridentine::issameday(datetime, Tridentine::AshWednesday(year)) || (Tridentine::IsHolyWeek(datetime)
				&& (weekday(datetime) == dowMonday || weekday(datetime) == dowTuesday || weekday(datetime) == dowWednesday))) ? 7 :

			/*!b_is_sunday && Tridentine::issameday(datetime, Tridentine::AllSouls(year)) ? 8 :*/

			Tridentine::issameday(datetime, Tridentine::PentecostVigil(year)) ? 9 :

			(datetime > Tridentine::Easter(year) && datetime < (Tridentine::Easter(year) + 8 * SECS_PER_DAY))
			|| (datetime > Tridentine::Pentecost(year) && datetime < (Tridentine::Pentecost(year) + 8 * SECS_PER_DAY)) ? 10 :

			/*IsUniversalFeast(datetime, true) ? 11 :*/

			Class_1960(mr_season) == 1 ? 12 :

			/*Tridentine::issameday(datetime, Tridentine::HolyFamily(year)) || Tridentine::issameday(datetime, Tridentine::BaptismOfTheLord(year))
			|| Tridentine::issameday(datetime, Tridentine::PurificationOfMary(year)) || Tridentine::issameday(datetime, Tridentine::TransfigurationOfTheLord(year))
			|| Tridentine::issameday(datetime, Tridentine::HolyCross(year)) || Tridentine::issameday(datetime, Tridentine::DedicationOfTheLateranBasilica(year)) ? 14 :*/

			b_is_sunday && Class_1960(mr_season) == 2 ? 15 :

			Class_1960(mr_season) == 2 && !IsVigil(mr_season) ? 16 :

			datetime >= (Tridentine::nativity(year) + SECS_PER_DAY) && datetime < (Tridentine::nativity(year) + SECS_PER_WEEK) ? 17 :

			(datetime >= Tridentine::date(17, 12, year) && datetime < Tridentine::date(23, 12, year)
				|| (Tridentine::IsEmberDay(datetime) && (season == SEASON_LENT || month(datetime) == 9 || season == SEASON_ADVENT))) ? 18 :

			((Class_1960(mr_season) == 2 && IsVigil(mr_season))) ? 21 :

			!b_is_sunday && !Tridentine::IsEmberDay(datetime)
			&& (season == SEASON_LENT && datetime >= Tridentine::AshWednesday(year) + SECS_PER_DAY && datetime < Tridentine::PalmSunday(year)) ? 22 :

			Class_1960(mr_season) == 3 && !IsVigil(mr_season) ? 24 :

			!b_is_sunday && season == SEASON_ADVENT && datetime < Tridentine::date(17, 12, year) && !Tridentine::IsEmberDay(datetime) ? 25 :

			Class_1960(mr_season) == 3 && IsVigil(mr_season) ? 26 :

			pp_feast.b_is_saturday_of_our_lady ? 27 :

			!b_is_sunday && Class_1960(mr_season) == 4 ? 28 : 255;


		feast_priority = /*Tridentine::issameday(datetime, Tridentine::nativity(year)) || Tridentine::issameday(datetime, Tridentine::Easter(year))
			|| Tridentine::issameday(datetime, Tridentine::Pentecost(year)) ? 1 :*/

			/*Tridentine::issameday(datetime, Tridentine::MaundyThursday(year)) || Tridentine::IsGoodFriday(datetime) || Tridentine::IsHolySaturday(datetime) ? 2 :*/

			Tridentine::issameday(datetime, Tridentine::Epiphany(year)) /*|| Tridentine::issameday(datetime, Tridentine::Ascension(year))
			|| Tridentine::issameday(datetime, Tridentine::TrinitySunday(year)) || Tridentine::issameday(datetime, Tridentine::CorpusChristi(year))
			|| Tridentine::issameday(datetime, Tridentine::SacredHeart(year))*/ || Tridentine::issameday(datetime, Tridentine::ChristTheKing(year)) ? 3 :

			Tridentine::IsImmaculateConception(datetime) || Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year)) ? 4 :

			Tridentine::issameday(datetime, Tridentine::nativity(year)) - SECS_PER_DAY || Tridentine::issameday(datetime, Tridentine::CircumcisionOfTheLord(year)) ? 5 :

			/*b_is_sunday && ((season == SEASON_LENT || season == SEASON_ADVENT) || (Tridentine::issameday(datetime, Tridentine::QuasimodoSunday(year)))) ? 6 :*/

			/*(Tridentine::issameday(datetime, Tridentine::AshWednesday(year)) || (Tridentine::IsHolyWeek(datetime)
				&& (weekday(datetime) == dowMonday || weekday(datetime) == dowTuesday || weekday(datetime) == dowWednesday))) ? 7 :*/

			!b_is_sunday && Tridentine::issameday(datetime, Tridentine::AllSouls(year)) ? 8 :

			/*Tridentine::issameday(datetime, Tridentine::PentecostVigil(year)) ? 9 :*/

			/*(datetime > Tridentine::Easter(year) && datetime < (Tridentine::Easter(year) + 8 * SECS_PER_DAY))
			|| (datetime > Tridentine::Pentecost(year) && datetime < (Tridentine::Pentecost(year) + 8 * SECS_PER_DAY)) ? 10 :*/

			IsUniversalFeast(datetime, true) ? 11 :

			Class_1960(mr_feast) == 1 ? 12 :

			Tridentine::issameday(datetime, Tridentine::HolyFamily(year)) || Tridentine::issameday(datetime, Tridentine::BaptismOfTheLord(year))
			|| Tridentine::issameday(datetime, Tridentine::PurificationOfMary(year)) || Tridentine::issameday(datetime, Tridentine::TransfigurationOfTheLord(year))
			|| Tridentine::issameday(datetime, Tridentine::HolyCross(year)) || Tridentine::issameday(datetime, Tridentine::DedicationOfTheLateranBasilica(year)) ? 14 :

			/*b_is_sunday && Class_1960(mr_season) == 2 ? 15 :*/

			Class_1960(mr_feast) == 2 && !IsVigil(mr_feast) ? 16 :

			/*datetime >= (Tridentine::nativity(year) + SECS_PER_DAY) && datetime < (Tridentine::nativity(year) + SECS_PER_WEEK) ? 17 :*/

			/*(datetime >= Tridentine::date(17, 12, year) && datetime < Tridentine::date(23, 12, year)
				|| (Tridentine::IsEmberDay(datetime) && (season == SEASON_LENT || month(datetime) == 9 || season == SEASON_ADVENT))) ? 18 :*/

			((Class_1960(mr_feast) == 2 && IsVigil(mr_feast))) ? 21 :

			/*!b_is_sunday && !Tridentine::IsEmberDay(datetime)
			&& (season == SEASON_LENT && datetime >= Tridentine::AshWednesday(year) + SECS_PER_DAY && datetime < Tridentine::PalmSunday(year)) ? 22 :*/

			Class_1960(mr_feast) == 3 && !IsVigil(mr_feast) ? 24 :

			/*!b_is_sunday && season == SEASON_ADVENT && datetime < Tridentine::date(17, 12, year) && !Tridentine::IsEmberDay(datetime) ? 25 :*/

			Class_1960(mr_feast) == 3 && IsVigil(mr_feast) ? 26 :

			/*pp_feast.b_is_saturday_of_our_lady ? 27 :*/

			!b_is_sunday && Class_1960(mr_feast) == 4 ? 28 : 255;
	}
	else {
	// can't find order of precedence for pre-1960 feasts/seasonal days, so will assume they haven't changed a great deal, and rely on these as a rough guide
	
	uint8_t season_vigil_class = VIGIL_NA;
	uint8_t season_vigil_type = VIGIL_TYPE_SUPPRESSED;
	bool b_season_is_vigil = IsVigil(mr_season, datetime, season_vigil_class, season_vigil_type);

	uint8_t feast_vigil_class = VIGIL_NA;
	uint8_t feast_vigil_type = VIGIL_TYPE_SUPPRESSED;
	bool b_feast_is_vigil = IsVigil(mr_feast, datetime, feast_vigil_class, feast_vigil_type);

	season_priority = Tridentine::issameday(datetime, Tridentine::nativity(year)) || Tridentine::issameday(datetime, Tridentine::Easter(year))
		|| Tridentine::issameday(datetime, Tridentine::Pentecost(year)) ? 1 :

		Tridentine::issameday(datetime, Tridentine::MaundyThursday(year)) || Tridentine::IsGoodFriday(datetime) || Tridentine::IsHolySaturday(datetime) ? 2 :

		/*Tridentine::issameday(datetime, Tridentine::Epiphany(year)) ||*/ Tridentine::issameday(datetime, Tridentine::Ascension(year))
		|| Tridentine::issameday(datetime, Tridentine::TrinitySunday(year)) || Tridentine::issameday(datetime, Tridentine::CorpusChristi(year))
		|| Tridentine::issameday(datetime, Tridentine::SacredHeart(year)) /* || Tridentine::issameday(datetime, Tridentine::ChristTheKing(year))*/ ? 3 :

		/*Tridentine::IsImmaculateConception(datetime) || Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year)) ? 4 :*/

		/*Tridentine::issameday(datetime, Tridentine::nativity(year)) - SECS_PER_DAY || Tridentine::issameday(datetime, Tridentine::CircumcisionOfTheLord(year)) ? 5 :*/

		b_is_sunday && ((season == SEASON_LENT || season == SEASON_ADVENT) || (Tridentine::issameday(datetime, Tridentine::QuasimodoSunday(year)))) ? 6 :

		(Tridentine::issameday(datetime, Tridentine::AshWednesday(year)) || (Tridentine::IsHolyWeek(datetime)
			&& (weekday(datetime) == dowMonday || weekday(datetime) == dowTuesday || weekday(datetime) == dowWednesday))) ? 7 :

		/*!b_is_sunday && Tridentine::issameday(datetime, Tridentine::AllSouls(year)) ? 8 :*/

		Tridentine::issameday(datetime, Tridentine::PentecostVigil(year)) ? 9 :

		(datetime > Tridentine::Easter(year) && datetime < (Tridentine::Easter(year) + 8 * SECS_PER_DAY))
		|| (datetime > Tridentine::Pentecost(year) && datetime < (Tridentine::Pentecost(year) + 8 * SECS_PER_DAY)) ? 10 :

		/*IsUniversalFeast(datetime, true) ? 11 :*/

		pp_season.b_is_duplex && pp_season.duplex_class == DUPLEX_CLASS_I/*Class_1960(mr_feast) == 1*/ ? 12 :

		/*Tridentine::issameday(datetime, Tridentine::HolyFamily(year)) || Tridentine::issameday(datetime, Tridentine::BaptismOfTheLord(year))
		|| Tridentine::issameday(datetime, Tridentine::PurificationOfMary(year)) || Tridentine::issameday(datetime, Tridentine::TransfigurationOfTheLord(year))
		|| Tridentine::issameday(datetime, Tridentine::HolyCross(year)) || Tridentine::issameday(datetime, Tridentine::DedicationOfTheLateranBasilica(year)) ? 14 :*/

		b_is_sunday && pp_season.b_is_duplex && pp_season.duplex_class == DUPLEX_CLASS_II /*Class_1960(mr_season) == 2*/ ? 15 :

		pp_season.b_is_duplex && pp_season.duplex_class == DUPLEX_CLASS_II && !b_season_is_vigil /*Class_1960(mr_season) == 2 && !IsVigil(mr_season)*/ ? 16 : //

		datetime >= (Tridentine::nativity(year) + SECS_PER_DAY) && datetime < (Tridentine::nativity(year) + SECS_PER_WEEK) ? 17 :

		(datetime >= Tridentine::date(17, 12, year) && datetime < Tridentine::date(23, 12, year)
			|| (Tridentine::IsEmberDay(datetime) && (season == SEASON_LENT || month(datetime) == 9 || season == SEASON_ADVENT))) ? 18 :

		b_season_is_vigil && season_vigil_type == VIGIL_CLASS_II /*(Class_1960(mr_season) == 2 && IsVigil(mr_season)))*/ ? 21 :

		!b_is_sunday && !Tridentine::IsEmberDay(datetime)
		&& (season == SEASON_LENT && datetime >= Tridentine::AshWednesday(year) + SECS_PER_DAY && datetime < Tridentine::PalmSunday(year)) ? 22 :

		pp_season.b_is_duplex && pp_season.duplex_class < DUPLEX_CLASS_II && !b_season_is_vigil/*Class_1960(mr_season) == 3 && !IsVigil(mr_season)*/ ? 24 :

		!b_is_sunday && season == SEASON_ADVENT && datetime < Tridentine::date(17, 12, year) && !Tridentine::IsEmberDay(datetime) ? 25 :

		pp_season.b_is_duplex && pp_season.duplex_class < DUPLEX_CLASS_II && b_season_is_vigil /*Class_1960(mr_season) == 3 && IsVigil(mr_season)*/ ? 26 :

		pp_feast.b_is_saturday_of_our_lady ? 27 :

		!b_is_sunday && (pp_season.day == DAY_SIMPLEX || pp_season.day == DAY_FERIAL && pp_season.ferial_class == FERIAL) /*Class_1960(mr_season) == 4*/ ? 28 : 255;


	feast_priority = /*Tridentine::issameday(datetime, Tridentine::nativity(year)) || Tridentine::issameday(datetime, Tridentine::Easter(year))
		|| Tridentine::issameday(datetime, Tridentine::Pentecost(year)) ? 1 :*/

		/*Tridentine::issameday(datetime, Tridentine::MaundyThursday(year)) || Tridentine::IsGoodFriday(datetime) || Tridentine::IsHolySaturday(datetime) ? 2 :*/

		Tridentine::issameday(datetime, Tridentine::Epiphany(year)) /*|| Tridentine::issameday(datetime, Tridentine::Ascension(year))
		|| Tridentine::issameday(datetime, Tridentine::TrinitySunday(year)) || Tridentine::issameday(datetime, Tridentine::CorpusChristi(year))
		|| Tridentine::issameday(datetime, Tridentine::SacredHeart(year))*/ || Tridentine::issameday(datetime, Tridentine::ChristTheKing(year)) ? 3 :

		Tridentine::IsImmaculateConception(datetime) || Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year)) ? 4 :

		Tridentine::issameday(datetime, Tridentine::nativity(year)) - SECS_PER_DAY || Tridentine::issameday(datetime, Tridentine::CircumcisionOfTheLord(year)) ? 5 :

		/*b_is_sunday && ((season == SEASON_LENT || season == SEASON_ADVENT) || (Tridentine::issameday(datetime, Tridentine::QuasimodoSunday(year)))) ? 6 :*/

		/*(Tridentine::issameday(datetime, Tridentine::AshWednesday(year)) || (Tridentine::IsHolyWeek(datetime)
			&& (weekday(datetime) == dowMonday || weekday(datetime) == dowTuesday || weekday(datetime) == dowWednesday))) ? 7 :*/

		!b_is_sunday && Tridentine::issameday(datetime, Tridentine::AllSouls(year)) ? 8 :

		/*Tridentine::issameday(datetime, Tridentine::PentecostVigil(year)) ? 9 :*/

		/*(datetime > Tridentine::Easter(year) && datetime < (Tridentine::Easter(year) + 8 * SECS_PER_DAY))
		|| (datetime > Tridentine::Pentecost(year) && datetime < (Tridentine::Pentecost(year) + 8 * SECS_PER_DAY)) ? 10 :*/

		IsUniversalFeast(datetime, true) ? 11 :

		pp_feast.b_is_duplex && pp_feast.duplex_class == DUPLEX_CLASS_I /*Class_1960(mr_feast) == 1*/ ? 12 :

		Tridentine::issameday(datetime, Tridentine::HolyFamily(year)) || Tridentine::issameday(datetime, Tridentine::BaptismOfTheLord(year))
		|| Tridentine::issameday(datetime, Tridentine::PurificationOfMary(year)) || Tridentine::issameday(datetime, Tridentine::TransfigurationOfTheLord(year))
		|| Tridentine::issameday(datetime, Tridentine::HolyCross(year)) || Tridentine::issameday(datetime, Tridentine::DedicationOfTheLateranBasilica(year)) ? 14 :

		/*b_is_sunday && Class_1960(mr_season) == 2 ? 15 :*/

		b_feast_is_vigil && feast_vigil_type == VIGIL_CLASS_II /*Class_1960(mr_feast) == 2 && !IsVigil(mr_feast)*/ ? 16 :

		/*datetime >= (Tridentine::nativity(year) + SECS_PER_DAY) && datetime < (Tridentine::nativity(year) + SECS_PER_WEEK) ? 17 :*/

		/*(datetime >= Tridentine::date(17, 12, year) && datetime < Tridentine::date(23, 12, year)
			|| (Tridentine::IsEmberDay(datetime) && (season == SEASON_LENT || month(datetime) == 9 || season == SEASON_ADVENT))) ? 18 :*/

		((Class_1960(mr_feast) == 2 && IsVigil(mr_feast))) ? 21 :

		/*!b_is_sunday && !Tridentine::IsEmberDay(datetime)
		&& (season == SEASON_LENT && datetime >= Tridentine::AshWednesday(year) + SECS_PER_DAY && datetime < Tridentine::PalmSunday(year)) ? 22 :*/

		pp_feast.b_is_duplex && pp_feast.duplex_class < DUPLEX_CLASS_II && !b_feast_is_vigil /*Class_1960(mr_feast) == 3 && !IsVigil(mr_feast)*/ ? 24 :

		/*!b_is_sunday && season == SEASON_ADVENT && datetime < Tridentine::date(17, 12, year) && !Tridentine::IsEmberDay(datetime) ? 25 :*/

		pp_feast.b_is_duplex && pp_feast.duplex_class < DUPLEX_CLASS_II && b_feast_is_vigil /*Class_1960(mr_feast) == 3 && IsVigil(mr_feast)*/ ? 26 :

		/*pp_feast.b_is_saturday_of_our_lady ? 27 :*/

		!b_is_sunday && (pp_feast.day == DAY_SIMPLEX || pp_feast.day == DAY_FERIAL && pp_feast.ferial_class == FERIAL) /*Class_1960(mr_season) == 4*/ ? 28 : 255;
	}
}



int8_t Precedence::Index1960_y(uint8_t liturgical_class_1960, bool b_is_vigil)
{
	if (!b_is_vigil)
	{
		return liturgical_class_1960 == 3 ? 4 :
			liturgical_class_1960 == 2 ? 2 :
			liturgical_class_1960 == 1 ? 0 : -1;
	}
	else {
		return liturgical_class_1960 == 3 ? 7 :
			liturgical_class_1960 == 2 ? 6 : -1;
	}
}

int8_t Precedence::Index1960_x(uint8_t liturgical_class_1960, bool b_is_vigil, bool b_Sunday, int8_t sunday_class, bool b_Feria, bool b_Advent, bool b_LentAndPassionTide, bool b_is_octave, uint8_t octave_class) {
		return 
			b_Sunday && sunday_class == 1 ? 15 :
			b_Sunday && sunday_class == 2 ? 14 :
			liturgical_class_1960 == 1 && b_Feria ? 13 :
			liturgical_class_1960 == 2 && b_Feria ? 12 :
			liturgical_class_1960 == 3 && b_Feria && b_Advent ? 11 :
			liturgical_class_1960 == 3 && b_Feria && b_LentAndPassionTide ? 10 :
			liturgical_class_1960 == 1 && b_is_vigil ? 9 :
			liturgical_class_1960 == 2 && b_is_vigil ? 8 :
			liturgical_class_1960 == 1 && !b_is_vigil ? 7 :
			liturgical_class_1960 == 2 && !b_is_vigil ? 5 :
			liturgical_class_1960 == 3 ? 3 :
			b_is_octave && octave_class == 1 ? 1 :
			b_is_octave && octave_class == 2 ? 0 : -1;
}

uint8_t Precedence::Class_1960(MissalReading& m) 
{
//	String mr_class = m.cls();
//
//	uint8_t mr_class_index = Tridentine::getClassIndex(mr_class, true);
//	
//	return mr_class_index < 2 ? 4 :
//		   mr_class_index < 5 ? 3 :
//		   mr_class_index < 6 ? 2 :
//		   mr_class_index < 7 ? 1 : 0;
//}
//
//uint8_t Precedence::getClassIndex1960(MissalReading& m) 
//{
	String cls = m.cls();

	if (cls == "") {
		if (m.heading(true).indexOf("Commemoratio:") != -1) {
			return 4;
		}
	}

	//DEBUG_PRT.on();
	//DEBUG_PRT.printf("[%s] [%s]", cls.c_str(), "Dies Octavæ I. classis");
	//DEBUG_PRT.off();

	if (cls.indexOf("Dies Octav") == 0 && cls.indexOf("I. classis") > 0) {	// Easter (testing on Win32, doesn't handle "æ" character too well (utf-8 support is patchy in Windows), hence this kludge
		return 1;
	}

	const char* const newtable[7] = {
		"none",
		"I. classis",
		"II. classis",
		"III. classis",
		"IV. classis",
	};

	for (uint8_t i = 1; i <= 4; i++) {
		if (cls.indexOf(newtable[i]) == 0) {
			return i;
		}
	}
	return 0;
}

bool Precedence::IsVigil(MissalReading& m)
{
	return (m.heading().indexOf("Vigil") != -1);
}

void Precedence::patchAscensionVigilClass(MissalReading& mr)
{
	if (mr.heading().indexOf(F("In Vigilia Ascensionis")) != -1) {
		mr.setClass(F("II. classis"));	// is set to 'Feria' in the 1960 Mass database
	}
}

bool Precedence::getOctave1960(int8_t& octave_class, time64_t datetime)
{
	bool bIsOctave = false;

	int year = Tridentine::year(datetime);
	time64_t Christmas = Tridentine::nativity(year);
	time64_t Easter = Tridentine::Easter(year);
	time64_t Pentecost = Tridentine::Pentecost(year);

	octave_class = 0;

	if (datetime >= Christmas && datetime < Christmas + 8 * SECS_PER_DAY) { bIsOctave = true; octave_class = 2; }
	if (datetime >= Easter && datetime < Easter + 8 * SECS_PER_DAY) { bIsOctave = true; octave_class = 1; };
	if (datetime >= Pentecost && datetime < Pentecost + 8 * SECS_PER_DAY) { bIsOctave = true; octave_class = 1; }

	return bIsOctave;
}
/*
bool Precedence::isSaturdayOfOurLady(time64_t datetime, bool b_use_new_classes, MissalReading& season, MissalReading& feast)
{
	if (weekday(datetime) != 7) return false; // is not saturday!

	uint8_t classindex_season = Tridentine::getClassIndex(season.cls(), b_use_new_classes);
	uint8_t classindex_feast = Tridentine::getClassIndex(feast.cls(), b_use_new_classes);

	uint8_t seas = Tridentine::Season(datetime);
	uint8_t season_week = Tridentine::Season_Week(datetime, seas);
	bool b_is_advent_septuagesima_or_lent = (seas == SEASON_ADVENT || seas == SEASON_SEPTUAGESIMA || seas == SEASON_LENT);
	bool b_is_ember_day = Tridentine::IsEmberDay(datetime);

	if (!b_use_new_classes) {
		return (classindex_season <= 2 && classindex_feast <= 2 && !b_is_advent_septuagesima_or_lent && !b_is_ember_day);
	}
	else {
		uint8_t season_classnumber = Class_1960(season);
		uint8_t feast_classnumber = Class_1960(feast);

		bool b_season_class_4_or_0 = (season_classnumber == 4 || season_classnumber == 0);
		bool b_feast_class_4_or_0 = (feast_classnumber == 4 || feast_classnumber == 0);

		//return (b_season_class_4_or_0 && b_feast_class_4_or_0 && !b_is_advent_septuagesima_or_lent && !b_is_ember_day);
		return (b_season_class_4_or_0 && b_feast_class_4_or_0 && !b_is_advent_septuagesima_or_lent && !b_is_ember_day); //http://divinumofficium.com/www/horas/Help/Rubrics/General%20Rubrics.html#9 
	}
}
*/
void Precedence::pr_1960_0(Ordering& ordering)
{
	//0 (will maintain the ordering of the input season and feast (the table does not specify))
	DEBUG_PRT.println("0 (will maintain the ordering of the input season and feast (the table does not specify))");

	if (ordering.b_is_votive) {
		ordering.ordering[0] = SECOND;
		ordering.ordering[1] = -1;
		ordering.ordering[2] = -1;
		ordering.b_com = false;
		ordering.b_com_at_lauds = false;
		ordering.b_com_at_vespers = false;	// appears 1960 Mass has no commemorations on Saturday of Our Lady (per D.O. Software example)
		return; // votive mass only in this case (ordering.headings[0]=&votive in this case - already determined it is a votive)
	}


	int8_t class_season = ordering.season_classnumber; // Class_1960(*ordering.headings[0]);
	int8_t class_feast = ordering.feast_classnumber; // Class_1960(*ordering.headings[1]);

	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = FIRST;
	ordering.ordering[2] = -1;

	if (class_feast == 0) {	// if there is no calendar feast day, don't try to display the heading for it
		ordering.ordering[1] = -1;
	}
	else if (class_season == 0) {	// if there is no seasonal day, don't try to display the heading for it
		ordering.ordering[0] = FIRST;
		ordering.ordering[1] = -1;
	}
	else if (class_season > class_feast) { // greater class number = lower priority
		ordering.ordering[0] = FIRST;	// swap feast and seasonal - display feast as heading, seasonal day as subheading
		ordering.ordering[1] = SECOND;
	}
	else if (class_season < class_feast) {
		ordering.ordering[1] = -1;	// show commemoration/feast only when the seasonal day is of a higher classnumber
	}
	// otherwise, if the feast and season are of the same class, will show the feast/commemoration as the subheading
}

void Precedence::pr_1960_1(Ordering& ordering)
{
	//1, "Office of 1st, nothing of 2nd"
	// table headings across the top (x direction) are assumed to be seasonal days/feasts, and are ordinally the 2nd place, since the horizontal headings begin with "and"
	// so =1 is first (feast/sanctoral) day (y direction), =0 is second (seasonal) day (x direction)
	DEBUG_PRT.println(F("1: Office of 1st, nothing of 2nd"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
}

void Precedence::pr_1960_2(Ordering& ordering)
{
	//2, "Office of 2nd, nothing of 1st"
	DEBUG_PRT.println(F("2: Office of 2nd, nothing of 1st"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
}

void Precedence::pr_1960_3(Ordering& ordering)
{
	//3, "Office of 1st, com. Of 2nd at Lauds and Vespers"
	DEBUG_PRT.println(F("3: Office of 1st, com. Of 2nd at Lauds and Vespers"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = SECOND;
	ordering.ordering[2] = -1;
	ordering.b_com_at_lauds = true; // always ordering.headings[ordering.ordering[1]] is the commemoration
	ordering.b_com_at_vespers = true; // always ordering.headings[ordering.ordering[1]] is the commemoration
}

void Precedence::pr_1960_4(Ordering& ordering)
{
	//4, "Office of 1st, com. Of 2nd at Lauds"
	DEBUG_PRT.println(F("4: Office of 1st, com.Of 2nd at Lauds"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = SECOND;
	ordering.ordering[2] = -1;
	ordering.b_com_at_lauds = true; // always ordering.headings[ordering.ordering[1]] is the commemoration
}

void Precedence::pr_1960_5(Ordering& ordering)
{
	//5, "Office of 2nd, com. Of 1st at Lauds"
	DEBUG_PRT.println(F("5: Office of 2nd, com. Of 1st at Lauds"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = FIRST;
	ordering.ordering[2] = -1;
	ordering.b_com_at_lauds = true; // always ordering.headings[ordering.ordering[1]] is the commemoration
}

void Precedence::pr_1960_6(Ordering& ordering)
{
	//6, "Office of 1st, transfer of 2nd"
	DEBUG_PRT.println(F("6: Office of 1st, transfer of 2nd"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
	ordering.b_transfer_2nd = true;
	ordering.transfer_heading_index = SECOND;
}

void Precedence::pr_1960_7(Ordering& ordering)
{
	//7, "Office of 2nd, transfer of 1st"
	DEBUG_PRT.println(F("7: Office of 2nd, transfer of 1st"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
	ordering.b_transfer_1st = true;
	ordering.transfer_heading_index = FIRST;
}

void Precedence::pr_1960_8(Ordering& ordering)
{
	//8, "Office of higher, transfer of other"
	DEBUG_PRT.println(F("8: Office of higher, transfer of other"));
	uint8_t feast_classnumber = ordering.feast_classnumber;
	uint8_t season_classnumber = ordering.season_classnumber;

	if (feast_classnumber > season_classnumber) { /* Class_1960(*ordering.headings[FIRST]) > Class_1960(*ordering.headings[SECOND])) {*/ // headings[0] is seasonal day (can be movable feast), headings[1] is the feast day of the current calendar day
		ordering.ordering[0] = SECOND;				// higher classnumber => lower priority
		ordering.ordering[1] = -1;
		ordering.ordering[2] = -1;
		ordering.b_transfer_1st = true;
		ordering.transfer_heading_index = FIRST;
	}
	else {
		ordering.ordering[0] = FIRST;
		ordering.ordering[1] = -1;
		ordering.ordering[2] = -1;
		ordering.b_transfer_2nd = true;
		ordering.transfer_heading_index = SECOND;
	}
}

void Precedence::pr_1960_9(Ordering& ordering)
{
	//9, "Office of movable feast, com. Of other at Lauds"
	DEBUG_PRT.println(F("9: Office of movable feast, com. Of other at Lauds"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = FIRST;
	ordering.ordering[2] = -1;
	ordering.b_com_at_lauds = true;
}

//void HandleCommemorations_1960(time64_t datetime, Ordering& ordering) {
//	1. A 1st or 2nd class feast of the Lord occurring on a Sunday takes the place of that Sunday with all rights and privileges; hence there is no commemoration of the Sunday.
// http://divinumofficium.com/www/horas/Help/Rubrics/Tables%201960.txt
//
//
//}


////////////////////////////////////////////////////////////////////////////////////////////////
//Pre-1960

/*
1, "Office of 1st, nothing of 2nd"
2, "Office of 2nd, nothing of 1st"
3, "Office of 1st, com. Of 2nd at Lauds and Vespers"
4, "Office of 1st, com. Of 2nd at Lauds"
5, "Office of 2nd, com. Of 1st at Lauds"
6, "Office of 1st, transfer of 2nd"
7, "Office of 2nd, transfer of 1st"
8, "Office of higher, transfer of other"
9, "Office of movable feast, com. Of other at Lauds"
*/

/*
0
1	Officium de 1, nihil de 2.
2	Officium de 2, nihil de 1.
3	Officium de 1, Commemoratio de 2.
4	Officium de 2, Commemoratio de 1.
5	Officium de 1, Translatio de 2.
6	Officium de 2, Translatio de 1.
7	Officium de nobiliori, Commemoratio de alio.
8	Officium de nobiliori, Translatio de alio.
*/
#ifdef _TEST_
#define SUNDAY_NA 0
#define SUNDAY_CLASS_I 1
#define SUNDAY_CLASS_II 2
#define SUNDAY_MINOR 3

#define FERIAL_NA 0
#define FERIAL_PRIVILEGED 1
#define FERIAL_MAJOR 2
#define FERIAL 3

#define DUPLEX_NA 0
#define SEMIDUPLEX 1
#define SEMIDUPLEX_CLASS_II 2
#define SEMIDUPLEX_CLASS_I 3
#define DUPLEX_MINOR 4
#define DUPLEX_MAJOR 5
#define DUPLEX_CLASS_II 6
#define DUPLEX_CLASS_I 7

#define DAY_NA 0
#define DAY_SIMPLEX 1
#define DAY_FERIAL 2
#define DAY_SUNDAY 3
#define DAY_SEMIDUPLEX 4
#define DAY_DUPLEX 5
#endif

uint8_t Precedence::Class_pre1960(MissalReading& m, time64_t datetime, bool& b_is_sunday, uint8_t& sunday_class, bool& b_is_ferial , uint8_t& ferial_class, bool& b_is_duplex, uint8_t& duplex_class)
{
	b_is_sunday = false;
	b_is_ferial = false;
	b_is_duplex = false;

	sunday_class = SUNDAY_NA;
	ferial_class = FERIAL_NA;
	duplex_class = DUPLEX_NA;
	/*
	const char* const tradtable[13] = {
		"none",
		"Simplex",
		"Ferial",
		"Feria major",
		"Feria privilegiata",
		"Semiduplex",
		"Semiduplex Dominica minor",
		"Semiduplex II. classis",
		"Semiduplex I. classis",
		"Duplex",
		"Duplex majus",
		"Duplex II. classis",
		"Duplex I. classis", 
	};
	*/
	String cls = m.cls();

	if (Tridentine::sunday(datetime)) {
		b_is_sunday = true;
		sunday_class = cls.indexOf(F("Duplex I.")) > 0 ? SUNDAY_CLASS_I :
					   cls.indexOf(F("Duplex II.")) > 0 ? SUNDAY_CLASS_II :
					   cls.indexOf(F("Semiduplex I.")) != -1 ? SUNDAY_SEMIDUPLEX_CLASS_I :
					   cls.indexOf(F("Semiduplex II.")) != -1 ? SUNDAY_SEMIDUPLEX_CLASS_II :
					   cls.indexOf(F("Dominica minor")) > 0 ? SUNDAY_SEMIDUPLEX_MINOR : SUNDAY_NA;

		if (sunday_class != SUNDAY_NA) return DAY_SUNDAY;
		//return DAY_SIMPLEX;
	}
	
	if (cls.indexOf(F("Feria")) == 0) {
		b_is_ferial = true;
		ferial_class = cls.indexOf(F("major")) > 0 ? FERIAL_MAJOR :
					   cls.indexOf(F("privilegiata")) > 0 ? FERIAL_PRIVILEGED : FERIAL;
		
		return DAY_FERIAL;

	}
	else if (cls.indexOf(F("Semiduplex")) == 0) {
		b_is_duplex = true;
		duplex_class = cls.indexOf(F("Semiduplex I. classis")) == 0 ? SEMIDUPLEX_CLASS_I :
					   cls.indexOf(F("Semiduplex II. classis")) == 0 ? SEMIDUPLEX_CLASS_II : SEMIDUPLEX_MINOR;

		return DAY_SEMIDUPLEX;
	}
	else if (cls.indexOf(F("Duplex")) == 0) {
		b_is_duplex = true;
		duplex_class = cls.indexOf(F("Duplex I. classis")) == 0 ? DUPLEX_CLASS_I :
					   cls.indexOf(F("Duplex II. classis")) == 0 ? DUPLEX_CLASS_II :
					   cls.indexOf(F("Duplex majus")) == 0 ? DUPLEX_MAJOR : DUPLEX_MINOR;
		
		return DAY_DUPLEX;
	}

	return DAY_SIMPLEX;

	//DEBUG_PRT.on();
	//DEBUG_PRT.printf("[%s] [%s]", cls.c_str(), "Dies Octavæ I. classis");
	//DEBUG_PRT.off();
}

#ifdef _TEST_
#define OCTAVE_NONE 0
#define OCTAVE_SIMPLE 1
#define OCTAVE_COMMON 2
#define OCTAVE_PRIVILEGED 3

#define OCTAVE_ORDER_NA 0
#define OCTAVE_ORDER_1st 1
#define OCTAVE_ORDER_2nd 2
#define OCTAVE_ORDER_3rd 3
#endif

bool Precedence::getOctave(time64_t datetime, bool b_get_seasonal, uint8_t mass_type, uint8_t& octave_type, uint8_t& privileged_octave_order, bool& b_is_octave_day)
{
	bool b_is_feast_day = false;
	return getOctave(datetime, b_get_seasonal, mass_type, octave_type, privileged_octave_order, b_is_octave_day, b_is_feast_day);
}


bool Precedence::getOctave(time64_t datetime, bool b_get_seasonal, uint8_t mass_type, uint8_t& octave_type, uint8_t& privileged_octave_order, bool& b_is_octave_day, bool& b_is_feast_day)
{
	//https://en.wikipedia.org/wiki/Octave_(liturgy)#From_Pius_V_to_Pius_XII
	
	int year = Tridentine::year(datetime);

	octave_type = OCTAVE_NONE;
	privileged_octave_order = OCTAVE_ORDER_NA;
	b_is_octave_day = false;
	bool in_octave_day = false;
	bool in_feast_day = false;

	// Privileged Octaves
	// moveable feasts
	if (b_get_seasonal) {
		if (withinOctave(datetime, Tridentine::Easter(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_1st; b_is_octave_day = in_octave_day;  b_is_feast_day = in_feast_day; return true; }

		time64_t Nativity = Tridentine::nativity(year) > datetime ? Tridentine::nativity(year - 1) : Tridentine::nativity(year);
		if (withinOctave(datetime, Nativity, in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_3rd; b_is_octave_day = in_octave_day;  b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::Pentecost(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_1st; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }

		if (mass_type >= MASS_1955) return false;	// 1955 rubrics - only Easter, Christmas and Pentecost octaves remain

		if (mass_type == MASS_DIVINEAFFLATU) {
			if (withinOctave(datetime, Tridentine::StJosephSponsi(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		}

		if (withinOctave(datetime, Tridentine::CorpusChristi(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_2nd; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }

		if (withinOctave(datetime, Tridentine::Ascension(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_3rd; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::SacredHeart(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_3rd; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
	}
	else
	{	// fixed feasts
		if (mass_type >= MASS_1955) return false; // 1955 rubrics - only Easter, Christmas and Pentecost octaves remain

		if (withinOctave(datetime, Tridentine::Epiphany(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_PRIVILEGED; privileged_octave_order = OCTAVE_ORDER_2nd; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }

		// Common Octaves
		if (withinOctave(datetime, Tridentine::ImmaculateConception(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::StJoseph(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }

		if (withinOctave(datetime, Tridentine::NativityStJohnBaptist(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::MartyrdomOfSSPeterAndPaul(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::AssumptionOfMary(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::AllSaints(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_COMMON; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }

		//if (octave_type == OCTAVE_COMMON) { return true; }

		time64_t StStephenProtomartyr = Tridentine::StStephenProtomartyr(year) > datetime ? Tridentine::StStephenProtomartyr(year - 1) : Tridentine::StStephenProtomartyr(year);
		time64_t StJohnApostle = Tridentine::StJohnApostle(year) > datetime ? Tridentine::StJohnApostle(year - 1) : Tridentine::StJohnApostle(year);
		time64_t HolyInnocents = Tridentine::HolyInnocents(year) > datetime ? Tridentine::HolyInnocents(year - 1) : Tridentine::HolyInnocents(year);

		if (withinOctave(datetime, StStephenProtomartyr, in_octave_day, in_feast_day)) { octave_type = OCTAVE_SIMPLE; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, StJohnApostle, in_octave_day, in_feast_day)) { octave_type = OCTAVE_SIMPLE; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, HolyInnocents, in_octave_day, in_feast_day)) { octave_type = OCTAVE_SIMPLE; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::StLawrence(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_SIMPLE; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
		if (withinOctave(datetime, Tridentine::NativityOfMary(year), in_octave_day, in_feast_day)) { octave_type = OCTAVE_SIMPLE; b_is_octave_day = in_octave_day; b_is_feast_day = in_feast_day; return true; }
	}
	//if (octave_type == OCTAVE_SIMPLE) { return true; }

	return false;
}

bool Precedence::withinOctave(time64_t start, time64_t feast) {
	bool b_is_octave_day = false;
	bool b_is_feast_day = false;

	return withinOctave(start, feast, b_is_octave_day, b_is_feast_day);
}

bool Precedence::withinOctave(time64_t start, time64_t feast, bool& b_is_octave_day, bool& b_is_feast_day) {	
	bool b_is_within_octave = (start >= feast && start < (feast + 8 * SECS_PER_DAY));
	b_is_octave_day = (b_is_within_octave && start >= (feast + 7 * SECS_PER_DAY));
	b_is_feast_day = Tridentine::issameday(start, feast);
	time64_t end = feast + 8 * SECS_PER_DAY;
	time64_t end_before_octave_day = feast + 7 * SECS_PER_DAY;

	//DEBUG_PRT.on();
	//DEBUG_PRT.printf("start=%lld, feast=%lld, end=%lld, last day before octave day=%lld, b_is_within_octave=%s, b_is_octave_day=%s\n", start, feast, end, end_before_octave_day, b_is_within_octave ? "true" : "false", b_is_octave_day ? "true" : "false");
	//DEBUG_PRT.off();
	return b_is_within_octave;
}

#ifdef _TEST_
#define VIGIL_NA 0
#define VIGIL 1
#define VIGIL_CLASS_II 2
#define VIGIL_CLASS_I 3

#define VIGIL_TYPE_SUPPRESSED 0
#define VIGIL_TYPE_COMMON 1
#define VIGIL_TYPE_PRIVILEGED 2
#endif



bool Precedence::IsVigil(MissalReading& m, time64_t datetime, uint8_t& vigil_class, uint8_t& vigil_type) {
	bool b_is_vigil = (m.heading().indexOf(F("Vigil")) != -1);
	vigil_class = VIGIL_NA;
	vigil_type = VIGIL_TYPE_SUPPRESSED;

	if (b_is_vigil) {
		if ((m.heading().indexOf(F("Pentecostes")) != -1) || (m.heading().indexOf(F("Nativitatis")) != -1)) {
			vigil_class = VIGIL_CLASS_I;
			vigil_type = VIGIL_TYPE_PRIVILEGED;
		}
		else if (m.heading().indexOf(F("Epiphan")) != -1) {
			vigil_class = VIGIL_CLASS_II;
			vigil_type = VIGIL_TYPE_COMMON;
		}
		else {
			vigil_class = VIGIL;
		}

		int year = Tridentine::year(datetime);

		if (Tridentine::issameday(datetime, Tridentine::AscensionVigil(year)) || 
			Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year) - SECS_PER_DAY) ||
			Tridentine::issameday(datetime, Tridentine::NativityOfJohnBaptist(year) - SECS_PER_DAY) ||
			Tridentine::issameday(datetime, Tridentine::MartyrdomOfSSPeterAndPaul(year) - SECS_PER_DAY) ||
			Tridentine::issameday(datetime, Tridentine::StLawrence(year) - SECS_PER_DAY))
		{
			vigil_type = VIGIL_TYPE_COMMON;
		}
	}

	return b_is_vigil;
}

bool Precedence::isSaturdayOfOurLady(time64_t datetime, uint8_t mass_type, MissalReading& season, MissalReading& feast)
{
	bool b_use_new_classes = (mass_type >= MASS_1960);

	if (weekday(datetime) != 7) return false; // is not saturday!

	uint8_t feast_day = DAY_NA;
	bool b_feast_is_sunday = false;
	bool b_feast_is_ferial = false;
	bool b_feast_is_duplex = false;
	uint8_t feast_sunday_class = SUNDAY_NA;
	uint8_t feast_ferial_class = FERIAL_NA;
	uint8_t feast_duplex_class = DUPLEX_NA;

	uint8_t season_day = DAY_NA;
	bool b_season_is_sunday = false;
	bool b_season_is_ferial = false;
	bool b_season_is_duplex = false;
	uint8_t season_sunday_class = SUNDAY_NA;
	uint8_t season_ferial_class = FERIAL_NA;
	uint8_t season_duplex_class = DUPLEX_NA;

	//uint8_t classindex_season = Tridentine::getClassIndex(season.cls(), b_use_new_classes);
	//uint8_t classindex_feast = Tridentine::getClassIndex(feast.cls(), b_use_new_classes);

	uint8_t seas = Tridentine::Season(datetime);
	uint8_t season_week = Tridentine::Season_Week(datetime, seas);
	int year = Tridentine::year(datetime);
	bool b_is_advent_septuagesima_or_lent = (seas == SEASON_ADVENT /*|| seas == SEASON_SEPTUAGESIMA*/ || seas == SEASON_LENT); // DOR p.xxxix "VIII. The Office of the Blessed Virgin on Saturday: "The Office of the Blessed Virgin on Saturday is said on all Saturdays throughout the year except the following: (a) The Saturdays of Advent and Lent (..)"
	bool b_is_ember_day = Tridentine::IsEmberDay(datetime);

	if (!b_use_new_classes) {
		feast_day = Class_pre1960(feast, datetime, b_feast_is_sunday, feast_sunday_class, b_feast_is_ferial, feast_ferial_class, b_feast_is_duplex, feast_duplex_class);
		season_day = Class_pre1960(season, datetime, b_season_is_sunday, season_sunday_class, b_season_is_ferial, season_ferial_class, b_season_is_duplex, season_duplex_class);

		if (mass_type < MASS_1960) {	// my version of DO set the ferias of Epiphany to be semiduplex (pre-1960 Masses) - This function adjusts this
			uint8_t season = Tridentine::Season(datetime);
			uint8_t season_week = Tridentine::Season_Week(datetime, season);

			if (seas == SEASON_CHRISTMAS && season_week == 0) {
				season_day = DAY_FERIAL;
				b_season_is_ferial = true;
				season_ferial_class = FERIAL_PRIVILEGED;
			}

			if (season == SEASON_EPIPHANY && season_week > 0 && !b_season_is_sunday) {
				b_season_is_duplex = false;
				season_duplex_class = DUPLEX_NA;
				season_day = DAY_SIMPLEX;	// should be DAY_FERIAL, but there is a hack in Index_x() value 2, which tests for != FERIAL_PRIVILEGED rather than == FERIAL_MAJOR, so includes FERIAL as well. This made the calendar work better when testing, but it needs looking at.
				b_season_is_ferial = true;
				season_ferial_class = FERIAL;
			}

		}

		uint8_t octave_type = OCTAVE_NONE;
		uint8_t privileged_octave_order = OCTAVE_ORDER_NA;
		bool b_is_octave_day_feast = false;
		bool b_is_octave_day_season = false;
		bool b_is_octave_first_day_feast = false;
		bool b_is_octave_first_day_season = false;

		bool b_is_octave = (getOctave(datetime, true, mass_type, octave_type, privileged_octave_order, b_is_octave_day_season, b_is_octave_first_day_season) 
						 || getOctave(datetime, false, mass_type, octave_type, privileged_octave_order, b_is_octave_day_feast, b_is_octave_first_day_feast));

		// DOR p.lxvi, Divino Afflatu "Additions and Variations" II. Precedence of Feasts: "b) .. The greater solemnity by reason of the Octave is considered only on the Feast and the Octave Day, not on the days within the Octave."
		//b_is_octave = (mass_type < MASS_DIVINEAFFLATU) ? b_is_octave : (b_is_octave_day_season || b_is_octave_first_day_season || b_is_octave_day_feast || b_is_octave_first_day_feast);

		uint8_t feast_vigil_class = VIGIL_NA;
		uint8_t feast_vigil_type = VIGIL_TYPE_SUPPRESSED;
		bool b_feast_is_vigil = IsVigil(feast, datetime, feast_vigil_class, feast_vigil_type);

		uint8_t season_vigil_class = VIGIL_NA;
		uint8_t season_vigil_type = VIGIL_TYPE_SUPPRESSED;
		bool b_season_is_vigil = IsVigil(season, datetime, season_vigil_class, season_vigil_type);

		bool b_is_vigil = ((b_season_is_vigil && season_vigil_type != VIGIL_TYPE_SUPPRESSED) || (b_feast_is_vigil && feast_vigil_type != VIGIL_TYPE_SUPPRESSED));

		//bool b_is_advent_or_lent = (seas == SEASON_ADVENT || seas == SEASON_LENT);
		//bool b_is_feast_of_nine_lessons = (feast_day == DAY_DUPLEX || feast_day == DAY_SEMIDUPLEX || season_day == DAY_DUPLEX || season_day == DAY_SEMIDUPLEX); // EnglishDORubrics.pdf p. lv, XXVI. Lessons: "On Doubles and Semidoubles nine Lessons are read"
		
		bool b_is_octave_of_Easter_or_Pentecost = (withinOctave(datetime, Tridentine::Easter(year)) || withinOctave(datetime, Tridentine::Pentecost(year)));

		return ((feast_day == DAY_SIMPLEX 
			|| (feast_day == DAY_FERIAL && feast_ferial_class == FERIAL) // this is to handle the days of January being made Ferial in the 1955 Mass. Hope it works! (That is to give precedence to feast of Holy Name, which otherwise has a commemoration for the January Day included in the text, which is not present in the Divinum Officium website for the same date (3-1-2021)
			|| (mass_type == MASS_1955 && feast_day == DAY_SEMIDUPLEX && feast_duplex_class == SEMIDUPLEX_MINOR))
				&& ((season_day == DAY_SIMPLEX && season_ferial_class != FERIAL_PRIVILEGED) || (season_day == DAY_SEMIDUPLEX && season_duplex_class == SEMIDUPLEX_MINOR))
				&& !b_is_advent_septuagesima_or_lent && !b_is_ember_day && !b_is_octave && !b_is_vigil && !b_is_octave_of_Easter_or_Pentecost);

		/*
		return ((feast_day == DAY_SIMPLEX ||
			(feast_day == DAY_FERIAL && feast_ferial_class == FERIAL) || // this is to handle the days of January being made Ferial in the 1955 Mass. Hope it works! (That is to give precedence to feast of Holy Name, which otherwise has a commemoration for the January Day included in the text, which is not present in the Divinum Officium website for the same date (3-1-2021)
			(feast_day == DAY_SEMIDUPLEX && feast_duplex_class == SEMIDUPLEX_MINOR)) &&
			(season_day == DAY_SIMPLEX || (season_day == DAY_SEMIDUPLEX && season_duplex_class == SEMIDUPLEX_MINOR)) &&
			!b_is_advent_septuagesima_or_lent && !b_is_ember_day && !b_is_octave);
		*/


		//return (classindex_season <= 2 && classindex_feast <= 2 && !b_is_advent_septuagesima_or_lent && !b_is_ember_day);
	}
	else {
		uint8_t season_classnumber = Class_1960(season);
		uint8_t feast_classnumber = Class_1960(feast);

		bool b_season_class_4_or_na = (season_classnumber == 4 || season_classnumber == 0);
		bool b_feast_class_4_or_na = (feast_classnumber == 4 || feast_classnumber == 0);

		//return (b_season_class_4_or_0 && b_feast_class_4_or_0 && !b_is_advent_septuagesima_or_lent && !b_is_ember_day);
		return (b_season_class_4_or_na && b_feast_class_4_or_na && !b_is_advent_septuagesima_or_lent && !b_is_ember_day); //http://divinumofficium.com/www/horas/Help/Rubrics/General%20Rubrics.html#9 
	}
}


int8_t Precedence::Index_y(PrecedenceParams& pp)
{
	// feast
	return pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_CLASS_I ? 0 :
		pp.day == DAY_DUPLEX && !pp.b_is_vigil && pp.duplex_class == DUPLEX_CLASS_II ? 1 :
		pp.b_is_octave && pp.b_is_in_octave_day && pp.octave_type == OCTAVE_COMMON ? 2 :
		pp.b_is_octave && !pp.b_is_in_octave_day && pp.octave_type == OCTAVE_COMMON ? 6 :
		pp.b_is_vigil ? 7 :
		pp.b_is_octave && pp.b_is_in_octave_day && pp.octave_type == OCTAVE_SIMPLE ? 8 :
		pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_MAJOR ? 3 :
		pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_MINOR ? 4 :
		pp.day == DAY_SEMIDUPLEX ? 5 :
		pp.day == DAY_SIMPLEX ? 9 : -1;
}

int8_t Precedence::Index_x(PrecedenceParams& pp) 
{
	// season
	return (pp.day == DAY_SUNDAY && (pp.sunday_class == SUNDAY_CLASS_I || pp.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_I)) ? 16 :
		(pp.day == DAY_SUNDAY && pp.sunday_class == SUNDAY_CLASS_II) ? 15 :
		(pp.day == DAY_SUNDAY && pp.sunday_class == SUNDAY_SEMIDUPLEX_MINOR) || pp.b_is_vigil_of_epiphany ? 14 :
		(pp.day == DAY_FERIAL && pp.ferial_class == FERIAL_PRIVILEGED) || (pp.b_is_vigil && pp.vigil_class == VIGIL_CLASS_I) || (pp.b_is_octave && pp.octave_type == OCTAVE_PRIVILEGED && pp.privileged_octave_order == OCTAVE_ORDER_1st) ? 13 :
		(pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_CLASS_I) ? 12 :
		(pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_CLASS_II) ? 11 :
		(pp.b_is_octave && pp.b_is_in_octave_day && pp.octave_type == OCTAVE_PRIVILEGED && pp.privileged_octave_order == OCTAVE_ORDER_2nd) ? 10 :
		((pp.b_is_octave && pp.b_is_in_octave_day) && ((pp.octave_type == OCTAVE_PRIVILEGED && pp.privileged_octave_order == OCTAVE_ORDER_3rd) || pp.octave_type == OCTAVE_COMMON)) ? 9 :
		(pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_MAJOR) ? 8 :
		(pp.day == DAY_DUPLEX && pp.duplex_class == DUPLEX_MINOR) ? 7 :
		//(pp.day == DAY_SEMIDUPLEX || (pp.day == DAY_SUNDAY && (/*pp.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_I ||*/ pp.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_II))) ? 6 : /*&& pp.duplex_class == SEMIDUPLEX (include semiduplexes of I and II class also? [edit: looks like class II only for Sundays] Rubric doesn't say - http://divinumofficium.com/www/horas/Help/Rubrics/EnglishDORubrics.pdf p24*/
		(pp.b_is_octave && pp.octave_type == OCTAVE_PRIVILEGED && pp.privileged_octave_order == OCTAVE_ORDER_2nd) ? 5 :
		(pp.b_is_octave && pp.octave_type == OCTAVE_PRIVILEGED && pp.privileged_octave_order == OCTAVE_ORDER_3rd) ? 4 :
		(pp.b_is_octave && pp.octave_type == OCTAVE_COMMON) ? 3 :
		(pp.day == DAY_SEMIDUPLEX || (pp.day == DAY_SUNDAY && (/*pp.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_I ||*/ pp.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_II))) ? 6 : /*&& pp.duplex_class == SEMIDUPLEX (include semiduplexes of I and II class also? [edit: looks like class II only for Sundays] Rubric doesn't say - http://divinumofficium.com/www/horas/Help/Rubrics/EnglishDORubrics.pdf p24*/
		(pp.day == DAY_FERIAL && pp.ferial_class != FERIAL_PRIVILEGED) ? 2 :
		(pp.b_is_octave && pp.b_is_in_octave_day && pp.octave_type == OCTAVE_SIMPLE) ? 1 :
		pp.b_is_saturday_of_our_lady ? 0 : -1;
}



void Precedence::pr_0(Ordering& ordering)
{
	//0 (will maintain the ordering of the input season and feast (the table does not specify))
	DEBUG_PRT.println(F("0: (will maintain the ordering of the input season and feast (the table does not specify))"));

	if (ordering.b_is_votive) {
		ordering.ordering[0] = SECOND;
		ordering.ordering[1] = THIRD;
		ordering.ordering[2] = -1;
		return; // votive mass only in this case (ordering.headings[0]=&votive in this case - already determined it is a votive)
	}
	int8_t class_season = ordering.season_classnumber; // Class_1960(*ordering.headings[0]);
	int8_t class_feast = ordering.feast_classnumber; // Class_1960(*ordering.headings[1]);

	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = FIRST;
	ordering.ordering[2] = -1;

	if (class_feast == 0) {	// if there is no calendar feast day, don't try to display the heading for it
		ordering.ordering[1] = -1;
	}
	else if (class_season == 0) {	// if there is no seasonal day, don't try to display the heading for it
		ordering.ordering[0] = FIRST;
		ordering.ordering[1] = -1;
	}
	else if (class_season > class_feast) { // greater class number = lower priority
		ordering.ordering[0] = FIRST;	// swap feast and seasonal - display feast as heading, seasonal day as subheading
		ordering.ordering[1] = SECOND;
	}
	else if (class_season < class_feast) {
		ordering.ordering[1] = -1;	// show commemoration/feast only when the seasonal day is of a higher classnumber
	}
	// otherwise, if the feast and season are of the same class, will show the feast/commemoration as the subheading

}

void Precedence::pr_1(Ordering& ordering)
{
	//1	Officium de 1, nihil de 2.
	DEBUG_PRT.println(F("1: Office of 1st, nothing of 2nd"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
}

void Precedence::pr_2(Ordering& ordering)
{
	//2	Officium de 2, nihil de 1.
	DEBUG_PRT.println(F("2: Office of 2nd, nothing of 1st"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
}

void Precedence::pr_3(Ordering& ordering)
{
	//3	Officium de 1, Commemoratio de 2.
	DEBUG_PRT.println(F("3: Office of 1st, commemoration of 2nd"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = SECOND;
	ordering.ordering[2] = -1;
	ordering.b_com = true;
}

void Precedence::pr_4(Ordering& ordering)
{
	//4	Officium de 2, Commemoratio de 1.
	DEBUG_PRT.println(F("4: Office of 2nd, commemoration of 1st"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = FIRST; 
	ordering.ordering[2] = -1;
	ordering.b_com = true;
}

void Precedence::pr_5(Ordering& ordering)
{
	//5	Officium de 1, Translatio de 2.
	DEBUG_PRT.println(F("5: Office of 1st, transfer of 2nd"));
	ordering.ordering[0] = FIRST;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
	ordering.b_transfer_2nd = true;
	ordering.transfer_heading_index = SECOND;

}

void Precedence::pr_6(Ordering& ordering)
{
	//6	Officium de 2, Translatio de 1.
	DEBUG_PRT.println(F("6: Office of 2nd, transfer of 1st"));
	ordering.ordering[0] = SECOND;
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
	ordering.b_transfer_1st = true;
	ordering.transfer_heading_index = FIRST;
}

void Precedence::pr_7(Ordering& ordering)
{
	//7	Officium de nobiliori, Commemoratio de alio.
	DEBUG_PRT.println(F("7: Office of higher, commemoration of other"));
	uint8_t feast_classnumber = ordering.feast_classnumber;
	uint8_t season_classnumber = ordering.season_classnumber;

	// will give any feast priority over a feria. pr 7 only occurs in five cases, and in all that of them I think the feast will be preferred to the Feria of the same class
	if (feast_classnumber > season_classnumber /*|| ordering.headings[0]->name().indexOf("Feria") == -1*/) { // headings[0] is seasonal day (SECOND) (can be movable feast), headings[1] is the feast day of the current calendar day (FIRST)
		ordering.ordering[0] = SECOND;			  // higher classnumber => lower priority (? no!)
		ordering.ordering[1] = FIRST;
		ordering.ordering[2] = -1;
	}
	else {
		ordering.ordering[0] = FIRST;
		ordering.ordering[1] = SECOND;
		ordering.ordering[2] = -1;
	}
	ordering.b_com = true;
}

void Precedence::pr_8(Ordering& ordering)
{
	//8	Officium de nobiliori, Translatio de alio.
	DEBUG_PRT.println(F("8: Office of higher, transfer of other"));
	uint8_t feast_classnumber = ordering.feast_classnumber;
	uint8_t season_classnumber = ordering.season_classnumber;

	// it appears from the table of Primary Doubles of the First Class (DOR. pp.lxxiii - lxxiv) that the precedence of moveable feasts is always higher than fixed feasts.
	// therefore it should be ok if this tablevalue is selected (it only occurs twice, Duplex I classis vs Duplex I classis and Duplex II classis vs Duplex II classis)
	// to set the season (SECOND) as the higher priority option
	ordering.ordering[0] = SECOND;			  // higher classnumber => lower priority (? no!)
	ordering.ordering[1] = -1;
	ordering.ordering[2] = -1;
	ordering.b_transfer_1st = true;
	ordering.transfer_heading_index = FIRST;

/*
	if (feast_classnumber > season_classnumber) {  //Class_1960(*ordering.headings[FIRST]) > Class_1960(*ordering.headings[SECOND])) { // headings[0] is seasonal day (can be movable feast), headings[1] is the feast day of the current calendar day
		ordering.ordering[0] = SECOND;				// higher classnumber => lower priority
		ordering.ordering[1] = -1;
		ordering.ordering[2] = -1;
		ordering.b_transfer_1st = true;
		ordering.transfer_heading_index = FIRST;
	}
	else {
		ordering.ordering[0] = FIRST;
		ordering.ordering[1] = -1;
		ordering.ordering[2] = -1;
		ordering.b_transfer_2nd = true;
		ordering.transfer_heading_index = SECOND;
	}
*/
}

void Precedence::handleCommemorations(time64_t datetime, uint8_t mass_type, Ordering& ordering, PrecedenceParams& pp_season, PrecedenceParams& pp_feast, uint8_t tablevalue)
{
	uint8_t season = Tridentine::Season(datetime);
	uint8_t season_week = Tridentine::Season_Week(datetime, season);
	int year = Tridentine::year(datetime);
	bool b_feast_has_precedence = (ordering.ordering[0] == 1);

	bool b_is_Ash_Wednesday = Tridentine::issameday(datetime, Tridentine::AshWednesday(year));

	//"..The other Sundays and Octaves are commemorated at Vespers and Lauds unless they fall on the Feasts mentioned above" - DOR. p.xli IX. Commemorations. pt. 7
	bool b_is_Corpus_Christi_Octave_Day = (mass_type < MASS_1955 && Tridentine::issameday(datetime, Tridentine::CorpusChristi(year) + SECS_PER_WEEK));
	bool b_is_Sacred_Heart = Tridentine::issameday(datetime, Tridentine::SacredHeart(year));
	bool b_is_Corpus_Christi = Tridentine::issameday(datetime, Tridentine::CorpusChristi(year));

	bool b_is_Christus_Rex = (mass_type > MASS_TRIDENTINE_1910 && Tridentine::issameday(datetime, Tridentine::ChristTheKing(year)));

	bool b_is_Circumcision = Tridentine::issameday(datetime, Tridentine::CircumcisionOfTheLord(year));
	bool b_is_Holy_Name = Tridentine::issameday(datetime, Tridentine::HolyName(year));
	bool b_is_Purification_Of_Mary = Tridentine::issameday(datetime, Tridentine::PurificationOfMary(year));

	bool b_is_StJosephOpificis = (mass_type == MASS_1955 && Tridentine::issameday(datetime, Tridentine::StJosephOpificis(year)));
	bool b_AscensionVigilHasPrecedence = (mass_type == MASS_1955 && Tridentine::issameday(datetime, Tridentine::AscensionVigil(year)));

	bool b_is_NativityStJohnBaptist = Tridentine::issameday(datetime, Tridentine::NativityOfJohnBaptist(year));
	bool b_is_Immaculate_Conception = Tridentine::IsImmaculateConception(datetime);
	bool b_is_Annunciation = Tridentine::IsLadyDay(datetime);
	bool b_is_Precious_Blood_Of_Jesus = Tridentine::issameday(datetime, Tridentine::PreciousBloodOfJesus(year));
	bool b_is_Visitation_of_Mary = Tridentine::issameday(datetime, Tridentine::VisitationOfMary(year));
	bool b_is_Assumption_of_Mary_Vigil = (mass_type <= MASS_1955 && mass_type >= MASS_DIVINEAFFLATU && Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year) - SECS_PER_DAY));
	bool b_is_Assumption_of_Mary = Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year));
	bool b_is_All_Saints = Tridentine::issameday(datetime, Tridentine::AllSaints(year));

	bool b_is_Finding_Of_Holy_Cross = Tridentine::issameday(datetime, Tridentine::FindingOfTheHolyCross(year));
	bool b_is_St_Joachim = Tridentine::issameday(datetime, Tridentine::StJoachimFatherOfMary(year));
	bool b_is_St_Anne = Tridentine::issameday(datetime, Tridentine::StAnneMotherOfMary(year));
	bool b_is_St_Lawrence = Tridentine::issameday(datetime, Tridentine::StLawrence(year));
	bool b_is_St_Michael = Tridentine::issameday(datetime, Tridentine::StMichaelArchangelDedication(year));
	bool b_is_Transfiguration_of_the_Lord = Tridentine::issameday(datetime, Tridentine::TransfigurationOfTheLord(year));

	bool b_is_PeterandPaul = Tridentine::issameday(datetime, Tridentine::MartyrdomOfSSPeterAndPaul(year));
	bool b_is_St_Matthias = Tridentine::issameday(datetime, Tridentine::StMatthias(year));
	bool b_is_SS_Philip_and_James = Tridentine::issameday(datetime, Tridentine::SSPhilipAndJames(year));
	bool b_is_St_James = Tridentine::issameday(datetime, Tridentine::StJames(year));
	bool b_is_St_Bartholomew = Tridentine::issameday(datetime, Tridentine::StBartholomew(year));
	bool b_is_St_Matthew = Tridentine::issameday(datetime, Tridentine::StMatthew(year));
	bool b_is_SS_Simon_and_Jude = Tridentine::issameday(datetime, Tridentine::SSSimonAndJude(year));
	bool b_is_St_Andrew = Tridentine::issameday(datetime, Tridentine::StAndrew(year));
	bool b_is_St_Thomas = Tridentine::issameday(datetime, Tridentine::StThomas(year));

	// Purification of Mary if it is celebrated on a Sunday has no commemoration of the Sunday
	if (Tridentine::IsAlsoFeastOfTheLord(datetime) || b_is_StJosephOpificis) {
		if (pp_season.b_is_sunday) {
			ordering.ordering[0] = FIRST;
			ordering.ordering[1] = SECOND;
			ordering.b_com = true;
#ifdef _WIN32
			Bidi::printf("(FOL patched (Sunday))");
#endif
			return;
		}
		else {
			// No comm. of feria on these days
			ordering.ordering[0] = FIRST;
			ordering.ordering[1] = SECOND;
			ordering.b_com = false;
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;
#ifdef _WIN32
			Bidi::printf("(FOL patched (feria))");
#endif
			return;
		}
	}

	if (pp_season.b_is_sunday && b_is_Immaculate_Conception) { // has to be in Advent, so won't check (Dec 8th)
		//tablevalue = 3; // Officium de 1, nihil de 2.(=1) (Following 1960 Rubrics and D.O. website, Occurrence of Immaculate Conception take precedence if it falls on on Sunday of Advent)
		// D.O. website transfers 1570 Occurrence of Immaculate Conception if it falls on Sunday of Advent, but celbrates it for all other versions.
		// I'm showing the Sunday of Advent as a Commemoration in this case! (=3)
#ifdef _WIN32
		Bidi::printf("<span style='color: darkgreen;'> Immaculate Conception on Advent Sunday </span>");
#endif
		if (mass_type > MASS_TRIDENTINE_1570) {
			// tablevalue = 3 (Celebrate Feast, commemorate Sunday)
			pr_3(ordering);
		}
		else {
			// tablevalue = 6 (transfer Immaculate Conception)
			pr_6(ordering);
		}
		return;
	}


	bool b_commemorate_sunday = pp_season.b_is_sunday && mass_type < MASS_1955
		&& (b_is_Finding_Of_Holy_Cross || b_is_NativityStJohnBaptist || b_is_St_Lawrence || b_is_Transfiguration_of_the_Lord
			|| b_is_Purification_Of_Mary || b_is_Annunciation || b_is_Immaculate_Conception || b_is_Assumption_of_Mary || b_is_Precious_Blood_Of_Jesus || b_is_Visitation_of_Mary
			|| b_is_PeterandPaul || b_is_St_Matthias || b_is_SS_Philip_and_James || b_is_St_James || b_is_St_Bartholomew || b_is_St_Matthew || b_is_SS_Simon_and_Jude || b_is_St_Andrew || b_is_St_Thomas);

	b_commemorate_sunday = b_commemorate_sunday || b_is_Assumption_of_Mary_Vigil || b_is_Christus_Rex || (pp_season.b_is_sunday && b_is_StJosephOpificis);

	if (b_commemorate_sunday) {
		if (season != SEASON_ADVENT && !b_is_Assumption_of_Mary_Vigil && !b_is_Immaculate_Conception) {
#ifdef _WIN32
			Bidi::printf("<span style='color: darkgreen;'> comm. Sunday </span>");
#endif
			ordering.ordering[0] = FIRST;
			ordering.ordering[1] = SECOND;
			ordering.b_com = true;
		}
		else {
#ifdef _WIN32
			Bidi::printf("<span style='color: darkgreen;'> comm. Apostle </span>");
#endif
			ordering.ordering[0] = SECOND; // St Andrew's day Nov 30th can fall on a Sunday of Advent, but the Sunday of Advent has priority
			ordering.ordering[1] = FIRST;  // also, Assumption of Mary vigil is commemorated if it falls on a Sunday (this is pretty tightly decoded, so there may be other similar situations I've missed)
			ordering.b_com = true;
		}

		return;
	}

	if (b_AscensionVigilHasPrecedence && ordering.b_com) {
#ifdef _WIN32
		Bidi::printf("<span style='color: lightgreen;'> Ascension Vigil has precedence (1955) </span>");
#endif
		ordering.ordering[0] = SECOND;
		ordering.ordering[1] = FIRST;
	}

	if (Tridentine::issameday(datetime, Tridentine::AllSouls(year))) {
		if (pp_feast.b_is_sunday) {	// All Souls is commemorated if it falls on Sunday (D.A. Mass) or has priority (1570 and 1910 Masses) (per D.O. software example)
			if (mass_type == MASS_DIVINEAFFLATU) {
				ordering.ordering[0] = SECOND;
				ordering.ordering[1] = FIRST;
#ifdef _WIN32
				Bidi::printf("<span style='color: darkgray;'> All Souls is commemorated (Sunday) (Divino Afflatu) (1955) </span>");
#endif
			}
			else {
				ordering.ordering[0] = FIRST;
				ordering.ordering[1] = SECOND;
#ifdef _WIN32
				Bidi::printf("<span style='color: darkgray;'> All Souls has precedence (Sunday) (1570, 1910, 1955) </span>");
#endif
			}
			ordering.b_com = true;
		}
		else {
			ordering.ordering[0] = FIRST;		// All Souls has priority in all three masses if it does not fall on a Sunday (per D.O. software example)
			ordering.ordering[1] = SECOND;
			ordering.b_com = false;
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;
#ifdef _WIN32
			Bidi::printf("<span style='color: darkgray;'> All Souls has precedence </span>");
#endif
		}
		return;
	}

	// DOR IX. Commemorations: 1. Simple Feasts are commemorated when they concur with the following:
	// (a) A Feast of nine Lessons (even when transferred);
	// (b) A Sunday, Octave or Saturday;
	// (c) a Ferial Office (to fit in the Office of a Sunday that has been passed over).
	bool b_season_is_feast_of_nine_lessons = (pp_season.day == DAY_DUPLEX || pp_season.day == DAY_SEMIDUPLEX); // EnglishDORubrics.pdf p. lv, XXVI. Lessons: "On Doubles and Semidoubles nine Lessons are read"
	bool b_is_saturday = weekday(datetime) == dowSaturday;
	bool b_is_in_octave = (pp_season.b_is_octave && !pp_season.b_is_in_octave_feast_day) || (pp_feast.b_is_octave && !pp_feast.b_is_in_octave_feast_day);
	bool b_is_Holy_Week_privileged_feria = !pp_season.b_is_sunday && Tridentine::IsHolyWeek(datetime);

	if (!pp_feast.b_is_saturday_of_our_lady 
		&& (mass_type < MASS_1955 && pp_feast.b_is_available && pp_feast.day == DAY_SIMPLEX 
			&& (pp_feast.b_is_sunday && ordering.ordering[1] == -1)	|| (!b_season_is_feast_of_nine_lessons && !b_is_Holy_Week_privileged_feria && !b_is_saturday && !b_is_in_octave && !b_is_Ash_Wednesday && ordering.ordering[1] == FIRST)))
	{	// if Simplex feast has been placed in Commemoration slot, promote it to main heading for Mass < 1955 (per D.O. software), and commemorate the Seasonal day

		if (pp_feast.b_is_sunday) {
#ifdef _WIN32
			Bidi::printf("<span style='color: darkslateblue;'> Simplex commemoration promoted (Sunday) (Mass < 1955) </span>");
#endif
			ordering.ordering[1] = FIRST;			// Days in the Octave of Immaculate Conception have priority for D.A. and 1910 Masses (per D.O. software)
			ordering.b_com = true;
		}
		else if (ordering.b_com) {
#ifdef _WIN32
			Bidi::printf("<span style='color: darkslateblue;'> Simplex commemoration promoted (Mass < 1955) </span>");
#endif
			ordering.ordering[0] = FIRST;			// Days in the Octave of Immaculate Conception have priority for D.A. and 1910 Masses (per D.O. software)
			ordering.ordering[1] = SECOND;
			ordering.b_com = false;					// do not Commemorate seasonal day
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;
		}
	}

	if (ordering.ordering[1] != -1) // only needs to be done when there is both a feast and a seasonal day
	{
		if (pp_feast.b_is_saturday_of_our_lady && pp_feast.b_is_available && tablevalue == 0) {
			ordering.ordering[1] = FIRST;	// tablevalue will be 0 on S Maria Sabbato days for all but Simplex feasts, but if there is a commemoration, want the subheading to reflect it
			ordering.b_com = true;	// display commemoration as subheading if so
			return;
		}

	 // handle transfers
	 // fixed feast falling on a movable seasonal day
		bool b_transfer_feast = (!pp_feast.b_is_saturday_of_our_lady && /*pp_feast.b_is_duplex &&*/ (
			(season == SEASON_ADVENT && pp_feast.b_is_sunday && mass_type == MASS_1955 && !Tridentine::issameday(datetime, Tridentine::nativity(year) - SECS_PER_DAY)) // comms. of feasts possible on Sundays of Advent for mass_type < MASS_1955
			|| (pp_season.b_is_sunday && (/*season == SEASON_SEPTUAGESIMA ||*/ (season == SEASON_LENT && mass_type == MASS_1955) || (season == SEASON_EASTER && (season_week <= 1))))		// DOR p.xlii - xliii "X. The Transference of Feasts" (D.O. differs?)
			|| (Tridentine::issameday(datetime, Tridentine::nativity(year)))
			//|| (datetime >= Tridentine::Epiphany(year) && (mass_type < MASS_1955 && datetime < (Tridentine::Epiphany(year) + 8 * SECS_PER_DAY)))
			|| (mass_type == MASS_1955 && Tridentine::issameday(datetime, Tridentine::HolyFamily(year))) // when it falls in the Octave of Epiphany, transfer the day of Epiphany (per D.O.) 1955 Mass only!
			|| (mass_type >= MASS_1955 && b_is_Ash_Wednesday)
			|| (mass_type == MASS_1955 && Tridentine::IsHolyWeek(datetime)) // per D.O., no commemorations in Holy Week in 1955 Mass, but present in earlier Masses 
			|| (season == SEASON_EASTER && season_week == 0 && (mass_type == MASS_1955 || (mass_type < MASS_1955 && weekday(datetime) < dowWednesday)))
			|| (Tridentine::issameday(datetime, Tridentine::Ascension(year)))
			|| (datetime >= Tridentine::PentecostVigil(year) && datetime < (Tridentine::TrinitySunday(year) + SECS_PER_DAY))
			|| (Tridentine::issameday(datetime, Tridentine::CorpusChristi(year))) /*|| (mass_type < MASS_1955 && Tridentine::issameday(datetime, Tridentine::CorpusChristi(year) + SECS_PER_WEEK)))*/	// D.O. doesn't seem to transfer feasts that fall on the Octave Day of Corpus Christi - maybe becuase they are Doctors of the Church in the cases I looked at. As I don't have this information, I will not transfer feasts on this day
			|| (mass_type > MASS_TRIDENTINE_1910 && Tridentine::issameday(datetime, Tridentine::SacredHeart(year)))
			));

		if (b_transfer_feast) {
#ifdef _WIN32
			int8_t reason = 0;
			if (season == SEASON_ADVENT && pp_feast.b_is_sunday && mass_type == MASS_1955 && !Tridentine::issameday(datetime, Tridentine::nativity(year) - SECS_PER_DAY)) reason = 1;
			if (pp_season.b_is_sunday && (/*season == SEASON_SEPTUAGESIMA ||*/ (season == SEASON_LENT && mass_type == MASS_1955) || (season == SEASON_EASTER && (season_week <= 1)))) reason = 2;		// DOR p.xlii - xliii "X. The Transference of Feasts" (D.O. differs?)
			if (Tridentine::issameday(datetime, Tridentine::nativity(year))) reason = 3;
			if (mass_type == MASS_1955 && Tridentine::issameday(datetime, Tridentine::HolyFamily(year))) reason = 4; // when it falls in the Octave of Epiphany, transfer the day of Epiphany (per D.O.)
			if (mass_type >= MASS_1955 && b_is_Ash_Wednesday) reason = 5;
			if (mass_type == MASS_1955 && Tridentine::IsHolyWeek(datetime)) reason = 6;
			if (season == SEASON_EASTER && season_week == 0 && (mass_type == MASS_1955 || (mass_type < MASS_1955 && weekday(datetime) < dowWednesday))) reason = 7;
			if (Tridentine::issameday(datetime, Tridentine::Ascension(year))) reason = 8;
			if (datetime >= Tridentine::PentecostVigil(year) && datetime < (Tridentine::TrinitySunday(year) + SECS_PER_DAY)) reason = 9;
			if (Tridentine::issameday(datetime, Tridentine::CorpusChristi(year))) reason = 10;/*|| (mass_type < MASS_1955 && Tridentine::issameday(datetime, Tridentine::CorpusChristi(year) + SECS_PER_WEEK)))*/	// D.O. doesn't seem to transfer feasts that fall on the Octave Day of Corpus Christi - maybe becuase they are Doctors of the Church in the cases I looked at. As I don't have this information, I will not transfer feasts on this day
			if (mass_type > MASS_TRIDENTINE_1910 && Tridentine::issameday(datetime, Tridentine::SacredHeart(year))) reason = 11;
			Bidi::printf("<span style='color: lightblue;'> transfer feast reason=%d </span>", reason);
#endif
			ordering.ordering[0] = SECOND; // seasonal day only, transfer other (TODO: Transfers written to disk and recalculated - but not even D.O. does this
			ordering.ordering[1] = -1; // 

			ordering.b_com = false;
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;

			ordering.b_transfer_2nd = true;				// need commenting
			ordering.transfer_heading_index = FIRST;	// 
				
			//Tridentine::setDeferredFeast(ordering.headings[1]->_filedir); // PLL-04-10-2024 save filename of Feast to deferred list
			return;
		}


		bool b_transfer_day_in_octave = mass_type < MASS_1955 && pp_season.b_is_octave && !pp_season.b_is_in_octave_day &&
			(b_is_Holy_Name || b_is_Purification_Of_Mary || b_is_Annunciation || b_is_Immaculate_Conception || b_is_Precious_Blood_Of_Jesus || b_is_Visitation_of_Mary
				|| b_is_Assumption_of_Mary || b_is_Finding_Of_Holy_Cross || b_is_NativityStJohnBaptist || b_is_PeterandPaul || b_is_Transfiguration_of_the_Lord
				|| b_is_St_Joachim || b_is_St_Anne || b_is_St_Lawrence || b_is_St_Michael || b_is_All_Saints
				|| b_is_St_Matthias || b_is_SS_Philip_and_James || b_is_St_James || b_is_St_Bartholomew || b_is_St_Matthew || b_is_SS_Simon_and_Jude || b_is_St_Andrew || b_is_St_Thomas);
		//

		// movable seasonal day falling on a fixed feast
		bool b_transfer_season = (b_transfer_day_in_octave || (!pp_season.b_is_sunday && !pp_season.b_is_saturday_of_our_lady && /*pp_season.b_is_duplex &&*/ (
			(Tridentine::issameday(datetime, Tridentine::nativity(year) - SECS_PER_DAY))
			|| (Tridentine::issameday(datetime, Tridentine::nativity(year) + SECS_PER_WEEK))
			|| (datetime >= Tridentine::Epiphany(year) && (mass_type < MASS_1955 && datetime < (Tridentine::Epiphany(year) + (8 * SECS_PER_DAY))))
			/*|| Tridentine::IsImmaculateConception(datetime)*/
			|| (Tridentine::issameday(datetime, Tridentine::NativityOfJohnBaptist(year)) && !b_is_Corpus_Christi_Octave_Day && !b_is_Sacred_Heart)
			|| (Tridentine::issameday(datetime, Tridentine::MartyrdomOfSSPeterAndPaul(year)) && !b_is_Corpus_Christi_Octave_Day && !b_is_Sacred_Heart)
			//|| (Tridentine::issameday(datetime, Tridentine::StJoseph(year))) // is movable - need to fix
			|| (Tridentine::issameday(datetime, Tridentine::AllSaints(year)))
			)));

		if (b_transfer_season) {
#ifdef _WIN32
			int8_t reason = 0;
			//(b_transfer_day_in_octave || (!pp_season.b_is_sunday && !pp_season.b_is_saturday_of_our_lady && (
			if (Tridentine::issameday(datetime, Tridentine::nativity(year) - SECS_PER_DAY)) reason = 1;
			if (Tridentine::issameday(datetime, Tridentine::nativity(year) + SECS_PER_WEEK)) reason = 2;
			if (datetime >= Tridentine::Epiphany(year) && (mass_type < MASS_1955 && datetime < (Tridentine::Epiphany(year) + (8 * SECS_PER_DAY)))) reason = 3;
			if (Tridentine::issameday(datetime, Tridentine::NativityOfJohnBaptist(year)) && !b_is_Corpus_Christi_Octave_Day && !b_is_Sacred_Heart) reason = 4;
			if (Tridentine::issameday(datetime, Tridentine::MartyrdomOfSSPeterAndPaul(year)) && !b_is_Corpus_Christi_Octave_Day && !b_is_Sacred_Heart) reason = 5;
			if (Tridentine::issameday(datetime, Tridentine::AllSaints(year))) reason = 6;
			Bidi::printf("<span style='color: lightblue;'> transfer season reason=%d%s </span>", reason, b_transfer_day_in_octave ? " [b_transfer_day_in_octave == 1] " : "");
#endif
			ordering.ordering[0] = FIRST; // feast day only, transfer other (TODO: Transfers written to disk and recalculated - but not even D.O. does this
			ordering.ordering[1] = -1; // 

			ordering.b_com = false;
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;

			//ordering.b_transfer_2nd = true;
			//ordering.transfer_heading_index = SECOND;
			////Tridentine::setDeferredFeast(ordering.headings[0]->_filedir); // PLL-04-10-2024 save filename of Seasonal day to deferred list
			return;
		}

		bool b_is_after_ashes = (datetime >= (Tridentine::AshWednesday(year) + SECS_PER_DAY) && (datetime < Tridentine::sunday_after(Tridentine::AshWednesday(year))));

		// Handle exceptions when commemorations *are* allowed
		if (pp_season.b_is_available && !(ordering.b_com || ordering.b_com_at_lauds || ordering.b_com_at_vespers)) { // check b_com etc since, if one or more is set, then the decision to display the commemoration has already been taken
			if ((season == SEASON_LENT && (pp_season.b_is_ferial || pp_season.day == DAY_SIMPLEX || pp_season.day == DAY_SEMIDUPLEX))
				|| (season == SEASON_ADVENT && tablevalue != 4 && pp_season.day == DAY_SEMIDUPLEX && pp_season.b_is_sunday == false && !(pp_feast.b_is_vigil && pp_feast.vigil_class > VIGIL)) // table value = 4 => seasonal day has priority. Privileged Vigil will be Nativity Vigil, where no commemoration is shown (per 3a)
				|| (month(datetime) == 9 && Tridentine::IsEmberDay(datetime)) // 2(d)
				|| (mass_type == MASS_1955 &&  //4 b) On feasts of the second class, and on the other Sundays only one commemoration is allowed. (1955 rubrics)
				((!b_feast_has_precedence && pp_feast.b_is_duplex && (pp_feast.duplex_class == DUPLEX_CLASS_II))) ||
					(b_feast_has_precedence && pp_season.b_is_duplex && (pp_season.duplex_class == DUPLEX_CLASS_II)))
				|| (mass_type < MASS_1955 && pp_feast.b_is_sunday && pp_feast.b_is_duplex
					&& pp_season.sunday_class == SUNDAY_SEMIDUPLEX_MINOR  // General Rubrics 1900 p. xxxix "IX. Commemorations": "The Sundays from Pentecost to Advent, from Epiphany to Septuagesima, from Low Sunday to Pentecost exclusive, are commemorated when a Double supplants them. When a Double falls on other Sundays, it is commemorated or transferred ..."
					&& (season == SEASON_EPIPHANY || season == SEASON_AFTER_PENTECOST || (season == SEASON_EASTER && season_week > 0)))
				|| (mass_type < MASS_1955 && b_is_Ash_Wednesday)
				|| (Tridentine::issameday(datetime, Tridentine::MajorRogation(Tridentine::year(datetime)))))
			{

#ifdef _WIN32
				int8_t reason = 0;
				if ((season == SEASON_LENT && (pp_season.b_is_ferial || pp_season.day == DAY_SIMPLEX || pp_season.day == DAY_SEMIDUPLEX))) reason = 1;
				if (season == SEASON_ADVENT && tablevalue != 4 && pp_season.day == DAY_SEMIDUPLEX && pp_season.b_is_sunday == false && !(pp_feast.b_is_vigil && pp_feast.vigil_class > VIGIL)) reason = 2;
				if (month(datetime) == 9 && Tridentine::IsEmberDay(datetime)) reason = 3;
				if (mass_type == MASS_1955 &&
					((!b_feast_has_precedence && pp_feast.b_is_duplex && pp_feast.duplex_class == DUPLEX_CLASS_II) ||
					(b_feast_has_precedence && pp_season.b_is_duplex && pp_season.duplex_class == DUPLEX_CLASS_II))) reason = 4;
				if (mass_type < MASS_1955 && pp_feast.b_is_sunday && pp_feast.b_is_duplex
					&& pp_season.sunday_class == SUNDAY_SEMIDUPLEX_MINOR  // General Rubrics 1900 p. xxxix "IX. Commemorations": "The Sundays from Pentecost to Advent, from Epiphany to Septuagesima, from Low Sunday to Pentecost exclusive, are commemorated when a Double supplants them. When a Double falls on other Sundays, it is commemorated or transferred ..."
					&& (season == SEASON_EPIPHANY || season == SEASON_AFTER_PENTECOST || (season == SEASON_EASTER && season_week > 0))) reason = 5;
				if (mass_type < MASS_1955 && b_is_Ash_Wednesday) reason = 8;
				if (Tridentine::issameday(datetime, Tridentine::MajorRogation(Tridentine::year(datetime)))) reason = 6;
				//if (pp_feast.b_is_vigil && pp_feast.b_is_sunday) reason = 7;

				Bidi::printf(" Allowing commemoration: reason = %d ", reason);
#endif

				/* http://divinumofficium.com/www/horas/Help/Rubrics/1955.txt Title III, Commemorations (but from DivinumOfficium appears to apply to 1570, 1910, Divino Afflatu masses also - need to check more thoroughly)
				2. The commemorations which are never to be omitted and which have
					absolute precedence are :
				...

				c) The ferias of Lent and Advent;

				d) The ferias and Saturday of the Ember Days of September

				e) The Major Litanies
				*/
				ordering.b_com = true;
				if (ordering.headings[1] != NULL && ordering.headings[1]->isCommemorationOnly() /*|| b_is_after_ashes*/) {
#ifdef _WIN32
					//Bidi::printf(" %s ", b_is_after_ashes ? "(after ashes)" : "(is comm.)");
					Bidi::printf("(is comm.)");
					
#endif
					ordering.ordering[0] = 0; // seasonal day has precedence
					ordering.ordering[1] = 1; // feast is commemoration
				}
				else
				{
					ordering.ordering[0] = 1; // feast has precedence
					ordering.ordering[1] = 0; // seasonal day is commemorated
				}
				return;
			}
		}

		/*
			http://divinumofficium.com/www/horas/Help/Rubrics/1955.txt :
			2. The commemorations which are never to be omitted and which have
				*absolute precedence* are :

			a) Any Sunday;

			b) A feast of the first class;

			c) The ferias of Lent and Advent;

			d) The ferias and Saturday of the Ember Days of September

			e) The Major Litanies

			3. Other commemorations which may occur are omitted providing that
				the orations are never more than three.

				4. The addition to and after the commemorations mentioned in n. 2,
				the arrangement regarding commemorations is as follows :

			a) On Sundays of the first class, feasts of the first class, privileged			[handles this]
			ferias and vigils, and moreover in high Masses and solemn votive Masses,
				no commemoration is allowed.
		*/

		// Handle exceptions when commemorations *are not* allowed

		int8_t comm_index = ordering.ordering[1];
		MissalReading* comm = ordering.headings[ordering.ordering[1]];
		//int year = Tridentine::year(datetime);
		bool b_is_feast_of_nine_lessons = (pp_feast.day == DAY_DUPLEX || pp_feast.day == DAY_SEMIDUPLEX); // EnglishDORubrics.pdf p. lv, XXVI. Lessons: "On Doubles and Semidoubles nine Lessons are read"
		bool b_is_ember_day = Tridentine::IsEmberDay(datetime);
		bool b_is_StJosephSponsi = Tridentine::issameday(datetime, Tridentine::StJosephSponsi(year));

		if (comm != NULL && comm->isOpen()) {	// there is a feast
			if (ordering.b_com || ordering.b_com_at_lauds || ordering.b_com_at_vespers || tablevalue == 0) {	// and it is to be commemorated			
				if ((pp_season.b_is_sunday && (mass_type != MASS_DIVINEAFFLATU || mass_type == MASS_DIVINEAFFLATU && Tridentine::IsHolyWeek(datetime)) && (pp_season.sunday_class == SUNDAY_CLASS_I || pp_season.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_I)) // DA Mass appears to allow Commemorations on 1st Class Sundays (per DivinumOfficium software) * Not Palm Sunday apparently (2023)
					|| (pp_season.b_is_duplex && pp_season.duplex_class == DUPLEX_CLASS_I && !(b_is_StJosephSponsi && mass_type == MASS_DIVINEAFFLATU && !pp_season.b_is_sunday)) // When St Joseph Sponsi falls on a weekday, commemorations appear to be allowed, despite the fact it is a Duplex Class I
					|| (pp_season.b_is_ferial && pp_season.ferial_class == FERIAL_PRIVILEGED && !(mass_type < MASS_1955 && (Tridentine::IsHolyWeek(datetime) || b_is_Ash_Wednesday)))
					|| (pp_season.b_is_vigil && pp_season.vigil_class > VIGIL)
					|| (mass_type == MASS_1955 && pp_feast.vigil_type > VIGIL_TYPE_SUPPRESSED)
					|| (mass_type <= MASS_TRIDENTINE_1910 && Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year) - SECS_PER_DAY)) // the Vigil of the Assumption of Mary (14 August) falls permanently within the octave of St. Lawrence in the 1570 and 1910 Masses, and is in the propers baked in as a commemoration, so show no caluclated commemoration on this day for these Masses
					|| (mass_type == MASS_1955 && pp_season.day == DAY_SEMIDUPLEX && !b_is_ember_day && pp_season.duplex_class == SEMIDUPLEX_MINOR && season != SEASON_ADVENT && season != SEASON_LENT) //1. The grade and rite of semi-double is suppressed. 2. The liturgical days which are now marked in the calendar as semi - double rite are celebrated in the simple rite except the ...
					|| (mass_type < MASS_1955 && tablevalue != 4 && !pp_season.b_is_sunday && !pp_season.b_is_saturday_of_our_lady && !pp_season.b_is_octave && !(b_is_feast_of_nine_lessons && (season == SEASON_ADVENT || season == SEASON_LENT)))	// EnglishDORubrics p.xxxix IX: "The Ferias of Advent, Lent ... are commemorated when a Feast of nine Lessons falls on them" [So ferias of other seasons are not- DO seems to confirm]. If tablevalue==4 then the commemoration is of the feast, not the seasonal day
					)
				{

#if _WIN32
					int8_t reason_not = 0;

					if (pp_season.b_is_sunday && (mass_type != MASS_DIVINEAFFLATU || mass_type == MASS_DIVINEAFFLATU && Tridentine::IsHolyWeek(datetime)) && (pp_season.sunday_class == SUNDAY_CLASS_I || pp_season.sunday_class == SUNDAY_SEMIDUPLEX_CLASS_I)) reason_not = 1; // DA Mass appears to allow Commemorations on 1st Class Sundays (per DivinumOfficium software) * Not Palm Sunday apparently (2023)
					if (pp_season.b_is_duplex && pp_season.duplex_class == DUPLEX_CLASS_I && !(b_is_StJosephSponsi && mass_type == MASS_DIVINEAFFLATU && !pp_season.b_is_sunday))  reason_not = 2; // When St Joseph Sponsi falls on a weekday, commemorations appear to be allowed, despite the fact it is a Duplex Class I
					if (pp_season.b_is_ferial && pp_season.ferial_class == FERIAL_PRIVILEGED && !(mass_type < MASS_1955 && (Tridentine::IsHolyWeek(datetime) || b_is_Ash_Wednesday))) reason_not = 3;
					if (pp_season.b_is_vigil && pp_season.vigil_class > VIGIL) reason_not = 4;
					if (mass_type == MASS_1955 && pp_feast.vigil_type > VIGIL_TYPE_SUPPRESSED)  reason_not = 5;
					if (mass_type <= MASS_TRIDENTINE_1910 && Tridentine::issameday(datetime, Tridentine::AssumptionOfMary(year) - SECS_PER_DAY)) reason_not = 6; // the Vigil of the Assumption of Mary (14 August) falls permanently within the octave of St. Lawrence in the 1570 and 1910 Masses, and is in the propers baked in as a commemoration, so show no caluclated commemoration on this day for these Masses
					if (mass_type == MASS_1955 && pp_season.day == DAY_SEMIDUPLEX && !b_is_ember_day && pp_season.duplex_class == SEMIDUPLEX_MINOR && season != SEASON_ADVENT && season != SEASON_LENT)  reason_not = 7; //1. The grade and rite of semi-double is suppressed. 2. The liturgical days which are now marked in the calendar as semi - double rite are celebrated in the simple rite except the ...
					if (mass_type < MASS_1955 && tablevalue != 4 && !pp_season.b_is_sunday && !pp_season.b_is_saturday_of_our_lady && !pp_season.b_is_octave && !(b_is_feast_of_nine_lessons && (season == SEASON_ADVENT || season == SEASON_LENT))) reason_not = 8;	// EnglishDORubrics p.xxxix IX: "The Ferias of Advent, Lent ... are commemorated when a Feast of nine Lessons falls on them" [So ferias of other seasons are not- DO seems to confirm]. If tablevalue==4 then the commemoration is of the feast, not the seasonal day

					Bidi::printf("<i>reason_not=%d</i>", reason_not);
#endif

					// no commemoration is possible in this case			

					// decide whether the feast will be shown in the bottom left of the epaper display or not (does not affect whether or not it is commemorated in the text)
					bool b_dont_display_feast = 
						(pp_feast.b_is_duplex && comm_index == 1 && 
							( // comm_index==1 means that the feast position is not occupied by a seasonal day								 //Vigil of Pentecost which is raised to the double rite. (*In the 1955, all these seasonal days are still recorded as semiduplex, so handle this)
								(
									// these lines correspond to EnglishDORubrics.pdf p.xlii, X, the transference of feasts
									   (b_dont_display_feast = Tridentine::issameday(datetime, Tridentine::Ascension(year)))		//
									|| (b_dont_display_feast = Tridentine::IsHolyWeek(datetime))									//
									|| (b_dont_display_feast = (season == SEASON_EASTER && season_week == 0))						//
									|| (b_dont_display_feast = (datetime >= Tridentine::PentecostVigil(year) && (datetime < Tridentine::TrinitySunday(year) + SECS_PER_DAY)))
									|| (b_dont_display_feast = (Tridentine::issameday(datetime, Tridentine::CorpusChristi(year)) || (mass_type < MASS_1955 && Tridentine::issameday(datetime, Tridentine::CorpusChristi(year) + SECS_PER_WEEK))))
									|| (b_dont_display_feast = (Tridentine::issameday(datetime, Tridentine::SacredHeart(year))))	//
									|| (b_dont_display_feast = (mass_type == MASS_DIVINEAFFLATU && b_is_StJosephSponsi))	//
								)
							)
						|| (b_dont_display_feast = (mass_type < MASS_1955 && b_is_Assumption_of_Mary))	// Seasonal day heading is suppressed for Assumption of Mary in D.O. software, for mass_type < D.A., but the Ordo shows it suppressed for D.A. Mass also.
						);

					if (b_dont_display_feast || !(season == SEASON_EPIPHANY || season == SEASON_SEPTUAGESIMA || season == SEASON_LENT || (season == SEASON_EASTER && season_week > 0) || season == SEASON_AFTER_PENTECOST || Tridentine::IsAlsoFeastOfTheLord(datetime))) {	// this will show the temporal day, but it wil not be commemorated in the propers, during the weeks after the Octave of Easter (which D.O. does do, showing them as "Tempora" rather than Commemoration					
						ordering.ordering[1] = -1;
#ifdef _WIN32
						Bidi::printf("<i> Suppressing commemoration <b>and heading</b> </i>");
#endif
					}
#ifdef _WIN32
					else {
						Bidi::printf("<i> Suppressing commemoration </i>");
					}
#endif
					ordering.b_com = false;
					ordering.b_com_at_lauds = false;
					ordering.b_com_at_vespers = false;
				}
				else if (Tridentine::issameday(datetime, Tridentine::nativity(year) - SECS_PER_DAY)) { // is Christmas Eve
					ordering.ordering[0] = FIRST;
					ordering.ordering[1] = -1; // no subheading for Advent on Christmas Eve
					ordering.b_com = false;
					ordering.b_com_at_lauds = false;
					ordering.b_com_at_vespers = false;
#ifdef _WIN32
					Bidi::printf("<span style='color:darkred;'> no comm. of day of advent on Christmas Eve </span>");
#endif
					return;
				}
				// DOR 1900 p.xxxvii "The Office of Vigils" - "e.g, if the Feast of Corpus Christi falls on the Vigil of St. John the Baptist - the Office of the Vigil is neither said nor commemorated *(except for the Vigil of the Epiphany)* [Jan 5th]" 
				else if (pp_season.b_is_sunday || (mass_type == MASS_1955 && !pp_season.b_is_saturday_of_our_lady && tablevalue != 7 && tablevalue != 0 && pp_feast.day == DAY_SEMIDUPLEX && pp_feast.duplex_class == SEMIDUPLEX_MINOR)) { // see above (semiduplex, for feasts)
					ordering.ordering[0] = 0; // season (sunday)
					ordering.ordering[1] = 1; // make feast heading subordinate to Sunday in these cases
#ifdef _WIN32
					Bidi::printf("<span style='color: teal;'> %s%s </span>", pp_season.b_is_sunday ? "Sunday has precedence " : "Semiduplex ", (mass_type == MASS_1955 && pp_season.b_is_sunday) ? "(1955)" : (mass_type == MASS_1955 && !pp_season.b_is_sunday) ? "Feria has precedence (1955)" : "");
#endif
				}
				else if (mass_type <= MASS_TRIDENTINE_1910
					&& !b_feast_has_precedence
					&& !pp_season.b_is_saturday_of_our_lady
					&& pp_feast.day == DAY_SIMPLEX && pp_season.day == DAY_SEMIDUPLEX
					&& !pp_feast.b_is_sunday
					&& tablevalue == 4
					&& season == SEASON_EASTER && season_week > 0 && season_week < 5)
				{
#ifdef _WIN32
					Bidi::printf("<i> giving simplex feast precedence during Easter for 1570 Mass </i>");
#endif
					// Can't find the rubric for this, but during the weeks of Easter after Low Sunday in the 1570 Mass, in Divinum Officium, Simplex feasts take 
					// precedence over ferial days of Easter, so implementing this here
					ordering.ordering[0] = FIRST;
					ordering.ordering[1] = SECOND;
					ordering.b_com = false;
					ordering.b_com_at_lauds = false;
					ordering.b_com_at_vespers = false;
				}

				bool b_is_octave_day_of_immaculate_conception = false;
				bool b_is_feast_day_of_immaculate_conception = false;
				bool b_is_within_octave_of_ImmaculateConception = withinOctave(datetime, Tridentine::ImmaculateConception(year), b_is_octave_day_of_immaculate_conception, b_is_feast_day_of_immaculate_conception);
				bool b_is_within_octave_of_Christmas = withinOctave(datetime, Tridentine::nativity(year));
				if (pp_season.b_is_sunday && mass_type == MASS_1955 && (b_is_octave_day_of_immaculate_conception || b_is_within_octave_of_Christmas))
				{
#ifdef _WIN32
					Bidi::printf("<span style='color: salmon;'> Sunday, octave day of Immaculate Conception or within Octave of Christmas (1955 Mass) </span>");
#endif
					ordering.ordering[1] = -1; // The octave day of Our Lady's Immaculate Conception is not commemorated on Sunday in Advent in the 1955 Mass (but it is in earlier Masses)
											   // Commemoration of the current day within the Octave of Christmas is not commemorated on Sunday within the Octave in 1955 Mass (per Divinum Officium example, though I can't find the rubric for it)
				}

				if (b_is_within_octave_of_ImmaculateConception && !pp_feast.b_is_sunday
					&& !b_is_feast_day_of_immaculate_conception
					&& !b_is_octave_day_of_immaculate_conception
					&& (mass_type == MASS_DIVINEAFFLATU || mass_type == MASS_TRIDENTINE_1910)
					/*&& (comm->name().indexOf(F("Octavam")) != -1)*/) {	// if in here, already have a commemoration.

#ifdef _WIN32
					Bidi::printf("<span style='color: skyblue;'> Day in octave of Immaculate Conception has priority </span>");
#endif
					ordering.ordering[0] = FIRST;			// Days in the Octave of Immaculate Conception have priority for D.A. and 1910 Masses (per D.O. software)
					ordering.ordering[1] = SECOND;
					ordering.b_com = true;
				}
			}
		}
	}
/*
	if (mass_type < MASS_1955 && pp_feast.b_is_available && pp_feast.day == DAY_SIMPLEX && (pp_feast.b_is_sunday && ordering.ordering[1] == -1) || ordering.ordering[1] == FIRST) {	// if Simplex feast has been placed in Commemoration slot, promote it to main heading for Mass < 1955 (per D.O. software), and commemorate the Seasonal day
		if (pp_feast.b_is_sunday) {
#ifdef _WIN32
			Bidi::printf("<span style='color: darkslateblue;'> Simplex commemoration promoted (Sunday) (Mass < 1955) </span>");
#endif
			ordering.ordering[1] = FIRST;			// Days in the Octave of Immaculate Conception have priority for D.A. and 1910 Masses (per D.O. software)
			ordering.b_com = true;
		}
		else if (ordering.b_com) {
#ifdef _WIN32
			Bidi::printf("<span style='color: darkslateblue;'> Simplex commemoration promoted (Mass < 1955) </span>");
#endif
			ordering.ordering[0] = FIRST;			// Days in the Octave of Immaculate Conception have priority for D.A. and 1910 Masses (per D.O. software)
			ordering.ordering[1] = SECOND;
			ordering.b_com = false;					// do not Commemorate seasonal day
			ordering.b_com_at_lauds = false;
			ordering.b_com_at_vespers = false;
		}
	}
*/
}

void Precedence::Promote_Sundays_1955_Advent_and_Lent(time64_t datetime, PrecedenceParams& pp, uint8_t mass_type)
{
	uint8_t season = Tridentine::Season(datetime);
	if (pp.b_is_sunday && (season == SEASON_ADVENT || season == SEASON_LENT)) {
		pp.duplex_class = DUPLEX_CLASS_I;
		pp.sunday_class = SUNDAY_CLASS_I;
	}
}

void Precedence::Set_Ember_Days_to_Semiduplex_1955(time64_t datetime, PrecedenceParams& pp, uint8_t mass_type)
{
	bool b_is_Ember_Day = Tridentine::IsEmberDay(datetime);

	if (b_is_Ember_Day && mass_type == MASS_1955) {
		pp.b_is_duplex = true;
		pp.duplex_class = SEMIDUPLEX_CLASS_II;
		pp.day = DAY_SEMIDUPLEX;
		//pp.day = DAY_FERIAL;
		//pp.b_is_ferial = true;
		//pp.ferial_class = FERIAL_PRIVILEGED;
	}
}


void Precedence::Set_Epiphany_Ferials(time64_t datetime, PrecedenceParams& pp, uint8_t mass_type)
{
	if (mass_type == MASS_1955 /*< MASS_1960*/) {	// my version of DO set the ferias of Epiphany to be semiduplex (pre-1960 Masses) - This function adjusts this
		uint8_t season = Tridentine::Season(datetime);
		uint8_t season_week = Tridentine::Season_Week(datetime, season);
		if (season == SEASON_EPIPHANY && season_week > 0 && !pp.b_is_sunday) {
			pp.b_is_duplex = false;
			pp.duplex_class = DUPLEX_NA;
			pp.day = DAY_SIMPLEX;	// should be DAY_FERIAL, but there is a hack in Index_x() value 2, which tests for != FERIAL_PRIVILEGED rather than == FERIAL_MAJOR, so includes FERIAL as well. This made the calendar work better when testing, but it needs looking at.
			pp.b_is_ferial = true;
			pp.ferial_class = FERIAL;
		}
	}
}

void Precedence::Set_Privileged_Ferias(time64_t datetime, PrecedenceParams& pp_season, uint8_t mass_type)
{
	//Privileged ferias - http://divinumofficium.com/www/horas/Help/Rubrics/EnglishDORubrics.pdf p.lxxiii
	//"which, when they fall on any Feasts whatever, are preferred to them - Ash Wednesday, All the Ferias in Holy Week"

	int year = Tridentine::year(datetime);
	int seas = Tridentine::Season(datetime);
	int season_week = Tridentine::Season_Week(datetime, seas);

	if (mass_type < MASS_1960) {	// my version of DO set the ferias of Epiphany to be semiduplex (pre-1960 Masses) - This function adjusts this
		if ((seas == SEASON_CHRISTMAS && season_week == 0)
			|| (Tridentine::IsHolyWeek(datetime) && !pp_season.b_is_sunday) 
			|| Tridentine::issameday(datetime, Tridentine::AshWednesday(year))) {
			pp_season.day = DAY_FERIAL;
			pp_season.b_is_ferial = true;
			pp_season.ferial_class = FERIAL_PRIVILEGED;
		}
	}
}