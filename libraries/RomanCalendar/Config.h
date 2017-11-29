#ifndef _CONFIG_H
#define _CONFIG_H

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <pins_arduino.h>
#include <I18n.h>

typedef struct {
  float timezone_offset;
  int lectionary_config_number;
} config_t __attribute__ ((packed));

class Config {
	I18n* p_i18n;
	WiFiServer* p_server;
	
	Config(I18n* i);
	~Config();
	bool ServeClient(void);
	String getQueryStringParam(String param, String querystring);
	bool sendHttpFile(WiFiClient* client, String filename);
	bool SaveConfig(String tz, String lect_num);
	void GetConfig(config_t* c);
	bool IsNumeric(String str);
	void storeStruct(void *data_source, size_t size);
	void loadStruct(void *data_dest, size_t size);
};
#endif