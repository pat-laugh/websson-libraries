//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utilsNumbers.h"

#include <cassert>
#include <cmath>

#include "errors.h"
#include "utilsSweepers.h"
#include "WebssonUtils/stringBuilder.h"

using namespace std;
using namespace webss;

#define FUNCTIONS_BIN isDigitBin, binToInt
#define FUNCTIONS_DEC isDigit, charToInt
#define FUNCTIONS_HEX isDigitHex, hexToInt

WebssInt getNumber(SmartIterator& it, NumberMagnitude mag, bool(*isDigit)(char c), int(*charToInt)(char c))
{
	StringBuilder sb;
	do
		sb += *it;
	while (skipLineJunk(++it) && isDigit(*it));

	WebssInt number;
	try
	{
		string s = sb.str();
		number = std::stoll(s, nullptr, (int)mag);
	}
	catch (out_of_range e)
	{
		throw runtime_error("integer is outside bounds");
	}
	return number;
}

double checkDecimals(SmartIterator& it, NumberMagnitude mag, bool(*isDigit)(char c), int(*charToInt)(char c))
{
	if (!isDigit(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);

	int numDigits = 0;
	StringBuilder sb;
	do
	{
		sb += *it;
		if (++numDigits > 17) //magic number: http://stackoverflow.com/questions/17244898/maximum-number-of-decimal-digits-that-can-affect-a-double#17245451
			throw runtime_error("too many decimals");
	}
	while (skipLineJunk(++it) && isDigit(*it));

	WebssInt dec;
	try
	{
		string s = sb.str();
		dec = std::stoll(s, nullptr, (int)mag);
	}
	catch (out_of_range e)
	{
		throw runtime_error("decimals are outside bounds");
	}

	return dec / std::pow((double)mag, (double)numDigits);
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
		assert(false); throw domain_error("");
	}
}

double webss::addNumberMagnitude(SmartIterator& it, double num, NumberMagnitude mag)
{
	bool negative = checkNumberStart(it);
	auto numMagnitude = getNumberDec(it);
	return num *= pow((double)mag, (double)(negative ? -numMagnitude : numMagnitude));
}

bool checkDigit(SmartIterator& it)
{
	if (!it)
		return false;
	if (isDigit(*it))
		return true;
	if (isLineJunk(*it))
		return checkDigit(skipLineJunk(++it));
	return false;
}

bool webss::checkNumberStart(SmartIterator& it)
{
	bool negative = *it == '-';
	if ((negative || *it == '+') && !checkDigit(++it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return negative;
}
