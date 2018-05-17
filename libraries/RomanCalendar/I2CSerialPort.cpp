#include "I2CSerialPort.h"
#include "Config.h"

I2CSerialPort::I2CSerialPort() {
	config_t c;
	
	Config conf;
	
	
	if (conf.GetConfig(&c)) {
		_b_enable = true;//c.debug_on;
	} 
	else {
		_b_enable = true; // turn on debug output if the EEPROM settings are corrupt or invalid.
	}
}

I2CSerialPort::~I2CSerialPort() {
}

void I2CSerialPort::begin(uint8_t sda, uint8_t scl, uint8_t address) {
	_sda = sda;
	_scl = scl;
	_sp_address = address;

	//Wire.begin(_sda,_scl); // join i2c bus (address optional for master)
	brzo_i2c_setup(_sda, _scl, SCL_STRETCH_TIMEOUT);
	
	uint8_t i = 1;
	do {
		error = soft_reset();
		if (error == 0) {
			//Serial.println("Soft reset OK ");
		}
		else {
			//Serial.print("Soft reset fail. Brzo error : ");
			//Serial.println(error);
		}
		i++;
	} while ((error != 0) && (i <= MAX_RETRIES));
}

size_t I2CSerialPort::write(uint8_t character) { /*blahblah is the name of your class*/
	if (!_b_enable) return 0;
	
	uint8_t	bcode = 0;
	uint8_t buffer[1];
	
	buffer[0] = character;
	
	brzo_i2c_start_transaction(_sp_address, SCL_speed);
		brzo_i2c_write(buffer, 1, false);
		delay(1);
	bcode = brzo_i2c_end_transaction();

	if (bcode != 0) {
		soft_reset();
	}

//  Wire.beginTransmission(_sp_address); // transmit to device #8
//  Wire.write(character);     // sends string
//  Wire.endTransmission();    // stop transmitting  
/*Code to display letter when given the ASCII code for it*/
}

size_t I2CSerialPort::write(char *str) { /*blahblah is the name of your class*/
	if (!_b_enable) return 0;

	uint8_t	bcode = 0;
	//buffer[0] = 0xE3;
	brzo_i2c_start_transaction(_sp_address, SCL_speed);
		brzo_i2c_write(buffer, strlen(str), true);
	bcode = brzo_i2c_end_transaction();

	if (bcode != 0) {
		soft_reset();
	}

//  Wire.beginTransmission(_sp_address); // transmit to device #8
//  while(*str) {
//	Wire.write(*str);     // sends string
//	str++;
//  }
//  Wire.endTransmission();    // stop transmitting  
/*Code to display string when given a pointer to the beginning -- remember, the last character will be null, so you can use a while(*str). You can increment str (str++) to get the next letter*/
}

size_t I2CSerialPort::write(uint8_t *buffer, size_t size) { /*blahblah is the name of your class*/
	if (!_b_enable) return 0;

	uint8_t	bcode = 0;
	//buffer[0] = 0xE3;
	brzo_i2c_start_transaction(_sp_address, SCL_speed);
		brzo_i2c_write(buffer, size, true);
	bcode = brzo_i2c_end_transaction();

	if (bcode != 0) {
		soft_reset();
	}

//  Wire.beginTransmission(_sp_address); // transmit to device #8
//  for(int i=0; i<size; i++) {
//	Wire.write(buffer[i]);     // sends string
//  }
//  Wire.endTransmission();    // stop transmitting  
/*Code to display array of chars when given a pointer to the beginning of the array and a size -- this will not end with the null character*/
}

int I2CSerialPort::available() {
	return 0; //Wire.available();
}

byte I2CSerialPort::read() {
	return 0; //Wire.read();
}

uint8_t ICACHE_RAM_ATTR I2CSerialPort::soft_reset() {
	uint8_t	bcode = 0;
	// Soft Reset command
	buffer[0] = 0xFE;
	brzo_i2c_start_transaction(_sp_address, SCL_speed);
		brzo_i2c_write(buffer, 1, false);
	bcode = brzo_i2c_end_transaction();
	return bcode;
}
