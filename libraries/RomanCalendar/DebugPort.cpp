#include "DebugPort.h"
#include "RCConfig.h"

FileDebugPort FileDebug;
I2CSerialPort I2CSerial;

DebugPort::DebugPort() {
//	config_t c = {0};
//	
////	Config conf;	
//	
//	if (Config::GetConfig(c)) {
//		_b_enable = c.data.debug_on;
//	} 
//	else {
//		_b_enable = false; //turn off - was true Need to manage debug output on a module basis now! // turn on debug output if the EEPROM settings are corrupt or invalid.
//	}
	
	_b_enable = true; // debugging PLL-27-04-2019
}

DebugPort::~DebugPort() {
}

void DebugPort::begin(String filename, uint8_t sda, uint8_t scl, uint8_t address) {
	I2CSerial.begin(sda, scl, address);
	FileDebug.begin(filename);
}

void DebugPort::end() {
	//I2CSerial.end();
	FileDebug.end();
}

size_t DebugPort::write(uint8_t character) { /*blahblah is the name of your class*/
	if (!_b_enable) {
		return 0;
	}

	I2CSerial.write(character);
	FileDebug.write(character);
	
	return 1;
}

size_t DebugPort::write(char *str) { /*blahblah is the name of your class*/
	if (!_b_enable) {
		return 0;
	}

	I2CSerial.write(str);
	FileDebug.write(str);

	return strlen(str);
}

size_t DebugPort::write(uint8_t *buffer, size_t size) { /*blahblah is the name of your class*/
	if (!_b_enable) {
		return 0;
	}

	I2CSerial.write(buffer, size);
	FileDebug.write(buffer, size);
	
	return size;
}

int DebugPort::available() {
	return 0;	
}

byte DebugPort::read() {
	return 0;
}
