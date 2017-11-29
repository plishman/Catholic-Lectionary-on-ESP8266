#include "Network.h"

bool Network::connect() {
	Serial.printf("\nTry connecting to WiFi with SSID '%s'\n", WiFi.SSID().c_str());

	WiFi.mode(WIFI_STA);
	WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM, last saved credentials
	while (WiFi.status() == WL_DISCONNECTED) {
		delay(500);
		Serial.print(".");
	}

	wl_status_t status = WiFi.status();
	if(status == WL_CONNECTED) {
		Serial.printf("\nConnected successfull to SSID '%s'\n", WiFi.SSID().c_str());
	} 
	else {
		Serial.printf("\nCould not connect to WiFi. state='%d'", status); 
		return false;
		
		//Serial.println("Please press WPS button on your router.\n Press any key to continue...");
		//bool connected = false;
		//wdt_reset();
		//delay(10000);
		//connected = startWPSPBC();			
		//
		//if (!connected) {
		//	Serial.println("Failed to connect with WPS :-(");  
		//	return false;
	}
	return true;
}

bool Network::startWPSPBC() {
  Serial.println("WPS config start");
  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) {
      // Well this means not always success :-/ in case of a timeout we have an empty ssid
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) {
        // WPSConfig has already connected in STA mode successfully to the new station. 
        Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
      } else {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}

bool Network::get_ntp_time(time_t* t)
{
	wl_status_t status = WiFi.status();
	if(status != WL_CONNECTED) {
		return false;
    }
	
	udp.begin(localPort);                          // Start listening for UDP messages on port 123
	//get a random server from the pool
	WiFi.hostByName(ntpServerName, timeServerIP); 

	Serial.println(timeServerIP);

	sendNTPpacket(timeServerIP); // send an NTP packet to a time server
	// wait to see if a reply is available

	int cb;
	delay(1000);
	cb = udp.parsePacket();
	if (!cb) return false;

	Serial.print("packet received, length=");
	Serial.println(cb);
	// We've received a packet, read the data from it
	udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

	//the timestamp starts at byte 40 of the received packet and is four bytes,
	// or two words, long. First, esxtract the two words:

	unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
	unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
	// combine the four bytes (two words) into a long integer
	// this is NTP time (seconds since Jan 1 1900):
	unsigned long secsSince1900 = highWord << 16 | lowWord;
	Serial.print("Seconds since Jan 1 1900 = " );
	Serial.println(secsSince1900);

	// now convert NTP time into everyday time:
	Serial.print("Unix time = ");
	// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
	const unsigned long seventyYears = 2208988800UL;
	// subtract seventy years:
	unsigned long epoch = secsSince1900 - seventyYears;
	// print Unix time:
	Serial.println(epoch);
	*t = (time_t)epoch;

	// print the hour, minute and second:
	Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
	Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
	Serial.print(':');
	if ( ((epoch % 3600) / 60) < 10 ) {
	// In the first 10 minutes of each hour, we'll want a leading '0'
	Serial.print('0');
	}
	Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
	Serial.print(':');
	if ( (epoch % 60) < 10 ) {
	// In the first 10 seconds of each minute, we'll want a leading '0'
	Serial.print('0');
	}
	Serial.println(epoch % 60); // print the second

	return true;
}

// send an NTP request to the time server at the given address
unsigned long Network::sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void Network::wifi_sleep( void ) {
	//ESP8266------
	//WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
	//-------------
}