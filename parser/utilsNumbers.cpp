//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsNumbers.hpp"

#include <cassert>
#include <cmath>
#include <functional>
#include <various/stringBuilder.hpp>
#include <various/utils.hpp>

#include "errors.hpp"
#include "utilsSweepers.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace various;
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

WebssInt getNumber(SmartIterator& it, NumberBase base, const function<bool(char c)>& isDigit)
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
	catch (const out_of_range&)
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

WebssInt webss::parseIntBin(SmartIterator& it) { return getNumber(it, NumberBase::BIN, isDigitBin); }
WebssInt webss::parseIntOct(SmartIterator& it) { return getNumber(it, NumberBase::OCT, isDigitOct); }
WebssInt webss::parseIntDec(SmartIterator& it) { return getNumber(it, NumberBase::DEC, isDigitDec); }
WebssInt webss::parseIntHex(SmartIterator& it) { return getNumber(it, NumberBase::HEX, isDigitHex); }

WebssInt webss::parseInt(SmartIterator& it, NumberBase base)
{
	if (base == NumberBase::DEC)
		return parseIntDec(it);
	else if (base == NumberBase::HEX)
		return parseIntHex(it);
	else if (base == NumberBase::BIN)
		return parseIntBin(it);
	assert(base == NumberBase::OCT);
	return parseIntOct(it);
}

double webss::parseDecimals(SmartIterator& it, NumberBase base)
{
	//max digit numbers: http://stackoverflow.com/questions/17244898/maximum-number-of-decimal-digits-that-can-affect-a-double#17245451
	//floor(<MantissaDouble(52)> * log 2 / log <base>) + 2
	if (base == NumberBase::DEC)
		return checkDecimals(it, base, 17, isDigitDec, decToInt);
	else if (base == NumberBase::HEX)
		return checkDecimals(it, base, 15, isDigitHex, hexToInt);
	else if (base == NumberBase::BIN)
		return checkDecimals(it, base, 54, isDigitBin, binToInt);
	assert(base == NumberBase::OCT);
	return checkDecimals(it, base, 19, isDigitOct, octToInt);
}

double webss::addExponent(SmartIterator& it, double num, NumberBase base)
{
	assert(base == NumberBase::DEC || base == NumberBase::HEX || base == NumberBase::BIN || base == NumberBase::OCT);
	bool negative = checkNumberNegative(it);
	auto numBase = parseIntDec(it);
	double exp = (double)(negative ? -numBase : numBase);
	num *= pow((double)(base == NumberBase::DEC ? base : NumberBase::BIN), exp);
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
		return NumberBase::DEC;
	
	switch (it.peek())
	{
	case 'b': case 'B':
		return checkDigitWrapper(it, isDigitBin, NumberBase::BIN);
	case 'c': case 'C':
		return checkDigitWrapper(it, isDigitOct, NumberBase::OCT);
	case 'd': case 'D':
		return checkDigitWrapper(it, isDigitDec, NumberBase::DEC);
	case 'x': case 'X':
		return checkDigitWrapper(it, isDigitHex, NumberBase::HEX);
	default:
		return NumberBase::DEC;
	}
}

bool webss::isNumberEnd(char c, NumberBase base)
{
	return c != CHAR_DECIMAL_SEPARATOR && !isBaseSeparator(c, base);
}

bool webss::isBaseSeparator(char c, NumberBase base)
{
	if (base == NumberBase::DEC)
		return c == 'e' || c == 'E';
	assert(base == NumberBase::HEX || base == NumberBase::BIN || base == NumberBase::OCT);
	return c == 'p' || c == 'P';
}