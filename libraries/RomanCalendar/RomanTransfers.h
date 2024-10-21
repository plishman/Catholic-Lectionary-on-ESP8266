#pragma once

#ifndef _TRANSFERS_H_
#define _TRANSFERS_H_

#ifdef _WIN32
	#include "WString.h"
	#include "SD.h"
	#include "TimeLib.h"
	#include "DebugPort.h"	
	#include "PROGMEM.h"
	#include "Bidi.h"
	#include <stdio.h>
	#include <stdint.h>
	#include <assert.h>
#else
	#include "Arduino.h"
	#include "DebugPort.h"
	#include "RCGlobals.h"
	#include "SD.h"
	#include "TimeLib.h"
#endif

#define TRANSFER_FN_SIZE 32 //transfers filename record is a fixed width field that will allow this number of characters
#define TRANSFER_IMAGEFN_SIZE 32 // transfers image filename record will allow this number of characters (does not have the full path prepended)
#ifdef _WIN32
#define TRANSFER_TRANSFERS_FN "./transfers.txt" // stores transferred feast records in a ring buffer of TranferRecord structs.
#else
#define TRANSFER_TRANSFERS_FN "/transfers.txt" // stores transferred feast records in a ring buffer of TranferRecord structs.
#endif
#define TRANSFER_F_QUEUESIZE 7 //maximum of 7 entries in the queue stored in the tranfers file
#define TRANSFER_DOW_PENDING -1 // the value that will be placed in the activedayofweek variable in TransferRecord structs for deferred feasts that have yet to be celebrated
#define TRANSFER_DOW_FINISHED -2 // the value that will be placed in the activedayofweek variable in TransferRecord structs for deferred feasts that are complete (have been celebrated on a previous day)
#define TRANSFER_F_OFFSET_NUMENTRIES 0 // the file offset of the uint8_t that contains the number of TransferRecord records in the file
#define TRANSFER_F_OFFSET_ACTIVEENTRY 1 // the file offset of the int8_t that contains the number of TransferRecord record that is currently active (or -1 if none)
#define TRANSFER_F_OFFSET_LASTACTIVEENTRY 2 // the file offset of the int8_t that contains the number of TransferRecord record that was last used (or -1 if none)
#define TRANSFER_F_OFFSET_LASTADDEDENTRY 3 // the file offset of the int8_t that contains the number of TransferRecord record that was last used (or -1 if none)
#define TRANSFER_F_OFFSET_XFERRECORDS 4 // the offset of the beginning of the transfer records in the TransferRecords file
#define TRANSFER_F_HEADERSIZE TRANSFER_F_OFFSET_XFERRECORDS // size of the file header (4 bytes)
#define TRANSFER_F_ACTIVEENTRY_NONE -1 // the ACTIVEENTRY byte is set to this if there is no active entry

//typedef struct TransferRecord {
//	int8_t	id;						// index so that the position that the next entry should be added at can be known
//	int8_t	activedayofweek;		// -1, then set to Time DOW number on first access, on day when it has been determined deferred feast is to be celebrated. set to -2 when complete
//	int8_t	precedencescore;		// a value which can be used to set the priority in which deferred feasts are celebrated, if there are more than one pending.
//	int8_t  lectionarynumber;		// a value which can be used to identify the lectionary to which this record applies (so that, if switching between lectionaries, pending feasts can be preserved from the one switched away from)
//	char	deferredfeastfilename[TRANSFER_FN_SIZE];
//	char	deferredfeastimagefilename[TRANSFER_IMAGEFN_SIZE];
//} TransferRecord;

typedef struct TransferRecord {
	int8_t	id;						// index so that the position that the next entry should be added at can be known
	int8_t	activedayofweek;		// -1, then set to Time DOW number on first access, on day when it has been determined deferred feast is to be celebrated. set to -2 when complete
	int8_t	precedencescore;		// a value which can be used to set the priority in which deferred feasts are celebrated, if there are more than one pending.
	int8_t  lectionarynumber;		// a value which can be used to identify the lectionary to which this record applies (so that, if switching between lectionaries, pending feasts can be preserved from the one switched away from)
	String	deferredfeastfilename;
	String	deferredfeastimagefilename;
} TransferRecord;

const int transferrecordsize = sizeof(int8_t) + 3 * sizeof(int8_t) + TRANSFER_FN_SIZE + TRANSFER_IMAGEFN_SIZE;

bool operator==(const TransferRecord& lhs, const TransferRecord& rhs);
bool operator!=(const TransferRecord& lhs, const TransferRecord& rhs);

class RomanTransfers {
public:
	// AddTransfer(FileDir_df, ImageFilename_df) this function (this form) should be called by lectionary code to add a new Transferred feast to non volatile storage
	static bool AddTransfer(String& FileDir_df, String& ImageFilename_df, int8_t precedencescore = 0, int8_t lectionarynumber = -1);

	// SetActiveTransfer() (this form) is the function for the lectionary to call to set a transfer as active, if not already set so. 
	// Called when the decision to celebrate a deferred feast is made that was already found pending or active by a prior call to GetTransfer().
	static bool SetActiveTransfer(TransferRecord& transfer, time64_t datetime);

	// GetTransfer() is the function the lectionary should call to obtain the next pending transfer record (if any)
	static bool GetTransfer(TransferRecord& transfer, time64_t datetime, int8_t lectionarynumber = -1);

	// Get (calculate) an image filename from a feastday filename. Works for feastdays of the form "<lect_path>/lang/m/d/". Needs fixing for other filename types
	static String GetImageFilenameFromFileDir(String filedir, String lect_fileroot, String lect_imageroot);
	
	// get a number corresponding to the lectionary edition in use. This function should probably be put in the Tridentine class
	static int8_t GetLectionaryVersionNumber(String& FileDir_df);

	// dump the transfers file to the browser
	static void DumpTransfersFile(bool bheaderonly = false);
	static void DumpTransferRecord(TransferRecord transfer, bool bdumptodebugprt = false);

private:
	static File tfile;

	static bool GetTransfer(TransferRecord& transfer, time64_t datetime, File& tfile, int8_t lectionarynumber = -1);
	static bool SetActiveTransfer(TransferRecord& transfer, time64_t datetime, File& tfile);

	static bool OpenTransfersFile(File& tfile);
	static bool ResetTransfersFile(File& tfile);

	static bool AddTransfer(TransferRecord transfer, bool& ballrecordspresent);
	
	static bool CheckOpenDuplicateTransfer(TransferRecord& transfer, File& tfile, bool bcloseduplicates = true, bool balsocompareprecedencescore = false);

	static int8_t GetActiveEntry(File& tfile, int8_t headerfieldnum = TRANSFER_F_OFFSET_ACTIVEENTRY);
	static bool SetActiveEntry(File& tfile, int8_t entry, int8_t headerfieldnum = TRANSFER_F_OFFSET_ACTIVEENTRY);

	static int8_t GetLastActiveEntry(File& tfile);
	static bool SetLastActiveEntry(File& tfile, int8_t entry);

	static int8_t GetLastAddedEntry(File& tfile);
	static bool SetLastAddedEntry(File& tfile, int8_t entry);

	static bool CloseTransfer(File& tfile, int8_t entry, bool bcloseevenifpending = false);

	static bool SetActiveTransfer(File& tfile, int8_t entry, int8_t activedayofweek, bool bsetactiveentry = false);
	static bool SetActiveTransfer(File& tfile, int8_t entry, int8_t activedayofweek, TransferRecord& transfer, bool bsetactiveentry = false);

	static bool SetNumEntries(File& tfile);
	static uint8_t GetNumEntries(File& tfile);
	
	static bool GetTransferRecord(File& tfile, uint8_t recordnum, TransferRecord& transfer);
		
	static bool PutTransferRecord(TransferRecord& transfer);
	static bool PutTransferRecord(File& tfile, uint8_t recordnum, TransferRecord& transfer, bool bappendrecord = false);

	static bool ReadFixedFieldString(String& str, File& tfile, int fieldsize);
	static bool WriteFixedFieldString(String& str, File& tfile, int fieldsize);
};

#endif