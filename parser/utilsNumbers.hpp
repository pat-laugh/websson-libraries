//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <various/smartIterator.hpp>

#include "structures/types.hpp"

namespace webss
{
	enum class NumberBase : int { BIN = 2, OCT = 8, DEC = 10, HEX = 16 };

	WebssInt parseInt(various::SmartIterator& it, NumberBase base);

	WebssInt parseIntBin(various::SmartIterator& it);
	WebssInt parseIntOct(various::SmartIterator& it);
	WebssInt parseIntDec(various::SmartIterator& it);
	WebssInt parseIntHex(various::SmartIterator& it);

	//gets a decimal number (0 <= x < 1) based on base
	double parseDecimals(various::SmartIterator& it, NumberBase base);

	//adds base to num
	double addExponent(various::SmartIterator& it, double num, NumberBase base);

	bool checkNumberNegative(various::SmartIterator& it);
	
	NumberBase checkNumberBase(various::SmartIterator& it);

	//anything other than a decimal or magnitude separator can end a number
	bool isNumberEnd(char c, NumberBase base);

	//'e' or 'E' for base dec; 'p' or 'P' for other bases
	bool isBaseSeparator(char c, NumberBase base);
}