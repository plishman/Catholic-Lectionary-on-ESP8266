#include "Battery.h"

Battery::Battery() {
}

bool Battery::power_connected( void ) {
	//return true; 														// debugging
	return (battery_voltage() > MIN_CHARGING_VOLTAGE);
}

float Battery::battery_voltage( void ) {
	float sensorValue = (float)analogRead(A0) / 1023.0;

	float adc_maxvoltage = (ADC_RESISTOR + 100.0) / 100.0; //max voltage (adc value 1023, which will be scaled by potential divider to 1 volt) = 4.89V for 169.0k adc resistor, 5.4V for 220k adc resistor
	float v = (adc_maxvoltage * sensorValue) + DIODE_DROP;

	//I2CSerial.println("battery_voltage = " + String(v));
	//I2CSerial.println("A0 sensor value = " + String(sensorValue));
	return v;
}

bool Battery::recharge_level_reached( void ) {
	return (battery_voltage() < MIN_BATT_VOLTAGE);
}

