#ifndef _FS_H
#define _FS_H

#include "Arduino.h"
#define FS_NO_GLOBALS
#include <FS.h>
#include <SD.h>

//simple non-polymorphic attempt to unify SPIFFS and SD card filesystem calls
class Filehandle {
public:
	File sd_file;
	fs::File spiffs_file;
	
	int read();
	
};

class Disk {
public:
	FileHandle fopen(String filename, String mode); // mode can be "r", "w", "a", "r+", "w+", "a+". Function will turn "r", "r+" into FILE_READ on SD, and "w", "w+" and "a+" into FILE_WRITE on SD
	bool fclose(Filehandle f);
	
};

#endif
