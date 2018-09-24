using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualBasic;
using System.IO;

namespace build_lectionary
{
    class Program
    {
        static int Main(string[] args)
        {
            // Test if input arguments were supplied:
            
            if (args.Length != 1)
            {
                System.Console.WriteLine("Please give filename and path to the lectionary CSV file.");
                System.Console.WriteLine("Usage: {0} <filename>", Environment.CommandLine);
                return 1;
            }
            

            // Try to spec the file. This will throw
            // an exception if the file does not exist.
            //            if(!(File.Exists(args[0])))
            //            {
            //                System.Console.WriteLine("Bible csv file not found.");
            //                System.Console.WriteLine("Usage: {0} <filename>", Environment.CommandLine);
            //                return 1;
            //            }

            string fileName = args[0];

            //string path = @".\Bible";

            //string fileName = "Sun_Lectionary_Raw.csv";

            IDictionary<int, LectionaryEntry> lectionary = new Dictionary<int, LectionaryEntry>();
            LectionaryEntry l;

            try
            {
                using (var sr = new StreamReader(fileName)) 
                {
                    string line;
                    //long pos = GetActualPosition(sr);
                    while ((line = sr.ReadLine()) != null)
                    {
                        //Console.WriteLine("-> {0} ", pos);

                        using (var parser = new Microsoft.VisualBasic.FileIO.TextFieldParser(new System.IO.StringReader(line)))
                        {
                            parser.TextFieldType = Microsoft.VisualBasic.FileIO.FieldType.Delimited;
                            parser.SetDelimiters(",");
                            while (!parser.EndOfData)
                            {
                                //Processing row
                                string[] fields = parser.ReadFields();
                                string readingType = fields[0]; 
                                string readings = fields[1];
                                int lectionary_number = int.Parse(fields[4]);
                                string years = fields[5];

                                try
                                {
                                    l = lectionary[lectionary_number];
                                }

                                catch(Exception e)
                                {
                                    lectionary[lectionary_number] = new LectionaryEntry();
                                    l = lectionary[lectionary_number];
                                }

                                if (years.Contains("A"))
                                {
                                    switch (readingType)
                                    {
                                        case "OT":
                                            l.A.OT += (readings + ";");
                                            break;

                                        case "NT":
                                            l.A.NT += (readings + ";");
                                            break;

                                        case "PS":
                                            l.A.PS += (readings + ";");
                                            break;

                                        case "G":
                                            l.A.G += (readings + ";");
                                            break;
                                    }
                                }

                                if (years.Contains("B"))
                                {
                                    switch (readingType)
                                    {
                                        case "OT":
                                            l.B.OT += (readings + ";");
                                            break;

                                        case "NT":
                                            l.B.NT += (readings + ";");
                                            break;

                                        case "PS":
                                            l.B.PS += (readings + ";");
                                            break;

                                        case "G":
                                            l.B.G += (readings + ";");
                                            break;
                                    }
                                }

                                if (years.Contains("C"))
                                {
                                    switch (readingType)
                                    {
                                        case "OT":
                                            l.C.OT += (readings + ";");
                                            break;

                                        case "NT":
                                            l.C.NT += (readings + ";");
                                            break;

                                        case "PS":
                                            l.C.PS += (readings + ";");
                                            break;

                                        case "G":
                                            l.C.G += (readings + ";");
                                            break;
                                    }
                                }

                                if (years.Contains("1"))
                                {
                                    switch (readingType)
                                    {
                                        case "OT":
                                            l.Y1.OT += (readings + ";");
                                            break;

                                        case "NT":
                                            l.Y1.NT += (readings + ";");
                                            break;

                                        case "PS":
                                            l.Y1.PS += (readings + ";");
                                            break;

                                        case "G":
                                            l.Y1.G += (readings + ";");
                                            break;
                                    }
                                }

                                if (years.Contains("2"))
                                {
                                    switch (readingType)
                                    {
                                        case "OT":
                                            l.Y2.OT += (readings + ";");
                                            break;

                                        case "NT":
                                            l.Y2.NT += (readings + ";");
                                            break;

                                        case "PS":
                                            l.Y2.PS += (readings + ";");
                                            break;

                                        case "G":
                                            l.Y2.G += (readings + ";");
                                            break;
                                    }
                                }

                                l.LectionaryNumber = lectionary_number;
/*
                                string dir = path + "\\" + book + "\\" + chapter;

                                if (!(Directory.Exists(dir)))
                                {
                                    DirectoryInfo di = Directory.CreateDirectory(dir);
                                }

                                string verseIndexFile = dir + "\\" + verse;

                                System.IO.File.WriteAllText(verseIndexFile, pos.ToString()); // pos is an index into the bible csv file, of that verse's 
                                                                                             // csv record (id, bookname, booknumber, chapter, verse, text)

                                //TODO: Process field
                                //Console.WriteLine(field);

                                printLineAtPos(pos, fileName);
*/
                            }
                        }
                        //pos = GetActualPosition(sr);
                    }
                }
            }

            catch (Exception e)
            {
                //Directory.Delete(path, true);

                Console.WriteLine("Failed to process file: {0}", e.Message);
                return 1;
            }
            //string outFileName = "Lectionary.csv";
            //using (StreamWriter swOutputFile = new StreamWriter(new FileStream(outFileName, FileMode.Create, FileAccess.Write, FileShare.Read)))
            //{}
            int i = 0;
            int index = 1;

            while (i < lectionary.Count)
            {
                try
                {
                    l = lectionary[index];
                    Console.WriteLine("Lectionary #" + l.LectionaryNumber);

                    if (l.A.OT != null || l.A.NT != null || l.A.PS != null || l.A.G != null)
                    {
                        WriteLectionaryEntry(l.LectionaryNumber, l.A.OT, LectionaryEntryType.OT, LectionaryEntryYear.YEAR_A);
                        WriteLectionaryEntry(l.LectionaryNumber, l.A.NT, LectionaryEntryType.NT, LectionaryEntryYear.YEAR_A);
                        WriteLectionaryEntry(l.LectionaryNumber, l.A.PS, LectionaryEntryType.PS, LectionaryEntryYear.YEAR_A);
                        WriteLectionaryEntry(l.LectionaryNumber, l.A.G, LectionaryEntryType.G, LectionaryEntryYear.YEAR_A);

                        Console.Write("\tA.OT\t" + l.A.OT);
                        Console.Write("\tA.NT\t" + l.A.NT);
                        Console.Write("\tA.PS\t" + l.A.PS);
                        Console.Write("\tA.G \t" + l.A.G);
                        Console.WriteLine();
                    }

                    if (l.B.OT != null || l.B.NT != null || l.B.PS != null || l.B.G != null)
                    {
                        WriteLectionaryEntry(l.LectionaryNumber, l.B.OT, LectionaryEntryType.OT, LectionaryEntryYear.YEAR_B);
                        WriteLectionaryEntry(l.LectionaryNumber, l.B.NT, LectionaryEntryType.NT, LectionaryEntryYear.YEAR_B);
                        WriteLectionaryEntry(l.LectionaryNumber, l.B.PS, LectionaryEntryType.PS, LectionaryEntryYear.YEAR_B);
                        WriteLectionaryEntry(l.LectionaryNumber, l.B.G, LectionaryEntryType.G, LectionaryEntryYear.YEAR_B);

                        Console.Write("\tB.OT\t" + l.B.OT);
                        Console.Write("\tB.NT\t" + l.B.NT);
                        Console.Write("\tB.PS\t" + l.B.PS);
                        Console.Write("\tB.G \t" + l.B.G);
                        Console.WriteLine();
                    }
                    if (l.C.OT != null || l.C.NT != null || l.C.PS != null || l.C.G != null)
                    {
                        WriteLectionaryEntry(l.LectionaryNumber, l.C.OT, LectionaryEntryType.OT, LectionaryEntryYear.YEAR_C);
                        WriteLectionaryEntry(l.LectionaryNumber, l.C.NT, LectionaryEntryType.NT, LectionaryEntryYear.YEAR_C);
                        WriteLectionaryEntry(l.LectionaryNumber, l.C.PS, LectionaryEntryType.PS, LectionaryEntryYear.YEAR_C);
                        WriteLectionaryEntry(l.LectionaryNumber, l.C.G, LectionaryEntryType.G, LectionaryEntryYear.YEAR_C);

                        Console.Write("\tC.OT\t" + l.C.OT);
                        Console.Write("\tC.NT\t" + l.C.NT);
                        Console.Write("\tC.PS\t" + l.C.PS);
                        Console.Write("\tC.G \t" + l.C.G);
                        Console.WriteLine();
                    }

                    if (l.Y1.OT != null || l.Y1.NT != null || l.Y1.PS != null || l.Y1.G != null)
                    {
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y1.OT, LectionaryEntryType.OT, LectionaryEntryYear.YEAR_1);
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y1.NT, LectionaryEntryType.NT, LectionaryEntryYear.YEAR_1);
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y1.PS, LectionaryEntryType.PS, LectionaryEntryYear.YEAR_1);
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y1.G, LectionaryEntryType.G, LectionaryEntryYear.YEAR_1);

                        Console.Write("\tY1.OT\t" + l.Y1.OT);
                        Console.Write("\tY1.NT\t" + l.Y1.NT);
                        Console.Write("\tY1.PS\t" + l.Y1.PS);
                        Console.Write("\tY1.G \t" + l.Y1.G);
                        Console.WriteLine();
                    }
                    if (l.Y2.OT != null || l.Y2.NT != null || l.Y2.PS != null || l.Y2.G != null)
                    {
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y2.OT, LectionaryEntryType.OT, LectionaryEntryYear.YEAR_2);
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y2.NT, LectionaryEntryType.NT, LectionaryEntryYear.YEAR_2);
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y2.PS, LectionaryEntryType.PS, LectionaryEntryYear.YEAR_2);
                        WriteLectionaryEntry(l.LectionaryNumber, l.Y2.G, LectionaryEntryType.G, LectionaryEntryYear.YEAR_2);

                        Console.Write("\tY2.OT\t" + l.Y2.OT);
                        Console.Write("\tY2.NT\t" + l.Y2.NT);
                        Console.Write("\tY2.PS\t" + l.Y2.PS);
                        Console.Write("\tY2.G \t" + l.Y2.G);
                        Console.WriteLine();
                    }

                    Console.WriteLine("--------------------------------------------------------------------------------");
                    index++;
                    i++;
                }

                catch (Exception e)
                {
                    index++;
                }
            }

            Console.ReadKey();
            return 0;
        }

        private enum LectionaryEntryType
        {
            OT = 0,
            NT = 1,
            PS = 2,
            G = 3
        }

        private enum LectionaryEntryYear
        {
            YEAR_A = 0,
            YEAR_B = 1,
            YEAR_C = 2,
            YEAR_1 = 3,
            YEAR_2 = 4
        }


        private static bool WriteLectionaryEntry(int lectionaryNumber, String text, LectionaryEntryType t, LectionaryEntryYear year)
        {
            string[] lectdirs = { "OT", "NT", "PS", "G" };
            string[] lectyears = { "A", "B", "C", "1", "2" };

            string filename;
            string filedir;

            try
            {
                filedir = ".\\Lect\\" + lectionaryNumber.ToString() + "\\" + lectdirs[(int)t];
                filename = filedir + "\\" + lectyears[(int)year];
                if (text != null)
                {
                    if (!(Directory.Exists(filedir)))
                    {
                        DirectoryInfo di = Directory.CreateDirectory(filedir);
                    }
                    System.IO.File.WriteAllText(filename, RemoveLast(text, ";"));
                }
                return true;
            }

            catch(Exception e)
            {
                return false;
            }
        }

        private static string RemoveLast(string text, string character)
        {
            if (text.Length < 1) return text;
            return text.Remove(text.ToString().LastIndexOf(character), character.Length);
        }

        private static void PrintLineAtPos(long pos, string fileName)
        {
            using (Stream stream = File.Open(fileName, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                stream.Seek(pos, SeekOrigin.Begin);
                using (StreamReader reader = new StreamReader(stream))
                {
                    string line = reader.ReadLine() + "\n";
                    Console.Write(line);
                }
            }
        }


        // Get the actual file position from a buffered StreamReader object. From: https://stackoverflow.com/a/17457085
        public static long GetActualPosition(System.IO.StreamReader reader)
        {
            System.Reflection.BindingFlags flags = System.Reflection.BindingFlags.DeclaredOnly | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.GetField;

            // The current buffer of decoded characters
            char[] charBuffer = (char[])reader.GetType().InvokeMember("charBuffer", flags, null, reader, null);

            // The index of the next char to be read from charBuffer
            int charPos = (int)reader.GetType().InvokeMember("charPos", flags, null, reader, null);

            // The number of decoded chars presently used in charBuffer
            int charLen = (int)reader.GetType().InvokeMember("charLen", flags, null, reader, null);

            // The current buffer of read bytes (byteBuffer.Length = 1024; this is critical).
            byte[] byteBuffer = (byte[])reader.GetType().InvokeMember("byteBuffer", flags, null, reader, null);

            // The number of bytes read while advancing reader.BaseStream.Position to (re)fill charBuffer
            int byteLen = (int)reader.GetType().InvokeMember("byteLen", flags, null, reader, null);

            // The number of bytes the remaining chars use in the original encoding.
            int numBytesLeft = reader.CurrentEncoding.GetByteCount(charBuffer, charPos, charLen - charPos);

            // For variable-byte encodings, deal with partial chars at the end of the buffer
            int numFragments = 0;
            if (byteLen > 0 && !reader.CurrentEncoding.IsSingleByte)
            {
                if (reader.CurrentEncoding.CodePage == 65001) // UTF-8
                {
                    byte byteCountMask = 0;
                    while ((byteBuffer[byteLen - numFragments - 1] >> 6) == 2) // if the byte is "10xx xxxx", it's a continuation-byte
                        byteCountMask |= (byte)(1 << ++numFragments); // count bytes & build the "complete char" mask
                    if ((byteBuffer[byteLen - numFragments - 1] >> 6) == 3) // if the byte is "11xx xxxx", it starts a multi-byte char.
                        byteCountMask |= (byte)(1 << ++numFragments); // count bytes & build the "complete char" mask
                                                                      // see if we found as many bytes as the leading-byte says to expect
                    if (numFragments > 1 && ((byteBuffer[byteLen - numFragments] >> 7 - numFragments) == byteCountMask))
                        numFragments = 0; // no partial-char in the byte-buffer to account for
                }
                else if (reader.CurrentEncoding.CodePage == 1200) // UTF-16LE
                {
                    if (byteBuffer[byteLen - 1] >= 0xd8) // high-surrogate
                        numFragments = 2; // account for the partial character
                }
                else if (reader.CurrentEncoding.CodePage == 1201) // UTF-16BE
                {
                    if (byteBuffer[byteLen - 2] >= 0xd8) // high-surrogate
                        numFragments = 2; // account for the partial character
                }
            }
            return reader.BaseStream.Position - numBytesLeft - numFragments;
        }
    }

    class LectionaryReadingEntry
    {
        public string OT;
        public string NT;
        public string PS;
        public string G;
    }

    class LectionaryEntry
    {
        public int LectionaryNumber;
        public LectionaryReadingEntry A;
        public LectionaryReadingEntry B;
        public LectionaryReadingEntry C;
        public LectionaryReadingEntry Y1;
        public LectionaryReadingEntry Y2;

        public LectionaryEntry()
        {
            this.A = new LectionaryReadingEntry();
            this.B = new LectionaryReadingEntry();
            this.C = new LectionaryReadingEntry();
            this.Y1 = new LectionaryReadingEntry();
            this.Y2 = new LectionaryReadingEntry();
        }

    }
}
