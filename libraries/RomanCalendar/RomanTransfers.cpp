#include "RomanTransfers.h"

File RomanTransfers::tfile;

bool RomanTransfers::OpenTransfersFile(File& tfile) {	
	bool btransferfileexists = SD.exists(TRANSFER_TRANSFERS_FN);
	if (!btransferfileexists) {
		ResetTransfersFile(tfile);
	}

	//if (tfile) { // close the file if it is already open
	//	tfile.close();
	//}

	bool btransfersfileisopen = tfile;

#ifdef _WIN32
	if (!btransfersfileisopen) {
		tfile = SD.open(TRANSFER_TRANSFERS_FN, FILE_READWRITE);
	}
#else
	if (!btransfersfileisopen) {
		tfile = SD.open(TRANSFER_TRANSFERS_FN, FILE_WRITE);
	}
#endif
	bool bok = tfile;

	if (!bok) {
		DEBUG_PRT.println(F("OpenTransfersFile() failed"));
		//assert(bok);
	}
	
	return bok;
}

bool RomanTransfers::ResetTransfersFile(File& tfile) { // empty the transfers file and return a handle to the newly-truncated/created file.
	if (tfile) {
		tfile.close();
	}
	bool bok = true;

	tfile = SD.open(TRANSFER_TRANSFERS_FN, FILE_WRITE);
#ifndef _WIN32
	bok = tfile.truncate(0);
#endif
	bok = bok && tfile;

	if (bok) {
		uint8_t numentries = 0;
		int8_t activeentry = TRANSFER_F_ACTIVEENTRY_NONE;
		int8_t lastactiveentry = TRANSFER_F_ACTIVEENTRY_NONE;
		int8_t lastaddedentry = TRANSFER_F_ACTIVEENTRY_NONE;

		tfile.write((uint8_t *)&numentries, 1);
		tfile.write((uint8_t *)&activeentry, 1);
		tfile.write((uint8_t *)&lastactiveentry, 1);
		tfile.write((uint8_t *)&lastaddedentry, 1);

		tfile.close();
	}

	DEBUG_PRT.print(F("ResetTransfersFile() "));
	if (bok) {
		DEBUG_PRT.println(F("success"));
	}
	else {
		DEBUG_PRT.println(F("failed"));
	}

	return bok;
}

////
// AddTransfer(FileDir_df, ImageFilename_df) this function (this form) should be called by lectionary code to add a new Transferred feast to non volatile storage
bool RomanTransfers::AddTransfer(String& FileDir_df, String& ImageFilename_df, int8_t precedencescore, int8_t lectionarynumber) {
	// TODO: need to pass in lect_path parameter, and remove it from FileDir_df, to shorten it. lectionarynumber parameter will allow records from different lectionaries
	// to be distinguished from one another.

	TransferRecord transfer;
	transfer.activedayofweek = TRANSFER_DOW_PENDING;
	transfer.precedencescore = precedencescore;
	transfer.deferredfeastfilename = FileDir_df;
	transfer.deferredfeastimagefilename = ImageFilename_df;
	//FileDir_df.toCharArray(transfer.deferredfeastfilename, TRANSFER_FN_SIZE);
	//ImageFilename_df.toCharArray(transfer.deferredfeastimagefilename, TRANSFER_IMAGEFN_SIZE);
	transfer.lectionarynumber = lectionarynumber;

	bool ballrecordspresent = false;

	return AddTransfer(transfer, ballrecordspresent);
}

bool operator==(const TransferRecord& lhs, const TransferRecord& rhs)
{
	return (lhs.activedayofweek == rhs.activedayofweek
		&& lhs.id == rhs.id
		&& lhs.lectionarynumber == rhs.lectionarynumber
		&& lhs.precedencescore == rhs.precedencescore
		&& lhs.deferredfeastimagefilename == rhs.deferredfeastimagefilename
		&& lhs.deferredfeastfilename == rhs.deferredfeastfilename);
}

bool operator!=(const TransferRecord& lhs, const TransferRecord& rhs)
{
	return (!(lhs == rhs));
}


bool RomanTransfers::AddTransfer(TransferRecord transfer, bool& ballrecordspresent) { // Append new transfer when the queue is still growing, will use Get/PutTransferRecord when all entries are present
	//File tfile;
	bool bhavefile = OpenTransfersFile(tfile);

	if (!bhavefile) {
		DEBUG_PRT.println(F("AddTransfer() could not stat transfers file!"));
		return false;
	}

	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;

	// file format is [uint8t numentries][int8_t activeentry][int8_t lastactiveentry][int8_t lastaddedentry][TransferRecord 1]..[TransferRecord n]

	uint8_t numentries = GetNumEntries(tfile);
	int8_t activeentry = GetActiveEntry(tfile);
	int8_t lastactiveentry = GetLastActiveEntry(tfile);
	int8_t lastaddedentry = GetLastAddedEntry(tfile);

	bool bissane = (tfile.size() == TRANSFER_F_HEADERSIZE + (numentries * transferrecordsize)
		&& recordcount <= TRANSFER_F_QUEUESIZE
		&& numentries == recordcount 
		&& activeentry < recordcount 
		&& lastactiveentry < recordcount
		&& lastaddedentry < recordcount); 
	// ^ check they all have sensible values

	if (!bissane) { // truncate/reset the transfers file if not
		DEBUG_PRT.print(F("AddTransfer() transfers file is corrupt, clearing and resetting it."));
		tfile.close();
		ResetTransfersFile(tfile);
		bhavefile = OpenTransfersFile(tfile);
		if (bhavefile) {
			recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize; // should be 0 after resetting transfers file
			if (!(recordcount == 0 && tfile.size() == TRANSFER_F_OFFSET_XFERRECORDS)) {
				DEBUG_PRT.println(F("..FAILED (transfers file is still corrupt after resetting it)"));
				tfile.close();
				return false;
			}
			DEBUG_PRT.println(F("..OK"));
		} 
		else {
			DEBUG_PRT.println(F("..FAILED"));
			tfile.close();
			return false;
		}
	}
	
	ballrecordspresent = (bhavefile && recordcount == TRANSFER_F_QUEUESIZE); // all queue records already present if so, so need to use Get/PutTransferRecord if so

	numentries = GetNumEntries(tfile);
	if (numentries == recordcount) {
		DEBUG_PRT.print(F("AddTransfer() "));
		TransferRecord tfr = transfer;
		
		if (CheckOpenDuplicateTransfer(tfr, tfile, false)) { // check for duplicate transfers, close all but first match and return first match in tfr (if any)
			transfer.id = tfr.id; // set the id of the record to add to that of the duplicate, and set it pending (the duplicate might be open rather than pending at this point)
			transfer.activedayofweek = TRANSFER_DOW_PENDING; // If the records now match (ie, both are pending), then no need to overwrite the duplicate
			
			bool bok = true;
			
			if (tfr != transfer) { // if tfr is not set to TRANSFER_DOW_PENDING, set it by overwriting the record with transfer, which has had this set, at the same id
				
				bok = (PutTransferRecord(tfile, transfer.id, transfer) // have already checked for duplicate pending entries, and closed any duplicate open ones
					&& SetLastAddedEntry(tfile, transfer.id)); // overwrite the duplicate entry with a pending copy
				
				DEBUG_PRT.println(F("succeeded (reset duplicate already open/pending transfer)"));
			}
			else {
				DEBUG_PRT.println(F("succeeded (using duplicate already pending transfer)"));
			}

			DumpTransferRecord(transfer, true);
			tfile.close();
			return bok;
		}

		if (!ballrecordspresent) { // still appending entries if < TRANSFER_F_QUEUESIZE (if not all transferrecords possible for the maximum queue size are yet present)
			transfer.id = numentries;	// record ids are zero-based, ie 0, 1, 2, but numentries would be 3 if there were 3 records in the file with ids 0, 1, 2
			int filesizeshouldbe = TRANSFER_F_OFFSET_XFERRECORDS + (numentries * transferrecordsize);
			int filesizeis = tfile.size();
			bool bok = (filesizeis == filesizeshouldbe);
			
			if (bok) {
				tfile.seek(filesizeis); // seems to always return false, if seeking to end of file?
			}
			
			bok = bok && (tfile.position() == filesizeis);

			//tfile.write((uint8_t *)&transfer, sizeof(TransferRecord));
			bok = bok && (PutTransferRecord(tfile, transfer.id, transfer, true));
			bok = bok && (SetNumEntries(tfile) && SetLastAddedEntry(tfile, transfer.id));
			ballrecordspresent = (GetNumEntries(tfile) == TRANSFER_F_QUEUESIZE); // so caller will know queue has all items populated
			
			if (bok) {
				DEBUG_PRT.println(F("succeeded (file is not yet fully populated), TransferRecord= "));
				DumpTransferRecord(transfer, true);
			}
			else {
				DEBUG_PRT.println(F("failed (with file not fully populated)"));
			}
			tfile.close();
			return bok;
		}
		else {
			int8_t currentry = GetLastAddedEntry(tfile); // will be -1 if no entries (this value is used as the id of each record, 0, 1, 2 etc, so if there are no records, is == -1 (the value of TRANSFER_F_ACTIVEENTRY_NONE)
			currentry = (currentry + 1) % TRANSFER_F_QUEUESIZE;
			int8_t entriescounted = 0;

			if (currentry < numentries) {
				TransferRecord tfr;
				while (GetTransferRecord(tfile, currentry, tfr) && tfr.activedayofweek != TRANSFER_DOW_FINISHED) { // Queue is full if so
					currentry = (currentry + 1) % TRANSFER_F_QUEUESIZE; //for the next entry
					if (entriescounted == numentries) { // have checked all available records if so, and none is free
						tfile.close();
						DEBUG_PRT.println(F("AddTransfer() Queue is full"));
						return false;
					}
					entriescounted++;
				}

				bool bresult = (PutTransferRecord(tfile, currentry, transfer) // have already checked for duplicate pending entries, and closed any duplicate open ones
					&& SetLastAddedEntry(tfile, currentry));

				if (bresult) {
					DEBUG_PRT.println(F("AddTransfer() succeeded (file is now fully populated), TransferRecord= "));
					DumpTransferRecord(transfer, true);
				}
				else {
					DEBUG_PRT.println(F("AddTransfer() failed (with file fully populated)"));
				}

				tfile.close();
				return bresult;
			}
		}
	}

	tfile.close();
	DEBUG_PRT.println(F("AddTransfer() failed, numentries != recordcount!"));
	return false;
}

bool RomanTransfers::CheckOpenDuplicateTransfer(TransferRecord& transfer, File& tfile, bool bcloseduplicates, bool balsocompareprecedencescore) {
	if (!tfile) {
		return false;
	}
	
	uint8_t numentries = GetNumEntries(tfile);
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;

	if (!(recordcount == numentries && numentries <= TRANSFER_F_QUEUESIZE)) { // sanity check to make sure actual and calculated number of queue entries match
		return false;
	}

	TransferRecord tfr;
	int8_t tfrid = GetLastAddedEntry(tfile);
	int tfrcount = 0;
	int duplicatecount = 0;
	bool bok = true;
	while (tfrcount < numentries && bok) {
		bok = GetTransferRecord(tfile, tfrid, tfr);
		if (!bok) {
			continue;
		}
		if (tfr.activedayofweek != TRANSFER_DOW_FINISHED 
			&& ((balsocompareprecedencescore && transfer.precedencescore == tfr.precedencescore) || !balsocompareprecedencescore)
			&& (transfer.lectionarynumber == tfr.lectionarynumber)
			&& (transfer.deferredfeastfilename == tfr.deferredfeastfilename)) 
		
		{ // already have the same feast open if so
			duplicatecount++;
			if (!bcloseduplicates && duplicatecount == 1) {
				transfer = tfr;
				bok = true;
				DEBUG_PRT.print(F("CheckOpenDuplicateTransfer() Returning a duplicate open transfer with id "));
				DEBUG_PRT.println(tfr.id);
			}
			else if (duplicatecount > 1) { // close all open duplicates but the first found, or all of them if bcloseduplicates = true
				bok = CloseTransfer(tfile, tfr.id, true);
				if (bok) {
					DEBUG_PRT.print(F("CheckOpenDuplicateTransfer() Closed a duplicate open transfer with id "));
					DEBUG_PRT.println(tfr.id);
				}
			}
		}
		tfrid = (tfrid + 1) % numentries;
		tfrcount++;
	}
	return (duplicatecount > 0);
}

int8_t RomanTransfers::GetActiveEntry(File& tfile, int8_t headerfieldnum) { // return the int8_t number of the active entry, or -1 if none, byte 2 of the transferrecords file
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;
	int numentries = GetNumEntries(tfile);

	if (tfile && numentries == recordcount) {
		if (headerfieldnum == TRANSFER_F_OFFSET_ACTIVEENTRY 
			|| headerfieldnum == TRANSFER_F_OFFSET_LASTACTIVEENTRY 
			|| headerfieldnum == TRANSFER_F_OFFSET_LASTADDEDENTRY)
		{
			tfile.seek(headerfieldnum);
			int8_t entry = (int8_t)tfile.read();
			if (entry > -1 && entry < recordcount 
				&& entry < TRANSFER_F_QUEUESIZE 
				&& tfile.size() >= (TRANSFER_F_OFFSET_XFERRECORDS + (entry * transferrecordsize))) { // activeentry is zero-based, -1 when initialized with no records
				return entry;
			}
		}
	}
	return -1;	// defaults to -1 if not found
}

bool RomanTransfers::SetActiveEntry(File& tfile, int8_t entry, int8_t headerfieldnum) { // set the int8_t number of the active entry
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;
	int numentries = GetNumEntries(tfile);

	if (tfile && numentries == recordcount && tfile.size() >= (TRANSFER_F_OFFSET_XFERRECORDS + (entry * transferrecordsize))) 
	{ // the file is open and at least big enough to contain the record, and the number of records in the file matches the number stored in the numentries field
		if (entry < numentries &&
		    (headerfieldnum == TRANSFER_F_OFFSET_ACTIVEENTRY 
				|| headerfieldnum == TRANSFER_F_OFFSET_LASTACTIVEENTRY 
				|| headerfieldnum == TRANSFER_F_OFFSET_LASTADDEDENTRY)
		   )
		{
			tfile.seek(headerfieldnum);
			tfile.write((uint8_t *)&entry, sizeof(uint8_t));
			return true;
		}
	}
	return false;	// defaults to -1 if not found
}

int8_t RomanTransfers::GetLastActiveEntry(File& tfile) {
	return GetActiveEntry(tfile, TRANSFER_F_OFFSET_LASTACTIVEENTRY);
}

bool RomanTransfers::SetLastActiveEntry(File& tfile, int8_t entry) {
	return SetActiveEntry(tfile, entry, TRANSFER_F_OFFSET_LASTACTIVEENTRY);
}

int8_t RomanTransfers::GetLastAddedEntry(File& tfile) {
	return GetActiveEntry(tfile, TRANSFER_F_OFFSET_LASTADDEDENTRY);
}

bool RomanTransfers::SetLastAddedEntry(File& tfile, int8_t entry) {
	return SetActiveEntry(tfile, entry, TRANSFER_F_OFFSET_LASTADDEDENTRY);
}

bool RomanTransfers::CloseTransfer(File& tfile, int8_t entry, bool bcloseevenifpending) {
	// set the DOW of the transfer numbered <entry> to be -2 (TRANSFER_DOW_FINISHED),
	// set the LASTACTIVEENTRY byte to be the same as the ACTIVEENTRY byte, and set the ACTIVEENTRY byte to -1 (=TRANSFER_F_ACTIVEENTRY_NONE)
	TransferRecord transfer;
	int8_t activetransfer = GetActiveEntry(tfile);

	DEBUG_PRT.print(F("CloseTransfer() "));

	if (GetTransferRecord(tfile, entry, transfer) 
		&& ((activetransfer == entry && transfer.activedayofweek >= dowSunday && transfer.activedayofweek <= dowSaturday)
		|| (bcloseevenifpending && transfer.activedayofweek == TRANSFER_DOW_PENDING)))
	{ // <entry> is an open transfer record
		transfer.activedayofweek = TRANSFER_DOW_FINISHED;
		bool bok = SetActiveEntry(tfile, TRANSFER_F_ACTIVEENTRY_NONE) && SetLastActiveEntry(tfile, entry) && PutTransferRecord(tfile, entry, transfer); // is not atomic, but hopefully the rest of the code will have enough checking to recover from nonatomic write interruption
		DEBUG_PRT.print(F("closed transfer with id="));
		DEBUG_PRT.println(transfer.id);
		return bok;
	}
	DEBUG_PRT.print(F("failed"));
	return false;
}

////
// SetActiveTransfer(transfer, datetime) (this form) is the function for the lectionary to call to set a transfer as active, if not already set so. 
// Called when the decision to celebrate a deferred feast is made that was already found pending or active by a prior call to GetTransfer().
bool RomanTransfers::SetActiveTransfer(TransferRecord& transfer, time64_t datetime) {
	//File tfile;
#ifdef _WIN32
	int opencount = SD.opencount;
#endif
	bool bok = SetActiveTransfer(transfer, datetime, tfile);

#ifdef _WIN32
	DEBUG_PRT.print(F("SetActiveTransfer() open file handles before call="));
	DEBUG_PRT.print(opencount);
	DEBUG_PRT.print(F("after call="));
	DEBUG_PRT.print(SD.opencount);
	DEBUG_PRT.println();

	assert(opencount == SD.opencount);
#endif

	return bok;
}

bool RomanTransfers::SetActiveTransfer(TransferRecord& transfer, time64_t datetime, File& tfile) {
	TransferRecord tr;
	
	// get whatever open transfer there is, there should be only one open, even if the 
	// lectionary version in use is changed
	bool bok = OpenTransfersFile(tfile);
	if (!bok) {
		DEBUG_PRT.println(F("SetActiveTransfer() failed, could not open transfers file"));
		return false;
	}
	
	bok = GetTransfer(tr, datetime, tfile);
	bool bneedtocloseactivetransferfromotherlectionaryfirst = (bok
															&& tr.activedayofweek != TRANSFER_DOW_PENDING
															&& transfer.lectionarynumber != -1 // -1 means ignore lectionarynumber, use any open or pending transferrecord available
															&& tr.lectionarynumber != transfer.lectionarynumber);
	
	if (bneedtocloseactivetransferfromotherlectionaryfirst) {
		bok = CloseTransfer(tfile, tr.id);
		if (!bok) {
			tfile.close();

			DEBUG_PRT.print(F("SetActiveTransfer() failed to close open transfer with lectionarynumber "));
			DEBUG_PRT.print(tr.lectionarynumber);
			DEBUG_PRT.print(F(" (id="));
			DEBUG_PRT.print(tr.id);
			DEBUG_PRT.print(F(") when trying to set transfer with lectionarynumber "));
			DEBUG_PRT.print(transfer.lectionarynumber);
			DEBUG_PRT.print(F(" (id="));
			DEBUG_PRT.print(transfer.id);
			DEBUG_PRT.print(F(") to be active"));

			return false;
		}
		else {
			DEBUG_PRT.print(F("SetActiveTransfer() lectionary version in use was changed, closed open transfer from previous version"));
#ifdef _WIN32
			Bidi::printf("[other lectionary version transfer closed]");
#endif
		}
	}

	bok = GetTransfer(tr, datetime, tfile, transfer.lectionarynumber); // now try to read the transfer (if available) that we want to set active

	int8_t activedayofweek = weekday(datetime);

	if (bok
		&& (transfer.lectionarynumber == -1 || tr.lectionarynumber == transfer.lectionarynumber)
		&& tr.activedayofweek == activedayofweek 
		&& tr.deferredfeastfilename == transfer.deferredfeastfilename) {
		// transfer is already active if so
		DEBUG_PRT.print(F("SetActiveTransfer() transfer "));
		DEBUG_PRT.print(transfer.id);
		DEBUG_PRT.println(F(" is already active"));
		tfile.close();
		return true;
	}

	if (bok && tr.activedayofweek == TRANSFER_DOW_PENDING) {
		if (tr.id == transfer.id) {
			bok = (tfile && SetActiveTransfer(tfile, TRANSFER_F_ACTIVEENTRY_NONE, activedayofweek, transfer, true));
			tfile.close();
			return bok;
		}
	}
	
	tfile.close();
	return false;
}
bool RomanTransfers::SetActiveTransfer(File& tfile, int8_t entry, int8_t activedayofweek, bool bsetactiveentry) {
	TransferRecord transfer;

	return (GetTransferRecord(tfile, entry, transfer) && SetActiveTransfer(tfile, TRANSFER_F_ACTIVEENTRY_NONE, activedayofweek, transfer, bsetactiveentry));
}

bool RomanTransfers::SetActiveTransfer(File& tfile, int8_t entry, int8_t activedayofweek, TransferRecord& transfer, bool bsetactiveentry) {
	if (!tfile) {
		return false;
	}

	uint8_t numentries = GetNumEntries(tfile);

	bool busetransferrecord = (entry == TRANSFER_F_ACTIVEENTRY_NONE // pass in entry==TRANSFER_SETENTRY_NONE to use pre-populated TransferRecord transfer instead
		&& transfer.id >= 0 
		&& transfer.id < numentries); // passed-in transferrecord is valid if so, so use this instead of reading it from the file
	
	bool bok = true;
	int8_t setentry = entry;

	if (!busetransferrecord && entry >= 0 && entry < numentries) {
		bok = GetTransferRecord(tfile, entry, transfer); // transfer record passed in should now be valid, if relying on the entry number of the transferrecord
	}
	else {
		setentry = transfer.id;
	}

	int8_t alreadyactiveentry = GetActiveEntry(tfile);
	
	bool bactivedayofweekisvalid = (activedayofweek >= dowSunday && activedayofweek <= dowSaturday);
	bool bsetentryisvalid = (setentry >= 0 && setentry < numentries);

	DEBUG_PRT.print(F("SetActiveTransfer() "));

	if (bok && alreadyactiveentry == setentry && transfer.activedayofweek == activedayofweek) {
		DEBUG_PRT.print(F("entry is already active, id="));
		DEBUG_PRT.print(transfer.id);
		DEBUG_PRT.print(F(", and activedayofweek="));
		DEBUG_PRT.println(transfer.activedayofweek);
		return true; // this entry is already active and valid if so - 
					 // not setting transfer.activedayofweek as it can only be set to a valid weekday once, and persists until the day is up
	}

	if (bok && transfer.activedayofweek == TRANSFER_DOW_PENDING // the record to set as active must be pending only (not complete or already in use)
		&& bactivedayofweekisvalid								// check activedayofweek is a valid day
		&& bsetentryisvalid										// check setentry, the record number to write, is in a valid range
		&& alreadyactiveentry == TRANSFER_F_ACTIVEENTRY_NONE)	// make sure no other entry is active 
	{
		transfer.activedayofweek = activedayofweek;
		bok = (PutTransferRecord(tfile, setentry, transfer) && SetActiveEntry(tfile, setentry));
		
		if (bok) {
			DEBUG_PRT.print(F("set active entry with id="));
			DEBUG_PRT.print(transfer.id);
			DEBUG_PRT.print(F(", and activedayofweek="));
			DEBUG_PRT.println(transfer.activedayofweek);
		}
		else {
			DEBUG_PRT.println(F("failed"));
		}
		return bok;
	}

	DEBUG_PRT.println(F("failed (neither transfer nor entry id were valid)"));
	return false;
}

bool RomanTransfers::SetNumEntries(File& tfile) { // calculate and set the number of entries in the list - will vary between 0 and TRANSFER_F_QUEUESIZE
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;

	if (tfile && tfile.size() == (TRANSFER_F_OFFSET_XFERRECORDS + (transferrecordsize * recordcount))) { // check if filesize is an integral number of records, plus the three header bytes (numentries, activeentry and lastactiveentry)
		recordcount = recordcount <= TRANSFER_F_QUEUESIZE ? recordcount : TRANSFER_F_QUEUESIZE; // make sure queue isn't allowed to grow larger than the maximum allowed size
		tfile.seek(TRANSFER_F_OFFSET_NUMENTRIES);
		tfile.write((uint8_t *) &recordcount, sizeof(uint8_t));
		return true;
	}
	return false;
}

uint8_t RomanTransfers::GetNumEntries(File& tfile) { // get the number of entries in the list - will vary between 0 and TRANSFER_F_QUEUESIZE
	if (!tfile) {
		DEBUG_PRT.println("GetNumEntries() tfile is not available!");
		return 0;
	}
	
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;

	if (tfile && tfile.size() == (TRANSFER_F_OFFSET_XFERRECORDS + (transferrecordsize * recordcount))) { // check if filesize is an integral number of records, plus the three header bytes (numentries, activeentry and lastactiveentry)
		tfile.seek(TRANSFER_F_OFFSET_NUMENTRIES);
		uint8_t numentries = (uint8_t)tfile.read();
		if (numentries == recordcount && numentries <= TRANSFER_F_QUEUESIZE) {
			return numentries;
		}
		else {
			if (!SetNumEntries(tfile)) { // may need to actually reset the file in this case, as it indicates the file structure is not as it should be!
				ResetTransfersFile(tfile);
				OpenTransfersFile(tfile);
				return 0;
			}
			
			return recordcount;
		}
	}
	return 0;
}

bool RomanTransfers::GetTransferRecord(File& tfile, uint8_t recordnum, TransferRecord& transfer) { // get transferrecord number <recordnum> from the transferrecords file
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordcount = (tfile.size() - TRANSFER_F_OFFSET_XFERRECORDS) / transferrecordsize;
	int recordoffset = (recordnum * transferrecordsize) + TRANSFER_F_OFFSET_XFERRECORDS;
	int recordoffset_filename = recordoffset + sizeof(transfer.id) + sizeof(transfer.activedayofweek) + sizeof(transfer.precedencescore) + sizeof(transfer.lectionarynumber);
	int recordoffset_imgfilename = recordoffset_filename + TRANSFER_FN_SIZE;

	if (tfile && tfile.size() >= recordoffset + transferrecordsize) { // there is the possibility of a whole record existing at this file offset if so
		tfile.seek(recordoffset);
		bool bok = (tfile.position() == recordoffset);

		//tfile.read((uint8_t *)&transfer, transferrecordsize);

		transfer.id = (uint8_t)tfile.read();
		transfer.activedayofweek = (int8_t)tfile.read();
		transfer.precedencescore = (int8_t)tfile.read();
		transfer.lectionarynumber = (int8_t)tfile.read();
		
		bok = bok && tfile.available(); // use available() here, because want to know the file is open and the file pointer is not at the end
		
		tfile.seek(recordoffset_filename);
		bok = bok && (tfile.position() == recordoffset_filename);
		bok = bok && ReadFixedFieldString(transfer.deferredfeastfilename, tfile, TRANSFER_FN_SIZE);

		tfile.seek(recordoffset_imgfilename);
		bok = bok && (tfile.position() == recordoffset_imgfilename);
		bok = bok && ReadFixedFieldString(transfer.deferredfeastimagefilename, tfile, TRANSFER_IMAGEFN_SIZE);
			   
		if (transfer.activedayofweek >= TRANSFER_DOW_FINISHED && transfer.activedayofweek <= dowSaturday && transfer.activedayofweek != 0) { // 0 is not used for weekday numbers (Sunday=1, Saturday=7)
			DEBUG_PRT.print(F("["));
			DEBUG_PRT.print(transfer.id);
			DEBUG_PRT.print(F("]"));
			return true;
		}
	}
	return false;
}

bool RomanTransfers::ReadFixedFieldString(String& str, File& tfile, int fieldsize) {
	// read a String from a fixed-width field in tfile from the current file position, 
	// for low memory environments where reserving large buffers is problematic

	if (!tfile.available()) { // use available() here, because want to know the file is open and the file position is not at the end
		return false;
	}

	int filepos = tfile.position();
	bool bhavestring = false;
	int strlength = 0;

	while (tfile.available() && !bhavestring && strlength < fieldsize) { // use available() here, because want to know the file is open and the file position is not at the end
		char c = tfile.read();
		if (c == '\0') {
			bhavestring = true;
			continue;
		}
		else {
			strlength++;
		}
	}

	bool bok = (bhavestring || strlength == 0);
	
	if (strlength > 0) {
		tfile.seek(filepos);
		bok = bok && (tfile.position() == filepos);
		
		str = "";
		str.reserve(strlength);
	
		for (int i = 0; i < strlength; i++) {
			str += (char)tfile.read();
		}
	}

	tfile.seek(filepos);
	bok = bok && (tfile.position() == filepos);
	
	return bok;
}

bool RomanTransfers::WriteFixedFieldString(String& str, File& tfile, int fieldsize) {
	// Write (or append) a String to a fixed-width field in tfile at the current file position, 
	// for low memory environments where reserving large buffers is problematic
	if (!tfile) { 
		return false;
	}

	int filepos = tfile.position();
	int charnum = 0;
	int strlength = str.length();

	int numcharstowrite = strlength < fieldsize ? strlength : fieldsize - 1;
	bool bok = true;

	char c = '\0';

	while (charnum < fieldsize && bok) {
		c = '\0';
		if (charnum < numcharstowrite) {
			c = str.charAt(charnum);
		}

		bok = bok && (tfile.write((uint8_t*)&c, sizeof(c)) > 0);
		charnum++;
	}

	tfile.seek(filepos);
	bok = bok && (tfile.position() == filepos);
	bok = bok && (strlength <= fieldsize - 1); //string has been truncated if so, have written out as much as possible, but will return false

	return bok;
}

// Put a transfer record (these functions do not update lastaddedentry)
bool RomanTransfers::PutTransferRecord(TransferRecord& transfer) {
	//File tfile;
	bool bok = OpenTransfersFile(tfile);
	if (bok) {
		bok = PutTransferRecord(tfile, transfer.id, transfer);
		tfile.close();
	}
	return bok;
}

bool RomanTransfers::PutTransferRecord(File& tfile, uint8_t recordnum, TransferRecord& transfer, bool bappendrecord) { // write transferrecord to record number <recordnum> in the transferrecords file
	if (!tfile) {
		return false;
	}
	
	//int transferrecordsize = sizeof(TransferRecord) / sizeof(uint8_t);
	int recordoffset = (recordnum * transferrecordsize) + TRANSFER_F_OFFSET_XFERRECORDS;
	int recordoffset_filename = recordoffset + sizeof(transfer.id) + sizeof(transfer.activedayofweek) + sizeof(transfer.precedencescore) + sizeof(transfer.lectionarynumber);
	int recordoffset_imgfilename = recordoffset_filename + TRANSFER_FN_SIZE;
	int filesizeis = tfile.size();
	int filesizeshouldbe = bappendrecord ? TRANSFER_F_OFFSET_XFERRECORDS + recordnum * transferrecordsize : recordoffset + transferrecordsize;
	if (tfile && ((!bappendrecord && filesizeis >= filesizeshouldbe) || (bappendrecord && filesizeis == filesizeshouldbe))) 
	{ // there is the possibility of a whole record existing at this file offset if so
		transfer.id = recordnum;
		tfile.seek(recordoffset);
		bool bok = (tfile.position() == recordoffset);
		//tfile.write((uint8_t *)&transfer, transferrecordsize);
		bok = bok && (tfile.write((uint8_t *)&transfer, recordoffset_filename - recordoffset) > 0);
		
		tfile.seek(recordoffset_filename);
		bok = bok && (tfile.position() == recordoffset_filename);
		bok = bok && WriteFixedFieldString(transfer.deferredfeastfilename, tfile, TRANSFER_FN_SIZE);
		
		tfile.seek(recordoffset_imgfilename);
		bok = bok && (tfile.position() == recordoffset_imgfilename);
		bok = bok && WriteFixedFieldString(transfer.deferredfeastimagefilename, tfile, TRANSFER_IMAGEFN_SIZE);

		if (bok) {
			DEBUG_PRT.print(F("PutTransferRecord() put record id="));
		}
		else {
			DEBUG_PRT.print(F("PutTransferRecord() error writing record with id="));
		}
		DEBUG_PRT.println(recordnum);

		return bok;
	}

	return false;
}

////
// GetTransfer() is the function the lectionary should call to obtain the next pending transfer record (if any)
bool RomanTransfers::GetTransfer(TransferRecord& transfer, time64_t datetime, int8_t lectionarynumber) { // get any available transfer. Will return the active one if available, or a Pending one
	//File tfile;

#ifdef _WIN32
	int opencount = SD.opencount;
#endif

	bool bok = GetTransfer(transfer, datetime, tfile, lectionarynumber);

#ifdef _WIN32	
	DEBUG_PRT.print(F("GetTransfer() open file handles before call="));
	DEBUG_PRT.print(opencount);
	DEBUG_PRT.print(F("after call="));
	DEBUG_PRT.print(SD.opencount);
	DEBUG_PRT.println();

	assert(opencount == SD.opencount);
#endif
	return bok;
}

bool RomanTransfers::GetTransfer(TransferRecord& transfer, time64_t datetime, File& tfile, int8_t lectionarynumber) { // get any available transfer. Will return the active one if available, or a Pending one
																		   // if available, or false if there are no transfers in use or pending
	DEBUG_PRT.print(F("GetTransfer() "));
	//return false; // testing/debugging
	bool bfilewasopen = tfile;
	bool bok = OpenTransfersFile(tfile);

	if (!bok) {
		DEBUG_PRT.println(F(" couldn't open transfers file"));
		return false;
	}

	uint8_t numentries = GetNumEntries(tfile);
	if (numentries == 0) {
		if (!bfilewasopen) tfile.close();
		DEBUG_PRT.println(F(" transfers file is empty"));
		return false; // no entries to get
	}

	int8_t activeentry = GetActiveEntry(tfile);
	//int transferrecordsize = sizeof(transfer) / sizeof(uint8_t);
	int8_t dayofweek = (int8_t)weekday(datetime);
	
	if (activeentry < numentries && activeentry != TRANSFER_F_ACTIVEENTRY_NONE 
		&& GetTransferRecord(tfile, activeentry, transfer)
		&& ((lectionarynumber > -1 && transfer.lectionarynumber == lectionarynumber) || lectionarynumber == -1)) // don't need to match passed in lectionarnumber parameter if it is -1
	
	{ // activeentry is zero-based
		if (transfer.activedayofweek == dayofweek) {
			if (!bfilewasopen) tfile.close();
			DEBUG_PRT.print(F(" Transfer record entry is already open: "));
			DumpTransferRecord(transfer, true);
			return true; // found the Transfer record being celebrated today
		}
		else if (transfer.activedayofweek != TRANSFER_DOW_PENDING) {
			// have an expired entry if DOW is not the same as that of the active entry and not set to TRANSFER_DOW_PENDING, so set it to -2 (TRANSFER_DOW_FINISHED)
			// and return false but there may still be another one pending immediately (if there are two or more deferred feasts, so need to check if so
			
			if (CloseTransfer(tfile, activeentry)) { // not checking return flag! (does 3 nonatomic writes, if it goes wrong, 
													// will have to rely on checking elsewhere in module to make handle file corruption
													// then drop through to see if there is a new pending transfer for today
				DEBUG_PRT.print(F(" current dayofweek="));
				DEBUG_PRT.print(dayofweek);
				DEBUG_PRT.print(F(", so closed already open transfer with activedayofweek="));
				DEBUG_PRT.print(transfer.activedayofweek);
				DEBUG_PRT.print(F(" and id="));
				DEBUG_PRT.println(transfer.id);

#ifdef _WIN32
				Bidi::printf("[stale transfer closed]");
#endif
			}
			else {
				DEBUG_PRT.println(F(" Tried to close stale transfer, but failed!"));
			}
		}
	}
	// there is no active entry (or have just closed an expired one), so find a pending one (if any)
	int8_t currentry = GetLastActiveEntry(tfile); // start from the last used entry. (currentry + 1) % numentries should be the same record as the one above if it
												  // didn't close the transfer because activedayofweek was == to TRANSFER_DOW_PENDING
		
	currentry = currentry == TRANSFER_F_ACTIVEENTRY_NONE ? 0 : (currentry + 1) % numentries; // look for the next entry after it, looping back to first entry as necessary

	int8_t highestprecedencescore = -1;
	int8_t highestprecedenceentry = currentry;

	int8_t entriescounted = 0;
	int8_t foundentry = -1;
	bool bfound = false;
	bool bhaveatleastonependingrecord = false;

	while (entriescounted < numentries && !bfound && GetTransferRecord(tfile, currentry, transfer)) {
		//if (transfer.activedayofweek == TRANSFER_DOW_PENDING) {
		if (transfer.activedayofweek == TRANSFER_DOW_PENDING 
			&& transfer.precedencescore > highestprecedencescore
			&& ((lectionarynumber > -1 && transfer.lectionarynumber == lectionarynumber) || lectionarynumber == -1)
			) 
		{
			highestprecedencescore = transfer.precedencescore;
			highestprecedenceentry = transfer.id;
			bhaveatleastonependingrecord = true;
		}
		if (entriescounted == numentries - 1 && bhaveatleastonependingrecord) { // have searched all records if so
			foundentry = highestprecedenceentry; // prioritise feasts with higher rank, which could be stored as the Precedence score in the TransferRecord struct
			bfound = true;			// for now, just find the first unused entry
				
			if (transfer.id != foundentry) { // already have highest precedencescore record
				bfound = GetTransferRecord(tfile, foundentry, transfer); // get the pending entry with the highest precedencescore scanned
			}
				
			continue;
		}
		//}
		currentry = (currentry + 1) % numentries;
		entriescounted++; // will terminate when they've all been scanned, or before if a pending entry is found
	}

	if (!bfilewasopen) tfile.close();
	//SetActiveTransfer(tfile, foundentry, dayofweek); // don't set it active here, before it has been determined if it is to be celebrated today
	if (bfound) {
		DEBUG_PRT.print(F(" Found pending entry:"));
		DumpTransferRecord(transfer, true);
	}
	else {
		DEBUG_PRT.println(F(" failed, no pending or active entries found"));
	}
	
	return bfound;
}

String RomanTransfers::GetImageFilenameFromFileDir(String filedir, String lect_fileroot, String lect_imageroot) { // lect_fileroot should include the lang part of the path (eg 1955/en/)
	// reverse calculate the image filename from the Missalreading filedir (used for finding image name when a deferred feast is being celebrated)
	String imagefn = "";

	int lectfilerootindex = filedir.indexOf(lect_fileroot);
	if (lectfilerootindex != 0) return imagefn;

	imagefn = filedir.substring(lect_fileroot.length());
	int offset_day = imagefn.indexOf(F("/"), 1);
	String month_of_year = imagefn.substring(1, offset_day);	// is of the form /12/24/ (where 12 is the month)
	String day_of_month = imagefn.substring(offset_day + 1, imagefn.lastIndexOf(F("/")));
	if (day_of_month.toInt() != 0 && month_of_year.toInt() != 0) {
		return String(F("/")) + String(month_of_year) + String(F("/")) + String(day_of_month) + String(F("-")) + String(month_of_year);
	}
#ifdef _WIN32
	Bidi::printf("GetImageFilenameFromFileDir() WARN: returning [%s]", imagefn);
#endif
	DEBUG_PRT.print(F("getImageFilenameFromFileDir() WARN (probably won't work!): returning "));
	DEBUG_PRT.println(imagefn);

	return imagefn; // check this - it will fail for imagefilenames for feasts of the Lord, which do not follow the m/dd-m.bwr naming convention!
}

int8_t RomanTransfers::GetLectionaryVersionNumber(String& FileDir_df) {
	int lectnum = -1;
	if (FileDir_df.indexOf("1570") != -1) { lectnum = 0; }
	else if (FileDir_df.indexOf("1910") != -1) { lectnum = 1; }
	else if (FileDir_df.indexOf("Affl") != -1) { lectnum = 2; }
	else if (FileDir_df.indexOf("1955") != -1) { lectnum = 3; }
	else if (FileDir_df.indexOf("1960") != -1) { lectnum = 4; }

	return lectnum;
}

void RomanTransfers::DumpTransfersFile(bool bheaderonly) {
	//File tfile;

#ifdef _WIN32
	tfile = SD.open(TRANSFER_TRANSFERS_FN, FILE_READWRITE);
#else
	tfile = SD.open(TRANSFER_TRANSFERS_FN, FILE_WRITE);
#endif
	bool bok = tfile;

#ifdef _WIN32
	Bidi::printf("<div>OpenTransfersFile() ok=%d</div>", bok);
#endif

	if (!bok) return;

	uint8_t numentries = GetNumEntries(tfile);
	int8_t activeentry = GetActiveEntry(tfile);
	int8_t lastactiveentry = GetLastActiveEntry(tfile);
	int8_t lastaddedentry = GetLastAddedEntry(tfile);

#ifdef _WIN32
	Bidi::printf("<div>");
	Bidi::printf("<div>numentries=%d, activeentry=%d, lastactiveentry=%d, lastaddedentry=%d</div>", numentries, activeentry, lastactiveentry, lastaddedentry);
#endif

	if (bheaderonly) {
#ifdef _WIN32
		Bidi::printf("</div>");
#endif
		tfile.close();
		return;
	}

	int8_t currentry = 0;
	while (currentry < numentries) {
		TransferRecord transfer;
		DEBUG_PRT.push();
		DEBUG_PRT.off();
		bok = GetTransferRecord(tfile, currentry, transfer);
		DEBUG_PRT.pop();
		if (bok) {
			DumpTransferRecord(transfer);
		}
		else {
#ifdef _WIN32
			Bidi::printf("<div>Couldn't get transfer record %d</div>", currentry);
#endif
		}

		currentry++;
	}
#ifdef _WIN32
	Bidi::printf("</div>");
#endif

	tfile.close();
}

void RomanTransfers::DumpTransferRecord(TransferRecord transfer, bool bdumptodebugprt) {
	String activedayofweek = transfer.activedayofweek == TRANSFER_DOW_PENDING ? "TRANSFER_DOW_PENDING" :
						     transfer.activedayofweek == TRANSFER_DOW_FINISHED ? "TRANSFER_DOW_FINISHED" : String(transfer.activedayofweek);

	if (bdumptodebugprt) {
		DEBUG_PRT.print(F(" id="));
		DEBUG_PRT.println(transfer.id);
		DEBUG_PRT.print(F(" precedencescore="));
		DEBUG_PRT.println(transfer.precedencescore);
		DEBUG_PRT.print(F(" lectionarynumber="));
		DEBUG_PRT.println(transfer.lectionarynumber);
		DEBUG_PRT.print(F(" transfer.deferredfeastfilename='"));
		DEBUG_PRT.print(transfer.deferredfeastfilename);
		DEBUG_PRT.print(F("', transfer.deferredfeastimagefilename='"));
		DEBUG_PRT.print(transfer.deferredfeastimagefilename);
		DEBUG_PRT.println(F("'"));
	}
	else {
#ifdef _WIN32
		Bidi::printf("<div style='border:1px; background: lightgrey; margin:10px; padding: 10px;'><div>id=%d</div><div>activedayofweek=%s</div><div>precedencescore=%d</div><div>lectionarynumber=%d</div><div>deferredfeastfilename=[%s]</div><div>deferredfeastimagefilename=[%s]</div></div>", transfer.id, activedayofweek.c_str(), transfer.precedencescore, transfer.lectionarynumber, transfer.deferredfeastfilename.c_str(), transfer.deferredfeastimagefilename.c_str());
#endif
	}
}
