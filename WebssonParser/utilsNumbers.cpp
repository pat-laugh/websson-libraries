//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsNumbers.h"
#include "utilsSweepers.h"

#include <cmath>

using namespace std;
using namespace webss;

#define FUNCTIONS_BIN [&]() { return isDigitBin(*it); }, [&]() { return binToInt(*it); }
#define FUNCTIONS_DEC [&]() { return isDigit(*it); }, [&]() { return charToInt(*it); }
#define FUNCTIONS_HEX [&]() { return isDigitHex(*it); }, [&]() { return hexToInt(*it); }

WebssInt getNumber(SmartIterator& it, NumberMagnitude mag, function<bool()> func1, function<int()> func2)
{
	WebssInt number = func2();
	while (skipLineJunk(++it) && func1())
		number = number * mag + func2();
	return number;
}

double checkDecimals(SmartIterator& it, NumberMagnitude mag, function<bool()> func1, function<int()> func2)
{
	double numDouble = 0;
	int decimalMultiplier = 1;
	if (!func1())
		throw runtime_error(ERROR_EXPECTED_NUMBER);

	do
		numDouble += (double)func2() / (decimalMultiplier *= mag);
	while (skipLineJunk(++it) && func1());
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
		throw domain_error(ERROR_UNDEFINED);
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
	if ((*it == '-' || *it == '+') && (!skipLineJunk(++it) || !isDigit(*it)))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return negative;
}