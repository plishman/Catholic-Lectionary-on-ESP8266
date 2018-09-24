using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Xml;
using System.Xml.Xsl;

namespace Bible_XML
{
    class Program
    {
        private const string stylesheet = @"csv.xslt";

        //private const string sourceFile = @"SF_2016-10-10_CZE_CZBKR_(CZECH BKR).xml";
        //private const string outputFile = @"cze_czbkr.csv";

        //private const string sourceFile = @"SF_2009-01-23_ENG_KJV_(KING JAMES VERSION).xml";
        //private const string outputFile = @"eng_kjv.csv";

        //private const string sourceFile = @"SF_2004-11-13_FRE_DEJER_(LA BIBLE DE JÉRUSALEM).xml";
        //private const string outputFile = @"njb_fr.csv";

        //private const string sourceFile = @"SF_2009-01-20_ITA_CEI74_(LA BIBBIA CEI 74).xml";
        //private const string outputFile = @"cei_it.csv";

        //private const string sourceFile = @"SF_2014-09-06_LAT_VULGATE_(BIBLIA SACRA VULGATA).xml";
        //private const string outputFile = @"lat_v.csv";

        //private const string sourceFile = @"SF_2009-01-20_KOR_KOR_(KOREAN VERSION).xml";
        //private const string outputFile = @"bbl_kor.csv";

        //private const string sourceFile = @"SF_2009-01-20_CHI_CVS_(NEW CHINESE VERSION SIMPLIFIED).xml";
        //private const string outputFile = @"chi_cvs.csv";

        private const string sourceFile = @"SF_2009-01-20_ARA_ARASVD_(SMITH VAN DYKE ARABIC BIBLE).xml";
        private const string outputFile = @"svd_ara.csv";

        static void Main(string[] args)
        {
            if (File.Exists(outputFile)) File.Delete(outputFile);
            
            // Enable XSLT debugging.  
            XslCompiledTransform xslt = new XslCompiledTransform(true);

            // Compile the style sheet.  
            xslt.Load(stylesheet);

      // Execute the XSLT transform.  
            FileStream outputStream = new FileStream(outputFile, FileMode.Append);
            xslt.Transform(sourceFile, null, outputStream);
        }
    }
}

