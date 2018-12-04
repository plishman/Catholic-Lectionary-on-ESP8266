//https://stackoverflow.com/questions/21253027/get-persian-characters-initial-medial-final-unicode
//This library maps persian characters to unicode and makes them rtl, in order to use in softwares like Unity3D
//Author: Shayan Edalatmanesh

using System.Collections.Generic;
using System.Text.RegularExpressions;

public static class PersianMap
{
    static string persianChars = "ئءؤرلایيةوزژظشسيبپلاتنمكکگطضصثقفغعهخحچجدذْلآآلأأـلإإًٌٍَُِّْٰ";
    static string tashkil = "ًٌٍَُِّْٰ";
static Dictionary<char?, char[]> map = new Dictionary<char?, char[]>()
{
        {'آ', new char[] {'ﺁ', 'ﺁ', 'ﺂ', 'ﺂ'}}, 
        {'ا', new char[] {'ﺍ', 'ﺍ', 'ﺎ', 'ﺎ'}}, 
        {'أ', new char[] {'ﺃ', 'ﺃ', 'ﺄ', 'ﺄ'}}, 
        {'إ', new char[] {'ﺇ', 'ﺇ', 'ﺈ', 'ﺈ'}}, 
        {'ب', new char[] {'ﺏ', 'ﺑ', 'ﺒ', 'ﺐ'}}, 
        {'پ', new char[] {'ﭖ', 'ﭘ', 'ﭙ', 'ﭗ'}}, 
        {'ت', new char[] {'ﺕ', 'ﺗ', 'ﺘ', 'ﺖ'}}, 
        {'ث', new char[] {'ﺙ', 'ﺛ', 'ﺜ', 'ﺚ'}}, 
        {'ج', new char[] {'ﺝ', 'ﺟ', 'ﺠ', 'ﺞ'}}, 
        {'چ', new char[] {'ﭺ', 'ﭼ', 'ﭽ', 'ﭻ'}}, 
        {'ح', new char[] {'ﺡ', 'ﺣ', 'ﺤ', 'ﺢ'}}, 
        {'خ', new char[] {'ﺥ', 'ﺧ', 'ﺨ', 'ﺦ'}}, 
        {'د', new char[] {'ﺩ', 'ﺩ', 'ﺪ', 'ﺪ'}}, 
        {'ذ', new char[] {'ﺫ', 'ﺫ', 'ﺬ', 'ﺬ'}}, 
        {'ر', new char[] {'ﺭ', 'ﺭ', 'ﺮ', 'ﺮ'}}, 
        {'ز', new char[] {'ﺯ', 'ﺯ', 'ﺰ', 'ﺰ'}}, 
        {'ژ', new char[] {'ﮊ', 'ﮊ', 'ﮋ', 'ﮋ'}}, 
        {'س', new char[] {'ﺱ', 'ﺳ', 'ﺴ', 'ﺲ'}}, 
        {'ش', new char[] {'ﺵ', 'ﺷ', 'ﺸ', 'ﺶ'}}, 
        {'ص', new char[] {'ﺹ', 'ﺻ', 'ﺼ', 'ﺺ'}}, 
        {'ض', new char[] {'ﺽ', 'ﺿ', 'ﻀ', 'ﺾ'}}, 
        {'ط', new char[] {'ﻁ', 'ﻃ', 'ﻄ', 'ﻂ'}}, 
        {'ظ', new char[] {'ﻅ', 'ﻇ', 'ﻈ', 'ﻆ'}}, 
        {'ع', new char[] {'ﻉ', 'ﻋ', 'ﻌ', 'ﻊ'}}, 
        {'غ', new char[] {'ﻍ', 'ﻏ', 'ﻐ', 'ﻎ'}}, 
        {'ف', new char[] {'ﻑ', 'ﻓ', 'ﻔ', 'ﻒ'}}, 
        {'ق', new char[] {'ﻕ', 'ﻗ', 'ﻘ', 'ﻖ'}}, 
        {'ک', new char[] {'ﮎ', 'ﮐ', 'ﮑ', 'ﮏ'}}, 
        {'ك', new char[] {'ﻙ', 'ﻛ', 'ﻜ', 'ﻚ'}}, 
        {'گ', new char[] {'ﮒ', 'ﮔ', 'ﮕ', 'ﮓ'}}, 
        {'ل', new char[] {'ﻝ', 'ﻟ', 'ﻠ', 'ﻞ'}}, 
        {'م', new char[] {'ﻡ', 'ﻣ', 'ﻤ', 'ﻢ'}}, 
        {'ن', new char[] {'ﻥ', 'ﻧ', 'ﻨ', 'ﻦ'}}, 
        {'و', new char[] {'ﻭ', 'ﻭ', 'ﻮ', 'ﻮ'}}, 
        {'ؤ', new char[] {'ﺅ', 'ﺅ', 'ﺆ', 'ﺆ'}}, 
        {'ه', new char[] {'ﻩ', 'ﻫ', 'ﻬ', 'ﻪ'}}, 
        {'ة', new char[] {'ﺓ', 'ﺓ', 'ﺔ', 'ﺔ'}}, 
        {'ی', new char[] {'ﯼ', 'ﯾ', 'ﯿ', 'ﯽ'}}, 
        {'ي', new char[] {'ﻱ', 'ﻳ', 'ﻴ', 'ﻲ'}}, 
        {'ئ', new char[] {'ﺉ', 'ﺋ', 'ﺌ', 'ﺊ'}}, 
        {'ـ', new char[] { 'ـ', 'ـ', 'ـ', 'ـ'}},
};

static Dictionary<string, char[]> doubleMap = new Dictionary<string, char[]>()
{
    {"لآ", new char[] {'ﻵ', 'ﻵ', 'ﻶ', 'ﻶ'}}, 
    {"لا", new char[] {'ﻻ', 'ﻻ', 'ﻼ', 'ﻼ'}}, 
    {"لأ", new char[] {'ﻷ', 'ﻷ', 'ﻸ', 'ﻸ'}}, 
    {"لإ", new char[] {'ﻹ', 'ﻹ', 'ﻺ', 'ﻺ'}}, 
};

//The letters that after them, next letter will be stuck
static List<char?> afterStick = new List<char?>()
{
    'ب','پ','ت','ث','ج','چ','ح','خ','س','ش','ص','ض','ط','ظ','ع','غ','ف','ق','ک','ك','گ',
    'ل','م','ن','ه','ی','ي','ئ','ـ',
};

//0: Isolated
//1: Begin
//2: Middle
//3: End
static int getGlyphType(char chr, char? before, char? after)
{
    if (before == null && after == null)
        return 0;

    bool afs_chr = afterStick.Contains(chr);
    bool afs_bfr = before != null && afterStick.Contains(before);

    if (afs_chr && after != null && map.ContainsKey(after))
    {
        if (afs_chr && afs_bfr)
            return 2;
        else
            return 1;
    }
    else
    {
        if (afs_bfr)
            return 3;
        else
            return 0;
    }
}

static string convertWord(string word)
{
    string result = "";
    //Convert word
    for (int i = 0; i < word.Length; i++)
    {
        if (!map.ContainsKey(word[i]))
        {
            result += word[i].ToString();
            continue;
        }

        char? before = null, after = null;

        //Find the previous letter that is not tashkil
        for (int k = i - 1; k >= 0; k--)
        {
            if (!tashkil.Contains(word[k].ToString()))
            {
                before = word[k];
                break;
            }
        }

        //Find the next letter that is not tashkil
        for (int k = i + 1; k < word.Length; k++)
        {
            if (!tashkil.Contains(word[k].ToString()))
            {
                after = word[k];
                break;
            }
        }           

        int glyph = getGlyphType(word[i], before, after);

        //Consider doubleMap
        if (after != null)
        {
            string dm = word[i].ToString() + after.ToString();
            if (doubleMap.ContainsKey(dm))
            {
                result += doubleMap[dm][glyph].ToString();
                i++; //Skip the next letter
                continue;
            }
        }

        char mapped = map[word[i]][glyph];

        result += mapped.ToString();
    }

    //Reverse
    int len = result.Length;
    char[] rev = new char[len];
    for (int i = 0; i < len; i++)
    {
        rev[i] = result[len - i - 1];
    }

    return new string(rev);
}

static Regex wordRE = new Regex("(?<fa>[" + persianChars + "]*)(?<nfa>[^" + persianChars + "]*)");
static Regex spaceRE = new Regex("( )*([^ ]*)( )*");
static Regex persianRE = new Regex("[" + persianChars + "]");
static string convertLine(string line)
{
    //Does not contain any persian characters
    if (!persianRE.IsMatch(line))
        return line;

    string res = "";
    MatchCollection matches = wordRE.Matches(line);
    for (int i = matches.Count - 1; i >= 0; i--)
    {
        //If the non-persian word contains space, put the space on it's other side
        res += spaceRE.Replace(matches[i].Groups["nfa"].Value, "$3$2$1");

        //Convert the persian part
        res += convertWord(matches[i].Groups["fa"].Value);
    }
    return res;
}

public static string ConvertPersian(string str)
{       
    string[] lines = str.Split('\n');
    for (int i = 0; i < lines.Length; i++)
    {
        lines[i] = convertLine(lines[i]);
    }
    return string.Join("\n", lines);
}

public static string FixPersian(this string str)
{
    return ConvertPersian(str);
}
}