#ifndef _TIMESERVER_H
#define _TIMESERVER_H

#include <TimeLib.h>
#include <TinyGPS++.h>
#include "arduino.h"
#include "I2CSerialPort.h"

class TimeServerGPS {
public:
	TinyGPSPlus gps;

	time64_t local_datetime( void );
	void gps_sleep( void );
	void gps_wake( void );
};

#endif