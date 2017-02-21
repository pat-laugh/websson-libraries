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

WebssInt getNumber(SmartIterator& it, NumberBase base, const function<bool(char c)>& isDigit, const function<int(char c)>& charToInt)
{
	StringBuilder sb;
	do
		sb += *it;
	while (checkDigit(++it, isDigit));

	try
	{
		string s = sb.str();
		return std::stoll(s, nullptr, (int)base);
	}
	catch (out_of_range e)
	{
		throw runtime_error("integer is outside bounds");
	}
}

double checkDecimals(SmartIterator& it, NumberBase base, int maxDigits, const function<bool(char c)>& isDigit, const function<int(char c)>& charToInt)
{
	if (!checkDigit(it, isDigit))
		throw runtime_error(ERROR_EXPECTED_NUMBER);

	int numDigits = 1;
	WebssInt dec = charToInt(*it);
	while (checkDigit(++it, isDigit))
		if (numDigits < maxDigits)
		{
			dec = dec * (int)base + charToInt(*it);
			++numDigits;
		}

	return dec / std::pow((double)base, (double)numDigits);
}

WebssInt webss::parseIntBin(SmartIterator& it) { return getNumber(it, NumberBase::Bin, isDigitBin, binToInt); }
WebssInt webss::parseIntOct(SmartIterator& it) { return getNumber(it, NumberBase::Oct, isDigitOct, octToInt); }
WebssInt webss::parseIntDec(SmartIterator& it) { return getNumber(it, NumberBase::Dec, isDigitDec, decToInt); }
WebssInt webss::parseIntHex(SmartIterator& it) { return getNumber(it, NumberBase::Hex, isDigitHex, hexToInt); }

WebssInt webss::parseInt(SmartIterator& it, NumberBase base)
{
	switch (base)
	{
	case NumberBase::Bin:
		return parseIntBin(it);
	case NumberBase::Oct:
		return parseIntOct(it);
	case NumberBase::Dec:
		return parseIntDec(it);
	case NumberBase::Hex:
		return parseIntHex(it);
	default:
		assert(false); throw domain_error("");
	}
}

double webss::parseDecimals(SmartIterator& it, NumberBase base)
{
	//max digit numbers: http://stackoverflow.com/questions/17244898/maximum-number-of-decimal-digits-that-can-affect-a-double#17245451
	//floor(<MantissaDouble(52)> * log 2 / log <base>) + 2
	switch (base)
	{
	case NumberBase::Bin:
		return checkDecimals(it, base, 54, isDigitBin, binToInt);
	case NumberBase::Oct:
		return checkDecimals(it, base, 19, isDigitOct, octToInt);
	case NumberBase::Dec:
		return checkDecimals(it, base, 17, isDigitDec, decToInt);
	case NumberBase::Hex:
		return checkDecimals(it, base, 15, isDigitHex, hexToInt);
	default:
		assert(false); throw domain_error("");
	}
}

double webss::addNumberBase(SmartIterator& it, double num, NumberBase base)
{
	bool negative = checkNumberNegative(it);
	auto numBase = parseIntDec(it);
	return num *= pow((double)base, (double)(negative ? -numBase : numBase));
}

bool webss::checkNumberNegative(SmartIterator& it)
{
	bool negative = *it == '-';
	if ((negative || *it == '+') && (!++it || !isDigitDec(*it)))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return negative;
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

