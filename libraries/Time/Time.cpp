/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009-2014

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  1.0  6  Jan 2010 - initial release
  1.1  12 Feb 2010 - fixed leap year calculation error
  1.2  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  1.3  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                     status, updated examples for Arduino 1.0, fixed ARM
                     compatibility issues, added TimeArduinoDue and TimeTeensy3
                     examples, add error checking and messages to RTC examples,
                     add examples to DS1307RTC library.
  1.4  5  Sep 2014 - compatibility with Arduino 1.5.7
*/
#ifndef _WIN32
#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif
#else
#include <sys\timeb.h> 
unsigned short millis();
unsigned short millis() {
	struct timeb tm;
	ftime(&tm);
	return tm.millitm;
}
#endif

#include "TimeLib.h"

// if _USE_LONG_TIME_T is defined then time_t will be defined as a long only (32bits), so need to have both 32bit (time_t) and 64 bit (time64_t) functions defined
#if defined(_USE_LONG_TIME_T) || __LONG_MAX__ > 0x7fffffffL
static tmElements_t tm;          // a cache of time elements
static time64_t cacheTime;       // the time the cache was updated
static uint32_t syncInterval = 300;  // time sync will be attempted after this many seconds


time64_t TO64( time_t t ) { return ((time64_t) t) & 0xFFFFFFFF; }

void refreshCache(time_t t) 
{
  refreshCache(TO64(t));
}

void refreshCache(time64_t t) 
{
  if (t != cacheTime) {
    breakTime(t, tm); 
    cacheTime = t; 
  }
}

int hour() { // the hour now 
  return hour(now()); 
}

int hour(time_t t) { // the hour for the given time
  return hour(TO64(t));
}

int hour(time64_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;  
}

int hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now()); 
}

int hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  return hourFormat12(TO64(t));
}

int hourFormat12(time64_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t isAM() { // returns true if time now is AM
  return !isPM(now()); 
}

uint8_t isAM(time_t t) { // returns true if given time is AM
  return isAM(TO64(t));  
}

uint8_t isAM(time64_t t) { // returns true if given time is AM
  return !isPM(t);  
}

uint8_t isPM() { // returns true if PM
  return isPM(now()); 
}

uint8_t isPM(time_t t) { // returns true if PM
  return isPM(TO64(t));
}

uint8_t isPM(time64_t t) { // returns true if PM
  return (hour(t) >= 12); 
}

int minute() {
  return minute(now()); 
}

int minute(time_t t) { // the minute for the given time
  return minute(TO64(t));
}

int minute(time64_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;  
}

int second() {
  return second(now()); 
}

int second(time_t t) {  // the second for the given time
  return second(TO64(t));
}

int second(time64_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int day(){
  return(day(now())); 
}

int day(time_t t) { // the day for the given time (0-6)
  return day(TO64(t));
}

int day(time64_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int weekday() {   // Sunday is day 1
  return  weekday(now()); 
}

int weekday(time_t t) {
  return weekday(TO64(t));
}

int weekday(time64_t t) {
  refreshCache(t);
  return tm.Wday;
}
   
int month(){
  return month(now()); 
}

int month(time_t t) {  // the month for the given time
  return month(TO64(t));
}

int month(time64_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int year() {  // as in Processing, the full four digit year: (2009, 2010 etc) 
  return year(now()); 
}

int year(time_t t) { // the year for the given time
  //Serial.printf("year(): %lx", t);
  return year(TO64(t));
}

int year(time64_t t) { // the year for the given time
  //Serial.printf("year(64bit): %llx", t);
  refreshCache(t);
  return tmYearToCalendar(tm.Year);
}

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

// 32 bit version for compatibility/file timestamping etc
time_t makeTime32(tmElements_t &tm) 
{
  return (time_t) makeTime(tm);
}

time64_t makeTime(tmElements_t &tm, bool busenewmethod){

	//if (tm.Year < 0) return 0; // tm.Year is unsigned, so this cannot happen
	
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  //T64 t64;
  
  int i;
  time64_t seconds; // was uint32_t seconds **** fix for y2038 bug
 
  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(SECS_PER_DAY * 365);
  
  if (tm.Year > 0) { // if not, then date in first year of BEGIN_EPOCH, so only need to calculate the part of the year (no full years up to this date)
	  if (busenewmethod) {
		  int64_t calyear = tmYearToCalendar(tm.Year) - 1; // subtract 1 since only full years are considered. The remainder of the current year is calculated next
		  //int64_t nearest4start = 1970 + (4 - (1970 % 4));
		  #define NEAREST4START (1970 + (4 - (1970 % 4))) // conserve stack by doing it this way (ESP8266 has only 4Kb stack, and the Lectionary code has already had stack overflow problems
		  int64_t nearest4end = calyear - (calyear % 4);

		  //int64_t nearest100start = 1970 + (100 - (1970 % 100));
		  #define NEAREST100START (1970 + (100 - (1970 % 100)))
		  int64_t nearest100end = calyear - (calyear % 100);

		  //int64_t nearest400start = 1970 + (400 - (1970 % 400));
		  #define NEAREST400START (1970 + (400 - (1970 % 400)))
		  int64_t nearest400end = calyear - (calyear % 400);

		  int64_t leapyearcount = (1 + ((nearest4end - NEAREST4START) / 4))
			  - (1 + ((nearest100end - NEAREST100START) / 100))
			  + (1 + ((nearest400end - NEAREST400START) / 400));

		  seconds += leapyearcount * SECS_PER_DAY;
	  }
	  else {
		  for (i = 0; i < tm.Year; i++) {
			  if (LEAP_YEAR(i)) {
				  seconds += SECS_PER_DAY;   // add extra days for leap years
			  }
		  }
	  }
  }
  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * SECS_PER_DAY;
  seconds+= tm.Hour * SECS_PER_HOUR;
  seconds+= tm.Minute * SECS_PER_MIN;
  seconds+= tm.Second;

  //t64.time = seconds; I2CSerial.printf("\n\nmakeTime() seconds=%lx %lx\n", t64.words[1], t64.words[0]);
  //Serial.printf(" ^%llx ", seconds);

  return seconds;
}
 
// 32 bit version for compatibility
void breakTime(time_t timeInput, tmElements_t &tm, bool busenewmethod)
{
  //Serial.printf("timeInput (time_t)=[%lx] ", timeInput);
  breakTime(TO64(timeInput), tm, busenewmethod);
}

void breakTime(time64_t timeInput, tmElements_t &tm, bool busenewmethod){
  //Serial.printf("timeInput (time64_t)=[%llx] ", timeInput);

  if (timeInput < 0) timeInput = 0; // doesn't support negative time yet (dates before 1970) * time_t, time64_t are unsigned, so this will do nothing...*

// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!! 
  uint32_t year; // **** was uint8_t year - fix for y2038 bug
  uint8_t month, monthLength;
  uint64_t time; // **** was uint32_t time - fix for y2038 bug changed to uint64_t
  uint64_t days; // **** was unsigned long - fix for y2038 bug
  //uint64_t sixty_ll = 60;
  //uint64_t twentyfour_ll = 24;
  
  //T64 t64;
  //t64.time = timeInput; I2CSerial.printf("\n\nbreakTime() seconds=%lx %lx\n", t64.words[1], t64.words[0]);
    
  //time = (uint64_t)timeInput; // **** was (uint32_t)timeInput - fix for y2038 bug changed to uint64_t
  time = timeInput; //(uint64_t)timeInput.words[1] << 32 | (uint64_t)timeInput.words[0];
  
  tm.Second = time % 60;
  time = time / 60; // now it is minutes
  tm.Minute = time % 60;
  time = time / 60; // now it is hours
  tm.Hour = time % 24;
  time = time / 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
 
  year = 0;  
  days = 0;

  if (busenewmethod) {
	  //int64_t daysper4centuries = (365 * 400) + ((400 / 4) - 3);
	  #define DAYSPER4CENTURIES ((365 * 400) + ((400 / 4) - 3))
	  int64_t numberof4cblocks = (time / DAYSPER4CENTURIES);
	  //int64_t daysremaininginpartial4cblock = (time % DAYSPER4CENTURIES);
	  year = year + (400 * numberof4cblocks);
	  time -= (numberof4cblocks * DAYSPER4CENTURIES);
  }

  while((days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
    //Serial.print("#");
    /*
    if ((year % 10) == 0) {
      Serial.printf("y=%lu ", year);
    }
    else {
      Serial.print("*");
    }
    */
    //I2CSerial.printf("year=%lu", year);
  }
  tm.Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1  
  tm.Day = time + 1;     // day of month
}

/*=====================================================*/	
/* Low level system time functions  */

static unsigned long long sysTime = 0;
static uint32_t prevMillis = 0;
static unsigned long long nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

#ifdef TIME_DRIFT_INFO   // define this to get drift data
time_t sysUnsyncedTime = 0; // the time sysTime unadjusted by sync  
#endif


time_t now32() 
{
  return (time_t)now();
}

time64_t now() {
	// calculate number of seconds passed since last call to now()
  while (millis() - prevMillis >= 1000) {
		// millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
    sysTime++;
    prevMillis += 1000;	
#ifdef TIME_DRIFT_INFO
    sysUnsyncedTime++; // this can be compared to the synced time to measure long term drift     
#endif
  }
  if (nextSyncTime <= sysTime) {
    if (getTimePtr != 0) {
      time64_t t = getTimePtr();
      //Serial.printf(" *%llu ", t);
      if (t != 0) {
        setTime(t);
      } else {
        nextSyncTime = sysTime + syncInterval;
        Status = (Status == timeNotSet) ?  timeNotSet : timeNeedsSync;
      }
    }
  }  
  return (time64_t)sysTime;
}

void setTime(time_t t) 
{ 
  setTime(TO64(t));
}

void setTime(time64_t t) 
{
#ifdef TIME_DRIFT_INFO
 if(sysUnsyncedTime == 0) 
   sysUnsyncedTime = t;   // store the time of the first call to set a valid Time   
#endif

  sysTime = (unsigned long long)t; 
  //Serial.printf(" =%llx ", t);
  nextSyncTime = (unsigned long long)t + syncInterval;
  Status = timeSet;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
} 

void setTime(int hr,int min,int sec,int dy, int mnth, int yr){
 // year can be given as full four digit year or two digts (2010 or 10 for 2010);  
 //it is converted to years since 1970
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;  
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  setTime(makeTime(tm));
}

void adjustTime(long adjustment) {
  sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
timeStatus_t timeStatus() {
  now(); // required to actually update the status
  return Status;
}

void setSyncProvider( getExternalTime getTimeFunction){
  getTimePtr = getTimeFunction;  
  nextSyncTime = sysTime;
  now(); // this will sync the clock
}

void setSyncInterval(time_t interval){ // set the number of seconds between re-sync
  syncInterval = (uint32_t)interval;
  nextSyncTime = sysTime + syncInterval;
}
#else
// 64 bit time native, so don't need to differentiate between 32 and 64 bit function prototypes
static tmElements_t tm;          // a cache of time elements
static time64_t cacheTime;       // the time the cache was updated
static uint32_t syncInterval = 300;  // time sync will be attempted after this many seconds


time64_t TO64( time_t t ) { return ((time64_t) t) & 0xFFFFFFFF; }

void refreshCache(time64_t t) 
{
  if (t != cacheTime) {
    breakTime(t, tm); 
    cacheTime = t; 
  }
}

int hour() { // the hour now 
  return hour(now()); 
}

int hour(time64_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;  
}

int hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now()); 
}

int hourFormat12(time64_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t isAM() { // returns true if time now is AM
  return !isPM(now()); 
}

uint8_t isAM(time64_t t) { // returns true if given time is AM
  return !isPM(t);  
}

uint8_t isPM() { // returns true if PM
  return isPM(now()); 
}

uint8_t isPM(time64_t t) { // returns true if PM
  return (hour(t) >= 12); 
}

int minute() {
  return minute(now()); 
}

int minute(time64_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;  
}

int second() {
  return second(now()); 
}

int second(time64_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int day(){
  return(day(now())); 
}

int day(time64_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int weekday() {   // Sunday is day 1
  return  weekday(now()); 
}

int weekday(time64_t t) {
  refreshCache(t);
  return tm.Wday;
}
   
int month(){
  return month(now()); 
}

int month(time64_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int year() {  // as in Processing, the full four digit year: (2009, 2010 etc) 
  return year(now()); 
}

int year(time64_t t) { // the year for the given time
  //Serial.printf("year(64bit): %llx", t);
  refreshCache(t);
  return tmYearToCalendar(tm.Year);
}

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

time64_t makeTime(tmElements_t &tm, bool busenewmethod){

	//if (tm.Year < 0) return 0; // tm.Year is unsigned, so this cannot happen
	
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  //T64 t64;
  
  int i;
  time64_t seconds; // was uint32_t seconds **** fix for y2038 bug
 
  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(SECS_PER_DAY * 365);

  if (tm.Year > 0) { // if not, then date in first year of BEGIN_EPOCH, so only need to calculate the part of the year (no full years up to this date)
	  if (busenewmethod) {
		  int64_t calyear = tmYearToCalendar(tm.Year) - 1; // subtract 1 since only full years are considered. The remainder of the current year is calculated next
		  //int64_t nearest4start = 1970 + (4 - (1970 % 4));
		  #define NEAREST4START (1970 + (4 - (1970 % 4))) // conserve stack by doing it this way (ESP8266 has only 4Kb stack, and the Lectionary code has already had stack overflow problems
		  int64_t nearest4end = calyear - (calyear % 4);
		
		  //int64_t nearest100start = 1970 + (100 - (1970 % 100));
		  #define NEAREST100START (1970 + (100 - (1970 % 100)))
		  int64_t nearest100end = calyear - (calyear % 100);
		  
		  //int64_t nearest400start = 1970 + (400 - (1970 % 400));
		  #define NEAREST400START (1970 + (400 - (1970 % 400)))
		  int64_t nearest400end = calyear - (calyear % 400);

		  int64_t leapyearcount = (1 + ((nearest4end - NEAREST4START) / 4))
			  - (1 + ((nearest100end - NEAREST100START) / 100))
			  + (1 + ((nearest400end - NEAREST400START) / 400));

		  seconds += leapyearcount * SECS_PER_DAY;
	  }
	  else {
		  for (i = 0; i < tm.Year; i++) {
			  if (LEAP_YEAR(i)) {
				  seconds += SECS_PER_DAY;   // add extra days for leap years
			  }
		  }
	  }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * SECS_PER_DAY;
  seconds+= tm.Hour * SECS_PER_HOUR;
  seconds+= tm.Minute * SECS_PER_MIN;
  seconds+= tm.Second;

  //t64.time = seconds; I2CSerial.printf("\n\nmakeTime() seconds=%lx %lx\n", t64.words[1], t64.words[0]);
  //Serial.printf(" ^%llx ", seconds);

  return seconds;
}
 
void breakTime(time64_t timeInput, tmElements_t &tm, bool busenewmethod){
  //Serial.printf("timeInput (time64_t)=[%llx] ", timeInput);

  if (timeInput < 0) timeInput = 0; // doesn't support negative time yet (dates before 1970) * time_t, time64_t are unsigned, so this will do nothing...*

// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!! 
  uint32_t year; // **** was uint8_t year - fix for y2038 bug
  uint8_t month, monthLength;
  uint64_t time; // **** was uint32_t time - fix for y2038 bug changed to uint64_t
  uint64_t days; // **** was unsigned long - fix for y2038 bug
  //uint64_t sixty_ll = 60;
  //uint64_t twentyfour_ll = 24;
  
  //T64 t64;
  //t64.time = timeInput; I2CSerial.printf("\n\nbreakTime() seconds=%lx %lx\n", t64.words[1], t64.words[0]);
    
  //time = (uint64_t)timeInput; // **** was (uint32_t)timeInput - fix for y2038 bug changed to uint64_t
  time = timeInput; //(uint64_t)timeInput.words[1] << 32 | (uint64_t)timeInput.words[0];
  
  tm.Second = time % 60;
  time = time / 60; // now it is minutes
  tm.Minute = time % 60;
  time = time / 60; // now it is hours
  tm.Hour = time % 24;
  time = time / 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
 
  year = 0;  
  days = 0;

  if (busenewmethod) {
	  //int64_t daysper4centuries = (365 * 400) + ((400 / 4) - 3);
	  #define DAYSPER4CENTURIES ((365 * 400) + ((400 / 4) - 3))
	  int64_t numberof4cblocks = (time / DAYSPER4CENTURIES);
	  //int64_t daysremaininginpartial4cblock = (time % DAYSPER4CENTURIES);
	  year = year + (400 * numberof4cblocks);
	  time -= (numberof4cblocks * DAYSPER4CENTURIES);
  }
  
  while((days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
    //Serial.print("#");
    /*
    if ((year % 10) == 0) {
      Serial.printf("y=%lu ", year);
    }
    else {
      Serial.print("*");
    }
    */
    //I2CSerial.printf("year=%lu", year);
  }
  tm.Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1  
  tm.Day = time + 1;     // day of month
}

/*=====================================================*/	
/* Low level system time functions  */

static unsigned long long sysTime = 0;
static uint32_t prevMillis = 0;
static unsigned long long nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

#ifdef TIME_DRIFT_INFO   // define this to get drift data
time_t sysUnsyncedTime = 0; // the time sysTime unadjusted by sync  
#endif


time64_t now() {
#ifndef _WIN32
	// calculate number of seconds passed since last call to now()
  while (millis() - prevMillis >= 1000) {
		// millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
    sysTime++;
    prevMillis += 1000;	
#ifdef TIME_DRIFT_INFO
    sysUnsyncedTime++; // this can be compared to the synced time to measure long term drift     
#endif
  }
  if (nextSyncTime <= sysTime) {
    if (getTimePtr != 0) {
      time64_t t = getTimePtr();
      //Serial.printf(" *%llu ", t);
      if (t != 0) {
        setTime(t);
      } else {
        nextSyncTime = sysTime + syncInterval;
        Status = (Status == timeNotSet) ?  timeNotSet : timeNeedsSync;
      }
    }
  }  
#endif
  return (time64_t)sysTime;
}

void setTime(time64_t t) 
{
#ifdef TIME_DRIFT_INFO
 if(sysUnsyncedTime == 0) 
   sysUnsyncedTime = t;   // store the time of the first call to set a valid Time   
#endif

  sysTime = (unsigned long long)t; 
  //Serial.printf(" =%llx ", t);
  nextSyncTime = (unsigned long long)t + syncInterval;
  Status = timeSet;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
} 

void setTime(int hr,int min,int sec,int dy, int mnth, int yr){
 // year can be given as full four digit year or two digts (2010 or 10 for 2010);  
 //it is converted to years since 1970
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;  
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  setTime(makeTime(tm));
}

void adjustTime(long adjustment) {
  sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
timeStatus_t timeStatus() {
  now(); // required to actually update the status
  return Status;
}

void setSyncProvider( getExternalTime getTimeFunction){
  getTimePtr = getTimeFunction;  
  nextSyncTime = sysTime;
  now(); // this will sync the clock
}

void setSyncInterval(time_t interval){ // set the number of seconds between re-sync
  syncInterval = (uint32_t)interval;
  nextSyncTime = sysTime + syncInterval;
}
#endif