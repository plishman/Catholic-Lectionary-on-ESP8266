// bibleindexEDB.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

#include <windows.h>
#include <stdint.h>
#include <Strsafe.h>
#include <Shlobj.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <stdio.h>
#include "csv.h"
#include "EDB.h"
#include "bibleindexEDB.h"

string _edbFilename = "";
string _edbBlankFilename = "";

// Create an EDB object with the appropriate write and read handlers
EDB edb(&writer, &reader);

int main(int argc, char* argv[])
{
	// Test if input arguments were supplied:

	if (!(argc == 2 || (argc == 3 && strcmp(argv[1], "-test") == 0)))
	{
		cout << "Please give filename and path to the bible CSV file.\n";
		cout << "Usage: " << argv[0] << " [-test] <filename>";
		return 1;
	}
	

	string fileName = "";
	bool bTestOnly = false;

	if (argc == 2) {
		fileName = argv[1];
	}
	else {
		bTestOnly = true;
		fileName = argv[2];
	}

	//string fileName = "njb.csv";
	//string fileName = "cei_it.csv";

	string path = ".\\Bible";


	if (bTestOnly) {
		cout << "Testing indexes for " << fileName << endl;
		if (readbackTest(path, fileName)) {
			cout << "Tested OK" << endl;
			return 0;
		}
		else {
			cout << "FAILED" << endl;
			return 1;
		}
	}

	ifstream bible_csv;

	//try
	//{
		std::wstring stemp = s2ws(path);
		LPCWSTR lpcwstr_path = stemp.c_str();

		if (!(dirExists(path)))
		{
			CreateDirectory(lpcwstr_path, NULL);
		}
		else
		{
			DeleteDirectoryAndAllSubfolders(lpcwstr_path);
			CreateDirectory(lpcwstr_path, NULL);
		}
		
		_edbFilename = path + "\\_blank.edb";
		_edbBlankFilename = _edbFilename;
		createBlankDb();

		Csv csv;

		bible_csv.open(fileName);
		string line;
		long pos = 0;
		uint32_t csvOffset = 0;

		while (std::getline(bible_csv, line))
		{
			std::cout << "offset\t" << csvOffset << "\t" << line << std::endl;

			unsigned int linepos = 0;
			string bookname = csv.getCsvField(line, &linepos);	// fields[0];
			string book = csv.getCsvField(line, &linepos);		// fields[1]; // use field 1 for the name, will use field 2 (the book index (1-73)), since SD card on arduino does not support long filenames
			string chapter = csv.getCsvField(line, &linepos);	// fields[2];
			string verse = csv.getCsvField(line, &linepos);		// fields[3];
			string versetext = csv.getCsvField(line, &linepos);	// fields[4];

			int16_t verseNumber = (int16_t)std::stoi(verse);
			
			int16_t bookNumber = (int16_t)std::stoi(book);
			int16_t chapterNumber = (int16_t)std::stoi(chapter);

			if (bookNumber < 10) book = "0" + book;
			if (chapterNumber < 10) chapter = "0" + chapter;
			if (chapterNumber < 100) chapter = "0" + chapter;

			_edbFilename = path + "\\" + book + "_" + chapter + ".edb"; // global _edbFilename

			//cout << bookname << ", " << book << ", " << chapter << ", " << verse << ", " << versetext;

			if (!(add_verse(verseNumber, csvOffset))) {
				cout << "Error adding verse\n";
				throw("Error adding verse");
			}

			csvOffset = bible_csv.tellg();
		}
	//}
/*
	catch(...)
	{
		bible_csv.close();

		std::wstring stemp = s2ws(path);
		LPCWSTR lpcwstr_path = stemp.c_str();
		RemoveDirectory(lpcwstr_path);

		cout << "Failed to process file";
		return 1;
	}
*/
	bible_csv.close();
	return 0;
}

bool add_verse(uint16_t verseNumber, uint32_t csvOffset)
{
	// on entry, _ebdFilename should be set to the database disk file to use

	VerseEntry v = { 0 };

	EDB_Status edb_result = EDB_OK;

	edb_result = edb.open(0);
	if (edb_result != EDB_OK) {
		cout << "Couldn't open database with filename " << _edbFilename << endl;
		return false;
	}

	edb_result = EDB_OK;
	//now edb file named _edbFilename should be open
	edb_result = edb.readRec(verseNumber, EDB_REC v);
	if (edb_result == EDB_OK) {
		if (v.fragment_count > 31) {
			cout << "found record with corrupted fragment count (>31)\n";
			return false;
		}

		if (v.verse_number != 0 && v.verse_number != verseNumber) {
			cout << "found record with verse_number != record number (verse_number = " << v.verse_number << " record number = " << verseNumber << ")\n";
			return false;
		}

		if (v.verse_number == 0) {
			v = { 0 };

			v.verse_number = verseNumber;
			v.fragment_count = 1;
			v.csv_offsets[0] = csvOffset;
		}
		else {
			v.csv_offsets[v.fragment_count++] = csvOffset;
		}

		edb_result = edb.updateRec(verseNumber, EDB_REC v);
		if (edb_result != EDB_OK) {
			printError(edb_result);
			return false;
		}
		//cout << "DONE\n"; // fall through, close file and return true.
	}
	else {
		printError(edb_result);
		return false;
	}

	return true;
}

void createBlankDb()
{
	//on entry, global _edbFilename is the name of the database file on disk to create
	
	cout << "Creating template blank table... ";
	EDB_Status edb_result = EDB_OK;

	VerseEntry v = { 0 };
	edb.create(0, TABLE_SIZE, (unsigned int)sizeof(v));
	cout << "DONE\n";

	cout << "Creating Records... ";
	int recno = 0;
	EDB_Status result;

	do {
		result = edb.appendRec(EDB_REC v);
		recno++;
	} while (result == EDB_OK);

	cout << "Created " << recno << " blank records\n";
}

bool readbackTest(string path, string fileName) {
	VerseEntry v = { 0 };
	EDB_Status edb_result = EDB_OK;

	ifstream bible_csv;
	Csv csv;

	bible_csv.open(fileName);
	string line;
	long pos = 0;
	uint32_t csvOffset = 0;
	int16_t fragmentIndex = 0;
	int16_t lastVerseNumber = 0;

	while (std::getline(bible_csv, line))
	{
		//std::cout << "offset\t" << csvOffset << "\t" << line << std::endl;

		unsigned int linepos = 0;
		string bookname = csv.getCsvField(line, &linepos);	// fields[0];
		string book = csv.getCsvField(line, &linepos);		// fields[1]; // use field 1 for the name, will use field 2 (the book index (1-73)), since SD card on arduino does not support long filenames
		string chapter = csv.getCsvField(line, &linepos);	// fields[2];
		string verse = csv.getCsvField(line, &linepos);		// fields[3];
		string versetext = csv.getCsvField(line, &linepos);	// fields[4];

		int16_t verseNumber = (int16_t)std::stoi(verse);
		int16_t bookNumber = (int16_t)std::stoi(book);
		int16_t chapterNumber = (int16_t)std::stoi(chapter);

		if (verseNumber == lastVerseNumber) {
			fragmentIndex++;
		}
		else {
			fragmentIndex = 0;
			lastVerseNumber = verseNumber;
		}

		string book_field = itos(bookNumber, 2);
		string chapter_field = itos(chapterNumber, 3);
		string verse_field = itos(verseNumber, 3);

		_edbFilename = path + "\\" + book_field + "_" + chapter_field + ".edb"; // global _edbFilename

		edb_result = edb.open(0);
		if (edb_result != EDB_OK) {
			cout << "Couldn't open database with filename " << _edbFilename << endl;
			return false;
		}

		v = { 0 };
		edb_result = EDB_OK;
		//now edb file named _edbFilename should be open
		edb_result = edb.readRec(verseNumber, EDB_REC v);
		if (edb_result == EDB_OK) {
			if (v.csv_offsets[fragmentIndex] != csvOffset) {
				cout << "csv offset for book " << bookNumber << " chapter " << chapterNumber << " verse " << verseNumber << " fragment number " << fragmentIndex << " should be " << csvOffset << ", but is " << v.csv_offsets[fragmentIndex] << endl;
				bible_csv.close();
				return false;
			}
			else {
				cout << book_field << "\t" << chapter_field << "\t" << verse_field << "[" << fragmentIndex << "]\t" << csvOffset << "\t" << v.csv_offsets[fragmentIndex] << endl;
			}
		}
		csvOffset = bible_csv.tellg();
	}

	bible_csv.close();
	return true;
}

string itos(int n, int numdigits) {
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(numdigits) << n;
	return out.str();
}


void printError(EDB_Status err)
{
	cout << "ERROR: ";
	switch (err)
	{
	case EDB_OUT_OF_RANGE:
		cout << "Recno out of range\n";
		break;
	case EDB_TABLE_FULL:
		cout << "Table full\n";
		break;
	case EDB_OK:
	default:
		cout << "OK\n";
		break;
	}
}

//https://stackoverflow.com/questions/8233842/how-to-check-if-directory-exist-using-c-and-winapi
bool dirExists(const string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

//https://stackoverflow.com/questions/1468774/why-am-i-having-problems-recursively-deleting-directories
LONG DeleteDirectoryAndAllSubfolders(LPCWSTR wzDirectory)
{
	WCHAR szDir[MAX_PATH + 1];  // +1 for the double null terminate
	SHFILEOPSTRUCTW fos = { 0 };

	StringCchCopy(szDir, MAX_PATH, wzDirectory);
	int len = lstrlenW(szDir);
	szDir[len + 1] = 0; // double null terminate for SHFileOperation

						// delete the folder and everything inside
	fos.wFunc = FO_DELETE;
	fos.pFrom = szDir;
	fos.fFlags = FOF_NO_UI;
	LONG res = SHFileOperation(&fos);

	if (res == 0)
	{
		SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, fos.pFrom, NULL);
	}
	return res;
}

//https://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

/*
std::wstring stemp = s2ws(myString);
LPCWSTR result = stemp.c_str();
*/

// The read and write handlers for using the SD Library
// Also blinks the led while writing/reading
void writer(unsigned long address, const byte* data, unsigned int recsize) {
	//dump_rec("wr ", data, recsize);
	
	FILE* fp;

	//if (!exists(_edbFilename)) {
		//fp = fopen(_edbFilename.c_str(), "w+b"); // create empty file
		//fclose(fp);
	//}

	if (_edbFilename == _edbBlankFilename && !(exists(_edbBlankFilename))) { // happens if blank database does not exist and is being created by this database write
		fp = fopen(_edbFilename.c_str(), "w+b"); // create empty file
		fclose(fp);
	} 
	else {
		copyBlankDbIfNotPresent(_edbBlankFilename, _edbFilename);
	}

	fp = fopen(_edbFilename.c_str(), "r+b");

	bool isOpen = (fp != NULL);
	bool seekResult = false;
	bool writeResult = false;
	bool flushResult = false;

	if (isOpen) {
		seekResult = (fseek(fp, address, 0) == 0);
		writeResult = (fwrite(data, 1, recsize, fp) == recsize);
		flushResult = (fflush(fp) == 0);
		fclose(fp);
	}
	else {
		cout << "Unable to open/create database file " << _edbFilename << endl;
	}

	if (!(seekResult == writeResult == flushResult == true)) {
		cout << "Error: seekResult=" << seekResult << " writeResult=" << writeResult << " flushResult=" << flushResult << endl;
	}

/*	
	fstream dbo;
	dbo.open(_edbFilename, std::ios::out | std::ios_base::ate | ios::binary); // ios::binary | ios_base::app | ios::ate | ios_base::out | ios_base::in

	bool isOpen = dbo.is_open();

	if (isOpen) {
		dbo.seekp(address, std::ios::beg);
		dbo.write((const char*)data, (std::streamsize)recsize);
		dbo.flush();
		dbo.close();
	}
	else {
		cout << "Unable to write to database file " << _edbFilename << endl;
	}
*/
}

void reader(unsigned long address, byte* data, unsigned int recsize) {
	ifstream dbi;
	dbi.open(_edbFilename, ios::in | ios::ate | ios::binary); // ios::binary | ios_base::app | ios_base::out | ios_base::in

	bool isOpen = dbi.is_open();

	if (isOpen) {
		dbi.seekg(address, std::ios::beg);
		dbi.read((char*)data, (std::streamsize)recsize);
		dbi.close();
		//dump_rec("rd ", (const byte*)data, recsize);
	}
	else {
		cout << "Unable to read database file " << _edbFilename << endl;
	}
}

void print(std::fstream &fs) {
//	cout << strerror(errno) << endl;
	cout << "-----------------------------\n"
		 << " fs.rdstate(): " << fs.rdstate()
		 << "\n fs.eof(): " << fs.eof()
		 << "\n fs.fail(): " << fs.fail()
		 << "\n fs.bad(): " << fs.bad()
		 << "\n fs.good(): " << fs.good() << endl;
}

void dump_rec(string txt, const byte* data, unsigned int recsize) {
	cout << txt << ": ";
	for (unsigned int i = 0; i < recsize; i++) {
		char c = (char)data[i];
		cout << setfill('0') << setw(2) << hex << (+c & 0xff) << " ";
	}
	cout << endl;
}

//https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
bool exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool copyBlankDbIfNotPresent(string _edbBlankFilename, string _edbFilename) {
	std::wstring source;	
	source = s2ws(_edbBlankFilename);
	LPCWSTR sourcefile = source.c_str();

	std::wstring dest;
	dest = s2ws(_edbFilename);
	LPCWSTR destfile = dest.c_str();

	return CopyFile(sourcefile, destfile, true);
}