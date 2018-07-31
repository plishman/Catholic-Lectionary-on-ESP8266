#include "RCConfig.h"
#include "I2CSerialPort.h"
#include "SD.h"

extern "C" {
#include "user_interface.h"
}

//Config::Config() {}
bool Config::bSettingsUpdated;

bool loadFromSdCard(String path) {
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if (!(path == "/config.csv" || path == "config.csv")) {  
	  if (path.startsWith("/")) {
		path = "/html" + path;  
	  }
	  else {
		path = "/html/" + path;	  
	  }
  }
  
  I2CSerial.print(F("path="));
  I2CSerial.println(path);
  
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  File dataFile = SD.open(path.c_str());
  if(dataFile.isDirectory()){
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile)
    return false;

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    I2CSerial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void handleNotFound() {
	if(loadFromSdCard(server.uri())) return;
	String message = "SDCARD Not Detected or File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET)?"GET":"POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i=0; i<server.args(); i++){
		message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
	I2CSerial.print(message);
}


void handleSettingsJson() {
    config_t c = {0};
    Config::GetConfig(c);
	
    I2CSerial.print(F("timezone = "));
	I2CSerial.println(String(c.data.timezone_offset));
    I2CSerial.print(F("lectionary = "));
	I2CSerial.println(String(c.data.lectionary_config_number));
	
    String line = "{\"tz_offset\":\"" + String(c.data.timezone_offset) + "\", \"lectionary_config_number\":\"" + String(c.data.lectionary_config_number) + "\"}"; // output in JSON format
    server.send(200, "application/json", line);
    I2CSerial.println(line);
}


void handleSetConf() {
    String tz = getQueryStringParam("timezone", ""); // "timezone", "0"
    String lect = getQueryStringParam("lectionary", ""); // "lectionary", "0"	

	String debug_mode = getQueryStringParam("debug", "");
	
    I2CSerial.print(F("timezone = "));
	I2CSerial.println(String(tz));
    I2CSerial.print(F("lectionary = "));
	I2CSerial.println(String(lect));
    
	if (debug_mode != "") {
		I2CSerial.print(F("debug = "));
		I2CSerial.println(String(debug_mode));
	}
	
//    bool bresult = SaveConfig(tz, lect, debug_mode);
//
//    I2CSerial.println("SaveConfig returned " + String(bresult?"true":"false"));
		
	uint32_t hh, mm, ss, day, mon, year;

	if (testArg(getQueryStringParam("hh",   ""), 0, 23, &hh) &&
		testArg(getQueryStringParam("mm",   ""), 0, 59, &mm) &&
		testArg(getQueryStringParam("ss",   ""), 0, 59, &ss) &&
		testArg(getQueryStringParam("day",  ""), 1, 31, &day) &&
		testArg(getQueryStringParam("mon",  ""), 1, 12, &mon) &&
		testArg(getQueryStringParam("year", ""), 1970, 0xFFFFFFFF, &year)) { // was max range was 65535
			
		tmElements_t tm;
		
		tm.Hour = (uint8_t)hh;
		tm.Minute = (uint8_t)mm;
		tm.Second = (uint8_t)ss;
		tm.Day = (uint8_t)day;
		tm.Month = (uint8_t)mon;
		tm.Year = year - 1970;
		
		time64_t t = makeTime(tm);
		Config::setDateTime(t);
	}
	
	uint32_t dststarthour = 0;
	uint32_t dststartday = 0;
	uint32_t dststartmonth = 0;
	
	uint32_t dstendhour = 0; 
	uint32_t dstendday = 0; 
	uint32_t dstendmonth = 0;

    String dstoffset = getQueryStringParam("dstoffset", "0");

	bool bresult = false;
	
	if (testArg(getQueryStringParam("dststarthour",  ""), 0, 23, &dststarthour) &&
		testArg(getQueryStringParam("dststartday",   ""), 1, 31, &dststartday) &&
		testArg(getQueryStringParam("dststartmonth", ""), 1, 12, &dststartmonth) &&
		testArg(getQueryStringParam("dstendhour",    ""), 0, 23, &dstendhour) &&
		testArg(getQueryStringParam("dstendday",     ""), 1, 31, &dstendday) &&
		testArg(getQueryStringParam("dstendmonth",   ""), 1, 12, &dstendmonth)) {

		bresult = Config::SaveConfig(tz, lect, debug_mode, dststartmonth, dststartday, dststarthour, dstendmonth, dstendday, dstendhour, dstoffset);
	}
	else {
		bresult = Config::SaveConfig(tz, lect, debug_mode);
	}
	
    I2CSerial.print(F("SaveConfig returned "));
	I2CSerial.println( bresult ? F("true") : F("false") );
	
    config_t c = {0};
    Config::GetConfig(c);
    I2CSerial.print(F("timezone = "));
	I2CSerial.println(String(c.data.timezone_offset));
    I2CSerial.print(F("lectionary = "));
	I2CSerial.println(String(c.data.lectionary_config_number));
	
	Config::bSettingsUpdated = true;

	bool b404 = !loadFromSdCard("setconf.htm");
	
	if (b404) {
		server.send(404, "text/plain", "Settings updated (setconf.htm not found)");
		I2CSerial.println(F("Sending 404"));
	}
}


String getQueryStringParam(String param_name, String default_value) {
  uint8_t i = 0;
  bool bFound = false;
  String res = default_value;
  
  while (i<server.args() && !bFound) {
    if (server.argName(i) == param_name) {
		res = server.arg(i);
		bFound = true;
	}
	else {
		i++;
	}
  }
  return res;
}



bool Config::StartServer() {
	bSettingsUpdated = false;
	
	wl_status_t status = WiFi.status();
	if(status == WL_CONNECTED) {
		I2CSerial.print(F("Config: Network '"));
		I2CSerial.print(WiFi.SSID());
		I2CSerial.println(F("' is connected, starting mDNS responder"));
	} 
	else {
		I2CSerial.print(F("\nCould not connect to WiFi. state="));
		I2CSerial.println(String(status)); 
		WiFi.printDiag(I2CSerial);
		return false;
	}
  
	// Set up mDNS responder:
	// - first argument is the domain name, in this example
	//   the fully-qualified domain name is "esp8266.local"
	// - second argument is the IP address to advertise
	//   we send our IP address on the WiFi network
	if (!MDNS.begin("lectionary")) {
		I2CSerial.println(F("Error setting up MDNS responder!"));
		return false;
	}

	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);
	
	I2CSerial.println(F("mDNS responder started"));
  
	server.on ( "/settings.json", handleSettingsJson );
	server.on ( "/setconf.htm", handleSetConf );
	server.onNotFound ( handleNotFound );

	server.begin();
	I2CSerial.println ( "HTTP server started" );
  
	return true;	
}

void Config::StopServer( void ) {
	server.close();
}

//Config::~Config() {
//}


bool Config::SaveConfig(String tz, String lect_num, String debug_mode) {
	return SaveConfig(tz, lect_num, debug_mode, 0,0,0,0,0,0,"0.0");
}


bool Config::SaveConfig(String tz, String lect_num, String debug_mode, 
						uint32_t dstStartMonth, uint32_t dstStartDay, uint32_t dstStartHour,
						uint32_t dstEndMonth,   uint32_t dstEndDay,   uint32_t dstEndHour,
						String dstoffset) 
{
  float timezone_offset = 0.0;
  int lectionary_config_number = 0;
  bool debug_on = false;
  bool debug_not_set = true;
  
  float dst_offset = 0.0;
  
  bool bGotTz = false;
  bool bGotLectNum = false;
  
  if (tz != "") {
	  if (IsNumeric(tz)) {
		timezone_offset = atof(tz.c_str());
		bGotTz = true;
		I2CSerial.print(F("timezone_offset="));
		I2CSerial.println(String(timezone_offset));
	  } 
	  else {
		I2CSerial.println(F("tz is not a number"));
		return false;
	  }
  }
  
  if (lect_num != "") {
	  if (IsNumeric(lect_num)) {
		lectionary_config_number = atoi(lect_num.c_str());  
		bGotLectNum = true;
		I2CSerial.print(F("lectionary_config_number="));
		I2CSerial.println(String(lectionary_config_number));
	  }
	  else {
		I2CSerial.println(F("lectionary config is not a number"));
		return false;
	  }
  }

  if (debug_mode == "1" || debug_mode == "true") {
	  debug_on = true;
	  debug_not_set = false;
  } 
  else if (debug_mode == "0" || debug_mode == "false") {
	  debug_on = false;
	  debug_not_set = false;
  }
  
  if (bGotTz && bGotLectNum) {
	  if (!(timezone_offset >= -12.0 && timezone_offset <= 12.0)) {
		I2CSerial.print(F("timezone_offset is out of range: "));
		I2CSerial.println(String(timezone_offset));
		return false;
	  }

	  if (IsNumeric(dstoffset)) {
		dst_offset = atof(dstoffset.c_str());
		I2CSerial.print(F("dst_offset="));
		I2CSerial.println(String(dst_offset));
	  } 
	  else {
		I2CSerial.println(F("dst_offset is not a number"));
		return false;
	  }

	  if (!(dst_offset >= 0.0 && dst_offset <= 3.0)) {
		I2CSerial.print(F("dst_offset is out of range: "));
		I2CSerial.println(String(dst_offset));
		return false;
	  }
  }
  
  config_t c = {0};
  GetConfig(c); // load config as it is and update, so when it is written back it doesn't destroy other members which have not changed

  if (!debug_not_set) {
	c.data.debug_on = debug_on;
  }
  
  if (bGotTz && bGotLectNum) {
	c.data.timezone_offset = timezone_offset;
	c.data.lectionary_config_number = lectionary_config_number;
 
	c.data.dst_offset = dst_offset;
	c.data.dst_start_month = dstStartMonth;
	c.data.dst_start_day = dstStartDay;
	c.data.dst_start_hour = dstStartHour;
	c.data.dst_end_month = dstEndMonth;
	c.data.dst_end_day = dstEndDay;
	c.data.dst_end_hour = dstEndHour;
  }  
  SaveConfig(c);
  
  dump_config(c);
  
  return true;
}

void Config::dump_config(config_t& c) {
	I2CSerial.print(F("c.data.timezone_offset:\t")); 			I2CSerial.println(String(c.data.timezone_offset));
	I2CSerial.print(F("c.data.lectionary_config_number:\t")); 	I2CSerial.println(String(c.data.lectionary_config_number));
	I2CSerial.print(F("c.data.debug_on:\t")); 					I2CSerial.println(String(c.data.debug_on));
	I2CSerial.print(F("c.data.dst_offset:\t")); 				I2CSerial.println(String(c.data.dst_offset));
	I2CSerial.print(F("c.data.dst_start_month:\t")); 			I2CSerial.println(String(c.data.dst_start_month));
	I2CSerial.print(F("c.data.dst_start_day:\t")); 				I2CSerial.println(String(c.data.dst_start_day));
	I2CSerial.print(F("c.data.dst_start_hour:\t")); 			I2CSerial.println(String(c.data.dst_start_hour));
	I2CSerial.print(F("c.data.dst_end_month:\t")); 				I2CSerial.println(String(c.data.dst_end_month));
	I2CSerial.print(F("c.data.dst_end_day:\t")); 				I2CSerial.println(String(c.data.dst_end_day));
	I2CSerial.print(F("c.data.dst_end_hour:\t")); 				I2CSerial.println(String(c.data.dst_end_hour));
	if (!DstIsValid(c)) {
		I2CSerial.print(F("DST settings invalid"));
	}
	
	I2CSerial.print(F("\nc.crc32:\t")); 						I2CSerial.println(String(c.crc32, HEX));
}

bool Config::DstIsValid(config_t& c) {
	uint8_t m[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	return ((c.data.dst_start_month >= 1)   && (c.data.dst_start_month <= 12) &&
	        (c.data.dst_end_month   >= 1)   && (c.data.dst_end_month   <= 12) &&
	        (c.data.dst_start_day   >= 1)   && (c.data.dst_start_day   <= m[c.data.dst_start_month]) && // dst beginning or ending on Feb 29 (leap year) is not supported
	        (c.data.dst_end_day     >= 1)   && (c.data.dst_end_day     <= m[c.data.dst_end_month]) &&
	        (c.data.dst_start_hour  >= 0)   && (c.data.dst_start_hour  <= 23) &&
	        (c.data.dst_end_hour    >= 0)   && (c.data.dst_end_hour    <= 23) &&
			(c.data.dst_offset      >= 0.0) && (c.data.dst_offset      <= 3.0));
}

void Config::InvalidateEEPROM() {
	I2CSerial.println(F("Invalidating EEPROM to oblige user to reconfigure time/dst settings etc"));
	config_t c = {0};
	storeStruct(&c, sizeof(config_t));
}

void Config::SaveConfig(config_t& c) {
  //c->checksum = CountBytes(c->timezone_offset) + CountBytes(c->lectionary_config_number) + CountBytes(c->century) + CountBytes(c->debug_on);
  uint32_t crcOfData = calculateCRC32((uint8_t*) (uint8_t*)&c.data, sizeof(c.data));
  c.crc32 = crcOfData;
  storeStruct(&c, sizeof(config_t));
}

bool Config::GetConfig(config_t& c) {
  loadStruct(&c, sizeof(config_t));

  uint32_t crcOfData = calculateCRC32((uint8_t*) (uint8_t*)&c.data, sizeof(c.data));
  if (crcOfData == c.crc32) {
	  return true;
  }
  
//  if (c->checksum == (CountBytes(c->timezone_offset) + CountBytes(c->lectionary_config_number) + CountBytes(c->century) + CountBytes(c->debug_on))) {
//	  return true;
//  }
  
  I2CSerial.println(F("Config: EEPROM checksum wrong or uninitialized"));
  return false;
}

bool Config::EEPROMChecksumValid() {
	time64_t t;
	if (!getDateTime(&t)){ // if the time is invalid, also return false
		return false;
	}
	
	config_t c = {0};
	return GetConfig(c);
}

bool Config::IsNumeric(String str) {
    unsigned int stringLength = str.length();
 
    if (stringLength == 0) {
        return false;
    }

    int startpos = 0;

    if (str.charAt(0) == '-') {
      startpos++;
    }
 
    boolean seenDecimal = false;

    for(unsigned int i = startpos; i < stringLength; ++i) {        
        if (isDigit(str.charAt(i))) {
            continue;
        }
 
        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
}

//https://github.com/esp8266/Arduino/issues/1539
void Config::storeStruct(void *data_source, size_t size)
{
  EEPROM.begin(1024/*size * 2*/);
  for(size_t i = 0; i < size; i++)
  {
    char data = ((char *)data_source)[i];
    EEPROM.write(i, data);
  }
  EEPROM.commit();
}

void Config::loadStruct(void *data_dest, size_t size)
{
    EEPROM.begin(1024/*size * 2*/);
    for(size_t i = 0; i < size; i++)
    {
        char data = EEPROM.read(i);
        ((char *)data_dest)[i] = data;
    }
}

bool Config::ClockWasReset() {
	bool clockwasreset = false;
	time64_t t = 0;
	getDateTime(&t, clockwasreset);
	return clockwasreset;
}

bool Config::getDateTime(time64_t* t) {
	bool clockwasreset = false;
	return getDateTime(t, clockwasreset);
}

bool Config::getDateTime(time64_t* t, bool& clockwasreset) {
  clockwasreset = false;

  uint8_t buf[7];
  uint8_t bcode = 0; 
  
  buf[0] = 0; // start at register 0 in the ds3231
  
  brzo_i2c_start_transaction(DS3231_I2C_ADDR, I2CSerial.SCL_speed); // 104 is DS3231 device address
    brzo_i2c_write(buf, 1, true);
    brzo_i2c_read(buf, 7, true);
    //delay(1);
  bcode = brzo_i2c_end_transaction();

  if (bcode != 0) {
    I2CSerial.soft_reset();
    return false;
  }

  uint8_t second, minute, hour, day, date, month, year;
 
  second = bcd2dec(buf[0]); 		//(((buf[0] & B11110000)>>4)*10 + (buf[0] & B00001111)); // convert BCD to decimal
  minute = bcd2dec(buf[1]); 		//(((buf[1] & B11110000)>>4)*10 + (buf[1] & B00001111)); // convert BCD to decimal
  hour   = bcd2dec(buf[2] & 0x3f); 	//(((buf[2] & B00110000)>>4)*10 + (buf[2] & B00001111)); // convert BCD to decimal (assume 24 hour mode)
  day    = buf[3] & 0x7; 			//(buf[3] & B00000111); // 1-7
  date   = bcd2dec(buf[4] & 0x3f); 	//(((buf[4] & B00110000)>>4)*10 + (buf[4] & B00001111)); // 1-31
  month  = bcd2dec(buf[5] & 0x1f); 	//(((buf[5] & B00010000)>>4)*10 + (buf[5] & B00001111)); //msb7 is century overflow
  year   = bcd2dec(buf[6]); 		//(((buf[6] & B11110000)>>4)*10 + (buf[6] & B00001111));
  
  bool century_overflowed = ((buf[5] & 0x80) != 0);
  
  if (day == 1 && date == 1 && month == 1 && year == 0 && hour == 0 && minute == 0 && second < 10) clockwasreset = true; // assume clock was reset (by power failure etc) if the values read are the same as the reset values (from DS3231 datasheet)
  
  //I2CSerial.printf("DS3231 Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", date, month, year, hour, minute, second);

  //I2CSerial.printf("-1-");
  
  int century = 0;
  
  config_t c = {0};
  if (GetConfig(c)) {
	if (century_overflowed) {
	  c.data.century++; // century rolled over since last call, so save new century
	  SaveConfig(c);
	  
	  bool ok = false;
	  int retries = 10;
	  while (!ok && --retries > 0) {
		ok = ClearCenturyFlag();
	  }
	  
	  if (!ok) {
		  I2CSerial.println(F("Century overflowed and was unable to reset the flag: century setting may be wrong on next call - connect power and use Lectionary config website to reset it"));
	  }
	  else {
		  I2CSerial.println(F("Century overflowed, flag in clock chip has been reset and century value incremented in EEPROM"));		  
	  }
	}

	century = c.data.century;  
  }
  
  //I2CSerial.printf("-2-");
  
  tmElements_t tm;
  
  tm.Second = second;
  tm.Minute = minute;
  tm.Hour = hour;
  tm.Day = date;
  tm.Month = month;
  tm.Year = y2kYearToTm(year + (century * 100));

  //I2CSerial.printf("-3- c.century=%d, year= %d", c.century, tm.Year);
  
  bool b_incorrectleapyear = false;
  
  if (tm.Year % 100 == 0 && tm.Year % 400 != 0 && tm.Month == 2 && tm.Day == 29) { // DS3231 is not designed to handle leap years correctly from 2100 (inclusive).
	tm.Day = 1;																	   // Guessing this is because it doesn't know that leap years occur on century boundaries 
	tm.Month = 3;																   // if the year is divisible by 400 as well as 100
	b_incorrectleapyear = true;	 												   // So it will incorrectly treat 2100 as a leap year, when it is not (2400 will be)
 }
  
  //I2CSerial.printf("%02d/%02d/%04d %02d:%02d:%02d\n", tm.Day, tm.Month, tm.Year, tm.Hour, tm.Minute, tm.Second);
  //I2CSerial.printf("-3-");
  *t = makeTime(tm);
  //I2CSerial.printf("-4-");
  
  if (b_incorrectleapyear) {
	  I2CSerial.print(F("Incorrect Leap Year detected from DS3231 - Feb 29 skipped, setting date to 1 March "));
	  I2CSerial.println(String(tm.Year));
	  setDateTime(*t);
  }
 
//  I2CSerial.printf("Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", tm.Day, tm.Month, tmYearToCalendar(tm.Year), tm.Hour, tm.Minute, tm.Second);
  
  if (*t < 3600 * 24 * 365) return false; // need some overhead. The liturgical calendar starts in 1970 (time_t value == 0), but the first season (Advent) begins in 1969, 
											// which is outside the range of a time64_t value, and will occur in calculations for year 1970 if not trapped

  
  //I2CSerial.printf("-4-");

  return true;
}

bool Config::isDST() {
	time64_t t;
	bool b_isdst = false;
	
	getLocalDateTime(&t, &b_isdst);
	
	return b_isdst;
} 

int Config::dstOffset() {
	config_t c = {0};
	if (!Config::GetConfig(c)) {
		return 0;
	}
	
	return (int)(c.data.dst_offset * 3600.0);
}

bool Config::getLocalDateTime(time64_t* t) { 
	bool isdst = false;
	
	return getLocalDateTime(t, &isdst);
}

bool Config::getLocalDateTime(time64_t* t, bool* isdst) { 
	tmElements_t ts = {0};

	config_t c = {0};
	if (!Config::GetConfig(c)) {
		*t = 0;
		return false;
	}
  
	bool bResult = Config::getDateTime(t);
  
	if (DstIsValid(c)) {
		if (bResult) {
			*t += (int)(c.data.timezone_offset * 3600.0);	  	
	  
			breakTime(*t, ts);
			// very basic DST support. DST start and end dates are provided by Javascript calculations in the config.htm web page, but in some jurisdictions these dates change from year to 
			// year, for example the change is made on the last Sunday of a month rather than the same date each year. In these jurisdictions, the DST compensation will likely be wrong in the 
			// years after the year in which the config.htm page was last used to set the time. This is easily corrected though by using the configuration interface again - this will need to
			// done at least once per year in order to keep the DST start and end dates current. (No user input is required in the configuration page, just click on the form submit button and 
			// the new start and end dates for DST will automatically be recorded.)
			//
			//dump_config(c);
			
			tmElements_t dst_start = {0};
			tmElements_t dst_end = {0};

			dst_start.Second = 0;
			dst_start.Minute = 0;
			dst_start.Hour   = c.data.dst_start_hour;
			dst_start.Day    = c.data.dst_start_day;
			dst_start.Month  = c.data.dst_start_month;
			dst_start.Year   = ts.Year;	

			dst_end.Second   = 0;
			dst_end.Minute   = 0;
			dst_end.Hour     = c.data.dst_end_hour;
			dst_end.Day      = c.data.dst_end_day;
			dst_end.Month    = c.data.dst_end_month;
			dst_end.Year     = ts.Year; 

			
			if (c.data.dst_start_month > c.data.dst_end_month) {// in southern hemisphere
				if (c.data.dst_end_month < ts.Month && ts.Month >= c.data.dst_start_month) { // ts.Month will be >= dst start month during dst, which is when the end of dst is in the 
					dst_end.Year++; // end dst month is in next year						 //	next year
					I2CSerial.printf("dst_end is in next year (southern hemisphere)\n");
				}
			}
			else if (c.data.dst_start_month < c.data.dst_end_month) { // in northern hemisphere
				if (c.data.dst_start_month < ts.Month && ts.Month >= c.data.dst_end_month) {
					dst_start.Year++; // start dst month is in next year
					I2CSerial.printf("dst_start is in next year (northern hemisphere)\n");
				}			
			}
			
			time64_t t_dst_start = makeTime(dst_start);
			time64_t t_dst_end = makeTime(dst_end);
			
			if (*t > t_dst_start && *t < t_dst_end) { // dst in effect if so, so add it on
				
				*t += (int)(c.data.dst_offset * 3600.0);	  
				*isdst = true;
				
				I2CSerial.println(F("DST in effect"));
			}
			else {
				*isdst = false;

				I2CSerial.println(F("Standard Time in effect"));
			}
		}
	}
	else {
		*isdst = false; // dst settings invalid
	}
	
	breakTime(*t, ts);
	I2CSerial.printf("Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

	return bResult;
}


bool Config::setDateTime(time64_t t) {
  tmElements_t tm;

  breakTime(t, tm);

  uint8_t buf[8];
  uint8_t bcode = 0; 

  buf[0] = 0; // start at register 0 in the ds3231
  buf[1] = (dec2bcd(tm.Second));
  buf[2] = (dec2bcd(tm.Minute));
  buf[3] = (dec2bcd(tm.Hour));      // sets 24 hour format
  buf[4] = (dec2bcd(tm.Wday));   
  buf[5] = (dec2bcd(tm.Day));
  buf[6] = (dec2bcd(tm.Month));
  buf[7] = (dec2bcd(tmYearToY2k(tm.Year % 100))); 
  
  brzo_i2c_start_transaction(DS3231_I2C_ADDR, I2CSerial.SCL_speed); // 104 is DS3231 device address
    brzo_i2c_write(buf, 8, true);
  bcode = brzo_i2c_end_transaction();

  if (bcode != 0) {
    I2CSerial.soft_reset();
	I2CSerial.printf("\nsetDateTime(): bcode=%d\n", bcode);
    return false;
  }

  config_t c = {0};
  
  if (GetConfig(c)) {
	c.data.century = (tmYearToY2k(tm.Year)/100);
	I2CSerial.printf("setDateTime() tmYearToY2k(tm.Year)=%d, tm.Year=%d, c.data.century=%d\n", tmYearToY2k(tm.Year), tm.Year, c.data.century);
  
	SaveConfig(c);
	GetConfig(c);
	I2CSerial.print(F("setDateTime() c.data.century="));
	I2CSerial.println(String(c.data.century));
  }
  return true;
}


bool Config::setAlarm(time64_t t, uint8_t alarm_number, uint8_t flags, bool enable_alarm) {
	if (isDST()) t-=dstOffset();

	tmElements_t tm;
	breakTime(t, tm);

	uint8_t bcode = 0; 

	if (alarm_number == 1) {
		uint8_t buf[5];
		buf[0] = DS3231_ALARM1_ADDR; // start at register 0x07 in the ds3231
		buf[1] = (dec2bcd(tm.Second));
		buf[2] = (dec2bcd(tm.Minute));
		buf[3] = (dec2bcd(tm.Hour));      // sets 24 hour format
		buf[4] = (dec2bcd(tm.Day));		  // also sets DY/DT = 0 -> Match Day of Month, and A1M4 to 0
		
		switch(flags) {	
		case A1_MATCH_DAY_AND_TIME:
			buf[4] = (dec2bcd(tm.Wday));   
			buf[4] |= 0x40;	// set DY/DT = 1 -> Match Weekday, A1M1 = A1M2 = A1M3 =A1M4 = 0
			break;
		
		case A1_ONCE_PER_SECOND:
			buf[1] |= 0x80; // set A1M1, A1M2, A1M3 and A1M4, alarm once per second
			buf[2] |= 0x80;
			buf[3] |= 0x80;
			buf[4] |= 0x80;

		case A1_MATCH_SECONDS:
			buf[2] |= 0x80; // set A1M2, A1M3 and A1M4, match seconds
			buf[3] |= 0x80;
			buf[4] |= 0x80;
			break;
		
		case A1_MATCH_MINUTES_SECONDS:
			buf[3] |= 0x80; // set A1M3 and A1M4, match seconds and minutes
			buf[4] |= 0x80;
			break;
		
		case A1_MATCH_HOURS_MINUTES_SECONDS:
			buf[4] |= 0x80; // set A1M4, match seconds, minutes and hours
			break;
			
		}
	  
		brzo_i2c_start_transaction(DS3231_I2C_ADDR, I2CSerial.SCL_speed); // 104 is DS3231 device address
			brzo_i2c_write(buf, 5, false);
		bcode = brzo_i2c_end_transaction();

		if (bcode != 0) {
			I2CSerial.soft_reset();
			I2CSerial.printf("\nsetAlarm(1): bcode=%d\n", bcode);
			return false;
		}
		
		if (enable_alarm) {
			return DS3231_arm_a1(true);
		}
	}
	else {
		uint8_t buf[4];
		buf[0] = DS3231_ALARM2_ADDR; // start at register 0x0B in the ds3231
		buf[1] = (dec2bcd(tm.Minute));
		buf[2] = (dec2bcd(tm.Hour));      // sets 24 hour format
		buf[3] = (dec2bcd(tm.Day));		  // also sets DY/DT = 0 -> Match Day of Month, and A2M4 to 0
		
		switch(flags) {	
		case A1_MATCH_DAY_AND_TIME:
			buf[3] = (dec2bcd(tm.Wday));   
			buf[3] |= 0x40;	// set DY/DT = 1 -> Match Weekday, A2M2 = A2M3 = A2M4 = 0
			break;
		
		case A1_ONCE_PER_SECOND:
			buf[1] |= 0x80;	// set A2M2, A2M3 and A2M4, alarm once per minute (there is no A2M1!)
			buf[2] |= 0x80;
			buf[3] |= 0x80;

		case A1_MATCH_SECONDS:
			buf[2] |= 0x80; // set A2M3 and A2M4, match minutes
			buf[3] |= 0x80;
			break;
		
		case A1_MATCH_MINUTES_SECONDS:
			buf[3] |= 0x80; // set A1M4, match hours and minutes
			break;			
		}
	  
		brzo_i2c_start_transaction(DS3231_I2C_ADDR, I2CSerial.SCL_speed); // 104 is DS3231 device address
			brzo_i2c_write(buf, 4, false);
		bcode = brzo_i2c_end_transaction();

		if (bcode != 0) {
			I2CSerial.soft_reset();
			I2CSerial.printf("\nsetAlarm(2): bcode=%d\n", bcode);
			return false;
		}
		
		if (enable_alarm) {
			return DS3231_arm_a2(true);
		}
	}
	
	return true;
}

bool Config::DS3231_set_addr(const uint8_t addr, const uint8_t val)
{
	uint8_t bcode = 0; 

	uint8_t buf[2];

	buf[0] = addr;
	buf[1] = val;
	
	brzo_i2c_start_transaction(DS3231_I2C_ADDR, I2CSerial.SCL_speed); // 104 is DS3231 device address
		brzo_i2c_write(buf, 2, false);
	bcode = brzo_i2c_end_transaction();

	if (bcode != 0) {
		I2CSerial.soft_reset();
		I2CSerial.printf("\nDS3231_set_addr: bcode=%d\n", bcode);
		return false;
	}
	
	return true;
}

uint8_t Config::DS3231_get_addr(const uint8_t addr, bool& ok)
{
	ok = true;
	
	uint8_t bcode = 0; 

	uint8_t buf[1];

	buf[0] = addr;
	
	brzo_i2c_start_transaction(DS3231_I2C_ADDR, I2CSerial.SCL_speed); // 104 is DS3231 device address
		brzo_i2c_write(buf, 1, false);
		brzo_i2c_read(buf, 1, false);
	bcode = brzo_i2c_end_transaction();

	if (bcode != 0) {
		I2CSerial.soft_reset();
		I2CSerial.printf("\nDS3231_get_addr: bcode=%d\n", bcode);
		ok = false;
	}	  

	return buf[0];
}

// control register

bool Config::DS3231_set_creg(const uint8_t val)
{
    return DS3231_set_addr(DS3231_CONTROL_ADDR, val);
}

// status register 0Fh/8Fh

/*
bit7 OSF      Oscillator Stop Flag (if 1 then oscillator has stopped and date might be innacurate)
bit3 EN32kHz  Enable 32kHz output (1 if needed)
bit2 BSY      Busy with TCXO functions
bit1 A2F      Alarm 2 Flag - (1 if alarm2 was triggered)
bit0 A1F      Alarm 1 Flag - (1 if alarm1 was triggered)
*/

bool Config::DS3231_set_sreg(const uint8_t val)
{
    return DS3231_set_addr(DS3231_STATUS_ADDR, val);
}


uint8_t Config::DS3231_get_sreg(bool& ok)
{
    uint8_t rv;
    rv = DS3231_get_addr(DS3231_STATUS_ADDR, ok);
    return rv;
}

uint8_t Config::DS3231_get_creg(bool& ok)
{
    uint8_t rv;
    rv = DS3231_get_addr(DS3231_CONTROL_ADDR, ok);
    return rv;
}


//Alarm 1
// when the alarm flag is cleared the pulldown on INT is also released
bool Config::DS3231_clear_a1f()
{
	bool ok = true;
	
    uint8_t reg_val;

    reg_val = DS3231_get_sreg(ok) & ~DS3231_A1F;
    if (ok) {
		ok = DS3231_set_sreg(reg_val);
	}
	
	return ok;
}


uint8_t Config::DS3231_triggered_a1(bool& ok)
{
    return DS3231_get_sreg(ok) & DS3231_A1F;
}


bool Config::DS3231_arm_a1(bool enable) {
	bool ok = true;
	
	uint8_t creg = DS3231_get_creg(ok);
	
	if (!ok) return false;
	
	if(enable){
		return DS3231_set_creg(creg | DS3231_INTCN | DS3231_A1IE);
	}
	else {
		return DS3231_set_creg(creg & ~DS3231_A1IE);
	}
}


//Alarm 2
// when the alarm flag is cleared the pulldown on INT is also released
bool Config::DS3231_clear_a2f()
{
	bool ok = true;
	
    uint8_t reg_val;

    reg_val = DS3231_get_sreg(ok) & ~DS3231_A2F;
    if (ok) {
		ok = DS3231_set_sreg(reg_val);
	}
	
	return ok;
}

uint8_t Config::DS3231_triggered_a2(bool& ok)
{
    return DS3231_get_sreg(ok) & DS3231_A2F;
}


bool Config::DS3231_arm_a2(bool enable) {
	bool ok = true;
	
	uint8_t creg = DS3231_get_creg(ok);
	
	if (!ok) return false;
	
	if(enable){
		return DS3231_set_creg(creg | DS3231_INTCN | DS3231_A2IE);
	}
	else {
		return DS3231_set_creg(creg & ~DS3231_A2IE);
	}
}


bool Config::Clock_reset() {
	if (DS3231_set_creg(0x1C) && // /EOSC = 0, BBSQW = 0, CONV = 0, RS2 = 1, RS1 = 1, INTCN = 1, A2IE = 0, A1IE = 0
	    DS3231_set_sreg(0x00))   // OSF (oscillator stopped) = 0, bits 6, 5, 4 = 0, EN32KHZ = 0 (disabled), BUSY = 0, A2F, A1F = 0 (A2 and A1 alarm match cleared, /INT+SQW pin released (pulled up))
	{
		return true;
	}
	
	return false;
}

bool Config::ClockStopped(bool& ok) {
	return ((DS3231_get_sreg(ok) & DS3231_OSF) != 0);
}

bool Config::ClearClockStoppedFlag() {
	bool ok = true;
	
    uint8_t reg_val;

    reg_val = DS3231_get_sreg(ok) & ~DS3231_OSF;
    if (ok) {
		ok = DS3231_set_sreg(reg_val);
	}
	
	return ok;	
}

bool Config::ClearCenturyFlag() {
	bool ok = true;
    uint8_t reg_val = 0;

    reg_val = (DS3231_get_addr(0x05, ok)) & 0x7F; // get Month value (register 0x05), msb is the century bit, which is cleared
    
	if (ok) {
		ok = DS3231_set_addr(0x05, reg_val);
	}
	
	return ok;		
}

uint8_t Config::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t Config::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

bool testArg(String arg, uint32_t min, uint32_t max, uint32_t* outval) {
	*outval = 0;
	if (Config::IsNumeric(arg)) {
		*outval = arg.toInt();
		return ((*outval >= min) && (*outval <= max));
	}
	else {
		return false;
	}
	
	return false;
}

bool Config::readRtcMemoryData(rtcData_t& rtcData) {
	  // Read struct from RTC memory
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    I2CSerial.println("Read: ");
    //printMemory();
    //I2CSerial.println("number of hours to next reading: " + String(rtcData.data.wake_hour_counter)); // set to true if the reading should be made this hour (read on waking)
    //I2CSerial.println("card displayed: " + String(rtcData.data.dcs)); // value shows if any of the error cards are displayed, so that, if the condition has not changed, it need not be redisplayed
    
    I2CSerial.println();
    uint32_t crcOfData = calculateCRC32((uint8_t*) (uint8_t*)&rtcData.data, sizeof(rtcData.data));
    I2CSerial.print(F("CRC32 of data: "));
    I2CSerial.println(crcOfData, HEX);
    I2CSerial.print(F("CRC32 read from RTC: "));
    I2CSerial.println(rtcData.crc32, HEX);
    
	if (crcOfData != rtcData.crc32) {
      I2CSerial.println(F("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!"));
	  return false;
    } 
    
	I2CSerial.println(F("CRC32 check ok, data is probably valid."));
	return true;
  }
  
  return false;
}

void Config::writeRtcMemoryData(rtcData_t& rtcData) {
	  // Update CRC32 of data
  rtcData.crc32 = calculateCRC32((uint8_t*) &rtcData.data, sizeof(rtcData.data));
  // Write struct to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    I2CSerial.println(F("Write: "));
    printMemory(rtcData);
	I2CSerial.println();
  }
}

uint32_t Config::calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

//prints all rtcData, including the leading crc32
void Config::printMemory(rtcData_t& rtcData) {
  char buf[3];
  uint8_t *ptr = (uint8_t *)&rtcData;
  for (size_t i = 0; i < sizeof(rtcData); i++) {
    sprintf(buf, "%02X", ptr[i]);
    I2CSerial.print(buf);
    if ((i + 1) % 32 == 0) {
      I2CSerial.println();
    } else {
      I2CSerial.print(" ");
    }
  }
  Serial.println();
}

wake_reasons Config::Wake_Reason() {
	int retries = 50;
  
	bool ok1 = false;
	bool triggered_a1 = false;
  
	while (!ok1 && --retries != 0) {
		triggered_a1 = Config::DS3231_triggered_a1(ok1);
		delay(50);
	}

	bool ok2 = false;
	bool triggered_a2 = false;

	while (!ok2 && --retries != 0) {
		triggered_a2 = Config::DS3231_triggered_a2(ok2);
		delay(50);
	}

	if (triggered_a1) {
		I2CSerial.println("Woken by Alarm 1");
		return WAKE_ALARM_1;
	}
  
	if (triggered_a2) {
		I2CSerial.println("Woken by Alarm 2");
		return WAKE_ALARM_2;
	}

    rst_info *rinfo;
    rinfo = ESP.getResetInfoPtr();

    if ((*rinfo).reason == REASON_DEEP_SLEEP_AWAKE) { // only check the hour count to the next reading if we awoke because of the deepsleep timer
		I2CSerial.println(String("ResetInfo.reason = ") + (*rinfo).reason);		
		return WAKE_DEEPSLEEP;
	}
   
	if (!(triggered_a1 || triggered_a2)) {
		I2CSerial.println("Woken by USB 5volts");
		return WAKE_USB_5V;
	}
	
	return WAKE_UNKNOWN;
}


bool Config::PowerOff(time64_t wake_datetime) {
	if (wake_datetime != 0) {
		if (Config::setAlarm(wake_datetime, 1, A1_MATCH_DAY_AND_TIME, true)) {
			I2CSerial.print("Alarm 1 set");
			tmElements_t ts;
			breakTime(wake_datetime, ts);
			I2CSerial.printf(" for %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
			delay(200);
		}
		else {
			I2CSerial.print("Problem setting Alarm 1");
			return false;
		}
	}
	else {
		I2CSerial.print("Not setting alarm, power off until USB5V connected");    		
	}
    
	bool okCleared_a1 = false;
	bool okCleared_a2 = false;

	int retries = 50;
  
	I2CSerial.println("Clearing A1F");
	while(!okCleared_a1 && --retries != 0) {
		okCleared_a1 = Config::DS3231_clear_a1f();
		delay(50);
	}
  
	//might not get here if A1F was just cleared, since clearing it removes power from ESP8266
  
	I2CSerial.println("Clearing A2F");
	while(!okCleared_a2 && --retries != 0) {
		okCleared_a2 = Config::DS3231_clear_a2f();
		delay(50);
	}
  
	delay(100);
	// should be powered off by this point.
	return false;
}

bool Config::SetPowerOn() {
	//Attempt to set A1F to cause /reset line to assert, hence hold up the enable line on the 3.3v LDO powering the ESP8266
	bool ok = false;
	
	time64_t t = 0;
	
	ok = getLocalDateTime(&t);	
	if (ok) {
		return Config::setAlarm(t+3, 1, A1_MATCH_DAY_AND_TIME, true); // this will go off in three seconds, and assert the /res line. This will hold up the ENable pin on the 3.3V LDO and keep the ESP on until the alarm is set to the wakeup time.
	}
	
	return false;
}