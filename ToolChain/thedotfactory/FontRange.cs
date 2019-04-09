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
namespace FontLibrary
{
	/// <summary>
	/// Structure to maintain range of Unicode code points.
	/// </summary>
	public class FontRange
	{
		#region Constructors
		/// <summary>
		/// Constructor for FontRange.
		/// </summary>
		/// <param name="first">First Unicode code point in range.</param>
		/// <param name="count">Number of Unicode code points in range.</param>
		public FontRange (
			ushort first,
			ushort count )
		{
			// Remember start of this range
			this.first = first;

			// Calculate end of this range
			end = (uint) first + (uint) count;
		}
		#endregion // Constructors

		#region Private data
		/// <summary>
		/// First Unicode code point in this range.
		/// </summary>
		private ushort first;

		/// <summary>
		/// Last Unicode code point in this range (plus one).
		/// </summary>
		private uint end;
		#endregion // Private data

		#region Properties
		/// <summary>
		/// Gets number of Unicode code points in range.
		/// </summary>
		public ushort Count
		{
			get {return (ushort) (end - (uint) first);}
		}

		/// <summary>
		/// Gets end of range (Last + 1).
		/// </summary>
		public uint End
		{
			get {return end;}
		}

		/// <summary>
		/// Gets first Unicode code point in range.
		/// </summary>
		public ushort First
		{
			get {return first;}
		}

		/// <summary>
		/// Gets last Unicode code point in range.
		/// </summary>
		public ushort Last
		{
			get {return (ushort) (end - 1UL);}
		}
		#endregion // Properties

		#region Public methods
		/// <summary>
		/// Gets value indicating if range contains Unicode code point.
		/// </summary>
		/// <param name="codePoint">Unicode code point.</param>
		/// <returns>True, if code point is in range.  Otherwise, false.</returns>
		public bool Contains (
			ushort codePoint )
		{
			return (codePoint >= first && (uint) codePoint < end);
		}
		#endregion // Public methods
	}
}
