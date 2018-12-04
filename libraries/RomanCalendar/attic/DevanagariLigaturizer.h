/*
 *
 * This file is part of the iText (R) project.
 * Copyright (c) 1998-2016 iText Group NV
 * Authors: Ram Narayan, Bruno Lowagie, et al.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation with the addition of the
 * following permission added to Section 15 as permitted in Section 7(a):
 * FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
 * ITEXT GROUP. ITEXT GROUP DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
 * OF THIRD PARTY RIGHTS
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, see http://www.gnu.org/licenses or write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA, 02110-1301 USA, or download the license from the following URL:
 * http://itextpdf.com/terms-of-use/
 *
 * The interactive user interfaces in modified source and object code versions
 * of this program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU Affero General Public License.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public License,
 * a covered work must retain the producer line in every PDF that is created
 * or manipulated using iText.
 *
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the iText software without
 * disclosing the source code of your own applications.
 * These activities include: offering paid services to customers as an ASP,
 * serving PDFs on the fly in a web application, shipping iText with a closed
 * source product.
 *
 * For more information, please contact iText Software Corp. at this
 * address: sales@itextpdf.com
 */
 
 /* converted to c++ from java by P Lishman */
 
 #include <IndicLigaturizer.h>
 
/**
 * Implementation of the IndicLigaturizer for Devanagari.
 *
 * Warning: this is an incomplete and experimental implementation of Devanagari. This implementation should not be used in production.
 */
 
 // Devanagari characters
#define DEVA_MATRA_AA   0x093E
#define DEVA_MATRA_I    0x093F
#define DEVA_MATRA_E    0x0947
#define DEVA_MATRA_AI   0x0948
#define DEVA_MATRA_HLR  0x0962
#define DEVA_MATRA_HLRR 0x0963
#define DEVA_LETTER_A   0x0905
#define DEVA_LETTER_AU  0x0914
#define DEVA_LETTER_KA  0x0915
#define DEVA_LETTER_HA  0x0939
#define DEVA_HALANTA    0x094D

class DevanagariLigaturizer:IndicLigaturizer {    
public:
	/**
     * Constructor for the IndicLigaturizer for Devanagari.
     */
    DevanagariLigaturizer() {
        langTable[MATRA_AA]   = DEVA_MATRA_AA;
        langTable[MATRA_I]    = DEVA_MATRA_I;
        langTable[MATRA_E]    = DEVA_MATRA_E;
        langTable[MATRA_AI]   = DEVA_MATRA_AI;
        langTable[MATRA_HLR]  = DEVA_MATRA_HLR;
        langTable[MATRA_HLRR] = DEVA_MATRA_HLRR;
        langTable[LETTER_A]   = DEVA_LETTER_A;
        langTable[LETTER_AU]  = DEVA_LETTER_AU;
        langTable[LETTER_KA]  = DEVA_LETTER_KA;
        langTable[LETTER_HA]  = DEVA_LETTER_HA;
        langTable[HALANTA]    = DEVA_HALANTA;
    }
};
