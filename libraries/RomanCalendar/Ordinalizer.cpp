#include "Ordinalizer.h"

const String Ordinalizer::types[8] = {"none", "english", "french", "latin", "chinese", "e", "period", "ordinal"};

Ordinalizer::Ordinalizer(String type) {
	int i = 0;
	type.toLowerCase();
	
	while (types[i] != type && i < ORDINAL_COUNT) {
		i++;
	}
	
	if (i < ORDINAL_COUNT) {
		_ordinal_type = i;
	}
	
	I2CSerial.printf("\nOrdinal type is %d, %s\n", _ordinal_type, type.c_str());
		
}

String Ordinalizer::ordinalize(int number) {
	switch(_ordinal_type) {
		case 0:
			//none
			return String(number);
			break;
			
		case 1:
			//english
			return ord_english(number);
		
		case 2:
			//french
			return ord_french(number);
			break;
		case 3:
			//latin
			return to_roman((unsigned)number) + "º";
		
		case 4:
			//chinese
			return "第" + String(number);
			break;
			
		case 5:
			//e
			return String(number) + "e";
			break;
			
		case 6:
			//period
			return String(number) + ".";
			break;
			
		case 7:
			//ordinal
			return String(number) + "º";
			break;
	}

	return String(number);
}

String Ordinalizer::ord_english(int number) {
	int modulo = number % 10;
	if ((number / 10) == 1) modulo = 9;

	String ord;

	switch (modulo) {
	case 1:
		ord = "st";
		break;

	case 2:
		ord = "nd";
		break;

	case 3:
		ord = "rd";
		break;

	default:
		ord = "th";
	}

	ord = String(number) + ord;
	return ord;
}

String Ordinalizer::ord_french(int number) {
	if (number == 1) {
		return String(number) + "er";
	} else {
		return String(number) + "e";
	}
}

//https://stackoverflow.com/questions/19266018/converting-integer-to-roman-numeral
String Ordinalizer::to_roman(unsigned int value)
{
    struct romandata_t { unsigned int value; char const* numeral; };
    const struct romandata_t romandata[] =
    {
        {1000, "M"}, {900, "CM"},
        {500, "D"}, {400, "CD"},
        {100, "C"}, { 90, "XC"},
        { 50, "L"}, { 40, "XL"},
        { 10, "X"}, { 9, "IX"},
        { 5, "V"}, { 4, "IV"},
        { 1, "I"},
        { 0, NULL} // end marker
    };

    String result;
    for (const romandata_t* current = romandata; current->value > 0; ++current)
    {
        while (value >= current->value)
        {
            result += current->numeral;
            value -= current->value;
        }
    }
    return result;
}