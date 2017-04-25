//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "structures/types.h"
#include "utils/iterators.h"

namespace webss
{
	enum class NumberBase : int { BIN = 2, OCT = 8, DEC = 10, HEX = 16 };

	WebssInt parseInt(SmartIterator& it, NumberBase base);

	WebssInt parseIntBin(SmartIterator& it);
	WebssInt parseIntOct(SmartIterator& it);
	WebssInt parseIntDec(SmartIterator& it);
	WebssInt parseIntHex(SmartIterator& it);

	//gets a decimal number (0 <= x < 1) based on base
	double parseDecimals(SmartIterator& it, NumberBase base);

	//adds base to num
	double addExponent(SmartIterator& it, double num, NumberBase base);

	bool checkNumberNegative(SmartIterator& it);
	
	NumberBase checkNumberBase(SmartIterator& it);

	//anything other than a decimal or magnitude separator can end a number
	bool isNumberEnd(char c, NumberBase base);

	//'e' or 'E' for base dec; 'p' or 'P' for other bases
	bool isBaseSeparator(char c, NumberBase base);
}