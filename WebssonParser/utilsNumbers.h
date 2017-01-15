//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "iterators.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/errors.h"
#include "WebssonStructures/types.h"

namespace webss
{
	enum NumberMagnitude { MAGNITUDE_BIN = 2, MAGNITUDE_DEC = 10, MAGNITUDE_HEX = 16 };

	WebssInt getNumberBin(SmartIterator& it); //parserNumbers
	WebssInt getNumberDec(SmartIterator& it); //parserNumbers
	WebssInt getNumberHex(SmartIterator& it); //parserNumbers, unicode

	//gets a decimal number (0 <= x < 1) based on mag
	double getDecimals(SmartIterator& it, NumberMagnitude mag); //parserNumbers

	//adds magnitude to num
	double addNumberMagnitude(SmartIterator& it, double num, NumberMagnitude mag); //parserNumbers

	//return true if number is positive, else if negative
	//if started with '-' or '+', checks if next non-whitespace character is a digit
	//if not, throws error
	bool checkNumberStart(SmartIterator& it); //parserNumbers
}