#ifndef _FCICHAINEDHASHTABLE
#define _FCICHAINEDHASHTABLE

#include "RCGlobals.h"

#include "Arduino.h"
#include "I2CSerialPort.h"

//#include "DiskFont.h"
#include <LinkedList.h>

#define FCI_NUMENTRIES 256		// 256*4 bytes = 1024 (1k)
#define FCI_MAXFCIBLOCKS 64		// 64*32 bytes = 2048 (2k) (each FontCharInfo structure is 32 bytes)
#define FCI_MINTOFREE 8			// when pruning, free minimum of 8 FCI entries

typedef struct {	// 32 bytes
	//uint8_t rtlflag;
	uint16_t widthbits;
	uint16_t heightbits;
	uint32_t bitmapfileoffset;
	double advanceWidth;
	double advanceHeight;
	uint32_t useCount;	//used for storing in memory (this field is not present in the diskfont)
	uint32_t codepoint; //used for storing in memory (this field is not present in the diskfont)
} DiskFont_FontCharInfo;

const int DiskFont_FontCharInfo_RecSize = 2+2+4+8+8; //bytes

class FCICache
{
public:
	//LinkedList<DiskFont_FontCharInfo*> FCILinkedList = LinkedList<DiskFont_FontCharInfo*>();
	typedef LinkedList<DiskFont_FontCharInfo*> FCILinkedList;
	FCILinkedList* _fciTable[FCI_NUMENTRIES];
	
	int _fci_count = 0;
	
	FCICache();
	~FCICache();
	int Prune(int highestUseCountToPrune);
	DiskFont_FontCharInfo* get(uint32_t codepoint);
	void add(uint32_t codepoint, DiskFont_FontCharInfo* &fci);
	void clear();
	uint16_t hash(uint32_t codepoint);
};

int fci_compare(DiskFont_FontCharInfo* &a, DiskFont_FontCharInfo* &b);

#endif
