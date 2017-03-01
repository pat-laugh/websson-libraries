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
	catch (const out_of_range& e)
	{
		throw overflow_error("integer is outside bounds");
	}
}

double checkDecimals(SmartIterator& it, NumberBase base, int maxDigits, const function<bool(char c)>& isDigit, const function<int(char c)>& charToInt)
{
	static const int MAX_LEADING_ZEROS = 23; //1 yocto smallest unit
	static const function<bool(char c)> isZero = [](char c) { return c == '0'; };

	if (!checkDigit(it, isDigit))
		throw runtime_error(ERROR_EXPECTED_NUMBER);

	//skim through leading zeros
	int power = 1;
	if (*it == '0')
	{
		do
			if (++power > MAX_LEADING_ZEROS + 1)
			{
				while (checkDigit(++it, isDigit))
					;
				return 0;
			}
		while (checkDigit(++it, isZero));

		if (!checkDigit(it, isDigit))
			return 0;
	}

	//parse significant digits
	int numDigits = 1;
	WebssInt dec = charToInt(*it);
	while (checkDigit(++it, isDigit))
		if (numDigits < maxDigits)
		{
			dec = dec * (int)base + charToInt(*it);
			++numDigits;
			++power;
		}

	return dec / std::pow((double)base, (double)power);
}

WebssInt webss::parseIntBin(SmartIterator& it) { return getNumber(it, NumberBase::Bin, isDigitBin, binToInt); }
WebssInt webss::parseIntOct(SmartIterator& it) { return getNumber(it, NumberBase::Oct, isDigitOct, octToInt); }
WebssInt webss::parseIntDec(SmartIterator& it) { return getNumber(it, NumberBase::Dec, isDigitDec, decToInt); }
WebssInt webss::parseIntHex(SmartIterator& it) { return getNumber(it, NumberBase::Hex, isDigitHex, hexToInt); }

WebssInt webss::parseInt(SmartIterator& it, NumberBase base)
{
	if (base == NumberBase::Dec)
		return parseIntDec(it);
	else if (base == NumberBase::Hex)
		return parseIntHex(it);
	else if (base == NumberBase::Bin)
		return parseIntBin(it);
	assert(base == NumberBase::Oct);
	return parseIntOct(it);
}

double webss::parseDecimals(SmartIterator& it, NumberBase base)
{
	//max digit numbers: http://stackoverflow.com/questions/17244898/maximum-number-of-decimal-digits-that-can-affect-a-double#17245451
	//floor(<MantissaDouble(52)> * log 2 / log <base>) + 2
	if (base == NumberBase::Dec)
		return checkDecimals(it, base, 17, isDigitDec, decToInt);
	else if (base == NumberBase::Hex)
		return checkDecimals(it, base, 15, isDigitHex, hexToInt);
	else if (base == NumberBase::Bin)
		return checkDecimals(it, base, 54, isDigitBin, binToInt);
	assert(base == NumberBase::Oct);
	return checkDecimals(it, base, 19, isDigitOct, octToInt);
}

double webss::addExponent(SmartIterator& it, double num, NumberBase base)
{
	assert(base == NumberBase::Dec || base == NumberBase::Hex || base == NumberBase::Bin || base == NumberBase::Oct);
	bool negative = checkNumberNegative(it);
	auto numBase = parseIntDec(it);
	double exp = negative ? -numBase : numBase;
	num *= pow((double)(base == NumberBase::Dec ? base : NumberBase::Bin), exp);
	if (!std::isfinite(num))
		throw runtime_error("invalid number, either infinite or NaN");
	return num;
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
	if (!checkDigit(it.incTwo(), isDigit))
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

bool webss::isNumberEnd(char c, Language lang, NumberBase base)
{
	return !isDecimalSeparator(c, lang) && !isBaseSeparator(c, base);
}

bool webss::isDecimalSeparator(char c, Language lang)
{
	assert(lang == DEFAULT || lang == INTL || lang == EN || lang == FR);
	if (c == '.')
		return lang == DEFAULT || lang == INTL || lang == EN;
	return c == ',' && (lang == INTL || lang == FR);
}

bool webss::isBaseSeparator(char c, NumberBase base)
{
	if (base == NumberBase::Dec)
		return c == 'e' || c == 'E';
	assert(base == NumberBase::Hex || base == NumberBase::Bin || base == NumberBase::Oct);
	return c == 'p' || c == 'P';
}