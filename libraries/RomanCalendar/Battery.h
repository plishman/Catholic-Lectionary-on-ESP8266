#ifndef _BATTERY_H
#define _BATTERY_H

#include "arduino.h"
#include "I2CSerialPort.h"

#define DIODE_DROP 0.5
#define BATT_MARGIN 0.25
#define MIN_BATT_VOLTAGE 3.2 // 3.45 shutdown voltage (connect charger message to be displayed)
#define NOMINAL_BATT_VOLTAGE 3.78 
#define MIN_CHARGING_VOLTAGE 5-DIODE_DROP-BATT_MARGIN

class Battery {
public:
	Battery();
	bool power_connected( void );
	float battery_voltage( void );
	bool recharge_level_reached( void );
};


#endif