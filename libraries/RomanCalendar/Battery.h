#ifndef _BATTERY_H
#define _BATTERY_H

#include "I18n.h"
#include <SD.h>
#include "arduino.h"
#include "I2CSerialPort.h"

#define DIODE_DROP 0.7
#define MARGIN 0.5
#define MIN_BATT_VOLTAGE 3.1 - DIODE_DROP - MARGIN // shutdown voltage (connect charger message to be displayed)
#define NOMINAL_BATT_VOLTAGE 3.78 - DIODE_DROP - MARGIN
#define MIN_CHARGING_VOLTAGE 5 - DIODE_DROP - MARGIN

class Battery {
public:
	Battery();
	bool power_connected( void );
	float battery_voltage( void );
	bool recharge_level_reached( void );
};


#endif