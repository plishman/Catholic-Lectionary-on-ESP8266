#pragma once

#ifndef _ENUMS_H
#define _ENUMS_H

#include "RCGlobals.h"

class Enums {
public:
	enum I18nLanguages {
		LANGUAGE_EN = 0,
		LANGUAGE_IT,
		LANGUAGE_FR,
		LANGUAGE_LA,
		LANGUAGE_CS
	};

	enum Season {
		SEASON_ADVENT,
		SEASON_CHRISTMAS,
		SEASON_LENT,
		SEASON_EASTER,
		SEASON_ORDINARY
	};

	enum Solemnities {
		SOLEMNITIES_NATIVITY = 0,
		SOLEMNITIES_HOLY_FAMILY,
		SOLEMNITIES_MOTHER_OF_GOD,
		SOLEMNITIES_EPIPHANY,
		SOLEMNITIES_BAPTISM_OF_LORD,
		SOLEMNITIES_ASH_WEDNESDAY,
		SOLEMNITIES_PALM_SUNDAY,
		SOLEMNITIES_GOOD_FRIDAY,
		SOLEMNITIES_HOLY_SATURDAY,
		SOLEMNITIES_EASTER_SUNDAY,
		SOLEMNITIES_ASCENSION,
		SOLEMNITIES_PENTECOST,
		SOLEMNITIES_CHRIST_PRIEST, // optional in some areas
		SOLEMNITIES_HOLY_TRINITY,
		SOLEMNITIES_CORPUS_CHRISTI,
		SOLEMNITIES_SACRED_HEART,
		SOLEMNITIES_IMMACULATE_HEART,
		SOLEMNITIES_CHRIST_KING
	};

	enum Ranks { // the ordinal number is used as an index into the array of ranks (the description), *and also for comparing seniority*
		RANKS_TRIDUUM = 13,									//= Rank.new(1.1, 'rank.1_1'),
		RANKS_PRIMARY = 12,									//= Rank.new(1.2, 'rank.1_2'), # description may not be exact
		RANKS_SOLEMNITY_GENERAL = 11,						//= Rank.new(1.3, 'rank.1_3', 'rank.short.solemnity'), # description may not be exact
		RANKS_SOLEMNITY_PROPER = 10,						//= Rank.new(1.4, 'rank.1_4', 'rank.short.solemnity'),
															//
		RANKS_FEAST_LORD_GENERAL = 9,						//= Rank.new(2.5, 'rank.2_5', 'rank.short.feast'),
		RANKS_SUNDAY_UNPRIVILEGED = 8,						//= Rank.new(2.6, 'rank.2_6', 'rank.short.sunday'),
		RANKS_FEAST_GENERAL = 7,							//= Rank.new(2.7, 'rank.2_7', 'rank.short.feast'),
		RANKS_FEAST_PROPER = 6,								//= Rank.new(2.8, 'rank.2_8', 'rank.short.feast'),
		RANKS_FERIAL_PRIVILEGED = 5,						//= Rank.new(2.9, 'rank.2_9', 'rank.short.ferial'),

		RANKS_MEMORIAL_GENERAL = 4,							//= Rank.new(3.10, 'rank.3_10', 'rank.short.memorial'),
		RANKS_MEMORIAL_PROPER = 3,							//= Rank.new(3.11, 'rank.3_11', 'rank.short.memorial'),
		RANKS_MEMORIAL_OPTIONAL = 2,						//= Rank.new(3.12, 'rank.3_12', 'rank.short.memorial_opt'),
		RANKS_FERIAL = 1,									//= Rank.new(3.13, 'rank.3_13', 'rank.short.ferial'),
															//# not included as a celebration rank on it's own
															//# in the Table of Liturgical Days
		RANKS_COMMEMORATION = 0,							//= Rank.new(4.0, 'rank.4_0', 'rank.short.commemoration')
	};

	enum Colours {
		COLOURS_GREEN = 0,
		COLOURS_VIOLET,
		COLOURS_WHITE,
		COLOURS_RED,
	};

	enum Liturgical_Year {
		LITURGICAL_YEAR_A = 0,
		LITURGICAL_YEAR_B,
		LITURGICAL_YEAR_C
	};

	enum Liturgical_Cycle {
		LITURGICAL_CYCLE_I = 0,
		LITURGICAL_CYCLE_II
	};

	Enums();
	~Enums();
};
#endif
