#pragma once
#ifndef _PRECEDENCE_H_
#define _PRECEDENCE_H_

#include <stdint.h>
#include "WString.h"
#include "Tridentine.h"
#include "TimeLib.h"

typedef struct Ordering {
	MissalReading* headings[3];
	// Missalreadings should be passed in as headings[]={Seasonal, Feast, Votive}
	int8_t ordering[3];
	bool b_com_at_vespers;
	bool b_com_at_lauds;
	bool b_com;
	int8_t transfer_heading_index;
	bool b_transfer_1st;
	bool b_transfer_2nd;
	uint8_t season_classnumber;
	uint8_t feast_classnumber;
	uint8_t votive_classnumber;
	bool b_feast_is_commemoration;
	bool b_is_votive;
} Ordering;

typedef struct PrecedenceParams {
	bool b_is_available;

	uint8_t day;
	
	bool b_is_sunday;
	uint8_t sunday_class;

	bool b_is_ferial;
	uint8_t ferial_class;

	bool b_is_vigil;
	uint8_t vigil_class;
	uint8_t vigil_type;
	bool b_is_vigil_of_epiphany;

	bool b_is_octave;
	uint8_t octave_type;
	uint8_t privileged_octave_order;
	bool b_is_in_octave_day;
	bool b_is_in_octave_feast_day;

	bool b_is_duplex;
	uint8_t duplex_class;

	bool b_is_saturday_of_our_lady;
} PrecedenceParams;

typedef struct PrecedenceParams_1960 {
	bool b_is_available;

	uint8_t day;
	uint8_t liturgical_class;

	bool b_is_sunday;
	uint8_t sunday_class;

	bool b_is_ferial;
	uint8_t ferial_class;

	bool b_is_vigil;
	uint8_t vigil_class;
	uint8_t vigil_type;

	bool b_is_octave;
	uint8_t octave_type;
	uint8_t privileged_octave_order;
	bool b_is_in_octave_day;

	bool b_is_saturday_of_our_lady;
} PrecedenceParams_1960;

class Precedence
{
public:
#define FIRST 1		// corresponds to sanctorale feasts
#define SECOND 0	// corresponds to moveable and temporale feasts
#define THIRD 2 // corresponds to the votive feast, or the seasonal day on votive days

#define MR_SEASON 0
#define MR_FEAST 1
#define MR_VOTIVE 2
#define MR_NONE -1


	static void doOrdering(time64_t datetime, uint8_t mass_type, MissalReading& season, MissalReading& feast, MissalReading& votive, Ordering& ordering);

	static uint8_t Class_1960(MissalReading& m);
	//static uint8_t getClassIndex1960(MissalReading& m);
	static bool IsUniversalFeast(time64_t datetime, bool b_fixed_feast_only = false);
	//static bool IsFeastOfTheLord(time64_t datetime);
	static void Priority(uint8_t mass_type, time64_t datetime, MissalReading& mr_feast, MissalReading& mr_season, PrecedenceParams& pp_feast, PrecedenceParams& pp_season, int8_t& season_priority, int8_t& feast_priority);

	static int8_t Index1960_y(uint8_t liturgical_class_1960, bool b_is_vigil);
	static int8_t Index1960_x(uint8_t liturgical_class_1960, bool b_is_vigil, bool b_Sunday, int8_t sunday_class, bool b_Feria, bool b_Advent, bool b_LentAndPassionTide, bool b_is_octave, uint8_t octave_class);
	static bool IsVigil(MissalReading& m);
	static void patchAscensionVigilClass(MissalReading& mr);
	static bool getOctave1960(int8_t& octave_class, time64_t datetime);
	static bool isSaturdayOfOurLady(time64_t datetime, /*bool b_use_new_classes*/uint8_t mass_type, MissalReading& season, MissalReading& feast);
	
	
	static void pr_1960_0(Ordering& ordering);
	static void pr_1960_1(Ordering& ordering);
	static void pr_1960_2(Ordering& ordering);
	static void pr_1960_3(Ordering& ordering);
	static void pr_1960_4(Ordering& ordering);
	static void pr_1960_5(Ordering& ordering);
	static void pr_1960_6(Ordering& ordering);
	static void pr_1960_7(Ordering& ordering);
	static void pr_1960_8(Ordering& ordering);
	static void pr_1960_9(Ordering& ordering);


#define SUNDAY_NA 0
#define SUNDAY_CLASS_I 1
#define SUNDAY_CLASS_II 2
#define SUNDAY_SEMIDUPLEX_CLASS_I 4
#define SUNDAY_SEMIDUPLEX_CLASS_II 5
#define SUNDAY_SEMIDUPLEX_MINOR 6

#define FERIAL_NA 0
#define FERIAL_PRIVILEGED 1
#define FERIAL_MAJOR 2
#define FERIAL 3

#define DUPLEX_NA 0
#define SEMIDUPLEX_MINOR 1
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
	static uint8_t Class_pre1960(MissalReading& m, time64_t datetime, bool& b_is_sunday, uint8_t& sunday_class, bool& b_is_ferial , uint8_t& ferial_class, bool& b_is_duplex, uint8_t& duplex_class);
	
#define OCTAVE_NONE 0
#define OCTAVE_SIMPLE 1
#define OCTAVE_COMMON 2
#define OCTAVE_PRIVILEGED 3

#define OCTAVE_ORDER_NA 0
#define OCTAVE_ORDER_1st 1
#define OCTAVE_ORDER_2nd 2
#define OCTAVE_ORDER_3rd 3
	static bool getOctave(time64_t datetime, bool b_get_seasonal, uint8_t mass_type, uint8_t& octave_type, uint8_t& privileged_octave_order, bool& b_is_octave_day);
	static bool getOctave(time64_t datetime, bool b_get_seasonal, uint8_t mass_type, uint8_t& octave_type, uint8_t& privileged_octave_order, bool& b_is_octave_day, bool& b_is_feast_day);
	static bool withinOctave(time64_t start, time64_t feast);
	static bool withinOctave(time64_t start, time64_t feast, bool& bIsOctaveDay, bool& b_is_feast_day);
	
#define VIGIL_NA 0
#define VIGIL 1
#define VIGIL_CLASS_II 2
#define VIGIL_CLASS_I 3

#define VIGIL_TYPE_SUPPRESSED 0
#define VIGIL_TYPE_COMMON 1
#define VIGIL_TYPE_PRIVILEGED 2

	static bool IsVigil(MissalReading& m, time64_t datetime, uint8_t& vigil_class, uint8_t& vigil_type);
	
	//static bool isSaturdayOfOurLady2(time64_t datetime, bool b_use_new_classes, MissalReading& season, MissalReading& feast);
	
	static int8_t Index_x(PrecedenceParams& pp);
	static int8_t Index_y(PrecedenceParams& pp);

	static void pr_0(Ordering& ordering);
	static void pr_1(Ordering& ordering);
	static void pr_2(Ordering& ordering);
	static void pr_3(Ordering& ordering);
	static void pr_4(Ordering& ordering);
	static void pr_5(Ordering& ordering);
	static void pr_6(Ordering& ordering);
	static void pr_7(Ordering& ordering);
	static void pr_8(Ordering& ordering);
	static void handleCommemorations(time64_t datetime, uint8_t mass_type, Ordering& ordering, PrecedenceParams& pp_season, PrecedenceParams& pp_feast, uint8_t tablevalue);
	static void Promote_Sundays_1955_Advent_and_Lent(time64_t datetime, PrecedenceParams& pp, uint8_t mass_type);
	static void Set_Ember_Days_to_Semiduplex_1955(time64_t datetime, PrecedenceParams& pp, uint8_t mass_type);
	static void Set_Epiphany_Ferials(time64_t datetime, PrecedenceParams& pp, uint8_t mass_type);
	static void Set_Privileged_Ferias(time64_t datetime, PrecedenceParams& pp_season, uint8_t mass_type);
};




/*
"Table of Occurrence (1960)"

"Feast or Vigil type", "and day within 2nd class octave", "and day within 1st class octave", "and particular 3rd class feast", "and universal 3rd class feast", "and particular 2nd class feast", "and universal 2rd class feast", "and particular 1st class feast", "and universal 1st class feast", "and 2nd class vigil", "and 1st class vigil", "and 3rd class feria of Lent and Passiontide", "and 3rd class feria of Advent", "and 2nd class feria", "and 1st class feria", "and 2nd class Sunday", "and 1st class Sunday"
"Universal 1st cl. Feast", 0, 7, 1, 1, 1, 1, 6, 8, 1, 7, 3, 3, 3, 7, 3, 7
"Particular 1st cl. Feast", 3, 7, 1, 1, 1, 1, 8, 7, 1, 7, 3, 3, 3, 7, 3, 7
"Universal 2nd cl. Feast", 3, 2, 4, 4, 4, 0, 2, 2, 4, 2, 3, 3, 3, 2, 5, 2
"Particular 2nd cl. Feast", 0, 2, 4, 4, 9, 5, 2, 2, 4, 2, 3, 3, 5, 2, 5, 2
"Universal 3rd cl. Feast", 0, 2, 5, 0, 5, 5, 2, 2, 5, 2, 5, 3, 5, 2, 2, 2
"Particular 3rd cl. Feast", 0, 2, 9, 4, 5, 5, 2, 2, 5, 2, 5, 3, 5, 2, 2, 2
"2nd cl. Vigil", 0, 0, 4, 4, 5, 5, 2, 2, 0, 0, 0, 0, 0, 0, 2, 0
"3rd cl. Vigil", 0, 0, 5, 0, 5, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0

"Key"
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

#endif