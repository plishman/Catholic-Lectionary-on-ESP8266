#pragma once

#ifndef _RCCONFIG_H
#define _RCCONFIG_H

#define _DUMP_CONFIG 1

#ifdef _WIN32
#include "DebugPort.h"
#endif

//#ifdef _TEST_CONFIG	// defined in test harness version of DebugPort.h

#ifdef _WIN32
#define BEGIN_EPOCH 1970

#include "WString.h"
#include "WCharacter.h"
#include "PROGMEM.h"
#include "Sd.h"
#include "TimeLib.h"
#include <stdio.h>
#include <stdint.h>
#else
#include "RCGlobals.h"

#include <Arduino.h>
#include <TimeLib.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <EEPROM.h>
#include <pins_arduino.h>
#endif 

#define EPD_CONTRAST_LEVELS 7

// flags are: A1M1 (seconds), A1M2 (minutes), A1M3 (hour), 
// A1M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0)
// 5 bit field: DY/DT, A1M4, A1M3, A1M2, A1M1 - only certain values are permissible

#define A1_ONCE_PER_SECOND 				0x0F
#define A1_MATCH_SECONDS				0x0E
#define A1_MATCH_MINUTES_SECONDS		0x0C
#define A1_MATCH_HOURS_MINUTES_SECONDS 	0x80
#define A1_MATCH_DATE_AND_TIME			0x00
#define A1_MATCH_DAY_AND_TIME			0x10

#define A2_ONCE_PER_MINUTE 				0x07
#define A2_MATCH_MINUTES				0x06
#define A2_MATCH_HOURS_MINUTES		 	0x04
#define A2_MATCH_DATE_AND_TIME_MINS		0x00
#define A2_MATCH_DAY_AND_TIME_MINS		0x80

// i2c slave address of the DS3231 chip
#define DS3231_I2C_ADDR 			0x68

// timekeeping registers
#define DS3231_TIME_CAL_ADDR        0x00
#define DS3231_ALARM1_ADDR          0x07
#define DS3231_ALARM2_ADDR          0x0B
#define DS3231_CONTROL_ADDR         0x0E
#define DS3231_STATUS_ADDR          0x0F
#define DS3231_AGING_OFFSET_ADDR    0x10
#define DS3231_TEMPERATURE_ADDR     0x11

// control register bits
#define DS3231_A1IE     0x1
#define DS3231_A2IE     0x2
#define DS3231_INTCN    0x4

// status register bits
#define DS3231_A1F      0x1
#define DS3231_A2F      0x2
#define DS3231_OSF      0x80

#define WAKE_FLAGS_NONE 0x0
#define WAKE_FLAGS_WPS_FAILED_LAST_ATTEMPT_BUT_STILL_CHARGING 0xff
#define WAKE_FLAGS_STILL_CHARGING_DONT_REDISPLAY_WIFI_CONNECTED_IMAGE 0xf0
enum wake_reasons {WAKE_UNKNOWN, WAKE_ALARM_1, WAKE_ALARM_2, WAKE_USB_5V, WAKE_DEEPSLEEP};

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
  uint32_t lectionary_config_number;
  uint32_t century;
  uint8_t debug_flags;
  float dst_offset;
  uint32_t dst_start_month;
  uint32_t dst_start_day;
  uint32_t dst_start_hour;
  uint32_t dst_end_month;
  uint32_t dst_end_day;
  uint32_t dst_end_hour;
  uint16_t epd_contrast;
} config_data_t;

#define DEBUG_FLAGS_I2CPORT  1
#define DEBUG_FLAGS_FILEPORT 2

#ifndef _WIN32
typedef struct {
  config_data_t data;
  uint32_t crc32;
} config_t __attribute__ ((packed));
//
#else
typedef struct {
#pragma pack(push, 1)
	config_data_t data;
	uint32_t crc32;
#pragma pack(pop)
} config_t;
#endif

//rtc memory data - specifies what to do on waking
//enum DisplayCardShown{dc_normal, dc_battery_recharge_image, dc_connect_power_image, dc_wps_connect_image, dc_clock_not_set_image, dc_sd_card_not_inserted_image};

typedef struct {
  uint8_t wake_hour_counter; // set number of hours to the next reading. Decreased by 1 each wake
  uint8_t wake_flags; // this so far is only used to detect when the lectionary woke while on charge (from deepSleep) without USB5V power being interrupted - and
					  // only after a WPS config failure. This means that during charging with no network available (which it would otherwise search for) the lectionary
					  // will still display a reading 10 minutes after the WPS config failure.
//  DisplayCardShown dcs; // value shows if any of the error cards are displayed, so that, if the condition has not changed, it need not be redisplayed
} lectionary_data_t;

typedef struct {
  uint32_t crc32;
  lectionary_data_t data;
} rtcData_t;
//

#ifndef _WIN32
extern ESP8266WebServer server;
	
bool loadFromSdCard(String path);
#ifdef CORE_v3_EXPERIMENTAL
//bool bUseHTTPServerWorkarounds = false;
size_t streamFile(File& file, String& contentType);	// workaround for server.streamFile on v3 core, which doesn't appear to work at the moment (even in exacmple sketch)
#endif

void handleNotFound();
void handleSettingsJson();
void handleFileUpload();
void handleConfigFileUpload();
void handleSetConf();
String getQueryStringParam(String param_name, String default_value);
bool testArg(String arg, uint32_t min, uint32_t max, uint32_t* outval);
bool copyfile(String fromFile, String toFile);
#endif

class Config {
public:
#ifdef _WIN32
	static config_t flashData;
	static rtcData_t rtcData;
#endif

	static String _lang;

//	I18n* _I18n;
//	ESP8266WebServer server;
	static bool bSettingsUpdated;
	static bool bComplete;
	
	static bool bHaveNewConfigCsv;
		
	//Config();
	//~Config();
//	bool loadFromSdCard(String path);
//	void handleNotFound();
//	void handleSettingsJson();
//	void handleSetConf();
//	String getQueryStringParam(String param_name, String default_value);
	static bool StartServer(String lang);
	static void StopServer( void );
	//bool ServeClient( bool* bSettingsUpdated );
	//String getQueryStringParam(String param, String querystring, String default_value);
	//bool sendHttpFile(WiFiClient* client, String filename);
	
	static bool SaveConfig(String tz, String lect_num, String debug_mode, String epdcontrast);	
	static bool SaveConfig(String tz, String lect_num, String debug_mode, String epdcontrast,
					uint32_t dstStartMon, uint32_t dstStartDay, uint32_t dstStartHour, 
					uint32_t dstEndMon, uint32_t dstEndDay, uint32_t dstEndHour, 
					String dstoffset);
	
	static void dump_config(config_t& c);
	
	static bool DstIsValid();
	static bool DstIsValid(config_t& c);
	static void InvalidateEEPROM();
	static void SaveConfig(config_t& c);
	static bool GetConfig(config_t& c);
	static uint16_t GetEPDContrast();
	static bool EEPROMChecksumValid();
	static bool IsNumeric(String str);
	static void storeStruct(void *data_source, size_t size);
	static void loadStruct(void *data_dest, size_t size);

	static bool isDSTfromUTC(time64_t t);
	static bool isDST();
	static bool isDST(time64_t t); // nb. var t is a Local Date/Time value, not UTC (which does not have timezone or DST applied)
	static int dstOffset();
	static time64_t dstStart(time64_t curr_t);
	static time64_t dstEnd(time64_t curr_t);
	static int tzOffset();
	static bool getUniversalDatetimeFromLocalDatetime(time64_t* t);
	static bool getLocalDateTime(time64_t* t);
	//static bool getLocalDateTime(time64_t* t, bool* isdst);
	static bool getLocalDateTime(time64_t* t, bool* isdst, bool b_use_current_time = true);
	static bool ClockWasReset();
	static bool getDateTime(time64_t* t);
	static bool getDateTime(time64_t* t, bool& clockwasreset);
	static bool setDateTime(time64_t t);

	static bool setAlarmLocalTime(time64_t t, uint8_t alarm_number, uint8_t flags, bool enable_alarm);
	static bool setAlarm(time64_t t, uint8_t alarm_number, uint8_t flags, bool enable_alarm);
	
#ifndef _WIN32
	static bool DS3231_set_addr(const uint8_t addr, const uint8_t val);
	static uint8_t DS3231_get_addr(const uint8_t addr, bool& ok);
	
	static bool DS3231_set_creg(const uint8_t val);
	static bool DS3231_set_sreg(const uint8_t val);
	static uint8_t DS3231_get_sreg(bool& ok);
	static uint8_t DS3231_get_creg(bool& ok);
#endif

	// Alarm 1
	static bool DS3231_clear_a1f(void);
#ifndef _WIN32
	static uint8_t DS3231_triggered_a1(bool& ok);
	static bool DS3231_arm_a1(bool enable);
	//
#endif

	// Alarm 2
	static bool DS3231_clear_a2f(void);
#ifndef _WIN32
	static uint8_t DS3231_triggered_a2(bool& ok);
	static bool DS3231_arm_a2(bool enable);
	//

	static bool Clock_reset();
#endif

	static bool ClockStopped(bool& ok);

#ifndef _WIN32
	static bool ClearClockStoppedFlag();
	static bool ClearCenturyFlag();
#endif

	static uint8_t dec2bcd(uint8_t num);
	static uint8_t bcd2dec(uint8_t num);
//	bool testArg(String arg, uint32_t min, uint32_t max, uint32_t* outval);
	
	//based on Arduino example code at https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino
	static bool readRtcMemoryData(rtcData_t& rtcData);
	static void writeRtcMemoryData(rtcData_t& rtcData);
	static uint32_t calculateCRC32(const uint8_t *data, size_t length);
	static void printMemory(rtcData_t& rtcData);

#ifndef _WIN32
	static wake_reasons Wake_Reason();
#endif

	static bool PowerOff(time64_t wake_datetime);
	static bool SetPowerOn();

	static void print_time(time64_t t, bool b_newline = true);
	static void print_time(String caption, time64_t t, bool b_newline = true);
	static void print_time_utc(String caption, time64_t t, bool b_convert_to_local_time, bool b_newline = true);
};

//extern Config Conf;
#endif
//#endif // #ifdef _TEST_CONFIG