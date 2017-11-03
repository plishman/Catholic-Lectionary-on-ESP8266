//#include "stdafx.h"
#include "Csv.h"

#define byte unsigned char

Csv::Csv()
{
}


Csv::~Csv()
{
}


//---------------------------------------
int Csv::charLenBytesUTF8(char s) {
	byte ch = (byte)s;
	//Serial.println(String(ch)+ ";");

	byte b;

	b = (ch & 0xE0);  // 2-byte utf-8 characters start with 110xxxxx
	if (b == 0xC0) return 2;

	b = (ch & 0xF0);  // 3-byte utf-8 characters start with 1110xxxx
	if (b == 0xE0) return 3;

	b = (ch & 0xF8);  // 4-byte utf-8 characters start with 11110xxx
	if (b == 0xF0) return 4;

	b = (ch & 0xC0);  // bytes within multibyte utf-8 characters are 10xxxxxx
	if (b == 0x80) return 0; //somewhere in a multi-byte utf-8 character, so don't know the length. Return 0 so the scanner can keep looking

	return 1; // character must be 0x7F or below, so return 1 (it is an ascii character)
}






//---------------------------------------
String Csv::utf8CharAt(String s, int pos) {
	//Serial.println("String=" + s);

	if (pos >= s.length()) {
		//Serial.println("utf8CharAt string length is " + String(ps->length()) + " *ppos = " + String(*ppos));
		return String("");
	}

	int charLen = charLenBytesUTF8(s.charAt(pos));

	//Serial.println("char at pos " + String(*ppos) + " = " + String(ps->charAt(*ppos)) + "utf8 charLen = " + String(charLen));

	if (charLen == 0) {
		return String("");
	}
	else {
		//Serial.print("substring is" + s.substring(pos, pos+charLen));
		return s.substring(pos, pos + charLen);
	}
}



//---------------------------------------
String Csv::getCsvField(String csvLine, int* ppos) {
	//Serial.println("getCsvField: csvLine=" + csvLine);
	//int pos = 0;
	String field = "";
	String currChar = "";
	bool bDone = false;

	currChar = utf8CharAt(csvLine, *ppos);

	if (currChar == "\n") {
		return String("");
	}

	if (currChar == "\"") {
		*ppos += String("\"").length();
		//Serial.println("csv: found a string");
		return readCsvString(csvLine, ppos, false);
	}

	//  if (currChar.length() != 1 || currChar == "") {             // recovered character was a partial utf-8 or a 2, 3 or 4 byte utf8 character, so not a digit
	//    *pos += ((currChar.length() == 0) ? 1 : currChar.length()); // skip over this character, so the csv scanner can at least move on, and not get stuck here (if it's a length of zero, then some partial utf8 character was encountered, which is one byte)
	//  }

	char currDigit = currChar.charAt(0);

	if (isDigit(currDigit)) {
		//Serial.println("csv: found a digit");
		//Serial.println("getCsvField:csvLine = " + csvLine);
		return readCsvNumber(csvLine, ppos);
	}

	return readCsvString(csvLine, ppos, true); // is an unquoted string (single word)      
											   //*ppos++; //skip this character, it wasn't the start of a digit or a string field.
	return String("-");
}



//---------------------------------------
String Csv::readCsvNumber(String csvLine, int* ppos) {
	String field = "";
	String currChar;
	bool bDone = false;
	bool bGotDecimalPoint = false;
	char currDigit;

	//Serial.println("readCsvNumber()");

	//Serial.println("readCsvNumber: *ppos = " + String(*ppos));
	//Serial.println("readCsvNumber: csvLine, csvLine.length() = " + String(csvLine.length()));

	while (!bDone) {
		currChar = utf8CharAt(csvLine, *ppos);

		if (currChar == ",") {
			bDone = true;
			*ppos += currChar.length();                                   // leave char counter pointing at next char after the field delimeter ','
			continue;                                                     // not a double quote, so end of field
		}

		if (*ppos >= csvLine.length()) {                                // zero based, so length() is one more than highest index
			//field = "";                                                   // partial field recovered before end of string encountered, so return empty string
			bDone = true;                                                 // at end of string (not line), partial field recovered
		}

		//Serial.println("currChar.length() = " + String(currChar.length()));
		
		if (currChar.length() != 1 || currChar == "") {             // recovered character was a partial utf-8 or a 2, 3 or 4 byte utf8 character, so not a digit
																	//Serial.print("Character is " + currChar + " length is " + currChar.length());
			//field = "";
			bDone = true;
			*ppos += ((currChar.length() == 0) ? 1 : currChar.length()); // skip over this character, so the csv scanner can at least move on, and not get stuck here (if it's a length of zero, then some partial utf8 character was encountered, which is one byte)
			continue;                                                 // it can't be a 7-bit ascii digit, must be some other 2, 3 or 4 byte utf-8 character
		}

		currDigit = currChar.charAt(0);
		//Serial.println("currDigit = " + String(currDigit) );

		if (isDigit(currDigit)) {
			//Serial.println("is a digit");
			field += currChar;
			*ppos += currChar.length();
			continue;
		}

		if (currChar == ".") {
			if (bGotDecimalPoint == false) {
				//Serial.println("is a decimal point");
				bGotDecimalPoint = true;
				field += currChar;
				*ppos += currChar.length();
				continue;
			}
			else {
#ifndef _WIN32
				Serial.println("more than one decimal point found");
#else
				printf("more than one decimal point found");
#endif
				field = "";                                                 // partial number is no use, so return empty string if more than one decimal point found
				bDone = true;
			}
		}
	}

	//Serial.println("number field=" + field);
	return field;
}



//---------------------------------------
String Csv::readCsvString(String csvLine, int* ppos, bool bSingleWordUnquoted) {
	String currChar = "";
	String field = "";
	bool bDone = false;
	bool bLookingForEscapeQuote = false;

	while (!bDone) {
		//Serial.print("pos=" + String(*ppos));

		currChar = utf8CharAt(csvLine, *ppos);

		if (currChar == "") {                                // recovered character was a partial utf-8, so utf8CharAt returned an empty string-> corrupted string or read past end of string.
			bDone = true;
			continue;
		}

		if (bSingleWordUnquoted && (currChar == "," || currChar == " ")) {
			//Serial.println("single word unquoted test: at end of string");
			*ppos += currChar.length();                                      // skip over the this character following the field delimiting quote, so the next character will not be a comma (field separator)
			bDone = true;
			continue;
		}

		if (bLookingForEscapeQuote == true && currChar != "\"") {       // if expecting a double quote, and got something else instead, its the end of the field.
#ifndef _WIN32
			//Serial.println("looking for escape quote and didn't find it: at end of string");
#else
			//printf("looking for escape quote and didn't find it: at end of string");
#endif
			*ppos += currChar.length();                                      // skip over the this character following the field delimiting quote, so the next character will not be a comma (field separator)
			bDone = true;
			continue;
		}

		if (currChar == "\"") {                                         // is the current character a quote
			if (bLookingForEscapeQuote == false) {                        // if not looking for an escape quote this is the first quote
																		  //Serial.println("now looking for escape quote");
				bLookingForEscapeQuote = true;                              // so set flag to say expecting next character to be a quote if it is within the field rather than the field delimiter
				*ppos += currChar.length();                                   // skip over the character
				continue;                                                   // and read the next character. If it's a quote, it will be added to the string, if not, it will not and the end of the
			}                                                             // field will have been reached
			else {                                                        // if true, this is the second quote, hence the quote is escaped and not at the end of the line
																		  //Serial.println("no longer looking for escape quote");
				bLookingForEscapeQuote = false;                             // reset the looking for escaped quote flag, no longer looking for it
			}
		}

		field += currChar;
		*ppos += currChar.length();                                       // next character

		if (*ppos >= csvLine.length()) bDone = true;                      // at end of string (not line), partial field recovered
	}

	//Serial.println();
	//Serial.println("field = " + field);

	return field;
}

bool Csv::isDigit(char c)
{
	String digits = "0123456789";
	
	return (digits.indexOf(c) != -1);
}
