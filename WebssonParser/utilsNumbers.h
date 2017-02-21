//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "WebssonStructures/types.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/iterators.h"

namespace webss
{
	enum class NumberBase : int { Bin = 2, Oct = 8, Dec = 10, Hex = 16 };

	WebssInt getNumberBin(SmartIterator& it); //parserNumbers
	WebssInt getNumberDec(SmartIterator& it); //parserNumbers
	WebssInt getNumberHex(SmartIterator& it); //parserNumbers, unicode

	//gets a decimal number (0 <= x < 1) based on base
	double getDecimals(SmartIterator& it, NumberBase base); //parserNumbers

	//adds base to num
	double addNumberBase(SmartIterator& it, double num, NumberBase base); //parserNumbers

	//return true if number is positive, else if negative
	//if started with '-' or '+', checks if next non-whitespace character is a digit
	//if not, throws error
//	bool checkNumberStart(SmartIterator& it); //parserNumbers

	bool checkNumberNegative(SmartIterator& it);
	
	NumberBase checkNumberBase(SmartIterator& it);
}