#ifndef DEBUGPORT_H
#define DEBUGPORT_H

#include "Arduino.h"
#include "I2CSerialPort.h"
#include "FileDebugPort.h"

class DebugPort : public Print {
public:
	bool _b_enable = false; // controls whether debug is enabled. Defaults to false.
	virtual size_t write(uint8_t);
	virtual size_t write(char *str);
	virtual size_t write(uint8_t *buffer, size_t size);
	virtual int available(void);
	virtual byte read(void);
	
	DebugPort();
	~DebugPort();
	void begin(String filename, uint8_t sda, uint8_t scl, uint8_t address);
	void end();
};

extern DebugPort Debug_Prt;
		
	
#endif