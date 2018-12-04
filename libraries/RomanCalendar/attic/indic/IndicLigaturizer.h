#ifndef _INDICLIGATURIZER_H
#define _INDICLIGATURIZER_H

#include <utf8string.h>

// Hashtable Indexes
#define MATRA_AA   0
#define MATRA_I    1
#define MATRA_E    2
#define MATRA_AI   3
#define MATRA_HLR  4
#define MATRA_HLRR 5
#define LETTER_A   6
#define LETTER_AU  7
#define LETTER_KA  8
#define LETTER_HA  9
#define HALANTA   10


class IndicLigaturizer{
public:
	uint16_t langTable[11];
	bool isRTL() {

protected:
	bool IsSwaraLetter(String ch) {
	bool IsSwaraMatra(String ch) {
	bool IsVyanjana(String ch) {

private:
	void swap(StringBuilder s, int i, int j) {
	
};


#endif
