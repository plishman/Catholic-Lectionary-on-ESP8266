#include "TimeServer.h"

time_t TimeServer::local_datetime( void ) {
  Serial.println("local_datetime()");
  
  // need to add a timeout
  String colon = ":";
  String slash = "/";
  bool b_nmea_message_received = false;
  String nmea_sentence = "";
    
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long fix_age;
  float lat, lng;
  
  do {
    wdt_reset();
    if (Serial.available() > 0) {
		byte b = Serial.read();
		b_nmea_message_received = gps.encode(b);
		
		//Serial.write(b);
		
		if (b_nmea_message_received) {
			if (nmea_sentence.indexOf("NMEA unknown msg") == -1) { // we're using the TX and RX pins used for debug output, so this output will be sent also to the GPS, which uses a checksum to validate messages, which debug messages will not have, so the GPS will produce an error when it sees them, which we don't print
				Serial.println(nmea_sentence);
			}
			nmea_sentence = "";
		}
		else {
			nmea_sentence += (char)b;
		}
	}
		
    if (b_nmea_message_received) {
	  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);
      gps.f_get_position(&lat, &lng, &fix_age); 
    }
  } while (!b_nmea_message_received || fix_age == TinyGPS::GPS_INVALID_AGE || fix_age > 5000);

  Serial.println();
  Serial.print("LAT="); Serial.print(lat, 6);
  Serial.print("LNG="); Serial.println(lng, 6);

  String d = String(day);
  String m = String(month);
  String y = String(year);
  String hh = String(hour);
  String mm = String(minute);
  String ss = String(second);

  Serial.print("Datetime=" + d + slash + m + slash + y + " "); // Year (2000+) (u16)     
  Serial.println(hh + colon + mm + colon + ss);
/*
  if (lng < -180.0) {
    while (lng < -180.0) lng += 360.0;
  }
  else if (lng > 180.0) {
    while (lng > 180.0) lng -= 360.0;    
  }
*/  
  setTime(hour, minute, second, day, month, year);
  float a = lng * (12/180);
  long adjustment = (long)round(a); // round is a macro in arduino, which returns a long, hence the intermediate processing with the float a
  Serial.println("set_local_datetime(): adjustment = " + String(adjustment));
  adjustTime(adjustment);
  return now();
}

void TimeServer::gps_sleep( void ) {
	const byte ublox_sleep_packet[] = {0xb5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4d, 0x3b};
	const byte ublox_sleep_packet_bytes = 16;
	
	for (byte i = 0; i < ublox_sleep_packet_bytes; i++) {
		Serial.write(ublox_sleep_packet[i]);
	}
	delay(1000);
}

void TimeServer::gps_wake( void ) {
	const byte ublox_wake_packet[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const byte ublox_wake_packet_bytes = 16;
	
	for (byte i = 0; i < ublox_wake_packet_bytes; i++) {
		Serial.write(ublox_wake_packet[i]);
	}
	delay(1000);
}

