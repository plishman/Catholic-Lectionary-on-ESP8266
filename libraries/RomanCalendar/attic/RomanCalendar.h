#pragma once
#ifndef RomanCalender_h
#define RomanCalendar_h

#ifdef _WIN32
	const int BEGIN_EPOCH = 1900; // 1900 for 64-bit time64_t, sometimes 1970 (may be on embedded system)
#else
	const int BEGIN_EPOCH = 1970; // 1900 for 64-bit time64_t, sometimes 1970 (may be on embedded system)
#endif

const int WEEK = 7;
const int DAY = 24 * 3600;

class RomanCalendar {
public:
	char _buffer[128];
	char _ordinal[32];
	char _rank[128];
	const char* _season;
	const char* _colour;
	int _bufferlength = 128;

	enum Season {
		SEASON_ADVENT,
		SEASON_CHRISTMAS,
		SEASON_LENT,
		SEASON_EASTER,
		SEASON_ORDINARY
	};

	static const char* const SEASONS[5];

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

	enum RankType {
		RANKTYPE_EASTER_TRIDUUM, // in the original ruby code, these first two were not included. They map to null text in the short descriptions
		RANKTYPE_PRIMARY_LITURGICAL_DAYS, // 
		RANKTYPE_SOLEMNITY,
		RANKTYPE_FEAST,
		RANKTYPE_SUNDAY,
		RANKTYPE_MEMORIAL,
		RANKTYPE_MEMORIAL_OPT,
		RANKTYPE_FERIAL,
		RANKTYPE_COMMEMORATION
	};

	static const RankType RANK_TYPE[14];

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
		SOLEMNITIES_HOLY_TRINITY,
		SOLEMNITIES_CORPUS_CHRISTI,
		SOLEMNITIES_SACRED_HEART,
		SOLEMNITIES_IMMACULATE_HEART,
		SOLEMNITIES_CHRIST_KING
	};

	enum Colours {
		COLOURS_GREEN = 0,
		COLOURS_VIOLET,
		COLOURS_WHITE,
		COLOURS_RED,
	};

	typedef struct {
		Solemnities s;
		Colours c;
		Ranks r;
	} Solemnity;

	static const char* const DAYS_OF_WEEK[7];

	static const char* const RANK_PRIORITY[14];
	static const char* const RANK_NAME[14];
	static const char* const RANK_TYPES[9];
	static const char* const COLOURS[4];

	static const char* const SUNDAY_TRANSFERABLE_SOLEMNITIES[3];
	static const char* const SOLEMNITIES[17];
	static const Ranks SOLEMNITIES_RANKS[17];
	static const Colours SOLEMNITIES_COLOURS[17];
	/*
	struct Celebration {
	CelebrationName celebration_name;
	CelebrationFullName celebration_full_name;
	Colour colour;
	Season season;
	RankEnum rank;

	};
	*/
	bool _transfer_to_sunday; // flag determines whether epiphany, ascension and corpus Christi should be transferred to sunday (us, uk etc)

	RomanCalendar(bool transfer_to_sunday);
	~RomanCalendar(void);

	void datestests();
	bool getTm(int day, int month, int year, int hours, int minutes, int seconds, struct tm* ts);
	time64_t date(int day, int month, int year);

	time64_t weekday_before(int weekdayBefore, time64_t date);
	time64_t sunday_before(time64_t date);
	time64_t monday_before(time64_t date);
	time64_t tuesday_before(time64_t date);
	time64_t wednesday_before(time64_t date);
	time64_t thursday_before(time64_t date);
	time64_t friday_before(time64_t date);
	time64_t saturday_before(time64_t date);

	time64_t weekday_after(int weekdayAfter, time64_t date);
	time64_t sunday_after(time64_t date);
	time64_t monday_after(time64_t date);
	time64_t tuesday_after(time64_t date);
	time64_t wednesday_after(time64_t date);
	time64_t thursday_after(time64_t date);
	time64_t friday_after(time64_t date);
	time64_t saturday_after(time64_t date);

	int dayofweek(time64_t date);
	bool sunday(time64_t date);
	bool monday(time64_t date);
	bool tuesday(time64_t date);
	bool wednesday(time64_t date);
	bool thursday(time64_t date);
	bool friday(time64_t date);
	bool saturday(time64_t date);

	int date_difference(time64_t date1, time64_t date2);
	int year(time64_t date);
	int dayofmonth(time64_t date);
	bool issameday(time64_t date1, time64_t date2);

	time64_t start_date(int year);
	time64_t end_date(int year);
	time64_t first_advent_sunday(int year);
	time64_t nativity(int year);
	time64_t holy_family(int year);
	time64_t mother_of_god(int year);
	time64_t epiphany(int year);
	time64_t baptism_of_lord(int year);
	time64_t ash_wednesday(int year);
	time64_t easter_sunday(int year);
	time64_t palm_sunday(int year);
	time64_t good_friday(int year);
	time64_t holy_saturday(int year);
	time64_t ascension(int year);
	time64_t pentecost(int year);
	time64_t holy_trinity(int year);
	time64_t corpus_christi(int year);
	time64_t sacred_heart(int year);
	time64_t immaculate_heart(int year);
	time64_t christ_king(int year);
	time64_t octave_of(time64_t date);

	int liturgical_year(time64_t date);
	int for_day(time64_t date);
	Season season(time64_t date);
	time64_t season_beginning(Season s, time64_t date);
	int season_week(Season seasonn, time64_t date);

	static const Season SEASONS_SUNDAY_PRIMARY[3];
	char* sunday_temporale(time64_t date);
	char* ferial_temporale(time64_t date);
	char* liturgical_day(time64_t date);

	char* ordinalize(int number);
	bool includes(const char* s, const char* const strarray[]);
	void temporaletests();
	int get_monthdays(int mon, int year);
	bool yisleap(int year);

	void easter_tests();
	void epiphany_tests(void);
	void print_date(time64_t t);
};

#endif
