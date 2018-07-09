#include "config.h"

Config::Config() : server(80) {}

bool Config::StartServer( I18n* i ) {
	if (i == NULL) return false;
	
	_I18n = i;
	
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

  // TCP server at port 80 will respond to HTTP requests
//  if (p_server == NULL) {
//	  p_server = new WiFiServer(80);
//  }
  
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("lectionary")) {
    I2CSerial.println(F("Error setting up MDNS responder!"));
	return false;
  }
  I2CSerial.println(F("mDNS responder started"));
  
//  if (p_server == NULL) {
//	  I2CSerial.println("p_server is null!");
//  }
  
  // Start TCP (HTTP) server
//  p_server->begin();
/*
	server.on ( "/", handleRoot );
	server.on ( "/test.svg", drawGraph );
	server.on ( "/inline", []() {
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
*/
  server.begin();
  I2CSerial.println(F("TCP server started"));
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  return true;	
}

void Config::StopServer( void ) {
	server.close();
//  if (p_server != NULL) {
//	  p_server->close();
//  }	
}

Config::~Config() {
  //delete p_server;
}

bool Config::ServeClient(bool* bSettingsUpdated)
{
	*bSettingsUpdated = false;
	
	if (_I18n == NULL) {
		return false;
	}
	
	wl_status_t status = WiFi.status();
	if(status != WL_CONNECTED) {
		I2CSerial.print(F("Network is not connected. state="));
		I2CSerial.println(String(status)); 
		WiFi.printDiag(I2CSerial);
		return false;
	}

  // Check if a client has connected
  //WiFiClient client = p_server->available();
  WiFiClient client = server.available();
  if (!client) {
    return true;
  }
  
  I2CSerial.println(F("\nNew client"));

  // Wait for data from client to become available
  while(client.connected() && !client.available()){
    delay(1);
  }
  
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');
  
  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    I2CSerial.print(F("Invalid request: "));
    I2CSerial.println(req);
    return true;
  }
  req = req.substring(addr_start + 1, addr_end);
  I2CSerial.print(F("Request: "));
  I2CSerial.println(req);

  String querystring = "";
  int querystring_start = req.indexOf("?");
  if (querystring_start != -1) querystring = req.substring(querystring_start + 1); 
  String filename = req.substring(0, querystring_start);

  I2CSerial.print(F("request filename: "));
  I2CSerial.println(filename);
  I2CSerial.print(F("request querystring: "));
  I2CSerial.println(querystring);
  
  client.flush();
  
  String header;
  String s;
  bool b404 = false;
  
  if (req == "/") {
    b404 = !sendHttpFile(&client, "/html/config.htm");
  }
  else if (filename == "/config.csv") {
    b404 = !sendHttpFile(&client, "/config.csv");
  }
  else if (filename == "/settings.json") {
    config_t c = {0};
    GetConfig(c);
	
    I2CSerial.print(F("timezone = "));
	I2CSerial.println(String(c.data.timezone_offset));
    I2CSerial.print(F("lectionary = "));
	I2CSerial.println(String(c.data.lectionary_config_number));
	
    String line = "{\"tz_offset\":\"" + String(c.data.timezone_offset) + "\", \"lectionary_config_number\":\"" + String(c.data.lectionary_config_number) + "\"}"; // output in JSON format
    String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + String(line.length()) + "\r\nConnection: close\r\n\r\n";
    client.print(header);
    I2CSerial.println(line);
    client.print(line);
  }
  else if (filename == "/setconf.htm") {
    String tz = getQueryStringParam("timezone", querystring, "0");
    String lect = getQueryStringParam("lectionary", querystring, "0");	

	String debug_mode = getQueryStringParam("debug", querystring, "");
	
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

	if (testArg(getQueryStringParam("hh", querystring, ""), 0, 23, &hh) &&
		testArg(getQueryStringParam("mm", querystring, ""), 0, 59, &mm) &&
		testArg(getQueryStringParam("ss", querystring, ""), 0, 59, &ss) &&
		testArg(getQueryStringParam("day", querystring, ""), 1, 31, &day) &&
		testArg(getQueryStringParam("mon", querystring, ""), 1, 12, &mon) &&
		testArg(getQueryStringParam("year", querystring, ""), 1970, 0xFFFFFFFF, &year)) { // was max range was 65535
			
		tmElements_t tm;
		
		tm.Hour = (uint8_t)hh;
		tm.Minute = (uint8_t)mm;
		tm.Second = (uint8_t)ss;
		tm.Day = (uint8_t)day;
		tm.Month = (uint8_t)mon;
		tm.Year = year - 1970;
		
		time64_t t = makeTime(tm);
		setDS3231DateTime(t);
	}
	
	uint32_t dststarthour = 0;
	uint32_t dststartday = 0;
	uint32_t dststartmonth = 0;
	
	uint32_t dstendhour = 0; 
	uint32_t dstendday = 0; 
	uint32_t dstendmonth = 0;

    String dstoffset = getQueryStringParam("dstoffset", querystring, "0");

	bool bresult = false;
	
	if (testArg(getQueryStringParam("dststarthour", querystring, ""), 0, 23, &dststarthour) &&
		testArg(getQueryStringParam("dststartday", querystring, ""), 1, 31, &dststartday) &&
		testArg(getQueryStringParam("dststartmonth", querystring, ""), 1, 12, &dststartmonth) &&
		testArg(getQueryStringParam("dstendhour", querystring, ""), 0, 23, &dstendhour) &&
		testArg(getQueryStringParam("dstendday", querystring, ""), 1, 31, &dstendday) &&
		testArg(getQueryStringParam("dstendmonth", querystring, ""), 1, 12, &dstendmonth)) {

		bresult = SaveConfig(tz, lect, debug_mode, dststartmonth, dststartday, dststarthour, dstendmonth, dstendday, dstendhour, dstoffset);
	}
	else {
		bresult = SaveConfig(tz, lect, debug_mode);
	}
	
    I2CSerial.print(F("SaveConfig returned "));
	I2CSerial.println( bresult ? F("true") : F("false") );
	
    config_t c = {0};
    GetConfig(c);
    I2CSerial.print(F("timezone = "));
	I2CSerial.println(String(c.data.timezone_offset));
    I2CSerial.print(F("lectionary = "));
	I2CSerial.println(String(c.data.lectionary_config_number));
	
	*bSettingsUpdated = true;

    b404 = !sendHttpFile(&client, "/html/setconf.htm");
  }
  else {
    b404 = true;
  }

  if (b404) {
    header = "HTTP/1.1 404 Not Found\r\n\r\n";
    client.print(header);
    I2CSerial.println(F("Sending 404"));
  }
      
  delay(1);
  client.stop();
  I2CSerial.println(F("Done with client"));
  return true;
}

String Config::getQueryStringParam(String param, String querystring, String default_value) {
  int param_offset = querystring.indexOf(param);
  
  if (param_offset == -1) {
	  return default_value;
  }
  
  param_offset += param.length() + 1; // add the length of the param name to the start position to get the start of the param value. Include the = after the variable name (+1)

  int param_end = querystring.indexOf("&", param_offset);
  if (param_end == -1) param_end = querystring.length();
  return querystring.substring(param_offset, param_end);  
}

bool Config::sendHttpFile(WiFiClient* client, String filename) {
	char filenamestr[80];
	filename.toCharArray(filenamestr, 80);
	
	if (!SD.exists(filenamestr)) {		
		I2CSerial.print(F("File "));
		I2CSerial.print(filename);
		I2CSerial.println(F(" not found"));
		return false;
	}

  File file = SD.open(filename, FILE_READ);
  if (!file.available()) {
    return false;
  } else {
	int filelength = file.size();
    String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + String(filelength) + "\r\nConnection: close\r\n\r\n";
    client->print(header);
    String line;
    do {
      line = _I18n->readLine(file) + "\r\n";
      client->print(line);
    } while (file.available());
    file.close();
  }
  return true;
}

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
  
  if (IsNumeric(tz)) {
    timezone_offset = atof(tz.c_str());
    I2CSerial.print(F("timezone_offset="));
	I2CSerial.println(String(timezone_offset));
  } 
  else {
    I2CSerial.println(F("tz is not a number"));
    return false;
  }

  if (IsNumeric(lect_num)) {
    lectionary_config_number = atoi(lect_num.c_str());  
    I2CSerial.print(F("lectionary_config_number="));
	I2CSerial.println(String(lectionary_config_number));
  }
  else {
    I2CSerial.println(F("lectionary config is not a number"));
    return false;
  }

  if (!(timezone_offset >= -12.0 && timezone_offset <= 12.0)) {
    I2CSerial.print(F("timezone_offset is out of range: "));
	I2CSerial.println(String(timezone_offset));
    return false;
  }

  if (debug_mode == "1" || debug_mode == "true") {
	  debug_on = true;
	  debug_not_set = false;
  } 
  else if (debug_mode == "0" || debug_mode == "false") {
	  debug_on = false;
	  debug_not_set = false;
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

  
  config_t c = {0};
  GetConfig(c); // load config as it is and update, so when it is written back it doesn't destroy other members which have not changed
  
  c.data.timezone_offset = timezone_offset;
  c.data.lectionary_config_number = lectionary_config_number;
  
  if (!debug_not_set) {
	c.data.debug_on = debug_on;
  }
  
  c.data.dst_offset = dst_offset;
  c.data.dst_start_month = dstStartMonth;
  c.data.dst_start_day = dstStartDay;
  c.data.dst_start_hour = dstStartHour;
  c.data.dst_end_month = dstEndMonth;
  c.data.dst_end_day = dstEndDay;
  c.data.dst_end_hour = dstEndHour;
  
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
	I2CSerial.print(F("\nc.crc32:\t")); 						I2CSerial.println(String(c.crc32, HEX));
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
	if (!getDS3231DateTime(&t)){ // if the time is invalid, also return false
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

bool Config::getDS3231DateTime(time64_t* t) {
  uint8_t buf[7];
  uint8_t bcode = 0; 
  
  buf[0] = 0; // start at register 0 in the ds3231
  
  brzo_i2c_start_transaction(DS3231_I2C_ADDRESS, I2CSerial.SCL_speed); // 104 is DS3231 device address
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
  
  //I2CSerial.printf("DS3231 Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", date, month, year, hour, minute, second);

  //I2CSerial.printf("-1-");
  
  int century = 0;
  
  config_t c = {0};
  if (GetConfig(c)) {
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
	  setDS3231DateTime(*t);
  }
 
//  I2CSerial.printf("Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", tm.Day, tm.Month, tmYearToCalendar(tm.Year), tm.Hour, tm.Minute, tm.Second);
  
  if (*t < 3600 * 24 * 365) return false; // need some overhead. The liturgical calendar starts in 1970 (time_t value == 0), but the first season (Advent) begins in 1969, 
											// which is outside the range of a time64_t value, and will occur in calculations for year 1970 if not trapped

  
  //I2CSerial.printf("-4-");

  return true;
}

bool Config::getLocalDS3231DateTime(time64_t* t) { 
	tmElements_t ts = {0};

	config_t c = {0};
	if (!GetConfig(c)) {
		*t = 0;
		return false;
	}
  
	bool bResult = getDS3231DateTime(t);
  
	if (bResult) {
		*t += (int)(c.data.timezone_offset * 3600.0);	  	
  
		breakTime(*t, ts);
		// very basic DST support. DST start and end dates are provided by Javascript calculations in the config.htm web page, but in some jurisdictions these dates change from year to 
		// year, for example the change is made on the last Sunday of a month rather than the same date each year. In these jurisdictions, the DST compensation will likely be wrong in the 
		// years after the year in which the config.htm page was last used to set the time. This is easily corrected though by using the configuration interface again - this will need to
		// done at least once per year in order to keep the DST start and end dates current. (No user input is required in the configuration page, just click on the form submit button and 
		// the new start and end dates for DST will automatically be recorded.)
		//
		dump_config(c);
		
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
			
			I2CSerial.println(F("DST in effect"));
		}
		else {
			I2CSerial.println(F("Standard Time in effect"));
		}
	}
	
	breakTime(*t, ts);
	I2CSerial.printf("Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

	return bResult;
}


bool Config::setDS3231DateTime(time64_t t) {
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
  
  brzo_i2c_start_transaction(DS3231_I2C_ADDRESS, I2CSerial.SCL_speed); // 104 is DS3231 device address
    brzo_i2c_write(buf, 8, true);
  bcode = brzo_i2c_end_transaction();

  if (bcode != 0) {
    I2CSerial.soft_reset();
    return false;
  }

  config_t c = {0};
  
  if (GetConfig(c)) {
	c.data.century = (tmYearToY2k(tm.Year)/100);
	I2CSerial.printf("setDS3231DateTime() tmYearToY2k(tm.Year)=%d, tm.Year=%d, c.data.century=%d\n", tmYearToY2k(tm.Year), tm.Year, c.data.century);
  
	SaveConfig(c);
	GetConfig(c);
	I2CSerial.print(F("setDS3231DateTime() c.data.century="));
	I2CSerial.println(String(c.data.century));
  }
  return true;
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

bool Config::testArg(String arg, uint32_t min, uint32_t max, uint32_t* outval) {
	*outval = 0;
	if (IsNumeric(arg)) {
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
