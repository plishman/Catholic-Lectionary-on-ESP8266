#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>
#include "I2CSerialPort.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <pins_arduino.h>
#include <I18n.h>

typedef struct {
  float timezone_offset;
  int lectionary_config_number;
} config_t __attribute__ ((packed));

class Config {
public:
	I18n* _I18n;
	//WiFiServer* p_server;
	WiFiServer server;
	
	Config();
	~Config();
	bool StartServer( I18n* i );
	void StopServer( void );
	bool ServeClient( bool* bSettingsUpdated );
	String getQueryStringParam(String param, String querystring);
	bool sendHttpFile(WiFiClient* client, String filename);
	bool SaveConfig(String tz, String lect_num);
	void GetConfig(config_t* c);
	bool IsNumeric(String str);
	void storeStruct(void *data_source, size_t size);
	void loadStruct(void *data_dest, size_t size);
};
#endif