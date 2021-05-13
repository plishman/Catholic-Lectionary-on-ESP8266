/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form Jean-Michel Gallego version
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */


/*******************************************************************************
 **                                                                            **
 **                       DEFINITIONS FOR FTP SERVER                           **
 **                                                                            **
 *******************************************************************************/

#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#define FTP_SERVER_VERSION "2020-12-08"

#include <FtpServerKey.h>
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#if(NETWORK_ESP8266_242 == DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266)
	#define ARDUINO_ESP8266_RELEASE_2_4_2
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 NETWORK_ESP8266
#endif
//
//#if(NETWORK_ESP8266_SD == DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266)
//	#define ESP8266_GT_2_4_2_SD_STORAGE_SELECTED
//	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 NETWORK_ESP8266
//#endif

#if !defined(FTP_SERVER_NETWORK_TYPE)
// select Network type based
	#if defined(ESP8266) || defined(ESP31B)
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_ESP8266
	#elif defined(ESP32)
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_ESP32
	#elif defined(ARDUINO_ARCH_SAMD)
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_SAMD
	#else
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_ARDUINO
	//	#define STORAGE_SD_ENABLED
	#endif
#endif

#if defined(ESP8266) || defined(ESP31B)
	#ifndef STORAGE_SD_FORCE_DISABLE
		#define STORAGE_SD_ENABLED
	#endif
	#ifndef STORAGE_SPIFFS_FORCE_DISABLE
		#define STORAGE_SPIFFS_ENABLED
	#endif
#elif defined(ESP32)
	#ifndef STORAGE_SD_FORCE_DISABLE
		#define STORAGE_SD_ENABLED
	#endif
	#ifndef STORAGE_SPIFFS_FORCE_DISABLE
		#define STORAGE_SPIFFS_ENABLED
	#endif
#else
	#ifndef STORAGE_SD_FORCE_DISABLE
		#define STORAGE_SD_ENABLED
	#endif
#endif


// Includes and defined based on Network Type
#if(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP8266_ASYNC)

// Note:
//   No SSL/WSS support for client in Async mode
//   TLS lib need a sync interface!

#if defined(ESP8266)
#include <ESP8266WiFi.h>
//#include <WiFiClientSecure.h>
#elif defined(ESP32)
#include <WiFi.h>
//#include <WiFiClientSecure.h>

#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiClientSecure
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer

#elif defined(ESP31B)
#include <ESP31BWiFi.h>
#else
#error "network type ESP8266 ASYNC only possible on the ESP mcu!"
#endif
//
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncTCPbuffer.h>
//#define FTP_CLIENT_NETWORK_CLASS AsyncTCPbuffer
//#define FTP_SERVER_NETWORK_SERVER_CLASS AsyncServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP8266 || FTP_SERVER_NETWORK_TYPE == NETWORK_ESP8266_242)

#if !defined(ESP8266) && !defined(ESP31B)
#error "network type ESP8266 only possible on the ESP mcu!"
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <ESP31BWiFi.h>
#endif
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiClientSecure
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer
#define NET_CLASS WiFi
#define CommandIs( a ) (command != NULL && ! strcmp_P( command, PSTR( a )))
#define ParameterIs( a ) ( parameter != NULL && ! strcmp_P( parameter, PSTR( a )))
#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_W5100)

#ifdef STM32_DEVICE
#define FTP_CLIENT_NETWORK_CLASS TCPClient
#define FTP_SERVER_NETWORK_SERVER_CLASS TCPServer
#define CommandIs( a ) ( ! strcmp_PF( command, PSTR( a )))
#define ParameterIs( a ) ( ! strcmp_PF( parameter, PSTR( a )))
#define NET_CLASS Ethernet
#else
#include <Ethernet.h>
#include <SPI.h>
#define FTP_CLIENT_NETWORK_CLASS EthernetClient
#define FTP_SERVER_NETWORK_SERVER_CLASS EthernetServer
#define NET_CLASS Ethernet
#define CommandIs( a ) ( ! strcmp_PF( command, PSTR( a )))
#define ParameterIs( a ) ( ! strcmp_PF( parameter, PSTR( a )))
#endif

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ENC28J60)

#include <UIPEthernet.h>

#define FTP_CLIENT_NETWORK_CLASS UIPClient
#define FTP_SERVER_NETWORK_SERVER_CLASS UIPServer
#define NET_CLASS Ethernet
#define CommandIs( a ) ( ! strcmp_PF( command, PSTR( a )))
#define ParameterIs( a ) ( ! strcmp_PF( parameter, PSTR( a )))
//#include <UIPEthernet.h>
//UIPClient base_client;
//SSLClient client(base_client, TAs, (size_t)TAs_NUM, A5);
//
//#define FTP_CLIENT_NETWORK_CLASS SSLClient
//#define FTP_SERVER_NETWORK_SERVER_CLASS UIPServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP32)

#include <WiFi.h>
//#include <WiFiClientSecure.h>
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiClientSecure
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer
#define NET_CLASS WiFi
#define CommandIs( a ) (command != NULL && ! strcmp_P( command, PSTR( a )))
#define ParameterIs( a ) ( parameter != NULL && ! strcmp_P( parameter, PSTR( a )))
#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP32_ETH)

#include <ETH.h>
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer
#define NET_CLASS Ethernet
#define CommandIs( a ) ( ! strcmp_PF( command, PSTR( a )))
#define ParameterIs( a ) ( ! strcmp_PF( parameter, PSTR( a )))
#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_WiFiNINA)

#include <WiFiNINA.h>
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiSSLClient
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer
#define NET_CLASS WiFi
#define CommandIs( a ) ( ! strcmp_PF( command, PSTR( a )))
#define ParameterIs( a ) ( ! strcmp_PF( parameter, PSTR( a )))
#else
#error "no network type selected!"
#endif

#if(STORAGE_TYPE == STORAGE_SPIFFS)
		#if defined(ESP32)
//			#define FS_NO_GLOBALS
			#include <SPIFFS.h>

			#define FTP_FILE File
  	  	  	#define FTP_DIR File
		#else
			#ifdef ARDUINO_ESP8266_RELEASE_2_4_2
				#define FS_NO_GLOBALS
				#include "FS.h"
			  #define FTP_FILE fs::File
			  #define FTP_DIR fs::Dir
			#else
				#include "FS.h"
			  #define FTP_FILE File
			  #define FTP_DIR Dir
			#endif

		#endif


#if ESP8266
		#define O_FTP_FILE_READ     "r"

	   #define O_FTP_FILE_RDONLY     "r"

	   #define O_FTP_FILE_WRITE    "w"
	   #define O_FTP_FILE_RDWR     "w+"
	   #define O_FTP_FILE_CREAT    "w+"
	   #define O_FTP_FILE_APPEND   "a+"
#else
	#define O_FTP_FILE_READ    "r"
#endif

		#define STORAGE_MANAGER SPIFFS
#elif(STORAGE_TYPE == STORAGE_LITTLEFS)
		#include "LittleFS.h"
		#define STORAGE_MANAGER LittleFS
		#define FTP_FILE File
		#define FTP_DIR Dir

		#define O_FTP_FILE_READ     "r"

		#define O_FTP_FILE_RDONLY     "r"

		#define O_FTP_FILE_WRITE    "w"
		#define O_FTP_FILE_RDWR     "w+"
		#define O_FTP_FILE_CREAT    "w+"
		#define O_FTP_FILE_APPEND   "a+"

#elif(STORAGE_TYPE == STORAGE_SD)
		#include <SPI.h>
		#include <SD.h>
	  #define STORAGE_MANAGER SD
  	  #define FTP_FILE File
  	  #define FTP_DIR File
    

#elif (STORAGE_TYPE <= STORAGE_SDFAT2)
  #include <SdFat.h>
  #include <sdios.h>

  #define STORAGE_MANAGER sd
  #define FTP_FILE SdFile
  #define FTP_DIR SdFile
  extern SdFat STORAGE_MANAGER;


#elif (STORAGE_TYPE == STORAGE_SPIFM)
  #include <SdFat.h>
  #include <Adafruit_SPIFlash.h>
#include <sdios.h>

  #define STORAGE_MANAGER fatfs
  #define FTP_FILE File
  #define FTP_DIR File
  extern FatFileSystem STORAGE_MANAGER;
  extern Adafruit_SPIFlash flash;
#elif (STORAGE_TYPE == STORAGE_FATFS)
  #include <FatFs.h>
#include <sdios.h>

  #define STORAGE_MANAGER sdff
  #define FTP_FILE FileFs
  #define FTP_DIR DirFs
  extern FatFsClass STORAGE_MANAGER;
  #define O_FTP_FILE_READ     FA_READ
  #define O_FTP_FILE_WRITE    FA_WRITE
  #define O_FTP_FILE_RDWR     FA_READ | FA_WRITE
  #define O_FTP_FILE_CREAT    FA_CREATE_ALWAYS
  #define O_FTP_FILE_APPEND   FA_OPEN_APPEND

#elif(STORAGE_TYPE == STORAGE_SD2)
		#include <SPI.h>
		#include <SD.h>
    #include <TimeLib.h>
	  #define STORAGE_MANAGER SDFS
  	  #define FTP_FILE File
  	  #define FTP_DIR Dir
  
  		#define O_FTP_FILE_READ     "r"
	    #define O_FTP_FILE_RDONLY     "r"
	    #define O_FTP_FILE_WRITE    "w"
	    #define O_FTP_FILE_RDWR     "w+"
	    #define O_FTP_FILE_CREAT    "w+"
	    #define O_FTP_FILE_APPEND   "a+"
  /*      
      #define O_FTP_FILE_READ    sdfat::O_READ    
      #define O_FTP_FILE_WRITE   sdfat::O_WRITE   
      #define O_FTP_FILE_RDONLY  sdfat::O_RDONLY    
      #define O_FTP_FILE_RDWR    sdfat::O_RDWR    
      #define O_FTP_FILE_CREAT   sdfat::O_CREAT   
      #define O_FTP_FILE_APPEND  sdfat::O_APPEND
  */
#endif


//#ifdef FTP_CLIENT_NETWORK_SSL_CLASS
//#define FTP_CLIENT_NETWORK_CLASS FTP_CLIENT_NETWORK_SSL_CLASS
//#endif

#define OPEN_CLOSE_SPIFFS
#define OPEN_CLOSE_SD

// Setup debug printing macros.
#ifdef FTP_SERVER_DEBUG
	#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
	#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
	#define DEBUG_PRINT(...) {}
	#define DEBUG_PRINTLN(...) {}
#endif


//#if FTP_FILESYST <= FTP_SDFAT2
//  #include <SdFat.h>
//  #define FTP_FS sd
//  #define FTP_FILE SdFile
//  #define FTP_DIR SdFile
//  extern SdFat FTP_FS;
//#elif FTP_FILESYST == FTP_SPIFM
//  #include <SdFat.h>
//  #include <Adafruit_SPIFlash.h>
//  #define FTP_FS fatfs
//  #define FTP_FILE File
//  #define FTP_DIR File
//  extern FatFileSystem FTP_FS;
//  extern Adafruit_SPIFlash flash;
//#elif FTP_FILESYST == FTP_FATFS
//  #include <FatFs.h>
//  #define FTP_FS sdff
//  #define FTP_FILE FileFs
//  #define FTP_DIR DirFs
//  extern FatFsClass FTP_FS;
//  #define O_FTP_FILE_READ     FA_READ
//  #define O_FTP_FILE_WRITE    FA_WRITE
//  #define O_FTP_FILE_RDWR     FA_READ | FA_WRITE
//  #define O_FTP_FILE_CREAT    FA_CREATE_ALWAYS
//  #define O_FTP_FILE_APPEND   FA_OPEN_APPEND
//#endif
//
//#ifdef ESP8266
//  #define FTP_SERVER WiFiServer
//  #define FTP_CLIENT WiFiClient
//  #define FTP_LOCALIP() WiFi.localIP()
//  #define CommandIs( a ) (command != NULL && ! strcmp_P( command, PSTR( a )))
//  #define ParameterIs( a ) ( parameter != NULL && ! strcmp_P( parameter, PSTR( a )))
//#else
//  #define FTP_SERVER EthernetServer
//  #define FTP_CLIENT EthernetClient
//  #define FTP_LOCALIP() Ethernet.localIP()
//  #define CommandIs( a ) ( ! strcmp_PF( command, PSTR( a )))
//  #define ParameterIs( a ) ( ! strcmp_PF( parameter, PSTR( a )))
//#endif

#define FTP_USER "arduino"        // Default user'name
#define FTP_PASS "test"           // Default password

#define FTP_CMD_PORT 21           // Command port on wich server is listening
#define FTP_DATA_PORT_DFLT 20     // Default data port in active mode
#define FTP_DATA_PORT_PASV 50009  // Data port in passive mode

#define FF_MAX_LFN 255            // max size of a long file name 
#define FTP_CMD_SIZE FF_MAX_LFN+8 // max size of a command
#define FTP_CWD_SIZE FF_MAX_LFN+8 // max size of a directory name
#define FTP_FIL_SIZE FF_MAX_LFN   // max size of a file name 
#define FTP_CRED_SIZE 16          // max size of username and password
#define FTP_NULLIP() IPAddress(0,0,0,0)

enum ftpCmd { FTP_Stop = 0,       //  In this stage, stop any connection
              FTP_Init,           //  initialize some variables
              FTP_Client,         //  wait for client connection
              FTP_User,           //  wait for user name
              FTP_Pass,           //  wait for user password
              FTP_Cmd };          //  answers to commands

enum ftpTransfer { FTP_Close = 0, // In this stage, close data channel
                   FTP_Retrieve,  //  retrieve file
                   FTP_Store,     //  store file
                   FTP_List,      //  list of files
                   FTP_Nlst,      //  list of name of files
                   FTP_Mlsd };    //  listing for machine processing

enum ftpDataConn { FTP_NoConn = 0,// No data connexion
                   FTP_Pasive,    // Pasive type
                   FTP_Active };  // Active type

/*
class FtpFile : public SdFile
{
#if STORAGE_TYPE == FTP_SPIFM
#endif
};
*/

class FtpServer
{
public:
  FtpServer( uint16_t _cmdPort = FTP_CMD_PORT, uint16_t _pasvPort = FTP_DATA_PORT_PASV );

  void    begin( const char * _user, const char * _pass );
  void    credentials( const char * _user, const char * _pass );
  uint8_t handleFTP();
  bool    isInUse();

private:
  void    iniVariables();
  void    clientConnected();
  void    disconnectClient();
  bool    processCommand();
  bool    haveParameter();
  int     dataConnect( bool out150 = true );
  bool    dataConnected();
  bool    doRetrieve();
  bool    doStore();
  bool    doList();
  bool    doMlsd();
  void    closeTransfer();
  void    abortTransfer();
  bool    makePath( char * fullName, char * param = NULL );
  bool    makeExistsPath( char * path, char * param = NULL );
  bool    openDir( FTP_DIR * pdir );
  bool    isDir( char * path );
  uint8_t getDateTime( char * dt, uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                       uint8_t * phour, uint8_t * pminute, uint8_t * second );
  char *  makeDateTimeStr( char * tstr, uint16_t date, uint16_t time );
  bool    timeStamp( char * path, uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second );
  bool    getFileModTime( char * path, uint16_t * pdate, uint16_t * ptime );
#if STORAGE_TYPE != STORAGE_FATFS
  bool    getFileModTime( uint16_t * pdate, uint16_t * ptime );
#endif
  int8_t  readChar();

  bool     exists( const char * path ) {
#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_SD
	  if (strcmp(path, "/") == 0) return true;
#endif
   	  return STORAGE_MANAGER.exists( path );
  };
  bool     remove( const char * path ) { return STORAGE_MANAGER.remove( path ); };
  bool     makeDir( const char * path ) { return STORAGE_MANAGER.mkdir( path ); };
  bool     removeDir( const char * path ) { return STORAGE_MANAGER.rmdir( path ); };

#if STORAGE_TYPE == STORAGE_SD
  bool     rename( const char * path, const char * newpath );
#endif

#if (STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD)
  bool     rename( const char * path, const char * newpath )
             { return STORAGE_MANAGER.rename( path, newpath ); };
  bool openFile( char path[ FTP_CWD_SIZE ], int readTypeInt );

#elif STORAGE_TYPE == STORAGE_SD2
  bool     rename( const char * path, const char * newpath )
  	  	  	  { return STORAGE_MANAGER.rename( path, newpath ); };
  bool openFile( char path[ FTP_CWD_SIZE ], const char * readType );

#else
  bool openFile( char path[ FTP_CWD_SIZE ], const char * readType );
#endif

  bool openFile( const char * path, const char * readType );
  uint16_t fileSize( FTP_FILE file );

#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS
#if ESP8266
  uint32_t capacity() {
	  FSInfo fi;
	  STORAGE_MANAGER.info(fi);

	  return fi.totalBytes >> 1;
  };
  uint32_t free() {
	  FSInfo fi;
	  STORAGE_MANAGER.info(fi);

	  return fi.totalBytes -
			  fi.usedBytes >> 1;
  };
#else
  uint32_t capacity() {
	  return STORAGE_MANAGER.totalBytes() >> 1;
  };
  uint32_t free() {
	  return (STORAGE_MANAGER.totalBytes() -
			  STORAGE_MANAGER.usedBytes()) >> 1;
  };
#endif
#elif STORAGE_TYPE == STORAGE_SD
  uint32_t capacity() { return true; };
  uint32_t free() { return true; };

#elif STORAGE_TYPE == STORAGE_SD2
  uint32_t capacity() { FSInfo fsinfo; if (STORAGE_MANAGER.info(fsinfo)) { return fsinfo.totalBytes; } else { return 0; } };
  uint32_t free()     { FSInfo fsinfo; if (STORAGE_MANAGER.info(fsinfo)) { return fsinfo.totalBytes - fsinfo.usedBytes; }  else { return 0; } };

#elif STORAGE_TYPE == STORAGE_SDFAT1
  uint32_t capacity() { return STORAGE_MANAGER.card()->cardSize() >> 1; };
  uint32_t free() { return STORAGE_MANAGER.vol()->freeClusterCount() *
                           STORAGE_MANAGER.vol()->sectorsPerCluster() >> 1; };

#elif STORAGE_TYPE == STORAGE_SDFAT2
  uint32_t capacity() { return STORAGE_MANAGER.card()->sectorCount() >> 1; };
  uint32_t free() { return STORAGE_MANAGER.vol()->freeClusterCount() *
                           STORAGE_MANAGER.vol()->sectorsPerCluster() >> 1; };
#elif STORAGE_TYPE == STORAGE_SPIFM
  uint32_t capacity() { return flash.size() >> 10; };
  uint32_t free() { return 0; };    // TODO //
#elif STORAGE_TYPE == STORAGE_FATFS
  uint32_t capacity() { return STORAGE_MANAGER.capacity(); };
  uint32_t free() { return STORAGE_MANAGER.free(); };
#endif
	bool    legalChar( char c ) // Return true if char c is allowed in a long file name
	{
		if( c == '"' || c == '*' || c == '?' || c == ':' || 
		    c == '<' || c == '>' || c == '|' )
		  return false;
#if STORAGE_TYPE == STORAGE_FATFS
		return 0x1f < c && c < 0xff;
#else
		return 0x1f < c && c < 0x7f;
#endif
	}
  
  IPAddress   localIp;                // IP address of server as seen by clients
  IPAddress   dataIp;                 // IP address of client for data
  FTP_SERVER_NETWORK_SERVER_CLASS  ftpServer;
  FTP_SERVER_NETWORK_SERVER_CLASS  dataServer;


  FTP_CLIENT_NETWORK_CLASS  client;
  FTP_CLIENT_NETWORK_CLASS  data;
  
  FTP_FILE     file;
  FTP_DIR      dir;
  
  ftpCmd      cmdStage;               // stage of ftp command connexion
  ftpTransfer transferStage;          // stage of data connexion
  ftpDataConn dataConn;               // type of data connexion

  bool _bServerIsInUse = false;       // set if the server has a client

  uint8_t  __attribute__((packed, aligned(4))) // need to be aligned to 32bit for Esp8266 SPIClass::transferBytes()
           buf[ FTP_BUF_SIZE ];       // data buffer for transfers
  char     cmdLine[ FTP_CMD_SIZE ];   // where to store incoming char from client
  char     cwdName[ FTP_CWD_SIZE ];   // name of current directory
  char     rnfrName[ FTP_CWD_SIZE ];  // name of file for RNFR command
  char     user[ FTP_CRED_SIZE ];     // user name
  char     pass[ FTP_CRED_SIZE ];     // password
  char     command[ 5 ];              // command sent by client
  bool     rnfrCmd;                   // previous command was RNFR
  char *   parameter;                 // point to begin of parameters sent by client
  uint16_t cmdPort,
           pasvPort,
           dataPort;
  uint16_t iCL;                       // pointer to cmdLine next incoming char
  uint16_t nbMatch;

  uint32_t millisDelay,               //
           millisEndConnection,       // 
           millisBeginTrans,          // store time of beginning of a transaction
           bytesTransfered;           //
};

#if STORAGE_TYPE == STORAGE_SD2
//time_t fsTimeStampCallback();   //FS callback for file timestamping
#endif

#endif // FTP_SERVER_H
