#ifndef _TIMESERVER_H
#define _TIMESERVER_H

#include <TimeLib.h>
#include <TinyGPS.h>
#include "arduino.h"

class TimeServer {
public:
	TinyGPS gps;

	time_t local_datetime( void );
	void gps_sleep( void );
	void gps_wake( void );
};

#endif