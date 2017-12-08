#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>
#include <TimeLib.h>
#include "I2CSerialPort.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <pins_arduino.h>
#include <I18n.h>

#define DS3231_I2C_ADDRESS 104

typedef struct {
  float timezone_offset;
  int lectionary_config_number;
  int century;
} config_t __attribute__ ((packed));

class Config {
public:
	I18n* _I18n;
	WiFiServer server;
	
	Config();
	~Config();
	bool StartServer( I18n* i );
	void StopServer( void );
	bool ServeClient( bool* bSettingsUpdated );
	String getQueryStringParam(String param, String querystring);
	bool sendHttpFile(WiFiClient* client, String filename);
	bool SaveConfig(String tz, String lect_num);
	void SaveConfig(config_t* c);
	void GetConfig(config_t* c);
	bool IsNumeric(String str);
	void storeStruct(void *data_source, size_t size);
	void loadStruct(void *data_dest, size_t size);
		
	bool getDS3231DateTime(time64_t* t);
	bool setDS3231DateTime(time64_t t);
	uint8_t dec2bcd(uint8_t num);
	uint8_t bcd2dec(uint8_t num);
	bool testArg(String arg, int min, int max, int* outval);
};

//extern Config Conf;
#endif