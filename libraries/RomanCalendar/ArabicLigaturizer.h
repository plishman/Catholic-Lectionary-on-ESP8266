#ifndef _ARABICLIGATURIZER_H
#define _ARABICLIGATURIZER_H

#include "RCGlobals.h"

#ifndef _WIN32
#include <Arduino.h>
#include <I2CSerialPort.h>
#include <utf8string.h>
#else
#include <stdint.h>
#include "wstring.h"
#include "utf8string.h"
#endif

class Charstruct {
public:
	uint16_t basechar = 0;
	uint16_t mark1 = 0;               /* has to be initialized to zero */
	uint16_t vowel = 0;
	int lignum = 0;           /* is a ligature with lignum aditional characters */
	int numshapes = 1;
		
	void clear() {
		basechar = 0;
		mark1 = 0;
		vowel = 0;
		lignum = 0;
		numshapes = 1;
	}
	
	Charstruct() {
		basechar = 0;
		mark1 = 0;               /* has to be initialized to zero */
		vowel = 0;
		lignum = 0;           /* is a ligature with lignum aditional characters */
		numshapes = 1;
	}

	Charstruct(const Charstruct &c2) { // copy constructor
		 basechar = c2.basechar; 
		 mark1 = c2.mark1; 
		 vowel = c2.vowel; 
		 lignum = c2.lignum;
		 numshapes = c2.numshapes;
	}
};

class ArabicLigaturizer
{
public:
	static const int ar_nothing  = 0x0;
	static const int ar_novowel = 0x1;
	static const int ar_composedtashkeel = 0x4;
	static const int ar_lig = 0x8;
	/**
	* Digit shaping option: Replace European digits (U+0030...U+0039) by Arabic-Indic digits.
	*/
	static const int DIGITS_EN2AN = 0x20;
	
	/**
	* Digit shaping option: Replace Arabic-Indic digits by European digits (U+0030...U+0039).
	*/
	static const int DIGITS_AN2EN = 0x40;
	
	/**
	* Digit shaping option:
	* Replace European digits (U+0030...U+0039) by Arabic-Indic digits
	* if the most recent strongly directional character
	* is an Arabic letter (its Bidi direction value is RIGHT_TO_LEFT_ARABIC).
	* The initial state at the start of the text is assumed to be not an Arabic,
	* letter, so European digits at the start of the text will not change.
	* Compare to DIGITS_ALEN2AN_INIT_AL.
	*/
	static const int DIGITS_EN2AN_INIT_LR = 0x60;
	
	/**
	* Digit shaping option:
	* Replace European digits (U+0030...U+0039) by Arabic-Indic digits
	* if the most recent strongly directional character
	* is an Arabic letter (its Bidi direction value is RIGHT_TO_LEFT_ARABIC).
	* The initial state at the start of the text is assumed to be an Arabic,
	* letter, so European digits at the start of the text will change.
	* Compare to DIGITS_ALEN2AN_INT_LR.
	*/
	static const int DIGITS_EN2AN_INIT_AL = 0x80;
	
	/** Not a valid option value. */
	//private const int DIGITS_RESERVED = 0xa0;
	
	/**
	* Bit mask for digit shaping options.
	*/
	static const int DIGITS_MASK = 0xe0;
	
	/**
	* Digit type option: Use Arabic-Indic digits (U+0660...U+0669).
	*/
	static const int DIGIT_TYPE_AN = 0;
	
	/**
	* Digit type option: Use Eastern (Extended) Arabic-Indic digits (U+06f0...U+06f9).
	*/
	static const int DIGIT_TYPE_AN_EXTENDED = 0x100;

	/**
	* Bit mask for digit type options.
	*/
	static const int DIGIT_TYPE_MASK = 0x0100; // 0x3f00?



    static bool IsVowel(String s);
	static bool IsVowel(uint32_t s);
    
	static String Charshape(String s, int which);
	static String Charshape(uint32_t s, int which);

	static int Shapecount(String s);
	static int Shapecount(uint32_t s);

	static int Ligature(String newchar, Charstruct& oldchar);
    static int Ligature(uint32_t newchar, Charstruct& oldchar);
	
	static void Copycstostring(String& str, Charstruct& s, int level);
    static void Doublelig(String& str, int level);
    static bool Connects_to_left(Charstruct a);
    static void Shape(String text, String& str, int level);
	//static int Arabic_shape(String src, int srcoffset, int srclength, String dest, int destoffset, int destlength, int level);
	static void ProcessNumbers(String text, int offset, int length, int options);

private:
	/** Not a valid option value. */
	static const int DIGITS_RESERVED = 0xa0;
	
	static const uint16_t ALEF = 0x0627;
	static const uint16_t ALEFHAMZA = 0x0623;
	static const uint16_t ALEFHAMZABELOW = 0x0625;
	static const uint16_t ALEFMADDA = 0x0622;
	static const uint16_t LAM = 0x0644;
	static const uint16_t HAMZA = 0x0621;
	static const uint16_t TATWEEL = 0x0640;
	static const uint16_t ZWJ = 0x200D;
	
	static const uint16_t HAMZAABOVE = 0x0654;
	static const uint16_t HAMZABELOW = 0x0655;
	
	static const uint16_t WAWHAMZA = 0x0624;
	static const uint16_t YEHHAMZA = 0x0626;
	static const uint16_t WAW = 0x0648;
	static const uint16_t ALEFMAKSURA = 0x0649;
	static const uint16_t YEH = 0x064A;
	static const uint16_t FARSIYEH = 0x06CC;
	
	static const uint16_t SHADDA = 0x0651;
	static const uint16_t KASRA = 0x0650;
	static const uint16_t FATHA = 0x064E;
	static const uint16_t DAMMA = 0x064F;
	static const uint16_t MADDA = 0x0653;
	
	static const uint16_t LAM_ALEF = 0xFEFB;
	static const uint16_t LAM_ALEFHAMZA = 0xFEF7;
	static const uint16_t LAM_ALEFHAMZABELOW = 0xFEF9;
	static const uint16_t LAM_ALEFMADDA = 0xFEF5;
	
	static const uint16_t chartable_length = 76;
	static const uint8_t  chartable_shapecount_index = 5;

#ifndef _WIN32
	static uint16_t chartable[76][6] PROGMEM;
#else
	static uint16_t chartable[76][6];
#endif

	static int Arabic_shape(String src, int srcoffset, int srclength, String& dest, int destoffset, int destlength, int level);	
	static void ShapeToArabicDigitsWithContext(String dest, int start, int length, uint16_t digitBase, bool lastStrongWasAL);
};


#endif