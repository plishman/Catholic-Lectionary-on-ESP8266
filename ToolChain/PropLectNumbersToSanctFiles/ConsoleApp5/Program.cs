using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace ConsoleApp5
{
    class Program
    {
        static int Main(string[] args)
        {
            Console.OutputEncoding = System.Text.Encoding.UTF8;
            if (args.Length != 2)
            {
                System.Console.WriteLine("Please give filename of Sanctorale file with Lectionary numbers filled in, and filename of Sanctorale file to have Lectionary numbers filled in.");
                System.Console.WriteLine("Usage: {0} <source filename> <dest filename>", Environment.CommandLine);
                return 1;
            }

            string srcFileName = args[0];
            string dstFileName = args[1];

            using (var sr = new StreamReader(dstFileName, System.Text.Encoding.UTF8, true)) //@"C:\Users\phill\Documents\Visual Studio 2017\Projects\bibleindex\bibleindex\new jerusalem catholic bible.csv"
            {
                string line;
                bool bInComment = false;
                string month = "";

                while ((line = sr.ReadLine()) != null)
                {
                    if (line.IndexOf("---") == 0 && !bInComment)
                    {
                        bInComment = true;
                        Console.Write(line + "\n");
                        continue;
                    }

                    if (bInComment)
                    {
                        Console.Write(line + "\n");
                        if (line.IndexOf("---") == 0)
                        {
                            bInComment = false;
                        }
                        continue;
                    }

                    if (line == "")
                    {
                        Console.Write("\n");
                        continue;
                    }

                    if (line.StartsWith("= "))
                    {
                        month = line;
                        Console.Write(line + "\n");
                        continue;
                    }

                    if (line.IndexOf(':') != -1)
                    {
                        string leftpart = line.Substring(0, line.IndexOf(':'));
                        string rightpart = line.Substring(line.IndexOf(':'));

                        //string LectNumber = leftpart.Substring(leftpart.IndexOf("L"), leftpart.IndexOf(' ', leftpart.IndexOf("L")));
                        string day = leftpart.Substring(0, leftpart.IndexOf(' '));

                        string LectNumber = getLectNumber(day, month, srcFileName);

                        if (LectNumber == "")
                        {
                            Console.Write(line + "\n");
                            continue;
                        } else
                        {
                            if (leftpart.IndexOf("L") != -1)
                            {
                                int start = leftpart.IndexOf("L");

                                int end = leftpart.IndexOf(' ', start);
                                if (end == -1)
                                {
                                    end = leftpart.Length;
                                }

                                int length = end - start;

                                string ExistingLectNumber = " " + leftpart.Substring(start, length);

                                leftpart = leftpart.Replace(ExistingLectNumber, "");
                            }

                            Console.Write(leftpart + LectNumber + " " + rightpart + "\n");
                            continue;
                        }
                    }

                }

            }
            return 0;
        }

        private static string getLectNumber(string day, string month, string srcFileName)
        {
            using (var sr = new StreamReader(srcFileName, System.Text.Encoding.UTF8, true)) //@"C:\Users\phill\Documents\Visual Studio 2017\Projects\bibleindex\bibleindex\new jerusalem catholic bible.csv"
            {
                string line;
                bool bInComment = false;
                string currmonth = "";

                while ((line = sr.ReadLine()) != null)
                {
                    if (line.IndexOf("---") == 0 && !bInComment)
                    {
                        bInComment = true;
                        continue;
                    }

                    if (bInComment)
                    {
                        //Console.Write(line);
                        if (line.IndexOf("---") == 0)
                        {
                            bInComment = false;
                        }
                        continue;
                    }

                    if (line == "")
                    {
                        //Console.Write(line);
                        continue;
                    }

                    if (line.StartsWith("= "))
                    {
                        currmonth = line;
                        //Console.Write(line);
                        continue;
                    }

                    if (line.IndexOf(':') != -1)
                    {
                        string leftpart = line.Substring(0, line.IndexOf(':'));
                        string rightpart = line.Substring(line.IndexOf(':'));

                        string LectNumber = "";

                        if (leftpart.IndexOf("L") != -1)
                        {
                            int start = leftpart.IndexOf("L");

                            int end = leftpart.IndexOf(' ', start);
                            if (end == -1)
                            {
                                end = leftpart.Length;
                            }

                            int length = end - start;

                            LectNumber = leftpart.Substring(start, length);
                        }

                        string currday = leftpart.Substring(0, leftpart.IndexOf(' '));

                        if (currmonth == month && currday == day)
                        {
                            return LectNumber;
                        }
                    }
                }
            }
            return "";
        }
    }
}
