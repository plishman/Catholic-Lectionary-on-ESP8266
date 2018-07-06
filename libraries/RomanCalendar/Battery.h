#ifndef _BATTERY_H
#define _BATTERY_H

#include "arduino.h"
#include "I2CSerialPort.h"

#define ADC_RESISTOR 470.0
//#define ADC_RESISTOR 200.0 + 220.0

#define DIODE_DROP 0.55 //0.5 //0.105 //0.435
#define MIN_BATT_VOLTAGE 3.4 // 3.45 shutdown voltage (connect charger message to be displayed)
#define BATT_MARGIN 0.7
#define MIN_CHARGING_VOLTAGE 5 - BATT_MARGIN

class Battery {
public:
	Battery();
	bool power_connected( void );
	float battery_voltage( void );
	bool recharge_level_reached( void );
};


#endif