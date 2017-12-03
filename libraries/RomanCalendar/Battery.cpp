#include "Battery.h"

Battery::Battery() {
}

bool Battery::power_connected( void ) {
	return (battery_voltage() > MIN_CHARGING_VOLTAGE);
}

float Battery::battery_voltage( void ) {
	int sensorValue = analogRead(A0);		
	//I2CSerial.println("battery_voltage = " + String (sensorValue * (3.3 / 1024.00) * 2));
	return sensorValue * (3.3 / 1024.00) * 2;
}

bool Battery::recharge_level_reached( void ) {
	return (battery_voltage() < MIN_BATT_VOLTAGE);
}

