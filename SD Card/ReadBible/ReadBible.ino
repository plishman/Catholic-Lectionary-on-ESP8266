#include <BibleVerse.h>

void setup() {

  Serial.begin(9600);

  while(!Serial) {
    delay(250);
  }
  Serial.println("Running");


  String s = "使用百度前必读 意见反馈"; //"oktávu Narození Páně";

  int i = 0;
  int l = s.length();

  while (i < l) {
    String utf8c = utf8CharAt(s, i);
    Serial.print(utf8c);
    i += (utf8c.length() > 0) ? utf8c.length() : 1; // if the returned utf8 character was "" (length 0), then it is a partial character, so keep skipping characters (+=1) until a whole utf-8 char is found
  }
  
  BibleVerse b(10);

  String verse;

  verse = b.verse(50,3,16);

  Serial.println(verse);

  int pos = 0;

  String f;
  do {
    Serial.println("pos = " + String(pos));
    f = getCsvField(verse, &pos);
    Serial.println(f);
  } while (f != "");
}

void loop(void) {

}

int charLenBytesUTF8(char s) {
  byte ch = (byte) s;

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

String utf8CharAt(String s, int pos) { 
  if (pos >= s.length()) return String("");
  
  int charLen = charLenBytesUTF8(s.charAt(pos));

  if (charLen == 0) {
    return String("");
  } else {
    return s.substring(pos, pos+charLen);
  }
}

String getCsvField(String csvLine, int* pos) {
  //int pos = 0;
  String field = "";
  String currChar = "";
  bool bDone = false;

  currChar = utf8CharAt(csvLine, *pos);

  if (currChar == "\n") {
    return String(""); 
  }

  if (currChar == "\"") {
    Serial.println("csv: found a string");
    return readCsvString(csvLine, *pos + String("\"").length());      
  }

  if (currChar.length() != 1 || currChar == "") {             // recovered character was a partial utf-8 or a 2, 3 or 4 byte utf8 character, so not a digit
    *pos += ((currChar.length() == 0) ? 1 : currChar.length()); // skip over this character, so the csv scanner can at least move on, and not get stuck here (if it's a length of zero, then some partial utf8 character was encountered, which is one byte)
  }

  char currDigit = currChar.charAt(0);

  if (isDigit(currDigit)) {
    Serial.println("csv: found a digit");
    return readCsvNumber(csvLine, *pos);
  }

  return String("");
}

String readCsvString(String csvLine, int* pos) {
  String currChar = "";
  String field = "";
  bool bDone = false;
  bool bLookingForEscapeQuote = false;
  
  while (!bDone) {
    currChar = utf8CharAt(csvLine, *pos);
    
    if (currChar == "") {                                // recovered character was a partial utf-8, so utf8CharAt returned an empty string-> corrupted string or read past end of string.
      bDone = true;
      continue;
    }

    if (bLookingForEscapeQuote == true && currChar != "\"") {       // if expecting a double quote, and got something else instead, its the end of the field.
     *pos+= currChar.length();                                      // skip over the this character following the field delimiting quote, so the next character will not be a comma (field separator)
      bDone = true;
      continue;
    }
    
    if (currChar == "\"") {                                         // is the current character a quote
      if (bLookingForEscapeQuote == false) {                        // if not looking for an escape quote this is the first quote
        bLookingForEscapeQuote = true;                              // so set flag to say expecting next character to be a quote if it is within the field rather than the field delimiter
        *pos+= currChar.length();                                   // skip over the character
        continue;                                                   // and read the next character. If it's a quote, it will be added to the string, if not, it will not and the end of the
      }                                                             // field will have been reached
      else {                                                        // if true, this is the second quote, hence the quote is escaped and not at the end of the line
        bLookingForEscapeQuote = false;                             // reset the looking for escaped quote flag, no longer looking for it
      }
    } 

    field += currChar;
    *pos+= currChar.length();                                       // next character

    if (*pos >= csvLine.length()) bDone = true;                      // at end of string (not line), partial field recovered
  }

  return field;
}

String readCsvNumber(String csvLine, int* pos) {
  String field = "";
  String currChar;
  bool bDone = false;
  bool bGotDecimalPoint = false;
  char currDigit;
  
  while (!bDone) {
    currChar = utf8CharAt(csvLine, *pos);
    
    if (currChar.length() != 1 || currChar == "") {             // recovered character was a partial utf-8 or a 2, 3 or 4 byte utf8 character, so not a digit
      field = "";
      bDone = true;
      *pos += ((currChar.length() == 0) ? 1 : currChar.length()); // skip over this character, so the csv scanner can at least move on, and not get stuck here (if it's a length of zero, then some partial utf8 character was encountered, which is one byte)
      continue;                                                 // it can't be a 7-bit ascii digit, must be some other 2, 3 or 4 byte utf-8 character
    }

    currDigit = currChar.charAt(0);

    if (isDigit(currDigit)) {
      field += currChar;
      *pos += currChar.length();
    }
    else if (currDigit == ".") {
      if (bGotDecimalPoint == false) {
        bGotDecimalPoint = true;      
        field += currChar;
        *pos += currChar.length();
      }
      else {
        field = "";                                                 // partial number is no use, so return empty string if more than one decimal point found
        bDone = true;
      }
    }
    else if (currChar == ",") {
        bDone = true;
        *pos += currChar.length();                                  // leave char counter pointing at next char after the field delimeter ','
        continue;                                                   // not a double quote, so end of field
    }
    else if (*pos >= csvLine.length()) {                            // zero based, so length() is one more than highest index
      field = "";                                                   // partial field recovered before end of string encountered, so return empty string
      bDone = true;                                                 // at end of string (not line), partial field recovered
    }
  }

  return field;
}

