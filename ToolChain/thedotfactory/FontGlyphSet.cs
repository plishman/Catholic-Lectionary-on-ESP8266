/******************************************************************************
* Copyright © 2007 Transeric Solutions.  All Rights Reserved.
* Author: Eric David Lynch
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the company Transeric Solutions nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Transeric Solutions "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Transeric Solutions BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
using System;
using System.Collections;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Media;

namespace FontLibrary
{
	/// <summary>
	/// Class to retain information present in a GLYPHSET structure.
	/// </summary>
	public class FontGlyphSet
	{
		#region Constructors
		/// <summary>
		/// Constructor for FontGlyphSet.
		/// </summary>
		/// <param name="font">Font for which glyph set is obtained.</param>
		public FontGlyphSet (Typeface typeface/*Font font*/)
		{
			// Assume everything will fail
			IntPtr glyphSetData = IntPtr.Zero;
			IntPtr savedFont = HGDI_ERROR;
			IntPtr hdc = IntPtr.Zero;
			Graphics g = null;

			try // Try to get GLYPHSET information for Font...
			{
				// Get a GDI+ drawing surface (any will do)
				g = Graphics.FromHwnd(IntPtr.Zero);

				// Get handle to device context for the graphics object
				hdc = g.GetHdc();

                // Get a handle to our font
                IntPtr hFont = new HandleRef(null, CreateFont(10, 0, 0, 0, 0, 0, 0, 0, 1/*Ansi_encoding*/, 0, 0, 4, 0, typeface.FontFamily.Source)).Handle;
                //IntPtr hFont = font.ToHfont();

                // Replace currently selected font with our font
                savedFont = SelectObject(hdc, hFont);
				if (savedFont == HGDI_ERROR)
					throw new Exception(
						"Unexpected failure of SelectObject.");

				// Get the size (in bytes) of the GLYPHSET structure
				size = GetFontUnicodeRanges(hdc, IntPtr.Zero);
				if (size == 0)
					throw new Exception(
						"Unexpected failure of GetFontUnicodeRanges.");

				// Allocate memory to receive GLYPHSET structure
				glyphSetData = Marshal.AllocHGlobal((int) size);

				// Copy the GLYPHSET structure into allocated memory
				if (GetFontUnicodeRanges(hdc, glyphSetData) == 0)
					throw new Exception(
						"Unexpected failure of GetFontUnicodeRanges.");

				// Skip size of glyph data (at byte offset 0)
				int offset = uintSize;

				// Get flags (at byte offset 4).
				flags = (uint) Marshal.ReadInt32(
					glyphSetData, offset);
				offset += uintSize;

				// Get number of Unicode code points for the
				// font (at byte offset 8).
				codePointCount = (uint) Marshal.ReadInt32(
					glyphSetData, offset);
				offset += uintSize;

				// Get number of ranges of Unicode code points
				// for the font (at byte offset 12).
				uint rangeCount = (uint) Marshal.ReadInt32(
					glyphSetData, offset);
				offset += uintSize;

				// Create an array for the ranges
				ranges = new FontRange[rangeCount];

				// Loop to get the ranges (starting at byte offset 16)...
				for(uint index = 0; index < rangeCount; index++)
				{
					// Read first Unicode code point in range
					ushort first = (ushort) Marshal.ReadInt16(
						glyphSetData, offset);
					offset += Marshal.SizeOf(typeof(ushort));

					// Read number of Unicode code points in range
					ushort count = (ushort) Marshal.ReadInt16(
						glyphSetData, offset);
					offset += Marshal.SizeOf(typeof(ushort));

					// Create a new range
					ranges[index] = new FontRange(first, count);
				} // Loop to get the ranges (starting at byte offset 16)...
			} // Try to get GLYPHSET information for Font...
			finally // Finally, clean up...
			{
				// Free the memory used for GLYPHSET structure
				if (glyphSetData != IntPtr.Zero)
					Marshal.FreeHGlobal(glyphSetData);

				// If we have a previously selected font, restore it
				if (savedFont != HGDI_ERROR)
					SelectObject(hdc, savedFont);

				// If we have a graphics surface...
				if (g != null)
				{
					// Release handle to device context for graphics surface
					if (hdc != IntPtr.Zero)
						g.ReleaseHdc(hdc);

					// Dispose of GDI+ graphics surface
					g.Dispose();
				} // If we have a graphics surface...
			} // Finally, clean up...
		}
		#endregion // Constructors

		#region Private data
		/// <summary>
		/// Flags for GLYPHSET structure (offset 4).
		/// </summary>
		private uint flags;

		/// <summary>
		/// Number of Unicode code points for the font (offset 8).
		/// </summary>
		private uint codePointCount;

		/// <summary>
		/// Optional quick lookup table (see IsFastContains property).
		/// </summary>
		private BitArray lookupTable;

		/// <summary>
		/// Ranges of Unicode code points for the font (offset 16).
		/// </summary>
		private FontRange[] ranges;

		/// <summary>
		/// Size (in bytes) of GLYPHSET structure (offset 0).
		/// </summary>
		private uint size;
		#endregion // Private data

		#region Constants
		/// <summary>
		/// Size (in bytes) of ushort data.
		/// </summary>
		private const int ushortSize = 2;

		/// <summary>
		/// Size (in bytes) of uint data.
		/// </summary>
		private const int uintSize = 4;

		/// <summary>
		/// Constant for SelectObject return error.
		/// </summary>
		static private readonly IntPtr HGDI_ERROR = new IntPtr(-1);

		/// <summary>
		/// Constant for GLYPHSET flags.
		/// </summary>
		private const uint GS_8BIT_INDICES = 1;
		#endregion // Constants

		#region Properties
		/// <summary>
		/// Gets number of Unicode code points for the font.
		/// </summary>
		public uint CodePointCount
		{
			get {return codePointCount;}
		}

		/// <summary>
		/// Gets flags for GLYPHSET structure.
		/// </summary>
		public uint Flags
		{
			get {return flags;}
		}

		/// <summary>
		/// Gets value indicating if glyph indices are treated
		/// as if they are 8 bits wide.
		/// </summary>
		public bool Is8BitIndices
		{
			get {return ((flags & GS_8BIT_INDICES) != 0);}
		}

		/// <summary>
		/// Gets or sets value indicating if a lookup table
		/// is created to speed up Contains method.  The
		/// lookup table requires approximately 8K of memory.
		/// </summary>
		public bool IsFastContains
		{
			get {return (lookupTable != null);}
			set
			{
				// If value is unchanged, just return
				if ((lookupTable != null) == value)
					return;

				// If setting value to false...
				if ( !value )
				{
					// Destroy reference to lookup table
					lookupTable = null;
					return;
				} // If setting value to false...

				// If no ranges, we can not have a lookup table
				if (ranges == null)
					return;

				// Create a lookup table
				lookupTable = new BitArray(ushort.MaxValue + 1);

				// Loop to set bits for each range...
				foreach(FontRange range in ranges)
				{
					// Get end of range
					int end = (int) range.End;

					// Loop to set bits for code points in range...
					for(int index = (int) range.First; index < end; index++)
						lookupTable[index] = true;
				} // Loop to set bits for each range...
			}
		}

		/// <summary>
		/// Gets number of ranges of Unicode code points for the font.
		/// </summary>
		public uint RangeCount
		{
			get
			{
				if (ranges != null)
					return (uint) ranges.Length;
				return 0;
			}
		}

		/// <summary>
		/// Gets ranges of Unicode code points for the font.
		/// </summary>
		public FontRange[] Ranges
		{
			get {return ranges;}
		}

		/// <summary>
		/// Gets size (in bytes) of GLYPHSET structure.
		/// </summary>
		public uint Size
		{
			get {return size;}
		}
		#endregion // Properties

		#region Public methods
		/// <summary>
		/// Gets value indicating if font contains glyph for specified
		/// character.
		/// </summary>
		/// <param name="charValue">Character to test.</param>
		/// <returns>True, if character is in range.  Otherwise, false.</returns>
		public bool Contains (
			char charValue )
		{
			// If we have a lookup table...
			if (lookupTable != null)
				return lookupTable[(int) charValue];

			// If we have ranges...
			if (ranges != null)
			{
				// Convert character to ushort
				ushort codePoint = (ushort) charValue;

				// Loop to find range containing Unicode code point...
				foreach(FontRange range in ranges)
					if (range.Contains(codePoint))
						return true;
			} // If we have ranges...

			// Indicate that none of the ranges contained the
			// specfied code point
			return false;
		}

		/// <summary>
		/// Remove characters without glyphs.
		/// </summary>
		/// <param name="inText">String containing source text.</param>
		/// <returns>String with characters removed.</returns>
		public string RemoveAbsent (
			string inText )
		{
			// Get length of input text
			int inLength = inText.Length;

			// Create an array in which we will build output string
			char[] outText = new char[inLength];

			// Start outputting with first character
			int outIndex = 0;

			// Loop for all characters in string...
			for(int inIndex = 0; inIndex < inLength; inIndex++)
			{
				// Get character from input string
				char charValue = inText[inIndex];

				// If character has no glyph in font (or is
				// part of surrogate), skip it
				if ( !Contains(charValue) )
					continue;

				// Copy character to output
				outText[outIndex++] = charValue;
			} // Loop for all characters in string...

			// Return new string
			return new string(outText, 0, outIndex);
		}

		/// <summary>
		/// Remove characters without glyphs.
		/// </summary>
		/// <param name="inText">Character array containing source text.</param>
		/// <param name="inStart">Index of starting element within source array.</param>
		/// <param name="inLength">Number of elements within source array.</param>
		/// <param name="outText">Character array to receive modified text.</param>
		/// <param name="outStart">Index of starting element within destination array.</param>
		/// <returns>Number of elements written to destination array.</returns>
		/// <remarks>
		/// The inText and outText arrays may overlap providing that
		/// the outStart is less than or equal to the inStart.
		/// </remarks>
		public int RemoveAbsent (
			char [] inText,
			int inStart,
			int inLength,
			char [] outText,
			int outStart )
		{
			// Start outputting with first character
			int outIndex = outStart;

			// Loop for all characters in string...
			for(int inIndex = 0; inIndex < inLength; inIndex++)
			{
				// Get character from input string
				char charValue = inText[inIndex];

				// If character has no glyph in font (or is
				// part of surrogate), skip it
				if ( !Contains(charValue) )
					continue;

				// Copy character to output
				outText[outIndex++] = charValue;
			} // Loop for all characters in string...

			// Return number of characters written
			return outIndex - outStart;
		}

		/// <summary>
		/// Replace characters without glyphs with the specified
		/// replacement character.
		/// </summary>
		/// <param name="inText">String containing source text.</param>
		/// <param name="replacement">Character to be used as replacement.</param>
		/// <returns>String with fonts removed.</returns>
		public string ReplaceAbsent (
			string inText,
			char replacement )
		{
			// Get length of input text
			int inLength = inText.Length;

			// Create an array in which we will build output string
			char[] outText = new char[inLength];

			// Start outputting with first character
			int outIndex = 0;

			// Loop for all characters in string...
			for(int inIndex = 0; inIndex < inLength; inIndex++)
			{
				// Get character from input string
				char charValue = inText[inIndex];

				// If character is second part of surrogate, skip it
				if (charValue >= (char) 0xDC00 &&
					charValue <= (char) 0xDFFF)
					continue;

				// If character has no glyph in font (or is
				// part of surrogate), replace it
				if (!Contains(charValue))
					charValue = replacement;

				// Copy character to output
				outText[outIndex++] = charValue;
			} // Loop for all characters in string...

			// Return new string
			return new string(outText, 0, outIndex);
		}

		/// <summary>
		/// Replace characters without glyphs with the specified
		/// replacement character.
		/// </summary>
		/// <param name="inText">Character array containing source text.</param>
		/// <param name="inStart">Index of starting element within source array.</param>
		/// <param name="inLength">Number of elements within source array.</param>
		/// <param name="outText">Character array to receive modified text.</param>
		/// <param name="outStart">Index of starting element within destination array.</param>
		/// <param name="replacement">Character to be used as replacement.</param>
		/// <returns>Number of elements written to destination array.</returns>
		/// <remarks>
		/// The inText and outText arrays may overlap providing that
		/// the outStart is less than or equal to the inStart.
		/// </remarks>
		public int ReplaceAbsent (
			char [] inText,
			int inStart,
			int inLength,
			char [] outText,
			int outStart,
			char replacement )
		{
			// Start outputting with first character
			int outIndex = outStart;

			// Loop for all characters in string...
			for(int inIndex = 0; inIndex < inLength; inIndex++)
			{
				// Get character from input string
				char charValue = inText[inIndex];

				// If character is second part of surrogate, skip it
				if (charValue >= (char) 0xDC00 &&
					charValue <= (char) 0xDFFF)
					continue;

				// If character has no glyph in font (or is
				// part of surrogate), replace it
				if ( !Contains(charValue) )
					charValue = replacement;

				// Copy character to output
				outText[outIndex++] = charValue;
			} // Loop for all characters in string...

			// Return number of characters written
			return outIndex - outStart;
		}
        #endregion // Public methods

        #region Imported methods
        [DllImport("gdi32.dll")]
        internal static extern IntPtr CreateFont(
            int nHeight,
            int nWidth,
            int nEscapement,
            int nOrientation,
            int fnWeight,
            uint fdwItalic,
            uint fdwUnderline,
            uint fdwStrikeOut,
            uint fdwCharSet,
            uint fdwOutputPrecision,
            uint fdwClipPrecision,
            uint fdwQuality,
            uint fdwPitchAndFamily,
            string lpszFace
            );



        /// <summary>
        /// The GetFontUnicodeRanges function returns information
        /// about which Unicode characters are supported by a font.
        /// The information is returned as a GLYPHSET structure.
        /// </summary>
        /// <param name="hdc">Handle to device context.</param>
        /// <param name="lpgs">Address of GLYPHSET structure to receive data or NULL.</param>
        /// <returns>
        /// If failure, zero is returned.  If success, size of GLYPHSET
        /// structure (if lgps is NULL) or number of bytes written to
        /// GLYPHSET structure (if lgps is non-NULL).
        /// </returns>
        [DllImport("gdi32.dll")]
		private static extern uint GetFontUnicodeRanges (
			IntPtr hdc, IntPtr lpgs);

		/// <summary>
		/// The SelectObject function selects an object into the
		/// specified device context (DC). The new object replaces
		/// the previous object of the same type. 
		/// </summary>
		/// <param name="hdc">Handle to device context.</param>
		/// <param name="hgdiobj">Handle to the GDI object to be selected.</param>
		/// <returns>
		/// If success, handle to object being replaced.
		/// If failure, 
		/// </returns>
		[DllImport("gdi32.dll")]
		private static extern IntPtr SelectObject (
			IntPtr hdc, IntPtr hgdiobj);
		#endregion // Imported methods
	}
}
