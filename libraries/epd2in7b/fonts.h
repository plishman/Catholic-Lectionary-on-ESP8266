/**
  ******************************************************************************
  * @file    fonts.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-February-2014
  * @brief   Header for fonts.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FONTS_H
#define __FONTS_H

/* Max size of bitmap will based on a font24 (17x24) */
#define MAX_HEIGHT_FONT         24
#define MAX_WIDTH_FONT          17
#define OFFSET_BITMAP           54

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
//vv the dot factory font structs

// ==========================================================================
// structure definition
// ==========================================================================

// This structure describes a single character's display information
typedef struct
{
	const uint8_t widthBits;				// width, in bits (or pixels), of the character
	const uint32_t offset;					// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array
	
} FONT_CHAR_INFO;	

typedef struct 
{
	const uint32_t startChar;
	const uint32_t endChar;
	FONT_CHAR_INFO* fontcharinfoBlock;
} FONT_CHAR_INFO_LOOKUP;

// Describes a single font
typedef struct
{
	const uint8_t 			heightPages;	// height, in pages (8 pixels), of the font's characters
	const uint32_t 			startChar;		// the first character in the font (e.g. in charInfo and data)
	const uint32_t 			endChar;		// the last character in the font
	const uint8_t			spacePixels;	// number of pixels that a space character takes up
	FONT_CHAR_INFO_LOOKUP*	fontcharinfoBlockLookup; // points to array of fontcharinfo lookup entries
	const uint16_t			blockCount;		// number of blocks in font (set to 0 if a single-block font)
	const FONT_CHAR_INFO*	charInfo;		// pointer to array of char information (NULL if multiple blocks are used in the font, in which case FONT_CHAR_INFO_LOOKUP* will be used)
	const uint8_t*			data;			// pointer to generated array of character visual representation
		
} FONT_INFO;	

//^^ the dot factory font structs

typedef struct _tFont
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
  
} sFONT;

extern sFONT Font24;
extern sFONT Font20;
extern sFONT Font16;
extern sFONT Font12;
extern sFONT Font8;

#ifdef __cplusplus
}
#endif
  
#endif /* __FONTS_H */
 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
