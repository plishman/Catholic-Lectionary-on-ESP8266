#include "Battery.h"

Battery::Battery() {
}

bool Battery::power_connected( void ) {
	Serial.println("battery voltage is " + String(battery_voltage()));
	Serial.println("min charging voltage is " + String(MIN_CHARGING_VOLTAGE));
	return (battery_voltage() > MIN_CHARGING_VOLTAGE);
}

float Battery::battery_voltage( void ) {
	int sensorValue = analogRead(A0);		
	return sensorValue * (3.3 / 1024.00) * 2;
}

bool Battery::recharge_level_reached( void ) {
	return (battery_voltage() < MIN_BATT_VOLTAGE);
}

