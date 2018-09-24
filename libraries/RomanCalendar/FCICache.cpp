#include "FCICache.h"
#include "utf8String.h"

//LinkedList<DiskFont_FontCharInfo*>* _fciTable[FCI_NUMENTRIES];
	
FCICache::FCICache(){
	for (int i = 0 ; i < FCI_NUMENTRIES; i++) {
		_fciTable[i] = NULL;
	}
	
	_fci_count = 0;
}

FCICache::~FCICache(){
	clear();
}

int FCICache::Prune(int highestUseCountToPrune){
	int numPruned = 0;
	
	for (int i = 0; i < FCI_NUMENTRIES; i++) {
		FCILinkedList* fci_ll = _fciTable[i];
		if (fci_ll != NULL) {
			int n = fci_ll->size();
			int m = 0;
			
			while(m < n) {
				DiskFont_FontCharInfo* pfci = fci_ll->get(m);
				if (pfci->useCount <= highestUseCountToPrune) {
					fci_ll->remove(m);
					n--;
					numPruned++;
					_fci_count--;
				}
				else {
					m++;
				}
			}
		}
	}

	DEBUG_PRT.printf("FCICache prune: pruned %d entries\n", numPruned);
	
	return numPruned;
}

DiskFont_FontCharInfo* FCICache::get(uint32_t codepoint){
	uint16_t fcitable_index = hash(codepoint); // will return a number between 0 and FCI_NUMENTRIES - 1, should be evenly distributed

	FCILinkedList* fci_ll = _fciTable[fcitable_index];

	if (fci_ll == NULL) {
		//DEBUG_PRT.println("FCICache miss");
		return NULL;
	}
	
	int n = fci_ll->size();
	DiskFont_FontCharInfo* pfci = NULL;
	
	//function binary_search(A, n, T):
    int L = 0;
    int R = n - 1;
	uint32_t T = codepoint;
	
    while (L <= R) {
        int m = int((L + R) / 2);
        
		pfci = fci_ll->get(m);
		if (pfci->codepoint < T) {
            L = m + 1;
        }
		else if (pfci->codepoint > T) {
            R = m - 1;
		}
        else {
			pfci->useCount++;
			//DEBUG_PRT.println("FCICache hit");
			return pfci;
		}
	}	
	//DEBUG_PRT.println("FCICache miss");
    return NULL;
}

void FCICache::add(uint32_t codepoint, DiskFont_FontCharInfo* &fci){
	DiskFont_FontCharInfo* existing_item = get(codepoint);
	if (existing_item == NULL) {
		int pruneMaxUseCount = 1;
		while (_fci_count == FCI_MAXFCIBLOCKS) {	// make room
			Prune(pruneMaxUseCount);
			pruneMaxUseCount++;
		}
	
		uint16_t fcitable_index = hash(codepoint); // will return a number between 0 and FCI_NUMENTRIES - 1, should be evenly distributed

		if (_fciTable[fcitable_index] == NULL) {
			_fciTable[fcitable_index] = new FCILinkedList();
		}
		
		FCILinkedList* fci_ll = _fciTable[fcitable_index];
		
		fci->useCount = 0;
		fci->codepoint = codepoint;
		fci_ll->add(fci);
		fci_ll->sort(fci_compare);
		
		_fci_count++;
		//DEBUG_PRT.printf("FCICache add %s\n", utf8fromCodepoint(codepoint).c_str());
	}
	else {
		delete fci;
		fci = existing_item;
		//DEBUG_PRT.printf("FCICache add %s (already present)\n", utf8fromCodepoint(codepoint).c_str());
	}
}

void FCICache::clear(){
	for (int i = 0 ; i < FCI_NUMENTRIES; i++) {
		FCILinkedList* fci_ll = _fciTable[i];
		if (fci_ll != NULL) {
			fci_ll->clear();
			delete fci_ll;
			_fciTable[i] = NULL;
		}
	}
	_fci_count = 0;
}

uint16_t FCICache::hash(uint32_t codepoint){
	return codepoint % FCI_NUMENTRIES;
}

int fci_compare(DiskFont_FontCharInfo* &a, DiskFont_FontCharInfo* &b) {
	if (a->codepoint == b->codepoint) return 0;
		
	return (a->codepoint < b->codepoint ? -1 : 1);
}