#include "config.h"

Config::Config() : server(80) {}

bool Config::StartServer( I18n* i ) {
	if (i == NULL) return false;
	
	_I18n = i;
	
	wl_status_t status = WiFi.status();
	if(status == WL_CONNECTED) {
		I2CSerial.println("Config: Network '" + WiFi.SSID() + "' is connected, starting mDNS responder\n");
	} 
	else {
		I2CSerial.println("\nCould not connect to WiFi. state='" + String(status) + "'"); 
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
    I2CSerial.println("Error setting up MDNS responder!");
	return false;
  }
  I2CSerial.println("mDNS responder started");
  
//  if (p_server == NULL) {
//	  I2CSerial.println("p_server is null!");
//  }
  
  // Start TCP (HTTP) server
//  p_server->begin();
  server.begin();
  I2CSerial.println("TCP server started");
  
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
		I2CSerial.println("Network is not connected. state='" + String(status) + "'"); 
		return false;
	}

  // Check if a client has connected
  //WiFiClient client = p_server->available();
  WiFiClient client = server.available();
  if (!client) {
    return true;
  }
  
  I2CSerial.println("");
  I2CSerial.println("New client");

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
    I2CSerial.print("Invalid request: ");
    I2CSerial.println(req);
    return true;
  }
  req = req.substring(addr_start + 1, addr_end);
  I2CSerial.print("Request: ");
  I2CSerial.println(req);

  String querystring = "";
  int querystring_start = req.indexOf("?");
  if (querystring_start != -1) querystring = req.substring(querystring_start + 1); 
  String filename = req.substring(0, querystring_start);

  I2CSerial.println("request filename: " + filename);
  I2CSerial.println("request querystring: " + querystring);
  
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
    config_t c;
    GetConfig(&c);
    I2CSerial.println("timezone = " + String(c.timezone_offset));
    I2CSerial.println("lectionary = " + String(c.lectionary_config_number));
    String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    client.print(header);
    String line = "{\"tz_offset\":\"" + String(c.timezone_offset) + "\", \"lectionary_config_number\":\"" + String(c.lectionary_config_number) + "\"}"; // output in JSON format
    I2CSerial.println(line);
    client.print(line);
  }
  else if (filename == "/setconf.htm") {
    String tz = getQueryStringParam("timezone", querystring, "0");
    String lect = getQueryStringParam("lectionary", querystring, "0");	

	String debug_mode = getQueryStringParam("debug", querystring, "0");
	
    I2CSerial.println("timezone = " + tz);
    I2CSerial.println("lectionary = " + lect);
    
	if (debug_mode != "") {
		I2CSerial.println("debug = " + debug_mode);
	}
	
    bool bresult = SaveConfig(tz, lect, debug_mode);

    I2CSerial.println("SaveConfig returned " + String(bresult?"true":"false"));
		
	int hh, mm, ss, day, mon, year;

	if (testArg(getQueryStringParam("hh", querystring, ""), 0, 23, &hh) &&
		testArg(getQueryStringParam("mm", querystring, ""), 0, 59, &mm) &&
		testArg(getQueryStringParam("ss", querystring, ""), 0, 59, &ss) &&
		testArg(getQueryStringParam("day", querystring, ""), 1, 31, &day) &&
		testArg(getQueryStringParam("mon", querystring, ""), 1, 12, &mon) &&
		testArg(getQueryStringParam("year", querystring, ""), 1970, 65535, &year)) {
			
		tmElements_t tm;
		
		tm.Hour = hh;
		tm.Minute = mm;
		tm.Second = ss;
		tm.Day = day;
		tm.Month = mon;
		tm.Year = year - 1970;
		
		time64_t t = makeTime(tm);
		setDS3231DateTime(t);
	}

    config_t c;
    GetConfig(&c);
    I2CSerial.println("timezone = " + String(c.timezone_offset));
    I2CSerial.println("lectionary = " + String(c.lectionary_config_number));
	*bSettingsUpdated = true;

    b404 = !sendHttpFile(&client, "/html/setconf.htm");
  }
  else {
    b404 = true;
  }

  if (b404) {
    header = "HTTP/1.1 404 Not Found\r\n\r\n";
    client.print(header);
    I2CSerial.println("Sending 404");
  }
      
  delay(1);
  client.stop();
  I2CSerial.println("Done with client");
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
  File file = _I18n->openFile(filename, FILE_READ);
  if (!file.available()) {
    return false;
  } else {
    String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    client->print(header);
    String line;
    do {
      line = _I18n->readLine(file) + "\r\n";
      client->print(line);
    } while (file.available());
    _I18n->closeFile(file);
  }
  return true;
}

bool Config::SaveConfig(String tz, String lect_num, String debug_mode) {
  float timezone_offset;
  int lectionary_config_number;
  bool debug_on = false;
  
  if (IsNumeric(tz)) {
    timezone_offset = atof(tz.c_str());
    I2CSerial.println("timezone_offset=" + String(timezone_offset));
  } 
  else {
    I2CSerial.println("tz is not a number");
    return false;
  }

  if (IsNumeric(lect_num)) {
    lectionary_config_number = atoi(lect_num.c_str());  
    I2CSerial.println("lectionary_config_number=" + String(lectionary_config_number));
  }
  else {
    I2CSerial.println("lectionary config is not a number");
    return false;
  }

  if (!(timezone_offset >= -12 && timezone_offset <= 12)) {
    I2CSerial.println("timezone_offset is out of range: " + String(timezone_offset));
    return false;
  }

  if (debug_mode == "1" || debug_mode == "true") {
	  debug_on = true;
  } else {
	  debug_on = false;
  }
  
  config_t c;
  GetConfig(&c); // load config as it is and update, so when it is written back it doesn't destroy other members which have not changed
  
  c.timezone_offset = timezone_offset;
  c.lectionary_config_number = lectionary_config_number;
  c.debug_on = debug_on;
  
  SaveConfig(&c);
  return true;
}

void Config::SaveConfig(config_t* c) {
  c->checksum = CountBytes(c->timezone_offset) + CountBytes(c->lectionary_config_number) + CountBytes(c->century) + CountBytes(c->debug_on);
  storeStruct(c, sizeof(config_t));
}

bool Config::GetConfig(config_t* c) {
  loadStruct(c, sizeof(config_t));

  if (c->checksum == (CountBytes(c->timezone_offset) + CountBytes(c->lectionary_config_number) + CountBytes(c->century) + CountBytes(c->debug_on))) {
	  return true;
  }
  
  I2CSerial.printf("Config: EEPROM checksum wrong or uninitialized\n");
  return false;
}

bool Config::EEPROMChecksumValid() {
	config_t c;
	return GetConfig(&c);
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
  EEPROM.begin(size * 2);
  for(size_t i = 0; i < size; i++)
  {
    char data = ((char *)data_source)[i];
    EEPROM.write(i, data);
  }
  EEPROM.commit();
}

void Config::loadStruct(void *data_dest, size_t size)
{
    EEPROM.begin(size * 2);
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
  
  I2CSerial.printf("DS3231 Datetime = %d/%d/%d %d:%d:%d\n", date, month, year, hour, minute, second);

  //I2CSerial.printf("-1-");
  config_t c;
  GetConfig(&c);
  //I2CSerial.printf("-2-");
  
  tmElements_t tm;
  
  tm.Second = second;
  tm.Minute = minute;
  tm.Hour = hour;
  tm.Day = date;
  tm.Month = month;
  tm.Year = y2kYearToTm(year + (c.century * 100));

  //I2CSerial.printf("-3- c.century=%d, year= %d", c.century, tm.Year);
  
  *t = makeTime(tm);
  
  //I2CSerial.printf("-4-");

  return true;
}

bool Config::getLocalDS3231DateTime(time64_t* t) {
  config_t c;
  GetConfig(&c);
  
  bool bResult = getDS3231DateTime(t);
  
  if (bResult) {
	  *t = *t + (time64_t)(c.timezone_offset * 3600);
  }
  
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

  config_t c;
  
  GetConfig(&c);
  c.century = (tmYearToY2k(tm.Year)/100);
  I2CSerial.printf("setDS3231DateTime() tmYearToY2k(tm.Year)=%d, tm.Year=%d, c.century=%d\n", tmYearToY2k(tm.Year), tm.Year, c.century);
  
  SaveConfig(&c);
  GetConfig(&c);
  I2CSerial.printf("setDS3231DateTime() c.century=%d\n", c.century);
  
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

bool Config::testArg(String arg, int min, int max, int* outval) {
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
