//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "utilsNumbers.h"

using namespace std;
using namespace webss;

std::pair<WebssInt, NumberMagnitude> parseInt(It& it);

Webss Parser::parseNumber(It& it)
{
	bool negative = checkNumberStart(it);
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

		if (!isMagnitudeSeparator(*it))
			throw runtime_error("invalid number");
	}

	if (!skipLineJunk(++it) || !isNumberStart(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return addNumberMagnitude(it, numDouble, magnitude);
}

pair<WebssInt, NumberMagnitude> parseInt(It& it)
{
#define CHECK_FIRST_DIGIT(x) if (!skipLineJunk(++it) || !x) throw runtime_error(ERROR_EXPECTED_NUMBER)
	if (*it == '0')
	{
		if (!skipLineJunk(++it))
			return{ 0, MAGNITUDE_DEC };

		switch (*it)
		{
		case 'b': case 'B':
			CHECK_FIRST_DIGIT(isDigitBin(*it));
			return{ getNumberBin(it), MAGNITUDE_BIN };
		case 'x': case 'X':
			CHECK_FIRST_DIGIT(isDigitHex(*it));
			return{ getNumberHex(it), MAGNITUDE_HEX };
		case 'd': case 'D':
			CHECK_FIRST_DIGIT(isDigit(*it));
			break;
		default:
			if (!isDigit(*it))
				return{ 0, MAGNITUDE_DEC };
		}
	}
	return{ getNumberDec(it), MAGNITUDE_DEC };
#undef CHECK_FIRST_DIGIT
}
