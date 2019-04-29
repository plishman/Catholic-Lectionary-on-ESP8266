#ifndef _FILEDEBUGPORT_H
#define _FILEDEBUGPORT_H

#include "RCGlobals.h"
#include "Arduino.h"
#include "SD.h"

class FileDebugPort : public Print {
public:
	bool _b_enable = false; // controls whether backchannel debug is enabled. Defaults to false.
	virtual size_t write(uint8_t);
	virtual size_t write(char *str);
	virtual size_t write(uint8_t *buffer, size_t size);
	virtual int available(void);
	virtual byte read(void);
	
	String _filename = "";
	bool _bFileAvailable = false;
	File _debugFile;
	
	bool open_debug_file();
	void close_debug_file();
	
	FileDebugPort();
	~FileDebugPort();
	void begin(String filename);
	void end();
};

extern FileDebugPort FileDebug;

#endif
