//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsNumbers.h"
#include "utilsSweepers.h"

#include <cassert>
#include <cmath>

using namespace std;
using namespace webss;

#define FUNCTIONS_BIN isDigitBin, binToInt
#define FUNCTIONS_DEC isDigit, charToInt
#define FUNCTIONS_HEX isDigitHex, hexToInt

WebssInt getNumber(SmartIterator& it, NumberMagnitude mag, bool(*isDigit)(char c), int(*charToInt)(char c))
{
	WebssInt number = charToInt(*it);
	while (skipLineJunk(++it) && isDigit(*it))
		number = number * mag + charToInt(*it);
	return number;
}

double checkDecimals(SmartIterator& it, NumberMagnitude mag, bool(*isDigit)(char c), int(*charToInt)(char c))
{
	double numDouble = 0;
	int decimalMultiplier = 1;
	if (!isDigit(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);

	do
		numDouble += (double)charToInt(*it) / (decimalMultiplier *= mag);
	while (skipLineJunk(++it) && isDigit(*it));
	return numDouble;
}

WebssInt webss::getNumberBin(SmartIterator& it) { return getNumber(it, MAGNITUDE_BIN, FUNCTIONS_BIN); }
WebssInt webss::getNumberDec(SmartIterator& it) { return getNumber(it, MAGNITUDE_DEC, FUNCTIONS_DEC); }
WebssInt webss::getNumberHex(SmartIterator& it) { return getNumber(it, MAGNITUDE_HEX, FUNCTIONS_HEX); }

double webss::getDecimals(SmartIterator& it, NumberMagnitude mag)
{
	switch (mag)
	{
	case MAGNITUDE_BIN:
		return checkDecimals(it, mag, FUNCTIONS_BIN);
	case MAGNITUDE_DEC:
		return checkDecimals(it, mag, FUNCTIONS_DEC);
	case MAGNITUDE_HEX:
		return checkDecimals(it, mag, FUNCTIONS_HEX);
	default:
		assert(false);
	}
}

double webss::addNumberMagnitude(SmartIterator& it, double num, NumberMagnitude mag)
{
	bool negative = checkNumberStart(it);
	auto numMagnitude = getNumberDec(it);
	return num *= pow((double)mag, (double)(negative ? -numMagnitude : numMagnitude));
}

bool webss::checkNumberStart(SmartIterator& it)
{
	bool negative = *it == '-';
	if ((negative || *it == '+') && (!skipLineJunk(++it) || !isDigit(*it)))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return negative;
}
