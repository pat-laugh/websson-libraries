//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utilsNumbers.h"

#include <cassert>
#include <cmath>
#include <functional>

#include "errors.h"
#include "utilsSweepers.h"
#include "WebssonUtils/stringBuilder.h"

using namespace std;
using namespace webss;

#define FUNCTIONS_BIN isDigitBin, binToInt
#define FUNCTIONS_DEC isDigitDec, decToInt
#define FUNCTIONS_HEX isDigitHex, hexToInt

WebssInt getNumber(SmartIterator& it, NumberBase base, bool(*isDigit)(char c), int(*charToInt)(char c))
{
	StringBuilder sb;
	do
		sb += *it;
	while (skipLineJunk(++it) && isDigit(*it));

	WebssInt number;
	try
	{
		string s = sb.str();
		number = std::stoll(s, nullptr, (int)base);
	}
	catch (out_of_range e)
	{
		throw runtime_error("integer is outside bounds");
	}
	return number;
}

double checkDecimals(SmartIterator& it, NumberBase base, bool(*isDigit)(char c), int(*charToInt)(char c))
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
		dec = std::stoll(s, nullptr, (int)base);
	}
	catch (out_of_range e)
	{
		throw runtime_error("decimals are outside bounds");
	}

	return dec / std::pow((double)base, (double)numDigits);
}

WebssInt webss::getNumberBin(SmartIterator& it) { return getNumber(it, NumberBase::Bin, FUNCTIONS_BIN); }
WebssInt webss::getNumberDec(SmartIterator& it) { return getNumber(it, NumberBase::Dec, FUNCTIONS_DEC); }
WebssInt webss::getNumberHex(SmartIterator& it) { return getNumber(it, NumberBase::Hex, FUNCTIONS_HEX); }

double webss::getDecimals(SmartIterator& it, NumberBase base)
{
	switch (base)
	{
	case NumberBase::Bin:
		return checkDecimals(it, base, FUNCTIONS_BIN);
	case NumberBase::Dec:
		return checkDecimals(it, base, FUNCTIONS_DEC);
	case NumberBase::Hex:
		return checkDecimals(it, base, FUNCTIONS_HEX);
	default:
		assert(false); throw domain_error("");
	}
}

double webss::addNumberBase(SmartIterator& it, double num, NumberBase base)
{
	bool negative = checkNumberStart(it);
	auto numBase = getNumberDec(it);
	return num *= pow((double)base, (double)(negative ? -numBase : numBase));
}

bool checkDigit(SmartIterator& it, const function<bool(char c)>& isDigit)
{
	if (!it)
		return false;
	if (isDigit(*it))
		return true;
	if (isLineJunk(*it))
		return checkDigit(skipLineJunk(++it), isDigit);
	return false;
}

bool webss::checkNumberNegative(SmartIterator& it)
{
	bool negative = *it == '-';
	if ((negative || *it == '+') && (!++it || !isDigitDec(*it)))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return !negative;
}

NumberBase checkDigitWrapper(SmartIterator& it, const function<bool(char c)>& isDigit, NumberBase base)
{
	if (!checkDigit(it.readTwo(), isDigit))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return base;
}

NumberBase webss::checkNumberBase(SmartIterator& it)
{
	if (*it != '0' || it.peekEnd())
		return NumberBase::Dec;
	
	switch (it.peek())
	{
	case 'b': case 'B':
		return checkDigitWrapper(it, isDigitBin, NumberBase::Bin);
	case 'c': case 'C':
		return checkDigitWrapper(it, isDigitOct, NumberBase::Oct);
	case 'd': case 'D':
		return checkDigitWrapper(it, isDigitDec, NumberBase::Dec);
	case 'x': case 'X':
		return checkDigitWrapper(it, isDigitHex, NumberBase::Hex);
	default:
		return NumberBase::Dec;
	}
}

