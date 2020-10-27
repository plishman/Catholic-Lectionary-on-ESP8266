#include "FileDebugPort.h"
#include "RCConfig.h"

FileDebugPort::FileDebugPort() {
	_b_enable = false; // debugging PLL-27-04-2019
}

FileDebugPort::~FileDebugPort() {
	close_debug_file();
}

void FileDebugPort::begin(String filename) {
	_b_enable = false;
	
	_filename = filename;
	
	if (open_debug_file()) {	// will set _bFileAvailable = true if the file could be opened;
		close_debug_file();
		
		config_t c = {0};
	
		if (Config::GetConfig(c)) {
			_b_enable = (c.data.debug_flags & DEBUG_FLAGS_FILEPORT) != 0;
		} 
		else {
			_b_enable = false; //turn off - was true Need to manage debug output on a module basis now! // turn on debug output if the EEPROM settings are corrupt or invalid.
		}
		
		return;
	}
}

void FileDebugPort::end() {
	close_debug_file();
}

bool FileDebugPort::open_debug_file() {
    if (_debugFile) return true;
	
	// Try and append
    _debugFile = SD.open(_filename, sdfat::O_RDWR | sdfat::O_APPEND);
    if (!_debugFile) {
      // It failed, so try and make a new file.
      _debugFile = SD.open(_filename, sdfat::O_RDWR | sdfat::O_CREAT);
	  
	  if (!_debugFile) {
        // It failed too, so give up.
        //Serial.println("Failed to open file.txt");
		_bFileAvailable = false;
		return false;
      }
    }
	
	_bFileAvailable = true;
	return true;
}

void FileDebugPort::close_debug_file() {
	if (_debugFile) _debugFile.close();
}

size_t FileDebugPort::write(uint8_t character) { /*blahblah is the name of your class*/
	if (!_b_enable) {
		return 0;
	}

	if (open_debug_file()) {
		_debugFile.write(character);
		//close_debug_file();
		return 1;
	}
	
	return 0;
}

size_t FileDebugPort::write(char *str) { /*blahblah is the name of your class*/
	if (!_b_enable) {
		return 0;
	}

	if (open_debug_file()) {
		_debugFile.write(str, strlen(str));
		//close_debug_file();
		return strlen(str);
	}
	
	return 0;
}

size_t FileDebugPort::write(uint8_t *buffer, size_t size) { /*blahblah is the name of your class*/
	if (!_b_enable) {
		return 0;
	}

	if (open_debug_file()) {
		_debugFile.write(buffer, size);
		//close_debug_file();
		return size;
	}
	
	return 0;
}

int FileDebugPort::available() {
	if (_bFileAvailable) return 0;	
	return 1;
}

byte FileDebugPort::read() {
	return 0;
}
