#include "config.h"

Config::Config(I18n* i)
{    
	p_i18n = i; // for file handling

	wl_status_t status = WiFi.status();
	if(status == WL_CONNECTED) {
		Serial.printf("Config: Network '%s' is connected, starting mDNS responder\n", WiFi.SSID().c_str());
	} 
	else {
		Serial.printf("\nCould not connect to WiFi. state='%d'", status); 
		return;
	}

  // TCP server at port 80 will respond to HTTP requests
  p_server = new WifiServer(80);
  
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("lectionary")) {
    Serial.println("Error setting up MDNS responder!");
	return;
  }
  Serial.println("mDNS responder started");
  
  // Start TCP (HTTP) server
  p_server->begin();
  Serial.println("TCP server started");
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  return;
}

Config::~Config() {
  delete p_server;
}

bool Config::ServeClient(void)
{
	wl_status_t status = WiFi.status();
	if(status != WL_CONNECTED) {
		Serial.printf("Network is not connected. state='%d'", status); 
		return false;
	}

  // Check if a client has connected
  WiFiClient client = p_server->available();
  if (!client) {
    return true;
  }
  
  Serial.println("");
  Serial.println("New client");

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
    Serial.print("Invalid request: ");
    Serial.println(req);
    return true;
  }
  req = req.substring(addr_start + 1, addr_end);
  Serial.print("Request: ");
  Serial.println(req);

  String querystring = "";
  int querystring_start = req.indexOf("?");
  if (querystring_start != -1) querystring = req.substring(querystring_start + 1); 
  String filename = req.substring(0, querystring_start);

  Serial.println("request filename: " + filename);
  Serial.println("request querystring: " + querystring);
  
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
    Serial.println("timezone = " + String(c.timezone_offset));
    Serial.println("lectionary = " + String(c.lectionary_config_number));
    String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    client.print(header);
    String line = "{\"tz_offset\":\"" + String(c.timezone_offset) + "\", \"lectionary_config_number\":\"" + String(c.lectionary_config_number) + "\"}"; // output in JSON format
    Serial.println(line);
    client.print(line);
  }
  else if (filename == "/setconf.htm") {
    String tz = getQueryStringParam("timezone", querystring);
    String lect = getQueryStringParam("lectionary", querystring);

    Serial.println("timezone = " + tz);
    Serial.println("lectionary = " + lect);

    bool bresult = SaveConfig(tz, lect);

    Serial.println("SaveConfig returned " + String(bresult?"true":"false"));

    config_t c;
    GetConfig(&c);
    Serial.println("timezone = " + String(c.timezone_offset));
    Serial.println("lectionary = " + String(c.lectionary_config_number));

    b404 = !sendHttpFile(&client, "/html/setconf.htm");
  }
  else {
    b404 = true;
  }

  if (b404) {
    header = "HTTP/1.1 404 Not Found\r\n\r\n";
    client.print(header);
    Serial.println("Sending 404");
  }
      
  delay(1);
  client.stop();
  Serial.println("Done with client");
  return true;
}

String Config::getQueryStringParam(String param, String querystring) {
  int param_offset = querystring.indexOf(param) + param.length() + 1; // include the = after the variable name
  int param_end = querystring.indexOf("&", param_offset);
  if (param_end == -1) param_end = querystring.length();
  return querystring.substring(param_offset, param_end);  
}

bool Config::sendHttpFile(WiFiClient* client, String filename) {
  File file = p_i18n->openFile(filename, FILE_READ);
  if (!file.available()) {
    return false;
  } else {
    String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    client->print(header);
    String line;
    do {
      line = p_i18n->readLine(file) + "\r\n";
      client->print(line);
    } while (file.available());
    p_i18n->closeFile(file);
  }
  return true;
}

bool Config::SaveConfig(String tz, String lect_num) {
  float timezone_offset;
  int lectionary_config_number;
  
  if (IsNumeric(tz)) {
    timezone_offset = atof(tz.c_str());
    Serial.println("timezone_offset=" + String(timezone_offset));
  } 
  else {
    Serial.println("tz is not a number");
    return false;
  }

  if (IsNumeric(lect_num)) {
    lectionary_config_number = atoi(lect_num.c_str());  
    Serial.println("lectionary_config_number=" + String(lectionary_config_number));
  }
  else {
    Serial.println("lectionary config is not a number");
    return false;
  }

  if (!(timezone_offset >= -12 && timezone_offset <= 12)) {
    Serial.println("timezone_offset is out of range: " + String(timezone_offset));
    return false;
  }

  config_t c;
  c.timezone_offset = timezone_offset;
  c.lectionary_config_number = lectionary_config_number;

  storeStruct(&c, sizeof(config_t));
  return true;
}

void Config::GetConfig(config_t* c) {
  loadStruct(c, sizeof(config_t));
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
