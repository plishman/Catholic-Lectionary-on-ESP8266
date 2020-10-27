#pragma once
#ifndef _UTF8STRING_H
#define _UTF8STRING_H
#ifndef _WIN32
#include <Arduino.h>
#include <pgmspace.h>
#else
#include <stdio.h>
#include <stdint.h>
typedef uint8_t byte;
#include "wstring.h"
#endif

// The bidi types

/** Left-to-right*/
#define BIDI_L		0

/** Left-to-Right Embedding */
#define BIDI_LRE	1

/** Left-to-Right Override */
#define BIDI_LRO	2

/** Right-to-Left */
#define BIDI_R		3

/** Right-to-Left Arabic */
#define BIDI_AL		4

/** Right-to-Left Embedding */
#define BIDI_RLE	5

/** Right-to-Left Override */
#define BIDI_RLO	6

/** Pop Directional Format */
#define BIDI_PDF	7

/** European Number */
#define BIDI_EN		8

/** European Number Separator */
#define BIDI_ES		9

/** European Number Terminator */
#define BIDI_ET		10

/** Arabic Number */
#define BIDI_AN		11

/** Common Number Separator */
#define BIDI_CS		12

/** Non-Spacing Mark */
#define BIDI_NSM	13

/** Boundary Neutral */
#define BIDI_BN		14

/** Paragraph Separator */
#define BIDI_B		15

/** Segment Separator */
#define BIDI_S		16

/** Whitespace */
#define BIDI_WS		17

/** Other Neutrals */
#define BIDI_ON		18

/** Minimum bidi type value. */
#define BIDI_TYPE_MIN	0

/** Maximum bidi type value. */
#define BIDI_TYPE_MAX	18

int charLenBytesUTF8(char s);
String utf8fromCodepoint(uint32_t c);
uint32_t codepointUtf8(String c);
String utf8CharAt(String& s, unsigned int pos);
void SetUtf8CharByIndex(String& s, int index, String ch);
String GetUtf8CharByIndex(String s, unsigned int index);
int Utf8CharCount(String s);
String Utf8substring(String s, unsigned int startutf8charindex, unsigned int endutf8charindex);
String DeleteUtf8CharByIndex(String s, unsigned int i);
String Utf8ReverseString(String instr);
bool IsRightToLeftChar(uint32_t c);
bool IsRightToLeftChar(String ch);

#ifndef _WIN32
//uint16_t baseTypes[] PROGMEM;
#else
uint16_t baseTypes[];
#endif

byte getBidiDirection(uint16_t uch);
byte getBidiDirection(String ch);

String SuperScriptNumber(int v);
#endif