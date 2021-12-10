#include "RCConfig.h"
#include "DebugPort.h"
#include "RCGlobals.h"
#include "SD.h"

extern "C" {
#include "user_interface.h"
}

//Config::Config() {}
bool Config::bSettingsUpdated = false;
bool Config::bComplete = false;
String Config::_lang = "default";

bool Config::bHaveNewConfigCsv = false;

bool loadFromSdCard(String path) {
  DEBUG_PRT.print(F("\n\nloadFromSdCard(): ---vvv---"));
  DEBUG_PRT.println(path);

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
  
  DEBUG_PRT.print(F("path="));
  DEBUG_PRT.print(path);
  
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = F("text/html");
  else if(path.endsWith(".css")) dataType = F("text/css");
  else if(path.endsWith(".js")) dataType  = F("application/javascript");
  else if(path.endsWith(".webp")) dataType = F("image/webp");
  else if(path.endsWith(".png")) dataType = F("image/png");
  else if(path.endsWith(".gif")) dataType = F("image/gif");
  else if(path.endsWith(".jpg")) dataType = F("image/jpeg");
  else if(path.endsWith(".ico")) dataType = F("image/x-icon");
  else if(path.endsWith(".xml")) dataType = F("text/xml");
  else if(path.endsWith(".pdf")) dataType = F("application/pdf");
  else if(path.endsWith(".zip")) dataType = F("application/zip");
  else if(path.endsWith(".jsn")) dataType = F("application/json");

  File dataFile = SD.open(path.c_str());
  if(dataFile.isDirectory()){
	dataFile.close();
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
	DEBUG_PRT.print(F(" - isDirectory=true: modified path="));
  	DEBUG_PRT.print(path);
  }

  DEBUG_PRT.print("\nRequested file: ");
  DEBUG_PRT.println(path);

  if (!dataFile) {
	DEBUG_PRT.println(F("...Couldn't open file."));
    return false;
  }

  if (server.hasArg("download")) dataType = "application/octet-stream";

  //#ifndef CORE_v3_EXPERIMENTAL
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  //#else
  //if (streamFile(dataFile, dataType) != dataFile.size()) {
  //#endif
    DEBUG_PRT.println(F("Sent less data than expected!"));
  }

  dataFile.close();
  DEBUG_PRT.println(F("...got file"));

  Config::bComplete = (Config::bHaveNewConfigCsv && Config::bSettingsUpdated /*&& path == "/html/lang/" + Config::_lang + ".json"*/); // PLL-15-12-2020 can check Config::bComplete reduce delay to reboot after settings updated

  DEBUG_PRT.print(F("\nbSettingsUpdated="));
  DEBUG_PRT.print(Config::bSettingsUpdated);
  DEBUG_PRT.print(F(", path=["));
  DEBUG_PRT.print(path);
  DEBUG_PRT.print(F(", Config::_lang=["));
  DEBUG_PRT.print(Config::_lang);
  DEBUG_PRT.print(F("], bComplete="));
  DEBUG_PRT.println(Config::bComplete);

  DEBUG_PRT.println(F("^^^loadFromSdCard()----^^^\n\n"));

  return true;
}

#ifdef CORE_v3_EXPERIMENTAL
// workaround for v3 server.streamFile() not working
size_t streamFile(File& file, String& contentType) {
    //if (bUseHTTPServerWorkarounds) {
      char buf[128];                              // streamFile workaround
      int siz = file.size();
      int out = siz;
      while(siz > 0) {
        size_t len = std::min((int)(sizeof(buf) - 1), siz);
        file.read((uint8_t *)buf, len);
        server.client().write((const char*)buf, len);
        siz -= len;
      }  
      return out-siz; // number of bytes transmitted
    //}
    
    //return server.streamFile(file, contentType);  // streamFile workaround not needed
}
#endif

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
	DEBUG_PRT.print(message);
}


void handleSettingsJson() {
    config_t c = {0};
    Config::GetConfig(c);
	
    DEBUG_PRT.print(F("timezone = "));
	DEBUG_PRT.println(String(c.data.timezone_offset));
    DEBUG_PRT.print(F("lectionary = "));
	DEBUG_PRT.println(String(c.data.lectionary_config_number));
    DEBUG_PRT.print(F("contrast = "));
	DEBUG_PRT.println(String(c.data.epd_contrast));
	
    String line = "{\"tz_offset\":\"" + String(c.data.timezone_offset) + 
				   "\", \"lectionary_config_number\":\"" + String(c.data.lectionary_config_number) + 
				   "\", \"contrast\":\"" + String(c.data.epd_contrast) + 
				   "\", \"lang\":\"" + Config::_lang + 
				   "\"}"; // output in JSON format
				   
    server.send(200, "application/json", line);
    DEBUG_PRT.println(line);
}


File fsUploadFile;              // a File object to temporarily store the received file

void handleFileUpload(){ // upload a new file to the SPIFFS
  //File fsUploadFile;              // a File object to temporarily store the received file (module/global)
  	HTTPUpload& upload = server.upload();
  	if(upload.status == UPLOAD_FILE_START)
	{
		if (fsUploadFile) 
		{
			DEBUG_PRT.println(F("Already uploading a file, only 1 upload file supported at a time!"));
	    	server.send(500, "text/plain", "500: only 1 file can be uploaded at a time");
			return;
		}

    	String filename = upload.filename;
    	if(!filename.startsWith("/")) filename = "/"+filename;
    	DEBUG_PRT.print(F("handleFileUpload Name: ")); Serial.println(filename);
    	fsUploadFile = SD.open(filename, "w");            // Open the file for writing in SD (create if it doesn't exist)
    	filename = String();
  	} 
	
	else if(upload.status == UPLOAD_FILE_WRITE) 
	{
    	if(fsUploadFile) 
		{
      		fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
		}
  	} 
	
	else if(upload.status == UPLOAD_FILE_END) 
	{
    	if(fsUploadFile) 
		{                                    // If the file was successfully created
      		fsUploadFile.close();                               // Close the file again
      		DEBUG_PRT.print(F("handleFileUpload Size: ")); Serial.println(upload.totalSize);
      		server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      		server.send(303);
		}
    } 
	
	else 
	{
      	server.send(500, "text/plain", "500: couldn't create file");
	 	if (fsUploadFile) 
		{
			fsUploadFile.close();
	  	}
    }
}

void handleConfigFileUpload() { // upload a new file to the SPIFFS
	DEBUG_PRT.println("handleConfigFileUpload()");

	String config_csv = F(CONFIG_CSV);
	String config_csv_new = config_csv + F(".new");
	String config_csv_old = config_csv + F(".old");

  	//File fsUploadFile;              // a File object to temporarily store the received file (module/global)
  	HTTPUpload& upload = server.upload();
  	if(upload.status == UPLOAD_FILE_START) 
	{
		DEBUG_PRT.println(F("UPLOAD_FILE_START"));
		
		if (fsUploadFile) 
		{
			DEBUG_PRT.println(F("Already uploading a file, only 1 upload file supported at a time!"));
	    	server.send(500, "text/plain", "500: only 1 file can be uploaded at a time");
			return;
		}

    	String filename = upload.filename;
    	
		if(!filename.startsWith("/")) 
		{
			filename = "/" + filename;
		}

    	DEBUG_PRT.print(F("handleConfigFileUpload Name: ")); 
		DEBUG_PRT.println(filename);
    	
		if (filename == config_csv) {
			SD.remove(config_csv_new);
			fsUploadFile = SD.open(config_csv_new, "w");            // Open the file for writing in SD (create if it doesn't exist)
    		//filename = String();
		}
  	} 

	else if(upload.status == UPLOAD_FILE_WRITE) 
	{
		DEBUG_PRT.println(F("UPLOAD_FILE_WRITE"));
    	if(fsUploadFile) 
		{
      		fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
		}
  	} 

	else if(upload.status == UPLOAD_FILE_END) 
	{
		DEBUG_PRT.println(F("UPLOAD_FILE_END"));
    	if(fsUploadFile) {                                    // If the file was successfully created
      		fsUploadFile.close();                               // Close the file again
      		
			DEBUG_PRT.print(F("handleConfigFileUpload Size: ")); 
	  		DEBUG_PRT.println(upload.totalSize);
	  		DEBUG_PRT.println(F("Attempting to commit new config.csv:"));
	  		//SD.remove(config_csv_old); // is done by copy function
	  		SD.remove(config_csv_old);
	  		copyfile(config_csv, config_csv_old);
	  		SD.remove(config_csv);

	  		if (!copyfile(config_csv_new, config_csv)) 
			{
	  			DEBUG_PRT.print(F("Couldn't copy ")); 
				DEBUG_PRT.print(config_csv_new); 
				DEBUG_PRT.print(F(" to ")); 
				DEBUG_PRT.print(config_csv); 
				DEBUG_PRT.print(F(" - trying to fall back to old file"));
				
				if(copyfile(config_csv_old, config_csv)) 
				{
	  	  			DEBUG_PRT.println(F("..Success"));
					server.send(500, "text/plain", "500: File uploaded, but Lectionary could not copy the file to the destination config.csv, old config.csv is active");
				}
				else 
				{
					DEBUG_PRT.println(F("PANIC - FAILED -- There will be no config file on SD card!"));
					server.send(500, "text/plain", "500: File uploaded, but Lectionary could not copy the file to the destination config.csv, Panic - NO CONFIG is active!");
	    		}
	  		}
			else 
			{
				DEBUG_PRT.println(F("File uploaded, new config.csv active"));
		  		server.send(200, "text/plain", "200: OK");
		  		Config::bHaveNewConfigCsv = true; // will trigger a rename of the new config.csv.new file to config.csv when the settings have also successfully been received.
			}
	  		//server.sendHeader("Location","/setdate.htm");      // Redirect the client to the success page
      		//server.send(303);
    	}
	}
	else 
	{
		DEBUG_PRT.println(F("Error uploading file: Sending Status:500"));
      	server.send(500, "text/plain", "500: couldn't create file");
	  	if (fsUploadFile) 
		{
		  	fsUploadFile.close();
	  	}
    }
}

void handleSetConf() {
    String tz = getQueryStringParam("timezone", ""); // "timezone", "0"
    String lect = getQueryStringParam("lectionary", ""); // "lectionary", "0"	
	String epdcontrast = getQueryStringParam("contrast", "");
	
	String debug_mode = getQueryStringParam("debug", "");
	
    DEBUG_PRT.print(F("timezone = "));
	DEBUG_PRT.println(String(tz));
    DEBUG_PRT.print(F("lectionary = "));
	DEBUG_PRT.println(String(lect));
    
	if (debug_mode != "") {
		DEBUG_PRT.print(F("debug = "));
		DEBUG_PRT.println(String(debug_mode));
	}
	
//    bool bresult = SaveConfig(tz, lect, debug_mode);
//
//    DEBUG_PRT.println("SaveConfig returned " + String(bresult?"true":"false"));
		
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

		bresult = Config::SaveConfig(tz, lect, debug_mode, epdcontrast, dststartmonth, dststartday, dststarthour, dstendmonth, dstendday, dstendhour, dstoffset);
	}
	else {
		bresult = Config::SaveConfig(tz, lect, debug_mode, epdcontrast);
	}
	
    DEBUG_PRT.print(F("SaveConfig returned "));
	DEBUG_PRT.println( bresult ? F("true") : F("false") );
	
    config_t c = {0};
    Config::GetConfig(c);
    DEBUG_PRT.print(F("timezone = "));
	DEBUG_PRT.println(String(c.data.timezone_offset));
    DEBUG_PRT.print(F("lectionary = "));
	DEBUG_PRT.println(String(c.data.lectionary_config_number));

	// remove config.csv.old, rename config.csv to config.csv.old and rename /config.csv.new to config.csv
	// the index.htm configuration webpage should already have uploaded the new config.csv, via an xhr request.
	/*
	if (Config::bHaveNewConfigCsv) {
		DEBUG_PRT.println(F("Attempting to commit new config.csv:"));
		String config_csv = F(CONFIG_CSV);
		String config_csv_new = config_csv + F(".new");
		String config_csv_old = config_csv + F(".old");

		//SD.remove(config_csv_old); // is done by copy function
		copyfile(config_csv, config_csv_old);
		if (copyfile(config_csv_new, config_csv)) {
		  	DEBUG_PRT.print(F("Couldn't copy ")); DEBUG_PRT.print(config_csv_new); DEBUG_PRT.print(F(" to ")); DEBUG_PRT.print(config_csv); DEBUG_PRT.print(F(" - trying to fall back to old file"));
		  	if(copyfile("/config.csv.old", "/config.csv")) {
				DEBUG_PRT.println(F("..Success"));
		  	}
		  	else {
			  	DEBUG_PRT.println(F("PANIC - FAILED -- There will be no config file on SD card!"));
		  	}
	  	}
	}
	*/
	Config::bSettingsUpdated = true;
	

	bool b404 = !loadFromSdCard("setconf.htm");
	
	if (b404) {
		server.send(404, "text/plain", "Settings updated (setconf.htm not found)");
		DEBUG_PRT.println(F("Sending 404"));
	}
}

bool copyfile(String fromFile, String toFile) {
	DEBUG_PRT.print(F("copyfile() Copying file from "));
	DEBUG_PRT.print(fromFile);
	DEBUG_PRT.print(F(" to "));
	DEBUG_PRT.println(toFile);

	if (SD.exists(toFile)) {
    	SD.remove(toFile);
  	}

  	File fileIn = SD.open(fromFile, FILE_READ);
	if (!fileIn) {
		return false;
	}

  	File fileOut = SD.open(toFile, FILE_WRITE);

	if (!fileOut) {
		fileIn.close();
		return false;
	}

	uint8_t buf[128];
	const int buf_size = 128;
	long bytes_remaining = 0;

	if (fileIn.available()) {
		bytes_remaining = fileIn.size();
	}

  	while (fileIn.available() && bytes_remaining > 0) {
    	long bytes_to_read = bytes_remaining < buf_size ? bytes_remaining : buf_size;
		fileIn.read(buf, bytes_to_read);
		fileOut.write(buf, bytes_to_read);
		bytes_remaining -= buf_size;
  	}
  	fileIn.close();
  	fileOut.close();
	
	return true;
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



bool Config::StartServer(String lang) {
	//bUseHTTPServerWorkarounds = String(system_get_sdk_version()).startsWith("2.2.2-dev");
  	//DEBUG_PRT.print(bUseHTTPServerWorkarounds ? F("U") : F("Not U"));
  	//DEBUG_PRT.println(F("sing streamFile workaround"));

	bSettingsUpdated = false;
	bComplete = false;
	
	bHaveNewConfigCsv = false;

	wl_status_t status = WiFi.status();
	if(status == WL_CONNECTED) {
		DEBUG_PRT.print(F("Config: Network '"));
		DEBUG_PRT.print(WiFi.SSID());
		DEBUG_PRT.println(F("' is connected, starting mDNS responder"));
	} 
	else {
		DEBUG_PRT.print(F("\nCould not connect to WiFi. state="));
		DEBUG_PRT.println(String(status)); 
		WiFi.printDiag(DEBUG_PRT);
		return false;
	}
  
	// Set up mDNS responder:
	// - first argument is the domain name, in this example
	//   the fully-qualified domain name is "esp8266.local"
	// - second argument is the IP address to advertise
	//   we send our IP address on the WiFi network
	
	////TODO: PLL-23-07-2020 Put this code back in when a working version of the mDNS library is made available
	// PLL-23-07-2020 mDNS not working in latest version
	//
	//if (!MDNS.begin("lectionary", WiFi.localIP())) {
	//	DEBUG_PRT.println(F("Error setting up MDNS responder!"));
	//	return false;
	//}
	//
	// Add service to MDNS-SD
	//MDNS.addService("http", "tcp", 80);
	//
	//DEBUG_PRT.println(F("mDNS responder started"));
  	server.on ("/uploadconfig.htm", HTTP_POST,                       // if the client posts to the upload page
    	[](){ server.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
    	handleConfigFileUpload                                    // Receive and save the file
  	);

	// allows files to be uploaded to any part of the SD card - flexible, but potentially a risk
  	server.on ("/upload.htm", HTTP_POST,                       // if the client posts to the upload page
    	[](){ server.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
    	handleFileUpload                                    // Receive and save the file
  	);


	server.on ( "/settings.json", handleSettingsJson );
	server.on ( "/setconf.htm", handleSetConf );
	server.onNotFound ( handleNotFound );

	if (lang != "" and SD.exists("/html/lang/" + lang + ".json")) {
		Config::_lang = lang;
	}
	else {
		Config::_lang = "default";
	}

	DEBUG_PRT.print(F("Config::_lang is set to ["));
	DEBUG_PRT.print(Config::_lang);
	DEBUG_PRT.println(F("]"));

	server.begin();
	DEBUG_PRT.println (F("HTTP server started"));
  
	return true;	
}

void Config::StopServer( void ) {
	server.close();
}

//Config::~Config() {
//}


bool Config::SaveConfig(String tz, String lect_num, String debug_mode, String epdcontrast) {
	return SaveConfig(tz, lect_num, debug_mode, epdcontrast, 0, 0, 0, 0, 0, 0, "0.0");
}


bool Config::SaveConfig(String tz, String lect_num, String debug_mode, String epdcontrast,
						uint32_t dstStartMonth, uint32_t dstStartDay, uint32_t dstStartHour,
						uint32_t dstEndMonth,   uint32_t dstEndDay,   uint32_t dstEndHour,
						String dstoffset) 
{
  float timezone_offset = 0.0;
  int lectionary_config_number = 0;
  uint16_t lectionary_epd_contrast = 0;
  uint8_t debug_flags = 0;
  bool debug_not_set = true;
  
  float dst_offset = 0.0;
  
  bool bGotTz = false;
  bool bGotLectNum = false;
  bool bGotContrast = false;
  
  if (tz != "") {
	  if (IsNumeric(tz)) {
		timezone_offset = atof(tz.c_str());
		bGotTz = true;
		DEBUG_PRT.print(F("timezone_offset="));
		DEBUG_PRT.println(String(timezone_offset));
	  } 
	  else {
		DEBUG_PRT.println(F("tz is not a number"));
		return false;
	  }
  }
  
  if (lect_num != "") {
	  if (IsNumeric(lect_num)) {
		lectionary_config_number = atoi(lect_num.c_str());  
		bGotLectNum = true;
		DEBUG_PRT.print(F("lectionary_config_number="));
		DEBUG_PRT.println(String(lectionary_config_number));
	  }
	  else {
		DEBUG_PRT.println(F("lectionary config is not a number"));
		return false;
	  }
  }  
  
  if (debug_mode == "1" || debug_mode == "true") { // flags are bitwise: 1=I2C debug, 2=file debug, 3=both, true=I2C debug
	  debug_not_set = false;
	  debug_flags |= DEBUG_FLAGS_I2CPORT;
  } 
  else if (debug_mode == "2") {
	  debug_not_set = false;
	  debug_flags |= DEBUG_FLAGS_FILEPORT;	  
  }
  else if (debug_mode == "3") {
	  debug_not_set = false;
	  debug_flags |= DEBUG_FLAGS_I2CPORT;	  
	  debug_flags |= DEBUG_FLAGS_FILEPORT;	  
  }
  else if (debug_mode == "0" || debug_mode == "false") {
	  debug_not_set = false;
	  debug_flags = 0;
  }
  
  if (epdcontrast != "") {
	  if (IsNumeric(epdcontrast)) {
		lectionary_epd_contrast = atoi(epdcontrast.c_str());  
		if (lectionary_epd_contrast < 1) lectionary_epd_contrast = 1;
		if (lectionary_epd_contrast > 7) lectionary_epd_contrast = 7;
		
		bGotContrast = true;
		DEBUG_PRT.print(F("lectionary_epd_contrast="));
		DEBUG_PRT.println(String(lectionary_epd_contrast));
	  }
	  else {
		DEBUG_PRT.println(F("epdcontrast is not a number"));
		return false;
	  }
  }
  
  if (bGotTz && bGotLectNum) {
	  if (!(timezone_offset >= -12.0 && timezone_offset <= 12.0)) {
		DEBUG_PRT.print(F("timezone_offset is out of range: "));
		DEBUG_PRT.println(String(timezone_offset));
		return false;
	  }

	  if (IsNumeric(dstoffset)) {
		dst_offset = atof(dstoffset.c_str());
		DEBUG_PRT.print(F("dst_offset="));
		DEBUG_PRT.println(String(dst_offset));
	  } 
	  else {
		DEBUG_PRT.println(F("dst_offset is not a number"));
		return false;
	  }

	  if (!(dst_offset >= 0.0 && dst_offset <= 3.0)) {
		DEBUG_PRT.print(F("dst_offset is out of range: "));
		DEBUG_PRT.println(String(dst_offset));
		return false;
	  }
  }
  
  config_t c = {0};
  GetConfig(c); // load config as it is and update, so when it is written back it doesn't destroy other members which have not changed

  if (!debug_not_set) {
	c.data.debug_flags = debug_flags;
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

  if(bGotContrast) {
	  c.data.epd_contrast = lectionary_epd_contrast;
  }
  
  SaveConfig(c);
  
  dump_config(c);
  
  return true;
}

uint16_t Config::GetEPDContrast() {
  config_t c = {0};
  if (GetConfig(c)) {
	return c.data.epd_contrast;		//7 = lowest contrast, 1 refreshes, 1 = highest contrast, 7 refreshes
  }
  else {
	  return 7;
  }
}

void Config::dump_config(config_t& c) {
	DEBUG_PRT.print(F("c.data.timezone_offset:\t")); 			DEBUG_PRT.println(String(c.data.timezone_offset));
	DEBUG_PRT.print(F("c.data.lectionary_config_number:\t")); 	DEBUG_PRT.println(String(c.data.lectionary_config_number));
//	DEBUG_PRT.print(F("c.data.debug_on:\t")); 					DEBUG_PRT.println(String(c.data.debug_on));
	DEBUG_PRT.print(F("c.data.debug_flags:\t")); 				DEBUG_PRT.println(String(c.data.debug_flags));
	DEBUG_PRT.print(F("c.data.dst_offset:\t")); 				DEBUG_PRT.println(String(c.data.dst_offset));
	DEBUG_PRT.print(F("c.data.dst_start_month:\t")); 			DEBUG_PRT.println(String(c.data.dst_start_month));
	DEBUG_PRT.print(F("c.data.dst_start_day:\t")); 				DEBUG_PRT.println(String(c.data.dst_start_day));
	DEBUG_PRT.print(F("c.data.dst_start_hour:\t")); 			DEBUG_PRT.println(String(c.data.dst_start_hour));
	DEBUG_PRT.print(F("c.data.dst_end_month:\t")); 				DEBUG_PRT.println(String(c.data.dst_end_month));
	DEBUG_PRT.print(F("c.data.dst_end_day:\t")); 				DEBUG_PRT.println(String(c.data.dst_end_day));
	DEBUG_PRT.print(F("c.data.dst_end_hour:\t")); 				DEBUG_PRT.println(String(c.data.dst_end_hour));
	if (!DstIsValid(c)) {
		DEBUG_PRT.print(F("DST settings invalid"));
	}
	DEBUG_PRT.print(F("c.data.epd_contrast:\t")); 				DEBUG_PRT.println(String(c.data.epd_contrast));
	
	DEBUG_PRT.print(F("\nc.crc32:\t")); 						DEBUG_PRT.println(String(c.crc32, HEX));
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
	DEBUG_PRT.println(F("Invalidating EEPROM to oblige user to reconfigure time/dst settings etc"));
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
  
  DEBUG_PRT.println(F("Config: EEPROM checksum wrong or uninitialized"));
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

  bool ok = false;
  config_t c = {0};

  if (ClockStopped(ok) && ok) {		//PLL 28-12-2018
	clockwasreset = true;
	return false;
  }
  
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
  
  DEBUG_PRT.printf("DS3231 Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", date, month, year, hour, minute, second);

  //DEBUG_PRT.printf("-1-");
  
  int century = 0;
    
  if (GetConfig(c)) {
	if (century_overflowed) {
	  DEBUG_PRT.println(F("Century overflowed"));
	  c.data.century++; // century rolled over since last call, so save new century
	  SaveConfig(c);
	  
	  bool ok = false;
	  int retries = 10;
	  while (!ok && --retries > 0) {
		ok = ClearCenturyFlag();
	  }
	  
	  if (!ok) {
		  DEBUG_PRT.println(F("Century overflowed and was unable to reset the flag: century setting may be wrong on next call - connect power and use Lectionary config website to reset it"));
	  }
	  else {
		  DEBUG_PRT.println(F("Century overflowed, flag in clock chip has been reset and century value incremented in EEPROM"));		  
	  }
	}
	
	century = c.data.century;  
  }
  
//  DEBUG_PRT.printf("-2-");
  
  tmElements_t tm;
  
  tm.Second = second;
  tm.Minute = minute;
  tm.Hour = hour;
  tm.Day = date;
  tm.Month = month;
  tm.Year = y2kYearToTm(year + (century * 100));

//  DEBUG_PRT.printf("-3- year=%d, century=%d, year= %d\n", year, century, tm.Year);
  
  bool b_incorrectleapyear = false;
  
  if (tm.Year % 100 == 0 && tm.Year % 400 != 0 && tm.Month == 2 && tm.Day == 29) { // DS3231 is not designed to handle leap years correctly from 2100 (inclusive).
	tm.Day = 1;																	   // Guessing this is because it doesn't know that leap years occur on century boundaries 
	tm.Month = 3;																   // if the year is divisible by 400 as well as 100
	b_incorrectleapyear = true;	 												   // So it will incorrectly treat 2100 as a leap year, when it is not (2400 will be)
 }
  
//  DEBUG_PRT.printf("%02d/%02d/%04d %02d:%02d:%02d\n", tm.Day, tm.Month, tm.Year, tm.Hour, tm.Minute, tm.Second);
//  DEBUG_PRT.printf("-3-");
  *t = makeTime(tm);
//  DEBUG_PRT.printf("-4-");
  
  if (b_incorrectleapyear) {
	  DEBUG_PRT.print(F("Incorrect Leap Year detected from DS3231 - Feb 29 skipped, setting date to 1 March "));
	  DEBUG_PRT.println(String(tm.Year));
	  setDateTime(*t);
  }
 
  DEBUG_PRT.printf("UTC Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", tm.Day, tm.Month, tmYearToCalendar(tm.Year), tm.Hour, tm.Minute, tm.Second);
  
  if (*t < 3600 * 24 * 365) return false; // need some overhead. The liturgical calendar starts in 1970 (time_t value == 0), but the first season (Advent) begins in 1969, 
											// which is outside the range of a time64_t value, and will occur in calculations for year 1970 if not trapped

  
  //DEBUG_PRT.printf("-4-");

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
			//DEBUG_PRT.printf("(int)(c.data.timezone_offset * 3600.0)=%d\n", (int)(c.data.timezone_offset * 3600.0));
			*t += (int)(c.data.timezone_offset * 3600.0);	  	
	  
			breakTime(*t, ts);
			// very basic DST support. DST start and end dates are provided by Javascript calculations in the config.htm web page, but in some jurisdictions these dates change from year to 
			// year, for example the change is made on the last Sunday of a month rather than the same date each year. In these jurisdictions, the DST compensation will likely be wrong in the 
			// years after the year in which the config.htm page was last used to set the time. This is easily corrected though by using the configuration interface again - this will need to
			// done at least once per year in order to keep the DST start and end dates current. (No user input is required in the configuration page, just click on the form submit button and 
			// the new start and end dates for DST will automatically be recorded.)
			//
			dump_config(c);
			
			if (c.data.dst_offset != 0.0) {	
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
					if (c.data.dst_end_month < ts.Month && ts.Month >= c.data.dst_start_month && ts.Day >= c.data.dst_start_day) { // ts.Month will be >= dst start month during dst, which is when the end of dst is in the 
						dst_end.Year++; // end dst month is in next year						 //	next year
						DEBUG_PRT.printf("dst_end is in next year (southern hemisphere)\n");
					}
				}
				else if (c.data.dst_start_month < c.data.dst_end_month) { // in northern hemisphere
					if (c.data.dst_start_month < ts.Month && ts.Month >= c.data.dst_end_month && ts.Day >= c.data.dst_end_day) {
						dst_start.Year++; // start dst month is in next year
						DEBUG_PRT.printf("dst_start is in next year (northern hemisphere)\n");
					}			
				}
				
				time64_t t_dst_start = makeTime(dst_start);
				time64_t t_dst_end = makeTime(dst_end);
				
				if (*t > t_dst_start && *t < t_dst_end) { // dst in effect if so, so add it on
					
					*t += (int)(c.data.dst_offset * 3600.0);	  
					*isdst = true;
					
					DEBUG_PRT.println(F("DST in effect"));
				}
				else {
					*isdst = false;

					DEBUG_PRT.println(F("Standard Time in effect"));
				}
			}
			else {
				*isdst = false;
				DEBUG_PRT.println(F("DST is not required in this locale"));				
			}
		}
	}
	else {
		*isdst = false; // dst settings invalid
	}
	
	breakTime(*t, ts);
	DEBUG_PRT.printf("Datetime = %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);

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
	DEBUG_PRT.print(F("\nsetDateTime(): bcode="));
	DEBUG_PRT.println(bcode);
    return false;
  }

  config_t c = {0};
  
  if (GetConfig(c)) {
	c.data.century = (tmYearToY2k(tm.Year)/100);
	DEBUG_PRT.printf("setDateTime() tmYearToY2k(tm.Year)=%d, tm.Year=%d, c.data.century=%d\n", tmYearToY2k(tm.Year), tm.Year, c.data.century);
  
	SaveConfig(c);
	GetConfig(c);
	DEBUG_PRT.print(F("setDateTime() c.data.century="));
	DEBUG_PRT.println(String(c.data.century));
  }
  return true;
}

// input time64_t value is local time, so it must be adjusted to subtract both the timezone offset and DST (if applicable)
bool Config::setAlarmLocalTime(time64_t t, uint8_t alarm_number, uint8_t flags, bool enable_alarm) {
	config_t c = {0};
	if (Config::GetConfig(c)) {
		t -= (int)(c.data.timezone_offset * 3600.0); // subtract timezone offset from local time
		 
		if (DstIsValid(c)) {
			if (isDST()) t -= dstOffset();			 // and subtract the dst offset (if applicable)
		}
		else {
			DEBUG_PRT.println(F("Config::setAlarmLocalTime() DST setting is not valid, so not adjusting alarm for local DST"));			
		}
	}
	else {
		DEBUG_PRT.println(F("Config::setAlarmLocalTime() couldn't read config, so not adjusting alarm for local time"));
	}
	
	return setAlarm(t, alarm_number, flags, enable_alarm);
}


bool Config::setAlarm(time64_t t, uint8_t alarm_number, uint8_t flags, bool enable_alarm) {
	//if (isDST()) t-=dstOffset();

	tmElements_t tm;
	breakTime(t, tm);

	uint8_t bcode = 0; 

	if (alarm_number == 1) {
		uint8_t buf[5];
		buf[0] = DS3231_ALARM1_ADDR; // start at register 0x07 in the ds3231
		buf[1] = (dec2bcd(tm.Second));
		buf[2] = (dec2bcd(tm.Minute));
		buf[3] = (dec2bcd(tm.Hour));      // sets 24 hour format
		buf[4] = (dec2bcd(tm.Day));		  // also sets DY/DT = 0 -> Match Day of Month, and A1M4 to 0 (Default for flags == A1_MATCH_DATE_AND_TIME (==0))
		
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
		case A2_MATCH_DAY_AND_TIME_MINS: 	//25-11-2018 was A1_MATCH_DAY_AND_TIME:
			buf[3] = (dec2bcd(tm.Wday));   
			buf[3] |= 0x40;	// set DY/DT = 1 -> Match Weekday, A2M2 = A2M3 = A2M4 = 0
			break;
		
		case A2_ONCE_PER_MINUTE: 			//25-11-2018 was A1_ONCE_PER_SECOND:
			buf[1] |= 0x80;	// set A2M2, A2M3 and A2M4, alarm once per minute (there is no A2M1!)
			buf[2] |= 0x80;
			buf[3] |= 0x80;

		case A2_MATCH_MINUTES: 				//25-11-2018 was A1_MATCH_SECONDS:
			buf[2] |= 0x80; // set A2M3 and A2M4, match minutes
			buf[3] |= 0x80;
			break;
		
		case A2_MATCH_HOURS_MINUTES:		//25-11-2018 was A1_MATCH_MINUTES_SECONDS:
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
    DEBUG_PRT.println(F("Read: "));
    //printMemory();
    //DEBUG_PRT.println("number of hours to next reading: " + String(rtcData.data.wake_hour_counter)); // set to true if the reading should be made this hour (read on waking)
    //DEBUG_PRT.println("card displayed: " + String(rtcData.data.dcs)); // value shows if any of the error cards are displayed, so that, if the condition has not changed, it need not be redisplayed
    
    DEBUG_PRT.println();
    uint32_t crcOfData = calculateCRC32((uint8_t*) (uint8_t*)&rtcData.data, sizeof(rtcData.data));
    DEBUG_PRT.print(F("CRC32 of data: "));
    DEBUG_PRT.println(crcOfData, HEX);
    DEBUG_PRT.print(F("CRC32 read from RTC: "));
    DEBUG_PRT.println(rtcData.crc32, HEX);
    
	if (crcOfData != rtcData.crc32) {
      DEBUG_PRT.println(F("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!"));
	  return false;
    } 
    
	DEBUG_PRT.println(F("CRC32 check ok, data is probably valid."));
	return true;
  }
  
  return false;
}

void Config::writeRtcMemoryData(rtcData_t& rtcData) {
	  // Update CRC32 of data
  rtcData.crc32 = calculateCRC32((uint8_t*) &rtcData.data, sizeof(rtcData.data));
  // Write struct to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    DEBUG_PRT.println(F("Write: "));
    printMemory(rtcData);
	DEBUG_PRT.println();
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
    DEBUG_PRT.print(buf);
    if ((i + 1) % 32 == 0) {
      DEBUG_PRT.println();
    } else {
      DEBUG_PRT.print(F(" "));
    }
  }
  DEBUG_PRT.println();
}

wake_reasons Config::Wake_Reason() {
	int retries = 50;
  
	bool ok1 = false;
	bool triggered_a1 = false;
  
	DEBUG_PRT.println("Checking if triggered by Alarm 1");
	while (!ok1 && --retries > 0) {
		triggered_a1 = Config::DS3231_triggered_a1(ok1);
		delay(50);
	}
	DEBUG_PRT.print("Finished checking if triggered by Alarm 1, retries = ");
	DEBUG_PRT.println(retries);

	bool ok2 = false;
	bool triggered_a2 = false;

	DEBUG_PRT.println("Checking if triggered by Alarm 2");
	while (!ok2 && --retries > 0) {
		triggered_a2 = Config::DS3231_triggered_a2(ok2);
		delay(50);
	}
	DEBUG_PRT.print("Finished checking if triggered by Alarm 2, retries = ");
	DEBUG_PRT.println(retries);

	if (triggered_a1) {
		DEBUG_PRT.println(F("Woken by Alarm 1"));
		return WAKE_ALARM_1;
	}
  
	if (triggered_a2) {
		DEBUG_PRT.println(F("Woken by Alarm 2"));
		return WAKE_ALARM_2;
	}

    rst_info *rinfo;
    rinfo = ESP.getResetInfoPtr();

    if ((*rinfo).reason == REASON_DEEP_SLEEP_AWAKE) { // only check the hour count to the next reading if we awoke because of the deepsleep timer
		DEBUG_PRT.print(F("ResetInfo.reason = ")); 
		DEBUG_PRT.println(String((*rinfo).reason));		
		return WAKE_DEEPSLEEP;
	}
   
	if (!(triggered_a1 || triggered_a2)) {
		DEBUG_PRT.println(F("Woken by USB 5volts"));
		return WAKE_USB_5V;
	}
	
	DEBUG_PRT.println(F("Woken by Unknown"));
	return WAKE_UNKNOWN;
}


bool Config::PowerOff(time64_t wake_datetime) {
	if (wake_datetime != 0) {
		if (Config::setAlarmLocalTime(wake_datetime, 1, A1_MATCH_DATE_AND_TIME, true)) { // wake alarm with A1_MATCH_DAY_AND_TIME allows sleep time of max. 1 week. Now using A1_MATCH_DATE_AND_TIME, should give 1 year.
			DEBUG_PRT.print(F("Alarm 1 set"));
			tmElements_t ts;
			breakTime(wake_datetime, ts);
			DEBUG_PRT.printf(" for %02d/%02d/%04d %02d:%02d:%02d\n", ts.Day, ts.Month, tmYearToCalendar(ts.Year), ts.Hour, ts.Minute, ts.Second);
			delay(200);
		}
		else {
			DEBUG_PRT.print(F("Problem setting Alarm 1"));
			return false;
		}
	}
	else {
		DEBUG_PRT.print(F("Not setting alarm, power off until USB5V connected"));
	}
    
	bool okCleared_a1 = false;
	bool okCleared_a2 = false;

	int retries = 50;
  
	DEBUG_PRT.println("Clearing A1F");
	while(!okCleared_a1 && --retries > 0) {
		okCleared_a1 = Config::DS3231_clear_a1f();
		delay(50);
	}
  
	//might not get here if A1F was just cleared, since clearing it removes power from ESP8266
  
	DEBUG_PRT.println(F("Clearing A2F"));
	while(!okCleared_a2 && --retries > 0) {
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
		return Config::setAlarmLocalTime(t+3, 1, A1_MATCH_DAY_AND_TIME, true); // this will go off in three seconds, and assert the /res line. This will hold up the ENable pin on the 3.3V LDO and keep the ESP on until the alarm is set to the wakeup time.
	}
	
	return false;
}