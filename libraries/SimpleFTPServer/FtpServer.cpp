/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form Jean-Michel Gallego version
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 *
 * Use Ethernet library
 * 
 * Commands implemented: 
 *   USER, PASS, AUTH (AUTH only return 'not implemented' code)
 *   CDUP, CWD, PWD, QUIT, NOOP
 *   MODE, PASV, PORT, STRU, TYPE
 *   ABOR, DELE, LIST, NLST, MLST, MLSD
 *   APPE, RETR, STOR
 *   MKD,  RMD
 *   RNTO, RNFR
 *   MDTM, MFMT
 *   FEAT, SIZE
 *   SITE FREE
 *
 * Tested with those clients:
 *   under Windows:
 *     FTP Rush
 *     Filezilla
 *     WinSCP
 *     NcFTP, ncftpget, ncftpput
 *     Firefox
 *     command line ftp.exe
 *   under Ubuntu:
 *     gFTP
 *     Filezilla
 *     NcFTP, ncftpget, ncftpput
 *     lftp
 *     ftp
 *     Firefox
 *   under Android:
 *     AndFTP
 *     FTP Express
 *     Firefox
 *   with a second Arduino and sketch of SurferTim at
 *     http://playground.arduino.cc/Code/FTP
 * 
 */

#include <FtpServer.h>

FtpServer::FtpServer( uint16_t _cmdPort, uint16_t _pasvPort )
         : ftpServer( _cmdPort ), dataServer( _pasvPort )
{
  cmdPort = _cmdPort;
  pasvPort = _pasvPort;

  millisDelay = 0;
  nbMatch = 0;
  iCL = 0;

  iniVariables();
}

void FtpServer::begin( const char * _user, const char * _pass )
{
  // Tells the ftp server to begin listening for incoming connection
  ftpServer.begin();
  #ifdef ESP8266
  ftpServer.setNoDelay( true );
  #endif
//  localIp = _localIP == FTP_NULLIP() || (uint32_t) _localIP == 0 ? NET_CLASS.localIP() : _localIP ;
  localIp = NET_CLASS.localIP(); //_localIP == FTP_NULLIP() || (uint32_t) _localIP == 0 ? NET_CLASS.localIP() : _localIP ;
//  strcpy( user, FTP_USER );
//  strcpy( pass, FTP_PASS );
  if( strlen( _user ) > 0 && strlen( _user ) < FTP_CRED_SIZE ) {
    strcpy( user, _user );
  }else{
	strcpy( user, FTP_USER );
  }
  if( strlen( _pass ) > 0 && strlen( _pass ) < FTP_CRED_SIZE ) {
    strcpy( pass, _pass );
  }else{
	  strcpy( pass, FTP_PASS );
  }

  dataServer.begin();
  millisDelay = 0;
  cmdStage = FTP_Stop;
  iniVariables();
}

void FtpServer::credentials( const char * _user, const char * _pass )
{
  if( strlen( _user ) > 0 && strlen( _user ) < FTP_CRED_SIZE )
    strcpy( user, _user );
  if( strlen( _pass ) > 0 && strlen( _pass ) < FTP_CRED_SIZE )
    strcpy( pass, _pass );
}

void FtpServer::iniVariables()
{
  // Default for data port
  dataPort = FTP_DATA_PORT_DFLT;
  
  // Default Data connection is Active
  dataConn = FTP_NoConn;
  
  // Set the root directory
  strcpy( cwdName, "/" );

  rnfrCmd = false;
  transferStage = FTP_Close;
  
  _bServerIsInUse = false;
}

uint8_t FtpServer::handleFTP()
{
  #ifdef FTP_DEBUG1
    int8_t data0 = data.status();
    ftpTransfer transferStage0 = transferStage;
    ftpCmd cmdStage0 = cmdStage;
    ftpDataConn dataConn0 = dataConn;
  #endif
  
  if((int32_t) ( millisDelay - millis() ) <= 0 )
  {
		if( cmdStage == FTP_Stop )
		{
		  if( client.connected())
		    disconnectClient();
		  cmdStage = FTP_Init;
		}
		else if( cmdStage == FTP_Init )       // Ftp server waiting for connection
		{
		  abortTransfer();
		  iniVariables();
		  DEBUG_PRINT( F(" Ftp server waiting for connection on port ") ); DEBUG_PRINTLN( cmdPort );

		  cmdStage = FTP_Client;
		}
		else if( cmdStage == FTP_Client )     // Ftp server idle
		{
		  #ifdef ESP8266
		  if( ftpServer.hasClient())
		  {
		    client.stop();
		    client = ftpServer.available();
		  }
		  #else
      if( client && ! client.connected())
        client.stop();
		  client = ftpServer.accept();
		  #endif
		  if( client.connected())             // A client connected
		  {
		    clientConnected();
		    millisEndConnection = millis() + 1000L * FTP_AUTH_TIME_OUT; // wait client id for 10 s.
		    cmdStage = FTP_User;
		  }
		}
		else if( readChar() > 0 )             // got response
		{
		  processCommand();
		  if( cmdStage == FTP_Stop )
		    millisEndConnection = millis() + 1000L * FTP_AUTH_TIME_OUT;  // wait authentication for 10 s.
		  else if( cmdStage < FTP_Cmd )
		    millisDelay = millis() + 200;     // delay of 100 ms
		  else
		    millisEndConnection = millis() + 1000L * FTP_TIME_OUT;
		}
		else if( ! client.connected() ) {
		  cmdStage = FTP_Init;
		}
		if( transferStage == FTP_Retrieve )   // Retrieve data
		{
		  if( ! doRetrieve()) {
		    transferStage = FTP_Close;
		  }
		}
		else if( transferStage == FTP_Store ) // Store data
		{
		  if( ! doStore()) {
		    transferStage = FTP_Close;
		  }
		}
		else if( transferStage == FTP_List ||
		         transferStage == FTP_Nlst)   // LIST or NLST
		{
		  if( ! doList()) {
		    transferStage = FTP_Close;
		  }
		}
		else if( transferStage == FTP_Mlsd )  // MLSD listing
		{
		  if( ! doMlsd()) {
		    transferStage = FTP_Close;
		  }
		}
		else if( cmdStage > FTP_Client &&
		         ! ((int32_t) ( millisEndConnection - millis() ) > 0 ))
		{
		  client.println(F("530 Timeout"));
		  millisDelay = millis() + 200;       // delay of 200 ms
		  cmdStage = FTP_Stop;
		}

		#ifdef FTP_ADDITIONAL_DEBUG
		  uint8_t dstat = data.status();
		  if( cmdStage != cmdStage0 || transferStage != transferStage0 ||
		      dataConn != dataConn0 || dstat != data0 ) {
			  DEBUG_PRINT( F("  Command: ") ); DEBUG_PRINT( cmdStage );
			  DEBUG_PRINT( F("  Transfer: ") ); DEBUG_PRINT( transferStage );
			  DEBUG_PRINT( F("  Data: ") ); DEBUG_PRINTLN( dataConn );
			  DEBUG_PRINT( F("  Data socket: ") ); DEBUG_PRINT( hex ); DEBUG_PRINT( int( dstat )); DEBUG_PRINTLN( dec );
		  }
		#endif
  }
  return cmdStage | ( transferStage << 3 ) | ( dataConn << 6 );
}

bool FtpServer::isInUse() 
{
  return _bServerIsInUse;
}

void FtpServer::clientConnected()
{
	DEBUG_PRINTLN( F(" Client connected!") );
  client.println(F("220--- Welcome to FTP for Arduino ---"));
  client.println(F("220---   By Jean-Michel Gallego   ---"));
  client.print(F("220 --    Version ")); client.print(FTP_SERVER_VERSION); client.println(F("    --"));
  iCL = 0;
  _bServerIsInUse = true;
}

void FtpServer::disconnectClient()
{
	DEBUG_PRINTLN( F(" Disconnecting client") );

  abortTransfer();
  client.println(F("221 Goodbye") );
  if( client ) {
    client.stop();
  }
  if( data ) {
    data.stop();
  }

  _bServerIsInUse = false;
}

bool FtpServer::processCommand()
{
  ///////////////////////////////////////
  //                                   //
  //      AUTHENTICATION COMMANDS      //
  //                                   //
  ///////////////////////////////////////

  //
  //  USER - User Identity 
  //
  if( CommandIs( "USER" ))
  {
    if( ! strcmp( parameter, user ))
    {
      client.println(F("331 Ok. Password required") );
      strcpy( cwdName, "/" );
      cmdStage = FTP_Pass;
    }
    else
    {
      client.println(F("530 ") );
      cmdStage = FTP_Stop;
    }
  }
  //
  //  PASS - Password
  //
  else if( CommandIs( "PASS" ))
  {
    if( cmdStage != FTP_Pass )
    {
      client.println(F("503 ") );
      cmdStage = FTP_Stop;
    }
    if( ! strcmp( parameter, pass ))
    {
    	DEBUG_PRINTLN( F(" Authentication Ok. Waiting for commands.") );

      client.println(F("230 Ok") );
      cmdStage = FTP_Cmd;
    }
    else
    {
    	client.println( F("530 ") );
      cmdStage = FTP_Stop;
    }
  }
  //
  //  FEAT - New Features
  //
  else if( CommandIs( "FEAT" ))
  {
    client.println(F("211-Extensions suported:"));
    client.println(F(" MLST type*;modify*;size*;") );
    client.println(F(" MLSD") );
    client.println(F(" MDTM") );
    client.println(F(" MFMT") );
    client.println(F(" SIZE") );
    client.println(F(" SITE FREE") );
    client.println(F("211 End.") );
  }
  //
  //  AUTH - Not implemented
  //
  else if( CommandIs( "AUTH" ))
    client.println(F("502 ") );
  //
  //  Unrecognized commands at stage of authentication
  //
  else if( cmdStage < FTP_Cmd )
  {
    client.println(F("530 ") );
    cmdStage = FTP_Stop;
  }

  ///////////////////////////////////////
  //                                   //
  //      ACCESS CONTROL COMMANDS      //
  //                                   //
  ///////////////////////////////////////

  //
  //  PWD - Print Directory
  //
  else if( CommandIs( "PWD" ) ||
           ( CommandIs( "CWD" ) && ParameterIs( "." ))) {
	  client.print( F("257 \"")); client.print( cwdName ); client.print( F("\"") ); client.println( F(" is your current directory") );
  //
  //  CDUP - Change to Parent Directory 
  //
  } else if( CommandIs( "CDUP" ) ||
           ( CommandIs( "CWD" ) && ParameterIs( ".." )))
  {
    bool ok = false;
    
    if( strlen( cwdName ) > 1 )            // do nothing if cwdName is root
    {
      // if cwdName ends with '/', remove it (must not append)
      if( cwdName[ strlen( cwdName ) - 1 ] == '/' ) {
        cwdName[ strlen( cwdName ) - 1 ] = 0;
      }
      // search last '/'
      char * pSep = strrchr( cwdName, '/' );
      ok = pSep > cwdName;
      // if found, ends the string on its position
      if( ok )
      {
        * pSep = 0;
        ok = exists( cwdName );
      }
    }
    // if an error appends, move to root
    if( ! ok ) {
      strcpy( cwdName, "/" );
    }
    client.print( F("250 Ok. Current directory is ") ); client.println( cwdName );
  }
  //
  //  CWD - Change Working Directory
  //
  else if( CommandIs( "CWD" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path ))
    {
      strcpy( cwdName, path );
      client.print( F("250 Directory changed to ") ); client.print(cwdName); client.println();
    }
  }
  //
  //  QUIT
  //
  else if( CommandIs( "QUIT" ))
  {
    client.println(F("221 Goodbye") );
    disconnectClient();
    cmdStage = FTP_Stop;
  }

  ///////////////////////////////////////
  //                                   //
  //    TRANSFER PARAMETER COMMANDS    //
  //                                   //
  ///////////////////////////////////////

  //
  //  MODE - Transfer Mode 
  //
  else if( CommandIs( "MODE" ))
  {
    if( ParameterIs( "S" )) {
      client.println(F("200 S Ok") );
    } else {
      client.println(F("504 Only S(tream) is suported") );
    }
  }
  //
  //  PASV - Passive Connection management
  //
  else if( CommandIs( "PASV" ))
  {
    data.stop();
    dataServer.begin();
    if((((uint32_t) NET_CLASS.localIP()) & ((uint32_t) NET_CLASS.subnetMask())) ==
       (((uint32_t) client.remoteIP()) & ((uint32_t) NET_CLASS.subnetMask()))) {
      dataIp = NET_CLASS.localIP();
    } else {
      dataIp = localIp;
    }
    dataPort = pasvPort;
    DEBUG_PRINTLN( F(" Connection management set to passive") );
    DEBUG_PRINT( F(" Listening at ") );
    DEBUG_PRINT( int( dataIp[0]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[1]) ); DEBUG_PRINT( F(".") );
    DEBUG_PRINT( int( dataIp[2]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[3]) );
    DEBUG_PRINT( F(":") ); DEBUG_PRINTLN( dataPort );

    client.print( F("227 Entering Passive Mode") ); client.print( F(" (") );
    client.print( int( dataIp[0]) ); client.print( F(",") ); client.print( int( dataIp[1]) ); client.print( F(",") );
    client.print( int( dataIp[2]) ); client.print( F(",") ); client.print( int( dataIp[3]) ); client.print( F(",") );
    client.print( ( dataPort >> 8 ) ); client.print( F(",") ); client.print( ( dataPort & 255 ) ); client.println( F(")") );
    dataConn = FTP_Pasive;
  }
  //
  //  PORT - Data Port
  //
  else if( CommandIs( "PORT" ))
  {
    data.stop();
    // get IP of data client
    dataIp[ 0 ] = atoi( parameter );
    char * p = strchr( parameter, ',' );
    for( uint8_t i = 1; i < 4; i ++ )
    {
      dataIp[ i ] = atoi( ++ p );
      p = strchr( p, ',' );
    }
    // get port of data client
    dataPort = 256 * atoi( ++ p );
    p = strchr( p, ',' );
    dataPort += atoi( ++ p );
    if( p == NULL ) {
      client.println(F("501 Can't interpret parameters") );
    } else
    {
    	DEBUG_PRINT( F(" Data IP set to ") ); DEBUG_PRINT( int( dataIp[0]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[1]) );
    	DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[2]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINTLN( int( dataIp[3]) );
    	DEBUG_PRINT( F(" Data port set to ") ); DEBUG_PRINTLN( dataPort );

      client.println(F("200 PORT command successful") );
      dataConn = FTP_Active;
    }
  }
  //
  //  STRU - File Structure
  //
  else if( CommandIs( "STRU" ))
  {
    if( ParameterIs( "F" )) {
      client.println(F("200 F Ok") );
    // else if( ParameterIs( "R" ))
    //  client.println(F("200 B Ok") );
    }else{
      client.println(F("504 Only F(ile) is suported") );
    }
  }
  //
  //  TYPE - Data Type
  //
  else if( CommandIs( "TYPE" ))
  {
    if( ParameterIs( "A" )) {
      client.println(F("200 TYPE is now ASCII"));
    } else if( ParameterIs( "I" )) {
      client.println(F("200 TYPE is now 8-bit binary") );
    } else {
      client.println(F("504 Unknow TYPE") );
    }
  }

  ///////////////////////////////////////
  //                                   //
  //        FTP SERVICE COMMANDS       //
  //                                   //
  ///////////////////////////////////////

  //
  //  ABOR - Abort
  //
  else if( CommandIs( "ABOR" ))
  {
    abortTransfer();
    client.println(F("226 Data connection closed"));
  }
  //
  //  DELE - Delete a File 
  //
  else if( CommandIs( "DELE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( remove( path )) {
        client.print( F("250 Deleted ") ); client.println( parameter );
      } else {
    	  client.print( F("450 Can't delete ") ); client.println( parameter );
      }
    }
  }
  //
  //  LIST - List
  //  NLST - Name List
  //  MLSD - Listing for Machine Processing (see RFC 3659)
  //
  else if( CommandIs( "LIST" ) || CommandIs( "NLST" ) || CommandIs( "MLSD" ))
  {
    if( dataConnect()){
      if( openDir( & dir ))
      {
        nbMatch = 0;
        if( CommandIs( "LIST" ))
          transferStage = FTP_List;
        else if( CommandIs( "NLST" ))
          transferStage = FTP_Nlst;
        else
          transferStage = FTP_Mlsd;
      }
      else
        data.stop();
    }
  }
  //
  //  MLST - Listing for Machine Processing (see RFC 3659)
  //
  else if( CommandIs( "MLST" ))
  {
    char path[ FTP_CWD_SIZE ];
    uint16_t dat, tim;
    char dtStr[ 15 ];
    bool isdir;
    if( haveParameter() && makeExistsPath( path )){
      if( ! getFileModTime( path, & dat, & tim )) {
        client.print( F("550 Unable to retrieve time for ") ); client.println( parameter );
      } else
      {
        isdir = isDir( path );
        client.println( F("250-Begin") );
        client.print( F(" Type=") ); client.print( ( isdir ? F("dir") : F("file")) );
        client.print( F(";Modify=") ); client.print( makeDateTimeStr( dtStr, dat, tim ) );
        if( ! isdir )
        {
          if( openFile( path, O_FTP_FILE_READ ))
          {
            client.print( F(";Size=") ); client.print( long( fileSize( file )) );
            file.close();
          }
        }
        client.print( F("; ") ); client.println( path );
        client.println( F("250 End.") );
      }
    }
  }
  //
  //  NOOP
  //
  else if( CommandIs( "NOOP" )) {
    client.println(F("200 Zzz...") );
  }
  //
  //  RETR - Retrieve
  //
  else if( CommandIs( "RETR" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( ! openFile( path, O_FTP_FILE_READ )) {
        client.print( F("450 Can't open ") ); client.print( parameter );
      } else if( dataConnect( false ))
      {
    	  DEBUG_PRINT( F(" Sending ") ); DEBUG_PRINT( parameter ); DEBUG_PRINT( F(" size ") ); DEBUG_PRINTLN( long( fileSize( file ))  );

        client.print( F("150-Connected to port ") ); client.println( dataPort );
        client.print( F("150 ") ); client.print( long( fileSize( file )) ); client.println( F(" bytes to download") );
        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStage = FTP_Retrieve;
      }
    }
  }
  //
  //  STOR - Store
  //  APPE - Append
  //
  else if( CommandIs( "STOR" ) || CommandIs( "APPE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makePath( path ))
    {
      bool open;
      if( exists( path )) {
#if (STORAGE_TYPE == STORAGE_SPIFFS) || (STORAGE_TYPE == STORAGE_LITTLEFS) || (STORAGE_TYPE == STORAGE_SD2)
        open = openFile( path, ( CommandIs( "APPE" ) ? O_FTP_FILE_APPEND : O_FTP_FILE_CREAT ));
#else
        open = openFile( path, O_WRITE | ( CommandIs( "APPE" ) ? O_APPEND : O_CREAT ));
#endif
      } else {
        open = openFile( path, O_FTP_FILE_CREAT );
      }

      data.stop();
      data.flush();

      DEBUG_PRINT(F("open/create "));
      DEBUG_PRINTLN(open);
      if( ! open ){
    	  client.print( F("451 Can't open/create ") ); client.println( parameter );
      }else if( ! dataConnect()) // && !data.available())
        file.close();
      else
      {
    	  DEBUG_PRINT( F(" Receiving ") ); DEBUG_PRINTLN( parameter );

        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStage = FTP_Store;
      }
    }
  }
  //
  //  MKD - Make Directory
  //
  else if( CommandIs( "MKD" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makePath( path ))
    {
      if( exists( path )) {
    	  client.print( F("521 \"") ); client.print( parameter ); client.println( F("\" directory already exists") );
      } else
      {
    	  DEBUG_PRINT( F(" Creating directory ")); DEBUG_PRINTLN( parameter );

#if STORAGE_TYPE != STORAGE_SPIFFS
        if( makeDir( path )) {
        	client.print( F("257 \"") ); client.print( parameter ); client.print( F("\"") ); client.println( F(" created") );
        } else {
#endif
        	client.print( F("550 Can't create \"") ); client.print( parameter ); client.println( F("\"") );
#if STORAGE_TYPE != STORAGE_SPIFFS
        }
#endif
      }
    }
  }
  //
  //  RMD - Remove a Directory 
  //
  else if( CommandIs( "RMD" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( removeDir( path ))
      {
    	  DEBUG_PRINT( F(" Deleting ") ); DEBUG_PRINTLN( path );

        client.print( F("250 \"") ); client.print( parameter ); client.println( F("\" deleted") );
      }
      else {
    	  client.print( F("550 Can't remove \"") ); client.print( parameter ); client.println( F("\". Directory not empty?") );
      }
    }
  }
  //
  //  RNFR - Rename From 
  //
  else if( CommandIs( "RNFR" ))
  {
    rnfrName[ 0 ] = 0;
    if( haveParameter() && makeExistsPath( rnfrName ))
    {
    	DEBUG_PRINT( F(" Ready for renaming ") ); DEBUG_PRINTLN( rnfrName );

      client.println(F("350 RNFR accepted - file exists, ready for destination") );
      rnfrCmd = true;
    }
  }
  //
  //  RNTO - Rename To 
  //
  else if( CommandIs( "RNTO" ))
  {
    char path[ FTP_CWD_SIZE ];
    char dirp[ FTP_FIL_SIZE ];
    if( strlen( rnfrName ) == 0 || ! rnfrCmd ) {
      client.println(F("503 Need RNFR before RNTO") );
    } else if( haveParameter() && makePath( path ))
    {
      if( exists( path )) {
        client.print( F("553 ") ); client.print( parameter ); client.println( F(" already exists") );
      } else
      {
        strcpy( dirp, path );
        char * psep = strrchr( dirp, '/' );
        bool fail = psep == NULL;
        if( ! fail )
        {
          if( psep == dirp )
            psep ++;
          * psep = 0;
//          fail = ! isDir( dirp );
//          if( fail ) {
//        	  client.print( F("550 \"") ); client.print( dirp ); client.println( F("\" is not directory") );
//          } else
//          {
        	  DEBUG_PRINT( F(" Renaming ") ); DEBUG_PRINT( rnfrName ); DEBUG_PRINT( F(" to ") ); DEBUG_PRINTLN( path );

            if( rename( rnfrName, path ))
              client.println(F("250 File successfully renamed or moved") );
            else
              fail = true;
//          }
        }
        if( fail )
          client.println(F("451 Rename/move failure") );
      }
    }
    rnfrCmd = false;
  }
  /*
  //
  //  SYST - System
  //
  else if( CommandIs( "SYST" ))
    FtpOutCli << F("215 MSDOS") << endl;
  */
  
  ///////////////////////////////////////
  //                                   //
  //   EXTENSIONS COMMANDS (RFC 3659)  //
  //                                   //
  ///////////////////////////////////////

  //
  //  MDTM && MFMT - File Modification Time (see RFC 3659)
  //
  else if( CommandIs( "MDTM" ) || CommandIs( "MFMT" ))
  {
    if( haveParameter())
    {
      char path[ FTP_CWD_SIZE ];
      char * fname = parameter;
      uint16_t year;
      uint8_t month, day, hour, minute, second, setTime;
      char dt[ 15 ];
      bool mdtm = CommandIs( "MDTM" );

      setTime = getDateTime( dt, & year, & month, & day, & hour, & minute, & second );
      // fname point to file name
      fname += setTime;
      if( strlen( fname ) <= 0 ) {
        client.println(F("501 No file name") );
      } else if( makeExistsPath( path, fname )) {
        if( setTime ) // set file modification time
        {
          if( timeStamp( path, year, month, day, hour, minute, second )) {
            client.print( F("213 ") ); client.println( dt );
          } else {
            client.println(F("550 Unable to modify time" ));
          }
        }
        else if( mdtm ) // get file modification time
        {
          uint16_t dat, tim;
          char dtStr[ 15 ];
          if( getFileModTime( path, & dat, & tim )) {
            client.print( F("213 ") ); client.println( makeDateTimeStr( dtStr, dat, tim ) );
          } else {
            client.println("550 Unable to retrieve time" );
          }
        }
      }
    }
  }
  //
  //  SIZE - Size of the file
  //
  else if( CommandIs( "SIZE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( ! openFile( path, O_FTP_FILE_READ )) {
        client.print( F("450 Can't open ") ); client.println( parameter );
      } else
      {
        client.print( F("213 ") ); client.println( long( fileSize( file )) );
        file.close();
      }
    }
  }
  //
  //  SITE - System command
  //
  else if( CommandIs( "SITE" ))
  {
    if( ParameterIs( "FREE" ))
    {
      uint32_t capa = capacity();
      if(( capa >> 10 ) < 1000 ) { // less than 1 Giga
        client.print( F("200 ") ); client.print( free() ); client.print( F(" kB free of ") );
        client.print( capa ); client.println( F(" kB capacity") );
      }else {
    	  client.print( F("200 ") ); client.print( ( free() >> 10 ) ); client.print( F(" MB free of ") );
    	  client.print( ( capa >> 10 ) ); client.println( F(" MB capacity") );
      }
    }
    else {
    	client.print( F("500 Unknow SITE command ") ); client.println( parameter );
    }
  }
  //
  //  Unrecognized commands ...
  //
  else
    client.println(F("500 Unknow command") );
  return true;
}

int FtpServer::dataConnect( bool out150 )
{
  if( ! data.connected()) {
    if( dataConn == FTP_Pasive )
    {
      uint16_t count = 1000; // wait up to a second
      while( ! data.connected() && count -- > 0 )
      {
        #ifdef ESP8266
			if( dataServer.hasClient())
			{
			  data.stop();
			  data = dataServer.available();
			}
        #else
        data = dataServer.accept();
        #endif
        delay( 1 );
      }
    }
    else if( dataConn == FTP_Active )
      data.connect( dataIp, dataPort );
  }

//#ifdef ESP8266
  if( ! ( data.connected() || data.available())) {
//#else
//	  if( ! ( data.connected() )) {
//#endif
    client.println(F("425 No data connection"));
  } else if( out150 ) {
    client.print( F("150 Accepted data connection to port ") ); client.println( dataPort );
  }
//#ifdef ESP8266
	  return  data.connected() || data.available();
//#else
//	  return  data.connected();
//#endif

}

bool FtpServer::dataConnected()
{
  if( data.connected())
    return true;
  data.stop();
  client.println(F("426 Data connection closed. Transfer aborted") );
  transferStage = FTP_Close;
  return false;
}
 
bool FtpServer::openDir( FTP_DIR * pdir )
{
  bool openD;
#if STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD2
  #if STORAGE_TYPE == STORAGE_SD2
    //dir.setTimeCallback(fsTimeStampCallback);
  #endif

  if( cwdName == 0 ) {
  
    #if STORAGE_TYPE == STORAGE_SD
	    dir = STORAGE_MANAGER.open( "/" );
	  } else {
	    dir = STORAGE_MANAGER.open( cwdName );
	  }
	  openD = true;
    
    #elif STORAGE_TYPE == STORAGE_SD2
      dir = STORAGE_MANAGER.openDir( "/" );
    } else {
      dir = STORAGE_MANAGER.openDir( cwdName );
    }
    openD = dir.rewind();
       
    #else
      dir = STORAGE_MANAGER.openDir( "/" );
    } else {
      dir = STORAGE_MANAGER.openDir( cwdName );
    }
    openD = dir.rewind();
  #endif

  if( ! openD ) {
    client.print( F("550 Can't open directory ") ); client.println( cwdName );
  }

#elif STORAGE_TYPE == STORAGE_SPIFFS
  if( cwdName == 0 || strcmp(cwdName, "/") == 0 ) {
	  DEBUG_PRINT("DIRECTORY / EXIST ");
#if ESP8266
	  dir = STORAGE_MANAGER.openDir( "/" );
#else
	  dir = STORAGE_MANAGER.open( "/" );
#endif
	  openD = true;

    } else {
//  	  openD = STORAGE_MANAGER.exists( cwdName );
//	  DEBUG_PRINT("DIRECTORY ");
//	  DEBUG_PRINT(cwdName);
//	  DEBUG_PRINTLN(openD);
//
//  	  if (openD){
//        dir = STORAGE_MANAGER.openDir( cwdName );
//  	  }
    	openD = false;
    }
    if( ! openD ) {
      client.print( F("550 Can't open directory ") ); client.println( cwdName );
    }
#else
  if( cwdName == 0 ) {
    openD = pdir->open( "/" );
  } else {
    openD = pdir->open( cwdName );
  }
  if( ! openD ) {
    client.print( F("550 Can't open directory ") ); client.println( cwdName );
  }
#endif
  return openD;
}

bool FtpServer::doRetrieve()
{
  if( ! dataConnected())
  {
    file.close();
    return false;
  }
  int16_t nb = file.read( buf, FTP_BUF_SIZE );
  if( nb > 0 )
  {
    data.write( buf, nb );
    DEBUG_PRINT(F("NB --> "));
    DEBUG_PRINTLN(nb);
    bytesTransfered += nb;
    return true;
  }
  closeTransfer();
  return false;
}

bool FtpServer::doStore()
{
  int16_t na = data.available();
  if( na == 0 ) {
	  DEBUG_PRINTLN("NO DATA AVAILABLE!");
    if( data.connected()) {
      return true;
    } else
    {
      closeTransfer();
      return false;
    }
  }
  if( na > FTP_BUF_SIZE ) {
    na = FTP_BUF_SIZE;
  }
  int16_t nb = data.read((uint8_t *) buf, na );
  int16_t rc = 0;
  if( nb > 0 )
  {
	    DEBUG_PRINT("NB -> ");
	    DEBUG_PRINTLN(nb);

    // FtpDebug << millis() << " " << nb << endl;
    rc = file.write( buf, nb );
    DEBUG_PRINT("RC -> ");
    DEBUG_PRINTLN(rc);
    bytesTransfered += nb;
  }
  if( nb < 0 || rc == nb  ) {
    return true;
  }
  client.println(F("552 Probably insufficient storage space") );
  file.close();
  data.stop();
  return false;
}

bool FtpServer::doList()
{
  if( ! dataConnected())
  {
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SD2
    dir.close();
#endif
    return false;
  }
#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD2
  #if ESP8266 && STORAGE_TYPE != STORAGE_SD // && STORAGE_TYPE != STORAGE_SD2
    if( dir.next())
  #else
    File fileDir = dir.openNextFile();
    if( fileDir )
  #endif
    {

  #if (STORAGE_TYPE == STORAGE_LITTLEFS)
	  if( dir.isDirectory()) {
		  data.print( F("+/,\t") );
	  } else {
  #endif
	data.print( F("+r,s") );
  #if ESP8266 && STORAGE_TYPE != STORAGE_SD // && STORAGE_TYPE != STORAGE_SD2
	data.print( long( dir.fileSize()) );
	data.print( F(",\t") );
    data.println( dir.fileName() );
  #else
	data.print( long( fileDir.size()) );
	data.print( F(",\t") );
    data.println( fileDir.name() );
  #endif

  #if (STORAGE_TYPE == STORAGE_LITTLEFS)
	  }
  #endif

    nbMatch ++;
    return true;
  }

#elif STORAGE_TYPE == STORAGE_FATFS
  if( dir.nextFile())
  {
    if( dir.isDir()) {
      data.print( F("+/,\t") );
    } else {
    	data.print( F("+r,s") ); data.print( long( dir.fileSize()) ); data.print( F(",\t") );
    }
    data.println( dir.fileName() );
    nbMatch ++;
    return true;
  }
#else
  if( file.openNext( &dir, O_RDONLY ))
  {
    if( file.isDir()) {
      data.print( F("+/,\t") );
    } else {
    	data.print( F("+r,s") ); data.print( long( fileSize( file )) ); data.print( F(",\t") );
    }
    file.printName( & data );
    data.println();
    file.close();
    nbMatch ++;
    return true;
  }
#endif
  client.print( F("226 ") ); client.print( nbMatch ); client.println( F(" matches total") );
#if STORAGE_TYPE != STORAGE_SPIFFS  && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SD2
  dir.close();
#endif
  data.stop();
  return false;
}


bool FtpServer::doMlsd()
{
  // Check to see if connected. Return false if not
  if( ! dataConnected())
  { 
  #if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SD2
    dir.close();
  #endif
    return false;
  }

  // Get next file  
  #if STORAGE_TYPE == STORAGE_SPIFFS  || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD2
    DEBUG_PRINTLN("DIR MLSD ");
    #if ESP8266 && STORAGE_TYPE != STORAGE_SD // && STORAGE_TYPE != STORAGE_SD2
      if( dir.next())
    #else
      File fileDir = dir.openNextFile();
      if( fileDir )
    #endif
      { // if ( dir.next() / if( fileDir ))
        DEBUG_PRINTLN("DIR NEXT ");
        char dtStr[ 15 ];

        // Get the file's datestamp
        #if STORAGE_TYPE == STORAGE_SD
          strcpy(dtStr, "19700101000000");

        #elif STORAGE_TYPE == STORAGE_SD2
          //time_t t = fileDir.getLastWrite();
          time_t t = dir.fileTime();
          //t = now();
          //strcpy(dtStr, "19710101000000");
          
          //timeinfo = localtime ( &time );
          // 2000 01 01 16 06 56
          
          sprintf(dtStr,"%04d%02d%02d%02d%02d%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));

          //strftime (dtStr,15,"%Y%m%d%H%M%S",timeinfo);       
        #else
          struct tm * timeinfo;
          #if ESP8266
            time_t time = dir.fileTime();
          #else
            time_t time = fileDir.getLastWrite();
          #endif

          timeinfo = localtime ( &time );

          // 2000 01 01 16 06 56

          strftime (dtStr,15,"%Y%m%d%H%M%S",timeinfo);       
        #endif

        #if ESP8266 && STORAGE_TYPE != STORAGE_SD // && STORAGE_TYPE != STORAGE_SD2
          // Get the filename fn
          String fn = dir.fileName();
          #if STORAGE_TYPE == STORAGE_SPIFFS
            fn.remove(0, 1);
          #endif
          // Get the filesize fz
          long fz = dir.fileSize();
        #else
          String fn = fileDir.name();
          #if STORAGE_TYPE != STORAGE_SD
            fn.remove(0, 1);
          #endif
          long fz = fileDir.size();
        #endif

        data.print( F("Type=") );

        #if STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD2
          // Send the kind of file (file or dir) to the client
          data.print( ( dir.isDirectory() ? F("dir") : F("file")) );

        #elif STORAGE_TYPE == STORAGE_SD
          data.print( ( fileDir.isDirectory() ? F("dir") : F("file")) );

        #else
          data.print( F("file") );

        #endif
        
        // Send datestamp dtstr, size fz and filename fn to the client
        data.print( F(";Modify=") ); data.print(dtStr);// data.print( makeDateTimeStr( dtStr, time, time) );
        data.print( F(";Size=") ); data.print( fz );
        data.print( F("; ") ); data.println( fn );

        DEBUG_PRINT( F("Type=") );

        #if STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD2
          DEBUG_PRINT( ( dir.isDirectory() ? F("dir") : F("file")) );
        
        #elif STORAGE_TYPE == STORAGE_SD
          DEBUG_PRINT( ( fileDir.isDirectory() ? F("dir") : F("file")) );
        
        #else
          DEBUG_PRINT( F("file") );
        
        #endif

        DEBUG_PRINT( F(";Modify=") ); DEBUG_PRINT(dtStr); //DEBUG_PRINT( makeDateTimeStr( dtStr, time, time) );
        DEBUG_PRINT( F(";Size=") ); DEBUG_PRINT( fz );
        DEBUG_PRINT( F("; ") ); DEBUG_PRINTLN( fn );
        
        nbMatch ++;
        return true;
      } // if ( dir.next() / if( fileDir ))

  #elif STORAGE_TYPE == STORAGE_FATFS
    if( dir.nextFile())
    {
      char dtStr[ 15 ];
      data.print( F("Type=") ); data.print( ( dir.isDir() ? F("dir") : F("file")) );
      data.print( F(";Modify=") ); data.print( makeDateTimeStr( dtStr, dir.fileModDate(), dir.fileModTime()) );
      data.print( F(";Size=") ); data.print( long( dir.fileSize()) );
      data.print( F("; ") ); data.println( dir.fileName() );
      nbMatch ++;
      return true;
    }
  #else
    if( file.openNext( &dir, O_RDONLY ))
    {
      char dtStr[ 15 ];
      uint16_t filelwd, filelwt;
      bool gfmt = getFileModTime( & filelwd, & filelwt );
      DEBUG_PRINT("gfmt --> ");
      DEBUG_PRINTLN(gfmt);
      if( gfmt )
      {
        data.print( F("Type=") ); data.print( ( file.isDir() ? F("dir") : F("file")) );
        data.print( F(";Modify=") ); data.print( makeDateTimeStr( dtStr, filelwd, filelwt ) );
        data.print( F(";Size=") ); data.print( long( fileSize( file )) ); data.print( F("; ") );
        file.printName( & data );
        data.println();

        DEBUG_PRINT( F("Type=") ); DEBUG_PRINT( ( file.isDir() ? F("dir") : F("file")) );
        DEBUG_PRINT( F(";Modify=") ); DEBUG_PRINT( makeDateTimeStr( dtStr, filelwd, filelwt ) );
        DEBUG_PRINT( F(";Size=") ); DEBUG_PRINT( long( fileSize( file )) ); DEBUG_PRINT( F("; ") );
        // DEBUG_PRINT(file.name());
        DEBUG_PRINTLN();
        nbMatch ++;
      }
      file.close();
      return gfmt;
    }
  #endif
  client.println(F("226-options: -a -l") );
  client.print( F("226 ") ); client.print( nbMatch ); client.println( F(" matches total") );
  #if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SD2
    dir.close();
  #endif
  data.stop();
  return false;
}

void FtpServer::closeTransfer()
{
  uint32_t deltaT = (int32_t) ( millis() - millisBeginTrans );
  if( deltaT > 0 && bytesTransfered > 0 )
  {
	  DEBUG_PRINT( F(" Transfer completed in ") ); DEBUG_PRINT( deltaT ); DEBUG_PRINTLN( F(" ms, ") );
	  DEBUG_PRINT( bytesTransfered / deltaT ); DEBUG_PRINTLN( F(" kbytes/s") );

    client.println(F("226-File successfully transferred") );
    client.print( F("226 ") ); client.print( deltaT ); client.print( F(" ms, ") );
    client.print( bytesTransfered / deltaT ); client.println( F(" kbytes/s") );
  }
  else
    client.println(F("226 File successfully transferred") );
  
  file.close();
  data.stop();
}

void FtpServer::abortTransfer()
{
  if( transferStage != FTP_Close )
  {
    file.close();
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SD2
    dir.close();
#endif
    client.println(F("426 Transfer aborted") );
    DEBUG_PRINTLN( F(" Transfer aborted!") );

    transferStage = FTP_Close;
  }
//  if( data.connected())
  data.stop(); 
}

// Read a char from client connected to ftp server
//
//  update cmdLine and command buffers, iCL and parameter pointers
//
//  return:
//    -2 if buffer cmdLine is full
//    -1 if line not completed
//     0 if empty line received
//    length of cmdLine (positive) if no empty line received 

int8_t FtpServer::readChar()
{
  int8_t rc = -1;

  if( client.available())
  {
    char c = client.read();
    DEBUG_PRINT( c );

    if( c == '\\' )
      c = '/';
    if( c != '\r' ){
      if( c != '\n' )
      {
        if( iCL < FTP_CMD_SIZE )
          cmdLine[ iCL ++ ] = c;
        else
          rc = -2; //  Line too long
      }
      else
      {
        cmdLine[ iCL ] = 0;
        command[ 0 ] = 0;
        parameter = NULL;
        // empty line?
        if( iCL == 0 )
          rc = 0;
        else
        {
          rc = iCL;
          // search for space between command and parameter
          parameter = strchr( cmdLine, ' ' );
          if( parameter != NULL )
          {
            if( parameter - cmdLine > 4 )
              rc = -2; // Syntax error
            else
            {
              strncpy( command, cmdLine, parameter - cmdLine );
              command[ parameter - cmdLine ] = 0;
              while( * ( ++ parameter ) == ' ' )
                ;
            }
          }
          else if( strlen( cmdLine ) > 4 )
            rc = -2; // Syntax error.
          else
            strcpy( command, cmdLine );
          iCL = 0;
        }
      }
    }
    if( rc > 0 )
      for( uint8_t i = 0 ; i < strlen( command ); i ++ )
        command[ i ] = toupper( command[ i ] );
    if( rc == -2 )
    {
      iCL = 0;
      client.println(F("500 Syntax error"));
    }
  }
  return rc;
}

bool FtpServer::haveParameter()
{
  if( parameter != NULL && strlen( parameter ) > 0 )
    return true;
  client.println("501 No file name" );
  return false;  
}

// Make complete path/name from cwdName and param
//
// 3 possible cases: param can be absolute path, relative path or only the name
//
// parameter:
//   fullName : where to store the path/name
//
// return:
//    true, if done

bool FtpServer::makePath( char * fullName, char * param )
{
  if( param == NULL )
    param = parameter;
    
  // Root or empty?
  if( strcmp( param, "/" ) == 0 || strlen( param ) == 0 )
  {
    strcpy( fullName, "/" );
    return true;
  }
  // If relative path, concatenate with current dir
  if( param[0] != '/' ) 
  {
    strcpy( fullName, cwdName );
    if( fullName[ strlen( fullName ) - 1 ] != '/' )
      strncat( fullName, "/", FTP_CWD_SIZE );
    strncat( fullName, param, FTP_CWD_SIZE );
  }
  else
    strcpy( fullName, param );
  // If ends with '/', remove it
  uint16_t strl = strlen( fullName ) - 1;
  if( fullName[ strl ] == '/' && strl > 1 )
    fullName[ strl ] = 0;
  if( strlen( fullName ) >= FTP_CWD_SIZE )
  {
    client.println(F("500 Command line too long"));
    return false;
  }
  for( uint8_t i = 0; i < strlen( fullName ); i ++ )
    if( ! legalChar( fullName[i]))
    {
      client.println(F("553 File name not allowed") );
      return false;
    }
  return true;
}

bool FtpServer::makeExistsPath( char * path, char * param )
{
  if( ! makePath( path, param ))
    return false;
#if STORAGE_TYPE == STORAGE_SPIFFS
  if (strcmp(path, "/") == 0)  return true;
#endif
  DEBUG_PRINT("PATH --> ")
  DEBUG_PRINT(path)
  if( exists( path )) {
	  DEBUG_PRINTLN(" ...EXIST!")
    return true;
  }
  DEBUG_PRINTLN(" ...NOT EXIST!")
  client.print(F("550 ")); client.print( path ); client.println( F(" not found.") );
  return false;
}

// Calculate year, month, day, hour, minute and second
//   from first parameter sent by MDTM command (YYYYMMDDHHMMSS)
// Accept longer parameter YYYYMMDDHHMMSSmmm where mmm are milliseconds
//   but don't take in account additional digits
//
// parameters:
//   dt: 15 length string for 14 digits and terminator
//   pyear, pmonth, pday, phour, pminute and psecond: pointer of
//     variables where to store data
//
// return:
//    0 if parameter is not YYYYMMDDHHMMSS
//    length of parameter + space
//
// Date/time are expressed as a 14 digits long string
//   terminated by a space and followed by name of file

uint8_t FtpServer::getDateTime( char * dt, uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                                uint8_t * phour, uint8_t * pminute, uint8_t * psecond )
{
  uint8_t i;
  dt[ 0 ] = 0;
  if( strlen( parameter ) < 15 ) //|| parameter[ 14 ] != ' ' )
    return 0;
  for( i = 0; i < 14; i ++ )
    if( ! isdigit( parameter[ i ]))
      return 0;
  for( i = 14; i < 18; i ++ )
    if( parameter[ i ] == ' ' )
      break;
    else if( ! isdigit( parameter[ i ]))
      return 0;
  if( i == 18 )
    return 0;
  i ++ ;
  
  strncpy( dt, parameter, 14 );
  dt[ 14 ] = 0;
  * psecond = atoi( dt + 12 ); 
  dt[ 12 ] = 0;
  * pminute = atoi( dt + 10 );
  dt[ 10 ] = 0;
  * phour = atoi( dt + 8 );
  dt[ 8 ] = 0;
  * pday = atoi( dt + 6 );
  dt[ 6 ] = 0 ;
  * pmonth = atoi( dt + 4 );
  dt[ 4 ] = 0 ;
  * pyear = atoi( dt );
  strncpy( dt, parameter, 14 );
  DEBUG_PRINT( F(" Modification time: ") ); DEBUG_PRINT( * pyear ); DEBUG_PRINT( F("/") ); DEBUG_PRINT( int(* pmonth) ); DEBUG_PRINT( F("/") ); DEBUG_PRINT( int(* pday) );
  DEBUG_PRINT( F(" ") ); DEBUG_PRINT( int(* phour) ); DEBUG_PRINT( F(":") ); DEBUG_PRINT( int(* pminute) ); DEBUG_PRINT( F(":") ); DEBUG_PRINT( int(* psecond) );
  DEBUG_PRINT( F(" of file: ") ); DEBUG_PRINTLN( (char *) ( parameter + i ) );

  return i;
}

// Create string YYYYMMDDHHMMSS from date and time
//
// parameters:
//    date, time 
//    tstr: where to store the string. Must be at least 15 characters long
//
// return:
//    pointer to tstr

char * FtpServer::makeDateTimeStr( char * tstr, uint16_t date, uint16_t time )
{
  sprintf( tstr, "%04u%02u%02u%02u%02u%02u",
           (( date & 0xFE00 ) >> 9 ) + 1980, ( date & 0x01E0 ) >> 5, date & 0x001F,
           ( time & 0xF800 ) >> 11, ( time & 0x07E0 ) >> 5, ( time & 0x001F ) << 1 );            
  return tstr;
}


uint16_t FtpServer::fileSize( FTP_FILE file ) {
#if (STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD2)
	return file.size();
#else
	return file.fileSize();
#endif
}

#if ESP32 && (STORAGE_TYPE == STORAGE_SPIFFS )
bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], int readTypeInt ) {
	String readType;
	if (readTypeInt == 0) {
		readType = "r";
	}else if (readTypeInt == 1) {
		readType = "w";
	}else if (readTypeInt == 2) {
		readType = "w";
	}else if (readTypeInt == 0x0008) {
		readType = "a";
	}else if (readTypeInt == 0x0200) {
		readType = "w";
	}else{
		readType = "w";
	}
	DEBUG_PRINT("READ TYPE --> ");
	DEBUG_PRINTLN(readType);
	return openFile( (const char*) path, readType.c_str() );
}
#endif

#if (STORAGE_TYPE == STORAGE_SD2)
bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], const char * readType ) {
	DEBUG_PRINT(F("File to open [") );
	DEBUG_PRINT( path );
	DEBUG_PRINT(F("] readType ") );
  DEBUG_PRINT(readType);
	DEBUG_PRINT(F(": ") );
	/*
  DEBUG_PRINT(readTypeInt);
  DEBUG_PRINT(F("] accessmode [") );
	DEBUG_PRINT(accessmode);
	DEBUG_PRINT(F("] openmode [") );
	DEBUG_PRINT(openmode);
	DEBUG_PRINT(F("]: ") );
	file = STORAGE_MANAGER.open( path, openmode, accessmode );
  */
	//file.setTimeCallback(fsTimeStampCallback);
  file = STORAGE_MANAGER.open( path, readType );
	if (!file) {
		DEBUG_PRINTLN(F("FALSE"));
		return false;
	}else{
    DEBUG_PRINTLN(F("TRUE"));
		return true;
	}
}

/*
time_t fsTimeStampCallback() {
  #ifdef TIME_64BIT
    time_t t = now32();
    Serial.printf("fsTimeStampCallback() (32bit): %2d/%2d/%4d %2d:%2d:%2d", year(t), month(t), day(t), hour(t), minute(t), second(t));
    return t;
  #else
    time64_t t = now();
    Serial.printf("fsTimeStampCallback() (64bit): %2d/%2d/%4d %2d:%2d:%2d", year(t), month(t), day(t), hour(t), minute(t), second(t));
    return (time_t)t;
  #endif
}
*/

#elif (STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD)
bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], const char * readType ) {
	return openFile( (const char*) path, readType );
}
bool FtpServer::openFile( const char * path, const char * readType ) {
		DEBUG_PRINT(F("File to open ") );
		DEBUG_PRINT( path );
		DEBUG_PRINT(F(" readType ") );
		DEBUG_PRINTLN(readType);
#if (STORAGE_TYPE == STORAGE_SD)
		if (readType == 0X01) {
			readType = FILE_READ;
		}else {
			readType = FILE_WRITE;
		}
#endif
		file = STORAGE_MANAGER.open( path, readType );
		if (!file && readType[0]=='r') {
			return false;
		}else{
			DEBUG_PRINTLN("TRUE");

			return true;
		}
}
#else
bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], int readType ) {
#if (STORAGE_TYPE == STORAGE_SDFAT1)

	if (readType == 0X00 || readType == 0X01) {
		readType = FILE_READ;
	}else {
		readType = FILE_WRITE;
	}
#elif (STORAGE_TYPE == STORAGE_SDFAT2)

	if (readType == 0X01) {
		readType = FILE_READ;
	}else {
		readType = FILE_WRITE;
	}
#endif
	file.close();
	return file.open( (const char*) path, readType );
}
#endif
// Return true if path points to a directory
bool FtpServer::isDir( char * path )
{
#if STORAGE_TYPE == STORAGE_LITTLEFS
  FTP_DIR dir;
  bool res;
  dir = STORAGE_MANAGER.openDir( path );

  return true;
  //	  res = dir.isDirectory();
  //	  return res;

#elif STORAGE_TYPE == STORAGE_SPIFFS
	if (strcmp(path, "/") == 0)  { return true; }
	return false; // no directory support

#elif STORAGE_TYPE == STORAGE_FATFS 
  return STORAGE_MANAGER.isDir( path );

#elif STORAGE_TYPE == STORAGE_SD2
  FTP_FILE file;
  bool res;
  if (strcmp(path, "/") == 0) return true;
  if( ! openFile( path, O_FTP_FILE_READ )) {
    return false;
  }
  res = file.isDirectory();
  file.close();
  DEBUG_PRINT(path);
  DEBUG_PRINT(" IS DIRECTORY --> ");
  DEBUG_PRINTLN(res);
  return res;

#elif STORAGE_TYPE == STORAGE_SDFAT1 || STORAGE_TYPE == STORAGE_SDFAT2
//  bool res = (!dir.open(path, O_READ) || !dir.isDir());
//  dir.close();
//  return res;
  if (strcmp(path, "/") == 0)  { return true; }
  if( ! openFile( path, O_READ )) {
      return false;
    }
  return true;
#else
  FTP_FILE file;
  bool res;
  
  if( ! openFile( path, O_READ )) {
    return false;
  }
#if STORAGE_TYPE == STORAGE_SD
//  if (strcmp(path, "/") == 0) return true;
//  res = file.isDirectory();
//  DEBUG_PRINT(path);
//  DEBUG_PRINT(" IS DIRECOTORY --> ");
//  DEBUG_PRINTLN(res);
  return true;
#else
//  res = file.isDir();
//  DEBUG_PRINT("IS DIRECTORY --> " );
//  DEBUG_PRINTLN(res);
#endif
  file.close();
  return res;
#endif
}

bool FtpServer::timeStamp( char * path, uint16_t year, uint8_t month, uint8_t day,
                           uint8_t hour, uint8_t minute, uint8_t second )
{
#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD2
//	struct tm tmDate = { second, minute, hour, day, month, year };
//    time_t rawtime = mktime(&tmDate);

    return true;
	// setTime(rawtime);
	// SPIFFS USE time() call
//  return STORAGE_MANAGER.timeStamp( path, year, month, day, hour, minute, second );
#elif STORAGE_TYPE == STORAGE_FATFS
  return STORAGE_MANAGER.timeStamp( path, year, month, day, hour, minute, second );
#else
  FTP_FILE file;
  bool res;

  if( ! openFile( path, O_RDWR ))
    return false;
  res = file.timestamp( T_WRITE, year, month, day, hour, minute, second );
  file.close();
  return res;
#endif
}
                        
bool FtpServer::getFileModTime( char * path, uint16_t * pdate, uint16_t * ptime )
{
#if STORAGE_TYPE == STORAGE_FATFS
  return STORAGE_MANAGER.getFileModTime( path, pdate, ptime );

#elif STORAGE_TYPE == STORAGE_SD2
  bool res;

  if( ! openFile( path, O_FTP_FILE_READ )) {
    return false;
  }
  time_t ft = file.getLastWrite();

 	tmElements_t ts;
	breakTime(ft, ts);

  *pdate = FAT_DATE(ts.Year, ts.Month, ts.Day);
  *ptime = FAT_TIME(ts.Hour, ts.Minute, ts.Second);

  file.close();
  return true;

#else
//  FTP_FILE file;
  bool res;

  if( ! openFile( path, O_READ )) {
    return false;
  }
  res = getFileModTime( pdate, ptime );
  file.close();
  return res;
#endif
}

// Assume SD library is SdFat (or family) and file is open
                        
#if STORAGE_TYPE != STORAGE_FATFS
bool FtpServer::getFileModTime( uint16_t * pdate, uint16_t * ptime )
{
#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS
	#if ESP8266
		return dir.fileTime();
	#else
		return dir.getLastWrite();
	#endif
#elif STORAGE_TYPE == STORAGE_SDFAT1 || STORAGE_TYPE == STORAGE_SPIFM
  dir_t d;

  if( ! file.dirEntry( & d ))
    return false;
  * pdate = d.lastWriteDate;
  * ptime = d.lastWriteTime;
  return true;
#elif  STORAGE_TYPE == STORAGE_SDFAT2
  return file.getModifyDateTime( pdate, ptime );
#endif
}
#endif

#if STORAGE_TYPE == STORAGE_SD
  bool     FtpServer::rename( const char * path, const char * newpath ){

		FTP_FILE myFileIn = STORAGE_MANAGER.open(path, FILE_READ);
		FTP_FILE myFileOut = STORAGE_MANAGER.open(newpath, FILE_WRITE);

		if(myFileOut) {
			while (myFileIn.available() > 0)
			      {
			        int i = myFileIn.readBytes(buf, FTP_BUF_SIZE);
			        myFileOut.write(buf, i);
			      }
			      // done, close the destination file
				myFileOut.close();
				myFileOut = STORAGE_MANAGER.open(newpath, FILE_READ);

		}
		bool operation = false;

		DEBUG_PRINT(F("RENAME --> "));
		DEBUG_PRINT(myFileIn.size());
		DEBUG_PRINT(" size ");
		DEBUG_PRINTLN(myFileOut.size());

		if (myFileIn.size() == myFileOut.size()) {
			operation = true;
		}


		if (!operation) return operation;

		myFileIn.close();
		myFileOut.close();

		return remove( path );
  };
#endif
