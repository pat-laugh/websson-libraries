//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "utilsNumbers.h"

using namespace std;
using namespace webss;

std::pair<WebssInt, NumberBase> parseInt(It& it);

Webss Parser::parseNumber(It& it)
{
	bool negative = checkNumberNegative(it);
	auto base = checkNumberBase(it);



	auto parsedPair = parseInt(it);
	auto num = parsedPair.first;
	if (negative)
		num = -num;

	if (!it || isNumberEnd(*it, language))
		return num;

	double numDouble = (double)num;
	auto magnitude = parsedPair.second;

	if (isDecimalSeparator(*it, language))
	{
		if (!skipLineJunk(++it))
			throw runtime_error(ERROR_EXPECTED_NUMBER);

		auto decimals = getDecimals(it, magnitude);
		if (negative)
			decimals = -decimals;

		numDouble += decimals;

		if (!it || isNumberEnd(*it, language))
			return numDouble;

		if (!isBaseSeparator(*it))
			throw runtime_error("invalid number");
	}

	if (!skipLineJunk(++it) || !isNumberStart(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return addNumberBase(it, numDouble, magnitude);
}

/* To get rid of!
pair<WebssInt, NumberBase> parseInt(It& it)
{
#define CheckFirstDigit(IsGoodDigit) if (!skipLineJunk(++it) || !IsGoodDigit) throw runtime_error(ERROR_EXPECTED_NUMBER)
	if (*it == '0')
	{
		if (!skipLineJunk(++it))
			return{ 0, NumberBase::Dec };

		switch (*it)
		{
		case 'b': case 'B':
			CheckFirstDigit(isDigitBin(*it));
			return{ getNumberBin(it), NumberBase::Bin };
		case 'x': case 'X':
			CheckFirstDigit(isDigitHex(*it));
			return{ getNumberHex(it), NumberBase::Hex };
		case 'd': case 'D':
			CheckFirstDigit(isDigit(*it));
			break;
		default:
			if (!isDigit(*it))
				return{ 0, NumberBase::Dec };
		}
	}
	return{ getNumberDec(it), NumberBase::Dec };
#undef CheckFirstDigit
}
*/