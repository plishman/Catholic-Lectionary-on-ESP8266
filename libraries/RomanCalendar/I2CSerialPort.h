#ifndef _I2C_SERIAL_H
#define _I2C_SERIAL_H
#include "Arduino.h"
//#include <Wire.h>
#include <brzo_i2c.h>

class I2CSerialPort : public Print {
public:
	virtual size_t write(uint8_t);
	virtual size_t write(char *str);
	virtual size_t write(uint8_t *buffer, size_t size);
	virtual int available(void);
	virtual byte read(void);
	
	uint8_t _sp_address = 8;
	uint8_t _sda = 1;
	uint8_t _scl = 3;
	uint8_t SCL_speed = 200;
	uint32_t SCL_STRETCH_TIMEOUT = 50000;
	uint8_t MAX_RETRIES = 3;
	uint8_t buffer[5];
	uint8_t error = 0;
	
	uint8_t ICACHE_RAM_ATTR soft_reset();
	
	I2CSerialPort();
	~I2CSerialPort();
	void begin(uint8_t sda, uint8_t scl, uint8_t address);
};

//#define PRINT(S) I2C_print(S)
//#define PRINTLN(S) I2C_println(S)

extern I2CSerialPort I2CSerial;

#endif
