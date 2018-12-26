/*
 * Copyright 2009, Eran "Pavius" Duchan
 * This file is part of The Dot Factory.
 *
 * The Dot Factory is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation, either version 3 of the License, or (at your option) 
 * any later version. The Dot Factory is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details. You should have received a copy of the GNU General Public License along 
 * with The Dot Factory. If not, see http://www.gnu.org/licenses/.
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Windows.Media;

namespace TheDotFactory
{
    public partial class MainForm : Form
    {
        // formatting strings
        public const string HeaderStringColumnStart = "\t0b";
        public const string HeaderStringColumnMid = ", 0b";
        public const string BitString1 = "1";
        public const string BitString0 = "0";
        private static String nl = Environment.NewLine;

        // application version
        public const string ApplicationVersion = "0.1.4";

        // current loaded bitmap
        private Bitmap m_currentLoadedBitmap = null;

        // output configuration
        public OutputConfigurationManager m_outputConfigurationManager = new OutputConfigurationManager();

        // output configuration
        private OutputConfiguration m_outputConfig;

        // info per font
        public class FontInfo
        {
            public int charHeight;
            public char startChar;
            public char endChar;
            public CharacterGenerationInfo[] characters;
            public Font font;
            public string generatedChars;
        }

        // to allow mapping string/value
        class ComboBoxItem
        {
            public string name;
            public string value;

            // ctor
            public ComboBoxItem(string name, string value)
            {
                this.name = name;
                this.value = value;
            }

            // override ToString() function
            public override string ToString()
            {
                // use name
                return this.name;
            }
        }

        // a bitmap border conta
        class BitmapBorder
        {
            public int bottomY = 0;
            public int rightX = 0;
            public int topY = int.MaxValue;
            public int leftX = int.MaxValue;
        }

        // character generation information
        public class CharacterGenerationInfo
        {
            // pointer the font info
            public FontInfo fontInfo;

            // the character
            public char character;

            // the original bitmap
            public Bitmap bitmapOriginal;

            // the bitmap to generate into a string (flipped, trimmed - if applicable)
            public Bitmap bitmapToGenerate;

            // value of pages (vertical 8 bits), in serial order from top of bitmap
            public ArrayList pages;

            // character size
            public int width;
            public int height;

            // offset into total array
            public int offsetInBytes;
        }

        // holds a range of chars
        public class CharacterDescriptorArrayBlock
        {
            // characters
            public ArrayList characters;

            public char startchar;
            public char endchar;

            // holds a range of chars
            public class Character
            {
                public FontInfo font;
                public char character;
                public int height;
                public int width;
                public int offset;
                // advance width and height: distance to advance before printing next character (when character is not rotated. If rotated, these values must be rotated also in the code that outputs the characters)
                public double advanceWidth;
                public double advanceHeight;
            }
        }

        // strings for comments
        string m_commentStartString = "";
        string m_commentEndString = "";
        string m_commentBlockMiddleString = "";
        string m_commentBlockEndString = "";

        ComboBoxItem CbxCharacterRange;
        string characterrange = "";

        private Font g_font = null;

        public MainForm()
        {
            InitializeComponent();

            // set UI properties that the designer does not set correctly
            // designer sets MinSize values before initializing the splitter distance which causes an exception
            splitContainer1.SplitterDistance = 340;
            splitContainer1.Panel1MinSize = 287;
            splitContainer1.Panel2MinSize = 260;

            CbxCharacterRange = new ComboBoxItem("Utf8 Character Range", characterrange);
        }

        public bool IsCharPresentInFont(int charindex)
        {
            //try
            //{
            if (g_font == null) return false;

            string fontname = g_font.Name;

            if (g_font.Bold) fontname += " Bold";
            if (g_font.Italic) fontname += " Italic";

            Typeface typeface = new Typeface(fontname);

            //GlyphTypeface glyph;
            typeface.TryGetGlyphTypeface(out GlyphTypeface glyph);

            if (glyph == null) return false;

            IDictionary<int, ushort> characterMap = glyph.CharacterToGlyphMap;

            ushort value;

            bool glyphIsPresent = characterMap.TryGetValue(charindex, out value);

            return glyphIsPresent;
            //}

            //catch(Exception e)
            //{
            //   return false;
            //}
        }

        public bool GetFontAscentAndDescent(ref double ascent, ref double descent, ref double linespacing)
        {
            if (g_font == null) return false;

            FontStyle f = FontStyle.Regular;

            if (g_font.Bold) f |= FontStyle.Bold;
            if (g_font.Italic) f |= FontStyle.Italic;

            Graphics g = this.CreateGraphics();

            ascent = g_font.FontFamily.GetCellAscent(f); //* (g.DpiX / 72) * g_font.Size;
            descent = g_font.FontFamily.GetCellDescent(f); //* (g.DpiX / 72) * g_font.Size;
            linespacing = g_font.FontFamily.GetLineSpacing(f); //* (g.DpiX / 72) * g_font.Size;

            ascent = g_font.Size * ascent / g_font.FontFamily.GetEmHeight(f);
            descent = g_font.Size * descent / g_font.FontFamily.GetEmHeight(f);
            linespacing = g_font.Size * linespacing / g_font.FontFamily.GetEmHeight(f);

            return true;
        }


        public bool GetCharAdvanceWidthAndHeight(int charindex, ref double width, ref double height)
        {
            if (g_font == null) return false;

            try
            {
                string fontname = g_font.Name;

                if (g_font.Bold) fontname += " Bold";
                if (g_font.Italic) fontname += " Italic";

                var typeface = new Typeface(fontname);

                //new GlyphTypeface(new Uri(fontname));
                GlyphTypeface glyph;
                typeface.TryGetGlyphTypeface(out glyph);

                var character = (char)charindex;
                var charIndex = glyph.CharacterToGlyphMap[character];

                var w = glyph.AdvanceWidths[charIndex];
                var h = glyph.Height - glyph.TopSideBearings[charIndex]
                                                - glyph.BottomSideBearings[charIndex];
                //And then I found the pixel size by

                double dx, dy;

                Graphics g = this.CreateGraphics();
                dx = g.DpiX;
                dy = g.DpiY;

                var widthInEms = w * (g.DpiX / 72) * g_font.Size;
                var heightInEms = h * (g.DpiY / 72) * g_font.Size;

                width = widthInEms; // (int)Math.Round(widthInEms);
                height = heightInEms; // (int)Math.Round(heightInEms);

                return true;
            }

            catch (Exception e)
            {
                return false;
            }
        }

        // force a redraw on size changed
        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            Refresh();
        }

        // update input font
        private void updateSelectedFont(Font fnt)
        {
            g_font = fnt;

            // set text name in the text box
            txtInputFont.Text = fnt.Name;

            // add to text
            txtInputFont.Text += " " + Math.Round(fnt.Size) + "pts";
           
            // check if bold
            if (fnt.Bold)
            {
                // add to text
                txtInputFont.Text += " / Bold";
            }

            // check if italic
            if (fnt.Italic)
            {
                // add to text
                txtInputFont.Text += " / Italic";
            }

            // set the font in the text box
            txtInputText.Font = (Font)fnt.Clone();

            // save into settings
            Properties.Settings.Default.InputFont = fnt;
            Properties.Settings.Default.Save();
        }

        private void btnFontSelect_Click(object sender, EventArgs e)
        {
            // set focus somewhere else
            label1.Focus();

            // open font chooser dialog
            if (fontDlgInputFont.ShowDialog(this) == DialogResult.OK)
            {
                updateSelectedFont(fontDlgInputFont.Font);
            }
        }

        // populate preformatted text
        private void populateTextInsertCheckbox()
        {
            string all = "", numbers = "", letters = "", uppercaseLetters = "", lowercaseLetters = "", symbols = "";

            // generate characters
            for (char character = ' '; character < 127; ++character)
            {
                // add to all
                all += character;

                // classify letter
                if (Char.IsNumber(character)) numbers += character;
                else if (Char.IsSymbol(character)) symbols += character;
                else if (Char.IsLetter(character) && Char.IsLower(character)) { letters += character; lowercaseLetters += character; }
                else if (Char.IsLetter(character) && !Char.IsLower(character)) { letters += character; uppercaseLetters += character; }
            }

            // add items
            cbxTextInsert.Items.Add(new ComboBoxItem("All", all));
            cbxTextInsert.Items.Add(new ComboBoxItem("Numbers (0-9)", numbers));
            cbxTextInsert.Items.Add(new ComboBoxItem("Letters (A-z)", letters));
            cbxTextInsert.Items.Add(new ComboBoxItem("Lowercase letters (a-z)", lowercaseLetters));
            cbxTextInsert.Items.Add(new ComboBoxItem("Uppercase letters (A-Z)", uppercaseLetters));
            cbxTextInsert.Items.Add(CbxCharacterRange);

            // use first
            cbxTextInsert.SelectedIndex = 0;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // use double buffering
            DoubleBuffered = true;

            // set version
            Text = String.Format("The Dot Factory v.{0}", ApplicationVersion);

            // set input box
            txtInputText.Text = Properties.Settings.Default.InputText;

            // load font
            fontDlgInputFont.Font = Properties.Settings.Default.InputFont;

            // load configurations from file
            m_outputConfigurationManager.loadFromFile("OutputConfigs.xml");

            // update the dropdown
            m_outputConfigurationManager.comboboxPopulate(cbxOutputConfiguration);

            // get saved output config index
            int lastUsedOutputConfigurationIndex = Properties.Settings.Default.OutputConfigIndex;

            // load recently used preset
            if (lastUsedOutputConfigurationIndex >= 0 &&
                lastUsedOutputConfigurationIndex < cbxOutputConfiguration.Items.Count)
            {
                // last used
                cbxOutputConfiguration.SelectedIndex = lastUsedOutputConfigurationIndex;

                // load selected configuration
                m_outputConfig = m_outputConfigurationManager.configurationGetAtIndex(lastUsedOutputConfigurationIndex);
            }
            else
            {
                // there's no saved configuration. get the working copy (default)
                m_outputConfig = m_outputConfigurationManager.workingOutputConfiguration;
            }

            // update comment strings
            updateCommentStrings();

            // set checkbox stuff
            populateTextInsertCheckbox();

            // apply font to all appropriate places
            updateSelectedFont(Properties.Settings.Default.InputFont);
        }

        // try to parse character range
        bool characterRangePointParse(string rangePointString, ref int rangePoint)
        {
            // trim the string
            rangePointString = rangePointString.Trim();

            // try to convert
            try
            {
                // check if 0x is start of range
                if (rangePointString.Substring(0, 2) == "0x")
                {
                    // remove 0x
                    rangePointString = rangePointString.Substring(2, rangePointString.Length - 2);

                    // do the parse
                    rangePoint = Int32.Parse(rangePointString, System.Globalization.NumberStyles.HexNumber);
                }
                else
                {
                    // do the parse
                    rangePoint = Int32.Parse(rangePointString);
                }
            }
            catch
            {
                // error converting
                return false;
            }

            // success
            return true;
        }

        // expand and remove character ranges ( look for << x - y >> )
        void expandAndRemoveCharacterRanges(ref string inputString)
        {
            // create the search pattern
            //String searchPattern = @"<<.*-.*>>";
            String searchPattern = @"<<(?<rangeStart>.*?)-(?<rangeEnd>.*?)>>";

            // create the regex
            Regex regex = new Regex(searchPattern, RegexOptions.Multiline);

            // get matches
            MatchCollection regexMatches = regex.Matches(inputString);

            // holds the number of characters removed
            int charactersRemoved = 0;

            // for each match
            foreach (Match regexMatch in regexMatches)
            {
                // get range start and end
                int rangeStart = 0, rangeEnd = 0;

                // try to parse ranges
                if (characterRangePointParse(regexMatch.Groups["rangeStart"].Value, ref rangeStart) &&
                    characterRangePointParse(regexMatch.Groups["rangeEnd"].Value, ref rangeEnd))
                {
                    // remove this from the string
                    inputString = inputString.Remove(regexMatch.Index - charactersRemoved, regexMatch.Length);

                    // save the number of chars removed so that we can fixup index (the index
                    // of the match changes as we remove characters)
                    charactersRemoved += regexMatch.Length;

                    // create a string from these values
                    for (int charIndex = rangeStart; charIndex <= rangeEnd; ++charIndex)
                    {
                        // shove this character to a unicode char container
                        char unicodeChar = (char)charIndex;

                        // add this to the string
                        inputString += unicodeChar;
                    }
                }
            }
        }

        // get the characters we need to generate
        string getCharactersToGenerate()
        {
            string inputText = txtInputText.Text;

            //
            // Expand and remove all ranges from the input text (look for << x - y >>
            //

            // espand the ranges into the input text
            expandAndRemoveCharacterRanges(ref inputText);

            //
            // iterate through the inputted text and shove to sorted string, removing all duplicates
            //

            // sorted list for insertion/duplication removal
            SortedList<char, char> characterList = new SortedList<char, char>();

            // iterate over the characters in the textbox
            for (int charIndex = 0; charIndex < inputText.Length; ++charIndex)
            {
                // get teh char
                char insertionCandidateChar = inputText[charIndex];

                // insert the char, if not already in the list and if not space ()
                if (!characterList.ContainsKey(insertionCandidateChar))
                {
                    // check if space character
                    if (insertionCandidateChar == ' ' && !m_outputConfig.generateSpaceCharacterBitmap)
                    {
                        // skip - space is not encoded rather generated dynamically by the driver
                        continue;
                    }

                    // dont generate newlines
                    if (insertionCandidateChar == '\n' || insertionCandidateChar == '\r')
                    {
                        // no such characters
                        continue;
                    }

                    // not in list, add
                    characterList.Add(inputText[charIndex], ' ');
                }
            }

            // now output the sorted list to a string
            string characterListString = "";

            // iterate over the sorted characters to create the string
            foreach (char characterKey in characterList.Keys)
            {
                // add to string
                characterListString += characterKey;
            }

            // return the character
            return characterListString;
        }

        // convert a letter to bitmap
        private void convertCharacterToBitmap(char character, Font font, out Bitmap outputBitmap, Rectangle largestBitmap)
        {
            // get the string
            string letterString = character.ToString();

            // create bitmap, sized to the correct size
            outputBitmap = new Bitmap((int)largestBitmap.Width, (int)largestBitmap.Height);

            // create grahpics entity for drawing
            Graphics gfx = Graphics.FromImage(outputBitmap);

            System.Drawing.Text.TextRenderingHint textRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;

            if (m_outputConfig.bEnableHinting) textRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAliasGridFit;

            // set anti alias as required
            switch (m_outputConfig.antialiasLevel)
            {
                case OutputConfiguration.AntialiasLevel.x2:
                    gfx.TextRenderingHint = textRenderingHint;
                    break;

                case OutputConfiguration.AntialiasLevel.x4:
                    gfx.TextRenderingHint = textRenderingHint;
                    break;

                default:
                    gfx.TextRenderingHint = System.Drawing.Text.TextRenderingHint.SingleBitPerPixelGridFit; // defaults to 1bpp
                    break;
            }

            // draw centered text
            Rectangle bitmapRect = new System.Drawing.Rectangle(0, 0, outputBitmap.Width, outputBitmap.Height);

            // Set format of string.
            StringFormat drawFormat = new StringFormat();
            drawFormat.Alignment = StringAlignment.Center;

            // draw the character
            gfx.FillRectangle(System.Drawing.Brushes.White, bitmapRect);
            gfx.DrawString(letterString, font, System.Drawing.Brushes.Black, bitmapRect, drawFormat);
        }

        // returns whether a bitmap column is empty (empty means all is back color)
        private bool bitmapColumnIsEmpty(Bitmap bitmap, int column)
        {
            // for each row in the column
            for (int row = 0; row < bitmap.Height; ++row)
            {
                // is the pixel black?
                //if (bitmap.GetPixel(column, row).ToArgb() == System.Drawing.Color.Black.ToArgb())
                //if (bitmap.GetPixel(column, row).ToArgb() != System.Drawing.Color.Empty.ToArgb())
                if (bitmap.GetPixel(column, row).G < 255)
                {
                    // found. column is not empty
                    return false;
                }
            }

            // column is empty
            return true;
        }

        // returns whether a bitmap row is empty (empty means all is back color)
        private bool bitmapRowIsEmpty(Bitmap bitmap, int row)
        {
            // for each column in the row
            for (int column = 0; column < bitmap.Width; ++column)
            {
                // is the pixel black?
                //if (bitmap.GetPixel(column, row).ToArgb() == System.Drawing.Color.Black.ToArgb())
                //if (bitmap.GetPixel(column, row).ToArgb() != System.Drawing.Color.Empty.ToArgb())
                if (bitmap.GetPixel(column, row).G < 255)
                {
                        // found. column is not empty
                        return false;
                }
            }

            // column is empty
            return true;
        }

        // get the bitmaps border - that is where the black parts start
        private bool getBitmapBorder(Bitmap bitmap, BitmapBorder border)
        {
            // search for first column (x) from the left to contain data
            for (border.leftX = 0; border.leftX < bitmap.Width; ++border.leftX)
            {
                // if found first column from the left, stop looking
                if (!bitmapColumnIsEmpty(bitmap, border.leftX)) break;
            }

            // search for first column (x) from the right to contain data
            for (border.rightX = bitmap.Width - 1; border.rightX >= 0; --border.rightX)
            {
                // if found first column from the left, stop looking
                if (!bitmapColumnIsEmpty(bitmap, border.rightX)) break;
            }

            // search for first row (y) from the top to contain data
            for (border.topY = 0; border.topY < bitmap.Height; ++border.topY)
            {
                // if found first column from the left, stop looking
                if (!bitmapRowIsEmpty(bitmap, border.topY)) break;
            }

            // search for first row (y) from the bottom to contain data
            for (border.bottomY = bitmap.Height - 1; border.bottomY >= 0; --border.bottomY)
            {
                // if found first column from the left, stop looking
                if (!bitmapRowIsEmpty(bitmap, border.bottomY)) break;
            }

            // check if the bitmap contains any black pixels
            if (border.rightX == -1)
            {
                // no pixels were found
                return false;
            }
            else
            {
                // at least one black pixel was found
                return true;
            }
        }

        // iterate through the original bitmaps and find the tightest common border
        private void findTightestCommonBitmapBorder(CharacterGenerationInfo[] charInfoArray,
                                                    ref BitmapBorder tightestBorder)
        {
            // iterate through bitmaps
            for (int charIdx = 0; charIdx < charInfoArray.Length; ++charIdx)
            {
                // create a border
                BitmapBorder bitmapBorder = new BitmapBorder();

                // get the bitmaps border
                getBitmapBorder(charInfoArray[charIdx].bitmapOriginal, bitmapBorder);

                // check if we need to loosen up the tightest border
                tightestBorder.leftX = Math.Min(bitmapBorder.leftX, tightestBorder.leftX);
                tightestBorder.topY = Math.Min(bitmapBorder.topY, tightestBorder.topY);
                tightestBorder.rightX = Math.Max(bitmapBorder.rightX, tightestBorder.rightX);
                tightestBorder.bottomY = Math.Max(bitmapBorder.bottomY, tightestBorder.bottomY);

            }
        }

        // get rotate flip type according to config
        private RotateFlipType getOutputRotateFlipType()
        {
            bool fx = m_outputConfig.flipHorizontal;
            bool fy = m_outputConfig.flipVertical;
            OutputConfiguration.Rotation rot = m_outputConfig.rotation;

            // zero degree rotation
            if (rot == OutputConfiguration.Rotation.RotateZero)
            {
                // return according to flip
                if (!fx && !fy) return RotateFlipType.RotateNoneFlipNone;
                if (fx && !fy) return RotateFlipType.RotateNoneFlipX;
                if (!fx && fy) return RotateFlipType.RotateNoneFlipY;
                if (fx && fy) return RotateFlipType.RotateNoneFlipXY;
            }

            // 90 degree rotation
            if (rot == OutputConfiguration.Rotation.RotateNinety)
            {
                // return according to flip
                if (!fx && !fy) return RotateFlipType.Rotate90FlipNone;
                if (fx && !fy) return RotateFlipType.Rotate90FlipX;
                if (!fx && fy) return RotateFlipType.Rotate90FlipY;
                if (fx && fy) return RotateFlipType.Rotate90FlipXY;
            }

            // 180 degree rotation
            if (rot == OutputConfiguration.Rotation.RotateOneEighty)
            {
                // return according to flip
                if (!fx && !fy) return RotateFlipType.Rotate180FlipNone;
                if (fx && !fy) return RotateFlipType.Rotate180FlipX;
                if (!fx && fy) return RotateFlipType.Rotate180FlipY;
                if (fx && fy) return RotateFlipType.Rotate180FlipXY;
            }

            // 270 degree rotation
            if (rot == OutputConfiguration.Rotation.RotateTwoSeventy)
            {
                // return according to flip
                if (!fx && !fy) return RotateFlipType.Rotate270FlipNone;
                if (fx && !fy) return RotateFlipType.Rotate270FlipX;
                if (!fx && fy) return RotateFlipType.Rotate270FlipY;
                if (fx && fy) return RotateFlipType.Rotate270FlipXY;
            }

            // unknown case, but just return no flip
            return RotateFlipType.RotateNoneFlipNone;
        }

        // generate the bitmap we will then use to convert to string (remove pad, flip)
        private bool manipulateBitmap(Bitmap bitmapOriginal,
                                      BitmapBorder tightestCommonBorder,
                                      out Bitmap bitmapManipulated,
                                      int minWidth, int minHeight)
        {
            //
            // First, crop
            //

            // get bitmap border - this sets teh crop rectangle to per bitmap, essentially
            BitmapBorder bitmapCropBorder = new BitmapBorder();
            if (getBitmapBorder(bitmapOriginal, bitmapCropBorder) == false && minWidth == 0 && minHeight == 0)
            {
                // no data
                bitmapManipulated = null;

                // bitmap contains no data
                return false;
            }

            // check that width exceeds minimum
            if (bitmapCropBorder.rightX - bitmapCropBorder.leftX + 1 < 0)
            {
                // replace
                bitmapCropBorder.leftX = 0;
                bitmapCropBorder.rightX = minWidth - 1;
            }

            // check that height exceeds minimum
            if (bitmapCropBorder.bottomY - bitmapCropBorder.topY + 1 < 0)
            {
                // replace
                bitmapCropBorder.topY = 0;
                bitmapCropBorder.bottomY = minHeight - 1;
            }

            // should we crop hotizontally according to common
            if (m_outputConfig.paddingRemovalHorizontal == OutputConfiguration.PaddingRemoval.Fixed)
            {
                // cropped Y is according to common
                bitmapCropBorder.topY = tightestCommonBorder.topY;
                bitmapCropBorder.bottomY = tightestCommonBorder.bottomY;
            }
            // check if no horizontal crop is required
            else if (m_outputConfig.paddingRemovalHorizontal == OutputConfiguration.PaddingRemoval.None)
            {
                // set y to actual max border of bitmap
                bitmapCropBorder.topY = 0;
                bitmapCropBorder.bottomY = bitmapOriginal.Height - 1;
            }

            // should we crop vertically according to common
            if (m_outputConfig.paddingRemovalVertical == OutputConfiguration.PaddingRemoval.Fixed)
            {
                // cropped X is according to common
                bitmapCropBorder.leftX = tightestCommonBorder.leftX;
                bitmapCropBorder.rightX = tightestCommonBorder.rightX;
            }
            // check if no vertical crop is required
            else if (m_outputConfig.paddingRemovalVertical == OutputConfiguration.PaddingRemoval.None)
            {
                // set x to actual max border of bitmap
                bitmapCropBorder.leftX = 0;
                bitmapCropBorder.rightX = bitmapOriginal.Width - 1;
            }

            // now copy the output bitmap, cropped as required, to a temporary bitmap
            Rectangle rect = new Rectangle(bitmapCropBorder.leftX,
                                            bitmapCropBorder.topY,
                                            bitmapCropBorder.rightX - bitmapCropBorder.leftX + 1,
                                            bitmapCropBorder.bottomY - bitmapCropBorder.topY + 1);

            // clone the cropped bitmap into the generated one
            bitmapManipulated = bitmapOriginal.Clone(rect, bitmapOriginal.PixelFormat);

            // get rotate type
            RotateFlipType flipType = getOutputRotateFlipType();

            // flip the cropped bitmap
            bitmapManipulated.RotateFlip(flipType);

            // bitmap contains data
            return true;
        }


        // create the page array
        private void convertBitmapToPageArray(Bitmap bitmapToGenerate, out ArrayList pages)
        {
            progressBar1.Maximum = (bitmapToGenerate.Width * bitmapToGenerate.Height) * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            // create pages
            pages = new ArrayList();

            // for each row
            for (int row = 0; row < bitmapToGenerate.Height; row++)
            {
                // current byte value
                byte currentValue = 0, bitsRead = 0;

                // for each column
                for (int column = 0; column < bitmapToGenerate.Width; ++column)
                {
                    progressBar1.PerformStep();
                    Application.DoEvents();

                    // is pixel set?
                    if (bitmapToGenerate.GetPixel(column, row).ToArgb() == System.Drawing.Color.Black.ToArgb())
                    {
                        // set the appropriate bit in the page
                        if (m_outputConfig.byteOrder == OutputConfiguration.ByteOrder.MsbFirst) currentValue |= (byte)(1 << (7 - bitsRead));
                        else currentValue |= (byte)(1 << bitsRead);
                    }

                    // increment number of bits read
                    ++bitsRead;

                    // have we filled a page?
                    if (bitsRead == 8)
                    {
                        // add byte to page array
                        pages.Add(currentValue);

                        // zero out current value
                        currentValue = 0;

                        // zero out bits read
                        bitsRead = 0;
                    }
                }

                // if we have bits left, add it as is
                if (bitsRead != 0) pages.Add(currentValue);
            }

            // transpose the pages if column major data is requested
            if (m_outputConfig.bitLayout == OutputConfiguration.BitLayout.ColumnMajor)
            {
                transposePageArray(bitmapToGenerate.Width, bitmapToGenerate.Height, pages, out pages);
            }
        }


        // create the page array with 2x antialiasing (2bpp)
        private void convertBitmapToPageArrayAA2X(Bitmap bitmapToGenerate, out ArrayList pages)
        {
            progressBar1.Maximum = (bitmapToGenerate.Width * bitmapToGenerate.Height) * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            // create pages
            pages = new ArrayList();

            // for each row
            for (int row = 0; row < bitmapToGenerate.Height; row++)
            {
                // current byte value
                byte currentValue = 0, bitsRead = 0;

                // for each column
                for (int column = 0; column < bitmapToGenerate.Width; ++column)
                {
                    progressBar1.PerformStep();
                    Application.DoEvents();

                    // is pixel set?
                    byte px = 0;
                    byte gval = bitmapToGenerate.GetPixel(column, row).G; // may cause problems if font antialiasing is rendered in A rather than RGB
                    
                    // threshold to 4 values (2 bits)
                    if (gval >= 0 && gval < 64)             // was 64
                    {
                        px = 3;
                    }
                    else if (gval >= 96 && gval < 128)      //  was 64 - 128
                    {
                        px = 2;
                    }
                    else if (gval >= 128 && gval < 192)     // was 128 - 192
                    {
                        px = 1;
                    }
                    else if (gval >= 192 && gval <= 255)
                    {
                        px = 0;
                    }

                    // set the appropriate bit in the page
                    if (m_outputConfig.byteOrder == OutputConfiguration.ByteOrder.MsbFirst) currentValue |= (byte)(px << (6 - bitsRead));
                    else currentValue |= (byte)(px << bitsRead);

                    // increment number of bits read
                    bitsRead+=2;

                    // have we filled a page?
                    if (bitsRead == 8)
                    {
                        // add byte to page array
                        pages.Add(currentValue);

                        // zero out current value
                        currentValue = 0;

                        // zero out bits read
                        bitsRead = 0;
                    }
                }

                // if we have bits left, add it as is
                if (bitsRead != 0) pages.Add(currentValue);
            }

            // transpose the pages if column major data is requested
            if (m_outputConfig.bitLayout == OutputConfiguration.BitLayout.ColumnMajor)
            {
                MessageBox.Show("Column Major output is not supported if generating Antialiased font!");
                //transposePageArray(bitmapToGenerate.Width, bitmapToGenerate.Height, pages, out pages);
            }
        }


        // create the page array with 4x antialiasing (2bpp)
        private void convertBitmapToPageArrayAA4X(Bitmap bitmapToGenerate, out ArrayList pages)
        {
            progressBar1.Maximum = (bitmapToGenerate.Width * bitmapToGenerate.Height) * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            // create pages
            pages = new ArrayList();

            // for each row
            for (int row = 0; row < bitmapToGenerate.Height; row++)
            {
                // current byte value
                byte currentValue = 0, bitsRead = 0;

                // for each column
                for (int column = 0; column < bitmapToGenerate.Width; ++column)
                {
                    progressBar1.PerformStep();
                    Application.DoEvents();

                    // is pixel set?
                    byte px = 0;
                    int gval = bitmapToGenerate.GetPixel(column, row).G; // may cause problems if font antialiasing is rendered in A rather than RGB

                    Console.Write(gval.ToString("x2").PadLeft(2,'0') + " ");

                    // threshold to 16 values (4 bits)

                    px = (byte) (15 - (gval / 16));

                    // set the appropriate bit in the page
                    if (m_outputConfig.byteOrder == OutputConfiguration.ByteOrder.MsbFirst) currentValue |= (byte)(px << (4 - bitsRead));
                    else currentValue |= (byte)(px << bitsRead);

                    // increment number of bits read
                    bitsRead += 4;

                    // have we filled a page?
                    if (bitsRead == 8)
                    {
                        // add byte to page array
                        pages.Add(currentValue);

                        // zero out current value
                        currentValue = 0;

                        // zero out bits read
                        bitsRead = 0;
                    }
                }

                // if we have bits left, add it as is
                if (bitsRead != 0) pages.Add(currentValue);

                Console.WriteLine();
            }

            // transpose the pages if column major data is requested
            if (m_outputConfig.bitLayout == OutputConfiguration.BitLayout.ColumnMajor)
            {
                MessageBox.Show("Column Major output is not supported if generating Antialiased font!");
                //transposePageArray(bitmapToGenerate.Width, bitmapToGenerate.Height, pages, out pages);
            }
        }



        // get absolute height/width of characters
        private void getAbsoluteCharacterDimensions(ref Bitmap charBitmap, ref int width, ref int height)
        {
            // check if bitmap exists, otherwise set as zero
            if (charBitmap == null)
            {
                // zero height
                width = 0;
                height = 0;
            }
            else
            {
                // get the absolute font character height. Depends on rotation
                if (m_outputConfig.rotation == OutputConfiguration.Rotation.RotateZero ||
                    m_outputConfig.rotation == OutputConfiguration.Rotation.RotateOneEighty)
                {
                    // if char is not rotated or rotated 180deg, its height is the actual height
                    height = charBitmap.Height;
                    width = charBitmap.Width;
                }
                else
                {
                    // if char is rotated by 90 or 270, its height is the width of the rotated bitmap
                    height = charBitmap.Width;
                    width = charBitmap.Height;
                }
            }
        }

        // get font info from string
        private void populateFontInfoFromCharacters(ref FontInfo fontInfo)
        {
            // do nothing if no chars defined
            if (fontInfo.characters.Length == 0) return;

            // total offset
            int charByteOffset = 0;
            int dummy = 0;

            // set start char
            fontInfo.startChar = (char)0xFFFF;
            fontInfo.endChar = ' ';

            // the fixed absolute character height
            // int fixedAbsoluteCharHeight;
            getAbsoluteCharacterDimensions(ref fontInfo.characters[0].bitmapToGenerate, ref dummy, ref fontInfo.charHeight);

            // iterate through letter string
            for (int charIdx = 0; charIdx < fontInfo.characters.Length; ++charIdx)
            {
                // skip empty bitmaps
                if (fontInfo.characters[charIdx].bitmapToGenerate == null) continue;

                // get char
                char currentChar = fontInfo.characters[charIdx].character;

                // is this character smaller than start char?
                if (currentChar < fontInfo.startChar) fontInfo.startChar = currentChar;

                // is this character bigger than end char?
                if (currentChar > fontInfo.endChar) fontInfo.endChar = currentChar;

                // populate number of rows
                getAbsoluteCharacterDimensions(ref fontInfo.characters[charIdx].bitmapToGenerate,
                                                ref fontInfo.characters[charIdx].width,
                                                ref fontInfo.characters[charIdx].height);

                // populate offset of character
                fontInfo.characters[charIdx].offsetInBytes = charByteOffset;

                // increment byte offset
                charByteOffset += fontInfo.characters[charIdx].pages.Count;
            }
        }

        // get widest bitmap
        Rectangle getLargestBitmapFromCharInfo(CharacterGenerationInfo[] charInfoArray)
        {
            // largest rect
            Rectangle largestRect = new Rectangle(0, 0, 0, 0);

            // iterate through chars
            for (int charIdx = 0; charIdx < charInfoArray.Length; ++charIdx)
            {
                // get the string of the characer
                string letterString = charInfoArray[charIdx].character.ToString();

                // measure the size of teh character in pixels
                Size stringSize = TextRenderer.MeasureText(letterString, charInfoArray[charIdx].fontInfo.font);

                // check if larger
                largestRect.Height = Math.Max(largestRect.Height, stringSize.Height);
                largestRect.Width = Math.Max(largestRect.Width, stringSize.Width);
            }

            // return largest
            return largestRect;
        }

        // populate the font info
        private FontInfo populateFontInfo(Font font)
        {
            // the font information
            FontInfo fontInfo = new FontInfo();

            // get teh characters we need to generate from the input text, removing duplicates
            fontInfo.generatedChars = getCharactersToGenerate();

            // set font into into
            fontInfo.font = font;

            // array holding all bitmaps and info per character
            fontInfo.characters = new CharacterGenerationInfo[fontInfo.generatedChars.Length];

            //
            // init char infos
            //
            for (int charIdx = 0; charIdx < fontInfo.generatedChars.Length; ++charIdx)
            {
                // create char info entity
                fontInfo.characters[charIdx] = new CharacterGenerationInfo();

                // point back to teh font
                fontInfo.characters[charIdx].fontInfo = fontInfo;

                // set the character
                fontInfo.characters[charIdx].character = fontInfo.generatedChars[charIdx];
                /*
                                double w = 0;
                                double h = 0;

                                GetCharAdvanceWidthAndHeight(fontInfo.characters[charIdx].character, ref w, ref h);

                                fontInfo.characters[charIdx].advanceWidth = w;
                                fontInfo.characters[charIdx].advanceHeight = h;
                */
            }

            //
            // Find the widest bitmap size we are going to draw
            //
            Rectangle largestBitmap = getLargestBitmapFromCharInfo(fontInfo.characters);

            //
            // create bitmaps per characater
            //

            // iterate over characters
            for (int charIdx = 0; charIdx < fontInfo.generatedChars.Length; ++charIdx)
            {
                // generate the original bitmap for the character
                convertCharacterToBitmap(fontInfo.generatedChars[charIdx],
                                         font,
                                         out fontInfo.characters[charIdx].bitmapOriginal, largestBitmap);

                // save
                // fontInfo.characters[charIdx].bitmapOriginal.Save(String.Format("C:/bms/{0}.bmp", fontInfo.characters[charIdx].character));
            }

            //
            // iterate through all bitmaps and find the tightest common border. only perform
            // this if the configuration specifies
            //

            // this will contain the values of the tightest border around the characters
            BitmapBorder tightestCommonBorder = new BitmapBorder();

            // only perform if padding type specifies
            if (m_outputConfig.paddingRemovalHorizontal == OutputConfiguration.PaddingRemoval.Fixed ||
                m_outputConfig.paddingRemovalVertical == OutputConfiguration.PaddingRemoval.Fixed)
            {
                // find the common tightest border
                findTightestCommonBitmapBorder(fontInfo.characters, ref tightestCommonBorder);
            }

            //
            // iterate thruogh all bitmaps and generate the bitmap we will convert to string
            // this means performing all manipulation (pad remove, flip)
            //

            // iterate over characters
            for (int charIdx = 0; charIdx < fontInfo.generatedChars.Length; ++charIdx)
            {
                // generate the original bitmap for the character
                manipulateBitmap(fontInfo.characters[charIdx].bitmapOriginal,
                                 tightestCommonBorder,
                                 out fontInfo.characters[charIdx].bitmapToGenerate,
                                 m_outputConfig.spaceGenerationPixels,
                                 fontInfo.characters[charIdx].bitmapOriginal.Height);

                // for debugging
                // fontInfo.characters[charIdx].bitmapToGenerate.Save(String.Format("C:/bms/{0}_cropped.bmp", fontInfo.characters[charIdx].character));
            }

            //
            // iterate through all characters and create the page array
            //

            // iterate over characters
            for (int charIdx = 0; charIdx < fontInfo.generatedChars.Length; ++charIdx)
            {
                // check if bitmap exists
                if (fontInfo.characters[charIdx].bitmapToGenerate != null)
                {
                    // create the page array for the character
                    switch (m_outputConfig.antialiasLevel)
                    {
                        case OutputConfiguration.AntialiasLevel.x4:
                            convertBitmapToPageArrayAA4X(fontInfo.characters[charIdx].bitmapToGenerate, out fontInfo.characters[charIdx].pages);
                            break;

                        case OutputConfiguration.AntialiasLevel.x2:
                            convertBitmapToPageArrayAA2X(fontInfo.characters[charIdx].bitmapToGenerate, out fontInfo.characters[charIdx].pages);
                            break;

                        default:
                            convertBitmapToPageArray(fontInfo.characters[charIdx].bitmapToGenerate, out fontInfo.characters[charIdx].pages);
                            break;
                    }
                }
            }

            // populate font info
            populateFontInfoFromCharacters(ref fontInfo);

            // return the font info
            return fontInfo;
        }

        // generate string from character info
        private string generateStringFromPageArray(int width, int height, ArrayList pages)
        {
            // generate the data rows
            string[] data;
            string[] visualizer;

            switch(m_outputConfig.antialiasLevel)
            {
                case OutputConfiguration.AntialiasLevel.x4:
                    generateDataAA4X(width, height, pages, m_outputConfig.bitLayout, out data);
                    // generate the visualizer
                    generateVisualizerAA4X(width, height, pages, m_outputConfig.bitLayout, out visualizer);
                    break;

                case OutputConfiguration.AntialiasLevel.x2:
                    generateDataAA2X(width, height, pages, m_outputConfig.bitLayout, out data);
                    // generate the visualizer
                    generateVisualizerAA2X(width, height, pages, m_outputConfig.bitLayout, out visualizer);
                    break;

                default:
                    generateData(width, height, pages, m_outputConfig.bitLayout, out data);
                    // generate the visualizer
                    generateVisualizer(width, height, pages, m_outputConfig.bitLayout, out visualizer);
                    break;
            }

            // build the result string
            StringBuilder resultString = new StringBuilder();

            // output row major data
            if (m_outputConfig.bitLayout == OutputConfiguration.BitLayout.RowMajor)
            {
                // the visualizer is drawn after the data on the same rows, so they must have the same length
                System.Diagnostics.Debug.Assert(data.Length == visualizer.Length);

                // output the data and visualizer together
                if (m_outputConfig.lineWrap == OutputConfiguration.LineWrap.AtColumn)
                {
                    // one line per row
                    for (int row = 0; row != data.Length; ++row)
                    {
                        resultString.Append("\t").Append(data[row]).Append(visualizer[row]).Append(nl);
                    }
                }
                else if (m_outputConfig.lineWrap == OutputConfiguration.LineWrap.AtBitmap)
                {
                    // one line per bitmap
                    resultString.Append("\t");
                    for (int row = 0; row != data.Length; ++row)
                    {
                        resultString.Append(data[row]);
                    }
                    resultString.Append(nl);
                }
            }

            // output column major data
            else if (m_outputConfig.bitLayout == OutputConfiguration.BitLayout.ColumnMajor)
            {
                // output the visualizer
                for (int row = 0; row != visualizer.Length; ++row)
                {
                    resultString.Append("\t").Append(visualizer[row]).Append(nl);
                }

                // output the data
                if (m_outputConfig.lineWrap == OutputConfiguration.LineWrap.AtColumn)
                {
                    // one line per row
                    for (int row = 0; row != data.Length; ++row)
                    {
                        resultString.Append("\t").Append(data[row]).Append(nl);
                    }
                }
                else if (m_outputConfig.lineWrap == OutputConfiguration.LineWrap.AtBitmap)
                {
                    // one line per bitmap
                    resultString.Append("\t");
                    for (int row = 0; row != data.Length; ++row)
                    {
                        resultString.Append(data[row]);
                    }
                    resultString.Append(nl);
                }
            }

            // return the result
            return resultString.ToString();
        }

        // generate an array of column major pages from row major pages
        private void transposePageArray(int width, int height, ArrayList rowMajorPages, out ArrayList colMajorPages)
        {
            // column major data has a byte for each column representing 8 rows
            int rowMajorPagesPerRow = (width + 7) / 8;
            int colMajorPagesPerRow = width;
            int colMajorRowCount = (height + 7) / 8;

            // create an array of pages filled with zeros for the column major data
            colMajorPages = new ArrayList(colMajorPagesPerRow * colMajorRowCount);
            for (int i = 0; i != colMajorPagesPerRow * colMajorRowCount; ++i)
                colMajorPages.Add((byte)0);

            // generate the column major data
            for (int row = 0; row != height; ++row)
            {
                for (int col = 0; col != width; ++col)
                {
                    // get the byte containing the bit we want
                    int srcIdx = row * rowMajorPagesPerRow + (col / 8);
                    int page = (byte)rowMajorPages[srcIdx];

                    // get the bit mask for the bit we want
                    int bitMask = getBitMask(7 - (col % 8));

                    // set the bit in the column major data
                    if ((page & bitMask) != 0)
                    {
                        int dstIdx = (row / 8) * colMajorPagesPerRow + col;
                        int p = (byte)colMajorPages[dstIdx];
                        colMajorPages[dstIdx] = (byte)(p | getBitMask(row % 8));
                    }
                }
            }
        }

        // builds a string array of the data in 'pages'
        private void generateData(int width, int height, ArrayList pages, OutputConfiguration.BitLayout layout, out string[] data)
        {
            int colCount = (layout == OutputConfiguration.BitLayout.RowMajor) ? (width + 7) / 8 : width;
            int rowCount = (layout == OutputConfiguration.BitLayout.RowMajor) ? height : (height + 7) / 8;

            data = new string[rowCount];

            // iterator over rows
            for (int row = 0; row != rowCount; ++row)
            {
                data[row] = "";

                // iterator over columns
                for (int col = 0; col != colCount; ++col)
                {
                    // get the byte to output
                    int page = (byte)pages[row * colCount + col];

                    // add leading character
                    data[row] += m_outputConfig.byteLeadingString;

                    // check format
                    if (m_outputConfig.byteFormat == OutputConfiguration.ByteFormat.Hex)
                    {
                        // convert byte to hex
                        data[row] += page.ToString("X").PadLeft(2, '0');
                    }
                    else
                    {
                        // convert byte to binary
                        data[row] += Convert.ToString(page, 2).PadLeft(8, '0');
                    }

                    // add comma
                    data[row] += ", ";
                }
            }
        }

        // builds a string array visualization of 'pages'
        private void generateVisualizer(int width, int height, ArrayList pages, OutputConfiguration.BitLayout layout, out string[] visualizer)
        {
            visualizer = new string[height];

            // the number of pages per row in 'pages'
            int colCount = (layout == OutputConfiguration.BitLayout.RowMajor) ? (width + 7) / 8 : width;
            int rowCount = (layout == OutputConfiguration.BitLayout.RowMajor) ? height : (height + 7) / 8;

            // iterator over rows
            for (int row = 0; row != height; ++row)
            {
                // each row is started with a line comment
                visualizer[row] = "// ";

                // iterator over columns
                for (int col = 0; col != width; ++col)
                {
                    // get the byte containing the bit we want
                    int page = (layout == OutputConfiguration.BitLayout.RowMajor)
                        ? (byte)pages[row * colCount + (col / 8)]
                        : (byte)pages[(row / 8) * colCount + col];

                    // make a mask to extract the bit we want
                    int bitMask = (layout == OutputConfiguration.BitLayout.RowMajor)
                        ? getBitMask(7 - (col % 8))
                        : getBitMask(row % 8);

                    // check if bit is set
                    visualizer[row] += (bitMask & page) != 0 ? m_outputConfig.bmpVisualizerChar : " ";
                }
            }

            // for debugging
            //foreach (var s in visualizer)
            //  System.Diagnostics.Debug.WriteLine(s);
        }


        // generate strings for AA2X bitmaps
        private void generateDataAA2X(int width, int height, ArrayList pages, OutputConfiguration.BitLayout layout, out string[] data)
        {   // *2bpp
            int colCount = ((width*2) + 7) / 8; // width;               //(layout == OutputConfiguration.BitLayout.RowMajor) ? (width + 7) / 8 : width;
            int rowCount = height;          // (height + 7) / 8;    //(layout == OutputConfiguration.BitLayout.RowMajor) ? height : (height + 7) / 8;
            // rowmajor not supported for aa2x

            data = new string[rowCount];

            // iterator over rows
            for (int row = 0; row != rowCount; ++row)
            {
                data[row] = "";

                // iterator over columns
                for (int col = 0; col != colCount; ++col)
                {
                    // get the byte to output
                    int page = (byte)pages[row * colCount + col];

                    // add leading character
                    data[row] += m_outputConfig.byteLeadingString;

                    // check format
                    if (m_outputConfig.byteFormat == OutputConfiguration.ByteFormat.Hex)
                    {
                        // convert byte to hex
                        data[row] += page.ToString("X").PadLeft(2, '0');
                    }
                    else
                    {
                        // convert byte to binary
                        data[row] += Convert.ToString(page, 2).PadLeft(8, '0');
                    }

                    // add comma
                    data[row] += ", ";
                }
            }
        }

        // builds a string array visualization of 'pages'
        private void generateVisualizerAA2X(int width, int height, ArrayList pages, OutputConfiguration.BitLayout layout, out string[] visualizer)
        {
            visualizer = new string[height];

            // the number of pages per row in 'pages'
            // *2bpp
            int colCount = ((width * 2) + 7) / 8; // width;               //(layout == OutputConfiguration.BitLayout.RowMajor) ? (width + 7) / 8 : width;
            int rowCount = height;                // (height + 7) / 8;    //(layout == OutputConfiguration.BitLayout.RowMajor) ? height : (height + 7) / 8;
            // non-rowmajor not supported for aa2x

            // iterator over rows
            for (int row = 0; row != height; ++row)
            {
                // each row is started with a line comment
                visualizer[row] = "// ";
                // iterator over columns
                int bit = 6;

                for (int col = 0; col != width; ++col)
                {
                    // get the byte containing the bit we want
                    //int page = (layout == OutputConfiguration.BitLayout.RowMajor)
                    //    ? (byte)pages[row * colCount + (col / 8)]
                    //    : (byte)pages[(row / 8) * colCount + col];

                    int page = (byte)pages[row * colCount + ((col * 2) / 8)]; // 2 bpp

                    int px = 0;

                    // make a mask to extract the bit we want
                    //int bitMask = (layout == OutputConfiguration.BitLayout.RowMajor)
                    //    ? getBitMask(7 - (col % 8))
                    //    : getBitMask(row % 8);

                    px = (page & (3 << bit)) >> bit;

                    visualizer[row] += px.ToString("x1");

                    bit = (bit == 0) ? 6 : bit - 2;
                }
            }

            // for debugging
            //foreach (var s in visualizer)
            //  System.Diagnostics.Debug.WriteLine(s);
        }





        // generate strings for AA2X bitmaps
        private void generateDataAA4X(int width, int height, ArrayList pages, OutputConfiguration.BitLayout layout, out string[] data)
        {   // *2bpp
            int colCount = ((width * 4) + 7) / 8; // width;               //(layout == OutputConfiguration.BitLayout.RowMajor) ? (width + 7) / 8 : width;
            int rowCount = height;          // (height + 7) / 8;    //(layout == OutputConfiguration.BitLayout.RowMajor) ? height : (height + 7) / 8;
            // rowmajor not supported for aa2x

            data = new string[rowCount];

            // iterator over rows
            for (int row = 0; row != rowCount; ++row)
            {
                data[row] = "";

                // iterator over columns
                for (int col = 0; col != colCount; ++col)
                {
                    // get the byte to output
                    int page = (byte)pages[row * colCount + col];

                    // add leading character
                    data[row] += m_outputConfig.byteLeadingString;

                    // check format
                    if (m_outputConfig.byteFormat == OutputConfiguration.ByteFormat.Hex)
                    {
                        // convert byte to hex
                        data[row] += page.ToString("X").PadLeft(2, '0');
                    }
                    else
                    {
                        // convert byte to binary
                        data[row] += Convert.ToString(page, 2).PadLeft(8, '0');
                    }

                    // add comma
                    data[row] += ", ";
                }
            }
        }

        // builds a string array visualization of 'pages'
        private void generateVisualizerAA4X(int width, int height, ArrayList pages, OutputConfiguration.BitLayout layout, out string[] visualizer)
        {
            visualizer = new string[height];

            // the number of pages per row in 'pages'
            // *2bpp
            int colCount = ((width * 4) + 7) / 8; // width;               //(layout == OutputConfiguration.BitLayout.RowMajor) ? (width + 7) / 8 : width;
            int rowCount = height;                // (height + 7) / 8;    //(layout == OutputConfiguration.BitLayout.RowMajor) ? height : (height + 7) / 8;
            // non-rowmajor not supported for aa2x

            // iterator over rows
            for (int row = 0; row != height; ++row)
            {
                // each row is started with a line comment
                visualizer[row] = "// ";
                // iterator over columns
                int bit = 4;

                for (int col = 0; col != width; ++col)
                {
                    // get the byte containing the bit we want
                    //int page = (layout == OutputConfiguration.BitLayout.RowMajor)
                    //    ? (byte)pages[row * colCount + (col / 8)]
                    //    : (byte)pages[(row / 8) * colCount + col];

                    int page = (byte)pages[row * colCount + ((col * 4) / 8)]; // 4 bpp

                    int px = 0;

                    // make a mask to extract the bit we want
                    //int bitMask = (layout == OutputConfiguration.BitLayout.RowMajor)
                    //    ? getBitMask(7 - (col % 8))
                    //    : getBitMask(row % 8);

                    px = (page & (0xf << bit)) >> bit;

                    visualizer[row] += px.ToString("x1");

                    bit = (bit == 0) ? 4 : bit - 4;
                }
            }

            // for debugging
            //foreach (var s in visualizer)
            //  System.Diagnostics.Debug.WriteLine(s);
        }




        // return a bitMask to pick out the 'bitIndex'th bit allowing for byteOrder
        // MsbFirst: bitIndex = 0 = 0x01, bitIndex = 7 = 0x80
        // LsbFirst: bitIndex = 0 = 0x80, bitIndex = 7 = 0x01
        private int getBitMask(int bitIndex)
        {
            return m_outputConfig.byteOrder == OutputConfiguration.ByteOrder.MsbFirst
                ? 0x01 << bitIndex
                : 0x80 >> bitIndex;
        }

        // make 'name' suitable as a variable name, starting with '_'
        // or a letter and containing only letters, digits, and '_'
        private string scrubVariableName(string name)
        {
            // scrub invalid characters from the font name
            StringBuilder outName = new StringBuilder();
            foreach (char ch in name)
            {
                if (Char.IsLetterOrDigit(ch) || ch == '_')
                    outName.Append(ch);
            }

            // prepend '_' if the first character is a number
            if (Char.IsDigit(outName[0]))
                outName.Insert(0, '_');

            // convert the first character to lower case
            outName[0] = Char.ToLower(outName[0]);

            // return name
            return outName.ToString();
        }

        // get the font name and format it
        private string getFontName(ref Font font)
        {
            return scrubVariableName(font.Name + "_" + Math.Round(font.Size) + "pt");
        }

        // convert bits to bytes according to desc format
        private int convertValueByDescriptorFormat(OutputConfiguration.DescriptorFormat descFormat, int valueInBits)
        {
            // according to format
            if (descFormat == OutputConfiguration.DescriptorFormat.DisplayInBytes)
            {
                // get value in bytes
                int valueInBytes = valueInBits / 8;
                if (valueInBits % 8 != 0) valueInBytes++;

                // set into string
                return valueInBytes;
            }
            else
            {
                // no conversion required
                return valueInBits;
            }
        }

        // get the character descriptor string
        private string getCharacterDescString(OutputConfiguration.DescriptorFormat descFormat, int valueInBits)
        {
            // don't display
            if (descFormat == OutputConfiguration.DescriptorFormat.DontDisplay) return "";

            // add comma and return
            return String.Format("{0}, ", convertValueByDescriptorFormat(descFormat, valueInBits));
        }

        // get teh character descriptor string
        string getCharacterDescName(string name, OutputConfiguration.DescriptorFormat descFormat)
        {
            // don't display
            if (descFormat == OutputConfiguration.DescriptorFormat.DontDisplay) return "";

            // create result string
            string descFormatName = "";

            // set value
            if (descFormat == OutputConfiguration.DescriptorFormat.DisplayInBits) descFormatName = "bits";
            if (descFormat == OutputConfiguration.DescriptorFormat.DisplayInBytes) descFormatName = "bytes";

            // add comma and return
            return String.Format("[Char {0} in {1}], ", name, descFormatName);
        }

        // get only the variable name from an expression in a specific format
        // e.g. input: const far unsigned int my_font[] = ; 
        //      output: my_font[]
        private string getVariableNameFromExpression(string expression)
        {
            // iterator
            int charIndex = 0;

            // invalid format string
            const string invalidFormatString = "##Invalid format##";

            //
            // Strip array parenthesis
            //

            // search for '[number, zero or more] '
            const string arrayRegexString = @"\[[0-9]*\]";

            // modify the expression
            expression = Regex.Replace(expression, arrayRegexString, "");

            //
            // Find the string between '=' and a space, trimming spaces from end
            //

            // start at the end and look for the letter or number
            for (charIndex = expression.Length - 1; charIndex != 1; --charIndex)
            {
                // find the last character of the variable name
                if (expression[charIndex] != '=' && expression[charIndex] != ' ') break;
            }

            // check that its valid
            if (charIndex == 0) return invalidFormatString;

            // save this index
            int lastVariableNameCharIndex = charIndex;

            // continue looking for a space
            for (charIndex = lastVariableNameCharIndex; charIndex != 0; --charIndex)
            {
                // find the last character of the variable name
                if (expression[charIndex] == ' ') break;
            }

            // check that its valid
            if (charIndex == 0) return invalidFormatString;

            // save this index as well
            int firstVariableNameCharIndex = charIndex + 1;

            // return the substring
            return expression.Substring(firstVariableNameCharIndex, lastVariableNameCharIndex - firstVariableNameCharIndex + 1);
        }

        // add a character to teh current char descriptor array
        private void charDescArrayAddCharacter(CharacterDescriptorArrayBlock desciptorBlock,
                                               FontInfo fontInfo,
                                               char character,
                                               int width, int height, int offset)
        {
            // create character descriptor
            CharacterDescriptorArrayBlock.Character charDescriptor = new CharacterDescriptorArrayBlock.Character();
            charDescriptor.character = character;
            charDescriptor.font = fontInfo;
            charDescriptor.height = height;
            charDescriptor.width = width;
            charDescriptor.offset = offset;

            double w = 0.0;
            double h = 0.0;


            if (charDescriptor.width > 0 && charDescriptor.height > 0)
            {
                GetCharAdvanceWidthAndHeight(character, ref w, ref h);
            }

            charDescriptor.advanceWidth = w;
            charDescriptor.advanceHeight = h;

            // shove this character to the descriptor block
            desciptorBlock.characters.Add(charDescriptor);
        }

        // gnereate a list of blocks describing the characters
        private void generateCharacterDescriptorBlockList(FontInfo fontInfo, ref ArrayList characterBlockList)
        {
            char currentCharacter, previousCharacter = '\0';

            // initialize first block
            CharacterDescriptorArrayBlock characterBlock = null;

            // get the difference between two characters required to create a new group
            int differenceBetweenCharsForNewGroup = m_outputConfig.generateLookupBlocks ?
                    m_outputConfig.lookupBlocksNewAfterCharCount : int.MaxValue;

            // iterate over characters, saving previous character each time
            progressBar1.Maximum = fontInfo.characters.Length * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            for (int charIndex = 0;
                 charIndex < fontInfo.characters.Length;
                 ++charIndex)
            {
                progressBar1.PerformStep();
                Application.DoEvents();

                // get character
                currentCharacter = fontInfo.characters[charIndex].character;

                // check if this character is too far from the previous character and it isn't the first char
                if (currentCharacter - previousCharacter < differenceBetweenCharsForNewGroup && previousCharacter != '\0')
                {
                    // it may not be far enough to generate a new group but it still may be non-sequential
                    // in this case we need to generate place holders
                    for (char sequentialCharIndex = (char)(previousCharacter + 1);
                            sequentialCharIndex < currentCharacter;
                            ++sequentialCharIndex)
                    {
                        // add the character placeholder to the current char block
                        charDescArrayAddCharacter(characterBlock, fontInfo, sequentialCharIndex, 0, 0, 0);
                    }

                    // fall through and add to current block
                }
                else
                {
                    // done with current block, add to list (null is for first character which hasn't
                    // created a group yet)
                    if (characterBlock != null)
                    {
                        characterBlock.endchar = previousCharacter;
                        characterBlockList.Add(characterBlock);
                    }

                    // create new block
                    characterBlock = new CharacterDescriptorArrayBlock();
                    characterBlock.startchar = currentCharacter;
                    characterBlock.characters = new ArrayList();
                }

                // add to current block
                charDescArrayAddCharacter(characterBlock, fontInfo, currentCharacter,
                                          fontInfo.characters[charIndex].width,
                                          fontInfo.characters[charIndex].height,
                                          fontInfo.characters[charIndex].offsetInBytes);

                // save previous char
                previousCharacter = currentCharacter;
            }

            // done; add current block to list
            characterBlock.endchar = previousCharacter;
            characterBlockList.Add(characterBlock);
        }

        // get character descriptor array block name
        private string charDescArrayGetBlockName(FontInfo fontInfo, int currentBlockIndex,
                                                 bool includeTypeDefinition, bool includeBlockIndex, bool includeProgmem)
        {
            // get block id
            string blockIdString = String.Format("Block{0}", currentBlockIndex);

            // variable name
            string variableName = String.Format(m_outputConfig.varNfCharInfo, getFontName(ref fontInfo.font));

            // remove type unless required
            if (!includeTypeDefinition) variableName = getVariableNameFromExpression(variableName);

            // return the block name
            return String.Format("{0}{1}{2}{3}",
                                    variableName,
                                    includeBlockIndex ? blockIdString : "",
                                    includeTypeDefinition ? "[]" : "",
                                    includeProgmem ? " PROGMEM" : "");
        }

        // get the display string for a character (ASCII is displayed as 'x', non-ASCII as numeric)
        private string getCharacterDisplayString(char character)
        {
            // ASCII?
            if (character < 255)
            {
                // as character
                return String.Format("'{0}'", character);
            }
            else
            {
                // display as number
                int numericValue = (int)character;

                // return string
                return numericValue.ToString();
            }
        }

        // generate source/header strings from a block list
        private void generateStringsFromCharacterDescriptorBlockList(FontInfo fontInfo, ArrayList characterBlockList,
                                                                     ref string resultTextSource, ref string resultTextHeader,
                                                                     ref bool blockLookupGenerated,
                                                                     ref int num_blocks)
        {
            // get wheter there are multiple block lsits
            num_blocks = characterBlockList.Count;

            bool multipleDescBlocksExist = characterBlockList.Count > 1;

            // set whether we'll generate lookups
            blockLookupGenerated = multipleDescBlocksExist;

            //
            // Generate descriptor arrays
            //

            progressBar1.Maximum = characterBlockList.Count * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            // iterate over blocks

            foreach (CharacterDescriptorArrayBlock block in characterBlockList)
            {
                progressBar1.PerformStep();
                Application.DoEvents();

                // according to config
                if (m_outputConfig.commentVariableName)
                {
                    string blockNumberString = String.Format("(block #{0})", characterBlockList.IndexOf(block));

                    // result string
                    resultTextSource += String.Format("{0}Character descriptors for {1} {2}pt{3}{4}" + nl,
                                                        m_commentStartString, fontInfo.font.Name,
                                                        Math.Round(fontInfo.font.Size), multipleDescBlocksExist ? blockNumberString : "",
                                                        m_commentEndString);

                    // describe character array
                    resultTextSource += String.Format("{0}{{ {1}{2}[Offset into {3}CharBitmaps in bytes] }}{4}" + nl,
                                                        m_commentStartString,
                                                        getCharacterDescName("width", m_outputConfig.descCharWidth),
                                                        getCharacterDescName("height", m_outputConfig.descCharHeight),
                                                        getFontName(ref fontInfo.font),
                                                        m_commentEndString);
                }

                // output block header
                resultTextSource += String.Format("{0} = " + nl + "{{" + nl, charDescArrayGetBlockName(fontInfo, characterBlockList.IndexOf(block), true, multipleDescBlocksExist, m_outputConfig.outputProgmemCharInfo));

                // iterate characters
                foreach (CharacterDescriptorArrayBlock.Character character in block.characters)
                {
                    //byte flags = (byte)(IsRightToLeftChar(character.character) ? 1 : 0); // right to left char flag

                    double advanceWidth = character.advanceWidth; //!= -1.0 ? character.advanceWidth : (double)character.width;
                    double advanceHeight = character.advanceHeight; //!= -1.0 ? character.advanceHeight : (double)character.height;

                    // add character
                    resultTextSource += String.Format("\t{{{0}{1}{2}{3}{4}}}, \t\t{5}{6}{7}" + nl,
                                                    getCharacterDescString(m_outputConfig.descCharWidth, character.width),
                                                    getCharacterDescString(m_outputConfig.descCharHeight, character.height),
                                                    character.offset,
                                                    m_outputConfig.outputAdvanceWidth ? ", " + advanceWidth.ToString() : "",
                                                    m_outputConfig.outputAdvanceHeight ? ", " + advanceHeight.ToString() : "",
                                                    m_commentStartString,
                                                    character.character,
                                                    m_commentEndString + " ");
                }

                // terminate current block
                resultTextSource += "};" + nl + nl;
            }

            //
            // Generate block lookup 
            //

            // if there is more than one block, we need to generate a block lookup
            if (multipleDescBlocksExist)
            {
                // start with comment, if required
                if (m_outputConfig.commentVariableName)
                {
                    // result string
                    resultTextSource += String.Format("{0}Block lookup array for {1} {2}pt {3}" + nl,
                                                        m_commentStartString, fontInfo.font.Name,
                                                        Math.Round(fontInfo.font.Size), m_commentEndString);

                    // describe character array
                    resultTextSource += String.Format("{0}{{ start character, end character, ptr to descriptor block array }}{1}" + nl,
                                                        m_commentStartString,
                                                        m_commentEndString);
                }

                // format the block lookup header
                resultTextSource += String.Format("const FONT_CHAR_INFO_LOOKUP {0}[] = " + nl + "{{" + nl,
                                                    getCharacterDescriptorArrayLookupDisplayString(fontInfo));

                // iterate
                foreach (CharacterDescriptorArrayBlock block in characterBlockList)
                {
                    // get first/last chars
                    CharacterDescriptorArrayBlock.Character firstChar = (CharacterDescriptorArrayBlock.Character)block.characters[0],
                                                            lastChar = (CharacterDescriptorArrayBlock.Character)block.characters[block.characters.Count - 1];

                    // create current block description
                    resultTextSource += String.Format("\t{{{0}, {1}, &{2}}}," + nl,
                                                                getCharacterDisplayString(firstChar.character),
                                                                getCharacterDisplayString(lastChar.character),
                                                                charDescArrayGetBlockName(fontInfo, characterBlockList.IndexOf(block), false, true, false));
                }

                // terminate block lookup
                resultTextSource += "};" + nl + nl;
            }
        }

        //
        private string getCharacterDescriptorArrayLookupDisplayString(FontInfo fontInfo)
        {
            // return the string
            return String.Format("{0}BlockLookup", getFontName(ref fontInfo.font));
        }

        // generate lookup array
        private void generateCharacterDescriptorArray(FontInfo fontInfo, ref string resultTextSource,
                                                        ref string resultTextHeader, ref bool blockLookupGenerated, ref int num_blocks)
        {
            // check if required by configuration
            if (m_outputConfig.generateLookupArray)
            {
                ArrayList characterBlockList = new ArrayList();

                // populate list of blocks
                generateCharacterDescriptorBlockList(fontInfo, ref characterBlockList);

                // generate strings from block list
                generateStringsFromCharacterDescriptorBlockList(fontInfo, characterBlockList, ref resultTextSource,
                                                                ref resultTextHeader, ref blockLookupGenerated,
                                                                ref num_blocks);
            }
        }

        private string getAscentDescentAndLineHeightString(bool bOutputString)
        {
            double ascent = 0.0;
            double descent = 0.0;
            double linespacing = 0.0;

            if (GetFontAscentAndDescent(ref ascent, ref descent, ref linespacing))
            {
                Console.WriteLine("Font ascent = " + ascent.ToString() + "px, descent = " + descent.ToString() + "px, linespacing = " + linespacing.ToString() + "px");
            }

            if (!bOutputString)
            {
                return "";
            }
            else
            {
                return String.Format("\t{2}, {0} Font ascent size (px){1}" + nl +
                                     "\t{3}, {0} Font descent size (px){1}" + nl +
                                     "\t{4}, {0} Font line spacing (px){1}" + nl,
                                     m_commentStartString,
                                     m_commentEndString,
                                     ascent,
                                     descent,
                                     linespacing
                                    );
            }
        }

        private string generateFontStructTypedefs()
        {
            string strAscentDescentAndLineHeightString = "";

            if (m_outputConfig.outputAscDescLineheight)
            {
                strAscentDescentAndLineHeightString = "\t\tconst double ascent;\t\t//Font ascent height (px)" + nl +
                                                      "\t\tconst double descent;\t\t//Font descent height (px)" + nl +
                                                      "\t\tconst double lineheight;\t\t//Font lineheight (px)" + nl;
            }

            return String.Format(nl + nl +
                "\ttypedef struct " + nl +
                "\t{{" + nl +
                "\t" + nl +
                "\tconst uint8_t widthBits;\t\t// width, in bits (or pixels), of the character" + nl +
                "\t\tconst uint8_t heightBits;\t\t// height, in bits (or pixels), of the character" + nl +
                "\t\tconst uint32_t offset;\t\t// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array" + nl +
                "\t}}" + nl +
                "\tFONT_CHAR_INFO;" + nl +
                "\t" + nl +
                "\ttypedef struct" + nl +
                "\t{{" + nl +
                "\t\tconst uint32_t startChar;" + nl +
                "\t\tconst uint32_t endChar;" + nl +
                "\t\tFONT_CHAR_INFO* fontcharinfoBlock;" + nl +
                "\t}}" + nl +
                "\tFONT_CHAR_INFO_LOOKUP;" + nl +
                "" + nl +
                "\t// Describes a single font" + nl +
                "\ttypedef struct" + nl +
                "\t{{" + nl +
                "\t\tconst uint8_t heightPages;\t\t// height, in pages (8 pixels), of the font's characters" + nl +
                "\t\tconst uint32_t startChar;\t\t// the first character in the font (e.g. in charInfo and data)" + nl +
                "\t\tconst uint32_t endChar;\t\t// the last character in the font" + nl +
                "\t\tconst uint8_t spacePixels;\t\t// number of pixels that a space character takes up" + nl +
                "\t\tFONT_CHAR_INFO_LOOKUP* fontcharinfoBlockLookup;\t\t// points to array of fontcharinfo lookup entries" + nl +
                "\t\tconst FONT_CHAR_INFO* charInfo;\t\t// pointer to array of char information (NULL if multiple blocks are used in the font, in which case FONT_CHAR_INFO_LOOKUP* will be used)" + nl +
                "\t\tconst uint16_t blockCount;\t\t// number of blocks in font (set to 0 if a single-block font)" + nl +
                "\t\tconst uint8_t* data;\t\t// pointer to generated array of character visual representation" + nl +
                "{0}" + 
                "\t}} FONT_INFO;" + nl + nl, strAscentDescentAndLineHeightString);
        }

        // generate the strings
        private void generateStringsFromFontInfo(FontInfo fontInfo, ref string resultTextSource, ref string resultTextHeader)
        {
            //
            // Character bitmaps
            //

            // according to config
            if (m_outputConfig.commentVariableName)
            {
                // add source header
                resultTextSource += String.Format("{0}Character bitmaps for {1} {2}pt{3}" + nl,
                                                    m_commentStartString, fontInfo.font.Name,
                                                    Math.Round(fontInfo.font.Size), m_commentEndString);
            }

            resultTextHeader += generateFontStructTypedefs();

            // get bitmap name
            string charBitmapVarName = String.Format(m_outputConfig.varNfBitmaps, getFontName(ref fontInfo.font)) + "[]";

            if (m_outputConfig.outputProgmemBitmaps) charBitmapVarName += " PROGMEM";
                    
            // header var
            resultTextHeader += String.Format("extern {0};" + nl, charBitmapVarName);

            // source var
            resultTextSource += String.Format("{0} = " + nl+"{{" + nl, charBitmapVarName);

            progressBar1.Maximum = fontInfo.characters.Length * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            // iterate through letters
            for (int charIdx = 0; charIdx < fontInfo.characters.Length; ++charIdx)
            {
                progressBar1.PerformStep();
                Application.DoEvents();
                // skip empty bitmaps
                if (fontInfo.characters[charIdx].bitmapToGenerate == null) continue;

                // according to config
                if (m_outputConfig.commentCharDescriptor)
                {
                    // output character header
                    resultTextSource += String.Format("\t{0}@{1} '{2}' ({3} pixels wide){4}" + nl,
                                                        m_commentStartString,
                                                        fontInfo.characters[charIdx].offsetInBytes,
                                                        fontInfo.characters[charIdx].character,
                                                        fontInfo.characters[charIdx].width,
                                                        m_commentEndString);
                }

                // now add letter array
                var charInfo = fontInfo.characters[charIdx];
                var bitmap = fontInfo.characters[charIdx].bitmapToGenerate;
                resultTextSource += generateStringFromPageArray(bitmap.Width, bitmap.Height, charInfo.pages);

                // space out
                if (charIdx != fontInfo.characters.Length - 1 && m_outputConfig.commentCharDescriptor)
                {
                    // space between chars
                    resultTextSource += nl;
                }
            }

            // space out
            resultTextSource += "};" + nl + nl;

            //
            // Charater descriptor
            //

            // whether or not block lookup was generated
            bool blockLookupGenerated = false;
            int num_blocks = 0;

            // generate the lookup array
            generateCharacterDescriptorArray(fontInfo, ref resultTextSource, ref resultTextHeader, ref blockLookupGenerated, ref num_blocks);

            //
            // Font descriptor
            //
            
            // according to config
            if (m_outputConfig.commentVariableName)
            {
                // result string
                resultTextSource += String.Format("{0}Font information for {1} {2}pt{3}" + nl,
                                                    m_commentStartString,
                                                    fontInfo.font.Name, Math.Round(fontInfo.font.Size),
                                                    m_commentEndString);
            }

            // character name
            string fontInfoVarName = String.Format(m_outputConfig.varNfFontInfo, getFontName(ref fontInfo.font));

            // add character array for header
            resultTextHeader += String.Format("extern {0};" + nl, fontInfoVarName);

            // the font character height
            string fontCharHeightString = "", spaceCharacterPixelWidthString = "";
            
            // get character height sstring - displayed according to output configuration
            if (m_outputConfig.descFontHeight != OutputConfiguration.DescriptorFormat.DontDisplay)
            {
                // convert the value
                fontCharHeightString = String.Format("\t{0}, {1} Character height{2}" + nl,
                                              convertValueByDescriptorFormat(m_outputConfig.descFontHeight, fontInfo.charHeight),
                                              m_commentStartString,
                                              m_commentEndString);
            }

            // get space char width, if it is up to driver to generate
            if (!m_outputConfig.generateSpaceCharacterBitmap)
            {
                // convert the value
                spaceCharacterPixelWidthString = String.Format("\t{0}, {1} Width, in pixels, of space character{2}" + nl,
                                                                m_outputConfig.spaceGenerationPixels,
                                                                m_commentStartString,
                                                                m_commentEndString);
            }


            // font info
            resultTextSource += String.Format("{2} =" + nl + "{{" + nl +
                                                          "{3}" +                                           //character height px (int)
                                                          "\t{4}, {0} Start character{1}" + nl +            //start character
                                                          "\t{5}, {0} End character{1}" + nl +              //end character
                                                          "{6}" +                                           //width of space char (int, px)
                                                          "{7}" +                                           //character block lookup, character descriptor array
                                                          "\t{8}, {0} Character block count{1}" + nl +
                                                          "\t{9}, {0} Character bitmap array{1}" + nl +
                                                          "{10}" +
                                                          "}};" + nl,
                                                          m_commentStartString,
                                                          m_commentEndString,
                                                          fontInfoVarName,
                                                          fontCharHeightString,
                                                          getCharacterDisplayString(fontInfo.startChar),
                                                          getCharacterDisplayString(fontInfo.endChar),
                                                          spaceCharacterPixelWidthString,
                                                          getFontInfoDescriptorsString(fontInfo, blockLookupGenerated),
                                                          num_blocks,
                                                          getVariableNameFromExpression(String.Format(m_outputConfig.varNfBitmaps, getFontName(ref fontInfo.font))),
                                                          getAscentDescentAndLineHeightString(m_outputConfig.outputAscDescLineheight)
                                              );

            // add the appropriate entity to the header
            if (blockLookupGenerated)
            {
                // add block lookup to header
                resultTextHeader += String.Format("extern const FONT_CHAR_INFO_LOOKUP {0}[];" + nl, getCharacterDescriptorArrayLookupDisplayString(fontInfo));
            }
            else
            {
                // add block lookup to header
                resultTextHeader += String.Format("extern {0}[];" + nl, String.Format(m_outputConfig.varNfCharInfo, getFontName(ref fontInfo.font)));
            }
        }
    
        // get the descriptors
        private string getFontInfoDescriptorsString(FontInfo fontInfo, bool blockLookupGenerated)
        {
            string descriptorString = "";

            // if a lookup arrays are required, point to it
            if (m_outputConfig.generateLookupBlocks)
            {
                // add to string
                descriptorString += String.Format("\t{0}, {1} Character block lookup{2}" + nl,
                                                    blockLookupGenerated ? getCharacterDescriptorArrayLookupDisplayString(fontInfo) : "NULL", 
                                                    m_commentStartString, m_commentEndString);

                // add to string
                descriptorString += String.Format("\t{0}, {1} Character descriptor array{2}" + nl,
                                                    blockLookupGenerated ? "NULL" : getVariableNameFromExpression(String.Format(m_outputConfig.varNfCharInfo, getFontName(ref fontInfo.font))),
                                                    m_commentStartString, m_commentEndString);
            }
            else
            {
                // add descriptor array
                descriptorString += String.Format("\t{0}, {1} Character descriptor array{2}" + nl, 
                                                    getVariableNameFromExpression(String.Format(m_outputConfig.varNfCharInfo, getFontName(ref fontInfo.font))), 
                                                    m_commentStartString, m_commentEndString);
            }

            // return the string
            return descriptorString;
        }
		

        // generate the required output for text
        private void generateOutputForFont(Font font, ref string resultTextSource, ref string resultTextHeader)
        {
            // do nothing if no chars defined
            if (txtInputText.Text.Length == 0) return;
            
            // according to config
            if (m_outputConfig.commentVariableName)
            {
                // add source file header
                resultTextSource += String.Format("{0}" + nl+"{1} Font data for {2} {3}pt" + nl+"{4}" + nl + nl,
                                                    m_commentStartString, m_commentBlockMiddleString, font.Name, Math.Round(font.Size),
                                                    m_commentBlockEndString);

                // add header file header
                resultTextHeader += String.Format("{0}Font data for {1} {2}pt{3}" + nl,
                                                    m_commentStartString, font.Name, Math.Round(font.Size),
                                                    m_commentEndString);
            }

            // populate the font info
            FontInfo fontInfo = populateFontInfo(font);
            
            // We now have all information required per font and per character. 
            // time to generate the string
            generateStringsFromFontInfo(fontInfo, ref resultTextSource, ref resultTextHeader);
        }

        // generate the required output for image
        private void generateOutputForImage(ref Bitmap bitmapOriginal, ref string resultTextSource, ref string resultTextHeader)
        {
            // the name of the bitmap
            string imageName = scrubVariableName(txtImageName.Text);

            // check if bitmap is assigned
            if (m_currentLoadedBitmap != null)
            {
                //
                // Bitmap manipulation
                //

                // get bitmap border
                BitmapBorder bitmapBorder = new BitmapBorder();
                getBitmapBorder(bitmapOriginal, bitmapBorder);

                // manipulate the bitmap
                Bitmap bitmapManipulated;

                // try to manipulate teh bitmap
                if (!manipulateBitmap(bitmapOriginal, bitmapBorder, out bitmapManipulated, 0, 0))
                {
                    // show error
                    MessageBox.Show("No black pixels found in bitmap (currently only monochrome bitmaps supported)",
                                    "Can't convert bitmap",
                                    MessageBoxButtons.OK,
                                    MessageBoxIcon.Error);

                    // stop here, failed to manipulate the bitmap for whatever reason
                    return;
                }

                // for debugging
                // bitmapManipulated.Save(String.Format("C:/bms/manip.bmp"));

                // according to config
                if (m_outputConfig.commentVariableName)
                {
                    // add source file header
                    resultTextSource += String.Format("{0}" + nl+"{1} Image data for {2}" + nl+"{3}" + nl + nl,
                                                        m_commentStartString, m_commentBlockMiddleString, imageName,
                                                        m_commentBlockEndString);

                    // add header file header
                    resultTextHeader += String.Format("{0}Bitmap info for {1}{2}" + nl,
                                                        m_commentStartString, imageName,
                                                        m_commentEndString);
                }

                // bitmap varname
                string dataVarName = String.Format(m_outputConfig.varNfBitmaps, imageName);

                // add to header
                resultTextHeader += String.Format("extern {0};" + nl, dataVarName);

                // add header
                resultTextSource += String.Format("{0} =" + nl+"{{" + nl, dataVarName);

                //
                // Bitmap to string
                //

                // page array
                ArrayList pages;

                // first convert to pages
                convertBitmapToPageArray(bitmapManipulated, out pages);

                // assign pages for fully populated 8 bits
                int pagesPerRow = convertValueByDescriptorFormat(OutputConfiguration.DescriptorFormat.DisplayInBytes, bitmapManipulated.Width);

                // now convert to string
                resultTextSource += generateStringFromPageArray(bitmapManipulated.Width, bitmapManipulated.Height, pages);

                // close
                resultTextSource += String.Format("}};" + nl + nl);

                // according to config
                if (m_outputConfig.commentVariableName)
                {
                    // set sizes comment
                    resultTextSource += String.Format("{0}Bitmap sizes for {1}{2}" + nl,
                                                        m_commentStartString, imageName, m_commentEndString);
                }

                // get var name
                string heightVarName = String.Format(m_outputConfig.varNfHeight, imageName);
                string widthVarName = String.Format(m_outputConfig.varNfWidth, imageName);

                // display width in bytes?
                if (m_outputConfig.descImgWidth == OutputConfiguration.DescriptorFormat.DisplayInBytes)
                {
                    // in pages
                    resultTextSource += String.Format("{0}Pages = {1};" + nl, widthVarName, pagesPerRow);
                    resultTextHeader += String.Format("extern {0}Pages;" + nl, widthVarName);
                }
                else
                {
                    // in pixels
                    resultTextSource += String.Format("{0}Pixels = {1};" + nl, widthVarName, bitmapManipulated.Width);
                    resultTextHeader += String.Format("extern {0}Pixels;" + nl, widthVarName);
                }

                // display height in bytes?
                if (m_outputConfig.descImgHeight == OutputConfiguration.DescriptorFormat.DisplayInBytes)
                {
                    // in pages
                    resultTextSource += String.Format("{0}Pages = {1};" + nl, heightVarName, convertValueByDescriptorFormat(OutputConfiguration.DescriptorFormat.DisplayInBytes, bitmapManipulated.Height));
                    resultTextHeader += String.Format("extern {0}Pages;" + nl, heightVarName);
                }
                else
                {
                    // in pixels
                    resultTextSource += String.Format("{0}Pixels = {1};"+nl, heightVarName, bitmapManipulated.Height);
                    resultTextHeader += String.Format("extern {0}Pixels;"+nl, heightVarName);
                }
            }
        }

        private bool GetCharacterRangeUtf8()
        {
            int startchar = 0;
            int endchar = 0;

            bool parse_result;

            parse_result = Int32.TryParse(txtStartCharUtf8.Text, out startchar);
            parse_result = parse_result && Int32.TryParse(txtEndCharUtf8.Text, out endchar);

            if (!parse_result)
            {
                //MessageBox.Show("Utf8 range", "Invalid Utf8 character range", MessageBoxButtons.OK);
                return false;
            }

            characterrange = "";

            for (int codepoint = startchar; codepoint <= endchar; codepoint++)
            {
                //characterrange += Utf8fromCodepoint(codepoint);
                if (IsCharPresentInFont(codepoint)) characterrange += (char)codepoint;
            }

            CbxCharacterRange.value = characterrange;
            txtInputText.Text = characterrange;
            return true;
        }

        private string Utf8fromCodepoint(int c)
        {
            char char0;
            char char1;
            char char2;
            char char3;

            if (c < 0x80)
            {
                char0 = (char)(c & 0x7f);                   // 1 byte
                char[] u = { char0 };
                return new string(u);
            }

            if (c >= 0x80 && c < 0x800)
            {                   // 2 bytes
                char1 = (char)((c & 0x3f) | 0x80);
                char0 = (char)((c & 0x7c0) >> 6 | 0xc0);
                char[] u = { char0, char1 };
                return new string(u);
            }

            if (c >= 0x800 && c < 0x10000)
            {               // 3 bytes
                char2 = (char)((c & 0x3f) | 0x80);
                char1 = (char)(((c & 0xfc0) >> 6) | 0x80);
                char0 = (char)(((c & 0xf000)) >> 12 | 0xe0);
                char[] u = { char0, char1, char2 };
                return new string(u);
            }

            if (c >= 0x10000 && c < 0x200000)
            {               // 4 bytes
                char3 = (char)((c & 0x3f) | 0x80);
                char2 = (char)(((c & 0xfc0) >> 6) | 0x80);
                char1 = (char)(((c & 0x3f000) >> 12) | 0x80);
                char0 = (char)(((c & 0x1c0000)) >> 18 | 0xF0);
                char[] u = { char0, char1, char2, char3 };
                return new string(u);
            }

            return "";
        }



        private void btnGenerate_Click(object sender, EventArgs e)
        {
            progressBar1.Enabled = true;

            //CbxCharacterRange



            // set focus somewhere else
            label1.Focus();
            
            // save default input text
            Properties.Settings.Default.InputText = txtInputText.Text;
            Properties.Settings.Default.Save();

            // will hold the resutl string            
            string resultStringSource = "";
            string resultStringHeader = "";

            // check which tab is active
            if (tcInput.SelectedTab.Text == "Text")
            {
                // generate output text
                generateOutputForFont(fontDlgInputFont.Font, ref resultStringSource, ref resultStringHeader);
            }
            else
            {
                // generate output bitmap
                generateOutputForImage(ref m_currentLoadedBitmap, ref resultStringSource, ref resultStringHeader);
            }

            // color code the strings and output
            outputSyntaxColoredString(resultStringSource, ref txtOutputTextSource);
            outputSyntaxColoredString(resultStringHeader, ref txtOutputTextHeader);

            progressBar1.Value = 0;
            progressBar1.Enabled = false;
        }

        private void btnBitmapLoad_Click(object sender, EventArgs e)
        {
            // set filter
            dlgOpenFile.Filter = "Image Files (*.jpg; *.jpeg; *.gif; *.bmp)|*.jpg; *.jpeg; *.gif; *.bmp";

            // open the dialog
            if (dlgOpenFile.ShowDialog() != DialogResult.Cancel)
            {
                // load the bitmap
                m_currentLoadedBitmap = new Bitmap(dlgOpenFile.FileName);

                // try to open the bitmap
                pbxBitmap.Image = m_currentLoadedBitmap;

                // set the path
                txtImagePath.Text = dlgOpenFile.FileName;

                // guess a name
                txtImageName.Text = Path.GetFileNameWithoutExtension(dlgOpenFile.FileName);
            }
        }

        // parse the output text line
        void outputSyntaxColoredString(string outputString, ref RichTextBox outputTextBox)
        {
            // clear the current text
            outputTextBox.Text = "";
            
            String [] lines = outputString.Split(new string[] {nl}, StringSplitOptions.None);

            // for now don't syntax color for more than 2000 lines
            if (lines.Length > 1500)
            {
                // just set text
                outputTextBox.Text = outputString;
                return;
            }

            Font fRegular = new Font("Courier New", 10, FontStyle.Regular);
            Font fBold = new Font("Courier New", 10, FontStyle.Bold);
            String[] keywords = { "uint8_t", "const", "extern", "char", "unsigned", "int", "short", "long" };
            Regex re = new Regex(@"([ \t{}();])");

            // iterate over the richtext box and color it
            foreach (string line in lines)
            {
                String[] tokens = re.Split(line);

                // for each found token
                foreach (string token in tokens)
                {
                    // Set the token's default color and font.
                    outputTextBox.SelectionColor = System.Drawing.Color.Black;
                    outputTextBox.SelectionFont = fRegular;

                    // Check for a comment.
                    if (token == "//" || token.StartsWith("//"))
                    {
                        // Find the start of the comment and then extract the whole comment.
                        int index = line.IndexOf("//");
                        string comment = line.Substring(index, line.Length - index);
                        outputTextBox.SelectionColor = System.Drawing.Color.Green;
                        outputTextBox.SelectionFont = fRegular;
                        outputTextBox.SelectedText = comment;
                        break;
                    }

                    // Check for a comment. TODO: terminate coloring
                    if (token == "/*" || token.StartsWith("/*"))
                    {
                        // Find the start of the comment and then extract the whole comment.
                        int index = line.IndexOf("/*");
                        string comment = line.Substring(index, line.Length - index);
                        outputTextBox.SelectionColor = System.Drawing.Color.Green;
                        outputTextBox.SelectionFont = fRegular;
                        outputTextBox.SelectedText = comment;
                        break;
                    }

                    // Check for a comment. TODO: terminate coloring
                    if (token == "**" || token.StartsWith("**"))
                    {
                        // Find the start of the comment and then extract the whole comment.
                        int index = line.IndexOf("**");
                        string comment = line.Substring(index, line.Length - index);
                        outputTextBox.SelectionColor = System.Drawing.Color.Green;
                        outputTextBox.SelectionFont = fRegular;
                        outputTextBox.SelectedText = comment;
                        break;
                    }

                    // Check for a comment. TODO: terminate coloring
                    if (token == "*/" || token.StartsWith("*/"))
                    {
                        // Find the start of the comment and then extract the whole comment.
                        int index = line.IndexOf("*/");
                        string comment = line.Substring(index, line.Length - index);
                        outputTextBox.SelectionColor = System.Drawing.Color.Green;
                        outputTextBox.SelectionFont = fRegular;
                        outputTextBox.SelectedText = comment;
                        break;
                    }

                    // Check whether the token is a keyword. 
                    
                    for (int i = 0; i < keywords.Length; i++)
                    {
                        if (keywords[i] == token)
                        {
                            // Apply alternative color and font to highlight keyword.
                            outputTextBox.SelectionColor = System.Drawing.Color.Blue;
                            outputTextBox.SelectionFont = fBold;
                            break;
                        }
                    }

                    // set the token text
                    outputTextBox.SelectedText = token;
                }
                outputTextBox.SelectedText = nl;
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // close self
            Close();
        }

        private void splitContainer1_MouseUp(object sender, MouseEventArgs e)
        {
            // no focus
            label1.Focus();
        }

        private void btnInsertText_Click(object sender, EventArgs e)
        {
            // no focus
            label1.Focus();

            // insert text
            txtInputText.Text += ((ComboBoxItem)cbxTextInsert.SelectedItem).value;
        }

        private void aboutToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            // about
            AboutForm about = new AboutForm();
            about.FormBorderStyle = FormBorderStyle.FixedToolWindow;

            // show teh about form
            about.Show();
        }
        
        // set comment strings according to config
        private void updateCommentStrings()
        {
            if (m_outputConfig.commentStyle == OutputConfiguration.CommentStyle.Cpp)
            {
                // strings for comments
                m_commentStartString = "// ";
                m_commentBlockEndString = m_commentBlockMiddleString = m_commentStartString;
                m_commentEndString = "";
            }
            else
            {
                // strings for comments
                m_commentStartString = "/* ";
                m_commentBlockMiddleString = "** ";
                m_commentEndString = " */";
                m_commentBlockEndString = "*/";
            }
        }
        
        private void btnOutputConfig_Click(object sender, EventArgs e)
        {
            // no focus
            label1.Focus();

            // get it
            OutputConfigurationForm outputConfigForm = new OutputConfigurationForm(ref m_outputConfigurationManager);
            
            // get the oc
            int selectedConfigurationIndex = outputConfigForm.getOutputConfiguration(cbxOutputConfiguration.SelectedIndex);

            // update the dropdown
            m_outputConfigurationManager.comboboxPopulate(cbxOutputConfiguration);

            // get working configuration
            m_outputConfig = m_outputConfigurationManager.workingOutputConfiguration;

            // set selected index
            cbxOutputConfiguration.SelectedIndex = selectedConfigurationIndex;

            // update comment strings according to conifg
            updateCommentStrings();
        }

        private void button4_Click(object sender, EventArgs e)
        {
        }

        private void cbxOutputConfiguration_SelectedIndexChanged(object sender, EventArgs e)
        {
            // check if any configuration selected
            if (cbxOutputConfiguration.SelectedIndex != -1)
            {
                // get the configuration
                m_outputConfig = m_outputConfigurationManager.configurationGetAtIndex(cbxOutputConfiguration.SelectedIndex);
            }

            // save selected index for next time
            Properties.Settings.Default.OutputConfigIndex = cbxOutputConfiguration.SelectedIndex;

            // save
            Properties.Settings.Default.Save();
        }

        private void button4_Click_1(object sender, EventArgs e)
        {
            
        }

        private void tsmCopySource_Click(object sender, EventArgs e)
        {
            // copy if any text
            if (txtOutputTextSource.Text != "")
            {
                // copy
                Clipboard.SetText(txtOutputTextSource.Text);
            }
        }

        private void tsmCopyHeader_Click(object sender, EventArgs e)
        {
            // copy if any text
            if (txtOutputTextHeader.Text != "")
            {
                // copy
                Clipboard.SetText(txtOutputTextHeader.Text);
            }
        }

        private void ctxMenuHeader_Opening(object sender, CancelEventArgs e)
        {

        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // zero out file name
            dlgSaveAs.FileName = "";

            // try to prompt
            if (dlgSaveAs.ShowDialog() != DialogResult.Cancel)
            {
                // get the file name
                string moduleName = dlgSaveAs.FileName;

                // save the text
                txtOutputTextSource.SaveFile(String.Format("{0}.c", moduleName), RichTextBoxStreamType.PlainText);
                txtOutputTextHeader.SaveFile(String.Format("{0}.h", moduleName), RichTextBoxStreamType.PlainText);
            }
        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void cbxTextInsert_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void txtInputText_TextChanged(object sender, EventArgs e)
        {

        }

        private void txtStartCharUtf8_TextChanged(object sender, EventArgs e)
        {
            //GetCharacterRangeUtf8();
        }

        private void txtEndCharUtf8_TextChanged(object sender, EventArgs e)
        {
            //GetCharacterRangeUtf8();
        }

        private void tableLayoutPanel3_Paint(object sender, PaintEventArgs e)
        {

        }

        private void WriteUInt32LittleEndian(ref FileStream F, UInt32 dword)
        {
            UInt32 mask = 0xff;
            for (int i = 0; i <= 3; i++)
            {
                UInt32 d = (dword & (mask << (i * 8))) >> (i * 8);
                F.WriteByte((byte)d);
            }

            //UInt32 b3 = (dword & 0xff000000) >> 24;
            //UInt32 b2 = (dword & 0xff0000) >> 16;
            //UInt32 b1 = (dword & 0xff00) >> 8;
            //UInt32 b0 = dword & 0xff;
        }

        private void WriteUInt16LittleEndian(ref FileStream F, UInt16 word)
        {
            F.WriteByte((byte)(word & 0xff));
            F.WriteByte((byte)((word &0xff00)>>8));
        }

        // https://stackoverflow.com/questions/4330951/how-to-detect-whether-a-character-belongs-to-a-right-to-left-language
        private bool IsRightToLeftChar(char c)
        {
            int hasRandALCat = 0;
            if (c >= 0x5BE && c <= 0x10B7F)
            {
                if (c <= 0x85E)
                {
                    if (c == 0x5BE) hasRandALCat = 1;
                    else if (c == 0x5C0) hasRandALCat = 1;
                    else if (c == 0x5C3) hasRandALCat = 1;
                    else if (c == 0x5C6) hasRandALCat = 1;
                    else if (0x5D0 <= c && c <= 0x5EA) hasRandALCat = 1;
                    else if (0x5F0 <= c && c <= 0x5F4) hasRandALCat = 1;
                    else if (c == 0x608) hasRandALCat = 1;
                    else if (c == 0x60B) hasRandALCat = 1;
                    else if (c == 0x60D) hasRandALCat = 1;
                    else if (c == 0x61B) hasRandALCat = 1;
                    else if (0x61E <= c && c <= 0x64A) hasRandALCat = 1;
                    else if (0x66D <= c && c <= 0x66F) hasRandALCat = 1;
                    else if (0x671 <= c && c <= 0x6D5) hasRandALCat = 1;
                    else if (0x6E5 <= c && c <= 0x6E6) hasRandALCat = 1;
                    else if (0x6EE <= c && c <= 0x6EF) hasRandALCat = 1;
                    else if (0x6FA <= c && c <= 0x70D) hasRandALCat = 1;
                    else if (c == 0x710) hasRandALCat = 1;
                    else if (0x712 <= c && c <= 0x72F) hasRandALCat = 1;
                    else if (0x74D <= c && c <= 0x7A5) hasRandALCat = 1;
                    else if (c == 0x7B1) hasRandALCat = 1;
                    else if (0x7C0 <= c && c <= 0x7EA) hasRandALCat = 1;
                    else if (0x7F4 <= c && c <= 0x7F5) hasRandALCat = 1;
                    else if (c == 0x7FA) hasRandALCat = 1;
                    else if (0x800 <= c && c <= 0x815) hasRandALCat = 1;
                    else if (c == 0x81A) hasRandALCat = 1;
                    else if (c == 0x824) hasRandALCat = 1;
                    else if (c == 0x828) hasRandALCat = 1;
                    else if (0x830 <= c && c <= 0x83E) hasRandALCat = 1;
                    else if (0x840 <= c && c <= 0x858) hasRandALCat = 1;
                    else if (c == 0x85E) hasRandALCat = 1;
                }
                else if (c == 0x200F) hasRandALCat = 1;
                else if (c >= 0xFB1D)
                {
                    if (c == 0xFB1D) hasRandALCat = 1;
                    else if (0xFB1F <= c && c <= 0xFB28) hasRandALCat = 1;
                    else if (0xFB2A <= c && c <= 0xFB36) hasRandALCat = 1;
                    else if (0xFB38 <= c && c <= 0xFB3C) hasRandALCat = 1;
                    else if (c == 0xFB3E) hasRandALCat = 1;
                    else if (0xFB40 <= c && c <= 0xFB41) hasRandALCat = 1;
                    else if (0xFB43 <= c && c <= 0xFB44) hasRandALCat = 1;
                    else if (0xFB46 <= c && c <= 0xFBC1) hasRandALCat = 1;
                    else if (0xFBD3 <= c && c <= 0xFD3D) hasRandALCat = 1;
                    else if (0xFD50 <= c && c <= 0xFD8F) hasRandALCat = 1;
                    else if (0xFD92 <= c && c <= 0xFDC7) hasRandALCat = 1;
                    else if (0xFDF0 <= c && c <= 0xFDFC) hasRandALCat = 1;
                    else if (0xFE70 <= c && c <= 0xFE74) hasRandALCat = 1;
                    else if (0xFE76 <= c && c <= 0xFEFC) hasRandALCat = 1;
                    else if (0x10800 <= c && c <= 0x10805) hasRandALCat = 1;
                    else if (c == 0x10808) hasRandALCat = 1;
                    else if (0x1080A <= c && c <= 0x10835) hasRandALCat = 1;
                    else if (0x10837 <= c && c <= 0x10838) hasRandALCat = 1;
                    else if (c == 0x1083C) hasRandALCat = 1;
                    else if (0x1083F <= c && c <= 0x10855) hasRandALCat = 1;
                    else if (0x10857 <= c && c <= 0x1085F) hasRandALCat = 1;
                    else if (0x10900 <= c && c <= 0x1091B) hasRandALCat = 1;
                    else if (0x10920 <= c && c <= 0x10939) hasRandALCat = 1;
                    else if (c == 0x1093F) hasRandALCat = 1;
                    else if (c == 0x10A00) hasRandALCat = 1;
                    else if (0x10A10 <= c && c <= 0x10A13) hasRandALCat = 1;
                    else if (0x10A15 <= c && c <= 0x10A17) hasRandALCat = 1;
                    else if (0x10A19 <= c && c <= 0x10A33) hasRandALCat = 1;
                    else if (0x10A40 <= c && c <= 0x10A47) hasRandALCat = 1;
                    else if (0x10A50 <= c && c <= 0x10A58) hasRandALCat = 1;
                    else if (0x10A60 <= c && c <= 0x10A7F) hasRandALCat = 1;
                    else if (0x10B00 <= c && c <= 0x10B35) hasRandALCat = 1;
                    else if (0x10B40 <= c && c <= 0x10B55) hasRandALCat = 1;
                    else if (0x10B58 <= c && c <= 0x10B72) hasRandALCat = 1;
                    else if (0x10B78 <= c && c <= 0x10B7F) hasRandALCat = 1;
                }
            }

            return hasRandALCat == 1 ? true : false;
        }


        private void btnSaveBinFont_Click(object sender, EventArgs e)
        {

            double ascent = 0.0;
            double descent = 0.0;
            double linespacing = 0.0;

            if (GetFontAscentAndDescent(ref ascent, ref descent, ref linespacing))
            {
                Console.WriteLine("Font ascent = " + ascent.ToString() + "px, descent = " + descent.ToString() + "px, linespacing = " + linespacing.ToString() + "px");
            }


            //const uint16_t FONT_HEADER_OFFSET_CHARHEIGHT = 8; // word
            //const uint32_t FONT_HEADER_OFFSET_STARTCHAR = 10; // dword
            //const uint32_t FONT_HEADER_OFFSET_ENDCHAR = 14; // dword
            //const uint16_t FONT_HEADER_OFFSET_NUMLOOKUPENTRIES = 18; // word
            //const uint8_t FONT_HEADER_OFFSET_SPACECHARWIDTH = 20; // byte
            //const double FONT_HEADER_OFFSET_ASCENT = 21;
            //const double FONT_HEADER_OFFSET_DESCENT = 29;
            //const double FONT_HEADER_OFFSET_LINESPACING = 37;
            //const uint8_t FONT_HEADER_OFFSET_DEPTH_BPP = 45; // byte
            const int FONT_HEADER_OFFSET_END = 46;
            const int BLOCKTABLE_OFFSET_BEGIN = FONT_HEADER_OFFSET_END;
            /*
            struct {
                uint16_t charheight;
                uint32_t startchar;
                uint32_t endchar;
                uint16_t numlookupblocks;
                uint8_t spacecharwidth;
                double ascent;
                double descent;
                double linespacing;
                uint8_t antialias_level;
            } _FontHeader;
            */

            // populate the font info
            FontInfo fontInfo = populateFontInfo(fontDlgInputFont.Font);
            ArrayList characterBlockList = new ArrayList();
            // populate list of blocks
            generateCharacterDescriptorBlockList(fontInfo, ref characterBlockList);

            if (characterBlockList.Count == 0)
            {
                MessageBox.Show("Save binary font", "No Characters", MessageBoxButtons.OK);
                return;
            }

            // let user choose filename for font binary
            string fontfilename = "";

            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Title = "Font filename";
            ofd.Filter = "Lectionary fonts|*.lft";
            ofd.CheckFileExists = false;
            //ofd.InitialDirectory = @"C:\";
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                fontfilename = ofd.FileName;
            } else
            {
                return;
            }

            byte aa_level = 0;

            switch (m_outputConfig.antialiasLevel)
            {
                case OutputConfiguration.AntialiasLevel.x4:
                    aa_level = 4;
                    break;

                case OutputConfiguration.AntialiasLevel.x2:
                    aa_level = 2;
                    break;

                case OutputConfiguration.AntialiasLevel.None:
                    aa_level = 1;
                    break;

                default:
                    aa_level = 1;
                    break;
            }

            // write header and offset field which will be filled in later with offset of font header (12 bytes total)
            FileStream F = new FileStream(fontfilename, FileMode.Create, FileAccess.ReadWrite);
            using (BinaryWriter writer = new BinaryWriter(F))
            {
                char[] header = { 'L', 'E', 'C', 'T', 'F', 'O', 'N', 'T' };

                for (int i = 0; i < 8; i++)
                {
                    //F.WriteByte((byte)header[i]);
                    writer.Write((byte)header[i]);
                }

                // write header and fontinfo structure to file
                //--------------------------------------------
                /*
                WriteUInt16LittleEndian(ref F, (UInt16)fontInfo.charHeight);       //16 bits FONT_HEADER_OFFSET_CHARHEIGHT offset bytes = 8-9
                WriteUInt32LittleEndian(ref F, (UInt32)fontInfo.startChar);        //32 bits FONT_HEADER_OFFSET_STARTCHAR                 10-13
                WriteUInt32LittleEndian(ref F, (UInt32)fontInfo.endChar);          //32 bits FONT_HEADER_OFFSET_ENDCHAR                   14-17
                WriteUInt16LittleEndian(ref F, (UInt16)characterBlockList.Count);  //16 bits FONT_HEADER_OFFSET_NUMLOOKUPENTRIES          18-19
                F.WriteByte((byte)m_outputConfig.spaceGenerationPixels);           // 8 bits FONT_HEADER_OFFSET_SPACECHARWIDTH            20
                */
                writer.Write((UInt16)fontInfo.charHeight);                         //16 bits FONT_HEADER_OFFSET_CHARHEIGHT offset bytes = 8-9
                writer.Write((UInt32)fontInfo.startChar);                          //32 bits FONT_HEADER_OFFSET_STARTCHAR                 10-13
                writer.Write((UInt32)fontInfo.endChar);                            //32 bits FONT_HEADER_OFFSET_ENDCHAR                   14-17
                writer.Write((UInt16)characterBlockList.Count);                    //16 bits FONT_HEADER_OFFSET_NUMLOOKUPENTRIES          18-19
                writer.Write((byte)m_outputConfig.spaceGenerationPixels);          // 8 bits FONT_HEADER_OFFSET_SPACECHARWIDTH            20
                writer.Write((double)ascent);                                      //64 bits FONT_HEADER_OFFSET_ASCENT                    21-28
                writer.Write((double)descent);                                     //64 bits FONT_HEADER_OFFSET_DESCENT                   29-36
                writer.Write((double)linespacing);                                 //64 bits FONT_HEADER_OFFSET_LINESPACING               37-44
                writer.Write((byte)aa_level);                                      // 8 bits FONT_HEADER_OFFSET_DEPTH_BPP                 45
                // Header blocksize is 46 bytes

                // after the header, 1 or more of 3xUint32 blocks, containing startchar, endchar and file offset to start of bitmap for the block.
                // write out font bitmap block lookup tables - 1 or more of 3xUInt32s, [startchar], [endchar] and [file offset to bitmap].
                //--------------------------------------------
                progressBar1.Maximum = fontInfo.characters.Length * progressBar1.Width;
                progressBar1.Minimum = 0;
                progressBar1.Value = 0;
                progressBar1.Step = progressBar1.Width;
                //progressBar1.PerformStep();

                for (int i = 0; i < characterBlockList.Count; i++)
                {
                    // sparse font, consists of 2 or more font bitmap lookup tables
                    // write out a blocktable entry for each blocktable.
                    UInt32 startchar = 0; // for clarity
                    UInt32 endchar = 0; // blocktable entry is startchar, endchar, offsetstartchar (all UInt32, big endian)
                    UInt32 bitmapoffsetstartchar = 0; // these values will be filled in after the block bitmap lookup table has been written out.
                    /*
                    WriteUInt32LittleEndian(ref F, startchar);
                    WriteUInt32LittleEndian(ref F, endchar);
                    WriteUInt32LittleEndian(ref F, bitmapoffsetstartchar);
                    */
                    writer.Write(startchar);
                    writer.Write(endchar);
                    writer.Write(bitmapoffsetstartchar);
                }

                //long fontbitmapoffset = F.Position; //offset in bytes of the start of the character bitmaps in the file
                long fontbitmapoffset = writer.Seek(0, SeekOrigin.Current); // get the current position in the stream

                // write out bitmap of entire font, iterate through letters
                // --------------------------------------------------------
                progressBar1.Maximum = fontInfo.characters.Length * progressBar1.Width;
                progressBar1.Minimum = 0;
                progressBar1.Value = 0;
                progressBar1.Step = progressBar1.Width;
                //progressBar1.PerformStep();

                Console.WriteLine("Writing out font bitmap...\n");

                for (int charIdx = 0; charIdx < fontInfo.characters.Length; ++charIdx)
                {
                    var charInfo = fontInfo.characters[charIdx];
                    Console.WriteLine("char='" + charInfo.character + "'\tcharindex = " + charIdx.ToString("X4") + "\n");

                    progressBar1.PerformStep();
                    Application.DoEvents();
                    // skip empty bitmaps
                    if (charInfo.bitmapToGenerate == null) continue;

                    //write out bitmap
                    for (int i = 0; i < charInfo.pages.Count; i++)
                    {
                        byte b = (byte)charInfo.pages[i];
                        //F.WriteByte(b);
                        writer.Write(b);
                        //Console.Write(b.ToString("X2") + " ");
                    }
                    //Console.WriteLine("\n\n");
                }

                // write out the lookup table for each block
                //--------------------------------------------
                Console.WriteLine("Writing out block lookup tables...\n");

                int blockIndex = 0;
                foreach (CharacterDescriptorArrayBlock block in characterBlockList)
                {
                    Console.WriteLine("Block " + blockIndex.ToString() + "\n");

                    progressBar1.PerformStep();
                    Application.DoEvents();

                    //UInt32 fontlookupoffset = (UInt32)F.Position; // save current file position, which will be the offset of the start of the font
                                                                  // lookup table (which will be stored in the font header, appended to the file)
                    UInt32 fontlookupoffset = (UInt32)writer.Seek(0, SeekOrigin.Current);

                    int charIndex = 0;

                    UInt32 startchar = block.startchar;
                    UInt32 endchar = block.endchar;

                    foreach (CharacterDescriptorArrayBlock.Character character in block.characters)
                    {
                        if (charIndex == 0)
                        {
                            startchar = (UInt32)character.character;
                        }

                        if (charIndex == block.characters.Count - 1)
                        {
                            endchar = (UInt32)character.character;
                        }

                        UInt16 widthBits = (UInt16)character.width;
                        UInt16 heightBits = (UInt16)character.height;
                        UInt32 offset = 0;

                        double advanceWidth = character.advanceWidth != -1.0 ? character.advanceWidth : (double)character.width;
                        double advanceHeight = character.advanceHeight != -1.0 ? character.advanceHeight : (double)character.height;

                        if (widthBits != 0 && heightBits != 0)
                        {
                            offset = (UInt32)(character.offset + fontbitmapoffset);
                        }

                        //deprecated vv
                        //byte flags = (byte)(IsRightToLeftChar(character.character) ? 1 : 0); // right to left char flag
                        //F.WriteByte(flags);
                        //^^

                        /*
                        WriteUInt16LittleEndian(ref F, widthBits);
                        WriteUInt16LittleEndian(ref F, heightBits);
                        WriteUInt32LittleEndian(ref F, offset);

                        if (m_outputConfig.outputAdvanceWidth) WriteDouble(ref F, advanceWidth);
                        if (m_outputConfig.outputAdvanceHeight) WriteDouble(ref F, advanceHeight);
                        */

                        writer.Write(widthBits);
                        writer.Write(heightBits);
                        writer.Write(offset);

                        if (m_outputConfig.outputAdvanceWidth) writer.Write(advanceWidth);
                        if (m_outputConfig.outputAdvanceHeight) writer.Write(advanceHeight);

                        Console.WriteLine("char='" + character.character + "'\tcharindex=" + charIndex.ToString("X4") + "\tw:" + widthBits.ToString("X4") + "\th:" + heightBits.ToString("X4") + "\toffset:" + offset.ToString("X8") + "\n");

                        charIndex++;
                    }

                    int blocktablepos = BLOCKTABLE_OFFSET_BEGIN + (blockIndex * 4 * 3);

                    //F.Seek(blocktablepos, SeekOrigin.Begin);
                    writer.Seek(blocktablepos, SeekOrigin.Begin);
                    /*
                    WriteUInt32LittleEndian(ref F, startchar);
                    WriteUInt32LittleEndian(ref F, endchar);
                    WriteUInt32LittleEndian(ref F, fontlookupoffset);
                    */

                    writer.Write(startchar);
                    writer.Write(endchar);
                    writer.Write(fontlookupoffset);

                    //F.Seek(0, SeekOrigin.End);
                    writer.Seek(0, SeekOrigin.End);

                    blockIndex++;
                }
                writer.Flush();
            }
            //F.Flush();
            F.Close();

            MessageBox.Show("Lectionary Font binary saved", "Success", MessageBoxButtons.OK);

            progressBar1.Value = 0;
            progressBar1.Enabled = false;

            /*
                        for (int charIdx = 0; charIdx < fontInfo.characters.Length; ++charIdx)
                        {
                            var charInfo = fontInfo.characters[charIdx];

                            progressBar1.PerformStep();
                            Application.DoEvents();
                            // skip empty bitmaps
                            //if (charInfo.bitmapToGenerate == null) continue;

                            //write out lookup. Lookup is 2+2+4 = 8 bytes per character
                            UInt16 widthBits = (UInt16)charInfo.width;
                            UInt16 heightBits = (UInt16)charInfo.height;
                            UInt32 offset = (UInt32)charInfo.offsetInBytes + fontbitmapoffset;

                            WriteUInt16LittleEndian(ref F, widthBits);
                            WriteUInt16LittleEndian(ref F, heightBits);
                            WriteUInt32LittleEndian(ref F, offset);
                        }

                        F.Flush();

                        F.Seek(FONT_HEADER_OFFSET_LOOKUP, SeekOrigin.Begin);

                        WriteUInt32LittleEndian(ref F, fontlookupoffset);

                        F.Flush();
                        F.Close();

                        MessageBox.Show("Lectionary Font binary saved", "Success", MessageBoxButtons.OK);

                        progressBar1.Value = 0;
                        progressBar1.Enabled = false;
            */
        }

        private void button4_Click_2(object sender, EventArgs e)
        {

        }

        private void btnInsertUnicodeRange_Click(object sender, EventArgs e)
        {
            GetCharacterRangeUtf8();
        }

        /*
        // generate source/header strings from a block list
        private void generateBinFromCharacterDescriptorBlockList(FontInfo fontInfo, ArrayList characterBlockList,
                                                                 ref bool blockLookupGenerated)
        {
            // get wheter there are multiple block lsits
            bool multipleDescBlocksExist = characterBlockList.Count > 1;

            // set whether we'll generate lookups
            blockLookupGenerated = multipleDescBlocksExist;

            //
            // Generate descriptor arrays
            //

            progressBar1.Maximum = characterBlockList.Count * progressBar1.Width;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;
            progressBar1.Step = progressBar1.Width;
            //progressBar1.PerformStep();

            // iterate over blocks

            foreach (CharacterDescriptorArrayBlock block in characterBlockList)
            {
                progressBar1.PerformStep();
                Application.DoEvents();

                // according to config
                if (m_outputConfig.commentVariableName)
                {
                    string blockNumberString = String.Format("(block #{0})", characterBlockList.IndexOf(block));

                    // result string
                    resultTextSource += String.Format("{0}Character descriptors for {1} {2}pt{3}{4}" + nl,
                                                        m_commentStartString, fontInfo.font.Name,
                                                        Math.Round(fontInfo.font.Size), multipleDescBlocksExist ? blockNumberString : "",
                                                        m_commentEndString);

                    // describe character array
                    resultTextSource += String.Format("{0}{{ {1}{2}[Offset into {3}CharBitmaps in bytes] }}{4}" + nl,
                                                        m_commentStartString,
                                                        getCharacterDescName("width", m_outputConfig.descCharWidth),
                                                        getCharacterDescName("height", m_outputConfig.descCharHeight),
                                                        getFontName(ref fontInfo.font),
                                                        m_commentEndString);
                }

                // output block header
                resultTextSource += String.Format("{0} = " + nl + "{{" + nl, charDescArrayGetBlockName(fontInfo, characterBlockList.IndexOf(block), true, multipleDescBlocksExist));

                // iterate characters
                foreach (CharacterDescriptorArrayBlock.Character character in block.characters)
                {
                    // add character
                    resultTextSource += String.Format("\t{{{0}{1}{2}}}, \t\t{3}{4}{5}" + nl,
                                                    getCharacterDescString(m_outputConfig.descCharWidth, character.width),
                                                    getCharacterDescString(m_outputConfig.descCharHeight, character.height),
                                                    character.offset,
                                                    m_commentStartString,
                                                    character.character,
                                                    m_commentEndString + " ");
                }

                // terminate current block
                resultTextSource += "};" + nl + nl;
            }

            //
            // Generate block lookup 
            //

            // if there is more than one block, we need to generate a block lookup
            if (multipleDescBlocksExist)
            {
                // start with comment, if required
                if (m_outputConfig.commentVariableName)
                {
                    // result string
                    resultTextSource += String.Format("{0}Block lookup array for {1} {2}pt {3}" + nl,
                                                        m_commentStartString, fontInfo.font.Name,
                                                        Math.Round(fontInfo.font.Size), m_commentEndString);

                    // describe character array
                    resultTextSource += String.Format("{0}{{ start character, end character, ptr to descriptor block array }}{1}" + nl,
                                                        m_commentStartString,
                                                        m_commentEndString);
                }

                // format the block lookup header
                resultTextSource += String.Format("const FONT_CHAR_INFO_LOOKUP {0}[] = " + nl + "{{" + nl,
                                                    getCharacterDescriptorArrayLookupDisplayString(fontInfo));

                // iterate
                foreach (CharacterDescriptorArrayBlock block in characterBlockList)
                {
                    // get first/last chars
                    CharacterDescriptorArrayBlock.Character firstChar = (CharacterDescriptorArrayBlock.Character)block.characters[0],
                                                            lastChar = (CharacterDescriptorArrayBlock.Character)block.characters[block.characters.Count - 1];

                    // create current block description
                    resultTextSource += String.Format("\t{{{0}, {1}, &{2}}}," + nl,
                                                                getCharacterDisplayString(firstChar.character),
                                                                getCharacterDisplayString(lastChar.character),
                                                                charDescArrayGetBlockName(fontInfo, characterBlockList.IndexOf(block), false, true));
                }

                // terminate block lookup
                resultTextSource += "};" + nl + nl;
            }
        }*/
    }
}
