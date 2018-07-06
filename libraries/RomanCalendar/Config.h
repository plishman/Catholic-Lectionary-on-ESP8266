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
// https://stackoverflow.com/questions/30223983/c-add-up-all-bytes-in-a-structure
template<typename T> int CountBytes(const T & t)
{
   int count = 0;
   const unsigned char * p = reinterpret_cast<const unsigned char *>(&t);
   for (int i=0; i<sizeof(t); i++) count += p[i];
   return count;
}
//////

// non-volatile (eeprom) configuration parameters
typedef struct {
  float timezone_offset;
  int lectionary_config_number;
  int century;
  bool debug_on;
  int checksum;
} config_t __attribute__ ((packed));
//

//rtc memory data - specifies what to do on waking
//enum DisplayCardShown{dc_normal, dc_battery_recharge_image, dc_connect_power_image, dc_wps_connect_image, dc_clock_not_set_image, dc_sd_card_not_inserted_image};

typedef struct {
  uint8_t wake_hour_counter; // set number of hours to the next reading. Decreased by 1 each wake
//  DisplayCardShown dcs; // value shows if any of the error cards are displayed, so that, if the condition has not changed, it need not be redisplayed
} lectionary_data_t;

typedef struct {
  uint32_t crc32;
  lectionary_data_t data;
} rtcData_t;
//

class Config {
public:
	I18n* _I18n;
	WiFiServer server;
	
	Config();
	~Config();
	bool StartServer( I18n* i );
	void StopServer( void );
	bool ServeClient( bool* bSettingsUpdated );
	String getQueryStringParam(String param, String querystring, String default_value);
	bool sendHttpFile(WiFiClient* client, String filename);
	bool SaveConfig(String tz, String lect_num, String debug_mode);
	void SaveConfig(config_t* c);
	bool GetConfig(config_t* c);
	bool EEPROMChecksumValid();
	bool IsNumeric(String str);
	void storeStruct(void *data_source, size_t size);
	void loadStruct(void *data_dest, size_t size);
		
	bool getDS3231DateTime(time64_t* t);
	bool setDS3231DateTime(time64_t t);
	bool getLocalDS3231DateTime(time64_t* t);
	uint8_t dec2bcd(uint8_t num);
	uint8_t bcd2dec(uint8_t num);
	bool testArg(String arg, uint32_t min, uint32_t max, uint32_t* outval);
	
	//based on Arduino example code at https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino
	static bool readRtcMemoryData(rtcData_t& rtcData);
	static void writeRtcMemoryData(rtcData_t& rtcData);
	static uint32_t calculateCRC32(const uint8_t *data, size_t length);
	static void printMemory(rtcData_t& rtcData);
	
};

//extern Config Conf;
#endif