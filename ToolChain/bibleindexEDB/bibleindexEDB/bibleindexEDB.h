#pragma once

// Arbitrary record definition for this table.
// This should be modified to reflect your record needs.

// some verses are subdivided (a,b,c, etc), and each subdivision has an entry in the csv. This structure allows for up to 14 fragments per verse.
// Fragments must be in the correct order in the csv for this process to work. This makes the record size for each verse (15*4)+2+2 = 64 bytes
// Each chapter will have multiple verses and a single EDB file, with 32-byte records for each stored in the EDB.
struct VerseEntry {
	uint16_t verse_number;
	uint16_t fragment_count;
	uint32_t csv_offsets[31];
};

#define TABLE_SIZE 32768  // needs room for 176 verses/chapter, plus the header

int main(int argc, char* argv[]);
bool add_verse(uint16_t verseNumber, uint32_t csvOffset);
void createBlankDb();
void printError(EDB_Status err);
bool readbackTest(string path, string fileName);
string itos(int n, int numdigits);
bool dirExists(const string& dirName_in);
LONG DeleteDirectoryAndAllSubfolders(LPCWSTR wzDirectory);
std::wstring s2ws(const std::string& s);
void writer(unsigned long address, const byte* data, unsigned int recsize);
void reader(unsigned long address, byte* data, unsigned int recsize);
void print(std::fstream &fs);
void dump_rec(string txt, const byte* data, unsigned int recsize);
bool exists(const std::string& name);
bool copyBlankDbIfNotPresent(string _edbBlankFilename, string _edbFilename);