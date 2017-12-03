#pragma once
#ifndef _TRANSFERS_H
#define _TRANSFERS_H

#ifndef _WIN32
	#include "Arduino.h"
	#include "I2CSerialPort.h"
	#include "../Time/TimeLib.h"
#else
	#include <time.h>
#endif

#include "LinkedList.h"
#include "Enums.h"
#include "I18n.h"
#include "Temporale.h"
#include "Sanctorale.h"

class Transfer {
public:
	time_t from;
	time_t to;
};

class Transfers
{
public:
	int _year = 0;
	I18n* _I18n;
	Temporale* _tc;
	Sanctorale* _sc;
	bool _transfer_to_sunday = false;
	LinkedList<Transfer*> transfersList = LinkedList<Transfer*>();
	Transfers(bool transfer_to_sunday, I18n* i);
	bool do_transfers(time_t date);
	bool get(time_t date);
	void transfers_lent(int year, Temporale* tc);
	bool transferred_from(time_t date, time_t* to);
	bool transferred_to(time_t date, time_t* from);
	bool do_transferred_from(time_t date, time_t* to);
	bool do_transferred_to(time_t date, time_t* from);
	~Transfers();
	bool valid_destination(time_t day, Temporale * tc, Sanctorale * sc);
	time_t next_day(time_t day);
	int get_monthdays(int mon, int year);
	bool yisleap(int year);
};
#endif