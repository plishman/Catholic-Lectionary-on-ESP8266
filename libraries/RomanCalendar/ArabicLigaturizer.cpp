//using System;
//using System.Text;
#include "stdafx.h"
#ifndef _WIN32
#include <ArabicLigaturizer.h>
#else
#include "ArabicLigaturizer.h"
#endif

/*
 * Copyright 2003 by Paulo Soares.
 *
 * The contents of this file are subject to the Mozilla Public License Version 1.1
 * (the "License"); you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the License.
 *
 * The Original Code is 'iText, a free JAVA-PDF library'.
 *
 * The Initial Developer of the Original Code is Bruno Lowagie. Portions created by
 * the Initial Developer are Copyright (C) 1999, 2000, 2001, 2002 by Bruno Lowagie.
 * All Rights Reserved.
 * Co-Developer of the code is Paulo Soares. Portions created by the Co-Developer
 * are Copyright (C) 2000, 2001, 2002 by Paulo Soares. All Rights Reserved.
 *
 * Contributor(s): all the names of the contributors are added in the source code
 * where applicable.
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * LGPL license (the "GNU LIBRARY GENERAL PUBLIC LICENSE"), in which case the
 * provisions of LGPL are applicable instead of those above.  If you wish to
 * allow use of your version of this file only under the terms of the LGPL
 * License and not to allow others to use your version of this file under
 * the MPL, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the LGPL.
 * If you do not delete the provisions above, a recipient may use your version
 * of this file under either the MPL or the GNU LIBRARY GENERAL PUBLIC LICENSE.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MPL as stated above or under the terms of the GNU
 * Library General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Library general Public License for more
 * details.
 *
 * If you didn't download this code from the following link, you should check if
 * you aren't using an obsolete version:
 * http://www.lowagie.com/iText/
 */

//namespace iTextSharp.text.pdf {

    /**
    * Shape arabic characters. This code was inspired by an LGPL'ed C library:
    * Pango ( see http://www.pango.com/ ). Note that the code of this is the
    * original work of Paulo Soares. Hence it is perfectly justifiable to distribute
    * it under the MPL.
    *
    * @author Paulo Soares (psoares@consiste.pt)
    */
	
        bool ArabicLigaturizer::IsVowel(String s) {
			return IsVowel(codepointUtf8(s));
		}
		
        bool ArabicLigaturizer::IsVowel(uint32_t s) {
            return ((s >= 0x064B) && (s <= 0x0655)) || (s == 0x0670);
        }

        String ArabicLigaturizer::Charshape(String s, int which) {
			return Charshape(codepointUtf8(s), which);			
		}
		
        String ArabicLigaturizer::Charshape(uint32_t s, int which)
        /* which 0=isolated 1=final 2=initial 3=medial */
        {
            int l, r, m;
            if ((s >= 0x0621) && (s <= 0x06D3)) {
                l = 0;
                r = chartable_length - 1;
                while (l <= r) {	// binary chop search
                    m = (l + r) / 2;
#ifndef _WIN32
					if (s == pgm_read_word(&chartable[m][0])) {
                        return utf8fromCodepoint(pgm_read_word(&chartable[m][which + 1]));
                    }
                    else if (s < pgm_read_word(&chartable[m][0])) {
#else
					if (s == chartable[m][0]) {
						return utf8fromCodepoint(chartable[m][which + 1]);
					}
					else if (s < chartable[m][0]) {
#endif
						r = m - 1;
                    }
                    else {
                        l = m + 1;
                    }
                }
            }
            else if (s >= 0xfef5 && s <= 0xfefb)
                return utf8fromCodepoint(s + which);
            return utf8fromCodepoint(s);
        }

		int ArabicLigaturizer::Shapecount(String s) {
			return Shapecount(codepointUtf8(s));
		}
		
        int ArabicLigaturizer::Shapecount(uint32_t s) {
            int l, r, m;
            if ((s >= 0x0621) && (s <= 0x06D3) && !IsVowel(s)) {
                l = 0;
                r = chartable_length - 1;
                while (l <= r) {
                    m = (l + r) / 2;
#ifndef _WIN32
                    if (s == pgm_read_word(&chartable[m][0])) {
                        return (int)(pgm_read_word(&chartable[m][chartable_shapecount_index])); //chartable[m].Length - 1;
                    }
                    else if (s < pgm_read_word(&chartable[m][0])) {
#else
					if (s == chartable[m][0]) {
						return (int)(chartable[m][chartable_shapecount_index]); //chartable[m].Length - 1;
					}
					else if (s < chartable[m][0]) {
#endif
						r = m - 1;
                    }
                    else {
                        l = m + 1;
                    }
                }
            }
            else if (s == ZWJ) {
                return 4;
            }
            return 1;
        }
        
        int ArabicLigaturizer::Ligature(String newchar, Charstruct& oldchar) {
			return Ligature(codepointUtf8(newchar), oldchar);
		}
		
        int ArabicLigaturizer::Ligature(uint32_t newchar, Charstruct& oldchar) {
        /* 0 == no ligature possible; 1 == vowel; 2 == two chars; 3 == Lam+Alef */
            int retval = 0;
            
            if (oldchar.basechar == 0)
                return 0;
            if (IsVowel(newchar)) {
                retval = 1;
                if ((oldchar.vowel != 0) && (newchar != SHADDA)) {
                    retval = 2;           /* we eliminate the old vowel .. */
                }
                switch (newchar) {
                    case SHADDA:
                        if (oldchar.mark1 == 0) {
                            oldchar.mark1 = SHADDA;
                        }
                        else {
                            return 0;         /* no ligature possible */
                        }
                        break;
                    case HAMZABELOW:
                        switch (oldchar.basechar) {
                            case ALEF:
                                oldchar.basechar = ALEFHAMZABELOW;
                                retval = 2;
                                break;
                            case LAM_ALEF:
                                oldchar.basechar = LAM_ALEFHAMZABELOW;
                                retval = 2;
                                break;
                            default:
                                oldchar.mark1 = HAMZABELOW;
                                break;
                        }
                        break;
                    case HAMZAABOVE:
                        switch (oldchar.basechar) {
                            case ALEF:
                                oldchar.basechar = ALEFHAMZA;
                                retval = 2;
                                break;
                            case LAM_ALEF:
                                oldchar.basechar = LAM_ALEFHAMZA;
                                retval = 2;
                                break;
                            case WAW:
                                oldchar.basechar = WAWHAMZA;
                                retval = 2;
                                break;
                            case YEH:
                            case ALEFMAKSURA:
                            case FARSIYEH:
                                oldchar.basechar = YEHHAMZA;
                                retval = 2;
                                break;
                            default:           /* whatever sense this may make .. */
                                oldchar.mark1 = HAMZAABOVE;
                                break;
                        }
                        break;
                    case MADDA:
                        switch (oldchar.basechar) {
                            case ALEF:
                                oldchar.basechar = ALEFMADDA;
                                retval = 2;
                                break;
                        }
                        break;
                    default:
                        oldchar.vowel = newchar;
                        break;
                }
                if (retval == 1) {
                    oldchar.lignum++;
                }
                return retval;
            }
            if (oldchar.vowel != 0) {  /* if we already joined a vowel, we can't join a Hamza */
                return 0;
            }
            
            switch (oldchar.basechar) {
                case LAM:
                    switch (newchar) {
                        case ALEF:
                            oldchar.basechar = LAM_ALEF;
                            oldchar.numshapes = 2;
                            retval = 3;
                            break;
                        case ALEFHAMZA:
                            oldchar.basechar = LAM_ALEFHAMZA;
                            oldchar.numshapes = 2;
                            retval = 3;
                            break;
                        case ALEFHAMZABELOW:
                            oldchar.basechar = LAM_ALEFHAMZABELOW;
                            oldchar.numshapes = 2;
                            retval = 3;
                            break;
                        case ALEFMADDA:
                            oldchar.basechar = LAM_ALEFMADDA;
                            oldchar.numshapes = 2;
                            retval = 3;
                            break;
                    }
                    break;
                case 0:
                    oldchar.basechar = newchar;
                    oldchar.numshapes = Shapecount(newchar);
                    retval = 1;
                    break;
            }
            return retval;
        }
        
        void ArabicLigaturizer::Copycstostring(String& str, Charstruct& s, int level) {
        /* s is a shaped charstruct; i is the index into the string */
            if (s.basechar == 0)
                return;
            
            str += utf8fromCodepoint(s.basechar);
            s.lignum--;
            if (s.mark1 != 0) {
                if ((level & ar_novowel) == 0) {
                    str += utf8fromCodepoint(s.mark1);
                    s.lignum--;
                }
                else {
                    s.lignum--;
                }
            }
            if (s.vowel != 0) {
                if ((level & ar_novowel) == 0) {
                    str += utf8fromCodepoint(s.vowel);
                    s.lignum--;
                }
                else {                       /* vowel elimination */
                    s.lignum--;
                }
            }
        }

	//private:
        // return len
        void ArabicLigaturizer::Doublelig(String& str, int level)
        /* Ok. We have presentation ligatures in our font. */
        {
            int len;
            int olen = len = Utf8CharCount(str);
            int j = 0, si = 1;
            uint32_t lapresult;
            
            while (si < olen) {
                lapresult = 0;
                if ((level & ar_composedtashkeel) != 0) {
                    switch (codepointUtf8(GetUtf8CharByIndex(str, j)/*str[j]*/)) {
                        case SHADDA:
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si)/*str[si]*/)) {
                                case KASRA:
                                    lapresult = 0xFC62;
                                    break;
                                case FATHA:
                                    lapresult = 0xFC60;
                                    break;
                                case DAMMA:
                                    lapresult = 0xFC61;
                                    break;
                                case 0x064C:
                                    lapresult = 0xFC5E;
                                    break;
                                case 0x064D:
                                    lapresult = 0xFC5F;
                                    break;
                            }
                            break;
                        case KASRA:
                            if (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/ == SHADDA)
                                lapresult = 0xFC62;
                            break;
                        case FATHA:
                            if (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/ == SHADDA)
                                lapresult = 0xFC60;
                            break;
                        case DAMMA:
                            if (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/ == SHADDA)
                                lapresult = 0xFC61;
                            break;
                    }
                }
                
                if ((level & ar_lig) != 0) {
                    switch (codepointUtf8(GetUtf8CharByIndex(str, j)/*str[j]*/)) {
                        case 0xFEDF:       /* LAM initial */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFE9E:
                                    lapresult = 0xFC3F;
                                    break;        /* JEEM final */
                                case 0xFEA0:
                                    lapresult = 0xFCC9;
                                    break;        /* JEEM medial */
                                case 0xFEA2:
                                    lapresult = 0xFC40;
                                    break;        /* HAH final */
                                case 0xFEA4:
                                    lapresult = 0xFCCA;
                                    break;        /* HAH medial */
                                case 0xFEA6:
                                    lapresult = 0xFC41;
                                    break;        /* KHAH final */
                                case 0xFEA8:
                                    lapresult = 0xFCCB;
                                    break;        /* KHAH medial */
                                case 0xFEE2:
                                    lapresult = 0xFC42;
                                    break;        /* MEEM final */
                                case 0xFEE4:
                                    lapresult = 0xFCCC;
                                    break;        /* MEEM medial */
                            }
                            break;
                        case 0xFE97:       /* TEH inital */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFEA0:
                                    lapresult = 0xFCA1;
                                    break;        /* JEEM medial */
                                case 0xFEA4:
                                    lapresult = 0xFCA2;
                                    break;        /* HAH medial */
                                case 0xFEA8:
                                    lapresult = 0xFCA3;
                                    break;        /* KHAH medial */
                            }
                            break;
                        case 0xFE91:       /* BEH inital */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFEA0:
                                    lapresult = 0xFC9C;
                                    break;        /* JEEM medial */
                                case 0xFEA4:
                                    lapresult = 0xFC9D;
                                    break;        /* HAH medial */
                                case 0xFEA8:
                                    lapresult = 0xFC9E;
                                    break;        /* KHAH medial */
                            }
                            break;
                        case 0xFEE7:       /* NOON inital */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFEA0:
                                    lapresult = 0xFCD2;
                                    break;        /* JEEM initial */
                                case 0xFEA4:
                                    lapresult = 0xFCD3;
                                    break;        /* HAH medial */
                                case 0xFEA8:
                                    lapresult = 0xFCD4;
                                    break;        /* KHAH medial */
                            }
                            break;
                            
                        case 0xFEE8:       /* NOON medial */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFEAE:
                                    lapresult = 0xFC8A;
                                    break;        /* REH final  */
                                case 0xFEB0:
                                    lapresult = 0xFC8B;
                                    break;        /* ZAIN final */
                            }
                            break;
                        case 0xFEE3:       /* MEEM initial */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFEA0:
                                    lapresult = 0xFCCE;
                                    break;        /* JEEM medial */
                                case 0xFEA4:
                                    lapresult = 0xFCCF;
                                    break;        /* HAH medial */
                                case 0xFEA8:
                                    lapresult = 0xFCD0;
                                    break;        /* KHAH medial */
                                case 0xFEE4:
                                    lapresult = 0xFCD1;
                                    break;        /* MEEM medial */
                            }
                            break;
                            
                        case 0xFED3:       /* FEH initial */
                            switch (codepointUtf8(GetUtf8CharByIndex(str, si))/*str[si]*/) {
                                case 0xFEF2:
                                    lapresult = 0xFC32;
                                    break;        /* YEH final */
                            }
                            break;
                            
                        default:
                            break;
                    }                   /* end switch string[si] */
                }
                if (lapresult != 0) {
                    SetUtf8CharByIndex(str, j, utf8fromCodepoint(lapresult));//  str[j] = lapresult;
                    len--;
                    si++;                 /* jump over one character */
                    /* we'll have to change this, too. */
                }
                else {
                    j++;
                    SetUtf8CharByIndex(str, j, GetUtf8CharByIndex(str, si)); //str[j] = str[si];
                    si++;
                }
            }
            str = Utf8substring(str, 0, len); //str.Length = len;
        }

        bool ArabicLigaturizer::Connects_to_left(Charstruct a) {
            return a.numshapes > 2;
        }
        
        void ArabicLigaturizer::Shape(String text, String& str, int level) {
    /* string is assumed to be empty and big enough.
    * text is the original text.
    * This routine does the basic arabic reshaping.
    * *len the number of non-null characters.
    *
    * Note: We have to unshape each character first!
    */
            int join;
            int which;
            String nextletter;
            
            int p = 0;                     /* initialize for output */
            Charstruct oldchar; //= new Charstruct();
            Charstruct curchar; //= new Charstruct();
			
			int len = Utf8CharCount(text);
			
            while (p < len) {
                nextletter = GetUtf8CharByIndex(text, p++); //text[p++];
                //nextletter = unshape (nextletter);
                
                join = Ligature(nextletter, curchar);
                if (join == 0) {                       /* shape curchar */
                    int nc = Shapecount(nextletter);
                    //(*len)++;
                    if (nc == 1) {
                        which = 0;        /* final or isolated */
                    }
                    else {
                        which = 2;        /* medial or initial */
                    }
                    if (Connects_to_left(oldchar)) {
                        which++;
                    }
                    
                    which = which % (curchar.numshapes);
                    curchar.basechar = codepointUtf8(Charshape(curchar.basechar, which));
                    
                    /* get rid of oldchar */
                    Copycstostring(str, oldchar, level);
                    oldchar = curchar;    /* new values in oldchar */ //uses copy constructor
                    
                    /* init new curchar */
                    curchar.clear(); // = new Charstruct();
                    curchar.basechar = codepointUtf8(nextletter);
                    curchar.numshapes = nc;
                    curchar.lignum++;
                    //          (*len) += unligature (&curchar, level);
                }
                else if (join == 1) {
                }
                //      else
                //        {
                //          (*len) += unligature (&curchar, level);
                //        }
                //      p = g_utf8_next_char (p);
            }
            
            /* Handle last char */
            if (Connects_to_left(oldchar))
                which = 1;
            else
                which = 0;
            which = which % (curchar.numshapes);
            curchar.basechar = codepointUtf8(Charshape(curchar.basechar, which));
            
            /* get rid of oldchar */
            Copycstostring(str, oldchar, level);
            Copycstostring(str, curchar, level);
        }

        int ArabicLigaturizer::Arabic_shape(String src, int srcoffset, int srclength, String& dest, int destoffset, int destlength, int level) {
            String str = Utf8ReverseString(Utf8substring(src, srcoffset, srclength)); // reverses a part of the string at offset srcoffset with length srclength (for numbers?)
			String str2 = "";
			/*
			char[] str = new char[srclength];
			for (int k = srclength + srcoffset - 1; k >= srcoffset; --k)
                str[k - srcoffset] = src[k];
            StringBuilder str2 = new StringBuilder(srclength);
            */
			Shape(str, str2, level);
            if ((level & (ar_composedtashkeel | ar_lig)) != 0)
                Doublelig(str2, level);
    //        string.Reverse();
            //System.Array.Copy(str2.ToString().ToCharArray(), 0, dest, destoffset, str2.Length);
            
			int len = Utf8CharCount(str2);
			for (int i = 0; i <= len; i++) {
				String ch = GetUtf8CharByIndex(str2, i);
				SetUtf8CharByIndex(dest, destoffset + i, ch);
			}
			
			//return str2.Length;							//		-Xfunction does not appear to be usedX-. Function shapes arabic text substrings within larger text strings
			return Utf8CharCount(str2);
        }

        void ArabicLigaturizer::ProcessNumbers(String text, int offset, int length, int options) {
            int limit = offset + length;
            if ((options & DIGITS_MASK) != 0) {
                int digitBase = 0x0030; // European digits
                switch (options & DIGIT_TYPE_MASK) {
                    case DIGIT_TYPE_AN:
                        digitBase = 0x0660;  // Arabic-Indic digits
                        break;
                        
                    case DIGIT_TYPE_AN_EXTENDED:
                        digitBase = 0x06f0;  // Eastern Arabic-Indic digits (Persian and Urdu)
                        break;
                        
                    default:
                        break;
                }
                
                switch (options & DIGITS_MASK) {
                    case DIGITS_EN2AN: {
                        int digitDelta = digitBase - 0x0030;
                        for (int i = offset; i < limit; ++i) {
                            int ch = codepointUtf8(GetUtf8CharByIndex(text, i)); //char ch = text[i];
                            if (ch <= 0x0039 && ch >= 0x0030) {
                                SetUtf8CharByIndex(text, i, utf8fromCodepoint(ch + digitDelta)); //text[i] += (char)digitDelta;
                            }
                        }
                    }
                    break;
                    
                    case DIGITS_AN2EN: {
                        int digitTop = digitBase + 9; //char digitTop = (char)(digitBase + 9);
                        int digitDelta = 0x0030 - digitBase;
                        for (int i = offset; i < limit; ++i) {
                            int ch = codepointUtf8(GetUtf8CharByIndex(text, i)); //char ch = text[i];
                            if (ch <= digitTop && ch >= digitBase) {
                                SetUtf8CharByIndex(text, i, utf8fromCodepoint(ch + digitDelta)); //text[i] += (char)digitDelta;
                            }
                        }
                    }
                    break;
                    
                    case DIGITS_EN2AN_INIT_LR:
                        ShapeToArabicDigitsWithContext(text, 0, length, digitBase, false);
                        break;
                        
                    case DIGITS_EN2AN_INIT_AL:
                        ShapeToArabicDigitsWithContext(text, 0, length, digitBase, true);
                        break;
                        
                    default:
                        break;
                }
            }
        }
        
        void ArabicLigaturizer::ShapeToArabicDigitsWithContext(String dest, int start, int length, uint16_t digitBase, bool lastStrongWasAL) {
            digitBase -= '0'; // move common adjustment out of loop
     
            int limit = start + length;
            for (int i = start; i < limit; ++i) {
                String ch = GetUtf8CharByIndex(dest, i); //char ch = dest[i];
                uint16_t uch = (uint16_t)codepointUtf8(ch);
				
				switch (getBidiDirection(uch)) {
                case BIDI_L:
                case BIDI_R:
                    lastStrongWasAL = false;
                    break;
                case BIDI_AL:
                    lastStrongWasAL = true;
                    break;
                case BIDI_EN:
                    if (lastStrongWasAL && uch <= 0x0039) {
                        SetUtf8CharByIndex(dest, i, utf8fromCodepoint(uch + digitBase)); //dest[i] = (char)(ch + digitBase);
                    }
                    break;
                default:
                    break;
                }
            }
        }

/*
        const uint16_t ALEF = 0x0627;
        const uint16_t ALEFHAMZA = 0x0623;
        const uint16_t ALEFHAMZABELOW = 0x0625;
        const uint16_t ALEFMADDA = 0x0622;
        const uint16_t LAM = 0x0644;
        const uint16_t HAMZA = 0x0621;
        const uint16_t TATWEEL = 0x0640;
        const uint16_t ZWJ = 0x200D;
                  
        const uint16_t HAMZAABOVE = 0x0654;
        const uint16_t HAMZABELOW = 0x0655;
                  
        const uint16_t WAWHAMZA = 0x0624;
        const uint16_t YEHHAMZA = 0x0626;
        const uint16_t WAW = 0x0648;
        const uint16_t ALEFMAKSURA = 0x0649;
        const uint16_t YEH = 0x064A;
        const uint16_t FARSIYEH = 0x06CC;
                  
        const uint16_t SHADDA = 0x0651;
        const uint16_t KASRA = 0x0650;
        const uint16_t FATHA = 0x064E;
        const uint16_t DAMMA = 0x064F;
        const uint16_t MADDA = 0x0653;
                  
        const uint16_t LAM_ALEF = 0xFEFB;
        const uint16_t LAM_ALEFHAMZA = 0xFEF7;
        const uint16_t LAM_ALEFHAMZABELOW = 0xFEF9;
        const uint16_t LAM_ALEFMADDA = 0xFEF5;

		const uint16_t chartable_length = 75;
		const uint8_t  chartable_shapecount_index = 5;
*/		
		// 0x0000 -> no character available for ligature
		// each character's sub-array contains: {general Unicode, Isolated, End, Middle, Beginning, number of shapes for this character}
#ifndef _WIN32
		uint16_t ArabicLigaturizer::chartable[76][6] PROGMEM = {
#else
		uint16_t ArabicLigaturizer::chartable[76][6] = {
#endif
				{0x0621, 0xFE80, 0x0000, 0x0000, 0x0000, 1}, /* HAMZA */
				{0x0622, 0xFE81, 0xFE82, 0x0000, 0x0000, 2}, /* ALEF WITH MADDA ABOVE */
				{0x0623, 0xFE83, 0xFE84, 0x0000, 0x0000, 2}, /* ALEF WITH HAMZA ABOVE */
				{0x0624, 0xFE85, 0xFE86, 0x0000, 0x0000, 2}, /* WAW WITH HAMZA ABOVE */
				{0x0625, 0xFE87, 0xFE88, 0x0000, 0x0000, 2}, /* ALEF WITH HAMZA BELOW */
				{0x0626, 0xFE89, 0xFE8A, 0xFE8B, 0xFE8C, 4}, /* YEH WITH HAMZA ABOVE */
				{0x0627, 0xFE8D, 0xFE8E, 0x0000, 0x0000, 2}, /* ALEF */
				{0x0628, 0xFE8F, 0xFE90, 0xFE91, 0xFE92, 4}, /* BEH */
				{0x0629, 0xFE93, 0xFE94, 0x0000, 0x0000, 2}, /* TEH MARBUTA */
				{0x062A, 0xFE95, 0xFE96, 0xFE97, 0xFE98, 4}, /* TEH */
				{0x062B, 0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, 4}, /* THEH */
				{0x062C, 0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, 4}, /* JEEM */
				{0x062D, 0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, 4}, /* HAH */
				{0x062E, 0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, 4}, /* KHAH */
				{0x062F, 0xFEA9, 0xFEAA, 0x0000, 0x0000, 2}, /* DAL */
				{0x0630, 0xFEAB, 0xFEAC, 0x0000, 0x0000, 2}, /* THAL */
				{0x0631, 0xFEAD, 0xFEAE, 0x0000, 0x0000, 2}, /* REH */
				{0x0632, 0xFEAF, 0xFEB0, 0x0000, 0x0000, 2}, /* ZAIN */
				{0x0633, 0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, 4}, /* SEEN */
				{0x0634, 0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, 4}, /* SHEEN */
				{0x0635, 0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, 4}, /* SAD */
				{0x0636, 0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, 4}, /* DAD */
				{0x0637, 0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, 4}, /* TAH */
				{0x0638, 0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, 4}, /* ZAH */
				{0x0639, 0xFEC9, 0xFECA, 0xFECB, 0xFECC, 4}, /* AIN */
				{0x063A, 0xFECD, 0xFECE, 0xFECF, 0xFED0, 4}, /* GHAIN */
				{0x0640, 0x0640, 0x0640, 0x0640, 0x0640, 4}, /* TATWEEL */
				{0x0641, 0xFED1, 0xFED2, 0xFED3, 0xFED4, 4}, /* FEH */
				{0x0642, 0xFED5, 0xFED6, 0xFED7, 0xFED8, 4}, /* QAF */
				{0x0643, 0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, 4}, /* KAF */
				{0x0644, 0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, 4}, /* LAM */
				{0x0645, 0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, 4}, /* MEEM */
				{0x0646, 0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, 4}, /* NOON */
				{0x0647, 0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, 4}, /* HEH */
				{0x0648, 0xFEED, 0xFEEE, 0x0000, 0x0000, 2}, /* WAW */
				{0x0649, 0xFEEF, 0xFEF0, 0xFBE8, 0xFBE9, 4}, /* ALEF MAKSURA */
				{0x064A, 0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, 4}, /* YEH */
				{0x0671, 0xFB50, 0xFB51, 0x0000, 0x0000, 2}, /* ALEF WASLA */
				{0x0679, 0xFB66, 0xFB67, 0xFB68, 0xFB69, 4}, /* TTEH */
				{0x067A, 0xFB5E, 0xFB5F, 0xFB60, 0xFB61, 4}, /* TTEHEH */
				{0x067B, 0xFB52, 0xFB53, 0xFB54, 0xFB55, 4}, /* BEEH */
				{0x067E, 0xFB56, 0xFB57, 0xFB58, 0xFB59, 4}, /* PEH */
				{0x067F, 0xFB62, 0xFB63, 0xFB64, 0xFB65, 4}, /* TEHEH */
				{0x0680, 0xFB5A, 0xFB5B, 0xFB5C, 0xFB5D, 4}, /* BEHEH */
				{0x0683, 0xFB76, 0xFB77, 0xFB78, 0xFB79, 4}, /* NYEH */
				{0x0684, 0xFB72, 0xFB73, 0xFB74, 0xFB75, 4}, /* DYEH */
				{0x0686, 0xFB7A, 0xFB7B, 0xFB7C, 0xFB7D, 4}, /* TCHEH */
				{0x0687, 0xFB7E, 0xFB7F, 0xFB80, 0xFB81, 4}, /* TCHEHEH */
				{0x0688, 0xFB88, 0xFB89, 0x0000, 0x0000, 2}, /* DDAL */
				{0x068C, 0xFB84, 0xFB85, 0x0000, 0x0000, 2}, /* DAHAL */
				{0x068D, 0xFB82, 0xFB83, 0x0000, 0x0000, 2}, /* DDAHAL */
				{0x068E, 0xFB86, 0xFB87, 0x0000, 0x0000, 2}, /* DUL */
				{0x0691, 0xFB8C, 0xFB8D, 0x0000, 0x0000, 2}, /* RREH */
				{0x0698, 0xFB8A, 0xFB8B, 0x0000, 0x0000, 2}, /* JEH */
				{0x06A4, 0xFB6A, 0xFB6B, 0xFB6C, 0xFB6D, 4}, /* VEH */
				{0x06A6, 0xFB6E, 0xFB6F, 0xFB70, 0xFB71, 4}, /* PEHEH */
				{0x06A9, 0xFB8E, 0xFB8F, 0xFB90, 0xFB91, 4}, /* KEHEH */
				{0x06AD, 0xFBD3, 0xFBD4, 0xFBD5, 0xFBD6, 4}, /* NG */
				{0x06AF, 0xFB92, 0xFB93, 0xFB94, 0xFB95, 4}, /* GAF */
				{0x06B1, 0xFB9A, 0xFB9B, 0xFB9C, 0xFB9D, 4}, /* NGOEH */
				{0x06B3, 0xFB96, 0xFB97, 0xFB98, 0xFB99, 4}, /* GUEH */
				{0x06BA, 0xFB9E, 0xFB9F, 0x0000, 0x0000, 2}, /* NOON GHUNNA */
				{0x06BB, 0xFBA0, 0xFBA1, 0xFBA2, 0xFBA3, 4}, /* RNOON */
				{0x06BE, 0xFBAA, 0xFBAB, 0xFBAC, 0xFBAD, 4}, /* HEH DOACHASHMEE */
				{0x06C0, 0xFBA4, 0xFBA5, 0x0000, 0x0000, 2}, /* HEH WITH YEH ABOVE */
				{0x06C1, 0xFBA6, 0xFBA7, 0xFBA8, 0xFBA9, 4}, /* HEH GOAL */
				{0x06C5, 0xFBE0, 0xFBE1, 0x0000, 0x0000, 2}, /* KIRGHIZ OE */
				{0x06C6, 0xFBD9, 0xFBDA, 0x0000, 0x0000, 2}, /* OE */
				{0x06C7, 0xFBD7, 0xFBD8, 0x0000, 0x0000, 2}, /* U */
				{0x06C8, 0xFBDB, 0xFBDC, 0x0000, 0x0000, 2}, /* YU */
				{0x06C9, 0xFBE2, 0xFBE3, 0x0000, 0x0000, 2}, /* KIRGHIZ YU */
				{0x06CB, 0xFBDE, 0xFBDF, 0x0000, 0x0000, 2}, /* VE */
				{0x06CC, 0xFBFC, 0xFBFD, 0xFBFE, 0xFBFF, 4}, /* FARSI YEH */
				{0x06D0, 0xFBE4, 0xFBE5, 0xFBE6, 0xFBE7, 4}, /* E */
				{0x06D2, 0xFBAE, 0xFBAF, 0x0000, 0x0000, 2}, /* YEH BARREE */
				{0x06D3, 0xFBB0, 0xFBB1, 0x0000, 0x0000, 2}  /* YEH BARREE WITH HAMZA ABOVE */
            };

            // public const int ar_nothing  = 0x0;
            // public const int ar_novowel = 0x1;
            // public const int ar_composedtashkeel = 0x4;
            // public const int ar_lig = 0x8;
            // /**
            // * Digit shaping option: Replace European digits (U+0030...U+0039) by Arabic-Indic digits.
            // */
            // public const int DIGITS_EN2AN = 0x20;
            // 
            // /**
            // * Digit shaping option: Replace Arabic-Indic digits by European digits (U+0030...U+0039).
            // */
            // public const int DIGITS_AN2EN = 0x40;
            // 
            // /**
            // * Digit shaping option:
            // * Replace European digits (U+0030...U+0039) by Arabic-Indic digits
            // * if the most recent strongly directional character
            // * is an Arabic letter (its Bidi direction value is RIGHT_TO_LEFT_ARABIC).
            // * The initial state at the start of the text is assumed to be not an Arabic,
            // * letter, so European digits at the start of the text will not change.
            // * Compare to DIGITS_ALEN2AN_INIT_AL.
            // */
            // public const int DIGITS_EN2AN_INIT_LR = 0x60;
            // 
            // /**
            // * Digit shaping option:
            // * Replace European digits (U+0030...U+0039) by Arabic-Indic digits
            // * if the most recent strongly directional character
            // * is an Arabic letter (its Bidi direction value is RIGHT_TO_LEFT_ARABIC).
            // * The initial state at the start of the text is assumed to be an Arabic,
            // * letter, so European digits at the start of the text will change.
            // * Compare to DIGITS_ALEN2AN_INT_LR.
            // */
            // public const int DIGITS_EN2AN_INIT_AL = 0x80;
            // 
            // /** Not a valid option value. */
            // private const int DIGITS_RESERVED = 0xa0;
            // 
            // /**
            // * Bit mask for digit shaping options.
            // */
            // public const int DIGITS_MASK = 0xe0;
            // 
            // /**
            // * Digit type option: Use Arabic-Indic digits (U+0660...U+0669).
            // */
            // public const int DIGIT_TYPE_AN = 0;
            // 
            // /**
            // * Digit type option: Use Eastern (Extended) Arabic-Indic digits (U+06f0...U+06f9).
            // */
            // public const int DIGIT_TYPE_AN_EXTENDED = 0x100;
            // 
            // /**
            // * Bit mask for digit type options.
            // */
            // public const int DIGIT_TYPE_MASK = 0x0100; // 0x3f00?

//            class Charstruct {
//               uint16_t basechar;
//                uint16_t mark1;               /* has to be initialized to zero */
//                uint16_t vowel;
//                int lignum;           /* is a ligature with lignum aditional characters */
//                int numshapes = 1;
//            };

